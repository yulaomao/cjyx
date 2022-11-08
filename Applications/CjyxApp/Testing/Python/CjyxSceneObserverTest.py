import unittest
import cjyx
import vtk


class testClass:
    """ Check that cjyx exits correctly after adding an observer to the dmml scene
    """

    def callback(self, caller, event):
        print(f'Got {event} from {caller}')

    def setUp(self):
        print("Adding observer to the scene")
        self.tag = cjyx.dmmlScene.AddObserver(vtk.vtkCommand.ModifiedEvent, self.callback)
        print("Modify the scene")
        cjyx.dmmlScene.Modified()


class CjyxSceneObserverTest(unittest.TestCase):

    def setUp(self):
        pass

    def test_testClass(self):
        test = testClass()
        test.setUp()
