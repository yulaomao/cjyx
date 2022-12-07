import logging
import os

import vtk,qt

import cjyx
from cjyx.ScriptedLoadableModule import *
from cjyx.util import VTKObservationMixin
import plistlib


#
# NewRebuild
#

class NewRebuild(ScriptedLoadableModule):
    """Uses ScriptedLoadableModule base class, available at:
    https://github.com/Cjyx/Cjyx/blob/master/Base/Python/cjyx/ScriptedLoadableModule.py
    """

    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = "NewRebuild"  # TODO: make this more human readable by adding spaces
        self.parent.categories = ["Examples"]  # TODO: set categories (folders where the module shows up in the module selector)
        self.parent.dependencies = []  # TODO: add here list of module names that this module requires
        self.parent.contributors = ["John Doe (AnyWare Corp.)"]  # TODO: replace with "Firstname Lastname (Organization)"
        # TODO: update with short description of the module and a link to online module documentation
        self.parent.helpText = """
This is an example of scripted loadable module bundled in an extension.
See more information in <a href="https://github.com/organization/projectname#NewRebuild">module documentation</a>.
"""
        # TODO: replace with organization, grant and thanks
        self.parent.acknowledgementText = """
This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc., Andras Lasso, PerkLab,
and Steve Pieper, Isomics, Inc. and was partially funded by NIH grant 3P41RR013218-12S1.
"""

# NewRebuildWidget
#

class NewRebuildWidget(ScriptedLoadableModuleWidget, VTKObservationMixin):
    """Uses ScriptedLoadableModuleWidget base class, available at:
    https://github.com/Cjyx/Cjyx/blob/master/Base/Python/cjyx/ScriptedLoadableModule.py
    """

    def __init__(self, parent=None):
        """
        Called when the user opens the module the first time and the widget is initialized.
        """
        ScriptedLoadableModuleWidget.__init__(self, parent)
        VTKObservationMixin.__init__(self)  # needed for parameter node observation
        self.logic = None
        self._parameterNode = None
        self._updatingGUIFromParameterNode = False

    def setup(self):
        """
        Called when the user opens the module the first time and the widget is initialized.
        """
        ScriptedLoadableModuleWidget.setup(self)

        # Load widget from .ui file (created by Qt Designer).
        # Additional widgets can be instantiated manually and added to self.layout.
        uiWidget = cjyx.util.loadUI(self.resourcePath('UI/NewRebuild.ui'))
        self.layout.addWidget(uiWidget)
        self.ui = cjyx.util.childWidgetVariables(uiWidget)


        # Set scene in DMML widgets. Make sure that in Qt designer the top-level qDMMLWidget's
        # "dmmlSceneChanged(vtkDMMLScene*)" signal in is connected to each DMML widget's.
        # "setDMMLScene(vtkDMMLScene*)" slot.
        uiWidget.setDMMLScene(cjyx.dmmlScene)

        # Create logic class. Logic implements all computations that should be possible to run
        # in batch mode, without a graphical user interface.
        self.logic = NewRebuildLogic()
        self.addObserver(cjyx.dmmlScene, cjyx.dmmlScene.StartCloseEvent, self.onSceneStartClose_seg)

        self.uiInitial()
        self.addObserver(cjyx.dmmlScene,cjyx.dmmlScene.NodeAddedEvent, self.onNodeAdded)
        self.addObserver(cjyx.dmmlScene, cjyx.dmmlScene.EndCloseEvent, self.cleanup1)

    
    def onSceneStartClose_seg(self,unusedArg1=None, unusedArg2=None, unusedArg3=None):
        """
        Called when the application closes and the module widget is destroyed.
        """
        print("onSceneStartClose_seg")
        cjyx.modules.inf.widgetRepresentation().self().onSceneStartClose_inf()
        #self.removeObservers()

    def cleanup1(self,unusedArg1=None, unusedArg2=None, unusedArg3=None):
        """
        Called when the application closes and the module widget is destroyed.
        """
        self.masterVolumeNode=None

        self.UpdataSegmentTableWidget()
        self.ui.pushButton_manager.click()
        #self.removeObservers()

    def cleanup(self):
        print("clean up")
        self.removeObservers()


    def enter(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        """
        Called each time the user opens this module.
        """
        self.init()
        self.UpdataSegmentTableWidget()

    def exit(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        """
        Called each time the user opens this module.
        """
        self.ui.pushButton_manager.click()

    @vtk.calldata_type(vtk.VTK_OBJECT)
    def onNodeAdded(self, caller, event, calldata):
        if isinstance(calldata, cjyx.vtkDMMLScalarVolumeNode):
            qt.QTimer.singleShot(0, self.onSegmentationNodeAdded)
            
    def onSegmentationNodeAdded(self):
        self.init()
        self.UpdataSegmentTableWidget()

        #初始化ui
    def uiInitial(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        #图标
        self.ui.pushButton_manager.setIcon(self.getTwoIcon("图层管理"))
        self.ui.pushButton_painting.setIcon(self.getTwoIcon("图层绘制"))
        self.ui.pushbutton_undo.setIcon(self.getTwoIcon("撤销"))
        self.ui.pushbutton_redo.setIcon(self.getTwoIcon("前进"))
        self.ui.pushButton_undo2.setIcon(self.getTwoIcon("撤销"))
        self.ui.pushButton_redo2.setIcon(self.getTwoIcon("前进"))
        
        self.ui.pushButton_erase.setIcon(self.getTwoIcon("图层擦除"))
        self.ui.pushButton_3DTool.setIcon(self.getTwoIcon("3D工具包"))
        self.ui.pushbutton_add_segment.setIcon(self.getTwoIcon("添加图层"))
        self.ui.pushbutton_remove_segment.setIcon(self.getTwoIcon("删除"))
        self.ui.pushButton_dray.setIcon(self.getTwoIcon("勾勒"))
        self.ui.pushButton_paint.setIcon(self.getTwoIcon("画笔"))
        #self.ui.pushButton_fill.setIcon(self.getTwoIcon("填充"))
        self.ui.pushButton_lasso.setIcon(self.getTwoIcon("套索"))
        self.ui.pushButton_dray_wrap.setIcon(self.getTwoIcon("勾勒"))
        self.ui.pushButton_paint_wrap.setIcon(self.getTwoIcon("橡皮"))
        self.ui.pushButton_scissors.setIcon(self.getTwoIcon("剪刀"))
        self.ui.pushButton_Threshold.setIcon(self.getTwoIcon("HU"))
        self.ui.pushButton_Island.setIcon(self.getTwoIcon("岛屿工具"))
        self.ui.pushButton_shrink.setIcon(self.getTwoIcon("腐蚀"))
        self.ui.pushButton_expend.setIcon(self.getTwoIcon("膨胀"))
        self.ui.pushButton_3Dfill.setIcon(self.getTwoIcon("切片填充"))
        self.ui.pushButton_bool.setIcon(self.getTwoIcon("BOOL"))
        self.ui.pushButton_rebuild.setIcon(self.getTwoIcon("三维重建"))
        self.ui.pushButton_grow.setIcon(self.getTwoIcon("区域增长"))
        self.ui.radioButton_jiao1.setIcon(self.getTwoIcon("交集"))
        self.ui.radioButton_bing1.setIcon(self.getTwoIcon("并集"))
        self.ui.radioButton_bu1.setIcon(self.getTwoIcon("补集"))
        self.ui.pushButton_fenli_paint.setIcon(self.getTwoIcon("画笔"))
        self.ui.pushButton_fenli_erase.setIcon(self.getTwoIcon("橡皮"))

        #窗口
        self.CloseAllToolWidget()
        self.ui.widget_manager.setVisible(1)


        #信号和槽

        #顶部四按钮
        self.ui.pushButton_manager.clicked.connect(self.OnpushButton_manager)
        self.ui.pushButton_painting.clicked.connect(self.OnpushButton_painting)
        self.ui.pushButton_erase.clicked.connect(self.OnpushButton_erase)
        self.ui.pushButton_3DTool.clicked.connect(self.OnpushButton_3DTool)
        #图层管理
        self.ui.pushbutton_add_segment.clicked.connect(self.Onpushbutton_add_segment)
        self.ui.pushbutton_remove_segment.clicked.connect(self.Onpushbutton_remove_segment)
        self.ui.pushbutton_undo.clicked.connect(self.Onpushbutton_undo)
        self.ui.pushbutton_redo.clicked.connect(self.Onpushbutton_redo)
        self.ui.pushButton_undo2.clicked.connect(self.Onpushbutton_undo)
        self.ui.pushButton_redo2.clicked.connect(self.Onpushbutton_redo)
        self.ui.pushbutton_copy.clicked.connect(self.Onpushbutton_copy)


        #图层绘制
        self.ui.pushButton_dray.clicked.connect(self.OnpushButton_dray)
        self.ui.pushButton_paint.clicked.connect(self.OnpushButton_paint)
        self.ui.pushButton_pick.clicked.connect(self.OnpushButton_pick)
        self.ui.pushButton_lasso.clicked.connect(self.OnpushButton_lasso)
        #图层擦除
        self.ui.pushButton_dray_wrap.clicked.connect(self.OnpushButton_dray_wrap)
        self.ui.pushButton_paint_wrap.clicked.connect(self.OnpushButton_paint_wrap)
        self.ui.pushButton_scissors.clicked.connect(self.OnpushButton_scissors)
        #3D工具
        self.ui.pushButton_Threshold.clicked.connect(self.OnpushButton_Threshold)
        self.ui.pushButton_Island.clicked.connect(self.OnpushButton_Island)
        self.ui.pushButton_shrink.clicked.connect(self.OnpushButton_shrink)
        self.ui.pushButton_expend.clicked.connect(self.OnpushButton_expend)
        self.ui.pushButton_3Dfill.clicked.connect(self.OnpushButton_3Dfill)
        self.ui.pushButton_bool.clicked.connect(self.OnpushButton_bool)
        self.ui.pushButton_rebuild.clicked.connect(self.OnpushButton_rebuild)
        self.ui.pushButton_grow.clicked.connect(self.OnpushButton_grow)
        self.ui.pushButton_seed.clicked.connect(self.OnpushButton_seed)


        # ---------------------画笔工具----------------------------------------------------
        self.ui.SliderWidget_paint_R.connect('valueChanged(double)', self.onDiameterValueChanged)
        self.ui.paint_circle.connect('clicked(bool)', self.Onpaint_circle)
        self.ui.paint_sphere.connect('clicked(bool)', self.Onpaint_sphere)
        self.ui.paint_editor_in_3D.connect('clicked(bool)', self.Onpaint_editor_in_3D)


        # ---------------------阈值分割----------------------------------------------------
        self.ui.threshold_ensure.connect('clicked(bool)', self.onThresholdButton)
        self.ui.RangeWidget.connect('valuesChanged(double,double)', self.onThresholdValuesChanged)
        self.ui.threshold_cancel.connect('clicked(bool)', self.onthresholdCancel)
        self.ui.threshold_skin.connect('clicked(bool)', self.onThreshold_skin)
        self.ui.threshold_bone.connect('clicked(bool)', self.onTthreshold_bone)
        self.ui.threshold_lung.connect('clicked(bool)', self.onThreshold_lung)

        # # -----------------------区域增长（岛屿工具）-----------------------------------------
        self.ui.pushButton_island_confirm.connect('clicked(bool)', self.onIslandTool)  # 岛屿工具
        self.ui.radioButton_island_keep.connect('clicked(bool)', self.onKeepSelectIsland)
        self.ui.pushButton_island_cancel.connect('clicked(bool)', self.onIslandCancel)
        self.ui.radioButton_island_max.connect('clicked(bool)', self.onIslandEnsureEnable)
        self.ui.radioButton_island_remove_s.connect('clicked(bool)', self.onIslandEnsureEnable)

        # # -----------------------腐蚀膨胀-----------------------------------------
        self.ui.pushButton_3DTool_confirm.connect('clicked(bool)', self.on3DToolConfirm)
        self.ui.pushButton_3DTool_cancel.connect('clicked(bool)', self.on3DToolCancel)

        # # -----------------------切片填充-----------------------------------------
        self.ui.Fill3D_Ensure.connect('clicked(bool)', self.onFillBetweenSlices)
        self.ui.Fill3D_cancel.connect('clicked(bool)', self.onFill3DCancel)

        # # -----------------------bool-----------------------------------------
        self.ui.pushButton_boolConfirm.connect('clicked(bool)', self.onBoolConfirm)
        self.ui.pushButton_boolCancel.connect('clicked(bool)', self.onBoolCancel)

        # # -----------------------重建-----------------------------------------
        self.ui.pushButton_CreatSurface.connect('clicked(bool)', self.onCreatSurface)
        self.ui.pushButton_rebuild2.connect('clicked(bool)', self.onCreatSurface)
        self.ui.widget_6.setVisible(0)
        #self.ui.pushButton_boolCancel.connect('clicked(bool)', self.onBoolCancel)

        # # -----------------------区域生长-----------------------------------------
        self.ui.pushButton_add_target.connect('clicked(bool)', self.onAddTarget)
        self.ui.pushButton_add_background.connect('clicked(bool)', self.onAddBackground)
        self.ui.pushButton_cancell_fenli.connect('clicked(bool)', self.onpushButton_cancell_fenli)
        self.ui.pushButton_preview_fenli.connect('clicked(bool)', self.onpushButton_preview_fenli)
        self.ui.pushButton_ensure_fenli.connect('clicked(bool)', self.onpushButton_ensure_fenli)
        self.ui.pushButton_fenli_paint.connect('clicked(bool)', self.onpushButton_fenli_paint)
        self.ui.pushButton_fenli_erase.connect('clicked(bool)', self.onpushButton_fenli_erase)
        self.ui.pushButton_fenli_pick.connect('clicked(bool)', self.onpushButton_fenli_pick)
        # # -----------------------可编辑区域-----------------------------------------
        self.ui.pushButton_ROI.connect('clicked(bool)', self.onFitToVolume)
        self.ui.CloseButton.connect("clicked(bool)",self.onCloseWidget)

        # # -----------------------图层分离-----------------------------------------






        iconsPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons')
        closeIconPath = os.path.join(iconsPath,"close.png")
        self.ui.CloseButton.setIcon(qt.QIcon(closeIconPath))




        #初始化分割
        self.segmentEditorWidget = cjyx.qDMMLSegmentEditorWidget()
        self.segmentEditorWidget.setDMMLScene(cjyx.dmmlScene)
        self.segmentEditorWidget.setAutoShowMasterVolumeNode(False)  # 设置不显示分割内容
        self.masterVolumeNode=None

        #table
        self.ui.SegmenttableWidget.horizontalHeader().setSectionResizeMode(qt.QHeaderView.ResizeToContents)
        self.ui.SegmenttableWidget.horizontalHeader().setSectionResizeMode(qt.QHeaderView.Stretch)
        self.ui.SegmenttableWidget.horizontalHeader().setSectionResizeMode(0, qt.QHeaderView.ResizeToContents)
        self.ui.SegmenttableWidget.horizontalHeader().setSectionResizeMode(2, qt.QHeaderView.ResizeToContents)
        self.ui.SegmenttableWidget.horizontalHeader().setSectionResizeMode(3, qt.QHeaderView.ResizeToContents)
        self.ui.SegmenttableWidget.verticalHeader().hide()
        self.ui.SegmenttableWidget.itemDoubleClicked.connect(self.onRename)
        self.ui.SegmenttableWidget.cellChanged.connect(self.onChanged)
        self.ui.SegmenttableWidget.itemClicked.connect(self.onSelectItem)
        self.ui.SegmenttableWidget.horizontalHeader().setHighlightSections(False)
        self.prename=None
        self.rename=None
        self.line=None
        self.popwidget=None
        self.thresholdPreSetValue=[225,3071,-500,0,0,255]

    # 初始化分割所需条件
    def init(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        print("初始化")
        if (self.masterVolumeNode==None):
            if len(cjyx.util.getNodesByClass('vtkDMMLScalarVolumeNode')) > 0:
                #cjyx.modules.segmenteditor.widgetRepresentation()
                segmentationNodes = cjyx.util.getNodesByClass("vtkDMMLSegmentationNode")
                # 判断手动分割的segmentation是否存在
                if len(segmentationNodes) > 0:
                    for i in range(0, len(segmentationNodes)):
                        a = segmentationNodes[i]
                        if 'Manual' in a.GetName():
                            self.segmentationNode = a
                        else:
                            self.segmentationNode = cjyx.dmmlScene.AddNewNodeByClass("vtkDMMLSegmentationNode", 'Manual')
                else:
                    self.segmentationNode = cjyx.dmmlScene.AddNewNodeByClass("vtkDMMLSegmentationNode", 'Manual')

                self.segmentationNode.CreateDefaultDisplayNodes()  # only needed for display

                # 判断segmenteditor节点是否存在
                if len(cjyx.util.getNodesByClass("vtkDMMLSegmentEditorNode")) > 0:
                    self.segmentEditorNode = cjyx.util.getNodesByClass("vtkDMMLSegmentEditorNode")[0]
                else:
                    self.segmentEditorNode = cjyx.dmmlScene.AddNewNodeByClass('vtkDMMLSegmentEditorNode')
                # 初始化segment 列表
                self.segmentEditorNode.SetOverwriteMode(cjyx.vtkDMMLSegmentEditorNode.OverwriteNone)
                if len(cjyx.util.getNodesByClass('vtkDMMLScalarVolumeNode')) > 0:
                    self.masterVolumeNode = cjyx.util.getNodesByClass('vtkDMMLScalarVolumeNode')[0]
                    self.segmentationNode.SetReferenceImageGeometryParameterFromVolumeNode(self.masterVolumeNode)
                    self.segmentEditorWidget.setDMMLSegmentEditorNode(self.segmentEditorNode)
                    self.segmentEditorWidget.setSegmentationNode(self.segmentationNode)
                    self.segmentEditorWidget.setMasterVolumeNode(self.masterVolumeNode)
                    #self.segmentationNodeChangedObserve=self.segmentationNode.AddObserver(vtk.vtkCommand.ModifiedEvent, self.onTableWidget)
                    segmentation = self.segmentationNode.GetSegmentation()
                    segmentation.SetConversionParameter("Smoothing factor", "0.0")
                    print("初始化完成")



    # 顶部四按钮
    def OnpushButton_manager(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        self.CloseAllToolWidget()
        self.ui.widget_manager.setVisible(1)

    def OnpushButton_painting(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        self.CloseAllToolWidget()
        self.ui.widget_painting.setVisible(1)


    def OnpushButton_erase(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        self.CloseAllToolWidget()
        self.ui.widget_wrap.setVisible(1)


    def OnpushButton_3DTool(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        self.CloseAllToolWidget()
        self.ui.widget_3DTool.setVisible(1)


    #图层管理
    def Onpushbutton_add_segment(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        self.OnCloseAllFunWidget()
        a = self.segmentationNode.GetSegmentation().AddEmptySegment()

        segment = self.segmentationNode.GetSegmentation().GetSegment(a)
        try:
            cjyx.modules.datamanager.widgetRepresentation().self().onSegmentNodeAdded(segment)
        except:
            pass
        self.onTableWidget(segment.GetName())
        self.selectCurrentItem(segment.GetName())


    def Onpushbutton_remove_segment(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        self.OnCloseAllFunWidget()
        row = self.ui.SegmenttableWidget.currentRow()
        try:
            text = self.ui.SegmenttableWidget.item(row, 1).text()
        except:
            for i in range(self.ui.SegmenttableWidget.rowCount):
                item = self.ui.SegmenttableWidget.item(i, 1)
                item.setSelected(False)
                return
        segmentID = self.segmentationNode.GetSegmentation().GetSegmentIdBySegmentName(text)
        segment = self.segmentationNode.GetSegmentation().GetSegment(segmentID)
        try:
            cjyx.modules.datamanager.widgetRepresentation().self().onSegmentNodeRemoved(segment)
        except:
            pass
        self.segmentationNode.GetSegmentation().RemoveSegment(segmentID)
        self.ui.SegmenttableWidget.removeRow(row)
        try:
            item = self.ui.SegmenttableWidget.item(self.ui.SegmenttableWidget.rowCount-1, 1)
            self.selectCurrentItem(item.text())
        except:
            print("选择最后节点出错")

    def Onpushbutton_undo(self):
        #记录undo前分段，以识别撤销新添的分段，并将其设置为3D隐藏
        segmentation = self.segmentationNode.GetSegmentation()
        if segmentation.GetNumberOfSegments() > 0:
            number = segmentation.GetNumberOfSegments()
            allSegmentsBefor = []
            for i in range(number):
                segment = segmentation.GetNthSegment(i)
                allSegmentsBefor.append(segment)
        self.segmentEditorWidget.undo()
        #处理'Cylinder'
        self.dealUndo()
        #恢复状态
        number = segmentation.GetNumberOfSegments()
        for i in range(number):
            segment = segmentation.GetNthSegment(i)
            if(segment not in allSegmentsBefor):
                segmentID=segmentation.GetSegmentIdBySegment(segment)
                self.segmentationNode.GetDisplayNode().SetSegmentVisibility3D(segmentID, 0)
        self.UpdataSegmentTableWidget()
        cjyx.modules.datamanager.widgetRepresentation().self().UpDateSegmentAfterUndo()

    def dealUndo(self):
        haveCylinder = 0
        segmentation = self.segmentationNode.GetSegmentation()
        if segmentation.GetNumberOfSegments() > 0:
            number = segmentation.GetNumberOfSegments()
            for i in range(0, number):
                segment = segmentation.GetNthSegment(i)
                if segment.GetName() == 'Cylinder':
                    haveCylinder = 1
                    break
        if (haveCylinder):
            if (self.ui.pushButton_ROI.checked):
                self.UpdateROIBySegment()
            else:
                self.ui.pushButton_ROI.setChecked(1)
                self.UpdateROIBySegment()
        else:
            if (self.ui.pushButton_ROI.checked):
                self.ui.pushButton_ROI.setChecked(0)
                self.segmentEditorNode.SetMaskMode(cjyx.vtkDMMLSegmentationNode.EditAllowedEverywhere)
                self.paint_roi.RemoveObserver(self.r_observe)
                cjyx.dmmlScene.RemoveNode(self.paint_roi)
            else:
                pass

    def Onpushbutton_redo(self):
        segmentation = self.segmentationNode.GetSegmentation()
        if segmentation.GetNumberOfSegments() > 0:
            number = segmentation.GetNumberOfSegments()
            allSegmentsBefor = []
            for i in range(number):
                segment = segmentation.GetNthSegment(i)
                allSegmentsBefor.append(segment)
        self.segmentEditorWidget.redo()
        self.dealUndo()
        number = segmentation.GetNumberOfSegments()
        for i in range(number):
            segment = segmentation.GetNthSegment(i)
            if(segment not in allSegmentsBefor):
                segmentID=segmentation.GetSegmentIdBySegment(segment)
                self.segmentationNode.GetDisplayNode().SetSegmentVisibility3D(segmentID, 0)
        self.UpdataSegmentTableWidget()
        cjyx.modules.datamanager.widgetRepresentation().self().UpDateSegmentAfterUndo()

    def UpdateROIBySegment(self):
        RoiNodes = cjyx.util.getNodesByClass('vtkDMMLMarkupsROINode')
        if (self.paint_roi in RoiNodes):
            self.paint_roi.RemoveObserver(self.r_observe)
            segmentID = self.segmentationNode.GetSegmentation().GetSegmentIdBySegmentName("Cylinder")
            segment = self.segmentationNode.GetSegmentation().GetSegment(segmentID)
            displayNode = self.segmentationNode.GetDisplayNode()
            displayNode.SetSegmentVisibility(segmentID, 0)
            b = [0, 0, 0, 0, 0, 0]
            segment.GetBounds(b)
            center = [(b[0] + b[1]) / 2, (b[2] + b[3]) / 2, (b[4] + b[5]) / 2]
            xyz = [(b[1] - b[0]) / 2, (b[3] - b[2]) / 2, (b[5] - b[4]) / 2]
            self.paint_roi.SetRadiusXYZ(xyz)
            self.paint_roi.SetXYZ(center)
            self.r_observe = self.paint_roi.AddObserver(vtk.vtkCommand.ModifiedEvent, self.UpdataFitToVolume)

        else:
            segmentID = self.segmentationNode.GetSegmentation().GetSegmentIdBySegmentName("Cylinder")
            segment = self.segmentationNode.GetSegmentation().GetSegment(segmentID)
            displayNode = self.segmentationNode.GetDisplayNode()
            displayNode.SetSegmentVisibility(segmentID, 0)
            b = [0, 0, 0, 0, 0, 0]
            segment.GetBounds(b)
            center = [(b[0] + b[1]) / 2, (b[2] + b[3]) / 2, (b[4] + b[5]) / 2]
            xyz = [(b[1] - b[0]) / 2, (b[3] - b[2]) / 2, (b[5] - b[4]) / 2]
            self.paint_roi = None
            self.paint_roi = cjyx.dmmlScene.AddNewNodeByClass('vtkDMMLMarkupsROINode', '可编辑区域')
            self.paint_roi.SetRadiusXYZ(xyz)
            self.paint_roi.SetXYZ(center)
            self.paint_roi.GetDisplayNode().SetScaleHandleVisibility(True)
            self.paint_roi.GetDisplayNode().SetInteractionHandleScale(1)
            self.paint_roi.GetDisplayNode().SetFillOpacity(0)
            self.paint_roi.GetDisplayNode().SetTextScale(0)
            self.paint_roi.GetDisplayNode().SetSelectedColor(0.48627450980392156, 0.7411764705882353,
                                                             0.15294117647058825)
            self.paint_roi.GetDisplayNode().SetActiveColor(0.615686274509804, 0.9098039215686274, 1.0)
            self.paint_roi.GetDisplayNode().SetInteractionHandleScale(1)
            # 创建vtk模型
            self.cube = None
            self.cube = vtk.vtkCubeSource()
            self.cube.SetCenter(center)
            self.cube.SetXLength(2 * xyz[0])
            self.cube.SetYLength(2 * xyz[1])
            self.cube.SetZLength(2 * xyz[2])
            self.cube.Update()
            self.segmentEditorNode.SetMaskMode(cjyx.vtkDMMLSegmentationNode.EditAllowedInsideSingleSegment)
            self.segmentEditorNode.SetMaskSegmentID(segmentID)
            self.r_observe = self.paint_roi.AddObserver(vtk.vtkCommand.ModifiedEvent, self.UpdataFitToVolume)

    def Onpushbutton_copy(self):
        a = self.segmentationNode.GetSegmentation().AddEmptySegment()
        segName=self.segmentationNode.GetSegmentation().GetSegment(a).GetName()
        self.segmentationNode.GetDisplayNode().SetSegmentVisibility3D(a, 0)
        self.onBoolTool(segName, self.name1, 'COPY')
        # 更新图层列表
        segment = self.segmentationNode.GetSegmentation().GetSegment(a)
        segment.SetName(self.GetUniqueNameByStringInSeg(self.name1+"-复制"))
        try:
            cjyx.modules.datamanager.widgetRepresentation().self().onSegmentNodeAdded(segment)
        except:
            pass
        self.UpdataSegmentTableWidget()


    #图层绘制
    def OnpushButton_dray(self,pushbutton=None):

        self.OnCloseAllFunWidget()
        self.PushbuttonChecked(self.ui.pushButton_dray)
        if self.ui.pushButton_dray.checked:
            self.segmentEditorWidget.setActiveEffectByName('Draw')
        else:
            self.segmentEditorWidget.setActiveEffectByName('None')


    def OnpushButton_paint(self,pushbutton=None):
        self.OnCloseAllFunWidget()
        self.PushbuttonChecked(self.ui.pushButton_paint)
        if self.ui.pushButton_paint.checked:
            self.ui.groupBox_paint.setVisible(1)
            self.segmentEditorWidget.setActiveEffectByName('Paint')
            effect = self.segmentEditorWidget.activeEffect()
            effect.setCommonParameter("BrushSphere", 0)
            self.ui.paint_circle.click()
            self.ui.paint_editor_in_3D.setChecked(0)
        else:
            self.segmentEditorWidget.setActiveEffectByName('None')

    def Onpaint_circle(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        effect = self.segmentEditorWidget.activeEffect()
        effect.setCommonParameter("BrushSphere",0)

    def Onpaint_sphere(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        effect = self.segmentEditorWidget.activeEffect()
        effect.setCommonParameter("BrushSphere",1)

    def Onpaint_editor_in_3D(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        effect = self.segmentEditorWidget.activeEffect()
        effect.setCommonParameter("EditIn3DViews", int(self.ui.paint_editor_in_3D.isChecked()))

    # 画笔大小函数
    def onDiameterValueChanged(self, value):
        effect = self.segmentEditorWidget.activeEffect()
        effect.setCommonParameter("BrushRelativeDiameter", value)



    def OnpushButton_fill(self,pushbutton=None):
        self.OnCloseAllFunWidget()
        self.PushbuttonChecked(self.ui.pushButton_fill)

    def OnpushButton_lasso(self,pushbutton=None):
        self.OnCloseAllFunWidget()
        self.PushbuttonChecked(self.ui.pushButton_lasso)
        if self.ui.pushButton_lasso.checked:
            self.segmentEditorWidget.setActiveEffectByName('Level tracing')
        else:
            self.segmentEditorWidget.setActiveEffectByName('None')

    def OnpushButton_pick(self,pushbutton=None):
        self.OnCloseAllFunWidget()
        self.PushbuttonChecked(self.ui.pushButton_pick)
        if self.ui.pushButton_pick.checked:
            self.segmentEditorWidget.setActiveEffectByName('Pick')
        else:
            self.segmentEditorWidget.setActiveEffectByName('None')


    #图层擦除
    def OnpushButton_dray_wrap(self,pushbutton=None):
        self.OnCloseAllFunWidget()
        self.PushbuttonChecked(self.ui.pushButton_dray_wrap)
        if self.ui.pushButton_dray_wrap.checked:
            self.segmentEditorWidget.setActiveEffectByName('DrawErase')
        else:
            self.segmentEditorWidget.setActiveEffectByName('None')

    def OnpushButton_paint_wrap(self,pushbutton=None):
        self.OnCloseAllFunWidget()
        self.PushbuttonChecked(self.ui.pushButton_paint_wrap)

        if self.ui.pushButton_paint_wrap.checked:
            self.ui.groupBox_paint.setVisible(1)
            self.segmentEditorWidget.setActiveEffectByName('Erase')
            effect = self.segmentEditorWidget.activeEffect()
            effect.setCommonParameter("BrushSphere", 0)
        else:
            self.segmentEditorWidget.setActiveEffectByName('None')
            self.ui.groupBox_paint.setVisible(0)



    def OnpushButton_scissors(self,pushbutton=None):
        self.OnCloseAllFunWidget()
        self.PushbuttonChecked(self.ui.pushButton_scissors)
        if self.ui.pushButton_scissors.checked:
            self.segmentEditorWidget.setActiveEffectByName('Scissors')
            effect = self.segmentEditorWidget.activeEffect()
            effect.setParameter('Operation', 'EraseInside')
        else:
            self.segmentEditorWidget.setActiveEffectByName('None')


    #3D工具
    # ---------------阈值分割-----------------------------------------------------
    def OnpushButton_Threshold(self,pushbutton=None):
        self.OnCloseAllFunWidget()
        self.PushbuttonChecked(self.ui.pushButton_Threshold)
        if self.ui.pushButton_Threshold.checked:
            self.ui.groupbox_threshold.setVisible(1)
            self.segmentEditorWidget.setActiveEffectByName('Threshold')
            effect = self.segmentEditorWidget.activeEffect()
            masterImageData = cjyx.SegmentEditorEffects.scriptedEffect.masterVolumeImageData()
            masterVolumeMin, masterVolumeMax = masterImageData.GetScalarRange()
            self.ui.RangeWidget.setRange(masterVolumeMin,masterVolumeMax)
            self.ui.RangeWidget.setMinimumValue(125.0)
            self.ui.RangeWidget.setMaximumValue(masterVolumeMax)
            self.UpdateThresholdPresetValueFromSetting()
            #self.thresholdPreSetValue[1]=masterVolumeMax

            #self.ui.SegmentEditorTip.setText('阈值分割工具可以通过设置阈值范围对图层进行填充，可以实时通过滑块或点击视图进行调节，点击应用完成操作。')
        else:
            self.segmentEditorWidget.setActiveEffectByName('None')

    def GetThresholdPresetValue(self):
        return self.thresholdPreSetValue

    def UpdateThresholdPresetValueFromSetting(self):
        self.thresholdPreSetValue[0]=cjyx.modules.setting.widgetRepresentation().self().gugemin
        self.thresholdPreSetValue[1]=cjyx.modules.setting.widgetRepresentation().self().gugemax
        self.thresholdPreSetValue[2]=cjyx.modules.setting.widgetRepresentation().self().jiroumin
        self.thresholdPreSetValue[3]=cjyx.modules.setting.widgetRepresentation().self().jiroumax
        self.thresholdPreSetValue[4]=cjyx.modules.setting.widgetRepresentation().self().pifumin
        self.thresholdPreSetValue[5]=cjyx.modules.setting.widgetRepresentation().self().pifumax

    def SetThresholdPresetValue(self,Value):
        self.thresholdPreSetValue=Value

    def onThreshold_skin(self):
        self.ui.RangeWidget.setMinimumValue(self.thresholdPreSetValue[2])
        self.ui.RangeWidget.setMaximumValue(self.thresholdPreSetValue[3])

    def onTthreshold_bone(self):
        self.ui.RangeWidget.setMinimumValue(self.thresholdPreSetValue[0])
        self.ui.RangeWidget.setMaximumValue(self.thresholdPreSetValue[1])
    def onThreshold_lung(self):
        self.ui.RangeWidget.setMinimumValue(self.thresholdPreSetValue[4])
        self.ui.RangeWidget.setMaximumValue(self.thresholdPreSetValue[5])
    def onThresholdValuesChanged(self, min, max):
        cjyx.SegmentEditorEffects.thresholdSlider.minimumValue = min
        cjyx.SegmentEditorEffects.thresholdSlider.maximumValue = max
        cjyx.SegmentEditorEffects.updateDMMLFromGUI()

    # 在SegmentEditorEffects中调用该函数，实现到鼠标在切片视图中滑动时，对应调节阈值滑块位置
    def updateGUIFromDMML(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        self.ui.RangeWidget.blockSignals(True)
        self.ui.RangeWidget.setMinimumValue(
            cjyx.SegmentEditorEffects.scriptedEffect.doubleParameter("MinimumThreshold"))
        self.ui.RangeWidget.setMaximumValue(
            cjyx.SegmentEditorEffects.scriptedEffect.doubleParameter("MaximumThreshold"))
        self.ui.RangeWidget.blockSignals(False)

    def onThresholdButton(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        cjyx.app.setOverrideCursor(qt.Qt.WaitCursor)
        min = self.ui.RangeWidget.minimumValue
        max = self.ui.RangeWidget.maximumValue
        effect = self.segmentEditorWidget.activeEffect()
        effect.setParameter('MinimumThreshold', min)
        effect.setParameter('MaximumThreshold', max)
        effect.self().onApply()
        cjyx.app.restoreOverrideCursor()
        self.ui.pushButton_Threshold.click()

    def onthresholdCancel(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        self.ui.pushButton_Threshold.click()





    # -------------------------区域增长（岛屿工具）-----------------------------------
    def OnpushButton_Island(self,pushbutton=None):
        self.OnCloseAllFunWidget()
        self.PushbuttonChecked(self.ui.pushButton_Island)
        if self.ui.pushButton_Island.checked:
            self.ui.groupBox_island.setVisible(1)
            self.ui.radioButton_island_max.click()


    def onIslandTool(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        # 自动复制一份当前图层
        a = self.segmentationNode.GetSegmentation().AddEmptySegment()
        segName=self.segmentationNode.GetSegmentation().GetSegment(a).GetName()
        self.segmentationNode.GetDisplayNode().SetSegmentVisibility3D(a, 0)
        self.onBoolTool(segName, self.name1, 'COPY')
        # 更新图层列表
        self.UpdataSegmentTableWidget()
        segment = self.segmentationNode.GetSegmentation().GetSegment(a)
        try:
            cjyx.modules.datamanager.widgetRepresentation().self().onSegmentNodeAdded(segment)
        except:
            pass
        KEEP_LARGEST_ISLAND = 'KEEP_LARGEST_ISLAND'
        KEEP_SELECTED_ISLAND = 'KEEP_SELECTED_ISLAND'
        REMOVE_SMALL_ISLANDS = 'REMOVE_SMALL_ISLANDS'
        REMOVE_SELECTED_ISLAND = 'REMOVE_SELECTED_ISLAND'
        ADD_SELECTED_ISLAND = 'ADD_SELECTED_ISLAND'
        SPLIT_ISLANDS_TO_SEGMENTS = 'SPLIT_ISLANDS_TO_SEGMENTS'

        self.segmentEditorWidget.setActiveEffectByName('Islands')
        effect = self.segmentEditorWidget.activeEffect()
        if(self.ui.radioButton_island_max.isChecked()):
            effect.setParameter('Operation', KEEP_LARGEST_ISLAND)
        else:
            effect.setParameter('Operation', REMOVE_SMALL_ISLANDS)
            effect.setParameter('MinimumSize', 1500)
        effect.self().onApply()

    def onKeepSelectIsland(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        a = self.segmentationNode.GetSegmentation().AddEmptySegment()
        segName=self.segmentationNode.GetSegmentation().GetSegment(a).GetName()
        self.segmentationNode.GetDisplayNode().SetSegmentVisibility3D(a, 0)
        self.onBoolTool(segName, self.name1, 'COPY')
        # 更新图层列表
        self.UpdataSegmentTableWidget()
        segment = self.segmentationNode.GetSegmentation().GetSegment(a)
        try:
            cjyx.modules.datamanager.widgetRepresentation().self().onSegmentNodeAdded(segment)
        except:
            pass
        KEEP_LARGEST_ISLAND = 'KEEP_LARGEST_ISLAND'
        KEEP_SELECTED_ISLAND = 'KEEP_SELECTED_ISLAND'
        REMOVE_SMALL_ISLANDS = 'REMOVE_SMALL_ISLANDS'
        REMOVE_SELECTED_ISLAND = 'REMOVE_SELECTED_ISLAND'
        ADD_SELECTED_ISLAND = 'ADD_SELECTED_ISLAND'
        SPLIT_ISLANDS_TO_SEGMENTS = 'SPLIT_ISLANDS_TO_SEGMENTS'

        self.segmentEditorWidget.setActiveEffectByName('Islands')
        effect = self.segmentEditorWidget.activeEffect()
        effect.setParameter('Operation', KEEP_SELECTED_ISLAND)
        #effect.self().onApply()
        self.ui.pushButton_island_confirm.setEnabled(0)

    def onIslandEnsureEnable(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        self.ui.pushButton_island_confirm.setEnabled(1)

    def onIslandCancel(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        self.ui.pushButton_Island.click()


    # --------------------------腐蚀膨胀-----------------------------------------------------------

    def OnpushButton_shrink(self,pushbutton=None):
        self.OnCloseAllFunWidget()
        self.PushbuttonChecked(self.ui.pushButton_shrink)
        if self.ui.pushButton_shrink.checked:
            self.ui.groupBox_shrink_expand.setVisible(1)
            self.ui.groupBox_shrink_expand.setTitle("图层腐蚀")
        

    def OnpushButton_expend(self,pushbutton=None):
        self.OnCloseAllFunWidget()
        self.PushbuttonChecked(self.ui.pushButton_expend)
        if self.ui.pushButton_expend.checked:
            self.ui.groupBox_shrink_expand.setVisible(1)
            self.ui.groupBox_shrink_expand.setTitle("图层膨胀")

    def on3DToolConfirm(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        value=self.ui.horizontalSlider_3DTool.value
        self.segmentEditorWidget.setActiveEffectByName('Margin')
        effect = self.segmentEditorWidget.activeEffect()
        if self.ui.pushButton_shrink.checked:
            effect.setParameter('MarginSizeMm', -value)
        else:
            effect.setParameter('MarginSizeMm', value)
        effect.self().onApply()


    def on3DToolCancel(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        if self.ui.pushButton_shrink.checked:
            self.ui.pushButton_shrink.click()
        else:
            self.ui.pushButton_expend.click()

    # --------------------------多层绘制（填充工具）-----------------------------------------------------------
    def OnpushButton_3Dfill(self,pushbutton=None):
        self.OnCloseAllFunWidget()
        self.PushbuttonChecked(self.ui.pushButton_3Dfill)
        if self.ui.pushButton_3Dfill.checked:
            self.ui.groupBox_3DFill.setVisible(1)


    def onFillBetweenSlices(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        self.GetSegmentState()
        self.SetAllVisibleFalse()
        self.FillSliceSegmentID = self.currentSelectSegmentID
        self.segmentationNode.GetDisplayNode().SetSegmentVisibility(self.FillSliceSegmentID, 1)
        self.UpdataSegmentTableWidget()
        self.segmentEditorWidget.setActiveEffectByName('Fill between slices')
        effect = self.segmentEditorWidget.activeEffect()
        effect.setParameter("AutoUpdate", 0)
        effect.self().onPreview()
        effect.self().onApply()
        self.SetSegmentVisibleTrue()
        self.UpdataSegmentTableWidget()

    def onFill3DCancel(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        self.ui.pushButton_3Dfill.click()
        self.SetSegmentVisibleTrue()
        self.UpdataSegmentTableWidget()

    # --------------------------bool-----------------------------------------------------------
    def OnpushButton_bool(self,pushbutton=None):
        self.OnCloseAllFunWidget()
        self.PushbuttonChecked(self.ui.pushButton_bool)
        if self.ui.pushButton_bool.checked:
            self.ui.groupbox_bool.setVisible(1)
            #向bool操作对象选择框内添加对象
            self.ui.comboBox_bool.clear()
            segmentation = self.segmentationNode.GetSegmentation()
            if segmentation.GetNumberOfSegments() > 0:
                number = segmentation.GetNumberOfSegments()
                for i in range(0, number):
                    segment = segmentation.GetNthSegment(i)
                    if segment.GetName() != 'Cylinder':
                        self.ui.comboBox_bool.addItem(segment.GetName())

    def onBoolConfirm(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        a = self.segmentationNode.GetSegmentation().AddEmptySegment()
        segName=self.segmentationNode.GetSegmentation().GetSegment(a).GetName()
        self.segmentationNode.GetDisplayNode().SetSegmentVisibility3D(a, 0)
        self.onBoolTool(segName, self.name1, 'COPY')
        # 更新图层列表
        self.UpdataSegmentTableWidget()
        segment = self.segmentationNode.GetSegmentation().GetSegment(a)
        try:
            cjyx.modules.datamanager.widgetRepresentation().self().onSegmentNodeAdded(segment)
        except:
            pass
        if(self.ui.radioButton_jiao1.checked):
            type="INTERSECT"
        elif(self.ui.radioButton_bing1.checked):
            type = "UNION"
        else:
            type = "SUBTRACT"
        row = self.ui.SegmenttableWidget.currentRow()
        try:
            text = self.ui.SegmenttableWidget.item(row, 1).text()
        except:
            for i in range(self.ui.SegmenttableWidget.rowCount):
                item = self.ui.SegmenttableWidget.item(i, 1)
                item.setSelected(False)
                return
        self.onBoolTool(text,self.ui.comboBox_bool.currentText,type)
        self.ui.comboBox_bool.clear()
        segmentation = self.segmentationNode.GetSegmentation()
        if segmentation.GetNumberOfSegments() > 0:
            number = segmentation.GetNumberOfSegments()
            for i in range(0, number):
                segment = segmentation.GetNthSegment(i)
                if segment.GetName() != 'Cylinder':
                    self.ui.comboBox_bool.addItem(segment.GetName())
        





    def onBoolCancel(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        self.ui.pushButton_bool.click()


    def onBoolTool(self, name1, name2, type):
        segid_tgt = self.segmentationNode.GetSegmentation().GetSegmentIdBySegmentName(name1)
        # 第二个listWidget
        segid_src = self.segmentationNode.GetSegmentation().GetSegmentIdBySegmentName(name2)
        self.segmentEditorWidget.setCurrentSegmentID(segid_tgt)
        self.segmentEditorWidget.setActiveEffectByName("Logical operators")
        effect = self.segmentEditorWidget.activeEffect()
        effect.setParameter("Operation", type)  # change the operation here
        effect.setParameter("ModifierSegmentID", segid_src)
        effect.self().onApply()

    # --------------------------重建-----------------------------------------------------------
    def OnpushButton_rebuild(self,pushbutton=None):
        self.OnCloseAllFunWidget()
        self.PushbuttonChecked(self.ui.pushButton_rebuild)
        if self.ui.pushButton_rebuild.checked:
            self.ui.groupbox_rebuild.setVisible(1)

    def onCreatSurface(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        segmentation = self.segmentationNode.GetSegmentation()
        currentsegmentID = self.segmentEditorWidget.currentSegmentID()
        name = segmentation.GetSegment(currentsegmentID).GetName()
        segment = segmentation.GetSegment(currentsegmentID)
        #若数据为空，则不重建
        try:
            full = cjyx.util.arrayFromSegment(self.segmentationNode, currentsegmentID).max() > 0
        except AttributeError:
            # there is no labelmap in the segment
            full = False
            b = qt.QMessageBox(qt.QMessageBox.Warning, '警告', "当前选中图层为空！", qt.QMessageBox.Apply)
            b.button(qt.QMessageBox().Apply).setText(' 确定 ')
        if(full):
            shNode = cjyx.dmmlScene.GetSubjectHierarchyNode()
            exportFolderItemId = shNode.GetSceneItemID()
            segmentIds = vtk.vtkStringArray()
            segmentIds.InsertNextValue(currentsegmentID)
            
            cjyx.modules.segmentations.logic().ExportSegmentsToModels(self.segmentationNode, segmentIds,
                                                                        exportFolderItemId)


    # --------------------------种子生长-----------------------------------------------------------
    def OnpushButton_grow(self,pushbutton=None):
        self.OnCloseAllFunWidget()
        self.PushbuttonChecked(self.ui.pushButton_grow)
        if self.ui.pushButton_grow.checked:
            self.ui.groupBox_fenli.setVisible(1)
            self.ui.pushButton_fenli_erase.setChecked(0)
            self.ui.pushButton_fenli_paint.setChecked(0)
            self.ui.pushButton_fenli_pick.setChecked(0)
            self.ui.pushButton_preview_fenli.setEnabled(1)
            self.growSetOtherEnable(0)
            self.target_names = []

        else:
            self.growSetOtherEnable(1)
            try:
                effect = self.segmentEditorWidget.activeEffect()
                effect.self().onCancel()
            except:
                print('图层分离跳过取消')

            pushbuttons_target = self.ui.groupBox_fenli.findChildren("QRadioButton")
            if len(pushbuttons_target)>0:
                for i in range(len(pushbuttons_target)):
                    segmentID = self.segmentationNode.GetSegmentation().GetSegmentIdBySegmentName(
                        pushbuttons_target[i].objectName)
                    pushbuttons_target[i].deleteLater()
                    segment = self.segmentationNode.GetSegmentation().GetSegment(segmentID)
                    try:
                        cjyx.modules.datamanager.widgetRepresentation().self().onSegmentNodeRemoved(segment)
                    except:
                        pass
                    self.segmentationNode.GetSegmentation().RemoveSegment(segmentID)
                self.UpdataSegmentTableWidget()


    def growSetOtherEnable(self,ifenable):
        self.ui.pushButton_manager.setEnabled(ifenable)
        self.ui.pushButton_painting.setEnabled(ifenable)
        self.ui.pushButton_erase.setEnabled(ifenable)
        # 3D工具
        self.ui.pushButton_Threshold.setEnabled(ifenable)
        self.ui.pushButton_Island.setEnabled(ifenable)
        self.ui.pushButton_shrink.setEnabled(ifenable)
        self.ui.pushButton_expend.setEnabled(ifenable)
        self.ui.pushButton_3Dfill.setEnabled(ifenable)
        self.ui.pushButton_bool.setEnabled(ifenable)
        self.ui.pushButton_rebuild.setEnabled(ifenable)
        self.ui.SegmenttableWidget.setEnabled(ifenable)

    def onAddTarget(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        a = self.segmentationNode.GetSegmentation().AddEmptySegment()
        segment = self.segmentationNode.GetSegmentation().GetSegment(a)
        try:
            cjyx.modules.datamanager.widgetRepresentation().self().onSegmentNodeAdded(segment)
        except:
            pass
        self.target_names.append(segment.GetName())
        RGBF = segment.GetColor()
        Hex = self.RGBF_to_Hex(RGBF)
        self.onTableWidget(segment.GetName())
        self.selectCurrentItem(segment.GetName())
        button = qt.QRadioButton()
        self.ui.Layout_target.addWidget(button)
        button.setObjectName(segment.GetName())
        button.setStyleSheet('QWidget{background-color:%s}' % Hex)
        button.clicked.connect(lambda arge1: self.selectCurrentItem(button.objectName))
        button.click()


    def onAddBackground(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        a = self.segmentationNode.GetSegmentation().AddEmptySegment()
        segment = self.segmentationNode.GetSegmentation().GetSegment(a)
        try:
            cjyx.modules.datamanager.widgetRepresentation().self().onSegmentNodeAdded(segment)
        except:
            pass
        RGBF = segment.GetColor()
        Hex = self.RGBF_to_Hex(RGBF)
        self.onTableWidget(segment.GetName())
        self.selectCurrentItem(segment.GetName())
        button = qt.QRadioButton()
        self.ui.Layout_background.addWidget(button)
        button.setObjectName(segment.GetName())
        button.setStyleSheet('QWidget{background-color:%s}' % Hex)
        button.clicked.connect(lambda arge1: self.selectCurrentItem(button.objectName))
        button.click()

    def onpushButton_fenli_erase(self,unusedArg1=None, unusedArg2=None, unusedArg3=None):
        self.PushbuttonChecked(self.ui.pushButton_fenli_erase)
        if self.ui.pushButton_fenli_erase.checked:
            self.ui.groupBox_paint.setVisible(1)
            self.segmentEditorWidget.setActiveEffectByName('Erase')
            effect = self.segmentEditorWidget.activeEffect()
            effect.setCommonParameter("BrushSphere", 0)
        else:
            self.segmentEditorWidget.setActiveEffectByName('None')
            self.ui.groupBox_paint.setVisible(0)

    def onpushButton_fenli_paint(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        self.PushbuttonChecked(self.ui.pushButton_fenli_paint)
        if self.ui.pushButton_fenli_paint.checked:
            self.ui.groupBox_paint.setVisible(1)
            self.segmentEditorWidget.setActiveEffectByName('Paint')
            effect = self.segmentEditorWidget.activeEffect()
            effect.setCommonParameter("BrushSphere", 0)
            self.ui.paint_circle.click()
            self.ui.paint_editor_in_3D.setChecked(0)
        else:
            self.segmentEditorWidget.setActiveEffectByName('None')
            self.ui.groupBox_paint.setVisible(0)

    def onpushButton_fenli_pick(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        self.PushbuttonChecked(self.ui.pushButton_fenli_pick)
        if self.ui.pushButton_fenli_pick.checked:
            self.segmentEditorWidget.setActiveEffectByName('Pick')
        else:
            self.segmentEditorWidget.setActiveEffectByName('None')


    def onpushButton_cancell_fenli(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):

        self.ui.pushButton_grow.click()





    def onpushButton_preview_fenli(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        self.GetSegmentState()
        self.SetAllVisibleFalse()
        self.ui.pushButton_fenli_erase.setChecked(0)
        self.ui.pushButton_fenli_paint.setChecked(0)
        self.ui.pushButton_fenli_pick.setChecked(0)
        pushbuttons_target = self.ui.groupBox_fenli.findChildren("QRadioButton")
        segmentation = self.segmentationNode.GetSegmentation()  # 使用self
        self.segment_all = []
        for i in range(len(pushbuttons_target)):
            self.segment_all.append(pushbuttons_target[i].objectName)
            segmentID = segmentation.GetSegmentIdBySegmentName(pushbuttons_target[i].objectName)
            self.segmentationNode.GetDisplayNode().SetSegmentVisibility(segmentID, 1)
        self.UpdataSegmentTableWidget()
        self.segmentEditorWidget.setActiveEffectByName('Grow from seeds')
        effect = self.segmentEditorWidget.activeEffect()
        effect.self().onPreview()
        self.ui.pushButton_preview_fenli.setEnabled(0)

    def onpushButton_ensure_fenli(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        if self.ui.pushButton_preview_fenli.enabled:
            self.GetSegmentState()
            self.SetAllVisibleFalse()
            pushbuttons_target=self.ui.groupBox_fenli.findChildren("QRadioButton")
            segmentation = self.segmentationNode.GetSegmentation()  # 使用self
            self.segment_all=[]
            for i in range(len(pushbuttons_target)):
                self.segment_all.append(pushbuttons_target[i].objectName)
                segmentID = segmentation.GetSegmentIdBySegmentName(pushbuttons_target[i].objectName)
                self.segmentationNode.GetDisplayNode().SetSegmentVisibility(segmentID, 1)
            self.UpdataSegmentTableWidget()
            self.segmentEditorWidget.setActiveEffectByName('Grow from seeds')
            effect = self.segmentEditorWidget.activeEffect()
            effect.self().onPreview()

        else:
            self.segmentEditorWidget.setActiveEffectByName('Grow from seeds')
            effect = self.segmentEditorWidget.activeEffect()


        effect.self().onApply()
        self.SetSegmentVisibleTrue()
        for i in range(len(self.segment_all)):
            if(self.segment_all[i] not in self.target_names):
                segmentID = self.segmentationNode.GetSegmentation().GetSegmentIdBySegmentName(self.segment_all[i])
                segment = self.segmentationNode.GetSegmentation().GetSegment(segmentID)
                try:
                    cjyx.modules.datamanager.widgetRepresentation().self().onSegmentNodeRemoved(segment)
                except:
                    pass
                self.segmentationNode.GetSegmentation().RemoveSegment(segmentID)

        self.UpdataSegmentTableWidget()
        pushbuttons_target = self.ui.groupBox_fenli.findChildren("QRadioButton")
        if len(pushbuttons_target) > 0:
            for i in range(len(pushbuttons_target)):
                pushbuttons_target[i].delete()
        if(self.ui.pushButton_grow.checked):
            self.ui.pushButton_grow.click()
        if(self.ui.pushButton_seed.checked):
            self.ui.pushButton_seed.click()


        # --------------------------可编辑区函数FitToVolume--------------------------------------------------------

    def onFitToVolume(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        if self.ui.pushButton_ROI.checked:
            a = cjyx.util.getNode('vtkDMMLScalarVolumeNode1')
            # [右、左、后、前、下、上]
            b = [0, 0, 0, 0, 0, 0]
            a.GetBounds(b)
            center = [(b[0] + b[1]) / 2, (b[2] + b[3]) / 2, (b[4] + b[5]) / 2]
            xyz = [(b[1] - b[0]) / 2, (b[3] - b[2]) / 2, (b[5] - b[4]) / 2]
            self.paint_roi = cjyx.dmmlScene.AddNewNodeByClass('vtkDMMLMarkupsROINode', '可编辑区域')
            self.paint_roi.SetRadiusXYZ(xyz)
            self.paint_roi.SetXYZ(center)
            self.paint_roi.GetDisplayNode().SetScaleHandleVisibility(True)
            self.paint_roi.GetDisplayNode().SetInteractionHandleScale(1)
            self.paint_roi.GetDisplayNode().SetFillOpacity(0)
            self.paint_roi.GetDisplayNode().SetTextScale(0)
            self.paint_roi.GetDisplayNode().SetSelectedColor(0.48627450980392156, 0.7411764705882353, 0.15294117647058825)
            self.paint_roi.GetDisplayNode().SetActiveColor(0.615686274509804, 0.9098039215686274, 1.0)
            self.paint_roi.GetDisplayNode().SetInteractionHandleScale(1)
            # 创建vtk模型
            self.cube = vtk.vtkCubeSource()
            self.cube.SetCenter(center)
            self.cube.SetXLength(2 * xyz[0])
            self.cube.SetYLength(2 * xyz[1])
            self.cube.SetZLength(2 * xyz[2])
            self.cube.Update()
            self.segmentationNode.AddSegmentFromClosedSurfaceRepresentation(self.cube.GetOutput(), "Cylinder",
                                                                            [0.0, 1.0, 0.0])
            displayNode = self.segmentationNode.GetDisplayNode()
            segmentation = self.segmentationNode.GetSegmentation()  # 使用self
            segmentID = segmentation.GetSegmentIdBySegmentName('Cylinder')
            displayNode.SetSegmentVisibility(segmentID, 0)
            self.segmentEditorNode.SetMaskMode(cjyx.vtkDMMLSegmentationNode.EditAllowedInsideSingleSegment)

            self.segmentEditorNode.SetMaskSegmentID(segmentID)
            self.r_observe = self.paint_roi.AddObserver(vtk.vtkCommand.ModifiedEvent, self.UpdataFitToVolume)
        else:
            self.segmentEditorNode.SetMaskMode(cjyx.vtkDMMLSegmentationNode.EditAllowedEverywhere)
            self.paint_roi.RemoveObserver(self.r_observe)
            segmentation = self.segmentationNode.GetSegmentation()  # 使用self
            segmentID = segmentation.GetSegmentIdBySegmentName('Cylinder')
            cjyx.dmmlScene.RemoveNode(self.paint_roi)
            self.segmentationNode.RemoveSegment(segmentID)
        # ROI观察者函数

    def UpdataFitToVolume(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        self.IfUpdateROI = 0
        ROIbounds = [0, 0, 0, 0, 0, 0]
        self.paint_roi.GetBounds(ROIbounds)
        qt.QTimer.singleShot(1500, lambda: self.UpdateROI(ROIbounds))

    def UpdateROI(self, ROIbound):
        bounds = [0, 0, 0, 0, 0, 0]
        self.paint_roi.GetBounds(bounds)
        if (bounds == ROIbound):
            print("更新ROI")
            center = [0, 0, 0]
            xyz = [0, 0, 0]
            self.paint_roi.GetXYZ(center)
            self.paint_roi.GetRadiusXYZ(xyz)
            self.cube.SetCenter(center)
            self.cube.SetXLength(2 * xyz[0])
            self.cube.SetYLength(2 * xyz[1])
            self.cube.SetZLength(2 * xyz[2])
            self.cube.Update()
            segmentation = self.segmentationNode.GetSegmentation()
            segmentID = segmentation.GetSegmentIdBySegmentName('Cylinder')
            self.segmentationNode.RemoveSegment(segmentID)
            self.segmentationNode.AddSegmentFromClosedSurfaceRepresentation(self.cube.GetOutput(), "Cylinder",
                                                                            [0.0, 1.0, 0.0])
            self.segmentEditorNode.SetMaskMode(cjyx.vtkDMMLSegmentationNode.EditAllowedInsideSingleSegment)

            displayNode = self.segmentationNode.GetDisplayNode()
            displayNode.SetSegmentVisibility(segmentID, 0)
            self.segmentEditorNode.SetMaskSegmentID(segmentID)


        # --------------------------图层分离--------------------------------------------------------
        # --------------------------种子生长-----------------------------------------------------------
    def OnpushButton_seed(self,pushbutton=None):
        self.OnCloseAllFunWidget()
        self.PushbuttonChecked(self.ui.pushButton_seed)
        if self.ui.pushButton_seed.checked:
            self.ui.groupBox_fenli.setVisible(1)
            self.ui.pushButton_fenli_erase.setChecked(0)
            self.ui.pushButton_fenli_paint.setChecked(0)
            self.ui.pushButton_fenli_pick.setChecked(0)
            self.ui.pushButton_preview_fenli.setEnabled(1)
            self.growSetOtherEnable(0)
            self.target_names = []
            cjyx.dmmlScene.AddNewNodeByClass("vtkDMMLScalarVolumeNode", 'seedUseVolume')
            self.seedUseVolume = cjyx.util.getNode('seedUseVolume')
            import SegmentEditorEffects
            maskVolumeWithSegment = SegmentEditorEffects.SegmentEditorMaskVolumeEffect.maskVolumeWithSegment
            fillValue = -1000  
            maskVolumeWithSegment(self.segmentationNode, self.currentSelectSegmentID, "FILL_OUTSIDE", [fillValue], self.masterVolumeNode, self.seedUseVolume)
            self.segmentEditorWidget.setMasterVolumeNode(self.seedUseVolume)
            print("seed:",self.currentSelectSegmentID)
            self.segmentEditorNode.SetMaskMode(cjyx.vtkDMMLSegmentationNode.EditAllowedInsideSingleSegment)
            self.segmentEditorNode.SetMaskSegmentID(self.currentSelectSegmentID)
            self.segmentEditorNode.SetMaskMode(cjyx.vtkDMMLSegmentationNode.EditAllowedInsideSingleSegment)
            self.segmentEditorNode.SetMaskSegmentID(self.currentSelectSegmentID)
            self.segmentEditorNode.SetMaskMode(cjyx.vtkDMMLSegmentationNode.EditAllowedInsideSingleSegment)
            self.segmentEditorNode.SetMaskSegmentID(self.currentSelectSegmentID)
            self.ui.pushButton_ROI.setEnabled(0)



        else:
            self.growSetOtherEnable(1)
            
            self.segmentEditorWidget.setMasterVolumeNode(self.masterVolumeNode)
            self.segmentEditorNode.SetMaskMode(cjyx.vtkDMMLSegmentationNode.EditAllowedEverywhere)
            self.ui.pushButton_ROI.setEnabled(1)
            try:
                cjyx.dmmlScene.RemoveNode(self.seedUseVolume)
            except:
                print('seedUseVolume删除')
            try:
                effect = self.segmentEditorWidget.activeEffect()
                effect.self().onCancel()
            except:
                print('图层分离跳过取消')

            pushbuttons_target = self.ui.groupBox_fenli.findChildren("QRadioButton")
            if len(pushbuttons_target)>0:
                for i in range(len(pushbuttons_target)):
                    segmentID = self.segmentationNode.GetSegmentation().GetSegmentIdBySegmentName(
                        pushbuttons_target[i].objectName)
                    pushbuttons_target[i].deleteLater()
                    segment = self.segmentationNode.GetSegmentation().GetSegment(segmentID)
                    try:
                        cjyx.modules.datamanager.widgetRepresentation().self().onSegmentNodeRemoved(segment)
                    except:
                        pass
                    self.segmentationNode.GetSegmentation().RemoveSegment(segmentID)
                self.UpdataSegmentTableWidget()



    # -------------------------QTableWidget函数-------------------------------
    # 设置颜色
    def colorDialog(self, pushbutton):
        # 设置上一个颜色为初始颜色，可以在打开弹窗时颜色位置在上一个颜色
        lis = self.segmentationNode.GetSegmentation().GetSegment(pushbutton.objectName)
        RGBF = lis.GetColor()
        Hex = self.RGBF_to_Hex(RGBF)
        a = qt.QColor(Hex)
        col = qt.QColorDialog().getColor(a)
        # 判断是否有效，可以避免点击取消或者关闭时颜色变为黑色
        if col.isValid():
            pushbutton.setStyleSheet('QWidget{background-color:%s}' % col.name())
            lis.SetColor(col.redF(), col.greenF(), col.blueF())
            cjyx.modules.datamanager.widgetRepresentation().self().updateTableWidgetItem()

    # 设置2D显示与隐藏
    def onShowHide(self, checkBox):
        self.selectCurrentItemByID(checkBox.objectName[:-1])
        if checkBox.checked:
            self.segmentationNode.GetDisplayNode().SetSegmentVisibility(checkBox.objectName[:-1], 1)
            for i in range(0, len(self.ui.SegmenttableWidget.findChildren('QPushButton'))):
                a = self.ui.SegmenttableWidget.findChildren('QPushButton')[i]
                if a.objectName == checkBox.objectName[:-1] + "b":
                    a.setEnabled(True)
        else:
            self.segmentationNode.GetDisplayNode().SetSegmentVisibility(checkBox.objectName[:-1], 0)
            for i in range(0, len(self.ui.SegmenttableWidget.findChildren('QPushButton'))):
                a = self.ui.SegmenttableWidget.findChildren('QPushButton')[i]
                if a.objectName == checkBox.objectName[:-1] + "b":
                    a.setChecked(False)
                    a.setEnabled(False)
                    self.segmentationNode.GetDisplayNode().SetSegmentVisibility3D(checkBox.objectName[:-1], 0)
        cjyx.modules.datamanager.widgetRepresentation().self().updateTableWidgetItem()
        self.segmentationNode.Modified()

    # 设置3D显示与隐藏
    def onShowHide3D(self, checkBox1):

        self.selectCurrentItemByID(checkBox1.objectName[:-1])
        if checkBox1.checked:
            cjyx.app.setOverrideCursor(qt.QCursor(qt.Qt.WaitCursor))
            show3Dstate = 0
            for i in range(0, len(self.ui.SegmenttableWidget.findChildren('QPushButton'))):
                a = self.ui.SegmenttableWidget.findChildren('QPushButton')[i]
                if a.objectName[-1] == 'b':
                    if a.checked:
                        show3Dstate += 1
            if show3Dstate == 1:
                self.segmentationNode.CreateClosedSurfaceRepresentation()  # Show 3D
            self.segmentationNode.GetDisplayNode().SetSegmentVisibility3D(checkBox1.objectName[:-1], 1)
            # cjyx.app.layoutManager().threeDWidget('View1').threeDView().resetFocalPoint()
            cjyx.app.restoreOverrideCursor()
        else:
            show3Dstate = 0
            self.segmentationNode.GetDisplayNode().SetSegmentVisibility3D(checkBox1.objectName[:-1], 0)
            for i in range(0, len(self.ui.SegmenttableWidget.findChildren('QPushButton'))):
                a = self.ui.SegmenttableWidget.findChildren('QPushButton')[i]
                if a.objectName[-1] == 'b':
                    if a.checked:
                        show3Dstate += 1
            if show3Dstate == 0:
                self.segmentationNode.RemoveClosedSurfaceRepresentation()

    # 双击重命名
    def onRename(self, item):
        self.prename = item.text()
        item.setFlags(item.flags() | qt.Qt.ItemIsEditable)
        self.ui.SegmenttableWidget.editItem(item)
        self.rename = True
        # 获取listWidget中全部Item名字
        self.AllName = []
        for i in range(0, self.ui.SegmenttableWidget.rowCount):
            if self.ui.SegmenttableWidget.item(i, 1).text() != self.prename:
                self.AllName.append(self.ui.SegmenttableWidget.item(i, 1).text())
        print('双击')
        self.line = self.ui.SegmenttableWidget.currentRow()

    # 重命名函数
    def onChanged(self, x, y):
        print('重命名', self.prename, self.ui.SegmenttableWidget.item(x, y).text())
        if self.rename and self.prename != self.ui.SegmenttableWidget.item(x, y).text() and self.line == x:
            if self.ui.SegmenttableWidget.item(x, y).text() in self.AllName:
                # 建立警告信息
                b = qt.QMessageBox(qt.QMessageBox.Warning, '警告', "名称已存在，请重新编辑！", qt.QMessageBox.Apply)
                b.button(qt.QMessageBox().Apply).setText(' 确定 ')
                c = b.exec()
                if c == qt.QMessageBox.Apply:
                    self.ui.SegmenttableWidget.item(x, y).setText(self.prename)
                    self.rename = False
            else:
                count = self.segmentationNode.GetSegmentation().GetNumberOfSegments()
                for i in range(0, count):
                    if self.prename == self.segmentationNode.GetSegmentation().GetNthSegment(i).GetName():
                        self.segmentationNode.GetSegmentation().GetNthSegment(i).SetName(
                            self.ui.SegmenttableWidget.item(x, y).text())
                        self.rename = False
                        cjyx.modules.datamanager.widgetRepresentation().self().rename1(self.segmentationNode.GetSegmentation().GetSegment(self.currentSelectSegmentID))
    # 单击Item选择对应分割
    def onSelectItem(self, item):
        try:
            Row = self.ui.SegmenttableWidget.row(item)
            text = self.ui.SegmenttableWidget.item(Row, 1).text()
            segmentation = self.segmentationNode.GetSegmentation()
            segmentID = segmentation.GetSegmentIdBySegmentName(text)
            self.segmentEditorNode.SetSelectedSegmentID(segmentID)
            self.name1 = text
            print('SegmenttableWidget:' + text)
            self.currentSelectSegmentID = self.segmentEditorWidget.currentSegmentID()
        except:
            pass

    # 选择当前的切片item
    def selectCurrentItem(self, name):
        for i in range(self.ui.SegmenttableWidget.rowCount):
            item = self.ui.SegmenttableWidget.item(i, 1)
            item.setSelected(False)
            if str(name) == self.ui.SegmenttableWidget.item(i, 1).text():
                item.setSelected(True)
                try:
                    Row = self.ui.SegmenttableWidget.row(item)
                    text = self.ui.SegmenttableWidget.item(Row, 1).text()
                    segmentation = self.segmentationNode.GetSegmentation()
                    segmentID = segmentation.GetSegmentIdBySegmentName(text)
                    self.segmentEditorNode.SetSelectedSegmentID(segmentID)
                    self.name1 = text
                    self.ui.SegmenttableWidget.setCurrentItem(item)
                    print('SegmenttableWidget:' + text)
                    self.currentSelectSegmentID = self.segmentEditorWidget.currentSegmentID()
                except:
                    pass

                
    # 选择当前的切片item
    def selectCurrentItemByID(self, ID):
        for i in range(self.ui.SegmenttableWidget.rowCount):
            item = self.ui.SegmenttableWidget.item(i, 1)
            item.setSelected(False)
            if str(ID) == self.ui.SegmenttableWidget.cellWidget(i, 0).objectName:
                item.setSelected(True)
                try:
                    Row = self.ui.SegmenttableWidget.row(item)
                    text = self.ui.SegmenttableWidget.item(Row, 1).text()
                    self.segmentEditorNode.SetSelectedSegmentID(ID)
                    self.name1 = text
                    self.ui.SegmenttableWidget.setCurrentItem(item)
                    print('SegmenttableWidget:' + text)
                    self.currentSelectSegmentID = self.segmentEditorWidget.currentSegmentID()
                except:
                    pass



    def updateTableWidget(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        segmentation = self.segmentationNode.GetSegmentation()
        number = segmentation.GetNumberOfSegments()
        if number > 0:
            for i in range(1, number + 1):
                name = segmentation.GetNthSegment(segmentation.GetNumberOfSegments() - i).GetName()
                self.onTableWidget(name)



# 为TableWidget添加行
    def onTableWidget(self, name,IfNew=1):
        if name:
            print("name",name)
            cur_row_count = self.ui.SegmenttableWidget.rowCount
            cur_row_count += 1
            self.ui.SegmenttableWidget.setRowCount(cur_row_count)
            segmentation = self.segmentationNode.GetSegmentation()
            segmentID = segmentation.GetSegmentIdBySegmentName(name)
            segment = segmentation.GetSegment(segmentID)
            RGBF = segment.GetColor()
            Hex = self.RGBF_to_Hex(RGBF)
            # b = qt.QColor("#37e4ff")
            self.col = Hex
            # segment.SetColor(b.redF(),b.greenF(),b.blueF())
            pushbutton = qt.QPushButton()
            pushbutton.setObjectName(segmentID)
            pushbutton.clicked.connect(lambda arg1: self.colorDialog(pushbutton))
            pushbutton.setStyleSheet('QWidget{background-color:%s}' % Hex)
            # pushbutton.resize(16,16)

            checkBox = qt.QPushButton()
            checkBox.setCheckable(True)
            iconVisible = self.getTwoIcon("show")
            checkBox.setIcon(iconVisible)
            checkBoxObjectName = segmentID + "a"
            checkBox.setObjectName(checkBoxObjectName)
            checkBox.setStyleSheet("border:0px;background:transparent;")
            Visible2D = self.segmentationNode.GetDisplayNode().GetSegmentVisibility(segmentID)
            checkBox.setChecked(Visible2D)
            checkBox.clicked.connect(lambda arg1: self.onShowHide(checkBox))
            checkBox1 = qt.QPushButton()
            checkBox1ObjectName = segmentID + "b"
            checkBox1.setObjectName(checkBox1ObjectName)
            checkBox1.setIcon(iconVisible)
            checkBox1.setCheckable(True)
            checkBox1.setStyleSheet("border:0px;background:transparent;")
            checkBox1.setChecked(False)

            if Visible2D == False:
                checkBox1.setEnabled(False)
            # self.segmentationNode.CreateClosedSurfaceRepresentation()  # Show 3D
            if IfNew:
                self.segmentationNode.GetDisplayNode().SetSegmentVisibility3D(segmentID, 0)
            # cjyx.app.layoutManager().threeDWidget('View1').threeDView().resetFocalPoint()
            checkBox1.clicked.connect(lambda arg1: self.onShowHide3D(checkBox1))
            Visible3D=self.segmentationNode.GetDisplayNode().GetSegmentVisibility3D(segmentID)
            checkBox1.setChecked(Visible3D)
            # self.iconsPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons')
            # ShowPath = os.path.join(self.iconsPath,'Show.png')
            # HidePath = os.path.join(self.iconsPath,'Hide.png')
            # checkBox.setStyleSheet('QCheckBox::checked {image: url(%s);}QCheckBox::unchecked {image: url(%s);}'%ShowPath%HidePath)
            self.ui.SegmenttableWidget.setCellWidget(cur_row_count - 1, 0, pushbutton)
            self.ui.SegmenttableWidget.setItem(cur_row_count - 1, 1, qt.QTableWidgetItem(name))
            self.ui.SegmenttableWidget.setCellWidget(cur_row_count - 1, 2, checkBox)
            self.ui.SegmenttableWidget.setCellWidget(cur_row_count - 1, 3, checkBox1)
            self.currentSelectSegmentID = self.segmentEditorWidget.currentSegmentID()
            # check = qt.QTableWidgetItem()
            # check.setCheckState(qt.Qt.Unchecked)
            # self.ui.tableWidget.setItem(0,2,check)
            # self.ui.tableWidget.item(0,2).setFlags(qt.Qt.ItemIsSelectable)

    # 更新图层列表
    def UpdataSegmentTableWidget(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        num=self.ui.SegmenttableWidget.rowCount
        for i in range(num):
            self.ui.SegmenttableWidget.removeRow(0)
        try:
            segmentation = self.segmentationNode.GetSegmentation()
            if segmentation.GetNumberOfSegments() > 0:
                number = segmentation.GetNumberOfSegments()
                for i in range(0, number):
                    segment = segmentation.GetNthSegment(i)
                    if segment.GetName() != 'Cylinder':
                        self.onTableWidget(segment.GetName(),0)
            try:
                name = segmentation.GetSegment(self.currentSelectSegmentID).GetName()
                self.selectCurrentItem(name)
            except:
                pass
        except:
            pass

    # 颜色转换(RGBF转化为十六进制)
    def RGBF_to_Hex(self, rgb):
        color = '#'
        R = int(rgb[0] * 255)
        G = int(rgb[1] * 255)
        B = int(rgb[2] * 255)
        RGB = []
        RGB.append(R)
        RGB.append(G)
        RGB.append(B)
        for i in RGB:
            color += str(hex(i))[-2:].replace('x', '0').upper()
        return color

    # 颜色转换(十六进制转化为RGBF)
    def Hex_to_RGBF(self, hex):
        r = int(hex[1:3], 16)
        g = int(hex[3:5], 16)
        b = int(hex[5:7], 16)
        RGBF = (r / 255, g / 255, b / 255)
        return RGBF

    def getTwoIcon(self,name):
        iconsPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons')
        ShowPath = os.path.join(iconsPath,name+".png")
        HidePath = os.path.join(iconsPath,name+"2.png")
        icon=qt.QIcon()
        icon.addPixmap(qt.QPixmap(ShowPath), qt.QIcon().Normal, qt.QIcon().Off)
        icon.addPixmap(qt.QPixmap(HidePath), qt.QIcon().Normal, qt.QIcon().On)
        return icon

    # -------------------------------------------图层分离和多层绘制设置状态-----------------------------
    # 获取所有分段的状态
    def GetSegmentState(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        try:
            if len(self.list) != 0:
                return
        except:
            self.list = []

        self.currentSelectSegmentID = self.segmentEditorWidget.currentSegmentID()

        segmentation = self.segmentationNode.GetSegmentation()
        if segmentation.GetNumberOfSegments() > 0:
            number = segmentation.GetNumberOfSegments()
            for i in range(0, number):
                segment = segmentation.GetNthSegment(i)
                segmentID = segmentation.GetSegmentIdBySegment(segment)
                Visible2D = self.segmentationNode.GetDisplayNode().GetSegmentVisibility(segmentID)
                Visible3D = self.segmentationNode.GetDisplayNode().GetSegmentVisibility3D(segmentID)
                self.list.append([segmentID, Visible2D, Visible3D])

    # 设置全部隐藏不可见
    def SetAllVisibleFalse(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        segmentation = self.segmentationNode.GetSegmentation()
        if segmentation.GetNumberOfSegments() > 0:
            number = segmentation.GetNumberOfSegments()
            for i in range(1, number + 1):
                segment = segmentation.GetNthSegment(segmentation.GetNumberOfSegments() - i)
                segmentID = segmentation.GetSegmentIdBySegment(segment)
                self.segmentationNode.GetDisplayNode().SetSegmentVisibility(segmentID, 0)
                self.segmentationNode.GetDisplayNode().SetSegmentVisibility3D(segmentID, 0)

    # 设置分段的可见性
    def SetSegmentVisibleTrue(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        segmentation = self.segmentationNode.GetSegmentation()
        number = segmentation.GetNumberOfSegments()
        for i in range(0, number):
            segment = segmentation.GetNthSegment(i)
            segmentID = segmentation.GetSegmentIdBySegment(segment)
            for j in range(len(self.list)):
                if segmentID in self.list[j]:
                    self.segmentationNode.GetDisplayNode().SetSegmentVisibility(segmentID, self.list[j][1])
                    self.segmentationNode.GetDisplayNode().SetSegmentVisibility3D(segmentID, self.list[j][2])
        self.list = []

    def function(self,sender=None):
        self.PushbuttonChecked(sender)

    def CloseAllToolWidget(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        self.ui.widget_manager.setVisible(0)
        self.ui.widget_painting.setVisible(0)
        self.ui.widget_wrap.setVisible(0)
        self.ui.widget_3DTool.setVisible(0)

        # 图层绘制
        self.ui.pushButton_dray.setChecked(False)
        self.ui.pushButton_paint.setChecked(False)
        self.ui.pushButton_pick.setChecked(False)
        self.ui.pushButton_lasso.setChecked(False)
        # 图层擦除
        self.ui.pushButton_dray_wrap.setChecked(False)
        self.ui.pushButton_paint_wrap.setChecked(False)
        self.ui.pushButton_scissors.setChecked(False)
        # 3D工具
        self.ui.pushButton_Threshold.setChecked(False)
        self.ui.pushButton_Island.setChecked(False)
        self.ui.pushButton_shrink.setChecked(False)
        self.ui.pushButton_expend.setChecked(False)
        self.ui.pushButton_3Dfill.setChecked(False)
        self.ui.pushButton_bool.setChecked(False)
        self.ui.pushButton_rebuild.setChecked(False)
        if(self.ui.pushButton_grow.checked):
            self.ui.pushButton_grow.click()
        if(self.ui.pushButton_seed.checked):
            self.ui.pushButton_seed.click()
        self.OnCloseAllFunWidget()

    def OnCloseAllFunWidget(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
       self.ui.groupBox_paint.setVisible(0)
       self.ui.groupBox_fenli.setVisible(0)
       self.ui.groupBox_island.setVisible(0)
       self.ui.groupBox_shrink_expand.setVisible(0)
       self.ui.groupbox_rebuild.setVisible(0)
       self.ui.groupbox_threshold.setVisible(0)
       self.ui.groupbox_bool.setVisible(0)
       self.ui.groupBox_3DFill.setVisible(0)

       try:
            self.segmentEditorWidget.setActiveEffectByName('None')
       except:
         pass

    #按钮互斥实现，可针对所有按钮
    def PushbuttonChecked(self,pushbutton):
        ifchecked = pushbutton.isChecked()
        pushList = pushbutton.parent().findChildren("QPushButton")
        for i in range(0,len(pushList)):
            if pushList[i]==self.ui.pushButton_ROI:
                continue
            pushList[i].setChecked(0)
            if ifchecked:
                pushbutton.setChecked(1)



    def OnMeunChanged(self,name):
        if (name == "Action100"):
        
            self.ui.pushButton_manager.click()
            self.ui.pushbutton_add_segment.click()
    
        if (name == "Action101"):
        
            self.ui.pushButton_manager.click()
            self.ui.pushbutton_remove_segment.click()

        if (name == "Action102"):
            self.ui.pushButton_manager.click()
            self.ui.pushbutton_undo.click()

        if (name == "Action103"):
            self.ui.pushButton_manager.click()
            self.ui.pushbutton_redo.click()

        if (name == "Action104"):
            self.ui.pushButton_manager.click()
            self.ui.pushbutton_copy.click()

        if (name == "Action110"):
        
            self.ui.pushButton_painting.click()
            self.ui.pushButton_dray.click()
    
        if (name == "Action111"):
        
            self.ui.pushButton_painting.click()
            self.ui.pushButton_paint.click()
        
        if (name == "Action112"):
        
            self.ui.pushButton_painting.click()
            self.ui.pushButton_lasso.click()

        if (name == "Action113"):
            self.ui.pushButton_painting.click()
            self.ui.pushButton_3Dfill.click()

        if (name == "Action114"):
            self.ui.pushButton_painting.click()
            self.ui.pushButton_pick.click()
        
        if (name == "Action123"):
        
            self.ui.pushButton_erase.click()
            self.ui.pushButton_dray_wrap.click()
        
        if (name == "Action121"):
        
            self.ui.pushButton_erase.click()
            self.ui.pushButton_paint_wrap.click()
        
        if (name == "Action122"):
        
            self.ui.pushButton_erase.click()
            self.ui.pushButton_scissors.click()



        if (name == "Action130"):
        
            self.ui.pushButton_3DTool.click()
            self.ui.pushButton_Threshold.click()
        
        if (name == "Action131"):
        
            self.ui.pushButton_3DTool.click()
            self.ui.pushButton_Island.click()
        
        if (name == "Action132"):
        
            self.ui.pushButton_3DTool.click()
            self.ui.pushButton_shrink.click()
        
        if (name == "Action133"):
        
            self.ui.pushButton_3DTool.click()
            self.ui.pushButton_expend.click()
        
        if (name == "Action134"):
        
            self.ui.pushButton_3DTool.click()
            self.ui.pushButton_3Dfill.click()
        
        if (name == "Action135"):
    
            self.ui.pushButton_3DTool.click()
            self.ui.pushButton_bool.click()
        
        if (name == "Action136"):
        
            self.ui.pushButton_3DTool.click()
            self.ui.pushButton_rebuild.click()
        
        if (name == "Action137"):
        
            self.ui.pushButton_3DTool.click()
            self.ui.pushButton_grow.click()

        if (name == "Action138"):
            self.ui.pushButton_3DTool.click()
            self.ui.pushButton_ROI.click()

    def CreateRenderingWidget3D(self):
        if(self.popwidget==None):
            layoutManager = cjyx.app.layoutManager()
            threeDWidget = layoutManager.threeDWidget(0).threeDView()
            self.popwidget = qt.QWidget()
            self.popwidget.setParent(threeDWidget)
            self.popwidget.setStyleSheet("background-color: rgba(69,69,69,0.2);")
            self.popwidget.resize(100, 150)
            popLayout = qt.QVBoxLayout()
            self.popwidget.setLayout(popLayout)
            moveX =threeDWidget.width-self.popwidget.width - 10.0
            self.popwidget.move(moveX, 10);

            #预设窗口
            widget_preset = qt.QWidget()
            presetLayout =qt.QVBoxLayout()
            #presetLayout.setContentsMargins(4, 4, 4, 4)
            widget_preset.setLayout(presetLayout)
            widget_preset.setStyleSheet("background-color:transparent;")
            label_preset =qt.QLabel("预设:")
            label_preset.setStyleSheet("background-color:transparent;")
            pushButton_1 =qt.QPushButton("CT 通用")
            pushButton_1.setStyleSheet("QPushButton:checked{background-color:#000000;color:#7cbd27}")
            pushButton_1.setCheckable(1)
            pushButton_1.clicked.connect(self.SetCTPreset)
            pushButton_1.setObjectName("CT")
            pushButton_2 =qt.QPushButton("MR 通用")
            pushButton_2.setStyleSheet("QPushButton:checked{background-color:#000000;color:#7cbd27}")
            pushButton_2.setCheckable(1)
            pushButton_2.setObjectName("MR")
            pushButton_2.clicked.connect(self.SetMRPreset)
            presetLayout.addWidget(label_preset)
            presetLayout.addWidget(pushButton_1)
            presetLayout.addWidget(pushButton_2)

            #强度窗口
            widget_shift =qt.QWidget()
            widget_shift.setStyleSheet("background-color:transparent;")
            shiftLayout =qt.QVBoxLayout()
            # shiftLayout.setContentsMargins(4, 4, 4, 4)
            widget_shift.setLayout(shiftLayout)
            label_shift =qt.QLabel("强度:")
            label_shift.setStyleSheet("background-color:transparent;")
            moveLayout = qt.QHBoxLayout()
            moveLayout.setContentsMargins(0, 0, 0, 0)
            widget_move = qt.QWidget()
            widget_move.setStyleSheet("background-color:transparent;")
            widget_move.setLayout(moveLayout)
            pushButton_up=qt.QPushButton("+")
            pushButton_up.setStyleSheet("QPushButton:hover{background-color:#000000;color:#7cbd27}")
            pushButton_up.resize(24,24)
            pushButton_up.clicked.connect(self.onpushButton_up)
            moveLayout.addWidget(pushButton_up)


            pushButton_down=qt.QPushButton("-")
            pushButton_down.setStyleSheet("QPushButton:hover{background-color:#000000;color:#7cbd27}")
            pushButton_down.resize(24, 24)
            pushButton_down.clicked.connect(self.onpushButton_down)
            moveLayout.addWidget(pushButton_down)

            #connect(slider, & QSlider::valueChanged, this, & QcjyxVolumeVisualizationView::onShift)
            shiftLayout.addWidget(label_shift)
            shiftLayout.addWidget(widget_move)
            popLayout.addWidget(widget_preset)
            popLayout.addWidget(widget_shift)
            self.updatePopWidgetPosition()
            self.popwidget.show()
        else:
            self.updatePopWidgetPosition()
            self.popwidget.show()


    def CloseRenderingWidget3D(self):
        self.popwidget.hide()

    def onpushButton_down(self,arg1=None,arg2=None):
        volRenWidget = cjyx.modules.volumerendering.widgetRepresentation()
        volRenLogic = cjyx.modules.volumerendering.logic()
        displayNode = volRenLogic.CreateDefaultVolumeRenderingNodes(cjyx.util.getNode('vtkDMMLScalarVolumeNode1'))
        if volRenWidget is None:
            logging.error('Failed to access volume rendering module')
            return
        # Make sure the proper volume property node is set
        volumePropertyNode = displayNode.GetVolumePropertyNode()
        if volumePropertyNode is None:
            logging.error('Failed to access volume properties')
            return
        volumePropertyNodeWidget = cjyx.util.findChild(volRenWidget, 'VolumePropertyNodeWidget')
        volumePropertyNodeWidget.setDMMLVolumePropertyNode(volumePropertyNode)
        # Adjust the transfer function
        volumePropertyNodeWidget.moveAllPoints(-100, 0, 0)

    def onpushButton_up(self, arg1=None, arg2=None):
        volRenWidget = cjyx.modules.volumerendering.widgetRepresentation()
        volRenLogic = cjyx.modules.volumerendering.logic()
        displayNode = volRenLogic.CreateDefaultVolumeRenderingNodes(cjyx.util.getNode('vtkDMMLScalarVolumeNode1'))
        if volRenWidget is None:
            logging.error('Failed to access volume rendering module')
            return
        # Make sure the proper volume property node is set
        volumePropertyNode = displayNode.GetVolumePropertyNode()
        if volumePropertyNode is None:
            logging.error('Failed to access volume properties')
            return
        volumePropertyNodeWidget = cjyx.util.findChild(volRenWidget, 'VolumePropertyNodeWidget')
        volumePropertyNodeWidget.setDMMLVolumePropertyNode(volumePropertyNode)
        # Adjust the transfer function
        volumePropertyNodeWidget.moveAllPoints(100, 0, 0)

    def SetCTPreset(self,arg1=None,arg2=None):
        if (self.popwidget.findChild("QPushButton","CT").isChecked()):
            volRenLogic = cjyx.modules.volumerendering.logic()
            displayNode = volRenLogic.CreateDefaultVolumeRenderingNodes(cjyx.util.getNode('vtkDMMLScalarVolumeNode1'))
            displayNode.SetVisibility(True)
            displayNode.GetVolumePropertyNode().Copy(volRenLogic.GetPresetByName("CT-Chest-Contrast-Enhanced"))
            self.popwidget.findChild("QPushButton", "MR").setChecked(0)
        else:
            volRenLogic = cjyx.modules.volumerendering.logic()
            displayNode = volRenLogic.CreateDefaultVolumeRenderingNodes(cjyx.util.getNode('vtkDMMLScalarVolumeNode1'))
            displayNode.SetVisibility(0)


    def SetMRPreset(self,arg1=None,arg2=None):
        if (self.popwidget.findChild("QPushButton","MR").isChecked()):
            volRenLogic = cjyx.modules.volumerendering.logic()
            displayNode = volRenLogic.CreateDefaultVolumeRenderingNodes(cjyx.util.getNode('vtkDMMLScalarVolumeNode1'))
            displayNode.SetVisibility(True)
            displayNode.GetVolumePropertyNode().Copy(volRenLogic.GetPresetByName("MR-Default"))
            self.popwidget.findChild("QPushButton", "CT").setChecked(0)
        else:
            volRenLogic = cjyx.modules.volumerendering.logic()
            displayNode = volRenLogic.CreateDefaultVolumeRenderingNodes(cjyx.util.getNode('vtkDMMLScalarVolumeNode1'))
            displayNode.SetVisibility(0)

    def onCloseWidget(self):
        cjyx.util.mainWindow().findChild("QDockWidget").hide()
        action = cjyx.util.mainWindow().findChild("QAction","ImageRebuildAction")
        action.setChecked(0)

    #渲染用
    def updatePopWidgetPosition(self,arg1=None,arg2=None,arg3=None):
        # print("pop1")
        layoutManager = cjyx.app.layoutManager()
        threeDWidget = layoutManager.threeDWidget(0).threeDView()
        print(threeDWidget.width,threeDWidget.size)
        if self.popwidget:
            moveX = threeDWidget.width - self.popwidget.width - 10.0
            try:
                self.popwidget.move(moveX, 10)
            except:
                pass

    #渲染用
    def updatePopWidgetPosition1(self,arg1=None,arg2=None,arg3=None):
        qt.QTimer.singleShot(60, self.runIpdatePop)

    def runIpdatePop(self):
        # print("pop1")
        layoutManager = cjyx.app.layoutManager()
        threeDWidget = layoutManager.threeDWidget(0).threeDView()
        print(threeDWidget.width,threeDWidget.size)
        if self.popwidget:
            moveX = threeDWidget.width - self.popwidget.width - 10.0
            try:
                self.popwidget.move(moveX, 10)
            except:
                pass
    #窗宽窗位切换使用
    def CancellHU(self,arg1=None,arg2=None,arg3=None):
        if(self.ui.pushButton_Threshold.checked):
            self.ui.pushButton_Threshold.click()


    def AddPopWidgetObserve(self):
        self.view1Observer = cjyx.util.getNode('vtkDMMLViewNode1').AddObserver(vtk.vtkCommand.ModifiedEvent,self.updatePopWidgetPosition1)


    def RemovePopWidgetObserve(self):
        cjyx.util.getNode('vtkDMMLViewNode1').RemoveObserver(self.view1Observer)


    # 外部更改节点名字
    def rename_seg(self, node):
        if node == None:
            return
        num = self.ui.SegmenttableWidget.rowCount
        segmentation = self.segmentationNode.GetSegmentation()
        ID = segmentation.GetSegmentIdBySegment(node)
        for i in range(num):
            if ID == self.ui.SegmenttableWidget.cellWidget(i, 0).objectName:

                if(self.ui.SegmenttableWidget.item(i, 1).text()==node.GetName()):
                    pass
                else:
                    self.ui.SegmenttableWidget.item(i, 1).setText(node.GetName())

    def GetUniqueNameByStringInSeg(self,name):
      segmentation = self.segmentationNode.GetSegmentation()
      number = segmentation.GetNumberOfSegments()
      names=[]
      for i in range(number-1):
        names.append(segmentation.GetNthSegment(i).GetName())
      if name in names:
        name1=name+"_1"
        i=2
        while name1 in names:
          name1=name+"_"+str(i)
          i+=1
        return name1
      return name



    def RenderingPreset(self,name):
        PresetPath = os.path.join(os.path.dirname(__file__), 'Resources/Preset')
        path = os.path.join(PresetPath,name+'.plist')
        print(path)
        with open(path, 'rb') as f:
            p = plistlib.load(f, fmt=plistlib.FMT_XML)

        volumeNode = cjyx.dmmlScene.GetFirstNodeByClass("vtkDMMLScalarVolumeNode")
        volRenLogic = cjyx.modules.volumerendering.logic()
        displayNode = volRenLogic.GetFirstVolumeRenderingDisplayNode(volumeNode)
        if not displayNode:
            displayNode = volRenLogic.CreateDefaultVolumeRenderingNodes(volumeNode)

        color_transfer = displayNode.GetVolumePropertyNode().GetVolumeProperty().GetRGBTransferFunction()
        color_transfer.RemoveAllPoints()
        curve_table = p['16bitClutCurves']
        color_table = p['16bitClutColors']

        for i, l in enumerate(curve_table):
            for j, lopacity in enumerate(l):
                gray_level = lopacity['x']
                r = color_table[i][j]['red']
                g = color_table[i][j]['green']
                b = color_table[i][j]['blue']
                color_transfer.AddRGBPoint(gray_level, r, g, b)

        opacity_transfer_func = displayNode.GetVolumePropertyNode().GetVolumeProperty().GetScalarOpacity()

        if p['advancedCLUT']:
            opacity_transfer_func.RemoveAllPoints()
            curve_table = p['16bitClutCurves']
            opacity_transfer_func.AddSegment(0, 0, 2 ** 16 - 1, 0)
            for i, l in enumerate(curve_table):
                for j, lopacity in enumerate(l):
                    gray_level = lopacity['x']
                    opacity = lopacity['y']
                    opacity_transfer_func.AddPoint(gray_level, opacity)
        else:
            opacity_transfer_func.RemoveAllPoints()
            ww = p['ww']
            wl = self.TranslateScale(p['wl'] - scale)
            l1 = wl - ww / 2.0
            l2 = wl + ww / 2.0
            wl = p['wl']
            opacity_transfer_func.RemoveAllPoints()
            opacity_transfer_func.AddSegment(0, 0, 2 ** 16 - 1, 0)
            k1 = 0.0
            k2 = 1.0
            opacity_transfer_func.AddPoint(l1, 0)
            opacity_transfer_func.AddPoint(l2, 1)

        displayNode.SetVisibility(True)






class NewRebuildLogic(ScriptedLoadableModuleLogic):
    """This class should implement all the actual
    computation done by your module.  The interface
    should be such that other python code can import
    this class and make use of the functionality without
    requiring an instance of the Widget.
    Uses ScriptedLoadableModuleLogic base class, available at:
    https://github.com/Cjyx/Cjyx/blob/master/Base/Python/cjyx/ScriptedLoadableModule.py
    """

    def __init__(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        """
        Called when the logic class is instantiated. Can be used for initializing member variables.
        """
        ScriptedLoadableModuleLogic.__init__(self)

    def setDefaultParameters(self, parameterNode):
        """
        Initialize parameter node with default settings.
        """
        if not parameterNode.GetParameter("Threshold"):
            parameterNode.SetParameter("Threshold", "100.0")
        if not parameterNode.GetParameter("Invert"):
            parameterNode.SetParameter("Invert", "false")

    def process(self, inputVolume, outputVolume, imageThreshold, invert=False, showResult=True):
        """
        Run the processing algorithm.
        Can be used without GUI widget.
        :param inputVolume: volume to be thresholded
        :param outputVolume: thresholding result
        :param imageThreshold: values above/below this threshold will be set to 0
        :param invert: if True then values above the threshold will be set to 0, otherwise values below are set to 0
        :param showResult: show output volume in slice viewers
        """




#
# NewRebuildTest
#

class NewRebuildTest(ScriptedLoadableModuleTest):
    """
    This is the test case for your scripted module.
    Uses ScriptedLoadableModuleTest base class, available at:
    https://github.com/Cjyx/Cjyx/blob/master/Base/Python/cjyx/ScriptedLoadableModule.py
    """

    def setUp(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        """ Do whatever is needed to reset the state - typically a scene clear will be enough.
        """
        cjyx.dmmlScene.Clear()

    def runTest(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        """Run as few or as many tests as needed here.
        """
        self.setUp()
        self.test_NewRebuild1()

    def test_NewRebuild1(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
        """ Ideally you should have several levels of tests.  At the lowest level
        tests should exercise the functionality of the logic with different inputs
        (both valid and invalid).  At higher levels your tests should emulate the
        way the user would interact with your code and confirm that it still works
        the way you intended.
        One of the most important features of the tests is that it should alert other
        developers when their changes will have an impact on the behavior of your
        module.  For example, if a developer removes a feature that you depend on,
        your test should break so they know that the feature is needed.
        """

        self.delayDisplay("Starting the test")

        # Get/create input data

        import SampleData
        registerSampleData()
        inputVolume = SampleData.downloadSample('NewRebuild1')
        self.delayDisplay('Loaded test data set')

        inputScalarRange = inputVolume.GetImageData().GetScalarRange()
        self.assertEqual(inputScalarRange[0], 0)
        self.assertEqual(inputScalarRange[1], 695)

        outputVolume = cjyx.dmmlScene.AddNewNodeByClass("vtkDMMLScalarVolumeNode")
        threshold = 100

        # Test the module logic

        logic = NewRebuildLogic()

        # Test algorithm with non-inverted threshold
        logic.process(inputVolume, outputVolume, threshold, True)
        outputScalarRange = outputVolume.GetImageData().GetScalarRange()
        self.assertEqual(outputScalarRange[0], inputScalarRange[0])
        self.assertEqual(outputScalarRange[1], threshold)

        # Test algorithm with inverted threshold
        logic.process(inputVolume, outputVolume, threshold, False)
        outputScalarRange = outputVolume.GetImageData().GetScalarRange()
        self.assertEqual(outputScalarRange[0], inputScalarRange[0])
        self.assertEqual(outputScalarRange[1], inputScalarRange[1])

        self.delayDisplay('Test passed')
