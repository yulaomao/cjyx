import os

import qt
import vtk

import cjyx
from cjyx.ScriptedLoadableModule import *
from cjyx.util import TESTING_DATA_URL


#
# ShaderProperties
#

class ShaderProperties(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        parent.title = "Shader Properties"
        parent.categories = ["Testing.TestCases"]
        parent.dependencies = []
        parent.contributors = ["Simon Drouin (BWH)", "Steve Pieper (Isomics)"]
        parent.helpText = """
    This module was developed as a self test for shader properties
    """
        parent.acknowledgementText = """
    This file was originally developed and was partially funded by NIH grant 3P41RR013218-12S1.
    """


#
# ShaderPropertiesWidget
#
class ShaderPropertiesWidget(ScriptedLoadableModuleWidget):

    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)

        moduleName = 'ShaderProperties'

        self.sphereTestButton = qt.QPushButton()
        self.sphereTestButton.text = "Sphere test"
        self.layout.addWidget(self.sphereTestButton)
        self.sphereTestButton.connect("clicked()", lambda: ShaderPropertiesTest().testSphereCut())

        self.wedgeTestButton = qt.QPushButton()
        self.wedgeTestButton.text = "Wedge test"
        self.layout.addWidget(self.wedgeTestButton)
        self.wedgeTestButton.connect("clicked()", lambda: ShaderPropertiesTest().testWedgeCut())

        # Add vertical spacer
        self.layout.addStretch(1)

    def runTests(self):
        tester = ShaderPropertiesTest()
        tester.testAll()


#
# ShaderPropertiesTest
#

class ShaderPropertiesTest(ScriptedLoadableModuleTest):

    def setUp(self):
        self.delayDisplay("Setup")
        layoutManager = cjyx.app.layoutManager()
        layoutManager.setLayout(cjyx.vtkDMMLLayoutNode.CjyxLayoutConventionalView)
        cjyx.dmmlScene.Clear(0)
        self.delayDisplay("Setup complete")

    def runTest(self):
        self.testSphereCut()
        self.testWedgeCut()

    def testWedgeCut(self):

        self.delayDisplay("Starting...")
        self.setUp()

        fileURL = TESTING_DATA_URL + 'SHA256/19ad4f794de8dcdabbe3290c40fa18072cf5e05b6b2466fcc508ea7a42aae71e'
        filePath = os.path.join(cjyx.util.tempDirectory(), 'MRRobot-Shoulder-MR.nrrd')
        cjyx.util.downloadFile(fileURL, filePath)
        shoulder = cjyx.util.loadVolume(filePath)

        self.delayDisplay("Shoulder downloaded...")

        cjyx.util.mainWindow().moduleSelector().selectModule('VolumeRendering')
        volumeRenderingWidgetRep = cjyx.modules.volumerendering.widgetRepresentation()
        volumeRenderingWidgetRep.setDMMLVolumeNode(shoulder)

        volumeRenderingNode = cjyx.dmmlScene.GetFirstNodeByName('VolumeRendering')
        volumeRenderingNode.SetVisibility(1)

        self.delayDisplay('Volume rendering on')

        methodComboBox = cjyx.util.findChildren(name='RenderingMethodComboBox')[0]
        methodComboBox.currentIndex = methodComboBox.findText('VTK GPU Ray Casting')

        self.delayDisplay('GPU Ray Casting on')

        endpoints = [[-162.94, 2.32192, -30.1792], [-144.842, 96.867, -36.8726]]
        markupNode = cjyx.dmmlScene.AddNode(cjyx.vtkDMMLMarkupsLineNode())
        for endpoint in endpoints:
            markupNode.AddControlPoint(vtk.vtkVector3d(endpoint))

        self.delayDisplay('Line added')

        # ------------------------------------------------------
        # Utility functions to get the position of the first
        # markup line in the scene and the shader property
        # node
        # ------------------------------------------------------
        def GetLineEndpoints():
            fn = cjyx.util.getNode('vtkDMMLMarkupsLineNode1')
            endpoints = []
            for n in range(2):
                endpoints.append([0, ] * 3)
                fn.GetNthControlPointPosition(n, endpoints[n])
            return endpoints

        def GetShaderPropertyNode():
            return cjyx.util.getNode('vtkDMMLShaderPropertyNode1')

        # ------------------------------------------------------
        # Get the shader property node which contains every custom
        # shader modifications for every mapper associated with
        # the first volume rendering display node
        # ------------------------------------------------------
        displayNode = cjyx.util.getNodesByClass('vtkDMMLGPURayCastVolumeRenderingDisplayNode')[0]
        shaderPropNode = displayNode.GetOrCreateShaderPropertyNode(cjyx.dmmlScene)
        shaderProp = shaderPropNode.GetShaderProperty()

        # turn off shading so carved region looks reasonable
        volumePropertyNode = displayNode.GetVolumePropertyNode()
        volumeProperty = volumePropertyNode.GetVolumeProperty()
        volumeProperty.ShadeOff()

        # ------------------------------------------------------
        # Declare and initialize custom uniform variables
        # used in our shader replacement
        # ------------------------------------------------------
        shaderUniforms = shaderPropNode.GetFragmentUniforms()
        shaderUniforms.RemoveAllUniforms()
        endpoints = GetLineEndpoints()
        shaderUniforms.SetUniform3f("endpoint0", endpoints[0])
        shaderUniforms.SetUniform3f("endpoint1", endpoints[1])
        shaderUniforms.SetUniformf("coneCutoff", 0.8)

        # ------------------------------------------------------
        # Replace the cropping implementation part of the
        # raycasting shader to skip everything in the sphere
        # defined by endpoints and radius
        # ------------------------------------------------------
        croppingImplShaderCode = """
        vec4 texCoordRAS = in_volumeMatrix[0] * in_textureDatasetMatrix[0]  * vec4(g_dataPos, 1.);
        vec3 samplePoint = texCoordRAS.xyz;
        vec3 toSample = normalize(samplePoint - endpoint0);
        vec3 toEnd = normalize(endpoint1 - endpoint0);
        float onLine = dot(toEnd, toSample);
        g_skip = (onLine > coneCutoff);
    """
        shaderProp.ClearAllFragmentShaderReplacements()
        shaderProp.AddFragmentShaderReplacement("//VTK::Cropping::Impl", True, croppingImplShaderCode, False)

        # ------------------------------------------------------
        # Add a callback when the line moves to adjust
        # the endpoints of the carving sphere accordingly
        # ------------------------------------------------------
        def onControlPointMoved():
            endpoints = GetLineEndpoints()
            propNode = GetShaderPropertyNode()
            propNode.GetFragmentUniforms().SetUniform3f("endpoint0", endpoints[0])
            propNode.GetFragmentUniforms().SetUniform3f("endpoint1", endpoints[1])

        fn = cjyx.util.getNode('vtkDMMLMarkupsLineNode1')
        fn.AddObserver(fn.PointModifiedEvent, lambda caller, event: onControlPointMoved())

        self.delayDisplay("Should be a carved out shoulder now")

    def testSphereCut(self):

        self.delayDisplay("Starting...")
        self.setUp()

        import SampleData
        mrHead = SampleData.downloadSample('MRHead')

        self.delayDisplay("Head downloaded...")

        cjyx.util.mainWindow().moduleSelector().selectModule('VolumeRendering')
        volumeRenderingWidgetRep = cjyx.modules.volumerendering.widgetRepresentation()
        volumeRenderingWidgetRep.setDMMLVolumeNode(mrHead)

        volumeRenderingNode = cjyx.dmmlScene.GetFirstNodeByName('VolumeRendering')
        volumeRenderingNode.SetVisibility(1)

        self.delayDisplay('Volume rendering on')

        methodComboBox = cjyx.util.findChildren(name='RenderingMethodComboBox')[0]
        methodComboBox.currentIndex = methodComboBox.findText('VTK GPU Ray Casting')

        self.delayDisplay('GPU Ray Casting on')

        markupNode = cjyx.dmmlScene.AddNewNodeByClass("vtkDMMLMarkupsFiducialNode")
        markupNode.AddControlPoint([0.0, 100.0, 0.0])

        self.delayDisplay('Point list added')

        # ------------------------------------------------------
        # Utility functions to get the position of the first
        # markups point list in the scene and the shader property
        # node
        # ------------------------------------------------------
        def GetPointPosition():
            fn = cjyx.util.getNode('vtkDMMLMarkupsFiducialNode1')
            p = [0.0, 0.0, 0.0]
            fn.GetNthControlPointPosition(0, p)
            return p

        def GetShaderPropertyNode():
            return cjyx.util.getNode('vtkDMMLShaderPropertyNode1')

        # ------------------------------------------------------
        # Get the shader property node which contains every custom
        # shader modifications for every mapper associated with
        # the first volume rendering display node
        # ------------------------------------------------------
        displayNode = cjyx.util.getNodesByClass('vtkDMMLGPURayCastVolumeRenderingDisplayNode')[0]
        shaderPropNode = displayNode.GetOrCreateShaderPropertyNode(cjyx.dmmlScene)
        shaderProp = shaderPropNode.GetShaderProperty()

        # turn off shading so carved region looks reasonable
        volumePropertyNode = displayNode.GetVolumePropertyNode()
        volumeProperty = volumePropertyNode.GetVolumeProperty()
        volumeProperty.ShadeOff()

        # ------------------------------------------------------
        # Declare and initialize custom uniform variables
        # used in our shader replacement
        # ------------------------------------------------------
        shaderUniforms = shaderPropNode.GetFragmentUniforms()
        shaderUniforms.RemoveAllUniforms()
        pointPos = GetPointPosition()
        shaderUniforms.SetUniform3f("center", pointPos)
        shaderUniforms.SetUniformf("radius", 50.)

        # ------------------------------------------------------
        # Replace the cropping implementation part of the
        # raycasting shader to skip everything in the sphere
        # defined by center and radius
        # ------------------------------------------------------
        croppingImplShaderCode = """
        vec4 texCoordRAS = in_volumeMatrix[0] * in_textureDatasetMatrix[0]  * vec4(g_dataPos, 1.);
        g_skip = length(texCoordRAS.xyz - center) < radius;
    """
        shaderProp.ClearAllFragmentShaderReplacements()
        shaderProp.AddFragmentShaderReplacement("//VTK::Cropping::Impl", True, croppingImplShaderCode, False)

        # ------------------------------------------------------
        # Add a callback when the point moves to adjust
        # the center of the carving sphere accordingly
        # ------------------------------------------------------
        def onPointMoved():
            p = GetPointPosition()
            propNode = GetShaderPropertyNode()
            propNode.GetFragmentUniforms().SetUniform3f("center", p)

        fn = cjyx.util.getNode('vtkDMMLMarkupsFiducialNode1')
        fn.AddObserver(fn.PointModifiedEvent, lambda caller, event: onPointMoved())

        self.delayDisplay("Should be a carved out nose now")
