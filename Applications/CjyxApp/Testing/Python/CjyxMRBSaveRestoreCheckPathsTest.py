import os

import ctk

import cjyx
from cjyx.ScriptedLoadableModule import *


#
# CjyxMRBSaveRestoreCheckPathsTest
#
class CjyxMRBSaveRestoreCheckPathsTest(ScriptedLoadableModule):
    """Uses ScriptedLoadableModule base class, available at:
    https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        parent.title = "CjyxMRBSaveRestoreCheckPathsTest"
        parent.categories = ["Testing.TestCases"]
        parent.contributors = ["Nicole Aucoin (BWH)"]
        parent.helpText = """
    Self test for MRB multiple save file paths.
    No module interface here, only used in SelfTests module
    """
        parent.acknowledgementText = """
    This test was developed by
    Nicole Aucoin, BWH
    and was partially funded by NIH grant 3P41RR013218.
    """


class CjyxMRBSaveRestoreCheckPaths(ScriptedLoadableModuleTest):
    """
    This is the test case for your scripted module.
    Uses ScriptedLoadableModuleTest base class, available at:
    https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self, uniqueDirectory=True, strict=False):
        """
        Tests the use of dmml and mrb save formats with volumes.
        Checks that after reopening an MRB and trying to save it again that the file paths are all correct.

        uniqueDirectory: boolean about save directory
                         False to reuse standard dir name
                         True timestamps dir name
        strict: boolean about how carefully to check result
                         True then check every detail
                         False then confirm basic operation, but allow non-critical issues to pass
        """
        ScriptedLoadableModuleTest.__init__(self)
        self.uniqueDirectory = uniqueDirectory
        self.strict = strict
        # this flag will need to be updated if the code in
        #  qCjyxSceneBundleReader::load
        # is changed from the current behavior (delete expanded
        # files after loading the MRB into Cjyx)
        self.mrbDeleteFilesAfterLoad = 1

    def setUp(self):
        cjyx.dmmlScene.Clear(0)

    def runTest(self):
        self.setUp()
        self.test_CjyxMRBSaveRestoreCheckPaths()

    #
    # find and return the storable node associated with this storage node.
    # Returns None if not found.
    #
    def getStorableNode(self, storageNode):
        numberOfStorableNodes = storageNode.GetScene().GetNumberOfNodesByClass('vtkDMMLStorableNode')
        for n in range(numberOfStorableNodes):
            storableNode = storageNode.GetScene().GetNthNodeByClass(n, 'vtkDMMLStorableNode')
            if storableNode.GetStorageNodeID() == storageNode.GetID():
                return storableNode
        return None

    #
    # create and return a list of all file names found in storage nodes in
    # the scene, not including those in scene views
    #
    def checkSceneFileNames(self, scene):
        # first get the main scene storage nodes
        numberOfStorageNodes = scene.GetNumberOfNodesByClass('vtkDMMLStorageNode')
        for n in range(numberOfStorageNodes):
            storageNode = scene.GetNthNodeByClass(n, 'vtkDMMLStorageNode')
            storableNode = self.getStorableNode(storageNode)
            if storageNode.GetSaveWithScene() and not storableNode.GetModifiedSinceRead():
                print('Checking storage node: ', storageNode.GetID())
                fileName = storageNode.GetFileName()
                absFileName = storageNode.GetAbsoluteFilePath(fileName)
                if not absFileName:
                    print('\tUnable to get absolute path for file name ', fileName)
                    self.numberOfFilesNotFound += 1
                elif not os.path.exists(absFileName):
                    print('\tfile does not exist: ', absFileName)
                    print('\t\tnon absolute file name = ', fileName)
                    print('\t\tscene of the node root dir = ', storageNode.GetScene().GetRootDirectory())
                    if storableNode is not None:
                        print('\t\tstorable node name = ', storableNode.GetName())
                        print('\t\tmodified since read = ', storableNode.GetModifiedSinceRead())
                    else:
                        print('\t\tNo storable node found for this storage node')
                    # double check that it's not due to the unzipped files being deleted
                    if (not self.mrbDeleteFilesAfterLoad) or ('BundleSaveTemp' in absFileName):
                        self.numberOfFilesNotFound += 1
                    else:
                        print('\t\tMRB files were deleted after load, not counting this file as not found for the purposes of this test')
                else:
                    print('\tfile exists:', absFileName)
                # check for the file list
                numberOfFileNames = storageNode.GetNumberOfFileNames()
                for n in range(numberOfFileNames):
                    fileName = storageNode.GetNthFileName(n)
                    absFileName = storageNode.GetAbsoluteFilePath(fileName)
                    if not os.path.exists(absFileName):
                        print('\t', n, 'th file list member does not exist: ', absFileName)
                        if (not self.mrbDeleteFilesAfterLoad) or ('BundleSaveTemp' in absFileName):
                            self.numberOfFilesNotFound += 1
                        else:
                            print('\t\tMRB files were deleted after load, not counting this file as not found for the purposes of this test')

    def checkSceneViewFileNames(self, scene):
        # check for any scene views
        numberOfSceneViews = scene.GetNumberOfNodesByClass('vtkDMMLSceneViewNode')
        cjyx.util.delayDisplay("Number of scene views = " + str(numberOfSceneViews))
        if numberOfSceneViews == 0:
            return
        for n in range(numberOfSceneViews):
            sceneViewNode = cjyx.dmmlScene.GetNthNodeByClass(n, 'vtkDMMLSceneViewNode')
            cjyx.util.delayDisplay('\nChecking scene view ' + sceneViewNode.GetName() + ', id = ' + sceneViewNode.GetID())
            self.checkSceneFileNames(sceneViewNode.GetStoredScene())

    def checkAllFileNames(self, scene):
        cjyx.util.delayDisplay("\n\nChecking all file names in scene")
        self.numberOfFilesNotFound = 0
        self.checkSceneFileNames(scene)
        self.checkSceneViewFileNames(scene)
        if self.numberOfFilesNotFound != 0:
            print('checkAllFilesNames: there are ', self.numberOfFilesNotFound, 'files that are missing from disk\n')
        self.assertEqual(self.numberOfFilesNotFound, 0)

    def test_CjyxMRBSaveRestoreCheckPaths(self):
        """
        Replicate the issue reported in bug 3956 where saving
        and restoring an MRB file does not work.
        """

        print("Running CjyxMRBSaveRestoreCheckPaths Test case with:")
        print("uniqueDirectory : %s" % self.uniqueDirectory)
        print("strict : %s" % self.strict)
        print("files deleted after load: %d" % self.mrbDeleteFilesAfterLoad)

        #
        # first, get the volume data
        #
        print("Getting MR Head Volume")
        import SampleData
        mrHeadVolume = SampleData.downloadSample("MRHead")

        cjyx.util.delayDisplay('Finished with download of volume')

        #
        # test all current file paths
        #
        self.checkAllFileNames(cjyx.dmmlScene)

        ioManager = cjyx.app.ioManager()
        # grab a testing screen shot
        layoutManager = cjyx.app.layoutManager()
        widget = layoutManager.threeDWidget(0)
        screenShot = ctk.ctkWidgetsUtils.grabWidget(widget)

        #
        # the remote download leaves the volume in a temp directory and removes
        # the storage node, commit the scene to get to a more stable starting
        # point with the volume saved in a regular directory
        #
        tempDir = cjyx.util.tempDirectory('__dmml__')
        cjyx.util.delayDisplay('Temp dir = %s ' % tempDir)
        dmmlFilePath = tempDir + '/CjyxMRBSaveRestoreCheckPath.dmml'
        cjyx.dmmlScene.SetURL(dmmlFilePath)
        cjyx.util.delayDisplay(f'Saving dmml file to {dmmlFilePath}, current url of scene is {cjyx.dmmlScene.GetURL()}')
        # saveScene just writes out the .dmml file
        self.assertTrue(ioManager.saveScene(dmmlFilePath, screenShot))
        cjyx.util.delayDisplay(f'Finished saving dmml file {dmmlFilePath}, dmml url is now {cjyx.dmmlScene.GetURL()}\n\n\n')
        cjyx.util.delayDisplay('dmml root dir = %s' % cjyx.dmmlScene.GetRootDirectory())
        # explicitly save MRHead
        ioManager.addDefaultStorageNodes()
        mrHeadVolume.GetStorageNode().WriteData(mrHeadVolume)

        #
        # test all current file paths
        #
        self.checkAllFileNames(cjyx.dmmlScene)

        #
        # save the mrb
        #
        mrbFilePath = cjyx.util.tempDirectory('__mrb__') + '/CjyxMRBSaveRestoreCheckPaths-1.mrb'
        cjyx.util.delayDisplay("\n\n\nSaving mrb to: %s" % mrbFilePath)
        self.assertTrue(
            ioManager.saveScene(mrbFilePath, screenShot)
        )
        cjyx.util.delayDisplay("Finished saving mrb\n\n\n")

        #
        # test all current file paths
        #
        self.checkAllFileNames(cjyx.dmmlScene)

        #
        # reload the mrb and restore a scene view
        #
        cjyx.dmmlScene.Clear(0)
        cjyx.util.delayDisplay('Now, reload the first saved MRB\n\n\n')
        mrbLoaded = ioManager.loadScene(mrbFilePath)
        # load can return false even though it succeeded - only fail if in strict mode
        self.assertTrue(not self.strict or mrbLoaded)
        cjyx.app.processEvents()
        cjyx.util.delayDisplay("\n\n\nFinished reloading the first saved MRB\n\n\n")
        #
        # test all current file paths
        #
        self.checkAllFileNames(cjyx.dmmlScene)

        #
        # Save it again
        #
        mrbFilePath = cjyx.util.tempDirectory('__mrb__') + '/CjyxMRBSaveRestoreCheckPaths-2.mrb'
        cjyx.util.delayDisplay("Saving second mrb to: %s\n\n\n\n" % mrbFilePath)
        self.assertTrue(
            ioManager.saveScene(mrbFilePath, screenShot)
        )
        cjyx.util.delayDisplay("\n\n\nFinished second saving mrb %s" % mrbFilePath)

        #
        # test all current file paths
        #
        self.checkAllFileNames(cjyx.dmmlScene)

        cjyx.util.delayDisplay("Test Finished")
