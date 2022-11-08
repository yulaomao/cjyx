import ctk
import qt

import cjyx
from cjyx.ScriptedLoadableModule import *


#
# VolumeRenderingSceneClose
#

class VolumeRenderingSceneClose(ScriptedLoadableModule):
    """Uses ScriptedLoadableModule base class, available at:
    https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        parent.title = "VolumeRenderingSceneClose"
        parent.categories = ["Testing.TestCases"]
        parent.dependencies = []
        parent.contributors = ["Nicole Aucoin (BWH)"]
        parent.helpText = """
    This is a scripted self test to check that scene close
    works while in the volume rendering module.
    """
        parent.acknowledgementText = """
    This file was contributed by Nicole Aucoin, BWH, and was partially funded by NIH grant 3P41RR013218-12S1.
"""


#
# qVolumeRenderingSceneCloseWidget
#

class VolumeRenderingSceneCloseWidget(ScriptedLoadableModuleWidget):
    """Uses ScriptedLoadableModuleWidget base class, available at:
    https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)
        # Instantiate and connect widgets ...

        #
        # Parameters Area
        #
        parametersCollapsibleButton = ctk.ctkCollapsibleButton()
        parametersCollapsibleButton.text = "Parameters"
        self.layout.addWidget(parametersCollapsibleButton)

        # Layout within the dummy collapsible button
        parametersFormLayout = qt.QFormLayout(parametersCollapsibleButton)

        #
        # Apply Button
        #
        self.applyButton = qt.QPushButton("Apply")
        self.applyButton.toolTip = "Run the algorithm."
        self.applyButton.enabled = True
        parametersFormLayout.addRow(self.applyButton)

        # connections
        self.applyButton.connect('clicked(bool)', self.onApplyButton)

        # Add vertical spacer
        self.layout.addStretch(1)

    def cleanup(self):
        pass

    def onApplyButton(self):
        logic = VolumeRenderingSceneCloseLogic()
        print("Run the algorithm")
        logic.run()


#
# VolumeRenderingSceneCloseLogic
#

class VolumeRenderingSceneCloseLogic(ScriptedLoadableModuleLogic):
    """This class should implement all the actual
    computation done by your module.  The interface
    should be such that other python code can import
    this class and make use of the functionality without
    requiring an instance of the Widget.
    Uses ScriptedLoadableModuleLogic base class, available at:
    https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def run(self):
        """
        Run the actual algorithm
        """

        layoutManager = cjyx.app.layoutManager()
        layoutManager.setLayout(cjyx.vtkDMMLLayoutNode.CjyxLayoutConventionalView)

        cjyx.util.delayDisplay('Running the aglorithm')

        import SampleData
        ctVolume = SampleData.downloadSample('CTChest')
        cjyx.util.delayDisplay('Downloaded CT sample data')

        # go to the volume rendering module
        cjyx.util.mainWindow().moduleSelector().selectModule('VolumeRendering')
        cjyx.util.delayDisplay('Volume Rendering module')

        # turn it on
        volumeRenderingWidgetRep = cjyx.modules.volumerendering.widgetRepresentation()
        volumeRenderingWidgetRep.setDMMLVolumeNode(ctVolume)
        volumeRenderingNode = cjyx.dmmlScene.GetFirstNodeByName('VolumeRendering')
        volumeRenderingNode.SetVisibility(1)
        cjyx.util.delayDisplay('Volume Rendering')

        # set up a cropping ROI
        volumeRenderingNode.SetCroppingEnabled(1)
        markupsROI = cjyx.dmmlScene.GetFirstNodeByClass('vtkDMMLMarkupsROINode')
        cjyx.util.delayDisplay('Cropping')

        # close the scene
        cjyx.dmmlScene.Clear(0)

        return True


class VolumeRenderingSceneCloseTest(ScriptedLoadableModuleTest):
    """
    This is the test case for your scripted module.
    Uses ScriptedLoadableModuleTest base class, available at:
    https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def setUp(self):
        """ Do whatever is needed to reset the state - typically a scene clear will be enough.
        """
        cjyx.dmmlScene.Clear(0)

    def runTest(self):
        """Run as few or as many tests as needed here.
        """
        self.setUp()
        self.test_VolumeRenderingSceneClose1()

    def test_VolumeRenderingSceneClose1(self):

        self.delayDisplay("Starting the test")

        logic = VolumeRenderingSceneCloseLogic()
        logic.run()

        self.delayDisplay('Test passed!')
