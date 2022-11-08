import unittest
import cjyx
import cjyx.util
import vtk


class CjyxUtilGetNodeTest(unittest.TestCase):

    def setUp(self):
        cjyx.dmmlScene.Clear(0)
        self.nodes = self._configure_scene(cjyx.dmmlScene)
        self.assertEqual(cjyx.dmmlScene.GetNumberOfNodesByClass("vtkDMMLScalarVolumeNode"), 4)

    @staticmethod
    def _configure_scene(scene):
        nodes = [cjyx.vtkDMMLScalarVolumeNode() for idx in range(4)]
        scene.AddNode(nodes[0]).SetName("Volume1")
        scene.AddNode(nodes[1]).SetName("Volume2")
        scene.AddNode(nodes[2]).SetName("Volume")
        scene.AddNode(nodes[3]).SetName("Volume")
        nodes[0].SetHideFromEditors(1)
        return nodes

    def test_getFirstNodeByName(self):
        self.assertEqual(cjyx.util.getFirstNodeByName("Volume", 'vtkDMMLScalarVolumeNode').GetName(), "Volume1")

    def test_getNode(self):

        # Test handling of requesting non-existing node
        with self.assertRaises(cjyx.util.DMMLNodeNotFoundException):
            cjyx.util.getNode("")
        with self.assertRaises(cjyx.util.DMMLNodeNotFoundException):
            cjyx.util.getNode("NotExistingNodeName")

        # For the following tests, use a dedicated scene where
        # all nodes are known.
        scene = cjyx.vtkDMMLScene()
        nodes = self._configure_scene(scene)
        self.assertEqual(cjyx.util.getNode("*", scene=scene), nodes[0])
        self.assertEqual(cjyx.util.getNode("*", index=1, scene=scene), nodes[1])
        self.assertEqual(cjyx.util.getNode(scene=scene), nodes[0])

    def test_getNodes(self):
        self.assertEqual(cjyx.util.getNodes(), cjyx.util.getNodes("*"))
        self.assertEqual(cjyx.util.getNodes(""), {})

        self.assertIn("Volume1", cjyx.util.getNodes("*"))
        self.assertIn("Volume2", cjyx.util.getNodes("*"))
        self.assertIn("Volume1", cjyx.util.getNodes())
        self.assertIn("Volume2", cjyx.util.getNodes())

        self.assertEqual(list(cjyx.util.getNodes("Volume1").keys()), ["Volume1"])
        self.assertEqual(list(cjyx.util.getNodes("Volume2").keys()), ["Volume2"])
        self.assertEqual(list(cjyx.util.getNodes("Volume*").keys()), ["Volume1", "Volume2", "Volume"])

    def test_getNodesMultipleNodesSharingName(self):

        self.assertIn("Volume", cjyx.util.getNodes("Volume"))
        self.assertIn("Volume", cjyx.util.getNodes("Volume", useLists=True))

        self.assertEqual(list(cjyx.util.getNodes("Volume").keys()), ["Volume"])
        self.assertIsInstance(cjyx.util.getNodes("Volume")["Volume"], vtk.vtkObject)
        self.assertEqual(list(cjyx.util.getNodes("Volume", useLists=True).keys()), ["Volume"])
        self.assertIsInstance(cjyx.util.getNodes("Volume", useLists=True)["Volume"], list)
