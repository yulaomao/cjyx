import ctk

import cjyx
from cjyx.ScriptedLoadableModule import *


#
# CjyxMRBMultipleSaveRestoreLoopTest
#

class CjyxMRBMultipleSaveRestoreLoopTest(ScriptedLoadableModule):
    """Uses ScriptedLoadableModule base class, available at:
    https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        parent.title = "CjyxMRBMultipleSaveRestoreLoopTest"
        parent.categories = ["Testing.TestCases"]
        parent.contributors = ["Nicole Aucoin (BWH)"]
        parent.helpText = """
    Self test for MRB and Scene Views multiple save.
    No module interface here, only used in SelfTests module
    """
        parent.acknowledgementText = """
    This test was developed by
    Nicole Aucoin, BWH
    and was partially funded by NIH grant 3P41RR013218.
    """


#
# CjyxMRBMultipleSaveRestoreLoopTestWidget
#

class CjyxMRBMultipleSaveRestoreLoopTestWidget(ScriptedLoadableModuleWidget):
    """Uses ScriptedLoadableModuleWidget base class, available at:
    https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)


class CjyxMRBMultipleSaveRestoreLoop(ScriptedLoadableModuleTest):
    """
    This is the test case for your scripted module.
    Uses ScriptedLoadableModuleTest base class, available at:
    https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self, methodName='runTest', numberOfIterations=5, uniqueDirectory=True, strict=False):
        """
        Tests the use of dmml and mrb save formats with volumes and point lists.
        Checks that scene views are saved and restored as expected after multiple
        MRB saves and loads.

        numberOfIterations: integer number of times to save and restore an MRB.
        uniqueDirectory: boolean about save directory
                         False to reuse standard dir name
                         True timestamps dir name
        strict: boolean about how carefully to check result
                         True then check every detail
                         False then confirm basic operation, but allow non-critical issues to pass
        """
        ScriptedLoadableModuleTest.__init__(self, methodName)
        self.numberOfIterations = numberOfIterations
        self.uniqueDirectory = uniqueDirectory
        self.strict = strict

    def setUp(self):
        cjyx.dmmlScene.Clear(0)

    def runTest(self):
        self.setUp()
        self.test_CjyxMRBMultipleSaveRestoreLoop()

    def test_CjyxMRBMultipleSaveRestoreLoop(self):
        """
        Stress test the issue reported in bug 3956 where saving
        and restoring an MRB file does not work.
        """

        print("Running CjyxMRBMultipleSaveRestoreLoop Test case with:")
        print("numberOfIterations: %s" % self.numberOfIterations)
        print("uniqueDirectory : %s" % self.uniqueDirectory)
        print("strict : %s" % self.strict)

        #
        # first, get the data
        #
        print("Getting MR Head Volume")
        import SampleData
        mrHeadVolume = SampleData.downloadSample("MRHead")

        # Place a control point
        pointListNode = cjyx.dmmlScene.AddNewNodeByClass("vtkDMMLMarkupsFiducialNode", "F")
        pointListNode.CreateDefaultDisplayNodes()
        fid1 = [0.0, 0.0, 0.0]
        fidIndex1 = pointListNode.AddControlPoint(fid1)

        self.delayDisplay('Finished with download and placing points')

        ioManager = cjyx.app.ioManager()
        widget = cjyx.app.layoutManager().viewport()
        self.pointPosition = fid1
        for i in range(self.numberOfIterations):

            print('\n\nIteration %s' % i)
            #
            # save the dmml scene to an mrb
            #
            sceneSaveDirectory = cjyx.util.tempDirectory('__scene__')
            mrbFilePath = cjyx.util.tempDirectory('__mrb__') + '/CjyxMRBMultipleSaveRestoreLoop-' + str(i) + '.mrb'
            self.delayDisplay("Saving mrb to: %s" % mrbFilePath)
            screenShot = ctk.ctkWidgetsUtils.grabWidget(widget)
            self.assertTrue(
                ioManager.saveScene(mrbFilePath, screenShot)
            )
            self.delayDisplay("Finished saving MRB %s" % i)

            #
            # reload the mrb
            #
            cjyx.dmmlScene.Clear(0)
            self.delayDisplay('Now, reload the saved MRB')
            mrbLoaded = ioManager.loadScene(mrbFilePath)

            # load can return false even though it succeeded - only fail if in strict mode
            self.assertTrue(not self.strict or mrbLoaded)
            cjyx.app.processEvents()

            # confirm that MRHead is in the background of the Red slice
            redComposite = cjyx.util.getNode('vtkDMMLSliceCompositeNodeRed')
            mrHead = cjyx.util.getNode('MRHead')
            self.assertEqual(redComposite.GetBackgroundVolumeID(), mrHead.GetID())
            self.delayDisplay('The MRHead volume is AGAIN in the background of the Red viewer')

            # confirm that the point list exists with 1 points
            pointListNode = cjyx.util.getNode('F')
            self.assertEqual(pointListNode.GetNumberOfControlPoints(), 1)
            self.delayDisplay('The point list has 1 point in it')

            # adjust the fid list location
            self.pointPosition = [i, i, i]
            print((i, ': reset point position array to ', self.pointPosition))
            pointListNode.SetNthControlPointPosition(0, self.pointPosition)
        self.delayDisplay("Loop Finished")

        print(('Point position from loop = ', self.pointPosition))
        pointListNode = cjyx.util.getNode('F')
        finalPointPosition = [0, 0, 0]
        pointListNode.GetNthControlPointPosition(0, finalPointPosition)
        print(('Final point scene pos = ', finalPointPosition))
        self.assertEqual(self.pointPosition, finalPointPosition)

        self.delayDisplay("Test Finished")
