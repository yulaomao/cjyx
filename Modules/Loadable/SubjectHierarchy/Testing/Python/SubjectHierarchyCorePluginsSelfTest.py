import qt

import cjyx
from cjyx.ScriptedLoadableModule import *


#
# SubjectHierarchyCorePluginsSelfTest
#

class SubjectHierarchyCorePluginsSelfTest(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        parent.title = "SubjectHierarchyCorePluginsSelfTest"
        parent.categories = ["Testing.TestCases"]
        parent.dependencies = ["SubjectHierarchy"]
        parent.contributors = ["Csaba Pinter (Queen's)"]
        parent.helpText = """
    This is a self test for the Subject hierarchy core plugins.
    """
        parent.acknowledgementText = """
This file was originally developed by Csaba Pinter, PerkLab, Queen's University and was supported through the Applied Cancer
 Research Unit program of Cancer Care Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care"""
        self.parent = parent

        # Add this test to the SelfTest module's list for discovery when the module
        # is created.  Since this module may be discovered before SelfTests itself,
        # create the list if it doesn't already exist.
        try:
            cjyx.selfTests
        except AttributeError:
            cjyx.selfTests = {}
        cjyx.selfTests['SubjectHierarchyCorePluginsSelfTest'] = self.runTest

    def runTest(self, msec=100, **kwargs):
        tester = SubjectHierarchyCorePluginsSelfTestTest()
        tester.runTest()


#
# SubjectHierarchyCorePluginsSelfTestWidget
#

class SubjectHierarchyCorePluginsSelfTestWidget(ScriptedLoadableModuleWidget):
    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)


#
# SubjectHierarchyCorePluginsSelfTestLogic
#

class SubjectHierarchyCorePluginsSelfTestLogic(ScriptedLoadableModuleLogic):
    """This class should implement all the actual
    computation done by your module.  The interface
    should be such that other python code can import
    this class and make use of the functionality without
    requiring an instance of the Widget
    """

    def __init__(self):
        pass


class SubjectHierarchyCorePluginsSelfTestTest(ScriptedLoadableModuleTest):
    """
    This is the test case for your scripted module.
    """

    def setUp(self):
        """ Do whatever is needed to reset the state - typically a scene clear will be enough.
        """
        cjyx.dmmlScene.Clear(0)

        self.delayMs = 700

        # TODO: Comment out (sample code for debugging)
        # logFile = open('d:/pyTestLog.txt', 'w')
        # logFile.write(repr(cjyx.modules.SubjectHierarchyCorePluginsSelfTest) + '\n')
        # logFile.write(repr(cjyx.modules.subjecthierarchy) + '\n')
        # logFile.close()

    def runTest(self):
        """Run as few or as many tests as needed here.
        """
        self.setUp()
        self.test_SubjectHierarchyCorePluginsSelfTest_FullTest1()

    # ------------------------------------------------------------------------------
    def test_SubjectHierarchyCorePluginsSelfTest_FullTest1(self):
        # Check for SubjectHierarchy module
        self.assertTrue(cjyx.modules.subjecthierarchy)

        # Switch to subject hierarchy module so that the changes can be seen as the test goes
        cjyx.util.selectModule('SubjectHierarchy')

        self.section_SetupPathsAndNames()
        self.section_MarkupRole()
        self.section_CloneNode()
        self.section_SegmentEditor()

    # ------------------------------------------------------------------------------
    def section_SetupPathsAndNames(self):
        # Set constants
        self.invalidItemID = cjyx.vtkDMMLSubjectHierarchyNode.GetInvalidItemID()
        self.sampleMarkupName = 'SampleMarkup'
        self.studyItemID = self.invalidItemID
        self.cloneNodeNamePostfix = cjyx.qCjyxSubjectHierarchyCloneNodePlugin().getCloneNodeNamePostfix()

        # Test printing of all context menu actions and their section numbers
        pluginHandler = cjyx.qCjyxSubjectHierarchyPluginHandler().instance()
        print(pluginHandler.dumpContextMenuActions())

    # ------------------------------------------------------------------------------
    def section_MarkupRole(self):
        self.delayDisplay("Markup role", self.delayMs)

        shNode = cjyx.dmmlScene.GetSubjectHierarchyNode()
        self.assertIsNotNone(shNode)

        # Create sample markups node
        markupsNode = cjyx.vtkDMMLMarkupsFiducialNode()
        cjyx.dmmlScene.AddNode(markupsNode)
        markupsNode.SetName(self.sampleMarkupName)
        fiducialPosition = [100.0, 0.0, 0.0]
        markupsNode.AddControlPoint(fiducialPosition)
        markupsShItemID = shNode.GetItemByDataNode(markupsNode)
        self.assertIsNotNone(markupsShItemID)
        self.assertEqual(shNode.GetItemOwnerPluginName(markupsShItemID), 'Markups')

        # Create patient and study
        patientItemID = shNode.CreateSubjectItem(shNode.GetSceneItemID(), 'Patient')
        self.studyItemID = shNode.CreateStudyItem(patientItemID, 'Study')

        # Add markups under study
        markupsShItemID2 = shNode.CreateItem(self.studyItemID, markupsNode)
        self.assertEqual(markupsShItemID, markupsShItemID2)
        self.assertEqual(shNode.GetItemParent(markupsShItemID), self.studyItemID)
        self.assertEqual(shNode.GetItemOwnerPluginName(markupsShItemID), 'Markups')

    # ------------------------------------------------------------------------------
    def section_CloneNode(self):
        self.delayDisplay("Clone node", self.delayMs)

        shNode = cjyx.dmmlScene.GetSubjectHierarchyNode()
        self.assertIsNotNone(shNode)

        markupsNode = cjyx.util.getNode(self.sampleMarkupName)
        markupsShItemID = shNode.GetItemByDataNode(markupsNode)

        self.assertIsNotNone(markupsShItemID)
        self.assertIsNotNone(shNode.GetItemDataNode(markupsShItemID))

        # Add storage node for markups node to test cloning those
        markupsStorageNode = cjyx.vtkDMMLMarkupsFiducialStorageNode()
        cjyx.dmmlScene.AddNode(markupsStorageNode)
        markupsNode.SetAndObserveStorageNodeID(markupsStorageNode.GetID())

        # Get clone node plugin
        pluginHandler = cjyx.qCjyxSubjectHierarchyPluginHandler().instance()
        self.assertIsNotNone(pluginHandler)

        cloneNodePlugin = pluginHandler.pluginByName('CloneNode')
        self.assertIsNotNone(cloneNodePlugin)

        # Set markup node as current (i.e. selected in the tree) for clone
        pluginHandler.setCurrentItem(markupsShItemID)

        # Get clone node context menu action and trigger
        cloneNodePlugin.itemContextMenuActions()[0].activate(qt.QAction.Trigger)

        self.assertEqual(cjyx.dmmlScene.GetNumberOfNodesByClass('vtkDMMLMarkupsFiducialNode'), 2)
        self.assertEqual(cjyx.dmmlScene.GetNumberOfNodesByClass('vtkDMMLMarkupsDisplayNode'), 2)
        self.assertEqual(cjyx.dmmlScene.GetNumberOfNodesByClass('vtkDMMLMarkupsFiducialStorageNode'), 2)

        clonedMarkupsName = self.sampleMarkupName + self.cloneNodeNamePostfix
        clonedMarkupsNode = cjyx.util.getNode(clonedMarkupsName)
        self.assertIsNotNone(clonedMarkupsNode)
        clonedMarkupsShItemID = shNode.GetItemChildWithName(self.studyItemID, clonedMarkupsName)
        self.assertIsNotNone(clonedMarkupsShItemID)
        self.assertIsNotNone(clonedMarkupsNode.GetDisplayNode())
        self.assertIsNotNone(clonedMarkupsNode.GetStorageNode())

        inSameStudy = cjyx.vtkCjyxSubjectHierarchyModuleLogic.AreItemsInSameBranch(
            shNode, markupsShItemID, clonedMarkupsShItemID, cjyx.vtkDMMLSubjectHierarchyConstants.GetDICOMLevelStudy())
        self.assertTrue(inSameStudy)

    # ------------------------------------------------------------------------------
    def section_SegmentEditor(self):
        self.delayDisplay("Segment Editor", self.delayMs)

        shNode = cjyx.dmmlScene.GetSubjectHierarchyNode()
        self.assertIsNotNone(shNode)

        import SampleData
        mrHeadNode = SampleData.SampleDataLogic().downloadMRHead()

        # Make sure Data module is initialized because the use case tested below
        # (https://github.com/Slicer/Slicer/issues/4877) needs an initialized SH
        # tree view so that applyReferenceHighlightForItems is run
        cjyx.util.selectModule('Data')

        folderItem = shNode.CreateFolderItem(shNode.GetSceneItemID(), 'TestFolder')

        mrHeadItem = shNode.GetItemByDataNode(mrHeadNode)
        shNode.SetItemParent(mrHeadItem, folderItem)

        dataModuleWidget = cjyx.modules.data.widgetRepresentation()
        treeView = cjyx.util.findChildren(dataModuleWidget, className='qDMMLSubjectHierarchyTreeView')[0]
        treeView.setCurrentItem(mrHeadItem)

        pluginHandler = cjyx.qCjyxSubjectHierarchyPluginHandler.instance()
        segmentEditorPlugin = pluginHandler.pluginByName('SegmentEditor').self()
        segmentEditorPlugin.segmentEditorAction.trigger()

        # Get segmentation node automatically created by "Segment this..." action
        segmentationNode = None
        segmentationNodes = cjyx.dmmlScene.GetNodesByClass('vtkDMMLSegmentationNode')
        segmentationNodes.UnRegister(None)
        for i in range(segmentationNodes.GetNumberOfItems()):
            currentSegNode = segmentationNodes.GetItemAsObject(i)
            if currentSegNode.GetNodeReferenceID(currentSegNode.GetReferenceImageGeometryReferenceRole()) == mrHeadNode.GetID():
                segmentationNode = currentSegNode
                break

        self.assertIsNotNone(segmentationNode)

        segmentationItem = shNode.GetItemByDataNode(segmentationNode)
        self.assertEqual(shNode.GetItemParent(segmentationItem), shNode.GetItemParent(mrHeadItem))
        self.assertEqual(segmentationNode.GetName()[:len(mrHeadNode.GetName())], mrHeadNode.GetName())