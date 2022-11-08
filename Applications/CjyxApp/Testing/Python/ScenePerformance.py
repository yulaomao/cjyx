import os

import qt

import cjyx
from cjyx.ScriptedLoadableModule import *
from cjyx.util import TESTING_DATA_URL


#
# ScenePerformance
#

class ScenePerformance(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        parent.title = "Scene Performance"
        parent.categories = ["Testing.TestCases"]
        parent.dependencies = []
        parent.contributors = ["Julien Finet (Kitware)"]
        parent.helpText = """
    This module was developed as a self test to perform the performance tests
    """
        parent.acknowledgementText = """
    This file was originally developed by Julien Finet, Kitware, Inc.  and was partially funded by NIH grant 3P41RR013218-12S1.
    """


#
# ScenePerformanceWidget
#
class ScenePerformanceWidget(ScriptedLoadableModuleWidget):

    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)

        moduleName = 'ScenePerformance'
        scriptedModulesPath = os.path.dirname(cjyx.util.modulePath(moduleName))
        path = os.path.join(scriptedModulesPath, 'Resources', 'UI', 'ScenePerformance.ui')
        widget = cjyx.util.loadUI(path)
        self.layout = self.parent.layout()
        self.layout.addWidget(widget)

        self.runTestsButton = qt.QPushButton("Run tests")
        self.runTestsButton.toolTip = "Run all the tests."
        self.runTestsButton.name = "Run tests"
        self.layout.addWidget(self.runTestsButton)
        self.runTestsButton.connect('clicked()', self.runTests)

        self.TimePushButton = self.findWidget(self.parent, 'TimePushButton')
        self.ActionComboBox = self.findWidget(self.parent, 'ActionComboBox')
        self.ActionPathLineEdit = self.findWidget(self.parent, 'ActionPathLineEdit')
        self.ResultsTextEdit = self.findWidget(self.parent, 'ResultsTextEdit')
        self.URLLineEdit = self.findWidget(self.parent, 'URLLineEdit')
        self.URLFileNameLineEdit = self.findWidget(self.parent, 'URLFileNameLineEdit')
        self.SceneViewSpinBox = self.findWidget(self.parent, 'SceneViewSpinBox')
        self.LayoutSpinBox = self.findWidget(self.parent, 'LayoutSpinBox')
        self.DMMLNodeComboBox = self.findWidget(self.parent, 'DMMLNodeComboBox')
        self.RepeatSpinBox = self.findWidget(self.parent, 'RepeatSpinBox')

        widget.setDMMLScene(cjyx.dmmlScene)
        # self.DMMLNodeComboBox.setDMMLScene(cjyx.dmmlScene)

        self.TimePushButton.connect('clicked()', self.timeAction)
        self.ActionComboBox.connect('currentIndexChanged(int)', self.updateActionProperties)
        self.updateActionProperties()

    def runTests(self):
        tester = ScenePerformanceTest()
        tester.testAll()

    def timeAction(self):
        tester = ScenePerformanceTest()
        tester.setUp()
        tester.setRepeat(self.RepeatSpinBox.value)
        if self.ActionComboBox.currentIndex == 0:  # Add Data
            if (self.URLLineEdit.text == ''):
                file = self.ActionPathLineEdit.currentPath
            else:
                logic = ScenePerformanceLogic()
                file = logic.downloadFile(self.URLLineEdit.text, self.URLFileNameLineEdit.text)
            results = tester.addData(file)
            self.ResultsTextEdit.append(results)
        elif self.ActionComboBox.currentIndex == 1:  # Restore
            results = tester.restoreSceneView(self.SceneViewSpinBox.value)
            self.ResultsTextEdit.append(results)
        elif self.ActionComboBox.currentIndex == 3:  # Layout
            results = tester.setLayout(self.LayoutSpinBox.value)
            self.ResultsTextEdit.append(results)
        elif self.ActionComboBox.currentIndex == 2:  # Close
            results = tester.closeScene()
            self.ResultsTextEdit.append(results)
        elif self.ActionComboBox.currentIndex == 4:  # Add Node
            node = self.DMMLNodeComboBox.currentNode()
            results = tester.addNode(node)
            self.ResultsTextEdit.append(results)
        elif self.ActionComboBox.currentIndex == 5:  # Modify Node
            node = self.DMMLNodeComboBox.currentNode()
            results = tester.modifyNode(node)
            self.ResultsTextEdit.append(results)

    def updateActionProperties(self):
        enableAddData = True if self.ActionComboBox.currentIndex == 0 else False
        self.ActionPathLineEdit.setEnabled(enableAddData)
        self.URLLineEdit.setEnabled(enableAddData)
        self.URLFileNameLineEdit.setEnabled(enableAddData)
        self.SceneViewSpinBox.setEnabled(True if self.ActionComboBox.currentIndex == 1 else False)
        self.LayoutSpinBox.setEnabled(True if self.ActionComboBox.currentIndex == 3 else False)
        self.DMMLNodeComboBox.setEnabled(True if self.ActionComboBox.currentIndex == 4 or self.ActionComboBox.currentIndex == 5 else False)

    def findWidget(self, widget, objectName):
        return cjyx.util.findChildren(widget, objectName)[0]


#
# ScenePerformanceLogic
#
class ScenePerformanceLogic(ScriptedLoadableModuleLogic):

    def downloadFile(self, downloadURL, downloadFileName, downloadFileChecksum=None):
        import SampleData
        return SampleData.downloadFromURL(
            fileNames=downloadFileName,
            uris=downloadURL,
            checksums=downloadFileChecksum)[0]

    def startTiming(self):
        self.Timer = qt.QTime()
        self.Timer.start()

    def stopTiming(self):
        return self.Timer.elapsed()


class ScenePerformanceTest(ScriptedLoadableModuleTest):

    def setUp(self):
        self.Repeat = 1
        self.delayDisplay("Setup")
        # layoutManager = cjyx.app.layoutManager()
        # layoutManager.setLayout(cjyx.vtkDMMLLayoutNode.CjyxLayoutConventionalView)
        # cjyx.dmmlScene.Clear(0)

    def setRepeat(self, repeat):
        self.Repeat = repeat

    def runTest(self):
        self.testAll()

    def testAll(self):
        self.setUp()

        self.addURLData(TESTING_DATA_URL + 'SHA256/688ebcc6f45989795be2bcdc6b8b5bfc461f1656d677ed3ddef8c313532687f1',
                        'BrainAtlas2012.mrb', 'SHA256:688ebcc6f45989795be2bcdc6b8b5bfc461f1656d677ed3ddef8c313532687f1')
        self.modifyNodeByID('vtkDMMLScalarVolumeNode1')
        self.modifyNodeByID('vtkDMMLScalarVolumeNode2')
        self.modifyNodeByID('vtkDMMLScalarVolumeNode3')
        self.modifyNodeByID('vtkDMMLScalarVolumeDisplayNode2')
        # self.modifyNodeByID('vtkDMMLModelHierarchyNode2')
        self.modifyNodeByID('vtkDMMLModelNode4')
        self.modifyNodeByID('vtkDMMLModelDisplayNode5')
        # self.modifyNodeByID('vtkDMMLModelHierarchyNode3')
        self.modifyNodeByID('vtkDMMLModelStorageNode1')
        self.addNodeByID('vtkDMMLModelNode302')
        self.setLayout(3)
        self.setLayout(2)
        self.setLayout(4)
        self.setLayout(5)
        self.setLayout(6)
        self.setLayout(7)
        self.setLayout(8)
        self.restoreSceneView(0)
        self.restoreSceneView(0)
        self.closeScene()

    def reportPerformance(self, action, property, time):
        message = self.displayPerformance(action, property, time)
        print(f'<DartMeasurement name="{action}-{property}" type="numeric/integer">{time}</DartMeasurement>')
        return message

    def displayPerformance(self, action, property, time):
        message = f'{action} ({property}) took {time} msecs '
        self.delayDisplay(message)
        return message

    def addURLData(self, url, file, checksum):
        logic = ScenePerformanceLogic()
        file = logic.downloadFile(url, file, checksum)
        self.addData(file)

    def addData(self, file):
        self.delayDisplay("Starting the AddData test")
        logic = ScenePerformanceLogic()
        averageTime = 0
        for x in range(self.Repeat):
            logic.startTiming()
            ioManager = cjyx.app.ioManager()
            ioManager.loadFile(file)
            time = logic.stopTiming()
            self.displayPerformance('AddData', file, time)
            averageTime = averageTime + time
        averageTime = averageTime / self.Repeat
        return self.reportPerformance('AddData', os.path.basename(file), averageTime)

    def closeScene(self):
        self.delayDisplay("Starting the Close Scene test")
        logic = ScenePerformanceLogic()
        averageTime = 0
        for x in range(self.Repeat):
            logic.startTiming()
            cjyx.dmmlScene.Clear(0)
            time = logic.stopTiming()
            self.displayPerformance('CloseScene', '', time)
            averageTime = averageTime + time
        averageTime = averageTime / self.Repeat
        return self.reportPerformance('CloseScene', '', averageTime)

    def restoreSceneView(self, sceneViewIndex):
        node = cjyx.dmmlScene.GetNthNodeByClass(sceneViewIndex, 'vtkDMMLSceneViewNode')
        return self.restoreSceneViewNode(node)

    def restoreSceneViewNode(self, node):
        self.delayDisplay("Starting the Restore Scene test")
        logic = ScenePerformanceLogic()
        averageTime = 0
        for x in range(self.Repeat):
            logic.startTiming()
            node.RestoreScene()
            time = logic.stopTiming()
            self.displayPerformance('RestoreSceneView', node.GetID(), time)
            averageTime = averageTime + time
        averageTime = averageTime / self.Repeat
        return self.reportPerformance('RestoreSceneView', node.GetID(), averageTime)

    def setLayout(self, layoutIndex):
        self.delayDisplay("Starting the layout test")
        logic = ScenePerformanceLogic()
        averageTime = 0
        for x in range(self.Repeat):
            logic.startTiming()
            layoutManager = cjyx.app.layoutManager()
            layoutManager.setLayout(layoutIndex)
            time = logic.stopTiming()
            self.displayPerformance('Layout', layoutIndex, time)
            averageTime = averageTime + time
        averageTime = averageTime / self.Repeat
        return self.reportPerformance('Layout', layoutIndex, averageTime)

    def addNodeByID(self, nodeID):
        node = cjyx.dmmlScene.GetNodeByID(nodeID)
        return self.addNode(node)

    def addNode(self, node):
        self.delayDisplay("Starting the add node test")
        logic = ScenePerformanceLogic()
        averageTime = 0
        for x in range(self.Repeat):
            newNode = node.CreateNodeInstance()
            newNode.UnRegister(node)
            newNode.Copy(node)
            logic.startTiming()
            cjyx.dmmlScene.AddNode(newNode)
            time = logic.stopTiming()
            self.displayPerformance('AddNode', node.GetID(), time)
            averageTime = averageTime + time
        averageTime = averageTime / self.Repeat
        return self.reportPerformance('AddNode', node.GetID(), averageTime)

    def modifyNodeByID(self, nodeID):
        node = cjyx.dmmlScene.GetNodeByID(nodeID)
        return self.modifyNode(node)

    def modifyNode(self, node):
        self.delayDisplay("Starting the modify node test")
        logic = ScenePerformanceLogic()
        averageTime = 0
        for x in range(self.Repeat):
            logic.startTiming()
            node.Modified()
            time = logic.stopTiming()
            self.displayPerformance('ModifyNode', node.GetID(), time)
            averageTime = averageTime + time
        averageTime = averageTime / self.Repeat
        return self.reportPerformance('ModifyNode', node.GetID(), averageTime)
