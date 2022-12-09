import os
import unittest
import logging
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
from slicer.util import VTKObservationMixin
import numpy as np
import math
import time
import threading
import socket

try:
  slicer.util.pip_install("pyserial")
  import serial
except:
  pass

#
# NoImage
#

class NoImage(ScriptedLoadableModule):

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "NoImage"  # TODO: make this more human readable by adding spaces
    self.parent.categories = ["Examples"]  # TODO: set categories (folders where the module shows up in the module selector)
    self.parent.dependencies = []  # TODO: add here list of module names that this module requires
    self.parent.contributors = ["John Doe (AnyWare Corp.)"]  # TODO: replace with "Firstname Lastname (Organization)"

   
class NoImageWidget(ScriptedLoadableModuleWidget, VTKObservationMixin):

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
    ScriptedLoadableModuleWidget.setup(self)

    # Load widget from .ui file (created by Qt Designer).
    # Additional widgets can be instantiated manually and added to self.layout.
    uiWidget = slicer.util.loadUI(self.resourcePath('UI/NoImage.ui'))
    self.layout.addWidget(uiWidget)
    self.ui = slicer.util.childWidgetVariables(uiWidget)

    # Set scene in MRML widgets. Make sure that in Qt designer the top-level qMRMLWidget's
    # "mrmlSceneChanged(vtkMRMLScene*)" signal in is connected to each MRML widget's.
    # "setMRMLScene(vtkMRMLScene*)" slot.
    uiWidget.setMRMLScene(slicer.mrmlScene)
    #----------------------------------------------------------------------------------------
    self.iconsPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons/NoImageIcon')
    self.FilePath = os.path.join(os.path.dirname(__file__), 'ssmdata')
    self.jiatiPath = os.path.join(os.path.dirname(__file__), '假体库')
    self.noimageWidget = slicer.util.findChild(slicer.util.mainWindow(),"NoImageWidget")
    self.FourWidget = slicer.util.findChild(slicer.util.mainWindow(),"widget")
    self.interactorNum = 0
    self.JingGu = 0
    self.TibiaJtSelectNum=0
    self.pyqt_data_x = []
    self.pyqt_data_y1 = []
    self.pyqt_data_y2 = []
    for i in range(141):
      self.pyqt_data_x.append(i-10)
      self.pyqt_data_y1.append(-5)
      self.pyqt_data_y2.append(5)
    # #设置骨骼参数表格自适应大小
    # self.ui.tableWidget.horizontalHeader().setSectionResizeMode(qt.QHeaderView.Stretch)
    # self.ui.tableWidget.verticalHeader().setSectionResizeMode(qt.QHeaderView.Stretch)
    # self.ui.TibiaTableWidget.horizontalHeader().setSectionResizeMode(qt.QHeaderView.Stretch)
    # self.ui.TibiaTableWidget.verticalHeader().setSectionResizeMode(qt.QHeaderView.Stretch)
    self.resizeEvent = ReSizeEvent()#自适应
    
    self.ui.PopupWidget.setVisible(False)
    self.ui.head1.setVisible(False)
    self.ui.OperationPlanWidget.setVisible(False)#手术规划每部分小界面
    self.onState()
    self.ui.OperationPlanWidget.setVisible(False)
    self.ui.ForceWidget.setVisible(False)

    #---------------初始化---------------------------------------------
    self.ui.Apply.connect('clicked(bool)',self.onApply)
    self.ui.StopSelect.connect('clicked(bool)', self.onStopSelect)
    #手术技术
    self.ui.CTMRI.toggled.connect(self.OperationTechnology)
    self.ui.Deformation.toggled.connect(self.OperationTechnology)
    #手术器械
    self.ui.FourAndOne.toggled.connect(self.OperationTool)
    self.ui.PSI.toggled.connect(self.OperationTool)
    self.ui.ZhiWei.toggled.connect(self.OperationTool)
    self.ui.ZhiXiang.toggled.connect(self.OperationTool)
    #手术顺序
    self.ui.TibiaFirst.toggled.connect(self.OperationOrder)
    self.ui.FemurFirst.toggled.connect(self.OperationOrder)
    #间隙平衡
    self.ui.JieGu.toggled.connect(self.OperationClearance)
    self.ui.RuanZuZhi.toggled.connect(self.OperationClearance)
    #-------------------前进后退-------------------------------------
    backPath = os.path.join(self.iconsPath, '后退.png')
    self.ui.BackToolButton.setIcon(qt.QIcon(backPath))
    #self.ui.BackToolButton.setEnabled(False)
    forwardPath = os.path.join(self.iconsPath, '前进.png')
    self.ui.ForwardToolButton.setIcon(qt.QIcon(forwardPath))
    #self.ui.ForwardToolButton.setEnabled(False)
    self.currentModel = 0
    # self.WidgetList = [self.ui.InitWidget,self.ui.SystemPrepareWidget,self.ui.SSMWidget,self.ui.SSMWidget,self.ui.FemurToolWidget,self.ui.TibiaToolWidget,self.ui.ReportToolWidget,self.ui.NavigationToolWidget]
    # self.LabelList = ["无","初始化","系统准备","股骨配准","胫骨配准","规划","规划","报告","导航","无"]
    self.WidgetList = [self.ui.InitWidget,self.ui.SystemPrepareWidget,self.ui.SSMWidget,self.ui.SSMWidget,self.ui.FemurToolWidget,self.ui.TibiaToolWidget,self.ui.NavigationToolWidget]
    self.LabelList = ["无","初始化","系统准备","股骨配准","胫骨配准","手术规划","手术规划","导航","无"]
    self.WidgetShow(self.currentModel)
    self.ui.ReportToolWidget.setVisible(False)
    self.ui.BackToolButton.connect('clicked(bool)',self.onBackToolButton)
    self.ui.ForwardToolButton.connect('clicked(bool)',self.onForwardToolButton)

    #--------------------------系统准备-----------------------------------------------------
    #powerOnButton 手术准备 positionButton 系统准备 signInButton 工具设置 
    #femurSystemButton 股骨侧 tibiaSystemButton 胫骨侧 test 校准检测
    self.HideAllSystemWidget(self.ui.SystemWidget)
    self.ui.powerOnButton.clicked.connect(lambda:self.onSystemButton(self.ui.powerOnButton))
    self.ui.positionButton.clicked.connect(lambda:self.onSystemButton(self.ui.positionButton))
    self.ui.signInButton.clicked.connect(lambda:self.onSystemButton(self.ui.signInButton))
    self.ui.femurSystemButton.clicked.connect(lambda:self.onSystemButton(self.ui.femurSystemButton))
    self.ui.tibiaSystemButton.clicked.connect(lambda:self.onSystemButton(self.ui.tibiaSystemButton))
    self.ui.testButton.clicked.connect(lambda:self.onSystemButton(self.ui.testButton))
    self.ui.SystemConfirm.clicked.connect(self.onSystemConfirm)
    self.ui.SystemReset.clicked.connect(self.onSystemReset)
    self.buttonMask("powerOn",self.ui.powerOnButton)
    self.buttonMask("position",self.ui.positionButton)
    self.buttonMask("signIn",self.ui.signInButton)
    self.buttonMask("femurSystem",self.ui.femurSystemButton)
    self.buttonMask("tibiaSystem",self.ui.tibiaSystemButton)
    self.buttonMask("test",self.ui.testButton)
    self.ui.SystemImage.setPixmap(qt.QPixmap(os.path.join(self.iconsPath,'SystemImage.png')))
    self.ui.SystemImage.setScaledContents(True)


    #------------配准（ssm模型）------------------------------------------------------------
    self.ui.Switch.connect('clicked(bool)',self.onSwitch)#显示切换
    self.ui.Confirm1.connect('clicked(bool)', self.onConfirm2)#确认
    self.ui.Select1.connect('clicked(bool)', self.onSelect1) #选取标志点
    self.ui.PointReset.clicked.connect(self.onPointReset)#重置
    self.ui.StopSelect.clicked.connect(self.onStopSelect)#停止
    self.ui.NextArea.clicked.connect(self.onNextArea)#下一区域
    self.SwitchState = 1
    femurPointCheckBox = [self.ui.femurPoint1,self.ui.femurPoint2,self.ui.femurPoint3,self.ui.femurPoint4,self.ui.femurPoint5,
                          self.ui.femurPoint6,self.ui.femurPoint7,self.ui.femurPoint8,self.ui.femurPoint9,self.ui.femurPoint10,
                          self.ui.femurPoint11,self.ui.femurPoint12,self.ui.femurPoint13,self.ui.femurPoint14]
    tibiaPointCheckBox = [self.ui.tibiaPoint1,self.ui.tibiaPoint2,self.ui.tibiaPoint3,self.ui.tibiaPoint4,self.ui.tibiaPoint5,
                          self.ui.tibiaPoint6,self.ui.tibiaPoint7,self.ui.tibiaPoint8,self.ui.tibiaPoint9]
    for i in range(0,len(femurPointCheckBox)):
      femurPointCheckBox[i].setEnabled(False)
    for i in range(0,len(tibiaPointCheckBox)):
      tibiaPointCheckBox[i].setEnabled(False)
    
    self.ui.GuGuTou.clicked.connect(self.onGuGuTou)#选取股骨头球心
    self.ui.GuGuTouConfirm.clicked.connect(self.onGuGuTouConfirm)#股骨头球心确认
    self.ui.HPoint.clicked.connect(self.onHPoint)#选取H点


    #---------------股骨规划-------------------------------------------------------
    self.ui.Parameter.connect('clicked(bool)', self.onParameter)#骨骼参数按钮
    self.ui.Adjustment.connect('clicked(bool)', self.onAdjustment)#截骨调整按钮
    self.ui.ViewChoose.connect('clicked(bool)', self.onViewSelect)#视图选择按钮
    self.ui.Reset.connect('clicked(bool)', self.onReset)#重置按钮
    self.ui.ForceLine.connect('clicked(bool)',self.onForceLine)#显示力线
    self.ui.ForceConfirm.connect('clicked(bool)',self.onForceConfirm)#力线确认
    #------------------------胫骨规划------------------------------------------------
    #self.ui.Parameter2.connect('clicked(bool)', self.onParameter2)#骨骼参数按钮
    self.ui.Parameter2.connect('clicked(bool)', self.onParameter)
    self.ui.Adjustment2.connect('clicked(bool)', self.onAdjustment2)#截骨调整按钮
    #self.ui.ViewChoose2.connect('clicked(bool)', self.onTibiaViewSelect)#视图选择按钮
    self.ui.ViewChoose2.connect('clicked(bool)', self.onViewSelect)#视图选择按钮
    self.ui.ReSet2.connect('clicked(bool)', self.onReset)#重置按钮
    self.ui.ForceLine2.connect('clicked(bool)',self.onForceLine)#显示力线
    #--------------------------报告---------------------------------------------
    self.ui.JieTu.connect('clicked(bool)', self.onJieTu)#调直按钮
    self.ui.CTReport.connect('clicked(bool)', self.onCTReport)#CT按钮
    self.ui.MRIReport.connect('clicked(bool)', self.onMRIReport)#MRI按钮
    self.ui.path.setText('D:/Data')
    self.ui.pathButton.connect('clicked(bool)', self.onPath)
    self.ui.ConfirmReport.connect('clicked(bool)', self.onConfirmReport)
    #--------------------------------------------------------------------------
    # head1按钮信号与槽
    self.ui.JiaTiButton.connect('clicked(bool)', self.onJiaTiButton)#假体按钮
    self.ui.BoneButton.connect('clicked(bool)', self.onBoneButton)#截骨面按钮
    self.ui.MarkerButton.connect('clicked(bool)', self.onMarkerButton)#标志点按钮
    self.ui.TransparentButton.connect('clicked(bool)', self.onTransparentButton)#透明显示按钮
    self.ui.FemurSwitch.connect('clicked(bool)',self.onFemurSwitch)#切换到股骨截骨调整
    self.ui.TibiaSwitch.connect('clicked(bool)',self.onTibiaSwitch)#切换到胫骨截骨调整
    self.ui.FemurR.connect('currentIndexChanged(int)',self.onFemurR)#股骨假体右侧ComboBox
    self.ui.FemurL.connect('currentIndexChanged(int)',self.onFemurL)#股骨假体左侧ComboBox
    self.ui.TibiaJiaTi.connect('currentIndexChanged(int)',self.onTibiaJiaTi)#胫骨假体ComboBox
    self.ui.TibiaShowHide.connect('clicked(bool)',self.onTibiaShowHide)#胫骨近端是否显示
    self.ui.FemurShowHide.connect('clicked(bool)',self.onFemurShowHide)#股骨远端是否显示
    #-----------------------------Graph功能-------------------------------------------
    self.ui.PopupImage.connect('clicked(bool)',self.PopupGraph)#弹出图像
    self.ui.ClearImage.connect('clicked(bool)',self.ClearGraph)#清空图像
    self.ui.DrawImage.connect('clicked(bool)',self.DrawGraph)#绘制图像
    self.ui.RecordImage.connect('clicked(bool)',self.RecordGraph)#记录图像
    #------------------------------导航----------------------------------------
    self.ui.NavigationSwitch.connect('clicked(bool)',self.onNavigationSwitch)#导航显示切换
    #工具校准
    self.ui.DriveJZ.connect('clicked(bool)',self.onDriveJZ)#电机校准按钮
    self.ui.Marker.connect('clicked(bool)',self.onMarker)#电机刀槽标识点
    #自校准
    self.ui.AutoJiaoZhun.connect('clicked(bool)',self.onAutoJiaoZhun)#电机自校准
    self.ui.DCMarker1.connect('clicked(bool)',self.onDCMarker1)
    self.ui.DCMarker2.connect('clicked(bool)',self.onDCMarker2)
    self.ui.DCMarker3.connect('clicked(bool)',self.onDCMarker3)
    self.ui.DCMarker4.connect('clicked(bool)',self.onDCMarker4)
    self.ui.DCMarker2.setEnabled(False)
    self.ui.DCMarker3.setEnabled(False)
    self.ui.DCMarker4.setEnabled(False)

    self.ui.FYAngleJiaoZhun.connect('clicked(bool)',self.onFYAngleJiaoZhun)#电机俯仰角校准
    #self.ui.OpenTool.connect('clicked(bool)', self.onOpenTool)#开始校准
    #self.ui.ConfirmTool.connect('clicked(bool)', self.onConfirmTool)#确认校准
    
    self.ui.FemurQG.connect('clicked(bool)',self.onFemurQG)#股骨切割按钮
    self.ui.TibiaQG.connect('clicked(bool)',self.onTibiaQG)#胫骨切割按钮

    #self.ui.InitDJ.connect('clicked(bool)',self.onInit)#初始化计算角度
    #切割
    self.ui.FirstQG.connect('clicked(bool)',self.onFirstQG)#第一刀
    self.ui.SecondQG.connect('clicked(bool)',self.onSecondQG)
    self.ui.ThirdQG.connect('clicked(bool)',self.onThirdQG)
    self.ui.FourthQG.connect('clicked(bool)', self.onFourthQG)
    self.ui.FifthQG.connect('clicked(bool)',self.onFifthQG)
    self.ui.QGReSet.connect('clicked(bool)',self.onQGReSet)
    #切割预览
    self.ui.FirstPreview.connect('clicked(bool)', self.onFirstPreview)
    self.ui.SecondPreview.connect('clicked(bool)', self.onSecondPreview)
    self.ui.ThirdPreview.connect('clicked(bool)', self.onThirdPreview)
    self.ui.FourthPreview.connect('clicked(bool)', self.onFourthPreview)
    self.ui.FifthPreview.connect('clicked(bool)', self.onFifthPreview)
    self.ui.PreviewReSet.connect('clicked(bool)', self.onPreviewReSet)
    self.DHButton()

    #添加logo图片
    Logo = slicer.util.findChild(slicer.util.mainWindow(),"Logo")
    pixmap = qt.QPixmap(self.iconsPath+'/Logo.png')
    Logo.setPixmap(pixmap)
  
    self.ui.FemurSwitch.setIcon(qt.QIcon(self.iconsPath+'/FemurSwitch.png'))
    self.ui.TibiaSwitch.setIcon(qt.QIcon(self.iconsPath+'/TibiaSwitch.png'))
    try:
      self.onstartDJ()
      print('###########蓝牙串口已开启###########')
    except:
      pass
    self.ifsend03=0

  #---------------前进后退----------------------------------------------- 
  def onForwardToolButton(self):
    #self.ui.ForwardToolButton.setEnabled(False)
    if self.currentModel == 6:
      return
    
    self.currentModel += 1
    if self.currentModel == 5:
      self.currentModel += 1

    self.WidgetShow(self.currentModel)

  def WidgetShow(self,index):
    for i in range (0,len(self.ui.GraphImage.children())):
      a = self.ui.GraphImage.children()[-1]
      a.delete() 
    self.ui.PopupWidget.setVisible(False)
    self.ui.head1.setVisible(False)
    self.ui.OperationPlanWidget.setVisible(False)#手术规划每部分小界面
    self.ui.NavigationWidget.setVisible(False)
    self.ui.Graph.setVisible(False)
    
    for i in range(0,len(self.WidgetList)):
      self.WidgetList[i].setVisible(False)
    self.WidgetList[index].setVisible(True)
    self.ui.BackToolButton.setToolTip(self.LabelList[index])
    self.ui.ModuleName.setText(self.LabelList[index+1])
    self.ui.ForwardToolButton.setToolTip(self.LabelList[index+2])
    
    if index == 0:#初始化
      for i in range(0,len(self.noimageWidget.findChildren("QLabel"))):
        self.noimageWidget.findChildren("QLabel")[-1].delete()
      
      self.FourImage(True)

    if index == 1:#系统准备
      self.FourImage(False)
      for i in range(0,len(self.noimageWidget.findChildren("QLabel"))):
        self.noimageWidget.findChildren("QLabel")[-1].delete()
      self.ui.powerOnButton.click()
      self.PngLabel = qt.QLabel(self.noimageWidget)
      self.PngLabel.resize(self.noimageWidget.width,self.noimageWidget.height)
      self.PngLabel.setScaledContents(True)
      self.PngLabel.setStyleSheet("QLabel{background-color:transparent;}")
      self.pixmap = qt.QPixmap(self.iconsPath+'/background.png')        
      self.PngLabel.setPixmap(self.pixmap)   
      self.PngLabel.show()
      self.ui.ForwardToolButton.setEnabled(True)

    if index == 2:#股骨配准
      self.ui.tibiaPointWidget.setVisible(False)
      self.ui.femurPointWidget.setVisible(True)
      self.EnterSet()
      self.ui.femurWidget2.setVisible(True)
      self.onFemurRadioButton()
      self.FemurOrTibia()
      s1 = 0
      s2 = 2
      s3 = 0
      s4 = 0
      s5 = 1
      # 股骨或者胫骨图片
      s6 = 1
      s7 = 0
      s8 = '0@\n'
      self.ser1.write(f'{s1},{s2},{s3},{s4},{s5},{s6},{s7},{s8}'.encode())
      print('已发送', f'{s1},{s2},{s3},{s4},{s5},{s6},{s7},{s8}')
    if index == 3: #胫骨配准
      self.ui.femurPointWidget.setVisible(False)
      self.ui.tibiaPointWidget.setVisible(True)
      self.EnterSet()
      self.onTibiaRadioButton()
      self.FemurOrTibia()
      s1 = 0
      s2 = 2
      s3 = 0  
      s4 = 1
      s5 = 1
      s6 = 1
      s7 = 0
      s8 = '0@\n'
      self.ser1.write(f'{s1},{s2},{s3},{s4},{s5},{s6},{s7},{s8}'.encode())
      print('已发送', f'{s1},{s2},{s3},{s4},{s5},{s6},{s7},{s8}')
    if index == 4:#股骨规划
      s1 = 0
      s2 = 3
      s3 = 0  
      s4 = 0
      s5 = 0
      s6 = 0
      s7 = 0
      s8 = '0@\n'
      self.ser1.write(f'{s1},{s2},{s3},{s4},{s5},{s6},{s7},{s8}'.encode())
      print('已发送', f'{s1},{s2},{s3},{s4},{s5},{s6},{s7},{s8}')
      self.StopClosestLine()#隐藏胫骨误差线
      self.FemurButtonChecked(None) 
      self.buildPointsInFemur() 

      for i in range (0,len(self.noimageWidget.findChildren('QLabel'))):
        self.noimageWidget.findChildren('QLabel')[-1].delete()
    if index == 5:#胫骨规划
      s1 = 0
      s2 = 3
      s3 = 0  
      s4 = 1
      s5 = 0
      s6 = 0
      s7 = 0
      s8 = '0@\n'
      self.ser1.write(f'{s1},{s2},{s3},{s4},{s5},{s6},{s7},{s8}'.encode())
      print('已发送', f'{s1},{s2},{s3},{s4},{s5},{s6},{s7},{s8}')
      self.HidePart()
      self.TibiaButtonChecked(None)
      self.ShowNode('Tibia')
    if index == 6:#导航
      s1 = 0
      s2 = 3
      s3 = 1  
      s4 = 2
      s5 = 0
      s6 = 0
      s7 = 0
      s8 = '0@\n'
      self.ser1.write(f'{s1},{s2},{s3},{s4},{s5},{s6},{s7},{s8}'.encode())
      print('已发送', f'{s1},{s2},{s3},{s4},{s5},{s6},{s7},{s8}')
      # try:
      #   try:
      #     self.hideInformation()
      #   except Exception as e:
      #     print(e)
      #   self.HideAll()
      #   self.HidePart()
      #   self.ShowNode('股骨切割')
      #   self.jiatiload.SetDisplayVisibility(True)
      #   self.ShowNode('胫骨切割')
      #   self.TibiaJiaTiload.SetDisplayVisibility(True)
      #   self.Camera1(self.view1)
      #   self.Camera2(self.view2)
      #   self.Camera3(self.view3)
      #   slicer.app.layoutManager().setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutFourOverFourView)
      #   view4=slicer.app.layoutManager().threeDWidget('View4').threeDView()
      #   self.TCamera3(view4)
      #   self.loadChenDian()
      #   self.showHide()
      # except:
      #     pass
      for i in range (0,len(self.noimageWidget.findChildren('QLabel'))):
        self.noimageWidget.findChildren('QLabel')[-1].delete()
      self.ThreeDViewAndImageWidget(0)
      slicer.modules.noimageoperationimage.widgetRepresentation().hide()
      self.SwitchState = 1

  def onBackToolButton(self):
    if self.currentModel == 0:
      return
    self.currentModel -= 1
    if self.currentModel == 5:
      self.currentModel -=1
    self.WidgetShow(self.currentModel)

  def handleCalc_do1(self,buf):
    buf = buf.strip('@').strip(',')
    buf_list = buf.split('@')
    for i in range(len(buf_list)):
      buf_list1 = buf_list[i].strip(',').split(',')
      if len(buf_list1) == 17:
        name = buf_list1[0]
        buf_list1.pop(0)
        trans = np.zeros([4, 4])
        for i in range(4):
          for j in range(4):
            trans[i][j] = buf_list1[i * 4 + j]
        self.handleData1(name,trans)
        print(name)

  def dealigt(self,asd=None,asdasd=None):
    # 创建客户端套接字
    sk = socket.socket()
    # 尝试连接服务器
    sk.connect(('192.168.3.99', 8898))
    while True:
      # 信息接收
      ret = sk.recv(10240)
      ret = ret.decode()
      # print(ret)
      self.handleCalc_do1(ret)
      time.sleep(0.014)

  def handleData1(self, name, trans):
    try:
      slicer.util.getNode(name).SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(trans))
    except:
      transnode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLLinearTransformNode', name)
      transnode.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(trans))

  #-----------------初始化-----------------------------------------------
  def onApply(self):
    self._received_thread_ = threading.Thread(target=self.dealigt, args=(self,))
    # print("thread")
    self._is_running_ = True
    # print("thread1")
    self._received_thread_.setDaemon(True)
    # print("thread2")
    self._received_thread_.setName("Seria124")
    self._received_thread_.start()
    print('开始监听信号')
    self.ui.ForwardToolButton.setEnabled(True)
    self.onPrepare()
    self.ifsend03=1
    message = qt.QMessageBox(qt.QMessageBox.Information,'提示',"初始化成功！",qt.QMessageBox.Ok)
    message.button(qt.QMessageBox().Ok).setText('确定')
    message.exec()
    self.ui.tool1.setEnabled(True)
    s1 = 0
    s2 = 1
    s3 = 1
    s4 = 2
    s5 = 0
    s6 = 0
    s7 = 0
    s8 = f'{int(self.ui.tool2.enabled)}' + f'{int(self.ui.tool3.enabled)}' + f'{int(self.ui.tool4.enabled)}' + f'{int(self.ui.tool5.enabled)}' + f'{int(self.ui.tool6.enabled)}'+'@\n'
    self.ser1.write(f'{s1},{s2},{s3},{s4},{s5},{s6},{s7},{s8}'.encode())
    print('已发送',f'{s1},{s2},{s3},{s4},{s5},{s6},{s7},{s8}'.encode())



  def onPrepare(self):
    slicer.util.loadScene(self.FilePath+'/cj/cj.mrml')
    Node1 = slicer.mrmlScene.GetFirstNodeByClass('vtkMRMLIGTLConnectorNode')
    Node1.Start()
    w = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLFiducialRegistrationWizardNode")
    w = slicer.util.getNode('FiducialRegistrationWizard')
    w.SetRegistrationModeToRigid()
    w.SetUpdateModeToManual()
    FemurToTool1node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", 'FromToTo_femur')
    tibiaToTool2node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", 'FromToTo_tibia')
    w.SetOutputTransformNodeId(FemurToTool1node.GetID())
    From1node = slicer.util.getNode('From')
    w.SetAndObserveFromFiducialListNodeId(From1node.GetID())
    To1node = slicer.util.getNode('To')
    w.SetAndObserveToFiducialListNodeId(To1node.GetID())
    probeToTransformNode = slicer.util.getNode("StylusTipToStylus")
    w.SetProbeTransformFromNodeId(probeToTransformNode.GetID())

    StylusNode = slicer.util.getNode('StylusToTracker')
    KnifeNode = slicer.util.getNode('KnifeToTracker')
    FemurNode = slicer.util.getNode('DianjiToTracker1')
    TibiaNode = slicer.util.getNode('TibiaToTracker')
    self.transform1 = np.array([[0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0]])
    self.transform2 = self.transform1
    self.transform3 = self.transform1
    self.transform4 = self.transform1
    self.count = 0
    self.count1 = 0
    # 所有观察者调用同一个函数
    StylusNode.AddObserver(slicer.vtkMRMLTransformNode.TransformModifiedEvent, self.StateChange)
    KnifeNode.AddObserver(slicer.vtkMRMLTransformNode.TransformModifiedEvent, self.StateChange)
    FemurNode.AddObserver(slicer.vtkMRMLTransformNode.TransformModifiedEvent, self.StateChange)
    TibiaNode.AddObserver(slicer.vtkMRMLTransformNode.TransformModifiedEvent, self.StateChange)

  #针刀槽股骨胫骨状态图标(在setup中调用)
  def onState(self):

    #为按钮设置图标
    self.ui.tool1.setIcon(qt.QIcon(self.iconsPath+'/TwoEye.png'))
    self.ui.tool1.setToolTip('光学指示灯')
    self.ui.tool2.setIcon(qt.QIcon(self.iconsPath+'/needle.png'))
    self.ui.tool2.setToolTip('针指示灯')
    self.ui.tool3.setIcon(qt.QIcon(self.iconsPath+'/knife.png'))
    self.ui.tool3.setToolTip('刀槽指示灯')
    self.ui.tool4.setIcon(qt.QIcon(self.iconsPath+'/femur.png'))
    self.ui.tool4.setToolTip('股骨指示灯')
    self.ui.tool5.setIcon(qt.QIcon(self.iconsPath+'/tibia.png'))
    self.ui.tool5.setToolTip('胫骨指示灯')   
    self.ui.tool6.setIcon(qt.QIcon(self.iconsPath+'/Plane.png'))
    self.ui.tool6.setToolTip('截骨面指示灯')  
   
    self.ui.tool1.setEnabled(False)
    self.ui.tool2.setEnabled(False)
    self.ui.tool3.setEnabled(False)
    self.ui.tool4.setEnabled(False)
    self.ui.tool5.setEnabled(False)
    self.ui.tool6.setEnabled(False)

  def StateChange(self,transformNode,unusedArg2=None, unusedArg3=None):
    self.count += 1
    if self.count%50==0:
      self.count1 += 1
      self.count=1
      StylusNode=slicer.util.getNode('StylusToTracker')
      KnifeNode = slicer.util.getNode('KnifeToTracker')
      FemurNode = slicer.util.getNode('DianjiToTracker1')
      TibiaNode = slicer.util.getNode('TibiaToTracker')
      transform1 = slicer.util.arrayFromTransformMatrix(StylusNode)
      transform2 = slicer.util.arrayFromTransformMatrix(KnifeNode)
      transform3 = slicer.util.arrayFromTransformMatrix(FemurNode)
      transform4 = slicer.util.arrayFromTransformMatrix(TibiaNode)
      if (transform1 == self.transform1).all():
        self.ui.tool2.setEnabled(False)        
      else:
        self.ui.tool2.setEnabled(True)
        self.transform1 = transform1

      if (transform2 == self.transform2).all():
        self.ui.tool3.setEnabled(False)        
      else:  
        self.ui.tool3.setEnabled(True)
        self.transform2 = transform2
      if (transform3 == self.transform3).all():
        self.ui.tool4.setEnabled(False)
      else:
        self.ui.tool4.setEnabled(True)
        self.transform3 = transform3

      if (transform4 == self.transform4).all():
        self.ui.tool5.setEnabled(False)
      else:       
        self.ui.tool5.setEnabled(True)
        self.transform4 = transform4
      #向小屏幕发送外翻角屈膝角以及工具状态
      s1 = 1
      s2 = 0
      try:
        s3 = round(self.currentY,2)  # 外翻
        s4 = round(self.currentX,2)   # 屈膝
      except:
        s3 = 0  # 外翻
        s4 = 0  # 屈膝

      s5 = 0
      s6 = 0
      s7 = 0
      s8 = f'{int(self.ui.tool2.enabled)}' + f'{int(self.ui.tool3.enabled)}' + f'{int(self.ui.tool4.enabled)}' + f'{int(self.ui.tool5.enabled)}' + f'{int(self.ui.tool6.enabled)}'+'@\n'
      self.ser1.write(f'{s1},{s2},{s3},{s4},{s5},{s6},{s7},{s8}'.encode())
      #print('已发送', f'{s1},{s2},{s3},{s4},{s5},{s6},{s7},{s8}')

 
  def OperationTechnology(self):
    try:
      self.Image1.findChild("QLabel").delete()
    except:
      pass
    PngLabel = qt.QLabel(self.Image1)
    PngLabel.resize(self.Image1.width,self.Image1.height)
    PngLabel.setScaledContents(True)
    PngLabel.setStyleSheet("QLabel{background-color:transparent;}")
    if self.ui.CTMRI.checked:
      pixmap = qt.QPixmap(self.iconsPath+'/CTMRI.png') 
    elif self.ui.Deformation.checked: 
      pixmap = qt.QPixmap(self.iconsPath+'/GuBianXing.png') 
    
    pixmap = pixmap.scaled(1024,1024,qt.Qt.KeepAspectRatio,qt.Qt.SmoothTransformation)
    PngLabel.setPixmap(pixmap)  
    PngLabel.resize(719,449) 
    PngLabel.show()

  def OperationTool(self):
    try:
      self.Image2.findChild("QLabel").delete()
    except:
      pass
    PngLabel = qt.QLabel(self.Image2)
    PngLabel.resize(self.Image2.width,self.Image2.height)
    PngLabel.setScaledContents(True)
    PngLabel.setStyleSheet("QLabel{background-color:transparent;}")
    if self.ui.FourAndOne.checked:
      pixmap = qt.QPixmap(self.iconsPath+'/FourAndOne.png')  
    elif self.ui.PSI.checked:
      pixmap = qt.QPixmap(self.iconsPath+'/PSI.png')  
    elif self.ui.ZhiWei.checked:
      pixmap = qt.QPixmap(self.iconsPath+'/ZhiWei.png')  
    elif self.ui.ZhiXiang.checked:
      pixmap = qt.QPixmap(self.iconsPath+'/ZhiXiang.png')  

    pixmap = pixmap.scaled(1024,1024,qt.Qt.KeepAspectRatio,qt.Qt.SmoothTransformation)
    PngLabel.setPixmap(pixmap)  
    PngLabel.resize(719,449) 
    PngLabel.show()

  def OperationOrder(self):
    try:
      self.Image3.findChild("QLabel").delete()
    except:
      pass
    PngLabel = qt.QLabel(self.Image3)
    PngLabel.resize(self.Image3.width,self.Image3.height)
    PngLabel.setScaledContents(True)
    PngLabel.setStyleSheet("QLabel{background-color:transparent;}")
    pixmap = qt.QPixmap(self.iconsPath+'/PSI.png')   
    if self.ui.TibiaFirst.checked:
      pixmap = qt.QPixmap(self.iconsPath+'/TibiaFirst.png') 
    elif self.ui.FemurFirst.checked:
      pixmap = qt.QPixmap(self.iconsPath+'/FemurFirst.png')
    PngLabel.setPixmap(pixmap)   
    PngLabel.resize(719,449)
    PngLabel.show()

  def OperationClearance(self):    
    try:
      self.Image4.findChild("QLabel").delete()
    except:
      pass
    PngLabel = qt.QLabel(self.Image4)
    PngLabel.resize(self.Image4.width,self.Image4.height)
    PngLabel.setScaledContents(True)
    PngLabel.setStyleSheet("QLabel{background-color:transparent;}")
    if self.ui.JieGu.checked:
      pixmap = qt.QPixmap(self.iconsPath+'/JieGu.png')   
    elif self.ui.RuanZuZhi.checked:
      pixmap = qt.QPixmap(self.iconsPath+'/RuanZuZhi.png')   
    
    PngLabel.setPixmap(pixmap)  
    PngLabel.resize(719,449) 
    PngLabel.show()

  #四张图片是否显示
  def FourImage(self,bool):    
    self.Image1 = slicer.util.findChild(slicer.util.mainWindow(),"Image1")
    self.Image2 = slicer.util.findChild(slicer.util.mainWindow(),"Image2")
    self.Image3 = slicer.util.findChild(slicer.util.mainWindow(),"Image3")
    self.Image4 = slicer.util.findChild(slicer.util.mainWindow(),"Image4")
    self.Image1.setVisible(bool)
    self.Image2.setVisible(bool)
    self.Image3.setVisible(bool)
    self.Image4.setVisible(bool)

  #------------------------------系统准备--------------------------------
  def HideAllSystemWidget(self,widget):
    self.ui.powerOnWidget.setVisible(False)
    self.ui.positionWidget.setVisible(False)
    self.ui.signInWidget.setVisible(False)
    self.ui.femurSystemWidget.setVisible(False)
    self.ui.tibiaSystemWidget.setVisible(False)
    self.ui.testWidget.setVisible(False)
    widget.setVisible(True)
  
  def onSystemButton(self,button):
    #手术室准备->系统准备->工具->股骨->胫骨->检测
    self.ui.powerOnButton.setChecked(False)
    self.ui.positionButton.setChecked(False)
    self.ui.femurSystemButton.setChecked(False)
    self.ui.tibiaSystemButton.setChecked(False)
    self.ui.testButton.setChecked(False)
    self.ui.signInButton.setChecked(False)
    self.ui.SystemTool.setVisible(False)
    button.setChecked(True)
    if button == self.ui.powerOnButton:
      self.HideAllSystemWidget(self.ui.powerOnWidget)
      self.setSystemCheckBoxState(self.ui.powerOn1)
      self.ui.SystemTool.setVisible(True)
      self.ui.SystemConfirm.setEnabled(True)
      self.powerOnNum = 0
    elif button == self.ui.positionButton:
      self.HideAllSystemWidget(self.ui.positionWidget)
    elif button == self.ui.signInButton:
      self.HideAllSystemWidget(self.ui.signInWidget)
      self.ui.SystemTool.setVisible(True)
      self.ui.SystemConfirm.setEnabled(True)
      self.setSystemCheckBoxState(self.ui.test1)
      self.testNum = 0

    elif button == self.ui.femurSystemButton:
      self.HideAllSystemWidget(self.ui.femurSystemWidget)
    elif button == self.ui.tibiaSystemButton:
      self.HideAllSystemWidget(self.ui.tibiaSystemWidget)
    elif button == self.ui.testButton:
      self.HideAllSystemWidget(self.ui.testWidget)

  def onSystemConfirm(self):
    PowerOnCheckBox = [self.ui.powerOn1,self.ui.powerOn2,self.ui.powerOn3]
    TestCheckBox = [self.ui.test1,self.ui.test2,self.ui.test3,self.ui.test4,self.ui.test5]
    if self.ui.powerOnButton.checked:
      PowerOnCheckBox[self.powerOnNum].setChecked(True)
      if self.powerOnNum == 2:
        self.ui.powerOn3.setStyleSheet("None")
        self.ui.SystemConfirm.setEnabled(False)
      else:
        self.setSystemCheckBoxState(PowerOnCheckBox[self.powerOnNum+1])
        self.powerOnNum += 1
    elif self.ui.signInButton.checked:
      TestCheckBox[self.testNum].setChecked(True)
      if self.testNum == 4:
        self.ui.SystemConfirm.setEnabled(False)
        self.ui.test5.setStyleSheet("None")
      else:
        self.setSystemCheckBoxState(TestCheckBox[self.testNum+1])
        self.testNum += 1

  def onSystemReset(self):
    PowerOnCheckBox = [self.ui.powerOn1,self.ui.powerOn2,self.ui.powerOn3]
    TestCheckBox = [self.ui.test1,self.ui.test2,self.ui.test3,self.ui.test4,self.ui.test5]
    if self.ui.powerOnButton.checked:
      for i in range(0,len(PowerOnCheckBox)):
        PowerOnCheckBox[i].setChecked(False)

      self.setSystemCheckBoxState(self.ui.powerOn1)
      self.ui.SystemConfirm.setEnabled(True)
      self.powerOnNum = 0
    elif self.ui.signInButton.checked:
      for i in range(0,len(TestCheckBox)):
        TestCheckBox[i].setChecked(False)
      self.ui.SystemConfirm.setEnabled(True)
      self.setSystemCheckBoxState(self.ui.test1)
      self.testNum = 0
  #为系统准备按钮添加Mask  
  def buttonMask(self,name,button):    
    abc=qt.QPixmap(os.path.join(self.iconsPath,name+".png"))
    button.setMask(abc.mask())
    icon1A = qt.QIcon()
    icons1APath = os.path.join(self.iconsPath, name+".png")
    icon1A.addPixmap(qt.QPixmap(icons1APath))
    button.setIcon(icon1A)
    button.setFlat(True)
    print(os.path.join(self.iconsPath,name+".png"))
  
  def setSystemCheckBoxState(self,checkBox):
    for i in range (0,len(self.ui.SystemWidget.findChildren("QCheckBox"))):
      self.ui.SystemWidget.findChildren("QCheckBox")[i].setStyleSheet("None")
    checkBox.setStyleSheet("background:#515151;color:#7bcd27;font-weight:bold;")

  def setCheckBoxState(self,checkBox,Label):
    for i in range (0,len(self.ui.femurPointWidget.findChildren("QCheckBox"))):
      self.ui.femurPointWidget.findChildren("QCheckBox")[i].setStyleSheet("None")
    for i in range (0,len(self.ui.tibiaPointWidget.findChildren("QCheckBox"))):
      self.ui.tibiaPointWidget.findChildren("QCheckBox")[i].setStyleSheet("None")

    for i in range (0,len(self.ui.femurPointWidget.findChildren("QLabel"))):
      self.ui.femurPointWidget.findChildren("QLabel")[i].setStyleSheet("None")
    for i in range (0,len(self.ui.tibiaPointWidget.findChildren("QCheckBox"))):
      self.ui.tibiaPointWidget.findChildren("QLabel")[i].setStyleSheet("None")

    checkBox.setStyleSheet("background:#515151;color:#7bcd27;font-weight:bold;")   
    Label.setStyleSheet("background:#515151;color:#7bcd27;font-weight:bold;")   

#-----------------ssm模型（股骨配准 And 胫骨配准）--------------------

  def EnterSet(self):
    self.ui.SelectWidget.setVisible(False)
    self.ui.NextArea.setVisible(False)
    self.ui.StopSelect.setVisible(False)
    self.ui.Confirm1.setEnabled(False)
    self.ui.PointReset.setEnabled(False)
    self.ui.SingleSelect.setChecked(True)
    self.ui.femurWidget2.setVisible(False)

  #点击前进到股骨配准和胫骨配准时调用
  def FemurOrTibia(self):
    From1node = slicer.util.getNode('From')
    From1node.RemoveAllControlPoints()
    Tonode = slicer.util.getNode('To')
    Tonode.RemoveAllControlPoints()
    if self.ui.ModuleName.text == '股骨配准':
      points = np.array([[2.421287, -1.137471, -31.445272],
                         [16.180315, -33.439308, -14.108915],
                         [-27.91506, -26.189674, -17.669632],
                         [23.150414, -9.324886, -35.455048],
                         [-21.972315, -4.575426, -38.243401],
                         [36.633526, -13.987321, -14.000193],
                         [-37.562634, -1.810003, -15.941791],
                         [23.380989, 24.045118, -1.354824],
                         [5.172663, 17.771029, -28.148647]])
      Transform_tmp=slicer.util.getNode('DianjiToTracker1')
      Tonode.SetAndObserveTransformNodeID(Transform_tmp.GetID())
    elif self.ui.ModuleName.text == '胫骨配准' :
      points = np.array([[-0.13547, -2.620466, 37.901165],
                         [15.534579, 21.889713, -5.486698],
                         [19.430536, -5.408284, 30.107407],
                         [-17.972166, 1.875922, 31.642845]])
      Transform_tmp=slicer.util.getNode('TibiaToTracker')
      Tonode.SetAndObserveTransformNodeID(Transform_tmp.GetID())
    # if slicer.modules.NoImageWelcomeWidget.judge == 'L':#左腿
    #   for i in range(len(points)):
    #     points[i][0]=-points[i][0]
    for i in range(len(points)):
      From1node.AddControlPoint(points[i][0], points[i][1], points[i][2])

  def ThreeDViewAndImageWidget(self,index):
        
    if index == 0:#显示图片视图
        self.FourWidget.setVisible(False)
        self.noimageWidget.setVisible(True)
    elif index == 1:#两者都显示
        self.FourWidget.setVisible(True)
        self.noimageWidget.setVisible(True)
    elif index == 2:#显示三维窗口
        self.FourWidget.setVisible(True)
        self.noimageWidget.setVisible(False)

  #切换三维视图和图片视图
  def onSwitch(self):
    #图片widget
    slicer.app.layoutManager().setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutOneUp3DView)
    self.ThreeDViewAndImageWidget(self.SwitchState)
    if self.SwitchState == 0:
      self.pixmap = qt.QPixmap(self.iconsPath+'/'+self.pngName+'.png')
      self.PngLabel.setPixmap(self.pixmap)
      self.PngLabel.resize(self.noimageWidget.width,self.noimageWidget.height)
      self.SwitchState = 1
    elif self.SwitchState == 1:
      self.pixmap = qt.QPixmap(self.iconsPath+'/'+self.pngName+'_1.png')
      self.PngLabel.setPixmap(self.pixmap)
      self.PngLabel.resize(self.noimageWidget.width,self.noimageWidget.height)
      self.SwitchState = 2
    elif self.SwitchState == 2:            
      self.SwitchState = 0

  #股骨校准时调用该函数显示图片
  def onFemurRadioButton(self):
    for i in range(0,len(self.noimageWidget.findChildren("QLabel"))):
      self.noimageWidget.findChildren("QLabel")[-1].clear()
    self.PngLabel = qt.QLabel(self.noimageWidget)
    self.PngLabel.resize(self.noimageWidget.width,self.noimageWidget.height)
    self.PngLabel.setScaledContents(True)
    self.PngLabel.setStyleSheet("QLabel{background-color:transparent;}")
    if self.noimageWidget.width>1000:
        self.pixmap = qt.QPixmap(self.iconsPath+'/femur1.png')
    else:
        self.pixmap = qt.QPixmap(self.iconsPath+'/femur1_1.png')
      
    #self.pixmap = self.pixmap.scaled(1437,897,qt.Qt.KeepAspectRatio,qt.Qt.SmoothTransformation)
    self.PngLabel.setPixmap(self.pixmap)   
    self.PngLabel.show()
    self.pngName = "femur1"
    #self.ui.femurPoint1.setChecked(True)
    self.setCheckBoxState(self.ui.femurPoint1,self.ui.femurPoint1Label)
    self.FemurPng = 2
    self.ui.Select1.setEnabled(True)
  #胫骨校准时调用该函数选择图片
  def onTibiaRadioButton(self):
    for i in range(0,len(self.noimageWidget.findChildren("QLabel"))):
      self.noimageWidget.findChildren("QLabel")[-1].clear()
    self.PngLabel = qt.QLabel(self.noimageWidget)
    self.PngLabel.setScaledContents(True)
    self.PngLabel.setStyleSheet("QLabel{background-color:transparent;}")
    if self.noimageWidget.width>1000:
      self.pixmap = qt.QPixmap(self.iconsPath+'/tibia1.png')
    else:
      self.pixmap = qt.QPixmap(self.iconsPath+'/tibia1_1.png')
    
    self.PngLabel.setPixmap(self.pixmap)
    self.PngLabel.resize(self.noimageWidget.width,self.noimageWidget.height)
    self.PngLabel.show()
    self.setCheckBoxState(self.ui.tibiaPoint1,self.ui.tibiaPoint1Label)
    self.TibiaPng = 2
    self.ui.Select1.setEnabled(True)

  # 选择一个点
  def SelectSinglePoint(self):
    Number = ["⑳", "⑲", "⑱", "⑰", "⑯", "⑮", "⑭", "⑬", "⑫", "⑪", "⑩", "⑨", "⑧", "⑦", "⑥", "⑤", "④", "③", "②", "①"]
    probeToTransformNode = slicer.util.getNode("StylusTipToStylus")
    toMarkupsNode = slicer.util.getNode("To")
    slicer.modules.fiducialregistrationwizard.logic().AddFiducial(probeToTransformNode, toMarkupsNode)
    if self.ui.ModuleName.text == '股骨配准':
      self.PointMove('To', 'DianjiToTracker1')
      try:
        if self.FemurPng > 10:
          if self.JudgePointInRightPosition(toMarkupsNode, 'Femur'):
            label_femur = [self.ui.femurPoint10Label, self.ui.femurPoint11Label, self.ui.femurPoint12Label,
                           self.ui.femurPoint13Label, self.ui.femurPoint14Label]
            label_femur[self.FemurPng - 11].setText(Number[19 - self.FemurPointCount[self.FemurPng - 11]])
            self.FemurPointCount[self.FemurPng - 11] += 1
          else:#移除不在区域内的点
            num = toMarkupsNode.GetNumberOfControlPoints()
            toMarkupsNode.RemoveNthControlPoint(num - 1)
      except:
        print(1)
    else:
      self.PointMove('To', 'TibiaToTracker')
      try:
        if self.FemurPng > 7:
          if self.JudgePointInRightPosition(toMarkupsNode, 'Tibia'):
            label_femur = [self.ui.tibiaPoint7Label, self.ui.tibiaPoint8Label, self.ui.tibiaPoint9Label]
            label_femur[self.TibiaPng - 8].setText(Number[19 - self.TibiaPointCount[self.TibiaPng - 8]])
            self.TibiaPointCount[self.TibiaPng - 8] += 1
          else:#移除不在区域内的点
            num = toMarkupsNode.GetNumberOfControlPoints()
            toMarkupsNode.RemoveNthControlPoint(num - 1)
      except:
        print(1)

  def JudgePointInRightPosition(self, PointNode, FOrT):
    point1 = [0, 0, 0]
    num = PointNode.GetNumberOfControlPoints()
    PointNode.GetNthControlPointPositionWorld(num - 1, point1)
    if FOrT == 'Femur':
      mark = ['外侧后髁', '内侧后髁', '外侧远端', '内侧远端', '外侧皮质高点']
      point2 = [0, 0, 0]
      print(mark[self.FemurPng - 11])
      slicer.util.getNode(mark[self.FemurPng - 11]).GetNthControlPointPositionWorld(0, point2)
    else:
      mark = ['外侧高点', '内侧高点', '胫骨结节']
      point2 = [0, 0, 0]
      slicer.util.getNode(mark[self.TibiaPng - 8]).GetNthControlPointPositionWorld(0, point2)
    d = self.distance(np.array(point1), np.array(point2))
    if d < 20:
      return 1
    else:
      return 0

  # 开始选择
  def MoreStart(self):
    Stylus = slicer.util.getNode("StylusTipToStylus")
    self.FemurObserver = Stylus.AddObserver(
      slicer.vtkMRMLTransformNode.TransformModifiedEvent, self.onAddMarkups)

  def onAddMarkups(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
    Number = ["⑳", "⑲", "⑱", "⑰", "⑯", "⑮", "⑭", "⑬", "⑫", "⑪", "⑩", "⑨", "⑧", "⑦", "⑥", "⑤", "④", "③", "②", "①"]
    probeToTransformNode = slicer.util.getNode("StylusTipToStylus")
    toMarkupsNode = slicer.util.getNode("To")
    slicer.modules.fiducialregistrationwizard.logic().AddFiducial(probeToTransformNode, toMarkupsNode)
    if self.ui.ModuleName.text == '股骨配准':
      self.PointMove('To', 'DianjiToTracker1')
      if self.FemurPng > 10:
        if self.JudgePointInRightPosition(toMarkupsNode, 'Femur'):
          label_femur = [self.ui.femurPoint10Label, self.ui.femurPoint11Label, self.ui.femurPoint12Label,
                         self.ui.femurPoint13Label, self.ui.femurPoint14Label]
          label_femur[self.FemurPng - 11].setText(Number[19 - self.FemurPointCount[self.FemurPng - 11]])
          self.FemurPointCount[self.FemurPng - 11] += 1
    else:
      self.PointMove('To', 'TibiaToTracker')
      if self.FemurPng > 7:
        if self.JudgePointInRightPosition(toMarkupsNode, 'Tibia'):
          label_femur = [self.ui.tibiaPoint7Label, self.ui.tibiaPoint8Label, self.ui.tibiaPoint9Label]
          label_femur[self.TibiaPng - 8].setText(Number[19 - self.TibiaPointCount[self.TibiaPng - 8]])
          self.TibiaPointCount[self.TibiaPng - 8] += 1
  #停止选择
  def onStopSelect(self):
    Stylus = slicer.util.getNode("StylusTipToStylus")
    Stylus.RemoveObserver(self.FemurObserver)
    self.ui.Select1.setVisible(True)
    self.ui.StopSelect.setVisible(False)
  
  def SwitchFemur(self):
    femurPointCheckBox = [self.ui.femurPoint1,self.ui.femurPoint2,self.ui.femurPoint3,self.ui.femurPoint4,self.ui.femurPoint5,
                          self.ui.femurPoint6,self.ui.femurPoint7,self.ui.femurPoint8,self.ui.femurPoint9,self.ui.femurPoint10,
                          self.ui.femurPoint11,self.ui.femurPoint12,self.ui.femurPoint13,self.ui.femurPoint14]
    femurPointLabel = [self.ui.femurPoint1Label,self.ui.femurPoint2Label,self.ui.femurPoint3Label,self.ui.femurPoint4Label,self.ui.femurPoint5Label,
                      self.ui.femurPoint6Label,self.ui.femurPoint7Label,self.ui.femurPoint8Label,self.ui.femurPoint9Label,self.ui.femurPoint10Label,
                      self.ui.femurPoint11Label,self.ui.femurPoint12Label,self.ui.femurPoint13Label,self.ui.femurPoint14Label]
    if self.noimageWidget.width>1000:
        self.pixmap = qt.QPixmap(self.iconsPath+'/femur'+str(self.FemurPng)+'.png')
    else:
      self.pixmap = qt.QPixmap(self.iconsPath+'/femur'+str(self.FemurPng)+'_1.png')
    self.PngLabel.setPixmap(self.pixmap)
    self.PngLabel.resize(self.noimageWidget.width,self.noimageWidget.height)
    self.PngLabel.show()
    femurPointCheckBox[self.FemurPng-2].setChecked(True)
    if self.FemurPng-1 == 14:      
      self.ui.femurPoint14.setStyleSheet("None")
      self.ui.femurPoint14Label.setStyleSheet("None")
      self.ui.NextArea.setEnabled(False)
      self.ui.Confirm1.setEnabled(True)
      
    else:
      self.setCheckBoxState(femurPointCheckBox[self.FemurPng-1],femurPointLabel[self.FemurPng-1])
      self.pngName = "femur"+str(self.FemurPng)
      self.FemurPng += 1
    
  def SwitchTibia(self):
    tibiaPointCheckBox = [self.ui.tibiaPoint1,self.ui.tibiaPoint2,self.ui.tibiaPoint3,self.ui.tibiaPoint4,self.ui.tibiaPoint5,
                          self.ui.tibiaPoint6,self.ui.tibiaPoint7,self.ui.tibiaPoint8,self.ui.tibiaPoint9]
    tibiaPointLabel = [self.ui.tibiaPoint1Label,self.ui.tibiaPoint2Label,self.ui.tibiaPoint3Label,self.ui.tibiaPoint4Label,self.ui.tibiaPoint5Label,
                      self.ui.tibiaPoint6Label,self.ui.tibiaPoint7Label,self.ui.tibiaPoint8Label,self.ui.tibiaPoint9Label]
    if self.noimageWidget.width>1000:
      self.pixmap = qt.QPixmap(self.iconsPath+'/tibia'+str(self.TibiaPng)+'.png')
    else:
      self.pixmap = qt.QPixmap(self.iconsPath+'/tibia'+str(self.TibiaPng)+'_1.png')
    self.PngLabel.setPixmap(self.pixmap)
    self.PngLabel.resize(self.noimageWidget.width,self.noimageWidget.height)
    self.PngLabel.show()
    tibiaPointCheckBox[self.TibiaPng-2].setChecked(True)
    if self.TibiaPng-1 == 9:
      self.ui.NextArea.setEnabled(False)
      self.ui.tibiaPoint9.setStyleSheet("None")
      self.ui.tibiaPoint9Label.setStyleSheet("None")
      self.ui.Confirm1.setEnabled(True)
    else:
      self.setCheckBoxState(tibiaPointCheckBox[self.TibiaPng-1],tibiaPointLabel[self.TibiaPng-1])
      self.pngName = "Tibia"+str(self.TibiaPng)
      self.TibiaPng += 1

  #整体切换图片函数、单选点、多选点函数
  def onSelect1(self,whoSend=None):
    self.ui.PointReset.setEnabled(True)
    print(self.FemurPng - 1)

    if self.ui.ModuleName.text == "股骨配准" :
      if self.FemurPng - 1 <9:
        self.SwitchFemur()
        self.SelectSinglePoint()
        if whoSend!='xiaopingmu':
          self.sendDian()
      elif self.FemurPng - 1 == 9:
        
        self.SwitchFemur()
        self.ui.NextArea.setVisible(True)
        self.ui.SelectWidget.setVisible(True)
        self.FemurPointCount=[0,0,0,0,0]
        if self.ui.SingleSelect.checked:
          self.SelectSinglePoint()
        else:
          self.MoreStart()
          self.ui.Select1.setVisible(False)
          self.ui.StopSelect.setVisible(True)
        if whoSend!='xiaopingmu':
          self.sendDian()
        self.onConfirm1_femur()
        

          
      else:
        if self.ui.SingleSelect.checked:
          self.SelectSinglePoint()
        else:
          self.MoreStart()
          self.ui.Select1.setVisible(False)
          self.ui.StopSelect.setVisible(True)


    elif self.ui.ModuleName.text == "胫骨配准" :
      self.ui.NextArea.setEnabled(True)
      if self.TibiaPng-1 < 6:
        self.SwitchTibia()
        self.SelectSinglePoint()
        if whoSend!='xiaopingmu':
          self.sendDian()
      elif self.TibiaPng-1 == 6:
        
        self.SwitchTibia()
        self.ui.NextArea.setVisible(True)
        self.ui.SelectWidget.setVisible(True)
        self.TibiaPointCount = [0, 0, 0]
        if self.ui.SingleSelect.checked:
          self.SelectSinglePoint()
        else:
          self.MoreStart()
          self.ui.Select1.setVisible(False)
          self.ui.StopSelect.setVisible(True)
        if whoSend!='xiaopingmu':
          self.sendDian()
        self.onConfirm1_tibia()
      else:
        if self.ui.SingleSelect.checked:
          self.SelectSinglePoint()
        else:
          self.MoreStart()
          self.ui.Select1.setVisible(False)
          self.ui.StopSelect.setVisible(True)


#向小屏幕发送选取点位的信息
  def sendDian(self,reset=0):
    try:
      if reset==0:
        s1 = 0
        s2 = 2
        s3 = 0  # 股骨不重置
        s4 = 0  # 胫骨不重置
        if self.ui.ModuleName.text == "股骨配准":
          s5 = 0
        else:
          s5 = 1
          # 股骨或者胫骨图片
        if s5 == 0:
          s6 = self.FemurPng - 1  # 顺序
        else:
          s6 = self.TibiaPng - 1
        s7 = 0
        s8 = '0@\n'
        self.ser1.write(f'{s1},{s2},{s3},{s4},{s5},{s6},{s7},{s8}'.encode())
        print('已发送', f'{s1},{s2},{s3},{s4},{s5},{s6},{s7},{s8}')
      else:
        s1 = 0
        s2 = 2
        if self.ui.ModuleName.text == "股骨配准":
          s3 = 1  # 股骨重置
          s4 = 0  # 胫骨不重置

        else:
          s3 = 0  # 股骨重置
          s4 = 1  # 胫骨不重置
        if self.ui.ModuleName.text == "股骨配准":
          s5 = 0
        else:
          s5 = 1
          # 股骨或者胫骨图片
        if s5 == 0:
          s6 = 1
        else:
          s6 = 1
        s7 = 0
        s8 = '0@\n'
        self.ser1.write(f'{s1},{s2},{s3},{s4},{s5},{s6},{s7},{s8}'.encode())
        print('已发送', f'{s1},{s2},{s3},{s4},{s5},{s6},{s7},{s8}')
    except:
      pass




  def onNextArea(self,whoSend=None):
    if self.ui.ModuleName.text == "股骨配准" :
      self.SwitchFemur()
    elif self.ui.ModuleName.text == "胫骨配准" :
      self.SwitchTibia()
    if whoSend!='xiaopingmu':
      self.sendDian()

  #选点重置函数
  def onPointReset(self):
    self.ui.PointReset.setEnabled(False)
    femurPointCheckBox = [self.ui.femurPoint1,self.ui.femurPoint2,self.ui.femurPoint3,self.ui.femurPoint4,self.ui.femurPoint5,
                          self.ui.femurPoint6,self.ui.femurPoint7,self.ui.femurPoint8,self.ui.femurPoint9,self.ui.femurPoint10,
                          self.ui.femurPoint11,self.ui.femurPoint12,self.ui.femurPoint13,self.ui.femurPoint14]
    tibiaPointCheckBox = [self.ui.tibiaPoint1,self.ui.tibiaPoint2,self.ui.tibiaPoint3,self.ui.tibiaPoint4,self.ui.tibiaPoint5,
                          self.ui.tibiaPoint6,self.ui.tibiaPoint7,self.ui.tibiaPoint8,self.ui.tibiaPoint9]
    self.ui.Select1.setEnabled(True)
    self.ui.SingleSelect.setChecked(True)
    if self.ui.ModuleName.text == "股骨配准" :
      for i in range(0,len(femurPointCheckBox)):
        femurPointCheckBox[i].setChecked(False)      
      self.onFemurRadioButton()
    elif self.ui.ModuleName.text == "胫骨配准":
      for i in range(0,len(tibiaPointCheckBox)):
        tibiaPointCheckBox[i].setChecked(False)
      self.onTibiaRadioButton()
    Tonode = slicer.util.getNode('To')
    Tonode.RemoveAllControlPoints()
    #发送点重置
    self.sendDian(1)

  #单选结束确认函数_gugu
  def onConfirm1_femur(self):
    ToNode=slicer.util.getNode("To")
    #将选择的点按名称加入场景中
    self.FemurPoints = ['开髓点','外侧后髁','内侧后髁','外侧远端','内侧远端','外侧凸点','内侧凹点','外侧皮质高点','A点']
    self.TibiaPoints = ['胫骨隆凸','胫骨结节','外侧高点','内侧高点']
    Points = slicer.util.arrayFromMarkupsControlPoints(ToNode)
    if self.ui.ModuleName.text == "股骨配准" :
      Transform_tmp = slicer.util.getNode('DianjiToTracker1')
      for i in range(len(Points)):
        PointNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', self.FemurPoints[i])
        PointNode.AddControlPoint(Points[i])
        PointNode.SetDisplayVisibility(0)
        PointNode.SetAndObserveTransformNodeID(Transform_tmp.GetID())
    else:
      Transform_tmp = slicer.util.getNode('TibiaToTracker')
      for i in range(len(Points)):
        PointNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', self.TibiaPoints[i])
        PointNode.AddControlPoint(Points[i])
        PointNode.SetDisplayVisibility(0)
        PointNode.SetAndObserveTransformNodeID(Transform_tmp.GetID())
    p=slicer.util.arrayFromMarkupsControlPoints(ToNode)
    ToNode.RemoveAllControlPoints()
    if slicer.modules.NoImageWelcomeWidget.judge == 'L':
      for i in range(len(p)):
        ToNode.AddControlPoint(-p[i][0],p[i][1],p[i][2])
    else:
      for i in range(len(p)):
        ToNode.AddControlPoint(p[i][0], p[i][1], p[i][2])
    Points = slicer.util.arrayFromMarkupsControlPoints(ToNode)
    FromPoints = self.simple_femur()
    FromNode = slicer.util.getNode("From")
    FromNode.RemoveAllControlPoints()
    for i in range(len(FromPoints)):
      FromNode.AddControlPoint(FromPoints[i])
    w = slicer.util.getNode('FiducialRegistrationWizard')
    slicer.modules.fiducialregistrationwizard.logic().UpdateCalibration(w)
    transNode=slicer.util.getNode('FromToTo_femur')
    trans=slicer.util.arrayFromTransformMatrix(transNode)
    target1 = slicer.util.arrayFromMarkupsControlPoints(ToNode)
    if self.ui.ModuleName.text == "股骨配准" :
      self.data=np.loadtxt(self.FilePath+'/femur.txt')
      # if slicer.modules.NoImageWelcomeWidget.judge == 'L':
      #   for i in range(len(self.data)):
      #     self.data[i][0] = -self.data[i][0]
      for i in range(0,len(self.data)):
        l=[-self.data[i][0],-self.data[i][1],self.data[i][2],1]
        self.data[i]=np.dot(trans,l)[0:3]
      index = [7841,6968,3089,8589,2161,7462, 2410,7457,7692]
    else:#胫骨
      self.data=np.loadtxt(self.FilePath+'/tibia.txt')
      # if slicer.modules.NoImageWelcomeWidget.judge == 'L':
      #   for i in range(len(self.data)):
      #     self.data[i][0] = -self.data[i][0]
      for i in range(0,len(self.data)):
        l=[-self.data[i][0],-self.data[i][1],self.data[i][2],1]
        self.data[i]=np.dot(trans,l)[0:3]
      index=[1910, 1291, 6676, 7247]

    for i in range(len(target1)):
      self.data = self.move(self.data, index[i], target1[i])
    data1=np.empty([len(self.data),3])
    for i in range(0,len(self.data)):
      data1[i]=np.array([-self.data[i][0],-self.data[i][1],self.data[i][2]])
    self.remesh(data1)
    if self.ui.ModuleName.text == "股骨配准" :
      self.model=slicer.util.loadModel(self.FilePath+'/Femur.vtk')
    else:
      self.model = slicer.util.loadModel(self.FilePath+'/Tibia.vtk')
    self.SsmNihe(transNode)
    #将模型置于股骨工具变化下
    # transNode1=slicer.util.getNode('FemurToReal')
    transformNode = slicer.util.getNode('DianjiToTracker1')
    # Ftrans3=slicer.util.arrayFromTransformMatrix(transformNode)
    # Ftrans3_ni=np.linalg.inv(Ftrans3)
    # transNode1.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(Ftrans3_ni))
    #transNode.SetAndObserveTransformNodeID(transformNode.GetID())
    Tonode = slicer.util.getNode('To')
    Tonode.RemoveAllControlPoints()
    ToNode.SetAndObserveTransformNodeID(transformNode.GetID())

  #单选结束确认函数_jinggu
  def onConfirm1_tibia(self):

    ToNode = slicer.util.getNode("To")
    #将选择的点按名称加入场景中
    self.TibiaPoints =  ['胫骨隆凸','胫骨结节','外侧高点','内侧高点']
    Points = slicer.util.arrayFromMarkupsControlPoints(ToNode)
    Transform_tmp = slicer.util.getNode('TibiaToTracker')
    for i in range(len(self.TibiaPoints)):
      PointNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', self.TibiaPoints[i])
      PointNode.AddControlPoint(Points[i])
      PointNode.SetDisplayVisibility(0)
      PointNode.SetAndObserveTransformNodeID(Transform_tmp.GetID())
    #添加踝穴中心点
    PointNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', '踝穴中心')
    PointNode.AddControlPoint((Points[4]+Points[5])/2)
    transformNode = slicer.util.getNode('TibiaToTracker')
    PointNode.SetAndObserveTransformNodeID(transformNode.GetID())
    PointNode.SetDisplayVisibility(0)
    p = slicer.util.arrayFromMarkupsControlPoints(ToNode)
    ToNode.RemoveAllControlPoints()
    if slicer.modules.NoImageWelcomeWidget.judge == 'L':
      for i in range(len(p)):
        ToNode.AddControlPoint(-p[i][0], p[i][1], p[i][2])
    else:
      for i in range(len(p)):
        ToNode.AddControlPoint(p[i][0], p[i][1], p[i][2])
    FromPoints = self.simple_tibia()
    FromNode = slicer.util.getNode("From")
    FromNode.RemoveAllControlPoints()
    for i in range(len(FromPoints)):
      FromNode.AddControlPoint(FromPoints[i])
    w = slicer.util.getNode('FiducialRegistrationWizard')
    transNode = slicer.util.getNode('FromToTo_tibia')
    w.SetOutputTransformNodeId(transNode.GetID())
    slicer.modules.fiducialregistrationwizard.logic().UpdateCalibration(w)



    for i in range(2):                    #移除不用于校准的点
      ToNode.RemoveNthControlPoint(4)
    slicer.modules.fiducialregistrationwizard.logic().UpdateCalibration(w)
    trans = slicer.util.arrayFromTransformMatrix(transNode)
    target1 = slicer.util.arrayFromMarkupsControlPoints(ToNode)
    self.data=np.loadtxt(self.FilePath+'/tibia.txt')
    # if slicer.modules.NoImageWelcomeWidget.judge == 'L':
    #   for i in range(len(self.data)):
    #     self.data[i][0] = -self.data[i][0]
    for i in range(0,len(self.data)):
      l=[-self.data[i][0],-self.data[i][1],self.data[i][2],1]
      self.data[i]=np.dot(trans,l)[0:3]
    index=[1910, 1291, 6676, 7247]
    for i in range(len(target1)):
      self.data = self.move(self.data, index[i], target1[i])
    data1=np.empty([len(self.data),3])
    for i in range(0,len(self.data)):
      data1[i]=np.array([-self.data[i][0],-self.data[i][1],self.data[i][2]])
    self.remesh(data1)
    self.model = slicer.util.loadModel(self.FilePath+'/Tibia.vtk')
    self.SsmNihe(transNode)
    #将模型置于jinggu工具变化下
    # transNode1=slicer.util.getNode('TibiaToReal')
    transformNode = slicer.util.getNode('TibiaToTracker')
    # Ftrans3=slicer.util.arrayFromTransformMatrix(transformNode)
    # Ftrans3_ni=np.linalg.inv(Ftrans3)
    # transNode1.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(Ftrans3_ni))
    #transNode.SetAndObserveTransformNodeID(transformNode.GetID())
    Tonode = slicer.util.getNode('To')
    Tonode.RemoveAllControlPoints()
    ToNode.SetAndObserveTransformNodeID(transformNode.GetID())


  def SsmNihe(self,transNode):
    ToNode = slicer.util.getNode("To")
    transNode.Inverse()
    ToNode.SetAndObserveTransformNodeID(transNode.GetID())
    self.model.SetAndObserveTransformNodeID(transNode.GetID())
    self.model.HardenTransform()
    ToNode.HardenTransform()
    transNode.Inverse()

    if self.ui.ModuleName.text == "股骨配准" :
      # if slicer.modules.NoImageWelcomeWidget.judge == 'L':
      #   self.FemurTransform = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", '左右镜像')
      #   FemurTrans=np.array([[-1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]])
      #   self.FemurTransform.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(FemurTrans))
      #   self.model.SetAndObserveTransformNodeID(self.FemurTransform.GetID())
      #   self.model.HardenTransform()
      #   ToNode.SetAndObserveTransformNodeID(self.FemurTransform.GetID())
      #   ToNode.HardenTransform()
      myStorageNode = self.model.CreateDefaultStorageNode()
      myStorageNode.SetFileName(self.FilePath+'/FemurTmp.vtk')
      myStorageNode.WriteData(self.model)
      slicer.mrmlScene.RemoveNode(self.model)
      toPoints = slicer.util.arrayFromMarkupsControlPoints(ToNode)
      # for i in range(len(toPoints)):
      #   toPoints[i][0] = -toPoints[i][0]
      #   toPoints[i][1] = -toPoints[i][1]
      try:
        os.remove(self.FilePath+'/test.txt')
      except Exception as e:
        print(e)
      f = open(self.FilePath+'/test.txt', 'w')
      da = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
      for i in range(len(toPoints)):
        # for j in range(len(toPoints)):
        f.write(f'{da[i]},{-toPoints[i][0]},{-toPoints[i][1]},{toPoints[i][2]}\n')
      f.close()
      try:
        os.remove(self.FilePath+'/tar.csv')
      except Exception as e:
        print(e)
      os.rename(self.FilePath+'/test.txt', self.FilePath+'/tar.csv')
      command = f'set PATH=C:/Users/35367/Downloads/itk_dll;C:/Users/35367/Downloads/hdf5_dll;%PATH% & '+self.FilePath+'/statismo-fit-surface.exe -v 2.0 -f '+self.FilePath+'/Femur.csv -m '+self.FilePath+'/tar.csv -i '+self.FilePath+'/femurY.h5 -t '+self.FilePath+'/FemurTmp.vtk -w 0.0001 -p -o '+self.FilePath+'/Femur.vtk'
      command=command.replace('-f ', '-f "')
      command =command.replace('-m ','-m "')
      command =command.replace('-i ','-i "')
      command =command.replace('& ','& "')
      command =command.replace('-t ','-t "')
      command = command.replace('-o ', '-o "')
      command =command.replace('.vtk','.vtk"')
      command =command.replace('.csv','.csv"')
      command = command.replace('.exe', '.exe"')
      command = command.replace('.h5', '.h5"')
      os.system(command)
      #os.remove('d:/SliceView/SixView/tar.csv')
      self.model = slicer.util.loadModel(self.FilePath+'/Femur.vtk')


    else:
      # if slicer.modules.NoImageWelcomeWidget.judge == 'L':
      #   self.FemurTransform = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", '左右镜像')
      #   FemurTrans=np.array([[-1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]])
      #   self.FemurTransform.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(FemurTrans))
      #   self.model.SetAndObserveTransformNodeID(self.FemurTransform.GetID())
      #   self.model.HardenTransform()
      #   ToNode.SetAndObserveTransformNodeID(self.FemurTransform.GetID())
      #   ToNode.HardenTransform()
      myStorageNode = self.model.CreateDefaultStorageNode()
      myStorageNode.SetFileName(self.FilePath+'/TibiaTmp.vtk')
      myStorageNode.WriteData(self.model)
      slicer.mrmlScene.RemoveNode(self.model)
      toPoints = slicer.util.arrayFromMarkupsControlPoints(ToNode)
      # for i in range(len(toPoints)):
      #   toPoints[i][0] = -toPoints[i][0]
      #   toPoints[i][1] = -toPoints[i][1]
      try:
        os.remove(self.FilePath+'/test.txt')
      except Exception as e:
        print(e)
      f = open(self.FilePath+'/test.txt', 'w')
      da = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
      for i in range(len(toPoints)):
        # for j in range(len(toPoints)):
        f.write(f'{da[i]},{toPoints[i][0]},{toPoints[i][1]},{toPoints[i][2]}\n')
      f.close()
      try:
        os.remove(self.FilePath+'/tar.csv')
      except Exception as e:
        print(e)
      os.rename(self.FilePath+'/test.txt', self.FilePath+'/tar.csv')
      command = f'set PATH=C:/Users/35367/Downloads/itk_dll;C:/Users/35367/Downloads/hdf5_dll;%PATH% & '+self.FilePath+'/statismo-fit-surface.exe -v 2.0 -f '+self.FilePath+'/Tibia.csv -m '+self.FilePath+'/tar.csv -i '+self.FilePath+'/tibiaY.h5 -t '+self.FilePath+'/TibiaTmp.vtk -w 0.0001 -p -o '+self.FilePath+'/Tibia.vtk'
      command=command.replace('-f ', '-f "')
      command =command.replace('-m ','-m "')
      command =command.replace('-i ','-i "')
      command =command.replace('& ','& "')
      command =command.replace('-t ','-t "')
      command = command.replace('-o ', '-o "')
      command =command.replace('.vtk','.vtk"')
      command =command.replace('.csv','.csv"')
      command = command.replace('.exe', '.exe"')
      command = command.replace('.h5', '.h5"')
      os.system(command)
      #os.remove('d:/SliceView/SixView/tar.csv')
      self.model = slicer.util.loadModel(self.FilePath+'/Tibia.vtk')
    # if slicer.modules.NoImageWelcomeWidget.judge == 'L':
    #   self.model.SetAndObserveTransformNodeID(self.FemurTransform.GetID())
    #   self.model.HardenTransform()
    #   slicer.mrmlScene.RemoveNode(self.FemurTransform)
    self.model.SetAndObserveTransformNodeID(transNode.GetID())
    if slicer.modules.NoImageWelcomeWidget.judge == 'L':
      FemurTransform = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", 'RToL')
      FemurTrans = np.array([[-1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]])
      FemurTransform.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(FemurTrans))
      if self.ui.ModuleName.text == "股骨配准":
        toolNode=slicer.util.getNode('DianjiToTracker1')
      else:
        toolNode = slicer.util.getNode('TibiaToTracker')

      transNode.SetAndObserveTransformNodeID(FemurTransform.GetID())
      FemurTransform.SetAndObserveTransformNodeID(toolNode.GetID())


    #self.model.HardenTransform()

  def simple_femur(self):
    points = np.loadtxt(self.FilePath + '/simple_femur.txt')
    dis = []
    movNode = slicer.util.getNode('To')
    for j in range(5400):
      landmarkTransform = vtk.vtkLandmarkTransform()
      landmarkTransform.SetModeToRigidBody()
      n = 9
      fix_point = vtk.vtkPoints()
      fix_point.SetNumberOfPoints(n)
      mov_point = vtk.vtkPoints()
      mov_point.SetNumberOfPoints(n)
      mov = slicer.util.arrayFromMarkupsControlPoints(movNode)

      fix = points[j * 9:j * 9 + 9]
      # if slicer.modules.NoImageWelcomeWidget.judge == 'L':
      #   for i in range(n):
      #     fix_point.SetPoint(i, -fix[i][0], fix[i][1], fix[i][2])
      #     mov_point.SetPoint(i, mov[i][0], mov[i][1], mov[i][2])
      # else:
      #   for i in range(n):
      #     fix_point.SetPoint(i, fix[i][0], fix[i][1], fix[i][2])
      #     mov_point.SetPoint(i, mov[i][0], mov[i][1], mov[i][2])
      for i in range(n):
        fix_point.SetPoint(i, fix[i][0], fix[i][1], fix[i][2])
        mov_point.SetPoint(i, mov[i][0], mov[i][1], mov[i][2])
      landmarkTransform.SetSourceLandmarks(mov_point)
      landmarkTransform.SetTargetLandmarks(fix_point)
      landmarkTransform.Update()
      trans = slicer.util.arrayFromVTKMatrix(landmarkTransform.GetMatrix())
      for i in range(0, len(mov)):
        l = [mov[i][0], mov[i][1], mov[i][2], 1]
        mov[i] = np.dot(trans, l)[0:3]
      # 计算平均距离
      d = np.linalg.norm(mov - fix)
      dis.append(d)
    print(min(dis), dis.index(min(dis)))
    num = dis.index(min(dis))
    i = int(num / 900)
    num = num % 900
    j = int(num / 180)
    num = num % 180
    k = int(num / 36)
    num = num % 36
    l = int(num / 9)
    num = num % 9
    m = int(num / 3)
    num = num % 3
    n = num

    command = f'set PATH=C:/Users/35367/Downloads/itk_dll;C:/Users/35367/Downloads/hdf5_dll;%PATH% & ' + self.FilePath +'/statismo-sample.exe -i '+self.FilePath + f'/femurY.h5 -p 1:{-2.5 + i / 5 * 5},2:{-2.5 + j / 4 * 5},3:{-2.5 + k / 4 * 5},4:{-2.5 + m / 3 * 5},5:{-2.5 + l / 2 * 5},6:{-2.5 + n / 2 * 5} ' + self.FilePath + '/Femur.vtk'
    command = command.replace('-m ', '-m "')
    command = command.replace(f'6:{-2.5 + n / 2 * 5} ', f'6:{-2.5 + n / 2 * 5} "')
    command = command.replace('-i ', '-i "')
    command = command.replace('& ', '& "')
    command = command.replace('-t ', '-t "')
    command = command.replace('-o ', '-o "')
    command = command.replace('.vtk', '.vtk"')
    command = command.replace('.csv', '.csv"')
    command = command.replace('.exe', '.exe"')
    command = command.replace('.h5', '.h5"')
    os.system(command)
    model=slicer.util.loadModel(self.FilePath + '/Femur.vtk')
    polydata = model.GetPolyData()
    a = polydata.GetNumberOfPoints()
    x1 = np.empty([a, 3])
    for i in range(0, a):
      x1[i] = polydata.GetPoint(i)
      x1[i][0] = -x1[i][0]
      x1[i][1] = -x1[i][1]
    np.savetxt(self.FilePath + "/femur.txt", x1, fmt='%6f')
    slicer.mrmlScene.RemoveNode(model)
    num = dis.index(min(dis))
    #将拟合所用点列替换为最佳模型所对应的值

    frompoints=points[num * 9:num * 9 + 9]
    try:
      os.remove(self.FilePath + '/test.txt')
    except Exception as e:
      print(e)
    f = open(self.FilePath + '/test.txt', 'w')
    da = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
    for i in range(len(frompoints)):
      # for j in range(len(toPoints)):
      f.write(f'{da[i]},{-frompoints[i][0]},{-frompoints[i][1]},{frompoints[i][2]}\n')
    f.close()
    try:
      os.remove(self.FilePath + '/Femur.csv')
    except Exception as e:
      print(e)
    os.rename(self.FilePath + '/test.txt', self.FilePath + '/Femur.csv')
    # if slicer.modules.NoImageWelcomeWidget.judge == 'L':
    #   for i in range(len(frompoints)):
    #     frompoints[i][0]=-frompoints[i][0]
    return frompoints
    
  def simple_tibia(self):
    points = np.loadtxt(self.FilePath + '/simple_tibia.txt')
    dis = []
    movNode = slicer.util.getNode('To')
    for j in range(5400):
      landmarkTransform = vtk.vtkLandmarkTransform()
      landmarkTransform.SetModeToRigidBody()
      n = 4
      fix_point = vtk.vtkPoints()
      fix_point.SetNumberOfPoints(n)
      mov_point = vtk.vtkPoints()
      mov_point.SetNumberOfPoints(n)
      mov = slicer.util.arrayFromMarkupsControlPoints(movNode)
      mov=mov[0:4]
      fix = points[j * 4:j * 4 + 4]
      # if slicer.modules.NoImageWelcomeWidget.judge == 'L':
      #   for i in range(n):
      #     fix_point.SetPoint(i, -fix[i][0], fix[i][1], fix[i][2])
      #     mov_point.SetPoint(i, mov[i][0], mov[i][1], mov[i][2])

      for i in range(n):
        fix_point.SetPoint(i, fix[i][0], fix[i][1], fix[i][2])
        mov_point.SetPoint(i, mov[i][0], mov[i][1], mov[i][2])
      landmarkTransform.SetSourceLandmarks(mov_point)
      landmarkTransform.SetTargetLandmarks(fix_point)
      landmarkTransform.Update()
      trans = slicer.util.arrayFromVTKMatrix(landmarkTransform.GetMatrix())
      for i in range(0, len(mov)):
        l = [mov[i][0], mov[i][1], mov[i][2], 1]
        mov[i] = np.dot(trans, l)[0:3]
      # 计算平均距离
      d = np.linalg.norm(mov - fix)
      dis.append(d)
    print(min(dis), dis.index(min(dis)))
    num = dis.index(min(dis))
    i = int(num / 900)
    num = num % 900
    j = int(num / 180)
    num = num % 180
    k = int(num / 36)
    num = num % 36
    l = int(num / 9)
    num = num % 9
    m = int(num / 3)
    num = num % 3
    n = num

    command = f'set PATH=C:/Users/35367/Downloads/itk_dll;C:/Users/35367/Downloads/hdf5_dll;%PATH% & ' + self.FilePath +'/statismo-sample.exe -i '+self.FilePath + f'/tibiaY.h5 -p 1:{-2.5 + i / 5 * 5},2:{-2.5 + j / 4 * 5},3:{-2.5 + k / 4 * 5},4:{-2.5 + m / 3 * 5},5:{-2.5 + l / 2 * 5},6:{-2.5 + n / 2 * 5} ' + self.FilePath + '/Tibia.vtk'
    command = command.replace('-m ', '-m "')
    command = command.replace(f'6:{-2.5 + n / 2 * 5} ', f'6:{-2.5 + n / 2 * 5} "')
    command = command.replace('-i ', '-i "')
    command = command.replace('& ', '& "')
    command = command.replace('-t ', '-t "')
    command = command.replace('-o ', '-o "')
    command = command.replace('.vtk', '.vtk"')
    command = command.replace('.csv', '.csv"')
    command = command.replace('.exe', '.exe"')
    command = command.replace('.h5', '.h5"')
    os.system(command)
    model=slicer.util.loadModel(self.FilePath + '/Tibia.vtk')
    polydata = model.GetPolyData()
    a = polydata.GetNumberOfPoints()
    x1 = np.empty([a, 3])
    for i in range(0, a):
      x1[i] = polydata.GetPoint(i)
      x1[i][0] = -x1[i][0]
      x1[i][1] = -x1[i][1]
    np.savetxt(self.FilePath + "/tibia.txt", x1, fmt='%6f')
    slicer.mrmlScene.RemoveNode(model)
    num = dis.index(min(dis))
    #将拟合所用点列替换为最佳模型所对应的值

    frompoints=points[num * 4:num * 4 + 4]
    try:
      os.remove(self.FilePath + '/test.txt')
    except Exception as e:
      print(e)
    f = open(self.FilePath + '/test.txt', 'w')
    da = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
    for i in range(len(frompoints)):
      # for j in range(len(toPoints)):
      f.write(f'{da[i]},{frompoints[i][0]},{frompoints[i][1]},{frompoints[i][2]}\n')
    f.close()
    try:
      os.remove(self.FilePath + '/Tibia.csv')
    except Exception as e:
      print(e)
    os.rename(self.FilePath + '/test.txt', self.FilePath + '/Tibia.csv')
    # if slicer.modules.NoImageWelcomeWidget.judge == 'L':
    #   for i in range(len(frompoints)):
    #     frompoints[i][0]=-frompoints[i][0]
    return frompoints

  def ssm_nihe_n(self,Topoints):
    if self.ui.ModuleName.text == "股骨配准":
      command = f'set PATH=C:/Users/35367/Downloads/itk_dll;C:/Users/35367/Downloads/hdf5_dll;%PATH% & '+self.FilePath+'/statismo-fit-surface.exe -v 2.0 -f '+self.FilePath+'/Femur.csv -m '+self.FilePath+'/tar.csv -i '+self.FilePath+'/femurY.h5 -t '+self.FilePath+'/Femur.vtk -w 0.0001 -p -o '+self.FilePath+'/Femur.vtk'
      command=command.replace('-f ', '-f "')
      command =command.replace('-m ','-m "')
      command =command.replace('-i ','-i "')
      command =command.replace('& ','& "')
      command =command.replace('-t ','-t "')
      command = command.replace('-o ', '-o "')
      command =command.replace('.vtk','.vtk"')
      command =command.replace('.csv','.csv"')
      command = command.replace('.exe', '.exe"')
      command = command.replace('.h5', '.h5"')
      os.system(command)
      self.model = slicer.util.loadModel(self.FilePath + '/Femur.vtk')
      polydata = self.model.GetPolyData()
      a = polydata.GetNumberOfPoints()
      x1 = np.empty([a, 3])
      for i in range(0, a):
        x1[i] = polydata.GetPoint(i)
      target_new = Topoints
      self.data = x1

      for i in range(len(target_new)):
        idx = self.panduan(self.data, target_new[i])
        # print(idx)
        self.data = self.move(self.data, idx, target_new[i])
      data1 = np.empty([len(self.data), 3])
      for i in range(0, len(self.data)):
        data1[i] = np.array([-self.data[i][0], -self.data[i][1], self.data[i][2]])
      self.remesh(data1)
      slicer.mrmlScene.RemoveNode(self.model)
      self.model = slicer.util.loadModel(self.FilePath + '/Femur.vtk')
    else:
      command = f'set PATH=C:/Users/35367/Downloads/itk_dll;C:/Users/35367/Downloads/hdf5_dll;%PATH% & ' + self.FilePath + '/statismo-fit-surface.exe -v 2.0 -f ' + self.FilePath + '/Tibia.csv -m ' + self.FilePath + '/tar.csv -i ' + self.FilePath + '/tibiaY.h5 -t ' + self.FilePath + '/Tibia.vtk -w 0.0001 -p -o ' + self.FilePath + '/Tibia.vtk'
      command = command.replace('-f ', '-f "')
      command = command.replace('-m ', '-m "')
      command = command.replace('-i ', '-i "')
      command = command.replace('& ', '& "')
      command = command.replace('-t ', '-t "')
      command = command.replace('-o ', '-o "')
      command = command.replace('.vtk', '.vtk"')
      command = command.replace('.csv', '.csv"')
      command = command.replace('.exe', '.exe"')
      command = command.replace('.h5', '.h5"')
      os.system(command)
      self.model = slicer.util.loadModel(self.FilePath + '/Tibia.vtk')
      polydata = self.model.GetPolyData()
      a = polydata.GetNumberOfPoints()
      x1 = np.empty([a, 3])
      for i in range(0, a):
        x1[i] = polydata.GetPoint(i)
      target_new = Topoints
      self.data = x1

      for i in range(len(target_new)):
        idx = self.panduan(self.data, target_new[i])
        # print(idx)
        self.data = self.move(self.data, idx, target_new[i])
      data1 = np.empty([len(self.data), 3])
      for i in range(0, len(self.data)):
        data1[i] = np.array([-self.data[i][0], -self.data[i][1], self.data[i][2]])
      self.remesh(data1)
      slicer.mrmlScene.RemoveNode(self.model)
      self.model = slicer.util.loadModel(self.FilePath + '/Tibia.vtk')
  
  #确认函数
  def onConfirm2(self):
    self.ui.ForwardToolButton.setEnabled(True)
    # 将精拟合所用的点置于股骨头坐标系，方便拟合
    polydata = self.model.GetPolyData()
    a = polydata.GetNumberOfPoints()
    x1 = np.empty([a, 3])
    for i in range(0, a):
      x1[i] = polydata.GetPoint(i)
    ToNode = slicer.util.getNode('To')
    Topoints = slicer.util.arrayFromMarkupsControlPoints(ToNode)
    if slicer.modules.NoImageWelcomeWidget.judge == 'L':
      ToNode.RemoveAllControlPoints()
      for i in range(len(Topoints)):
        ToNode.AddControlPoint(-Topoints[i][0], Topoints[i][1], Topoints[i][2])
    # ToNode.RemoveAllControlPoints()
    # if self.ui.ModuleName.text == "股骨配准":
    #   p1=np.loadtxt('E:/mesh_points.txt')
    # else:
    #   p1=np.loadtxt('E:/t2.txt')
    # for i in range(len(p1)):
    #   ToNode.AddControlPoint(p1[i])
    Topoints = slicer.util.arrayFromMarkupsControlPoints(ToNode)
    if self.ui.ModuleName.text == "股骨配准":
      transNode = slicer.util.getNode('FromToTo_femur')
    else:
      transNode = slicer.util.getNode('FromToTo_tibia')
    trans = slicer.util.arrayFromTransformMatrix(transNode)
    trans_ni = np.linalg.inv(trans)
    for i in range(0, len(Topoints)):
      l = [Topoints[i][0], Topoints[i][1], Topoints[i][2], 1]
      Topoints[i] = np.dot(trans_ni, l)[0:3]
    # target_new = [[9.914172,-6.626008,42.820110]]
    target_new = Topoints
    self.data = x1
    #for j in range(0,3):
    # idx=[]
    # for i in range(len(target_new)):
    #   idx.append(self.panduan(self.data, target_new[i]))
      # print(idx)


    # landmarkTransform = vtk.vtkLandmarkTransform()
    # landmarkTransform.SetModeToRigidBody()
    # n = len(target_new)
    # fix_point = vtk.vtkPoints()
    # fix_point.SetNumberOfPoints(n)
    # mov_point = vtk.vtkPoints()
    # mov_point.SetNumberOfPoints(n)
    # mov=self.data[idx]
    # fix=target_new
    # for i in range(n):
    #   fix_point.SetPoint(i, fix[i][0], fix[i][1], fix[i][2])
    #   mov_point.SetPoint(i, mov[i][0], mov[i][1], mov[i][2])
    # landmarkTransform.SetSourceLandmarks(mov_point)
    # landmarkTransform.SetTargetLandmarks(fix_point)
    # landmarkTransform.Update()
    # trans = slicer.util.arrayFromVTKMatrix(landmarkTransform.GetMatrix())
    # for i in range(0, len(self.data)):
    #   l = [self.data[i][0], self.data[i][1], self.data[i][2], 1]
    #   self.data[i] = np.dot(trans, l)[0:3]


    fiducialsPolyData = vtk.vtkPolyData()
    points = vtk.vtkPoints()
    for i in range(len(target_new)):
      points.InsertNextPoint(target_new[i])

    tempPolyData = vtk.vtkPolyData()
    tempPolyData.SetPoints(points)
    vertex = vtk.vtkVertexGlyphFilter()
    vertex.SetInputData(tempPolyData)
    vertex.Update()
    fiducialsPolyData.ShallowCopy(vertex.GetOutput())

    icpTransform = vtk.vtkIterativeClosestPointTransform()
    icpTransform.SetSource(fiducialsPolyData)
    icpTransform.SetTarget(polydata)
    icpTransform.GetLandmarkTransform().SetModeToRigidBody()
    icpTransform.SetMaximumNumberOfIterations(100)
    icpTransform.Modified()
    icpTransform.Update()
    trans_ni = slicer.util.arrayFromVTKMatrix(icpTransform.GetMatrix())
    trans = np.linalg.inv(trans_ni)
    print('面配准矩阵',trans)
    for i in range(0, len(self.data)):
      l = [self.data[i][0], self.data[i][1], self.data[i][2], 1]
      self.data[i] = np.dot(trans, l)[0:3]


    for i in range(len(target_new)):
      idx=self.panduan(self.data, target_new[i])
      self.data = self.move(self.data, idx, target_new[i])
    data1 = np.empty([len(self.data), 3])
    for i in range(0, len(self.data)):
      data1[i] = np.array([-self.data[i][0], -self.data[i][1], self.data[i][2]])
    self.remesh(data1)
    #多次拟合
    # self.ssm_nihe_n(Topoints)
    # self.ssm_nihe_n(Topoints)
    slicer.mrmlScene.RemoveNode(self.model)
    if self.ui.ModuleName.text == "股骨配准" :
      self.model=slicer.util.loadModel(self.FilePath+'/Femur.vtk')
      self.model.SetName('Femur')
      self.model.SetAndObserveTransformNodeID(transNode.GetID())
      self.model.HardenTransform()
      if slicer.modules.NoImageWelcomeWidget.judge == 'L':
        slicer.mrmlScene.RemoveNode(slicer.util.getNode('RToL'))
        FemurTransform = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", 'RToL')
        FemurTrans = np.array([[-1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]])
        FemurTransform.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(FemurTrans))
        self.model.SetAndObserveTransformNodeID(FemurTransform.GetID())
        self.model.HardenTransform()
        slicer.mrmlScene.RemoveNode(FemurTransform)
        print('jingnihe')
        #镜像转化代码无效，重复执行一次
        slicer.mrmlScene.RemoveNode(self.model)
        transNode = slicer.util.getNode('FromToTo_femur')
        self.model = slicer.util.loadModel(self.FilePath+'/Femur.vtk')
        self.model.SetName('Femur')
        self.model.SetAndObserveTransformNodeID(transNode.GetID())
        self.model.HardenTransform()
        # slicer.mrmlScene.RemoveNode(slicer.util.getNode('RToL'))
        FemurTransform = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", 'RToL')
        FemurTrans = np.array([[-1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]])
        FemurTransform.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(FemurTrans))
        self.model.SetAndObserveTransformNodeID(FemurTransform.GetID())
        self.model.HardenTransform()
        slicer.mrmlScene.RemoveNode(FemurTransform)
        self.EtctMove('Femur','DianjiToTracker1')
      else:
        self.NodeMove('Femur','DianjiToTracker1')

    else:
      self.model = slicer.util.loadModel(self.FilePath+'/Tibia.vtk')
      self.model.SetName('Tibia')
      self.model.SetAndObserveTransformNodeID(transNode.GetID())
      self.model.HardenTransform()
      if slicer.modules.NoImageWelcomeWidget.judge == 'L':
        slicer.mrmlScene.RemoveNode(slicer.util.getNode('RToL'))
        FemurTransform = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", 'RToL')
        FemurTrans = np.array([[-1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]])
        FemurTransform.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(FemurTrans))
        self.model.SetAndObserveTransformNodeID(FemurTransform.GetID())
        self.model.HardenTransform()
        slicer.mrmlScene.RemoveNode(FemurTransform)
        slicer.mrmlScene.RemoveNode(self.model)
        transNode = slicer.util.getNode('FromToTo_tibia')
        self.model = slicer.util.loadModel(self.FilePath+'/Tibia.vtk')
        self.model.SetName('Tibia')
        self.model.SetAndObserveTransformNodeID(transNode.GetID())
        self.model.HardenTransform()
        # slicer.mrmlScene.RemoveNode(slicer.util.getNode('RToL'))
        FemurTransform = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", 'RToL')
        FemurTrans = np.array([[-1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]])
        FemurTransform.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(FemurTrans))
        self.model.SetAndObserveTransformNodeID(FemurTransform.GetID())
        self.model.HardenTransform()
        slicer.mrmlScene.RemoveNode(FemurTransform)
        self.EtctMove('Tibia','TibiaToTracker')
      else:
        self.NodeMove('Tibia','TibiaToTracker')
      slicer.util.getNode('mp').SetDisplayVisibility(False)
      slicer.util.getNode('NeedleModel').SetDisplayVisibility(False)
      slicer.util.getNode('From').SetDisplayVisibility(False)
      slicer.util.getNode('To').SetDisplayVisibility(False)
      #添加计算所需的点。
      self.buildPointsInFemur()
    self.Smooth_Model(self.model)
    self.StartClosestLine()

  def StartClosestLine(self):
    #添加显示误差值功能，在针尖与模型表面最近点之间连线，显示距离
    p = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', 'StylusTip')
    p.AddControlPoint(0, 0, 0)
    StylusTip = slicer.util.getNode('StylusTipToStylus')
    p.SetAndObserveTransformNodeID(StylusTip.GetID())

    self.line_closed = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsLineNode', 'line_closed')
    self.line_closed.AddControlPoint(0, 0, 0)
    self.line_closed.AddControlPoint(0, 0, 0)
    a = self.line_closed.GetDisplayNode()
    a.SetTextScale(3)
    self.ClosestLineObserver = StylusTip.AddObserver(slicer.vtkMRMLTransformNode.TransformModifiedEvent,
                                                          self.ShowClosestLine)

  def StopClosestLine(self):
    # 添加显示误差值功能，在针尖与模型表面最近点之间连线，显示距离
    try:
      p = slicer.util.getNode('StylusTip')
      StylusTip = slicer.util.getNode('StylusTipToStylus')
      StylusTip.RemoveObserver(self.ClosestLineObserver)
      slicer.mrmlScene.RemoveNode(p)
      slicer.mrmlScene.RemoveNode(self.line_closed)
    except:
      pass

  def ShowClosestLine(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
    ras = [0, 0, 0]
    slicer.util.getNode('StylusTip').GetNthControlPointPositionWorld(0, ras)
    inputModel1 = slicer.util.getNode('Femur')
    surface_World1 = inputModel1.GetPolyData()
    transNode = slicer.util.getNode('DianjiToTracker1')
    trans_femur = slicer.util.arrayFromTransformMatrix(transNode)
    trans_femur_ni = np.linalg.inv(trans_femur)
    l = [ras[0], ras[1], ras[2], 1]
    ras_femur = np.dot(trans_femur_ni, l)[0:3]
    d_femur, closestPointOnSurface_World = self.CaculateClosedPoint(surface_World1, ras_femur)

    try:
      inputModel2 = slicer.util.getNode('Tibia')
      surface_World2 = inputModel2.GetPolyData()
      transNode = slicer.util.getNode('TibiaToTracker')
      trans_tibia = slicer.util.arrayFromTransformMatrix(transNode)
      trans_tibia_ni = np.linalg.inv(trans_tibia)
      l = [ras[0], ras[1], ras[2], 1]
      ras_tibia = np.dot(trans_tibia_ni, l)[0:3]
    
      d_tibia, closestPointOnSurface_World_tibia = self.CaculateClosedPoint(surface_World2, ras_tibia)
      if d_tibia > d_femur:
        l = [closestPointOnSurface_World_tibia[0], closestPointOnSurface_World_tibia[1], closestPointOnSurface_World_tibia[2], 1]
        ras_tibia = np.dot(trans_tibia, l)[0:3]
        self.line_closed.SetNthControlPointPosition(0, ras_tibia)
        self.line_closed.SetNthControlPointPositionFromArray(1, ras)
      else:
        l = [closestPointOnSurface_World_femur[0], closestPointOnSurface_World_femur[1], closestPointOnSurface_World_femur[2], 1]
        ras_femur = np.dot(trans_femur, l)[0:3]
        self.line_closed.SetNthControlPointPosition(0, ras_femur)
        self.line_closed.SetNthControlPointPositionFromArray(1, ras)

    except:
      l1 = [closestPointOnSurface_World[0], closestPointOnSurface_World[1], closestPointOnSurface_World[2], 1]
      ras_femur = np.dot(trans_femur, l1)[0:3]
      self.line_closed.SetNthControlPointPosition(0, ras_femur)
      self.line_closed.SetNthControlPointPositionFromArray(1, ras)

      


  def CaculateClosedPoint(self, surface_World, point_World):
    distanceFilter = vtk.vtkImplicitPolyDataDistance()
    distanceFilter.SetInput(surface_World)
    closestPointOnSurface_World = np.zeros(3)
    closestPointDistance = distanceFilter.EvaluateFunctionAndGetClosestPoint(point_World, closestPointOnSurface_World)
    return closestPointDistance,closestPointOnSurface_World

  def Smooth_Model(self,model):
    import SurfaceToolbox
    slicer.modules.surfacetoolbox.widgetRepresentation()
    slicer.modules.SurfaceToolboxWidget._parameterNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScriptedModuleNode")
    slicer.modules.SurfaceToolboxWidget.setParameterNode(slicer.modules.SurfaceToolboxWidget._parameterNode)
    slicer.modules.SurfaceToolboxWidget._parameterNode.SetNodeReferenceID("inputModel", model.GetID())
    slicer.modules.SurfaceToolboxWidget._parameterNode.SetNodeReferenceID("outputModel", model.GetID())
    slicer.modules.SurfaceToolboxWidget._parameterNode.SetParameter("smoothing", "true")
    slicer.modules.SurfaceToolboxWidget._parameterNode.SetParameter("smoothingMethod", "Laplace")
    slicer.modules.SurfaceToolboxWidget._parameterNode.SetParameter("smoothingLaplaceIterations", "11")
    slicer.modules.SurfaceToolboxWidget._parameterNode.SetParameter("smoothingLaplaceRelaxation", "0.1")
    slicer.modules.SurfaceToolboxWidget.onApplyButton()
    slicer.mrmlScene.RemoveNode(slicer.modules.SurfaceToolboxWidget._parameterNode)


  def onHPoint(self):
    f = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', 'H点')
    probeToTransformNode = slicer.util.getNode("StylusTipToStylus")
    slicer.modules.fiducialregistrationwizard.logic().AddFiducial(probeToTransformNode, f)
    transformNode = slicer.util.getNode('DianjiToTracker1')
    f.SetAndObserveTransformNodeID(transformNode.GetID())
    self.PointMove('H点','DianjiToTracker1')
    self.StopClosestLine()
    
  def onGuGuTou(self):
    name=[]
    nodes=slicer.util.getNodesByClass('vtkMRMLMarkupsFiducialNode')
    for i in range(len(nodes)):
      node=nodes[i]
      name.append(node.GetName())
    if '球心拟合' in name:
      f=slicer.util.getNode('球心拟合')

    else:
      f = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', '球心拟合')
      transformNode = slicer.util.getNode('DianjiToTracker1')
      f.SetAndObserveTransformNodeID(transformNode.GetID())
      self.StopClosestLine()

    probeToTransformNode = slicer.util.getNode("StylusTipToStylus")
    slicer.modules.fiducialregistrationwizard.logic().AddFiducial(probeToTransformNode, f)
    self.PointMove('球心拟合', 'DianjiToTracker1')
  
  def onGuGuTouConfirm(self):
    f=slicer.util.getNode('球心拟合')
    points=slicer.util.arrayFromMarkupsControlPoints(f)
    slicer.mrmlScene.RemoveNode(f)
    points = points.astype(np.float64)  # 防止溢出
    num_points = points.shape[0]
    print(num_points)
    x = points[:, 0]
    y = points[:, 1]
    z = points[:, 2]
    x_avr = sum(x) / num_points
    y_avr = sum(y) / num_points
    z_avr = sum(z) / num_points
    xx_avr = sum(x * x) / num_points
    yy_avr = sum(y * y) / num_points
    zz_avr = sum(z * z) / num_points
    xy_avr = sum(x * y) / num_points
    xz_avr = sum(x * z) / num_points
    yz_avr = sum(y * z) / num_points
    xxx_avr = sum(x * x * x) / num_points
    xxy_avr = sum(x * x * y) / num_points
    xxz_avr = sum(x * x * z) / num_points
    xyy_avr = sum(x * y * y) / num_points
    xzz_avr = sum(x * z * z) / num_points
    yyy_avr = sum(y * y * y) / num_points
    yyz_avr = sum(y * y * z) / num_points
    yzz_avr = sum(y * z * z) / num_points
    zzz_avr = sum(z * z * z) / num_points

    A = np.array([[xx_avr - x_avr * x_avr, xy_avr - x_avr * y_avr, xz_avr - x_avr * z_avr],
                  [xy_avr - x_avr * y_avr, yy_avr - y_avr * y_avr, yz_avr - y_avr * z_avr],
                  [xz_avr - x_avr * z_avr, yz_avr - y_avr * z_avr, zz_avr - z_avr * z_avr]])
    b = np.array([xxx_avr - x_avr * xx_avr + xyy_avr - x_avr * yy_avr + xzz_avr - x_avr * zz_avr,
                  xxy_avr - y_avr * xx_avr + yyy_avr - y_avr * yy_avr + yzz_avr - y_avr * zz_avr,
                  xxz_avr - z_avr * xx_avr + yyz_avr - z_avr * yy_avr + zzz_avr - z_avr * zz_avr])
    # print(A, b)
    b = b / 2
    center = np.linalg.solve(A, b)
    x0 = center[0]
    y0 = center[1]
    z0 = center[2]
    r2 = xx_avr - 2 * x0 * x_avr + x0 * x0 + yy_avr - 2 * y0 * y_avr + y0 * y0 + zz_avr - 2 * z0 * z_avr + z0 * z0
    r = r2 ** 0.5
    print(center, r)
    f = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', '股骨头球心')
    transformNode = slicer.util.getNode('DianjiToTracker1')
    f.SetAndObserveTransformNodeID(transformNode.GetID())
    f.AddControlPoint(center[0],center[1],center[2])
    f.SetDisplayVisibility(False)
    h=slicer.util.getNode('H点')
    h.SetDisplayVisibility(False)


  #配准过程中调用的函数
  def distance(self, p1, p2):
    p3 = p1 - p2
    d = np.sqrt(np.dot(p3, p3))
    return d

  def move(self, data, i, target):
    n = target - data[i]
    r = 30
    point_index = []
    point_d = []
    for j in range(len(data)):
      d = self.distance(data[j], data[i])
      if d < r:
        point_index.append(j)
        point_d.append(d)
    for j in range(len(point_index)):
      data[point_index[j]] = (1 - point_d[j] / r) * n + data[point_index[j]]
    return data

  def panduan(self, old_target, target):
    hh = {}
    for i in range(len(old_target)):
      d = self.distance(old_target[i], target)
      # print(d)
      hh[i] = d
    return int(min(hh, key=hh.get))

  def remesh(self, data):
    try:
      if self.ui.ModuleName.text == "股骨配准" :
        os.remove(self.FilePath+'/Femur.vtk')
      else:
        os.remove(self.FilePath+'/Tibia.vtk')
    except Exception as e:
      print(e)
    hhh = data.tolist()
    if self.ui.ModuleName.text == "股骨配准" :
      mesh = open(self.FilePath+'/mesh_femur.txt').readlines()
    else:
      mesh = open(self.FilePath+'/mesh_tibia.txt').readlines()
    for i in range(len(mesh)):
      hhh.append(mesh[i].replace('\n', ''))
    f = open(self.FilePath+'/out.txt', 'a+')
    f.write('''# vtk DataFile Version 3.0
            vtk output
            ASCII
            DATASET POLYDATA
            POINTS 10000 float''')
    for i in range(len(hhh)):
      # print(f"\n{str(hhh[i]).replace(',', ' ').replace('[', '').replace(']', '')}")
      f.write(f"\n{str(hhh[i]).replace(',', ' ').replace('[', '').replace(']', '')}")
    f.close()
    if self.ui.ModuleName.text == "股骨配准" :
      os.rename(self.FilePath+'/out.txt', self.FilePath+'/Femur.vtk')
    else:
      os.rename(self.FilePath+'/out.txt', self.FilePath+'/Tibia.vtk')

  #-----------一些初始化的函数---------------------------------------------
  def FemurButtonChecked(self,button):
    self.ui.Parameter.setChecked(False)
    self.ui.Adjustment.setChecked(False)
    self.ui.ViewChoose.setChecked(False)
    self.ui.Reset.setChecked(False)
    self.ui.ForceLine.setChecked(False)
    if button == None:
      self.HideAll()
    else:
      button.setChecked(True)

  def TibiaButtonChecked(self,button):
    self.ui.Parameter2.setChecked(False)
    self.ui.Adjustment2.setChecked(False)
    self.ui.ViewChoose2.setChecked(False)
    self.ui.ReSet2.setChecked(False)
    self.ui.ForceLine2.setChecked(False)
    if button == None:
        self.HideAll()
    else:
        button.setChecked(True)

  def ReportButtonChecked(self,button):
    self.ui.JieTu.setChecked(False)
    self.ui.CTReport.setChecked(False)
    self.ui.MRIReport.setChecked(False)
    if button == None:
        self.HideAll()
    else:
        button.setChecked(True)
  
  def NavigationButtonChecked(self,button):
    self.ui.DriveJZ.setChecked(False)
    self.ui.FemurQG.setChecked(False)
    self.ui.TibiaQG.setChecked(False)
    if button == None:
        self.HideAll()
    else:      
        button.setChecked(True)


  #模型及点随工具而动
  def NodeMove(self, modelName, transName):
    modelNode = slicer.util.getNode(modelName)
    transNode = slicer.util.getNode(transName)
    Ftrans3 = slicer.util.arrayFromTransformMatrix(transNode)
    Ftrans3_ni = np.linalg.inv(Ftrans3)
    Transform_tmp = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", 'trans_tmp')
    Transform_tmp.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(Ftrans3_ni))
    modelNode.SetAndObserveTransformNodeID(Transform_tmp.GetID())
    modelNode.HardenTransform()
    modelNode.SetAndObserveTransformNodeID(transNode.GetID())
    slicer.mrmlScene.RemoveNode(Transform_tmp)

  def PointMove(self, pointName, transName):
    pointNode = slicer.util.getNode(pointName)
    transNode = slicer.util.getNode(transName)
    Ftrans3 = slicer.util.arrayFromTransformMatrix(transNode)
    Ftrans3_ni = np.linalg.inv(Ftrans3)
    points = slicer.util.arrayFromMarkupsControlPoints(pointNode)
    l = [points[-1][0], points[-1][1], points[-1][2], 1]
    mov = np.dot(Ftrans3_ni, l)[0:3]
    pointNode.RemoveNthControlPoint(len(points)-1)
    pointNode.AddControlPoint(mov)

  def EtctMove(self, nodeName, transName):
    pointNode = slicer.util.getNode(nodeName)
    transNode = slicer.util.getNode(transName)
    pointNode.SetAndObserveTransformNodeID(transNode.GetID())

  def addAxisFemur(self):
    o = [0, 0, 0]
    z = [0, 0, 1]
    y = [0, 1, 0]
    x = [1, 0, 0]

    f = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', 'Femur_ZAxis')
    f.AddControlPoint(o)
    f.AddControlPoint(z)
    Femur_ZAxis = slicer.util.getNode('变换_1')
    f.SetAndObserveTransformNodeID(Femur_ZAxis.GetID())
    f.SetDisplayVisibility(False)


    f = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', 'Femur_XAxis')
    f.AddControlPoint(o)
    f.AddControlPoint(x)
    Femur_XAxis = slicer.util.getNode('变换_1')
    f.SetAndObserveTransformNodeID(Femur_XAxis.GetID())
    f.SetDisplayVisibility(False)

    f = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', 'Femur_ZJtAxis')
    f.AddControlPoint(o)
    f.AddControlPoint(z)
    Femur_ZJtAxis = slicer.util.getNode('变换_R')
    f.SetAndObserveTransformNodeID(Femur_ZJtAxis.GetID())
    f.SetDisplayVisibility(False)

    f = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', 'Femur_YJtAxis')
    f.AddControlPoint(o)
    f.AddControlPoint(y)
    Femur_ZJtAxis = slicer.util.getNode('变换_R')
    f.SetAndObserveTransformNodeID(Femur_ZJtAxis.GetID())
    f.SetDisplayVisibility(False)

  def addAxisTibia(self):
    o = [0, 0, 0]
    x=[1,0,0]
    z = [0, 0, 1]
    y = [0, 1, 0]

    f = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', 'Tibia_ZAxis')
    f.AddControlPoint(o)
    f.AddControlPoint(z)
    Tibia_ZAxis = slicer.util.getNode('变换_4')
    f.SetAndObserveTransformNodeID(Tibia_ZAxis.GetID())
    f.SetDisplayVisibility(False)

    f = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', 'Tibia_XAxis')
    f.AddControlPoint(o)
    f.AddControlPoint(x)
    Tibia_XAxis = slicer.util.getNode('变换_4')
    f.SetAndObserveTransformNodeID(Tibia_XAxis.GetID())
    f.SetDisplayVisibility(False)

    f = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', 'Tibia_YAxis')
    f.AddControlPoint(o)
    f.AddControlPoint(y)
    Tibia_YAxis = slicer.util.getNode('变换_4')
    f.SetAndObserveTransformNodeID(Tibia_YAxis.GetID())
    f.SetDisplayVisibility(False)

    f = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', 'Tibia_ZJtAxis')
    f.AddControlPoint(o)
    f.AddControlPoint(z)
    Tibia_ZJtAxis = slicer.util.getNode('变换_约束')
    f.SetAndObserveTransformNodeID(Tibia_ZJtAxis.GetID())
    f.SetDisplayVisibility(False)

    f = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', 'Tibia_YZPlane')
    f.AddControlPoint(o)
    f.AddControlPoint(y)
    f.AddControlPoint(z)
    Tibia_YZPlane = slicer.util.getNode('变换_4')
    f.SetAndObserveTransformNodeID(Tibia_YZPlane.GetID())
    f.SetDisplayVisibility(False)

    f = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', 'Tibia_XZPlane')
    f.AddControlPoint(o)
    f.AddControlPoint(x)
    f.AddControlPoint(z)
    Tibia_XZPlane = slicer.util.getNode('变换_4')
    f.SetAndObserveTransformNodeID(Tibia_XZPlane.GetID())
    f.SetDisplayVisibility(False)

  def buildPointsInFemur(self):

    line_outside = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsCurveNode", 'line_outside')
    l1 = slicer.util.getNode('外侧后髁')
    l2 = slicer.util.getNode('外侧远端')
    p1 = slicer.util.arrayFromMarkupsControlPoints(l1)[0]
    p2 = slicer.util.arrayFromMarkupsControlPoints(l2)[0]
    line_outside.AddControlPoint(p1)
    line_outside.AddControlPoint(p2)
    transNode = slicer.util.getNode('DianjiToTracker1')
    line_outside.SetAndObserveTransformNodeID(transNode.GetID())
    line_outside.SetCurveTypeToShortestDistanceOnSurface(slicer.util.getNode('Femur'))
    line_outside.ResampleCurveSurface(2, slicer.util.getNode('Femur'))

    self.points_outside = slicer.util.arrayFromMarkupsControlPoints(line_outside)
    line_inside = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsCurveNode", 'line_inside')
    l1 = slicer.util.getNode('内侧后髁')
    l2 = slicer.util.getNode('内侧远端')
    p1 = slicer.util.arrayFromMarkupsControlPoints(l1)[0]
    p2 = slicer.util.arrayFromMarkupsControlPoints(l2)[0]
    line_inside.AddControlPoint(p1)
    line_inside.AddControlPoint(p2)
    line_inside.SetAndObserveTransformNodeID(transNode.GetID())
    line_inside.SetCurveTypeToShortestDistanceOnSurface(slicer.util.getNode('Femur'))
    line_inside.ResampleCurveSurface(2, slicer.util.getNode('Femur'))
    self.points_inside = slicer.util.arrayFromMarkupsControlPoints(line_inside)
    slicer.mrmlScene.RemoveNode(line_outside)
    slicer.mrmlScene.RemoveNode(line_inside)

  # 计算当前最低值及角度
  def caculateLowPoint(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
    TibiaJGM = self.GetTransPoint('胫骨截骨面')
    d = []
    trans = slicer.util.arrayFromTransformMatrix(slicer.util.getNode('DianjiToTracker1'))
    for i in range(len(self.points_outside)):
      l = [self.points_outside[i][0], self.points_outside[i][1], self.points_outside[i][2], 1]
      mov = np.dot(trans, l)[0:3]
      d.append(self.point2area_distance(TibiaJGM, mov))
    d_outside = min(d)
    closet_outside=self.points_outside[d.index(d_outside)]
    d = []
    for i in range(len(self.points_inside)):
      l = [self.points_inside[i][0], self.points_inside[i][1], self.points_inside[i][2], 1]
      mov = np.dot(trans, l)[0:3]
      d.append(self.point2area_distance(TibiaJGM, mov))
    d_inside = min(d)
    closet_inside = self.points_inside[d.index(d_inside)]
    Femur_ZAxis_Z = self.caculateTouYingNorml('Femur_ZAxis', 'Tibia_YZPlane')
    Tibia_ZAxis = self.GetNorlm('Tibia_ZAxis')
    #quxiAngle = self.angle(Femur_ZAxis_Z, Tibia_ZAxis)
    quxiAngle=self.Angle(Femur_ZAxis_Z, Tibia_ZAxis)
    Ifzf1=np.dot(Femur_ZAxis_Z,self.GetNorlm('Tibia_YAxis'))
    Femur_XAxis_Z = self.caculateTouYingNorml('Femur_ZAxis', 'Tibia_XZPlane')
    Ifzf2 = np.dot(Femur_XAxis_Z, self.GetNorlm('Tibia_XAxis'))
    #Tibia_XAxis = self.GetNorlm('Tibia_XAxis')
    if Ifzf2 < 0:
      waifanAngle = -float(self.angle(Femur_XAxis_Z, Tibia_ZAxis))
    else:
      waifanAngle = float(self.angle(Femur_XAxis_Z, Tibia_ZAxis))
    self.currentY=waifanAngle
    if Ifzf1<0:
      self.pyqt_data_x[10-int(float(quxiAngle))] = -float(quxiAngle)
      self.pyqt_data_y1[10-int(float(quxiAngle))] = -float(d_inside)
      self.pyqt_data_y2[10-int(float(quxiAngle))] = float(d_outside)
      self.currentX=-float(quxiAngle)
    else:
      self.currentX=float(quxiAngle)
    self.pyqt_data_x[int(float(quxiAngle))+10]=float(quxiAngle)
    self.pyqt_data_y1[int(float(quxiAngle))+10]=-float(d_inside)
    self.pyqt_data_y2[int(float(quxiAngle))+10] = float(d_outside)
    
    


    # print('内侧间隙，', '外侧间隙，', '屈膝角度，', '外翻角度')
    # print(d_inside, d_outside, quxiAngle, waifanAngle)
    # print('Ifzf',Ifzf)
    if len(self.ui.GraphImage.children())==2:
      try:
        self.updatePyqtgraph()
      except:
        pass
      self.updataJieGuJianxi(closet_outside,closet_inside,trans,TibiaJGM)

  def updataJieGuJianxi(self,closet_outside,closet_inside,trans,TibiaJGM):
    a=[closet_outside[0],closet_outside[1],closet_outside[2],1]
    WaiCePoint = np.dot(trans,a)[0:3]
    WaiCePoint1 = self.TouYing(TibiaJGM,WaiCePoint)
    WaiCeLine = slicer.util.getNode('OutSide')
    WaiCeLine.SetNthControlPointPosition(0,WaiCePoint)
    WaiCeLine.SetNthControlPointPosition(1,WaiCePoint1)
    b = [closet_inside[0],closet_inside[1],closet_inside[2],1]
    NeiCePoint = np.dot(trans,b)[0:3]
    NeiCePoint1 = self.TouYing(TibiaJGM,NeiCePoint)
    NeiCeLine = slicer.util.getNode('InSide')
    NeiCeLine.SetNthControlPointPosition(0,NeiCePoint)
    NeiCeLine.SetNthControlPointPosition(1,NeiCePoint1)

  #计算两点在一平面的投影角度
  def caculateTouYingNorml(self, NName, PlaneName):
    PlaneNode = self.GetTransPoint(PlaneName)
    ras1 = [0, 0, 0]
    ras2 = [0, 0, 0]
    slicer.util.getNode(NName).GetNthControlPointPositionWorld(0, ras1)
    slicer.util.getNode(NName).GetNthControlPointPositionWorld(1, ras2)
    n = np.array(self.TouYing(PlaneNode, ras2)) - np.array(self.TouYing(PlaneNode, ras1))
    return n

  # 计算两点点列世界坐标系构成的向量
  def GetNorlm(self, NodeName):
    ras1 = [0, 0, 0]
    ras2 = [0, 0, 0]
    slicer.util.getNode(NodeName).GetNthControlPointPositionWorld(0, ras1)
    slicer.util.getNode(NodeName).GetNthControlPointPositionWorld(1, ras2)
    n1 = np.array([[ras1[0], ras1[1], ras1[2]]])
    n2 = np.array([[ras2[0], ras2[1], ras2[2]]])
    n = n2 - n1
    return n[0]

  #正常三维视窗状态设置
  def ThreeDState(self):
    #初始化每个三维视窗名字
    if self.interactorNum == 0:
        self.view1 = slicer.app.layoutManager().threeDWidget('View1').threeDView()
        self.view2 = slicer.app.layoutManager().threeDWidget('View2').threeDView()
        self.view3 = slicer.app.layoutManager().threeDWidget('View3').threeDView()
        self.interactorStyle1 = self.view1.interactorStyle()
        self.interactor1 = self.interactorStyle1.GetInteractor()
        self.interactorStyle2 = self.view2.interactorStyle()
        self.interactor2 = self.interactorStyle2.GetInteractor()
        self.interactorStyle3 = self.view3.interactorStyle()
        self.interactor3 = self.interactorStyle3.GetInteractor()
        self.interactorNum =1
    self.interactorStyle1.SetInteractor(self.interactor1)
    self.interactorStyle2.SetInteractor(self.interactor2)
    self.interactorStyle3.SetInteractor(self.interactor3)

  # 隐藏节点
  def HideNode(self,name):
      try:
          slicer.util.getNode(name).SetDisplayVisibility(False)
      except Exception as e:
          print(e)
  #显示节点
  def ShowNode(self,name):
    try:
      slicer.util.getNode(name).SetDisplayVisibility(True)
    except Exception as e:
      print(e)
  # 隐藏所有控件
  def HideAll(self):
    self.ui.GuGe.setVisible(False)#骨骼参数
    self.ui.ReportWidget.setVisible(False)#手术报告
    self.ui.head1.setVisible(False)#骨头调整上面的显示与隐藏        
    slicer.modules.popup.widgetRepresentation().hide()#股骨调整
    slicer.modules.tibiapopup.widgetRepresentation().hide()#胫骨调整
    slicer.modules.viewselect.widgetRepresentation().hide()#股骨视图选择
    slicer.modules.tibiaviewselect.widgetRepresentation().hide()#胫骨视图选择
    self.ui.OperationPlanWidget.setVisible(False)#存放小部件内容
    self.ui.PopupWidget.setVisible(False)#存放模块内容
    self.ui.DriveJZWidget.setVisible(False)
    self.ui.FemurQGWidget.setVisible(False)
    self.ui.Graph.setVisible(False)
    self.ui.ForceWidget.setVisible(False)
  #隐藏分割过程中产生的各个部分
  def HidePart(self):
    self.HideNode('股骨切割')
    self.HideNode('Femur')
    self.HideNode('Tibia')
    self.HideNode('胫骨近端')
    self.HideNode('胫骨切割')
    self.HideNode('股骨远端')
    self.HideNode('部件1')
    self.HideNode('部件2')
    self.HideNode('部件3')
    self.HideNode('部件4')
    self.HideNode('部件5')
    self.HideNode('部件6')
    self.HideNode('H点')
    self.HideNode('股骨头球心')
    self.HideNode('A点')
    self.HideNode('内侧后髁')
    self.HideNode('外侧后髁')
    self.HideNode('外侧远端')
    self.HideNode('内侧远端')
    self.HideNode('开髓点')
    self.HideNode('外侧凸点')
    self.HideNode('内侧凹点')
    self.HideNode('外侧皮质高点')
    self.HideNode('外侧高点')
    self.HideNode('内侧高点')
    self.HideNode('胫骨隆凸')
    self.HideNode('胫骨结节')
    self.HideNode('踝穴中心')
    self.HideNode('内侧凸点')

    try:
      self.TibiaJiaTiload.SetDisplayVisibility(False)
    except Exception as e:
      print(e)
    try:
      self.jiatiload.SetDisplayVisibility(False)
    except Exception as e:
      print(e)
    try:
      self.ChenDian.SetDisplayVisibility(False)
    except Exception as e:
      print(e)
  
  #--------------------------------------功能函数-----------------------------------------------
  # 获取transform下的点
  def GetTransPoint(self, node):
    point1, point2, point3 = [0, 0, 0], [0, 0, 0], [0, 0, 0]
    slicer.util.getNode(node).GetNthControlPointPositionWorld(0, point1)
    slicer.util.getNode(node).GetNthControlPointPositionWorld(1, point2)
    slicer.util.getNode(node).GetNthControlPointPositionWorld(2, point3)
    zb = np.array([point1, point2, point3])
    return zb
  # 三个点确定平面方程
  def define_area(self, a):
    point1 = a[0]
    point2 = a[1]
    point3 = a[2]
    AB = np.asmatrix(point2 - point1)
    AC = np.asmatrix(point3 - point1)
    N = np.cross(AB, AC)  # 向量叉乘，求法向量
    # Ax+By+Cz
    Ax = N[0, 0]
    By = N[0, 1]
    Cz = N[0, 2]
    D = -(Ax * point1[0] + By * point1[1] + Cz * point1[2])
    return Ax, By, Cz, D
  # 点到面的距离
  def point2area_distance(self, a, point4):
    Ax, By, Cz, D = self.define_area(a)
    mod_d = Ax * point4[0] + By * point4[1] + Cz * point4[2] + D
    mod_area = np.sqrt(np.sum(np.square([Ax, By, Cz])))
    d = abs(mod_d) / mod_area
    return d
  # 获得投影点（a为三个点确定的平面，point为要获得投影点的点）
  def TouYing(self, a, point):
    Ax, By, Cz, D = self.define_area(a)
    k = (Ax * point[0] + By * point[1] + Cz * point[2] + D) / (np.sum(np.square([Ax, By, Cz])))
    b = [point[0] - k * Ax, point[1] - k * By, point[2] - k * Cz]
    return b
  # 求角度-传递两个向量（求两个向量的夹角）
  def Angle(self, xiangliang1, xiangliang2):
    import math
    cosa = np.dot(xiangliang1, xiangliang2)/math.sqrt(np.dot(xiangliang1,xiangliang1))/math.sqrt(np.dot(xiangliang2, xiangliang2))
    a = math.degrees(math.acos(cosa))
    return a
  #旋转角度变换
  def GetMarix(self,trans,jd,point):
    import math
    jd = math.radians(jd)
    trans_ni=np.linalg.inv(trans)
    Tjxlx=[1,0,0]
    xzjz = [[math.cos(jd) + Tjxlx[0] * Tjxlx[0] * (1 - math.cos(jd)),
                -Tjxlx[2] * math.sin(jd) + Tjxlx[0] * Tjxlx[1] * (1 - math.cos(jd)),
                Tjxlx[1] * math.sin(jd) + Tjxlx[0] * Tjxlx[2] * (1 - math.cos(jd)), 0],
            [Tjxlx[2] * math.sin(jd) + Tjxlx[0] * Tjxlx[1] * (1 - math.cos(jd)),
                math.cos(jd) + Tjxlx[1] * Tjxlx[1] * (1 - math.cos(jd)),
                -Tjxlx[0] * math.sin(jd) + Tjxlx[1] * Tjxlx[2] * (1 - math.cos(jd)), 0],
            [-Tjxlx[1] * math.sin(jd) + Tjxlx[0] * Tjxlx[2] * (1 - math.cos(jd)),
                Tjxlx[0] * math.sin(jd) + Tjxlx[1] * Tjxlx[2] * (1 - math.cos(jd)),
                math.cos(jd) + Tjxlx[2] * Tjxlx[2] * (1 - math.cos(jd)), 0],
            [0, 0, 0, 1]]
    
    point=np.array([point[0],point[1],point[2],1])
    point_tmp1=np.dot(trans_ni,point)
    point_tmp2=np.dot(xzjz,point_tmp1)
    point=np.dot(trans,point_tmp2)
    return point[0:3]

  def GetMarix_z(self,jd):
    import math
    jd = math.radians(jd)
    Tjxlx=[0,0,1]
    xzjz = np.array([[math.cos(jd) + Tjxlx[0] * Tjxlx[0] * (1 - math.cos(jd)),
                -Tjxlx[2] * math.sin(jd) + Tjxlx[0] * Tjxlx[1] * (1 - math.cos(jd)),
                Tjxlx[1] * math.sin(jd) + Tjxlx[0] * Tjxlx[2] * (1 - math.cos(jd)), 0],
            [Tjxlx[2] * math.sin(jd) + Tjxlx[0] * Tjxlx[1] * (1 - math.cos(jd)),
                math.cos(jd) + Tjxlx[1] * Tjxlx[1] * (1 - math.cos(jd)),
                -Tjxlx[0] * math.sin(jd) + Tjxlx[1] * Tjxlx[2] * (1 - math.cos(jd)), 0],
            [-Tjxlx[1] * math.sin(jd) + Tjxlx[0] * Tjxlx[2] * (1 - math.cos(jd)),
                Tjxlx[0] * math.sin(jd) + Tjxlx[1] * Tjxlx[2] * (1 - math.cos(jd)),
                math.cos(jd) + Tjxlx[2] * Tjxlx[2] * (1 - math.cos(jd)), 0],
            [0, 0, 0, 1]])
    return xzjz

  def GetMarix_x(self,jd):
        jd = math.radians(jd)
        Tjxlx=[1,0,0]
        xzjz = np.array([[math.cos(jd) + Tjxlx[0] * Tjxlx[0] * (1 - math.cos(jd)),
                    -Tjxlx[2] * math.sin(jd) + Tjxlx[0] * Tjxlx[1] * (1 - math.cos(jd)),
                    Tjxlx[1] * math.sin(jd) + Tjxlx[0] * Tjxlx[2] * (1 - math.cos(jd)), 0],
                [Tjxlx[2] * math.sin(jd) + Tjxlx[0] * Tjxlx[1] * (1 - math.cos(jd)),
                    math.cos(jd) + Tjxlx[1] * Tjxlx[1] * (1 - math.cos(jd)),
                    -Tjxlx[0] * math.sin(jd) + Tjxlx[1] * Tjxlx[2] * (1 - math.cos(jd)), 0],
                [-Tjxlx[1] * math.sin(jd) + Tjxlx[0] * Tjxlx[2] * (1 - math.cos(jd)),
                    Tjxlx[0] * math.sin(jd) + Tjxlx[1] * Tjxlx[2] * (1 - math.cos(jd)),
                    math.cos(jd) + Tjxlx[2] * Tjxlx[2] * (1 - math.cos(jd)), 0],
                [0, 0, 0, 1]])
        return xzjz
  
  #------------------------------------股骨相机--------------------------------------------------
  #--------------------------------三维视图上的按钮---------------------------------------------
  # 视图1按钮
  def onV1Button(self):
    self.V2Button.click()
    self.V2Button.click()
    if (self.V1Button.toolTip=='<p>锁定</p>'):
      self.interactorStyle1.SetInteractor(None)
      self.V1Button.setToolTip('解锁')

    else:
      self.interactorStyle1.SetInteractor(self.interactor1)
      self.V1Button.setToolTip('锁定')
  # 视图2按钮
  def onV2Button(self):
    cameraNode=self.view1.cameraNode()
    cameraNode2 = self.view2.cameraNode()
    if slicer.modules.NoImageWelcomeWidget.judge == 'L':
        if (cameraNode.GetName() == 'FC1'):
            if (cameraNode2.GetName() == 'FC2'):
                self.Camera1(self.view2)
                self.Camera2(self.view1)
                self.DeleteTip(self.view1, self.view2, self.view3)
                self.Camera1Tip(self.view2)
                self.Camera2Tip(self.view1)
                self.Camera3Tip(self.view3)
            else:
                self.Camera1(self.view2)
                self.Camera3(self.view1)
                self.DeleteTip(self.view1, self.view3, self.view2)
                self.Camera1Tip(self.view2)
                self.Camera2Tip(self.view3)
                self.Camera3Tip(self.view1)

        elif (cameraNode.GetName() == 'FC2'):
            if (cameraNode2.GetName() == 'FC1'):
                self.Camera1(self.view1)
                self.Camera2(self.view2)
                self.DeleteTip(self.view2, self.view1, self.view3)
                self.Camera1Tip(self.view1)
                self.Camera2Tip(self.view2)
                self.Camera3Tip(self.view3)
            else:
                self.Camera3(self.view1)
                self.Camera2(self.view2)
                self.DeleteTip(self.view3, self.view1, self.view2)
                self.Camera1Tip(self.view3)
                self.Camera2Tip(self.view2)
                self.Camera3Tip(self.view1)
        else:
            if (cameraNode2.GetName() == 'FC2'):
                self.Camera3(self.view2)
                self.Camera2(self.view1)
                self.DeleteTip(self.view3, self.view2, self.view1)
                self.Camera1Tip(self.view3)
                self.Camera2Tip(self.view1)
                self.Camera3Tip(self.view2)
            else:
                self.Camera3(self.view2)
                self.Camera1(self.view1)
                self.DeleteTip(self.view2, self.view3, self.view1)
                self.Camera1Tip(self.view1)
                self.Camera2Tip(self.view3)
                self.Camera3Tip(self.view2)
    else:
        if (cameraNode.GetName() == 'FC1'):
            if (cameraNode2.GetName() == 'FC2'):
                self.Camera1(self.view2)
                self.Camera2(self.view1)
                self.DeleteTip(self.view1, self.view2, self.view3)
                self.Camera1TipRight(self.view2)
                self.Camera2Tip(self.view1)
                self.Camera3TipRight(self.view3)
            else:
                self.Camera1(self.view2)
                self.Camera3(self.view1)
                self.DeleteTip(self.view1, self.view3, self.view2)
                self.Camera1TipRight(self.view2)
                self.Camera2Tip(self.view3)
                self.Camera3TipRight(self.view1)

        elif (cameraNode.GetName() == 'FC2'):
            if (cameraNode2.GetName() == 'FC1'):
                self.Camera1(self.view1)
                self.Camera2(self.view2)
                self.DeleteTip(self.view2, self.view1, self.view3)
                self.Camera1TipRight(self.view1)
                self.Camera2Tip(self.view2)
                self.Camera3TipRight(self.view3)
            else:
                self.Camera3(self.view1)
                self.Camera2(self.view2)
                self.DeleteTip(self.view3, self.view1, self.view2)
                self.Camera1TipRight(self.view3)
                self.Camera2Tip(self.view2)
                self.Camera3TipRight(self.view1)
        else:
            if (cameraNode2.GetName() == 'FC2'):
                self.Camera3(self.view2)
                self.Camera2(self.view1)
                self.DeleteTip(self.view3, self.view2, self.view1)
                self.Camera1TipRight(self.view3)
                self.Camera2Tip(self.view1)
                self.Camera3TipRight(self.view2)
            else:
                self.Camera3(self.view2)
                self.Camera1(self.view1)
                self.DeleteTip(self.view2, self.view3, self.view1)
                self.Camera1TipRight(self.view1)
                self.Camera2Tip(self.view3)
                self.Camera3TipRight(self.view2)
  # 视图3按钮
  def onV3Button(self):
    cameraNode = self.view1.cameraNode()
    cameraNode3 = self.view3.cameraNode()
    if slicer.modules.NoImageWelcomeWidget.judge == 'L':
        if (cameraNode.GetName() == 'FC1'):
            if (cameraNode3.GetName() == 'FC2'):
                self.Camera1(self.view3)
                self.Camera2(self.view1)
                self.DeleteTip(self.view1, self.view3, self.view2)
                self.Camera1Tip(self.view3)
                self.Camera2Tip(self.view1)
                self.Camera3Tip(self.view2)
            else:
                self.Camera1(self.view3)
                self.Camera3(self.view1)
                self.DeleteTip(self.view1, self.view2, self.view3)
                self.Camera1Tip(self.view3)
                self.Camera2Tip(self.view2)
                self.Camera3Tip(self.view1)
        elif (cameraNode.GetName() == 'FC2'):
            if (cameraNode3.GetName() == 'FC1'):
                self.Camera1(self.view1)
                self.Camera2(self.view3)
                self.DeleteTip(self.view2, self.view3, self.view1)
                self.Camera1Tip(self.view1)
                self.Camera2Tip(self.view3)
                self.Camera3Tip(self.view2)
            else:
                self.Camera3(self.view1)
                self.Camera2(self.view3)
                self.DeleteTip(self.view2, self.view1, self.view3)
                self.Camera1Tip(self.view2)
                self.Camera2Tip(self.view3)
                self.Camera3Tip(self.view1)
        else:
            if (cameraNode3.GetName() == 'FC2'):
                self.Camera3(self.view3)
                self.Camera2(self.view1)
                self.DeleteTip(self.view1, self.view2, self.view3)
                self.Camera1Tip(self.view2)
                self.Camera2Tip(self.view1)
                self.Camera3Tip(self.view3)
            else:
                self.Camera3(self.view3)
                self.Camera1(self.view1)
                self.DeleteTip(self.view3, self.view2, self.view1)
                self.Camera1Tip(self.view1)
                self.Camera2Tip(self.view2)
                self.Camera3Tip(self.view3)
    else:
        if (cameraNode.GetName() == 'FC1'):
            if (cameraNode3.GetName() == 'FC2'):
                self.Camera1(self.view3)
                self.Camera2(self.view1)
                self.DeleteTip(self.view1, self.view3, self.view2)
                self.Camera1TipRight(self.view3)
                self.Camera2Tip(self.view1)
                self.Camera3TipRight(self.view2)
            else:
                self.Camera1(self.view3)
                self.Camera3(self.view1)
                self.DeleteTip(self.view1, self.view2, self.view3)
                self.Camera1TipRight(self.view3)
                self.Camera2Tip(self.view2)
                self.Camera3TipRight(self.view1)
        elif (cameraNode.GetName() == 'FC2'):
            if (cameraNode3.GetName() == 'FC1'):
                self.Camera1(self.view1)
                self.Camera2(self.view3)
                self.DeleteTip(self.view2, self.view3, self.view1)
                self.Camera1TipRight(self.view1)
                self.Camera2Tip(self.view3)
                self.Camera3TipRight(self.view2)
            else:
                self.Camera3(self.view1)
                self.Camera2(self.view3)
                self.DeleteTip(self.view2, self.view1, self.view3)
                self.Camera1TipRight(self.view2)
                self.Camera2Tip(self.view3)
                self.Camera3TipRight(self.view1)
        else:
            if (cameraNode3.GetName() == 'FC2'):
                self.Camera3(self.view3)
                self.Camera2(self.view1)
                self.DeleteTip(self.view1, self.view2, self.view3)
                self.Camera1TipRight(self.view2)
                self.Camera2Tip(self.view1)
                self.Camera3TipRight(self.view3)
            else:
                self.Camera3(self.view3)
                self.Camera1(self.view1)
                self.DeleteTip(self.view3, self.view2, self.view1)
                self.Camera1TipRight(self.view1)
                self.Camera2Tip(self.view2)
                self.Camera3TipRight(self.view3)

  #-------------------------------------三维视图上相机的位置----------------------------------------
  #股骨相机1
  def Camera1(self, view):
    cameraNode = view.cameraNode()
    positiontmp = np.array([0, -500, 0, 1])
    Ftrans1 = np.array([[-1, 0, 0, 0],
                        [0, -1, 0, 0],
                        [0, 0, 1, 0],
                        [0, 0, 0, 1]])
    transNode = slicer.util.getNode('DianjiToTracker1')
    Ftrans_dj = slicer.util.arrayFromTransformMatrix(transNode)
    Ftrans1=np.dot(Ftrans_dj,Ftrans1)
    Ftrans3 = np.dot(Ftrans1, self.Ftrans2)
    position1 = np.dot(Ftrans3, positiontmp)
    viewUpDirection = (float(Ftrans3[0][2]), float(Ftrans3[1][2]), float(Ftrans3[2][2]))
    focalPoint1 = [Ftrans3[0][3], Ftrans3[1][3], Ftrans3[2][3]]
    cameraNode.SetFocalPoint(focalPoint1)
    cameraNode.SetPosition(position1[0], position1[1], position1[2])
    cameraNode.SetViewUp(viewUpDirection)
    cameraNode.SetName('FC1')

  #股骨相机2
  def Camera2(self, view):
    cameraNode = view.cameraNode()
    positiontmp = np.array([-500, 0, 0, 1])
    Ftrans1 = np.array([[-1, 0, 0, 0],
                        [0, -1, 0, 0],
                        [0, 0, 1, 0],
                        [0, 0, 0, 1]])
    transNode = slicer.util.getNode('DianjiToTracker1')
    Ftrans_dj = slicer.util.arrayFromTransformMatrix(transNode)
    Ftrans1 = np.dot(Ftrans_dj, Ftrans1)
    Ftrans3 = np.dot(Ftrans1, self.Ftrans2)
    position2 = np.dot(Ftrans3, positiontmp)
    viewUpDirection = (float(Ftrans3[0][2]), float(Ftrans3[1][2]), float(Ftrans3[2][2]))
    focalPoint2 = [Ftrans3[0][3], Ftrans3[1][3], Ftrans3[2][3]]
    cameraNode.SetFocalPoint(focalPoint2)
    cameraNode.SetPosition(position2[0], position2[1], position2[2])
    cameraNode.SetViewUp(viewUpDirection)
    cameraNode.SetName('FC2')

  #股骨相机3
  def Camera3(self, view):
    cameraNode = view.cameraNode()
    positiontmp = np.array([0, 0, -500, 1])
    Ftrans1 = np.array([[-1, 0, 0, 0],
                        [0, -1, 0, 0],
                        [0, 0, 1, 0],
                        [0, 0, 0, 1]])
    transNode = slicer.util.getNode('DianjiToTracker1')
    Ftrans_dj = slicer.util.arrayFromTransformMatrix(transNode)
    Ftrans1=np.dot(Ftrans_dj,Ftrans1)
    Ftrans3 = np.dot(Ftrans1, self.Ftrans2)
    position3 = np.dot(Ftrans3, positiontmp)
    viewUpDirection = (-float(Ftrans3[0][1]), -float(Ftrans3[1][1]), -float(Ftrans3[2][1]))
    focalPoint3 = [Ftrans3[0][3], Ftrans3[1][3], Ftrans3[2][3]]
    cameraNode.SetFocalPoint(focalPoint3)
    cameraNode.SetPosition(position3[0], position3[1], position3[2])
    cameraNode.SetViewUp(viewUpDirection)
    cameraNode.SetName('FC3')
  #-----------------------------------三维视图上的注释（根据左右腿分内外两侧）--------------------------------
  #相机1注释
  def Camera1Tip(self, view):
      V11 = qt.QLabel(view)
      V12 = qt.QLabel(view)
      V13 = qt.QLabel(view)
      self.V14 = qt.QLabel(view)
      V15 = qt.QLabel(view)
      self.V16 = qt.QLabel(view)
      V17 = qt.QLabel(view)
      V18 = qt.QLabel(view)
      V19 = qt.QLabel(view)
      self.V1A = qt.QLabel(view)
      V1B = qt.QLabel(view)
      self.V1C = qt.QLabel(view)

      V11.setObjectName('1')
      V12.setObjectName('2')
      V13.setObjectName('3')
      self.V14.setObjectName('4')
      V15.setObjectName('5')
      self.V16.setObjectName('6')
      V17.setObjectName('7')
      V18.setObjectName('8')
      V19.setObjectName('9')
      self.V1A.setObjectName('10')
      V1B.setObjectName('11')
      self.V1C.setObjectName('12')

      V11.setGeometry(view.contentsRect().width() - 100, 25, 100, 40)
      V11.setText(" 外翻/内翻 ")
      V11.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      V11.show()
      try:
          V12.setGeometry(view.contentsRect().width() - 100, 50, 100, 25)
          V12.setText(' '+str(round(self.WaiFanJiao,1))+'°')
          V12.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
          V12.show()
      except Exception as e:
          print(e)

      V13.setGeometry(0, view.contentsRect().height() - 125, 100, 25)
      V13.setText(" 内侧远端截骨 ")
      V13.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      V13.show()
      self.V14.setGeometry(0, view.contentsRect().height() - 100, 100, 25)
      try:
          self.V14.setText(' '+ str(round(slicer.modules.PopupWidget.FemurNeiCeYuanDuan, 1))+ 'mm')
      except Exception as e:
          print(e)
      self.V14.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      self.V14.show()
      V15.setGeometry(0, view.contentsRect().height() - 75, 100, 25)
      V15.setText(" 内侧伸直间隙 ")
      V15.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      V15.show()
      self.V16.setGeometry(0, view.contentsRect().height() - 50, 100, 25)
      self.V16.setText("")
      self.V16.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      self.V16.show()
      V17.setGeometry(5, 0.5 * view.contentsRect().height(), 100, 40)
      V17.setText("内 ")
      V17.setStyleSheet('QLabel{background-color:transparent;color:#A9A9A9;font:30px;}')
      V17.show()
      V18.setGeometry(view.contentsRect().width() - 50, 0.5 * view.contentsRect().height(), 100, 40)
      V18.setText(" 外 ")
      V18.setStyleSheet('QLabel{background-color:transparent;color:#A9A9A9;font:30px;}')
      V18.show()
      V19.setGeometry(view.contentsRect().width() - 100, view.contentsRect().height() - 125, 100, 25)
      V19.setText(" 外侧远端截骨 ")
      V19.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      V19.show()
      self.V1A.setGeometry(view.contentsRect().width() - 100, view.contentsRect().height() - 100, 100, 25)
      try:
          self.V1A.setText(' ' + str(round(slicer.modules.PopupWidget.FemurWaiCeYuanDuan, 1)) + 'mm')
      except Exception as e:
          print(e)
      self.V1A.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      self.V1A.show()
      V1B.setGeometry(view.contentsRect().width() - 100, view.contentsRect().height() - 75, 100, 25)
      V1B.setText(" 外侧伸直间隙 ")
      V1B.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      V1B.show()
      self.V1C.setGeometry(view.contentsRect().width() - 100, view.contentsRect().height() - 50, 100, 25)
      self.V1C.setText("")
      self.V1C.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      self.V1C.show()

  def Camera1TipRight(self, view):
      V11 = qt.QLabel(view)
      V12 = qt.QLabel(view)
      V13 = qt.QLabel(view)
      self.V14 = qt.QLabel(view)
      V15 = qt.QLabel(view)
      self.V16 = qt.QLabel(view)
      V17 = qt.QLabel(view)
      V18 = qt.QLabel(view)
      V19 = qt.QLabel(view)
      self.V1A = qt.QLabel(view)
      V1B = qt.QLabel(view)
      self.V1C = qt.QLabel(view)

      V11.setObjectName('1')
      V12.setObjectName('2')
      V13.setObjectName('3')
      self.V14.setObjectName('4')
      V15.setObjectName('5')
      self.V16.setObjectName('6')
      V17.setObjectName('7')
      V18.setObjectName('8')
      V19.setObjectName('9')
      self.V1A.setObjectName('10')
      V1B.setObjectName('11')
      self.V1C.setObjectName('12')

      V11.setGeometry(view.contentsRect().width() - 100, 25, 100, 40)
      V11.setText(" 外翻/内翻 ")
      V11.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      V11.show()
      try:
          V12.setGeometry(view.contentsRect().width() - 100, 50, 100, 25)
          V12.setText(' '+str(round(self.WaiFanJiao,1))+'°')
          V12.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
          V12.show()
      except Exception as e:
          print(e)

      V13.setGeometry(view.contentsRect().width() - 100, view.contentsRect().height() - 125, 100, 25)
      V13.setText(" 内侧远端截骨 ")
      V13.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      V13.show()
      self.V14.setGeometry(view.contentsRect().width() - 100, view.contentsRect().height() - 100, 100, 25)
      try:
          self.V14.setText(' '+ str(round(slicer.modules.PopupWidget.FemurNeiCeYuanDuan, 1))+ 'mm')
      except Exception as e:
          print(e)
      self.V14.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      self.V14.show()
      V15.setGeometry(view.contentsRect().width() - 100, view.contentsRect().height() - 75, 100, 25)
      V15.setText(" 内侧伸直间隙 ")
      V15.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      V15.show()
      self.V16.setGeometry(view.contentsRect().width() - 100, view.contentsRect().height() - 50, 100, 25)
      self.V16.setText("")
      self.V16.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      self.V16.show()
      V17.setGeometry(view.contentsRect().width() - 50, 0.5 * view.contentsRect().height(), 100, 40)
      V17.setText(" 内 ")
      V17.setStyleSheet('QLabel{background-color:transparent;color:#A9A9A9;font:30px;}')
      V17.show()
      V18.setGeometry( 5, 0.5 * view.contentsRect().height(), 100, 40)
      V18.setText("外 ")
      V18.setStyleSheet('QLabel{background-color:transparent;color:#A9A9A9;font:30px;}')
      V18.show()
      V19.setGeometry(0, view.contentsRect().height() - 125, 100, 25)
      V19.setText(" 外侧远端截骨 ")
      V19.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      V19.show()
      self.V1A.setGeometry( 0, view.contentsRect().height() - 100, 100, 25)
      try:
          self.V1A.setText(' ' + str(round(slicer.modules.PopupWidget.FemurWaiCeYuanDuan, 1)) + 'mm')
      except Exception as e:
          print(e)
      self.V1A.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      self.V1A.show()
      V1B.setGeometry( 0, view.contentsRect().height() - 75, 100, 25)
      V1B.setText(" 外侧伸直间隙 ")
      V1B.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      V1B.show()
      self.V1C.setGeometry( 0, view.contentsRect().height() - 50, 100, 25)
      self.V1C.setText("")
      self.V1C.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      self.V1C.show()

  #相机2注释
  def Camera2Tip(self, view):
      V11 = qt.QLabel(view)
      self.V12 = qt.QLabel(view)
      V13 = qt.QLabel(view)
      V14 = qt.QLabel(view)

      V11.setObjectName('13')
      self.V12.setObjectName('14')
      V13.setObjectName('15')
      V14.setObjectName('16')

      V11.setGeometry(view.contentsRect().width() - 100, 25, 100, 40)
      V11.setText(" 前倾/后倾 ")
      V11.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      V11.show()
      
      try:
          self.V12.setGeometry(view.contentsRect().width() - 100, 50, 100, 25)
          self.V12.setText(' '+str(round(slicer.modules.PopupWidget.HouQingJiao,1))+'°')
          self.V12.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
          self.V12.show()
          
      except Exception as e:
          print(e)
      V13.setGeometry(5, 0.5*view.contentsRect().height(), 100, 40)
      V13.setText("前 ")
      V13.setStyleSheet('QLabel{background-color:transparent;color:#A9A9A9;font:30px;}')
      V13.show()
      V14.setGeometry(view.contentsRect().width() - 50, 0.5*view.contentsRect().height(), 100, 40)
      V14.setText(" 后 ")
      V14.setStyleSheet('QLabel{background-color:transparent;color:#A9A9A9;font:30px;}')
      V14.show()

  def Camera2TipRight(self, view):
      V11 = qt.QLabel(view)
      self.V12 = qt.QLabel(view)
      V13 = qt.QLabel(view)
      V14 = qt.QLabel(view)

      V11.setObjectName('13')
      self.V12.setObjectName('14')
      V13.setObjectName('15')
      V14.setObjectName('16')

      V11.setGeometry(view.contentsRect().width() - 100, 25, 100, 40)
      V11.setText(" 前倾/后倾 ")
      V11.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      V11.show()

      try:
          self.V12.setGeometry(view.contentsRect().width() - 100, 50, 100, 25)
          self.V12.setText(' ' + str(round(slicer.modules.PopupWidget.HouQingJiao, 1)) + '°')
          self.V12.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
          self.V12.show()

      except Exception as e:
          print(e)
      V13.setGeometry(5, 0.5*view.contentsRect().height(), 100, 40)
      V13.setText("前 ")
      V13.setStyleSheet('QLabel{background-color:transparent;color:#A9A9A9;font:30px;}')
      V13.show()
      V14.setGeometry(view.contentsRect().width() - 50, 0.5*view.contentsRect().height(), 100, 40)
      V14.setText(" 后 ")
      V14.setStyleSheet('QLabel{background-color:transparent;color:#A9A9A9;font:30px;}')
      V14.show()

  #相机3注释
  def Camera3Tip(self, view):
      V21 = qt.QLabel(view)
      self.V22 = qt.QLabel(view)
      V23 = qt.QLabel(view)
      self.V24 = qt.QLabel(view)
      V25 = qt.QLabel(view)
      self.V26 = qt.QLabel(view)
      V27 = qt.QLabel(view)
      V28 = qt.QLabel(view)
      V29 = qt.QLabel(view)
      self.V2A = qt.QLabel(view)
      V2B = qt.QLabel(view)
      self.V2C = qt.QLabel(view)

      V21.setObjectName('1')
      self.V22.setObjectName('2')
      V23.setObjectName('3')
      self.V24.setObjectName('4')
      V25.setObjectName('5')
      self.V26.setObjectName('6')
      V27.setObjectName('7')
      V28.setObjectName('8')
      V29.setObjectName('9')
      self.V2A.setObjectName('10')
      V2B.setObjectName('11')
      self.V2C.setObjectName('12')

      V21.setGeometry(view.contentsRect().width() - 100, 25, 100, 40)
      V21.setText(" 外旋/内旋 ")
      V21.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      V21.show()
      self.V22.setGeometry(view.contentsRect().width() - 100, 50, 100, 25)
      try:

          self.V22.setText(' '+str(round(slicer.modules.PopupWidget.WaiXuanJiao, 1))+'°')

      except Exception as e:
          print(e)
      self.V22.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      self.V22.show()
      V23.setGeometry(0, view.contentsRect().height() - 125, 100, 25)
      V23.setText(" 内侧后髁截骨 ")
      V23.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      V23.show()
      self.V24.setGeometry(0, view.contentsRect().height() - 100, 100, 25)
      try:
          self.V24.setText(
              ' ' + str(round(slicer.modules.PopupWidget.FemurNeiCeHouKe, 1)) + 'mm')
      except Exception as e:
          print(e)

      self.V24.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      self.V24.show()
      V25.setGeometry(0, view.contentsRect().height() - 75, 100, 25)
      V25.setText(" 内侧屈膝截骨 ")
      V25.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      V25.show()
      self.V26.setGeometry(0, view.contentsRect().height() - 50, 100, 25)
      self.V26.setText("")
      self.V26.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      self.V26.show()
      V27.setGeometry(5, 0.5*view.contentsRect().height(), 100, 40)
      V27.setText("内 ")
      V27.setStyleSheet('QLabel{background-color:transparent;color:#A9A9A9;font:30px;}')
      V27.show()
      V28.setGeometry(view.contentsRect().width() - 50, 0.5*view.contentsRect().height(), 100, 40)
      V28.setText(" 外 ")
      V28.setStyleSheet('QLabel{background-color:transparent;color:#A9A9A9;font:30px;}')
      V28.show()
      V29.setGeometry(view.contentsRect().width() - 100, view.contentsRect().height() - 125, 100, 25)
      V29.setText(" 外侧后髁截骨 ")
      V29.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      V29.show()
      self.V2A.setGeometry(view.contentsRect().width() - 100, view.contentsRect().height() - 100, 100, 25)
      try:
          self.V2A.setText( ' ' + str(round(slicer.modules.PopupWidget.FemurWaiCeHouKe, 1)) + 'mm')
      except Exception as e:
          print(e)
      self.V2A.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      self.V2A.show()
      V2B.setGeometry(view.contentsRect().width() - 100, view.contentsRect().height() - 75, 100, 25)
      V2B.setText(" 外侧屈膝截骨 ")
      V2B.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      V2B.show()
      self.V2C.setGeometry(view.contentsRect().width() - 100,view.contentsRect().height() - 50, 100, 25)
      self.V2C.setText("")
      self.V2C.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
      self.V2C.show()

  def Camera3TipRight(self, view):
    V21 = qt.QLabel(view)
    self.V22 = qt.QLabel(view)
    V23 = qt.QLabel(view)
    self.V24 = qt.QLabel(view)
    V25 = qt.QLabel(view)
    self.V26 = qt.QLabel(view)
    V27 = qt.QLabel(view)
    V28 = qt.QLabel(view)
    V29 = qt.QLabel(view)
    self.V2A = qt.QLabel(view)
    V2B = qt.QLabel(view)
    self.V2C = qt.QLabel(view)

    V21.setObjectName('1')
    self.V22.setObjectName('2')
    V23.setObjectName('3')
    self.V24.setObjectName('4')
    V25.setObjectName('5')
    self.V26.setObjectName('6')
    V27.setObjectName('7')
    V28.setObjectName('8')
    V29.setObjectName('9')
    self.V2A.setObjectName('10')
    V2B.setObjectName('11')
    self.V2C.setObjectName('12')

    V21.setGeometry(view.contentsRect().width() - 100, 25, 100, 40)
    V21.setText(" 外旋/内旋 ")
    V21.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V21.show()
    self.V22.setGeometry(view.contentsRect().width() - 100, 50, 100, 25)
    try:

        self.V22.setText(' '+str(round(slicer.modules.PopupWidget.WaiXuanJiao, 1))+'°')

    except Exception as e:
        print(e)
    self.V22.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    self.V22.show()
    V23.setGeometry(view.contentsRect().width() - 100, view.contentsRect().height() - 125, 100, 25)
    V23.setText(" 内侧后髁截骨 ")
    V23.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V23.show()
    self.V24.setGeometry(view.contentsRect().width() - 100, view.contentsRect().height() - 100, 100, 25)
    try:
        self.V24.setText(
            ' ' + str(round(slicer.modules.PopupWidget.FemurNeiCeHouKe, 1)) + 'mm')
    except Exception as e:
        print(e)

    self.V24.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    self.V24.show()
    V25.setGeometry(view.contentsRect().width() - 100, view.contentsRect().height() - 75, 100, 25)
    V25.setText(" 内侧屈曲截骨 ")
    V25.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V25.show()
    self.V26.setGeometry(view.contentsRect().width() - 100,view.contentsRect().height() - 50, 100, 25)
    self.V26.setText("")
    self.V26.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    self.V26.show()
    V27.setGeometry(view.contentsRect().width() - 100, 0.5*view.contentsRect().height(), 100, 40)
    V27.setText(" 内 ")
    V27.setStyleSheet('QLabel{background-color:transparent;color:#A9A9A9;font:30px;}')
    V27.show()
    V28.setGeometry( 5, 0.5*view.contentsRect().height(), 100, 40)
    V28.setText("外 ")
    V28.setStyleSheet('QLabel{background-color:transparent;color:#A9A9A9;font:30px;}')
    V28.show()
    V29.setGeometry( 0, view.contentsRect().height() - 125, 100, 25)
    V29.setText(" 外侧后髁截骨 ")
    V29.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V29.show()
    self.V2A.setGeometry( 0, view.contentsRect().height() - 100, 100, 25)
    try:
        self.V2A.setText( ' ' + str(round(slicer.modules.PopupWidget.FemurWaiCeHouKe, 1)) + 'mm')
    except Exception as e:
        print(e)
    self.V2A.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    self.V2A.show()
    V2B.setGeometry( 0, view.contentsRect().height() - 75, 100, 25)
    V2B.setText(" 外侧屈曲截骨 ")
    V2B.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V2B.show()
    self.V2C.setGeometry( 0, view.contentsRect().height() - 50, 100, 25)
    self.V2C.setText("")
    self.V2C.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    self.V2C.show()
  #------------------------------胫骨相机------------------------------------------------------
  # 胫骨相机1
  def TCamera1(self, view):
    cameraNode = view.cameraNode()
    positiontmp = np.array([0, -500, 0, 1])
    Ttrans1 = np.array([[-1, 0, 0, 0],
                        [0, -1, 0, 0],
                        [0, 0, 1, 0],
                        [0, 0, 0, 1]])
    transNode = slicer.util.getNode('TibiaToTracker')
    Ttrans_dj = slicer.util.arrayFromTransformMatrix(transNode)
    Ttrans1=np.dot(Ttrans_dj,Ttrans1)
    Ttrans3 = np.dot(Ttrans1, self.Ttrans2)
    position1 = np.dot(Ttrans3, positiontmp)
    viewUpDirection = (float(Ttrans3[0][2]), float(Ttrans3[1][2]), float(Ttrans3[2][2]))
    focalPoint1 = [Ttrans3[0][3], Ttrans3[1][3], Ttrans3[2][3]]
    cameraNode.SetFocalPoint(focalPoint1)
    cameraNode.SetPosition(position1[0], position1[1], position1[2])
    cameraNode.SetViewUp(viewUpDirection)
    #self.Camera1(view)
    view.cameraNode().SetName('TC1')

  # 胫骨相机2
  def TCamera2(self, view):
    cameraNode = view.cameraNode()
    positiontmp = np.array([-500, 0, 0, 1])
    Ttrans1 = np.array([[-1, 0, 0, 0],
                        [0, -1, 0, 0],
                        [0, 0, 1, 0],
                        [0, 0, 0, 1]])
    transNode = slicer.util.getNode('TibiaToTracker')
    Ttrans_dj = slicer.util.arrayFromTransformMatrix(transNode)
    Ttrans1=np.dot(Ttrans_dj,Ttrans1)
    Ttrans3 = np.dot(Ttrans1, self.Ttrans2)
    position2 = np.dot(Ttrans3, positiontmp)
    viewUpDirection = (float(Ttrans3[0][2]), float(Ttrans3[1][2]), float(Ttrans3[2][2]))
    focalPoint2 = [Ttrans3[0][3], Ttrans3[1][3], Ttrans3[2][3]]
    cameraNode.SetFocalPoint(focalPoint2)
    cameraNode.SetPosition(position2[0], position2[1], position2[2])
    cameraNode.SetViewUp(viewUpDirection)
    #self.Camera2(view)
    view.cameraNode().SetName('TC2')

  # 胫骨相机3
  def TCamera3(self, view):
    cameraNode = view.cameraNode()
    positiontmp = np.array([0, 0, 500, 1])
    Ttrans1 = np.array([[-1, 0, 0, 0],
                        [0, -1, 0, 0],
                        [0, 0, 1, 0],
                        [0, 0, 0, 1]])
    transNode = slicer.util.getNode('TibiaToTracker')
    Ttrans_dj = slicer.util.arrayFromTransformMatrix(transNode)
    Ttrans1=np.dot(Ttrans_dj,Ttrans1)
    Ttrans3 = np.dot(Ttrans1, self.Ttrans2)
    position3 = np.dot(Ttrans3, positiontmp)
    viewUpDirection = (float(Ttrans3[0][1]), float(Ttrans3[1][1]), float(Ttrans3[2][1]))
    focalPoint3 = [Ttrans3[0][3], Ttrans3[1][3], Ttrans3[2][3]]
    cameraNode.SetFocalPoint(focalPoint3)
    cameraNode.SetPosition(position3[0], position3[1], position3[2])
    cameraNode.SetViewUp(viewUpDirection)
    #self.Camera3(view)
    view.cameraNode().SetName('TC3')
  
  # 胫骨视图1按钮
  def onTV1Button(self):
    self.TV2Button.click()
    self.TV2Button.click()
    if (self.TV1Button.toolTip=='<p>锁定</p>'):
        self.interactorStyle1.SetInteractor(None)
        self.TV1Button.setToolTip('解锁')

    else:
        self.interactorStyle1.SetInteractor(self.interactor1)
        self.TV1Button.setToolTip('锁定')

  # 胫骨视图2按钮
  def onTV2Button(self):
    cameraNode=self.view1.cameraNode()
    cameraNode2 = self.view2.cameraNode()
    if slicer.modules.NoImageWelcomeWidget.judge == 'L':
      if (cameraNode.GetName() == 'TC1'):
        if (cameraNode2.GetName() == 'TC2'):
          self.TCamera1(self.view2)
          self.TCamera2(self.view1)
          self.DeleteTip(self.view1, self.view2, self.view3)
          self.TCamera1Tip(self.view2)
          self.TCamera2Tip(self.view1)
          self.TCamera3Tip(self.view3)
        else:
          self.TCamera3(self.view1)
          self.TCamera1(self.view2)
          self.DeleteTip(self.view1, self.view3, self.view2)
          self.TCamera1Tip(self.view2)
          self.TCamera2Tip(self.view3)
          self.TCamera3Tip(self.view1)
      elif (cameraNode.GetName() == 'TC2'):
        if (cameraNode2.GetName() == 'TC1'):
          self.TCamera1(self.view1)
          self.TCamera2(self.view2)
          self.DeleteTip(self.view2, self.view1, self.view3)
          self.TCamera1Tip(self.view1)
          self.TCamera2Tip(self.view2)
          self.TCamera3Tip(self.view3)
        else:
          self.TCamera3(self.view1)
          self.TCamera2(self.view2)
          self.DeleteTip(self.view3, self.view1, self.view2)
          self.TCamera1Tip(self.view3)
          self.TCamera2Tip(self.view2)
          self.TCamera3Tip(self.view1)
      else:
        if (cameraNode2.GetName() == 'TC2'):
          self.TCamera3(self.view2)
          self.TCamera2(self.view1)
          self.DeleteTip(self.view3, self.view2, self.view1)
          self.TCamera1Tip(self.view3)
          self.TCamera2Tip(self.view1)
          self.TCamera3Tip(self.view2)
        else:
          self.TCamera3(self.view2)
          self.TCamera1(self.view1)
          self.DeleteTip(self.view2, self.view3, self.view1)
          self.TCamera1Tip(self.view1)
          self.TCamera2Tip(self.view3)
          self.TCamera3Tip(self.view2)
    else:
      if (cameraNode.GetName() == 'TC1'):
        if (cameraNode2.GetName() == 'TC2'):
            self.TCamera1(self.view2)
            self.TCamera2(self.view1)
            self.DeleteTip(self.view1, self.view2, self.view3)
            self.TCamera1TipRight(self.view2)
            self.TCamera2Tip(self.view1)
            self.TCamera3TipRight(self.view3)
        else:
            self.TCamera3(self.view1)
            self.TCamera1(self.view2)
            self.DeleteTip(self.view1, self.view3, self.view2)
            self.TCamera1TipRight(self.view2)
            self.TCamera2Tip(self.view3)
            self.TCamera3TipRight(self.view1)
      elif (cameraNode.GetName() == 'TC2'):
        if (cameraNode2.GetName() == 'TC1'):
          self.TCamera1(self.view1)
          self.TCamera2(self.view2)
          self.DeleteTip(self.view2, self.view1, self.view3)
          self.TCamera1TipRight(self.view1)
          self.TCamera2Tip(self.view2)
          self.TCamera3TipRight(self.view3)
        else:
          self.TCamera3(self.view1)
          self.TCamera2(self.view2)
          self.DeleteTip(self.view3, self.view1, self.view2)
          self.TCamera1TipRight(self.view3)
          self.TCamera2Tip(self.view2)
          self.TCamera3TipRight(self.view1)
      else:
        if (cameraNode2.GetName() == 'TC2'):
          self.TCamera3(self.view2)
          self.TCamera2(self.view1)
          self.DeleteTip(self.view3, self.view2, self.view1)
          self.TCamera1TipRight(self.view3)
          self.TCamera2Tip(self.view1)
          self.TCamera3TipRight(self.view2)
        else:
          self.TCamera3(self.view2)
          self.TCamera1(self.view1)
          self.DeleteTip(self.view2, self.view3, self.view1)
          self.TCamera1TipRight(self.view1)
          self.TCamera2Tip(self.view3)
          self.TCamera3TipRight(self.view2)

  # 胫骨视图3按钮
  def onTV3Button(self):
    viewNode1 = slicer.mrmlScene.GetSingletonNode("1", "vtkMRMLViewNode")
    cameraNode = slicer.modules.cameras.logic().GetViewActiveCameraNode(viewNode1)
    viewNode3 = slicer.mrmlScene.GetSingletonNode("3", "vtkMRMLViewNode")
    cameraNode3 = slicer.modules.cameras.logic().GetViewActiveCameraNode(viewNode3)
    if slicer.modules.NoImageWelcomeWidget.judge == 'L':
        if (cameraNode.GetName() == 'TC1'):
            if (cameraNode3.GetName() == 'TC2'):
                self.TCamera1(self.view3)
                self.TCamera2(self.view1)
                self.DeleteTip(self.view1, self.view3, self.view2)
                self.TCamera1Tip(self.view3)
                self.TCamera2Tip(self.view1)
                self.TCamera3Tip(self.view2)
            else:
                self.TCamera3(self.view1)
                self.TCamera1(self.view3)
                self.DeleteTip(self.view1, self.view2, self.view3)
                self.TCamera1Tip(self.view3)
                self.TCamera2Tip(self.view2)
                self.TCamera3Tip(self.view1)

        elif (cameraNode.GetName() == 'TC2'):
            if (cameraNode3.GetName() == 'TC1'):
                self.TCamera1(self.view1)
                self.TCamera2(self.view3)
                self.DeleteTip(self.view2, self.view3, self.view1)
                self.TCamera1Tip(self.view1)
                self.TCamera2Tip(self.view3)
                self.TCamera3Tip(self.view2)
            else:
                self.TCamera3(self.view1)
                self.TCamera2(self.view3)
                self.DeleteTip(self.view2, self.view1, self.view3)
                self.TCamera1Tip(self.view2)
                self.TCamera2Tip(self.view3)
                self.TCamera3Tip(self.view1)
        else:
            if (cameraNode3.GetName() == 'TC2'):
                self.TCamera3(self.view3)
                self.TCamera2(self.view1)
                self.DeleteTip(self.view1, self.view2, self.view3)
                self.TCamera1Tip(self.view2)
                self.TCamera2Tip(self.view1)
                self.TCamera3Tip(self.view3)
            else:
                self.TCamera3(self.view3)
                self.TCamera1(self.view1)
                self.DeleteTip(self.view3, self.view2, self.view1)
                self.TCamera1Tip(self.view1)
                self.TCamera2Tip(self.view2)
                self.TCamera3Tip(self.view3)
    else:
        if (cameraNode.GetName() == 'TC1'):
            if (cameraNode3.GetName() == 'TC2'):
                self.TCamera1(self.view3)
                self.TCamera2(self.view1)
                self.DeleteTip(self.view1, self.view3, self.view2)
                self.TCamera1TipRight(self.view3)
                self.TCamera2Tip(self.view1)
                self.TCamera3TipRight(self.view2)
            else:
                self.TCamera3(self.view1)
                self.TCamera1(self.view3)
                self.DeleteTip(self.view1, self.view2, self.view3)
                self.TCamera1TipRight(self.view3)
                self.TCamera2Tip(self.view2)
                self.TCamera3TipRight(self.view1)

        elif (cameraNode.GetName() == 'TC2'):
            if (cameraNode3.GetName() == 'TC1'):
                self.TCamera1(self.view1)
                self.TCamera2(self.view3)
                self.DeleteTip(self.view2, self.view3, self.view1)
                self.TCamera1TipRight(self.view1)
                self.TCamera2Tip(self.view3)
                self.TCamera3TipRight(self.view2)
            else:
                self.TCamera3(self.view1)
                self.TCamera2(self.view3)
                self.DeleteTip(self.view2, self.view1, self.view3)
                self.TCamera1TipRight(self.view2)
                self.TCamera2Tip(self.view3)
                self.TCamera3TipRight(self.view1)
        else:
            if (cameraNode3.GetName() == 'TC2'):
                self.TCamera3(self.view3)
                self.TCamera2(self.view1)
                self.DeleteTip(self.view1, self.view2, self.view3)
                self.TCamera1TipRight(self.view2)
                self.TCamera2Tip(self.view1)
                self.TCamera3TipRight(self.view3)
            else:
                self.TCamera3(self.view3)
                self.TCamera1(self.view1)
                self.DeleteTip(self.view3, self.view2, self.view1)
                self.TCamera1TipRight(self.view1)
                self.TCamera2Tip(self.view2)
                self.TCamera3TipRight(self.view3)
  
  #胫骨相机1注释
  def TCamera1Tip(self, view):
    V11 = qt.QLabel(view)
    V12 = qt.QLabel(view)
    V13 = qt.QLabel(view)
    self.V14 = qt.QLabel(view)
    V15 = qt.QLabel(view)
    self.V16 = qt.QLabel(view)
    V17 = qt.QLabel(view)
    V18 = qt.QLabel(view)
    V19 = qt.QLabel(view)
    self.V1A = qt.QLabel(view)
    V1B = qt.QLabel(view)
    self.V1C = qt.QLabel(view)
    V1D = qt.QLabel(view)
    self.V1E = qt.QLabel(view)
    V1F = qt.QLabel(view)
    self.V1G = qt.QLabel(view)

    V11.setGeometry(view.contentsRect().width() - 100, 25, 100, 40)
    V11.setText(" 外翻/内翻 ")
    V11.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V11.show()
    try:
        V12.setGeometry(view.contentsRect().width() - 100, 50, 100, 25)
        V12.setText(' 0.0°')
        V12.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
        V12.show()
    except Exception as e:
        print(e)

    V1D.setGeometry(0, view.contentsRect().height() - 175, 100, 25)
    V1D.setText(" 内侧截骨 ")
    V1D.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V1D.show()

    self.V1E.setGeometry(0, view.contentsRect().height() - 150, 100, 25)
    try:
        self.V1E.setText(" "+str(round(slicer.modules.TibiaPopupWidget.TibiaNeiCeJieGu,1))+"mm")
    except Exception as e:
        print(e)
    self.V1E.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    self.V1E.show()


    V13.setGeometry(0, view.contentsRect().height() - 125, 100, 25)
    V13.setText(" 内侧伸直间隙 ")
    V13.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V13.show()
    self.V14.setGeometry(0, view.contentsRect().height() - 100, 100, 25)
    self.V14.setText("")
    self.V14.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    self.V14.show()
    V15.setGeometry(0, view.contentsRect().height() - 75, 100, 25)
    V15.setText(" 内侧屈膝间隙 ")
    V15.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V15.show()
    self.V16.setGeometry(0, view.contentsRect().height() - 50, 100, 25)
    self.V16.setText("")
    self.V16.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    self.V16.show()
    V17.setGeometry(5, 0.5 * view.contentsRect().height(), 100, 40)
    V17.setText("内 ")
    V17.setStyleSheet('QLabel{background-color:transparent;color:#A9A9A9;font:30px;}')
    V17.show()
    V18.setGeometry(view.contentsRect().width() - 50, 0.5 * view.contentsRect().height(), 100, 40)
    V18.setText(" 外 ")
    V18.setStyleSheet('QLabel{background-color:transparent;color:#A9A9A9;font:30px;}')
    V18.show()

    V1F.setGeometry(view.contentsRect().width() - 100, view.contentsRect().height() - 175, 100, 25)
    V1F.setText(" 外侧截骨 ")
    V1F.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V1F.show()
    self.V1G.setGeometry(view.contentsRect().width() - 100, view.contentsRect().height() - 150, 100, 25)
    try:
        self.V1G.setText(" "+str(round(slicer.modules.TibiaPopupWidget.TibiaWaiCeJieGu,1))+"mm")
    except Exception as e:
        print(e)
    self.V1G.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    self.V1G.show()

    V19.setGeometry(view.contentsRect().width() - 100, view.contentsRect().height() - 125, 100, 25)
    V19.setText(" 外侧伸直间隙 ")
    V19.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V19.show()
    self.V1A.setGeometry(view.contentsRect().width() - 100, view.contentsRect().height() - 100, 100, 25)
    self.V1A.setText("")
    self.V1A.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    self.V1A.show()
    V1B.setGeometry(view.contentsRect().width() - 100, view.contentsRect().height() - 75, 100, 25)
    V1B.setText(" 外侧屈膝间隙 ")
    V1B.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V1B.show()
    self.V1C.setGeometry(view.contentsRect().width() - 100, view.contentsRect().height() - 50, 100, 25)
    self.V1C.setText("")
    self.V1C.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    self.V1C.show()
  
  #右侧胫骨相机1注释
  def TCamera1TipRight(self, view):
    V11 = qt.QLabel(view)
    V12 = qt.QLabel(view)
    V13 = qt.QLabel(view)
    self.V14 = qt.QLabel(view)
    V15 = qt.QLabel(view)
    self.V16 = qt.QLabel(view)
    V17 = qt.QLabel(view)
    V18 = qt.QLabel(view)
    V19 = qt.QLabel(view)
    self.V1A = qt.QLabel(view)
    V1B = qt.QLabel(view)
    self.V1C = qt.QLabel(view)
    V1D = qt.QLabel(view)
    self.V1E = qt.QLabel(view)
    V1F = qt.QLabel(view)
    self.V1G = qt.QLabel(view)

    V11.setGeometry(view.contentsRect().width() - 100, 25, 100, 40)
    V11.setText(" 外翻/内翻 ")
    V11.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V11.show()
    try:
        V12.setGeometry(view.contentsRect().width() - 100, 50, 100, 25)
        V12.setText(' 0.0°')
        V12.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
        V12.show()
    except Exception as e:
        print(e)

    V1D.setGeometry(view.contentsRect().width() - 100, view.contentsRect().height() - 175, 100, 25)
    V1D.setText(" 内侧截骨 ")
    V1D.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V1D.show()

    self.V1E.setGeometry(view.contentsRect().width() - 100, view.contentsRect().height() - 150, 100, 25)
    try:
        self.V1E.setText(" "+str(round(slicer.modules.TibiaPopupWidget.TibiaNeiCeJieGu,1))+"mm")
    except Exception as e:
        print(e)
    self.V1E.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    self.V1E.show()


    V13.setGeometry(view.contentsRect().width() - 100, view.contentsRect().height() - 125, 100, 25)
    V13.setText(" 内侧伸直间隙 ")
    V13.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V13.show()
    self.V14.setGeometry( view.contentsRect().width() - 100, view.contentsRect().height() - 100, 100, 25)
    self.V14.setText("")
    self.V14.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    self.V14.show()
    V15.setGeometry( view.contentsRect().width() - 100, view.contentsRect().height() - 75, 100, 25)
    V15.setText(" 内侧屈膝间隙 ")
    V15.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V15.show()
    self.V16.setGeometry( view.contentsRect().width() - 100, view.contentsRect().height() - 50, 100, 25)
    self.V16.setText("")
    self.V16.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    self.V16.show()
    V17.setGeometry(view.contentsRect().width() - 50, 0.5 * view.contentsRect().height(), 100, 40)
    V17.setText(" 内 ")
    V17.setStyleSheet('QLabel{background-color:transparent;color:#A9A9A9;font:30px;}')
    V17.show()
    V18.setGeometry( 5, 0.5 * view.contentsRect().height(), 100, 40)
    V18.setText("外 ")
    V18.setStyleSheet('QLabel{background-color:transparent;color:#A9A9A9;font:30px;}')
    V18.show()

    V1F.setGeometry( 0, view.contentsRect().height() - 175, 100, 25)
    V1F.setText(" 外侧截骨 ")
    V1F.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V1F.show()
    self.V1G.setGeometry( 0, view.contentsRect().height() - 150, 100, 25)
    try:
        self.V1G.setText(" "+str(round(slicer.modules.TibiaPopupWidget.TibiaWaiCeJieGu,1))+"mm")
    except Exception as e:
        print(e)
    self.V1G.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    self.V1G.show()

    V19.setGeometry( 0, view.contentsRect().height() - 125, 100, 25)
    V19.setText(" 外侧伸直间隙 ")
    V19.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V19.show()
    self.V1A.setGeometry(0, view.contentsRect().height() - 100, 100, 25)
    self.V1A.setText("")
    self.V1A.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    self.V1A.show()
    V1B.setGeometry(0, view.contentsRect().height() - 75, 100, 25)
    V1B.setText(" 外侧屈膝间隙 ")
    V1B.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V1B.show()
    self.V1C.setGeometry(0, view.contentsRect().height() - 50, 100, 25)
    self.V1C.setText("")
    self.V1C.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    self.V1C.show()

  # 胫骨相机2注释
  def TCamera2Tip(self, view):
    V11 = qt.QLabel(view)
    self.T12 = qt.QLabel(view)
    V13 = qt.QLabel(view)
    V14 = qt.QLabel(view)
    V11.setGeometry(view.contentsRect().width() - 100, 25, 100, 40)
    V11.setText(" 前倾/后倾 ")
    V11.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V11.show()
    self.T12.setGeometry(view.contentsRect().width() - 100, 50, 100, 25)
    try:
        self.T12.setText(' '+str(round(slicer.modules.TibiaPopupWidget.TibiaHouQingJiao, 1))+'°')
    except Exception as e:
        print(e)
    self.T12.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    self.T12.show()
    V13.setGeometry(5, 0.5 * view.contentsRect().height(), 100, 40)
    V13.setText("前 ")
    V13.setStyleSheet('QLabel{background-color:transparent;color:#A9A9A9;font:30px;}')
    V13.show()
    V14.setGeometry(view.contentsRect().width() - 50, 0.5 * view.contentsRect().height(), 100, 40)
    V14.setText(" 后 ")
    V14.setStyleSheet('QLabel{background-color:transparent;color:#A9A9A9;font:30px;}')
    V14.show()

  # 胫骨相机3注释
  def TCamera3Tip(self, view):
    V21 = qt.QLabel(view)
    self.T22 = qt.QLabel(view)
    V23 = qt.QLabel(view)
    V24 = qt.QLabel(view)

    V21.setGeometry(view.contentsRect().width() - 100, 25, 100, 40)
    V21.setText(" 外旋/内旋 ")
    V21.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V21.show()
    self.T22.setGeometry(view.contentsRect().width() - 100, 50, 100, 25)
    try:
        self.T22.setText(' '+str(round(slicer.modules.TibiaPopupWidget.TibiaWaiXuanJiao, 1))+'°')
    except Exception as e:
        print(e)

    self.T22.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    self.T22.show()
    V23.setGeometry(5, 0.5 * view.contentsRect().height(), 100, 40)
    V23.setText("内 ")
    V23.setStyleSheet('QLabel{background-color:transparent;color:#A9A9A9;font:30px;}')
    V23.show()
    V24.setGeometry(view.contentsRect().width() - 50, 0.5 * view.contentsRect().height(), 100, 40)
    V24.setText(" 外 ")
    V24.setStyleSheet('QLabel{background-color:transparent;color:#A9A9A9;font:30px;}')
    V24.show()

  def TCamera3TipRight(self, view):
    V21 = qt.QLabel(view)
    self.T22 = qt.QLabel(view)
    V23 = qt.QLabel(view)
    V24 = qt.QLabel(view)

    V21.setGeometry(view.contentsRect().width() - 100, 25, 100, 40)
    V21.setText(" 外旋/内旋 ")
    V21.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V21.show()
    self.T22.setGeometry(view.contentsRect().width() - 100, 50, 100, 25)
    try:
        self.T22.setText(' '+str(round(slicer.modules.TibiaPopupWidget.TibiaWaiXuanJiao, 1))+'°')
    except Exception as e:
        print(e)

    self.T22.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    self.T22.show()
    V23.setGeometry(view.contentsRect().width() - 50, 0.5 * view.contentsRect().height(), 100, 40)
    V23.setText(" 内 ")
    V23.setStyleSheet('QLabel{background-color:transparent;color:#A9A9A9;font:30px;}')
    V23.show()
    V24.setGeometry( 5, 0.5 * view.contentsRect().height(), 100, 40)
    V24.setText("外 ")
    V24.setStyleSheet('QLabel{background-color:transparent;color:#A9A9A9;font:30px;}')
    V24.show()

    V21 = qt.QLabel(view)
    self.T22 = qt.QLabel(view)
    V23 = qt.QLabel(view)
    V24 = qt.QLabel(view)

    V21.setGeometry(view.contentsRect().width() - 100, 25, 100, 40)
    V21.setText(" 外旋/内旋 ")
    V21.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V21.show()
    self.T22.setGeometry(view.contentsRect().width() - 100, 50, 100, 25)
    try:
        self.T22.setText(' '+str(round(slicer.modules.TibiaPopupWidget.TibiaWaiXuanJiao, 1))+'°')
    except Exception as e:
        print(e)

    self.T22.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    self.T22.show()
    V23.setGeometry(view.contentsRect().width() - 100, 0.5 * view.contentsRect().height(), 100, 40)
    V23.setText(" 内侧 ")
    V23.setStyleSheet('QLabel{background-color:transparent;color:#A9A9A9;font:30px;}')
    V23.show()
    V24.setGeometry( 5, 0.5 * view.contentsRect().height(), 100, 40)
    V24.setText(" 外侧 ")
    V24.setStyleSheet('QLabel{background-color:transparent;color:#A9A9A9;font:30px;}')
    V24.show()

  # 隐藏三维视图显示的信息及按钮
  def hideInformation(self):
    try:
      self.V1Button.hide()
      self.V2Button.hide()
      self.V3Button.hide()
    except Exception as e:
      print(e)

    try:
      self.TV1Button.hide()
      self.TV2Button.hide()
      self.TV3Button.hide()
    except Exception as e:
      print(e)

    self.DeleteTip(self.view1, self.view2, self.view3)
  #删除Tip
  def DeleteTip(self, view1, view2, view3):
    for i in range(0, len(view1.findChildren(qt.QLabel))):
        view1.findChildren(qt.QLabel)[-1].delete()
    for i in range(0, len(view2.findChildren(qt.QLabel))):
        view2.findChildren(qt.QLabel)[-1].delete()
    for i in range(0, len(view3.findChildren(qt.QLabel))):
        view3.findChildren(qt.QLabel)[-1].delete()
  #三维视图观察者添加
  def AddObserver(self):
    self.view1Observer = slicer.util.getNode('vtkMRMLViewNode1').AddObserver(vtk.vtkWidgetEvent.Resize,self.UpdateTip)
    self.view2Observer = slicer.util.getNode('vtkMRMLViewNode2').AddObserver(vtk.vtkCommand.ModifiedEvent,self.UpdateTip)
    self.view3Observer = slicer.util.getNode('vtkMRMLViewNode3').AddObserver(vtk.vtkCommand.ModifiedEvent,self.UpdateTip)
  #三维视图观察者移除
  def RemoveObserver(self):
    slicer.util.getNode('vtkMRMLViewNode1').RemoveObserver(self.view1Observer)
    slicer.util.getNode('vtkMRMLViewNode2').RemoveObserver(self.view2Observer)
    slicer.util.getNode('vtkMRMLViewNode3').RemoveObserver(self.view3Observer)

  #更新Tip位置
  def UpdateTip(self,unusedArg1=None, unusedArg2=None, unusedArg3=None):
    print('更新三维视图的label')
    View1Tip = slicer.app.layoutManager().threeDWidget('View1').threeDView().findChildren('QLabel')
    View2Tip = slicer.app.layoutManager().threeDWidget('View2').threeDView().findChildren('QLabel')
    View3Tip = slicer.app.layoutManager().threeDWidget('View3').threeDView().findChildren('QLabel')
    view1 =  slicer.app.layoutManager().threeDWidget('View1').threeDView()
    view2 =  slicer.app.layoutManager().threeDWidget('View2').threeDView()
    view3 =  slicer.app.layoutManager().threeDWidget('View3').threeDView()
    ViewTip = [View1Tip,View2Tip,View3Tip]
    SumPosition = [self.Position(view1),self.Position(view2),self.Position(view3)]
    for j in range (0,len(ViewTip)):
        for i in range (0,len(ViewTip[j])):                        
            Tip = ViewTip[j][i]
            index = int(Tip.objectName)-1
            position = SumPosition[j]
            Tip.setGeometry(position[index][0],position[index][1],position[index][2],position[index][3])
    print('更新完成')

  def Position(self,view):
    position = np.array([[view.width - 100, 25, 100, 40],
        [view.width - 100, 50, 100, 25],
        [0, view.height - 125, 100, 25],
        [0, view.height - 100, 100, 25],
        [0, view.height - 75, 100, 25],
        [0, view.height - 50, 100, 25],
        [5, 0.5 * view.height, 100, 40],
        [view.width - 100, 0.5 * view.height, 100, 40],
        [view.width - 100, view.height - 125, 100, 25],
        [view.width - 100, view.height - 100, 100, 25],
        [view.width - 100, view.height - 75, 100, 25],
        [view.width - 100, view.height - 50, 100, 25],
        [view.width - 100, 25, 100, 40],
        [view.width - 100, 50, 100, 25],
        [0.5*view.width, 25, 100, 40],
        [0.5*view.width, view.height-40, 100, 40]])
    return position 

  #--------------------------股骨------------------------------------------
  #股骨骨骼参数
  def onParameter(self):
    try:
      slicer.mrmlScene.RemoveNode(slicer.util.getNode("TibiaLine"))
      slicer.mrmlScene.RemoveNode(slicer.util.getNode("FemurLine"))
      self.p1.clear()
      
    except:
      pass
    self.CanShuImage()
    self.ThreeDViewAndImageWidget(0)
    for i in range (0,len(self.ui.GraphImage.children())):
      a = self.ui.GraphImage.children()[-1]
      a.delete() 

    slicer.modules.noimageoperationimage.widgetRepresentation().setParent(self.noimageWidget)
    # UI设置为与noImagewidget同宽高
    slicer.modules.noimageoperationimage.widgetRepresentation().resize(self.noimageWidget.width, self.noimageWidget.height)
    slicer.modules.noimageoperationimage.widgetRepresentation().show()

    slicer.app.layoutManager().setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutTriple3DEndoscopyView)
    self.ThreeDState()#三维视图状态正常
    self.HideAll()#隐藏UI中的每一部分
    self.HidePart()#隐藏运行过程中产生的模型等
    self.ui.OperationPlanWidget.setVisible(True)#设置手术规划窗口可见
    self.ui.GuGe.setVisible(True)#设置骨骼参数窗口可见
    self.FemurButtonChecked(self.ui.Parameter)#设置当前选中的是骨骼参数按钮
    #获取所有模型节点
    models = slicer.util.getNodesByClass('vtkMRMLModelNode')
    Name = []
    for i in range(0, len(models)):
      a = models[i]
      Name.append(a.GetName())
    ROIs = slicer.util.getNodesByClass('vtkMRMLMarkupsROINode')
    #加载的数据为工程文件
    if '股骨远端' in Name:
      self.HidePart()
      self.ShowNode('股骨远端')
     
    #加载的数据为患者数据
    else:
      self.ShowNode('Femur')
      if len(ROIs) < 5:
        ras1, ras2, ras3, ras4 = [0, 0, 0], [0, 0, 0], [0, 0, 0], [0, 0, 0]
        slicer.util.getNode('股骨头球心').GetNthFiducialPosition(0, ras1)
        slicer.util.getNode('开髓点').GetNthFiducialPosition(0, ras2)
        slicer.util.getNode('外侧凸点').GetNthFiducialPosition(0, ras3)
        slicer.util.getNode('内侧凹点').GetNthFiducialPosition(0, ras4)
        zb1 = [-ras1[0], -ras1[1], ras1[2]]  # 坐标1，球心
        zb2 = [-ras2[0], -ras2[1], ras2[2]]  # 坐标2，原点
        zb3 = [-ras3[0], -ras3[1], ras3[2]]  # 坐标3，左侧点
        zb4 = [-ras4[0], -ras4[1], ras4[2]]  # 坐标4，右侧点
        jxlz = [0, 0, 0]  # Y轴基向量
        for i in range(0, 3):
            jxlz[i] = zb1[i] - zb2[i]
        moz = np.sqrt(np.square(jxlz[0]) + np.square(jxlz[1]) + np.square(jxlz[2]))  # 基向量z的模
        for i in range(0, 3):
            jxlz[i] = jxlz[i] / moz
        csD = jxlz[0] * zb2[0] + jxlz[1] * zb2[1] + jxlz[2] * zb2[2]  # 平面方程参数D
        csT3 = (jxlz[0] * zb3[0] + jxlz[1] * zb3[1] + jxlz[2] * zb3[2] - csD) / (
                jxlz[0] * jxlz[0] + jxlz[1] * jxlz[1] + jxlz[2] * jxlz[2])  # 坐标3平面方程参数T
        ty3 = [0, 0, 0]  # 坐标3在YZ平面的投影
        for i in range(0, 3):
            ty3[i] = zb3[i] - jxlz[i] * csT3
        csT4 = (jxlz[0] * zb4[0] + jxlz[1] * zb4[1] + jxlz[2] * zb4[2] - csD) / (
                jxlz[0] * jxlz[0] + jxlz[1] * jxlz[1] + jxlz[2] * jxlz[2])
        ty4 = [0, 0, 0]
        for i in range(0, 3):
            ty4[i] = zb4[i] - jxlz[i] * csT4
        jxlx = [0, 0, 0]  # X轴基向量
        for i in range(0, 3):
            if slicer.modules.NoImageWelcomeWidget.judge == 'L':
                jxlx[i] = ty3[i] - ty4[i]
            else:
                jxlx[i] = ty4[i] - ty3[i]
        mox = np.sqrt(np.square(jxlx[0]) + np.square(jxlx[1]) + np.square(jxlx[2]))  # 基向量X的模
        for i in range(0, 3):
            jxlx[i] = jxlx[i] / mox
        jxly = [0, 0, 0]  # y轴基向量
        jxly[0] = -(jxlx[1] * jxlz[2] - jxlx[2] * jxlz[1])
        jxly[1] = -(jxlx[2] * jxlz[0] - jxlx[0] * jxlz[2])
        jxly[2] = -(jxlx[0] * jxlz[1] - jxlx[1] * jxlz[0])
        moy = np.sqrt(np.square(jxly[0]) + np.square(jxly[1]) + np.square(jxly[2]))  # 基向量y的模
        for i in range(0, 3):
            jxly[i] = jxly[i] / moy
        ccb = ([jxlx, jxly, jxlz])
        ccc = np.asarray(ccb)
        ccd = ccc.T
        np.savetxt(self.FilePath + "/Femur-jxl.txt", ccd, fmt='%6f')
        Ftrans1 = np.array([[-1, 0, 0, 0],
                            [0, -1, 0, 0],
                            [0, 0, 1, 0],
                            [0, 0, 0, 1]])
        self.Ftrans2 = np.array([[float(jxlx[0]), float(jxly[0]), float(jxlz[0]), zb2[0]],
                                [float(jxlx[1]), float(jxly[1]), float(jxlz[1]), zb2[1]],
                                [float(jxlx[2]), float(jxly[2]), float(jxlz[2]), zb2[2]],
                                [0, 0, 0, 1]])
        Ftrans3 = np.array([[1, 0, 0, 0],
                            [0, 1, 0, 0],
                            [0, 0, 1, 0],
                            [0, 0, 0, 1]])

        Ftransform1 = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", '变换')
        Ftransform2 = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", '变换_1')
        Ftransform3 = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", '变换_2')
        FtransformTmp = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", '变换_临时')
        FtransformR = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", '变换_R')
        Ftransform1.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(Ftrans1))
        Ftransform2.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(self.Ftrans2))
        Ftransform3.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(Ftrans3))
        FtransformR.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(Ftrans3))
        Ftransform3.SetAndObserveTransformNodeID(FtransformTmp.GetID())
        FtransformTmp.SetAndObserveTransformNodeID(Ftransform2.GetID())
        Ftransform2.SetAndObserveTransformNodeID(Ftransform1.GetID())
        FtransformR.SetAndObserveTransformNodeID(Ftransform3.GetID())
        self.EtctMove('变换','DianjiToTracker1')

        roiNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsROINode', '股骨远端切割')
        roiNode.SetCenter([0, 0, 0])
        roiNode.SetSize([100, 100, 140])
        roiNode.SetDisplayVisibility(False)
        roiNode.SetAndObserveTransformNodeID(FtransformR.GetID())
        OutputModel = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLModelNode", "股骨远端")
        inputModel = slicer.util.getNode('Femur')
        dynamicModelerNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLDynamicModelerNode")
        dynamicModelerNode.SetToolName("ROI cut")
        dynamicModelerNode.SetNodeReferenceID("ROICut.InputModel", inputModel.GetID())
        dynamicModelerNode.SetNodeReferenceID("ROICut.InputROI", roiNode.GetID())
        dynamicModelerNode.SetNodeReferenceID("ROICut.OutputPositiveModel", OutputModel.GetID())
        # dynamicModelerNode.SetContinuousUpdate(1)
        dynamicModelerNode.SetAttribute("ROICut.CapSurface", '1')
        slicer.modules.dynamicmodeler.logic().RunDynamicModelerTool(dynamicModelerNode)
        self.NodeMove('股骨远端','DianjiToTracker1')
        inputModel.SetDisplayVisibility(False)
        self.addAxisFemur()


    #根据参数智能推荐假体
    try:
      if slicer.modules.NoImageWelcomeWidget.judge == 'L':
        self.SelectJiaTi('l')
      else:
        self.SelectJiaTi('R')
    except Exception as e:
      print(e)
    self.onParameter2()
    #设置相机并居中
    self.Camera1(self.view1)
    self.Camera2(self.view2)
    self.Camera3(self.view3)
    self.view1.resetFocalPoint()
    self.view2.resetFocalPoint()
    self.view3.resetFocalPoint()
    self.HidePart()
    self.ShowNode('股骨远端')
    self.ShowNode('胫骨近端')
    self.ui.ModuleName.setText('手术规划')

  def CanShuImage(self):
    APMLPath = os.path.join(self.iconsPath, 'APML.png')
    self.ui.APImage.setMaximumSize(200, 172)
    self.ui.APImage.setPixmap(qt.QPixmap(APMLPath))
    self.ui.APImage.setScaledContents(True)

    self.ui.MLImage.setPixmap(qt.QPixmap(APMLPath))
    self.ui.MLImage.setMaximumSize(200, 172)
    self.ui.MLImage.setScaledContents(True)

    self.ui.TibiaImage.setPixmap(qt.QPixmap(APMLPath))
    self.ui.TibiaImage.setMaximumSize(200, 172)
    self.ui.TibiaImage.setScaledContents(True)
  
  #获取第一刀截骨面(根据内侧远端到第一截骨面的距离确定)
  def FirstJieGu(self):
    point = [0, 0, 0]
    point1 = [0,0,0]
    slicer.util.getNode('内侧远端').GetNthControlPointPositionWorld(0, point)
    slicer.util.getNode('外侧皮质高点').GetNthControlPointPositionWorld(0, point1)
    transformR = slicer.util.getNode('变换_R')
    Femur1JieGu = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', '股骨第一截骨面')
    Femur1JieGu.AddControlPoint(15.063, 25, 0)
    Femur1JieGu.AddControlPoint(0, 0, 0)
    Femur1JieGu.AddControlPoint(-21.372, -23.271, 0)
    Femur1JieGu.SetAndObserveTransformNodeID(transformR.GetID())
    Femur2JieGu = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', '股骨第二截骨面')
    Femur2JieGu.AddControlPoint(15.063, -23.063, 22.222)
    Femur2JieGu.AddControlPoint(-1.143, -23.207, 29.894)
    Femur2JieGu.AddControlPoint(-21.372, -23.271, 24.171)
    Femur2JieGu.SetAndObserveTransformNodeID(transformR.GetID())
    # 隐藏股骨第一截骨面的点
    self.HideNode('股骨第一截骨面')
    self.HideNode('股骨第二截骨面')
    Femur1JGM = self.GetTransPoint('股骨第一截骨面')
    Femur2JGM = self.GetTransPoint('股骨第二截骨面')
    d = self.point2area_distance(Femur1JGM, point)
    d1 = self.point2area_distance(Femur2JGM,point1)
    self.destance = 8-d
    FtransTmp = np.array([[1, 0, 0, 2],
                        [0, 1, 0, d1+3],
                        [0, 0, 1, self.destance],
                        [0, 0, 0, 1]])
    FtransformTmp = slicer.util.getNode('变换_临时')
    FtransformTmp.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(FtransTmp))

  #获取上层变换的合集
  def FemurTrans(self):
    transform = slicer.util.arrayFromTransformMatrix(slicer.util.getNode('变换'))
    transform1 = slicer.util.arrayFromTransformMatrix(slicer.util.getNode('变换_1'))
    transform2 = slicer.util.arrayFromTransformMatrix(slicer.util.getNode('变换_2'))
    transform_tmp = slicer.util.arrayFromTransformMatrix(slicer.util.getNode('变换_临时'))
    Trans = np.dot(np.dot(np.dot(transform, transform1), transform_tmp),transform2)
    return Trans
  #推荐股骨假体    
  def SelectJiaTi(self, judge):
    import math
    self.FirstJieGu()
    PointPath = os.path.join(os.path.dirname(__file__), '假体库/a')
    point1 = self.QiuDianInTop('外侧皮质高点')
    point2 = self.QiuDianInTop('外侧后髁')
    trans = self.FemurTrans()

    #骨骼纵向高度 （算上软骨2mm）
    height = abs(point1[1] - point2[1])-6
    self.ui.AP.setText(height)
    self.select = 0
    #假体纵向高度
    list = [35.69, 38.69, 41.09, 43.69, 47.59, 50.07]
    
    #假体型号
    list3 = ['1-5', '2', '2-5', '3', '4', '5']
    Femur1JGM = self.GetTransPoint('股骨第一截骨面')
    #皮质高点到第一刀截骨面的距离
    ras1 = [0, 0, 0]
    slicer.util.getNode('外侧皮质高点').GetNthControlPointPositionWorld(0, ras1)
    d = self.point2area_distance(Femur1JGM, ras1)
    #第二刀倾斜角为6°
    k=math.tan(math.radians(6))
    judge1=[]
    index1=[]
    b1 = 0
    for i in range(0, len(list)):
      inputPoints=[]
      rang = k*d + list[i]
      # self.ui.tableWidget.setItem(i, 0, qt.QTableWidgetItem(str(rang)))
      if rang < height:
        name = 'femur-' + judge + list3[i]
        lujing = os.path.join(PointPath,name+'.txt')
        print('lujing',lujing)
        point  =  np.loadtxt(lujing) 

        for j in range(0,8):
          b=np.array([float(point[j][0]),float(point[j][1]),point[j][2],1])
          a=np.dot(trans,b)
          inputPoints.append(a)
        
        inputModel=slicer.util.getNode('Femur')
        surface_World = inputModel.GetPolyData()
        distanceFilter = vtk.vtkImplicitPolyDataDistance()
        distanceFilter.SetInput(surface_World)
        nOfFiducialPoints = 8
        distances = np.zeros(nOfFiducialPoints)
        for j in range(nOfFiducialPoints):
          point_World = np.asarray(inputPoints)[j,0:3] 
          closestPointOnSurface_World = np.zeros(3)
          closestPointDistance = distanceFilter.EvaluateFunctionAndGetClosestPoint(point_World, closestPointOnSurface_World)
          distances[j] = closestPointDistance
        
        print('distances:',distances)
        a=0
        if i == 0:
          b1=distances[0]

        if distances[0]+distances[4]<0 and  distances[1]+distances[5]<0 and distances[2]+distances[6]<0 and distances[3]+distances[7]<0:
          a=8

        if a==8:
          sum = 0
          for j in range(nOfFiducialPoints):
            sum =sum + distances[j]
          judge1.append(sum)        
          index1.append(i)
      try:
        max_judge1=judge1.index(max(judge1))
        self.select = index1[max_judge1]
      except:
        self.select = 2

    if len(judge1)<1 :
      self.select = 0

    Name = 'femur-'+judge+list3[self.select]
    self.ui.JiaTiName.setText(Name)

        
    if judge == 'l':
      knee = '左侧'
      self.ui.FemurL.setCurrentText(Name)

    else:
      knee = '右侧'
      self.ui.FemurR.setCurrentText(Name)
        
    self.ui.knee.setText(knee)
    MLList=[57.000999450683594, 60.0, 63.0, 66.0, 71.0, 73.0000991821289]
    # for i in range(0,len(MLList)):
      # self.ui.tableWidget.setItem(i, 1, qt.QTableWidgetItem(str(MLList[i])))
    if len(index1)==0:
      self.ui.ML.setText(str(MLList[len(index1)]-np.abs(b1)/10))
      print(len(index1))
    else: 
      self.ui.ML.setText(str(MLList[len(index1)-1]+np.abs(b1)/10))
      print(len(index1))
    #for i in range(0,len(index1)):
    # self.ui.tableWidget.item(len(index1)-1, 0).setBackground(qt.QColor(124, 189, 39))
    # self.ui.tableWidget.item(len(index1)-1,1).setBackground(qt.QColor(124, 189, 39))

  #加载股骨假体           
  def loadJiaTi(self, name):
    try:
        slicer.mrmlScene.RemoveNode(self.jiatiload)
    except Exception as e:
        print(e) 
    lujing = os.path.join(self.jiatiPath, name + '.stl')
    self.jiatiload = slicer.util.loadModel(lujing)
    self.jiatiload.SetName(name)

    #将假体放在FTransformR变换下：
    FtransformR = slicer.util.getNode('变换_R')
    self.jiatiload.SetAndObserveTransformNodeID(FtransformR.GetID())
    self.loaddier()
  #在世界坐标系下求点的坐标
  def QiuDianInTop(self, name):
    ras1 = [0, 0, 0]
    slicer.util.getNode(name).GetNthControlPointPositionWorld(0, ras1)
    transNode = slicer.util.getNode('DianjiToTracker1')
    trans0 = slicer.util.arrayFromTransformMatrix(transNode)
    transformNode = slicer.util.getNode('变换')
    trans = slicer.util.arrayFromTransformMatrix(transformNode)
    transformNode1 = slicer.util.getNode('变换_1')
    trans1 = slicer.util.arrayFromTransformMatrix(transformNode1)
    trans2=np.dot(np.dot(trans0,trans),trans1)
    Trans_ni=np.linalg.inv(trans2)
    point = np.array([ras1[0],ras1[1],ras1[2],1])
    point2 = np.dot(point,Trans_ni)
    return point2
    
  def get_point_femur_to_ras(self,p):
      # 股骨变换
      transformNode = slicer.util.getNode('变换')
      trans = slicer.util.arrayFromTransformMatrix(transformNode)
      transformNode1 = slicer.util.getNode('变换_1')
      trans1 = slicer.util.arrayFromTransformMatrix(transformNode1)
      transformNodeTmp = slicer.util.getNode('变换_临时')
      transTmp = slicer.util.arrayFromTransformMatrix(transformNodeTmp)
      transformNode2 = slicer.util.getNode('变换_2')
      trans2 = slicer.util.arrayFromTransformMatrix(transformNode2)
      transformNodeR = slicer.util.getNode('变换_R')
      transR = slicer.util.arrayFromTransformMatrix(transformNodeR)
      FemurTrans = np.dot(np.dot(np.dot(np.dot(trans, trans1), transTmp), trans2), transR)
      FemurTrans_ni = np.linalg.inv(FemurTrans)
      point=np.array([p[0],p[1],p[2],1])
      point=np.dot(FemurTrans_ni,point)
      return point[0:3]

  def YueShu(self):
    import math
    ras = [0, 0, 0]
    ras1 = [0, 0, 0]
    slicer.util.getNode('内侧远端').GetNthFiducialPosition(0, ras)
    position=self.get_point_femur_to_ras(ras)[2]
    distance = 8+position
    transformR = slicer.util.getNode('变换_R')
    a =slicer.util.arrayFromTransformMatrix(transformR)
    FtransTmp = np.array([[1, 0, 0, 0],
                        [0, 1, 0, 0],
                        [0, 0, 1, distance],
                        [0, 0, 0, 1]])
    trans = np.dot(a,FtransTmp)
    transformR.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(trans))
    print('distance 赋值完成')
    Femur2JGM = self.GetTransPoint('股骨第二截骨面')
    slicer.util.getNode('外侧皮质高点').GetNthControlPointPositionWorld(0, ras1)
    #根据外侧皮质高点在第二刀切面的位置来判断调整正负
    slicer.util.getNode('内侧远端').GetNthControlPointPositionWorld(0, ras)
    #t_tibia = self.Ftrans2
    #n1=[t_tibia[0][1], t_tibia[1][1], t_tibia[2][1]]
    n1=self.GetNorlm('Femur_YJtAxis')
    n2 = np.array(ras1) - self.TouYing(Femur2JGM, ras1)
    if np.dot(n1,n2)>0:
        direction = 1
    else:
        direction = -1
    d = self.point2area_distance(Femur2JGM, ras1)
    x = d/math.cos(math.radians(6))
    #direction = slicer.modules.PopupWidget.direction
    x = direction *x
    a =slicer.util.arrayFromTransformMatrix(transformR)

    FtransTmp = np.array([[1, 0, 0, 0],
                        [0, 1, 0, x],
                        [0, 0, 1, 0],
                        [0, 0, 0, 1]])
    trans = np.dot(a,FtransTmp)
    transformR.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(trans))
    print(trans)
    print('所有矩阵赋值完成')


  #加载股骨第二截骨面
  def loaddier(self):
    # 第二刀截骨面
    FtransformR = slicer.util.getNode('变换_R')
    segs = slicer.util.getNodesByClass('vtkMRMLMarkupsFiducialNode')
    Name = []

    for i in range(0, len(segs)):
        a = segs[i]
        Name.append(a.GetName())
    if '股骨第二截骨面' in Name:
        slicer.mrmlScene.RemoveNode(slicer.util.getNode('股骨第二截骨面'))
    
    FtransTmp = np.array([[1, 0, 0, 0],
                        [0, 1, 0, 0],
                        [0, 0, 1, self.destance],
                        [0, 0, 0, 1]])
    FtransformTmp = slicer.util.getNode('变换_临时')
    FtransformTmp.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(FtransTmp))

    if self.jiatiload.GetName() == 'femur-R1-5' or self.jiatiload.GetName() == 'femur-l1-5':

        Femur2JieGu = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', '股骨第二截骨面')
        Femur2JieGu.AddControlPoint(15.063, -23.063, 22.222)
        Femur2JieGu.AddControlPoint(-1.143, -23.207, 29.894)
        Femur2JieGu.AddControlPoint(-21.372, -23.271, 24.171)

    elif self.jiatiload.GetName() == 'femur-R2' or self.jiatiload.GetName() == 'femur-l2':

        Femur2JieGu = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', '股骨第二截骨面')
        Femur2JieGu.AddControlPoint(16.637, -24.306, 21.857)
        Femur2JieGu.AddControlPoint(-1.185, -25.471, 32.778)
        Femur2JieGu.AddControlPoint(-22.742, -24.575, 24.382)

    elif self.jiatiload.GetName() == 'femur-R2-5' or self.jiatiload.GetName() == 'femur-l2-5':

        Femur2JieGu = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', '股骨第二截骨面')
        Femur2JieGu.AddControlPoint(17.709, -25.954, 22.334)
        Femur2JieGu.AddControlPoint(-1.430, -27.379, 35.696)
        Femur2JieGu.AddControlPoint(-24.282, -26.237, 24.987)

    elif self.jiatiload.GetName() == 'femur-R3' or self.jiatiload.GetName() == 'femur-l3':

        Femur2JieGu = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', '股骨第二截骨面')
        Femur2JieGu.AddControlPoint(18.255, -27.746, 22.792)
        Femur2JieGu.AddControlPoint(-1.727, -29.360, 37.930)
        Femur2JieGu.AddControlPoint(-25.269, -28.035, 25.497)

    elif self.jiatiload.GetName() == 'femur-R4' or self.jiatiload.GetName() == 'femur-l4':

        Femur2JieGu = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', '股骨第二截骨面')
        Femur2JieGu.AddControlPoint(19.597, -30.070, 23.700)
        Femur2JieGu.AddControlPoint(-2.089, -32.049, 42.257)
        Femur2JieGu.AddControlPoint(-27.321, -30.453, 27.293)

    elif self.jiatiload.GetName() == 'femur-R5' or self.jiatiload.GetName() == 'femur-l5':

        Femur2JieGu = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', '股骨第二截骨面')
        Femur2JieGu.AddControlPoint(19.941, -30.642, 24.760)
        Femur2JieGu.AddControlPoint(-2.089, -32.049, 42.257)
        Femur2JieGu.AddControlPoint(-27.877, -31.025, 28.357)

    Femur2JieGu.SetAndObserveTransformNodeID(FtransformR.GetID())
    # 隐藏股骨第二截骨面的点
    self.HideNode('股骨第二截骨面')
    Femur2JGM = self.GetTransPoint('股骨第二截骨面')
    try:
      ras1 = [0, 0, 0]
      slicer.util.getNode('外侧皮质高点').GetNthControlPointPositionWorld(0, ras1)
      d = self.point2area_distance(Femur2JGM, ras1)
      x = d/math.cos(math.radians(6))
      self.record = x
      FtransTmp = np.array([[1, 0, 0, 0],
              [0, 1, 0, x],
              [0, 0, 1, self.destance],
              [0, 0, 0, 1]])
      FtransformTmp.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(FtransTmp))
    except Exception as e:
      print(e)

  # 股骨截骨调整
  def onAdjustment(self):
    rotationTransformNode = slicer.util.getNode('DianjiToTracker1')
    try:
      rotationTransformNode.RemoveObserver(self.updataForceAngle)
    except:
      pass
    try:
      slicer.mrmlScene.RemoveNode(slicer.util.getNode("TibiaLine"))
      slicer.mrmlScene.RemoveNode(slicer.util.getNode("FemurLine"))
    except:
      pass

    self.FourWidget.installEventFilter(self.resizeEvent)
    for i in range(0,len(self.ui.GraphImage.children())):
      self.ui.GraphImage.children()[-1].delete()
    slicer.app.layoutManager().setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutTriple3DEndoscopyView)
    self.ThreeDViewAndImageWidget(2)
    if self.JingGu == 0:
      self.ui.Adjustment2.click()
      self.JingGu = 1
    # try:
    #   self.ui.BoneButton.setChecked(False)
    # except:
    #   pass 

    try:
      self.jiatiload.SetDisplayVisibility(True)
    except:
      if slicer.modules.NoImageWelcomeWidget.judge == 'L':
        self.loadJiaTi(self.ui.FemurL.currentText)
      else:
        self.loadJiaTi(self.ui.FemurR.currentText)
    #widget内按钮互斥
    self.FemurButtonChecked(self.ui.Adjustment)
    self.InitHeadWidget()
    #设置视图2，视图3不可转动
    self.interactorStyle2.SetInteractor(None)
    self.interactorStyle3.SetInteractor(None)
    #self.AddObserver()
    models = slicer.util.getNodesByClass('vtkMRMLModelNode')
    Name = []
    for i in range(0, len(models)):
      a = models[i]
      Name.append(a.GetName())
    if '股骨切割' in Name:
      pass
    #显示假体
    try:
        self.jiatiload.SetDisplayVisibility(True)
    except Exception as e:
        print(e)
    TransformR = slicer.util.getNode('变换_R')
    Name = []
    # 隐藏股骨第一截骨面的点
    self.HideNode('股骨第一截骨面')

    # 第三刀截骨面
    if '股骨第三截骨面' in Name:
      slicer.mrmlScene.RemoveNode(slicer.util.getNode('股骨第三截骨面'))

    Femur3JieGu = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', '股骨第三截骨面')
    if self.jiatiload.GetName() == 'femur-R1-5' or self.jiatiload.GetName() == 'femur-l1-5':
      Femur3JieGu.AddControlPoint(-27.557, 16.002, 11.971)
      Femur3JieGu.AddControlPoint(-11.793, 16.055, 15.009)
      Femur3JieGu.AddControlPoint(27.508, 15.977, 10.607)

    elif self.jiatiload.GetName() == 'femur-R2' or self.jiatiload.GetName() == 'femur-l2':
      Femur3JieGu.AddControlPoint(-28.561, 17.734, 13.564)
      Femur3JieGu.AddControlPoint(-11.861, 17.780, 16.172)
      Femur3JieGu.AddControlPoint(28.927, 17.695, 11.328)

    elif self.jiatiload.GetName() == 'femur-R2-5' or self.jiatiload.GetName() == 'femur-l2-5':
      Femur3JieGu.AddControlPoint(-30.390, 18.590, 16.982)
      Femur3JieGu.AddControlPoint(-11.778, 18.497, 11.740)
      Femur3JieGu.AddControlPoint(30.265, 18.453, 9.234)

    elif self.jiatiload.GetName() == 'femur-R3' or self.jiatiload.GetName() == 'femur-l3':
      Femur3JieGu.AddControlPoint(-32.234, 19.490, 15.771)
      Femur3JieGu.AddControlPoint(-11.778, 18.497, 11.740)
      Femur3JieGu.AddControlPoint(31.921, 19.462, 14.190)

    elif self.jiatiload.GetName() == 'femur-R4' or self.jiatiload.GetName() == 'femur-l4':
      Femur3JieGu.AddControlPoint(-34.653, 20.921, 11.327)
      Femur3JieGu.AddControlPoint(-12.011, 20.998, 15.694)
      Femur3JieGu.AddControlPoint(34.498, 21.007, 16.178)

    elif self.jiatiload.GetName() == 'femur-R5' or self.jiatiload.GetName() == 'femur-l5':
      Femur3JieGu.AddControlPoint(-35.419, 23.102, 13.337)
      Femur3JieGu.AddControlPoint(-11.893, 23.144, 15.654)
      Femur3JieGu.AddControlPoint(35.384, 23.198, 18.747)

    Femur3JieGu.SetAndObserveTransformNodeID(TransformR.GetID())
    # 隐藏股骨第三截骨面的点
    self.HideNode('股骨第三截骨面')

    # -----------------S----------------------------------
    # 计算角度
    # ---------------------------------------------------
    try:
      ras1, ras2, ras3, ras4 = [0, 0, 0], [0, 0, 0], [0, 0, 0], [0, 0, 0],
      ras5, ras6, ras7 = [0, 0, 0], [0, 0, 0], [0, 0, 0]
      slicer.util.getNode('股骨头球心').GetNthControlPointPositionWorld(0, ras1)
      slicer.util.getNode('开髓点').GetNthControlPointPositionWorld(0, ras2)
      slicer.util.getNode('外侧凸点').GetNthControlPointPositionWorld(0, ras3)
      slicer.util.getNode('内侧凹点').GetNthControlPointPositionWorld(0, ras4)
      slicer.util.getNode('H点').GetNthControlPointPositionWorld(0, ras5)
      slicer.util.getNode('内侧后髁').GetNthControlPointPositionWorld(0, ras6)
      slicer.util.getNode('外侧后髁').GetNthControlPointPositionWorld(0, ras7)

      # 外翻角
      xl1 = [ras1[0] - ras2[0], ras1[1] - ras2[1], ras1[2] - ras2[2]]
      xl2 = [ras5[0] - ras2[0], ras5[1] - ras2[1], ras5[2] - ras2[2]]
      self.WaiFanJiao = self.Angle(xl1, xl2)

      # 外旋角
      FemurJGM= self.GetTransPoint('股骨第一截骨面')
      NeiAoTY = self.TouYing(FemurJGM, ras4)
      WaiTuTY = self.TouYing(FemurJGM, ras3)
      NeiKeTY = self.TouYing(FemurJGM, ras6)
      WaiKeTY = self.TouYing(FemurJGM, ras7)

      xl3 = np.array([WaiTuTY[0] - NeiAoTY[0], WaiTuTY[1] - NeiAoTY[1], WaiTuTY[2] - NeiAoTY[2]])
      xl4 = np.array([WaiKeTY[0] - NeiKeTY[0], WaiKeTY[1] - NeiKeTY[1], WaiKeTY[2] - NeiKeTY[2]])
      waixuanjiao = self.Angle(xl3, xl4)
      self.WaiXuanJiao = waixuanjiao

    except Exception as e:
      print(e)
    
    self.HideAll()
    self.ui.OperationPlanWidget.setVisible(True)
    self.ui.PopupWidget.setVisible(True)
    self.ui.head1.setVisible(True)
    self.ui.Graph.setVisible(True)  
    #初始化显示/隐藏，显示假体
    self.ui.JiaTiButton.setChecked(True)
    self.ui.PopupWidget.setMinimumHeight(470)
    slicer.modules.popup.widgetRepresentation().setParent(self.ui.PopupWidget)
    slicer.modules.popup.widgetRepresentation().setGeometry(-15,0,450,500)
    slicer.modules.popup.widgetRepresentation().setStyleSheet('QPushButton{background-color: rgb(69,70,71);}QPushButton:hover{background-color: #7cbd27;}QPushButton:pressed{background-color: #7cbd27;}')
    slicer.modules.popup.widgetRepresentation().show()
    slicer.modules.PopupWidget.ui.xMoveButton1.click()
    #self.TibiaButtonChecked(None)
    self.ui.Adjustment2.setChecked(0)
    self.ShowHide()
    if len(self.ui.GraphImage.children()) <2:
      self.PyQtGraph()
    else:
      self.ui.GraphImage.show()
    self.HidePart()
    self.ShowNode('股骨切割')
    self.jiatiload.SetDisplayVisibility(True)
    self.ui.ModuleName.setText('股骨截骨')
    import time
    time.sleep(1)
    self.FemurCameraTip()
    self.V2Button.click()
    self.V2Button.click()


  #股骨三维视图注释
  def FemurCameraTip(self):
    self.hideInformation()#删除掉之前视图上的内容
    #视图1按钮
    icon1A = qt.QIcon()
    icons1APath = os.path.join(self.iconsPath, '重置.png')
    icon1A.addPixmap(qt.QPixmap(icons1APath))
    self.V1Button = qt.QPushButton(self.view1)
    self.V1Button.setGeometry(5, 5, 50, 50)
    self.V1Button.setIconSize(qt.QSize(60, 60))
    self.V1Button.setIcon(icon1A)
    self.V1Button.setFlat(True)
    self.V1Button.setStyleSheet("QPushButton{border:none;background:transparent;color:transparent;}")
    self.V1Button.connect('clicked(bool)',self.onV1Button)
    self.V1Button.setToolTip('锁定')
    self.V1Button.show()
    # -------------------------------------------------------------------------------------------------------
    #视图2按钮
    icon2A = qt.QIcon()
    icons2APath = os.path.join(self.iconsPath, '箭头.png')
    icon2A.addPixmap(qt.QPixmap(icons2APath))
    self.V2Button = qt.QPushButton(self.view2)
    self.V2Button.setGeometry(5, 5, 41, 41)
    self.V2Button.setIconSize(qt.QSize(41, 41))
    self.V2Button.setIcon(icon2A)
    self.V2Button.setFlat(True)
    self.V2Button.setStyleSheet("QPushButton{border:none;background:transparent;}")
    self.V2Button.connect('clicked(bool)', self.onV2Button)
    self.V2Button.show()

    # ---------------------------------------------------------------------------------------------------------
    #视图3按钮
    icon3A = qt.QIcon()
    icons3APath = os.path.join(self.iconsPath, '箭头.png')
    icon3A.addPixmap(qt.QPixmap(icons3APath))
    self.V3Button = qt.QPushButton(self.view3)
    self.V3Button.setGeometry(5, 5, 46, 46)
    self.V3Button.setIconSize(qt.QSize(46, 46))
    self.V3Button.setIcon(icon3A)
    self.V3Button.setFlat(True)
    self.V3Button.setStyleSheet("QPushButton{border:none;background:transparent;}")
    self.V3Button.connect('clicked(bool)', self.onV3Button)
    self.V3Button.show()
    self.Camera1(self.view1)
    self.Camera2(self.view2)
    self.Camera3(self.view3)
    cameraNode=self.view1.cameraNode()
    cameraNode2 = self.view2.cameraNode()

    #显示每个视图的注释
    # 左腿
    if slicer.modules.NoImageWelcomeWidget.judge == 'L':
      if (cameraNode.GetName() == 'FC1'):
        if (cameraNode2.GetName() == 'FC2'):
          self.Camera1Tip(self.view1)
          self.Camera2Tip(self.view2)
          self.Camera3Tip(self.view3)
        else:
          self.Camera1Tip(self.view1)
          self.Camera2Tip(self.view3)
          self.Camera3Tip(self.view2)

      elif (cameraNode.GetName() == 'FC2'):
        if (cameraNode2.GetName() == 'FC1'):
          self.Camera1Tip(self.view2)
          self.Camera2Tip(self.view1)
          self.Camera3Tip(self.view3)
        else:
          self.Camera1Tip(self.view2)
          self.Camera2Tip(self.view3)
          self.Camera3Tip(self.view1)
      else:
          if (cameraNode2.GetName() == 'FC2'):
            self.Camera1Tip(self.view3)
            self.Camera2Tip(self.view2)
            self.Camera3Tip(self.view1)
          else:
            self.Camera1Tip(self.view3)
            self.Camera2Tip(self.view1)
            self.Camera3Tip(self.view2)
    else:
      if (cameraNode.GetName() == 'FC1'):
        if (cameraNode2.GetName() == 'FC2'):
          self.Camera1TipRight(self.view1)
          self.Camera2Tip(self.view2)
          self.Camera3TipRight(self.view3)
        else:
          self.Camera1TipRight(self.view1)
          self.Camera2Tip(self.view3)
          self.Camera3TipRight(self.view2)
      elif (cameraNode.GetName() == 'FC2'):
        if (cameraNode2.GetName() == 'FC1'):
          self.Camera1TipRight(self.view2)
          self.Camera2Tip(self.view1)
          self.Camera3TipRight(self.view3)
        else:
          self.Camera1TipRight(self.view2)
          self.Camera2Tip(self.view3)
          self.Camera3TipRight(self.view1)
      else:
        if (cameraNode2.GetName() == 'FC2'):
          self.Camera1TipRight(self.view3)
          self.Camera2Tip(self.view2)
          self.Camera3TipRight(self.view1)
        else:
          self.Camera1TipRight(self.view3)
          self.Camera2Tip(self.view1)
          self.Camera3TipRight(self.view2)

  #股骨视图选择
  def onViewSelect(self):
    rotationTransformNode = slicer.util.getNode('DianjiToTracker1')
    try:
      rotationTransformNode.RemoveObserver(self.ZuiDiDian)
    except:
      pass
    try:
      slicer.mrmlScene.RemoveNode(slicer.util.getNode('OutSide'))
      slicer.mrmlScene.RemoveNode(slicer.util.getNode('InSide'))
      slicer.mrmlScene.RemoveNode(self.JieGuJianXi)
    except:
      pass
    self.HideAll()
    self.ui.PopupWidget.setVisible(True)
    self.hideInformation()
    self.ThreeDState()
    self.ui.PopupWidget.setMinimumHeight(650)
    slicer.app.layoutManager().setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutTriple3DEndoscopyView)    
    slicer.modules.viewselect.widgetRepresentation().setParent(self.ui.PopupWidget)
    slicer.modules.viewselect.widgetRepresentation().setGeometry(-10,-10,624,624)
    slicer.modules.viewselect.widgetRepresentation().show()
    self.FemurButtonChecked(self.ui.ViewChoose)
    self.TibiaButtonChecked(self.ui.ViewChoose2)
    self.ui.ModuleName.setText('手术规划')
  #重置
  def onReset(self):
    message = qt.QMessageBox(qt.QMessageBox.Information,'重置',"是否要重置规划的方案？",qt.QMessageBox.Ok|qt.QMessageBox.Cancel)
    message.button(qt.QMessageBox().Ok).setText('是')
    message.button(qt.QMessageBox().Cancel).setText('否')
    c= message.exec()
    if c == qt.QMessageBox.Ok:
      self.FemurButtonChecked(self.ui.Reset)
      self.TibiaButtonChecked(self.ui.ReSet2)
      slicer.app.setOverrideCursor(qt.Qt.WaitCursor)  # 光标变成圆圈
      self.DeleteNode('变换')
      self.DeleteNode('变换_1')
      self.DeleteNode('变换_2')
      self.DeleteNode('变换_临时')
      self.DeleteNode('变换_R')
      self.DeleteNode(self.jiatiload.GetName())
      self.DeleteNode('股骨远端切割')
      self.DeleteNode('股骨第一截骨面')
      self.DeleteNode('股骨第二截骨面')
      self.DeleteNode('股骨第三截骨面')
      self.DeleteNode('DynamicModeler')
      self.DeleteNode('股骨远端')
      self.DeleteNode('切割1')
      self.DeleteNode('切割2')
      self.DeleteNode('切割3')
      self.DeleteNode('切割4')
      self.DeleteNode('切割5')
      self.DeleteNode('部件1')
      self.DeleteNode('部件2')
      self.DeleteNode('部件3')
      self.DeleteNode('部件4')
      self.DeleteNode('部件5')
      self.DeleteNode('动态切割1')
      self.DeleteNode('动态切割2')
      self.DeleteNode('动态切割3')
      self.DeleteNode('动态切割4')
      self.DeleteNode('动态切割5')
      self.DeleteNode('股骨切割')
      self.DeleteNode('变换_3')
      self.DeleteNode('变换_4')
      self.DeleteNode('变换_5')
      self.DeleteNode('变换_胫骨')
      self.DeleteNode('变换_约束')
      self.DeleteNode(self.TibiaJiaTiload.GetName())
      self.DeleteNode('胫骨近端切割')
      self.DeleteNode('胫骨截骨面')
      self.DeleteNode('胫骨近端')
      self.DeleteNode('DynamicModeler_1')
      self.DeleteNode('胫骨切割')
      self.DeleteNode('切割6')
      self.DeleteNode('部件6')
      self.DeleteNode('动态切割6')
      slicer.app.restoreOverrideCursor()  # 变回光标原来的形状

      # #显示所有的点
      # self.ShowNode('A点')
      # self.ShowNode('内侧后髁')
      # self.ShowNode('外侧后髁')
      # self.ShowNode('外侧远端')
      # self.ShowNode('内侧远端')
      # self.ShowNode('外侧凸点')
      # self.ShowNode('内侧凹点')
      # self.ShowNode('外侧皮质高点')
      # self.ShowNode('开髓点')
      # self.ShowNode('股骨头球心')
      # self.ShowNode('H点')
      #显示股骨并将股骨透明化
      Femur = slicer.util.getNode('Femur')
      Femur.SetDisplayVisibility(1)
      Femur.GetDisplayNode().SetOpacity(0.2)
      # 回到骨骼参数
      self.ui.Parameter.click()

    elif c == qt.QMessageBox.Cancel:
      self.ui.Reset.setChecked(False)
      self.ui.Reset2.setChecked(False)
  #----------------------------------------------------------------------------
  #显示力线
  def onForceLine(self):

    self.FemurButtonChecked(self.ui.ForceLine)
    self.TibiaButtonChecked(self.ui.ForceLine2)
    
    try:
      slicer.mrmlScene.RemoveNode(slicer.util.getNode('OutSide'))
      slicer.mrmlScene.RemoveNode(slicer.util.getNode('InSide'))
    except:
      pass
    self.HideAll()
    self.ui.Graph.show()
    self.ui.ForceWidget.setVisible(True)

    slicer.app.layoutManager().setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutTriple3DEndoscopyView)#将 view1,view2,view3添加到场景中
    self.ThreeDState()#三维视图状态正常
    self.hideInformation()
    self.DrawLine()
    slicer.app.layoutManager().threeDWidget('View1').installEventFilter(self.resizeEvent)
    slicer.app.layoutManager().setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutSideBySideView)
    self.ThreeDViewAndImageWidget(2)
    ras1, ras2, ras3, ras4 = [0, 0, 0], [0, 0, 0], [0, 0, 0], [0, 0, 0]
    slicer.util.getNode('股骨头球心').GetNthFiducialPosition(0, ras1)
    slicer.util.getNode('开髓点').GetNthFiducialPosition(0, ras2)
    slicer.util.getNode('外侧凸点').GetNthFiducialPosition(0, ras3)
    slicer.util.getNode('内侧凹点').GetNthFiducialPosition(0, ras4)
    zb1 = [-ras1[0], -ras1[1], ras1[2]]  # 坐标1，球心
    zb2 = [-ras2[0], -ras2[1], ras2[2]]  # 坐标2，原点
    zb3 = [-ras3[0], -ras3[1], ras3[2]]  # 坐标3，左侧点
    zb4 = [-ras4[0], -ras4[1], ras4[2]]  # 坐标4，右侧点
    jxlz = [0, 0, 0]  # Y轴基向量
    for i in range(0, 3):
        jxlz[i] = zb1[i] - zb2[i]
    moz = np.sqrt(np.square(jxlz[0]) + np.square(jxlz[1]) + np.square(jxlz[2]))  # 基向量z的模
    for i in range(0, 3):
        jxlz[i] = jxlz[i] / moz
    csD = jxlz[0] * zb2[0] + jxlz[1] * zb2[1] + jxlz[2] * zb2[2]  # 平面方程参数D
    csT3 = (jxlz[0] * zb3[0] + jxlz[1] * zb3[1] + jxlz[2] * zb3[2] - csD) / (
            jxlz[0] * jxlz[0] + jxlz[1] * jxlz[1] + jxlz[2] * jxlz[2])  # 坐标3平面方程参数T
    ty3 = [0, 0, 0]  # 坐标3在YZ平面的投影
    for i in range(0, 3):
        ty3[i] = zb3[i] - jxlz[i] * csT3
    csT4 = (jxlz[0] * zb4[0] + jxlz[1] * zb4[1] + jxlz[2] * zb4[2] - csD) / (
            jxlz[0] * jxlz[0] + jxlz[1] * jxlz[1] + jxlz[2] * jxlz[2])
    ty4 = [0, 0, 0]
    for i in range(0, 3):
        ty4[i] = zb4[i] - jxlz[i] * csT4
    jxlx = [0, 0, 0]  # X轴基向量
    for i in range(0, 3):
        if slicer.modules.NoImageWelcomeWidget.judge == 'L':
            jxlx[i] = ty3[i] - ty4[i]
        else:
            jxlx[i] = ty4[i] - ty3[i]
    mox = np.sqrt(np.square(jxlx[0]) + np.square(jxlx[1]) + np.square(jxlx[2]))  # 基向量X的模
    for i in range(0, 3):
        jxlx[i] = jxlx[i] / mox
    jxly = [0, 0, 0]  # y轴基向量
    jxly[0] = -(jxlx[1] * jxlz[2] - jxlx[2] * jxlz[1])
    jxly[1] = -(jxlx[2] * jxlz[0] - jxlx[0] * jxlz[2])
    jxly[2] = -(jxlx[0] * jxlz[1] - jxlx[1] * jxlz[0])
    moy = np.sqrt(np.square(jxly[0]) + np.square(jxly[1]) + np.square(jxly[2]))  # 基向量y的模
    for i in range(0, 3):
        jxly[i] = jxly[i] / moy
    ccb = ([jxlx, jxly, jxlz])
    ccc = np.asarray(ccb)
    ccd = ccc.T
    np.savetxt(self.FilePath + "/Femur-jxl.txt", ccd, fmt='%6f')
    Ftrans1 = np.array([[-1, 0, 0, 0],
                        [0, -1, 0, 0],
                        [0, 0, 1, 0],
                        [0, 0, 0, 1]])
    self.Ftrans2 = np.array([[float(jxlx[0]), float(jxly[0]), float(jxlz[0]), zb2[0]],
                            [float(jxlx[1]), float(jxly[1]), float(jxlz[1]), zb2[1]],
                            [float(jxlx[2]), float(jxly[2]), float(jxlz[2]), zb2[2]],
                            [0, 0, 0, 1]])
    self.FemurForceTrans = np.dot(Ftrans1,self.Ftrans2)
    
    ras1, ras2, ras3= [0, 0, 0], [0, 0, 0], [0, 0, 0]
    slicer.util.getNode('胫骨隆凸').GetNthFiducialPosition(0, ras1)
    slicer.util.getNode('胫骨结节').GetNthFiducialPosition(0, ras2)
    slicer.util.getNode('踝穴中心').GetNthFiducialPosition(0, ras3)

    Tzb1 = [-ras1[0], -ras1[1], ras1[2]]  # 坐标1，原点，髌骨近端的点
    Tzb2 = [-ras2[0], -ras2[1], ras2[2]]  # 坐标2，髌骨中间的点
    Tzb3 = [-ras3[0], -ras3[1], ras3[2]]  # 坐标2，髌骨远端的点
    Tjxlz = [0, 0, 0]  # z轴基向量
    for i in range(0, 3):
        Tjxlz[i] = Tzb1[i] - Tzb3[i]
    moz = np.sqrt(np.square(Tjxlz[0]) + np.square(Tjxlz[1]) + np.square(Tjxlz[2]))  # 基向量z的模
    for i in range(0, 3):
        Tjxlz[i] = Tjxlz[i] / moz
    TcsD = Tjxlz[0] * Tzb1[0] + Tjxlz[1] * Tzb1[1] + Tjxlz[2] * Tzb1[2]  # 平面方程参数D
    TcsT2 = (Tjxlz[0] * Tzb2[0] + Tjxlz[1] * Tzb2[1] + Tjxlz[2] * Tzb2[2] - TcsD) / (
            Tjxlz[0] * Tjxlz[0] + Tjxlz[1] * Tjxlz[1] + Tjxlz[2] * Tjxlz[2])  # 坐标3平面方程参数T
    Tty2 = [0, 0, 0]  # 坐标2在XY平面的投影
    for i in range(0, 3):
        Tty2[i] = Tzb2[i] - Tjxlz[i] * TcsT2
    Tjxly = [0, 0, 0]  # y轴基向量
    for i in range(0, 3):
        Tjxly[i] = Tzb1[i] - Tty2[i]
    moy = np.sqrt(np.square(Tjxly[0]) + np.square(Tjxly[1]) + np.square(Tjxly[2]))  # 基向量y的模
    for i in range(0, 3):
        Tjxly[i] = Tjxly[i] / moy
    Tjxlx = [0, 0, 0]  # x轴基向量
    Tjxlx[0] = -(Tjxlz[1] * Tjxly[2] - Tjxlz[2] * Tjxly[1])
    Tjxlx[1] = -(Tjxlz[2] * Tjxly[0] - Tjxlz[0] * Tjxly[2])
    Tjxlx[2] = -(Tjxlz[0] * Tjxly[1] - Tjxlz[1] * Tjxly[0])
    mox = np.sqrt(np.square(Tjxly[0]) + np.square(Tjxly[1]) + np.square(Tjxly[2]))  # 基向量x的模
    for i in range(0, 3):
        Tjxlx[i] = Tjxlx[i] / mox
    Tzb3xz = []
    jd = 1
    jd = math.radians(jd)
    zhjz = np.array([[Tjxlx[0], Tjxly[0], Tjxlz[0], Tzb1[0]], [Tjxlx[1], Tjxly[1], Tjxlz[1], Tzb1[1]],
            [Tjxlx[2], Tjxly[2], Tjxlz[2], Tzb1[2]], [0, 0, 0, 1]])
    Tzb3xz3 = self.GetMarix(zhjz,1,Tzb3)
    # for i in range(0, 3):
    #     Tzb3[i] = Tzb3xz3[i]
    Tjxlz = [0, 0, 0]  # z轴基向量
    for i in range(0, 3):
        Tjxlz[i] = Tzb1[i] - Tzb3[i]
    moz = np.sqrt(np.square(Tjxlz[0]) + np.square(Tjxlz[1]) + np.square(Tjxlz[2]))  # 基向量z的模
    for i in range(0, 3):
        Tjxlz[i] = Tjxlz[i] / moz
    TcsD = Tjxlz[0] * Tzb1[0] + Tjxlz[1] * Tzb1[1] + Tjxlz[2] * Tzb1[2]  # 平面方程参数D
    TcsT2 = (Tjxlz[0] * Tzb2[0] + Tjxlz[1] * Tzb2[1] + Tjxlz[2] * Tzb2[2] - TcsD) / (
            Tjxlz[0] * Tjxlz[0] + Tjxlz[1] * Tjxlz[1] + Tjxlz[2] * Tjxlz[2])  # 坐标3平面方程参数T
    Tty2 = [0, 0, 0]  # 坐标2在XY平面的投影
    for i in range(0, 3):
        Tty2[i] = Tzb2[i] - Tjxlz[i] * TcsT2
    Tjxly = [0, 0, 0]  # y轴基向量
    for i in range(0, 3):
        Tjxly[i] = Tzb1[i] - Tty2[i]
    moy = np.sqrt(np.square(Tjxly[0]) + np.square(Tjxly[1]) + np.square(Tjxly[2]))  # 基向量y的模
    for i in range(0, 3):
        Tjxly[i] = Tjxly[i] / moy
    Tjxlx = [0, 0, 0]  # X轴基向量
    Tjxlx[0] = -(Tjxlz[1] * Tjxly[2] - Tjxlz[2] * Tjxly[1])
    Tjxlx[1] = -(Tjxlz[2] * Tjxly[0] - Tjxlz[0] * Tjxly[2])
    Tjxlx[2] = -(Tjxlz[0] * Tjxly[1] - Tjxlz[1] * Tjxly[0])
    mox = np.sqrt(np.square(Tjxlx[0]) + np.square(Tjxlx[1]) + np.square(Tjxlx[2]))  # 基向量x的模
    for i in range(0, 3):
        Tjxlx[i] = Tjxlx[i] / mox
    ccb = ([Tjxlx, Tjxly, Tjxlz])
    ccc = np.asarray(ccb)
    ccd = ccc.T
    np.savetxt(self.FilePath + "/Tibia-jxl.txt", ccd, fmt='%6f')
    Ttrans1 = np.array([[-1, 0, 0, 0],
                        [0, -1, 0, 0],
                        [0, 0, 1, 0],
                        [0, 0, 0, 1]])
    Ttrans2 = np.array([[float(Tjxlx[0]), float(Tjxly[0]), float(Tjxlz[0]), Tzb1[0]],
                            [float(Tjxlx[1]), float(Tjxly[1]), float(Tjxlz[1]), Tzb1[1]],
                            [float(Tjxlx[2]), float(Tjxly[2]), float(Tjxlz[2]), Tzb1[2]],
                            [0, 0, 0, 1]])
    self.TibiaForceTrans = np.dot(Ttrans1,Ttrans2)

    self.ForceLineImage()

    self.interactorStyle1.SetInteractor(None)
    self.interactorStyle2.SetInteractor(None)
    rotationTransformNode = slicer.util.getNode('DianjiToTracker1')
    try:
      rotationTransformNode.RemoveObserver(self.ZuiDiDian)
    except:
      pass
    self.updataForceAngle = rotationTransformNode.AddObserver(
      slicer.vtkMRMLTransformNode.TransformModifiedEvent, self.updataAngle)
    self.ForceCamera1(self.view1)
    self.ForceCamera2(self.view2)
    self.view1.resetFocalPoint()
    self.view2.resetFocalPoint()

  
  def ForceLineImage(self):
    import textwrap
    from PySide2.QtWidgets import QVBoxLayout
    import shiboken2
    from pyqtgraph.Qt import QtGui, QtCore
    import pyqtgraph as pg
    self.ui.GraphImage.setAutoFillBackground(True)
    viewLayout = qt.QVBoxLayout()
    self.ui.GraphImage.setLayout(viewLayout)
    pg.setConfigOption('background', '#454647')
    win = pg.GraphicsLayoutWidget(show=True, title="Basic plotting examples")
    layout = shiboken2.wrapInstance(hash(viewLayout),QVBoxLayout)
    layout.addWidget(win)
    pg.setConfigOptions(antialias=True)
    self.p1 = win.addPlot()
    self.p1.showGrid(True,True,0.1)
    line = self.p1.plot([0,0],[0,0],pen='#7cbd27')
    self.lineList=[]
    for i in range(141):
      self.lineList.append(0)
    self.lineList2=[]
    for i in range(141):
      self.lineList2.append(0)

    
    #显示图例
    legend3 = pg.LegendItem((20,5), offset=(10,27))
    legend3.setParentItem(self.p1)
    legend3.addItem(line, '角度(°)')
    ticks1 = [-20,-10,-3,0,3,10,20]
    ticks = [-10,0,10,20,30,40,50,60,70,80,90,100,110,120,130]
    #Y轴
    ay = self.p1.getAxis('left')
    ay.setTicks([[(v, str(v)) for v in ticks ]])
    #X轴
    ax = self.p1.getAxis('bottom')   
    ax.setTicks([[(v, str(v)) for v in ticks1 ]])  

  def ForceCamera1(self, view):
    cameraNode = view.cameraNode()
    positiontmp = np.array([0, -1500, 0, 1])
    Ftrans1 = np.array([[-1, 0, 0, 0],
                        [0, -1, 0, 0],
                        [0, 0, 1, 0],
                        [0, 0, 0, 1]])
    transNode = slicer.util.getNode('DianjiToTracker1')
    Ftrans_dj = slicer.util.arrayFromTransformMatrix(transNode)
    Ftrans1=np.dot(Ftrans_dj,Ftrans1)
    Ftrans3 = np.dot(Ftrans1, self.Ftrans2)
    position1 = np.dot(Ftrans3, positiontmp)
    viewUpDirection = (float(Ftrans3[0][2]), float(Ftrans3[1][2]), float(Ftrans3[2][2]))
    focalPoint1 = [Ftrans3[0][3], Ftrans3[1][3], Ftrans3[2][3]]
    cameraNode.SetFocalPoint(focalPoint1)
    cameraNode.SetPosition(position1[0], position1[1], position1[2])
    cameraNode.SetViewUp(viewUpDirection)
    cameraNode.SetAndObserveTransformNodeID(transNode.GetID())


  def ForceCamera2(self, view):
    cameraNode = view.cameraNode()
    positiontmp = np.array([-1500, 0, 0, 1])
    Ftrans1 = np.array([[-1, 0, 0, 0],
                        [0, -1, 0, 0],
                        [0, 0, 1, 0],
                        [0, 0, 0, 1]])
    transNode = slicer.util.getNode('DianjiToTracker1')
    Ftrans_dj = slicer.util.arrayFromTransformMatrix(transNode)
    Ftrans1 = np.dot(Ftrans_dj, Ftrans1)
    Ftrans3 = np.dot(Ftrans1, self.Ftrans2)
    position2 = np.dot(Ftrans3, positiontmp)
    viewUpDirection = (float(Ftrans3[0][2]), float(Ftrans3[1][2]), float(Ftrans3[2][2]))
    focalPoint2 = [Ftrans3[0][3], Ftrans3[1][3], Ftrans3[2][3]]
    cameraNode.SetFocalPoint(focalPoint2)
    cameraNode.SetPosition(position2[0], position2[1], position2[2])
    cameraNode.SetViewUp(viewUpDirection)
    cameraNode.SetAndObserveTransformNodeID(transNode.GetID())

  
  def onForceConfirm(self):
    angle = 0
    self.ui.ForceAngle.setText('力线角度：'+str(angle))
  
  #画外翻角角度
  def DrawLine(self):
    try:
      slicer.mrmlScene.RemoveNode(slicer.util.getNode("FemurLine"))
      slicer.mrmlScene.RemoveNode(slicer.util.getNode("TibiaLine"))      
    except:
      pass

    ras1,ras2,ras3,ras4 = [0,0,0],[0,0,0],[0,0,0],[0,0,0]
    slicer.util.getNode('股骨头球心').GetNthControlPointPositionWorld(0, ras1)
    slicer.util.getNode('开髓点').GetNthControlPointPositionWorld(0, ras2)
    slicer.util.getNode('胫骨隆凸').GetNthControlPointPositionWorld(0, ras3)
    slicer.util.getNode('踝穴中心').GetNthControlPointPositionWorld(0, ras4)
    ras5 = [ras2[0]+ras4[0]-ras3[0],ras2[1]+ras4[1]-ras3[1],ras2[2]+ras4[2]-ras3[2]]
    FemurLine = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsLineNode','FemurLine')
    FemurLine.AddControlPoint(ras1)
    FemurLine.AddControlPoint(ras2)
    FemurLine.GetDisplayNode().SetPropertiesLabelVisibility(0)
    FemurLine.GetDisplayNode().SetGlyphScale(2) 
    FemurLine.GetDisplayNode().SetSelectedColor(0.48627450980392156, 0.7411764705882353, 0.15294117647058825)
    FemurLine.SetNthControlPointLocked(0,True)
    FemurLine.SetNthControlPointLocked(1,True)
    TibiaLine = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsLineNode','TibiaLine')
    TibiaLine.AddControlPoint(ras3)
    TibiaLine.AddControlPoint(ras4)
    TibiaLine.GetDisplayNode().SetPropertiesLabelVisibility(0)
    TibiaLine.GetDisplayNode().SetGlyphScale(2) 
    TibiaLine.GetDisplayNode().SetSelectedColor(0.48627450980392156, 0.7411764705882353, 0.15294117647058825)
    TibiaLine.SetNthControlPointLocked(0,True)
    TibiaLine.SetNthControlPointLocked(1,True)
    # WaiFanJiao = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsAngleNode','Angle')
    # WaiFanJiao.AddControlPoint(ras1)
    # WaiFanJiao.AddControlPoint(ras2)
    # WaiFanJiao.AddControlPoint(ras5)
    # WaiFanJiao.GetDisplayNode().SetTextScale(3)
    # WaiFanJiao.SetNthControlPointLocked(0,True)
    # WaiFanJiao.SetNthControlPointLocked(1,True)
    # WaiFanJiao.SetNthControlPointLocked(2,True)
    xiangliang1 = [ras1[0]-ras2[0],ras1[1]-ras2[1],ras1[2]-ras2[2]]
    xiangliang2 = [ras4[0]-ras3[0],ras4[1]-ras3[1],ras4[2]-ras3[2]]
    angle = self.Angle(xiangliang1,xiangliang2)
    print(angle)
    self.ForceLabel1 = qt.QLabel(self.view1)
    self.ForceLabel2 = qt.QLabel(self.view2)
    self.ForceLabel1.setGeometry(self.view1.contentsRect().width()/2, 25, 200, 40)
    self.ForceLabel1.setText("外翻角度："+str(round(angle,2))+'°')
    self.ForceLabel1.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    self.ForceLabel1.show()
    self.ForceLabel2.setGeometry(self.view2.contentsRect().width()/2, 25, 200, 40)
    self.ForceLabel2.setText("屈膝角度："+str(round(angle,2))+'°')
    self.ForceLabel2.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    self.ForceLabel2.show()
  
  #实时更新角度
  def updataAngle(self,unusedArg1=None, unusedArg2=None, unusedArg3=None):
    import pyqtgraph as pg
    #更新画的两条线
    FemurLine = slicer.util.getNode('FemurLine')
    TibiaLine = slicer.util.getNode('TibiaLine')
    ras1,ras2,ras3,ras4 = [0,0,0],[0,0,0],[0,0,0],[0,0,0]
    slicer.util.getNode('股骨头球心').GetNthControlPointPositionWorld(0, ras1)
    slicer.util.getNode('开髓点').GetNthControlPointPositionWorld(0, ras2)
    slicer.util.getNode('胫骨隆凸').GetNthControlPointPositionWorld(0, ras3)
    slicer.util.getNode('踝穴中心').GetNthControlPointPositionWorld(0, ras4)
    FemurLine.SetNthControlPointPosition(0,ras1)
    FemurLine.SetNthControlPointPosition(1,ras2)
    TibiaLine.SetNthControlPointPosition(0,ras3)
    TibiaLine.SetNthControlPointPosition(1,ras4)

    #更新角度
    transNode=slicer.util.getNode('DianjiToTracker1')
    trans_dj=slicer.util.arrayFromTransformMatrix(transNode)
    TibiaNode = slicer.util.getNode('TibiaToTracker')
    trans_Tibia=slicer.util.arrayFromTransformMatrix(TibiaNode)
    Femurtans = np.dot(trans_dj,self.FemurForceTrans)
    Tibiatrans = np.dot(trans_Tibia,self.TibiaForceTrans)
    FemurPoint = np.array([[0.0, 0.0, 0.0, 1.0], [100.0, 0.0, 0.0, 1.0], [0.0, 100.0, 0.0, 1.0], [0.0, 0.0, 100.0, 1.0]])
    TibiaPoint = np.array([[0.0, 0.0, 0.0, 1.0], [100.0, 0.0, 0.0, 1.0], [0.0, 100.0, 0.0, 1.0], [0.0, 0.0, 100.0, 1.0]])
    for i in range(4):
      FemurPoint[i]=np.dot(Femurtans,FemurPoint[i])
    for i in range(4):
      TibiaPoint[i]=np.dot(Tibiatrans,TibiaPoint[i])
    Tibia_YZPlane=TibiaPoint[[0,2,3],0:3]
    Tibia_XZPlane = TibiaPoint[[0,1,3],0:3]
    Femur_Z = self.TouYing(Tibia_YZPlane,FemurPoint[3][0:3])
    Femur_L = self.TouYing(Tibia_YZPlane,FemurPoint[0][0:3])
    Femur_ZAxis_Z = np.array(Femur_Z)-np.array(Femur_L)

    Femur_Z1 = self.TouYing(Tibia_XZPlane,FemurPoint[1][0:3])
    Femur_L1 = self.TouYing(Tibia_XZPlane,FemurPoint[0][0:3])
    Femur_XAxis_Z = np.array(Femur_Z1)-np.array(Femur_L1)

    Tibia_XAxis = (TibiaPoint[1]-TibiaPoint[0])[0:3]
    Tibia_YAxis = (TibiaPoint[2]-TibiaPoint[0])[0:3]
    Tibia_ZAxis = (TibiaPoint[3]-TibiaPoint[0])[0:3]

    quxiAngle = self.Angle(Femur_ZAxis_Z,Tibia_ZAxis)
    
    Ifzf1 = np.dot(Femur_ZAxis_Z,Tibia_YAxis)
    Ifzf2 = np.dot(Femur_XAxis_Z,Tibia_ZAxis)
    if Ifzf2 < 0:
      waifanAngle = -float(self.angle(Femur_XAxis_Z, Tibia_XAxis))
    else:
      waifanAngle = float(self.angle(Femur_XAxis_Z, Tibia_XAxis))
    if Ifzf1<0:
      quxiAngle = -quxiAngle
    
    self.ForceLabel1.setText('外翻角度：'+str(round(waifanAngle,1))+'°')
    self.ForceLabel2.setText('屈膝角度：'+str(round(quxiAngle,1))+'°')
    #更新图
    if Ifzf1<0:
      if Ifzf2>0:#屈膝角度是负值，外翻角度是正值
        if self.lineList[10-int(float(quxiAngle))]==0 and (10-int(float(quxiAngle)))%2 == 0:
          line=self.p1.plot([0,waifanAngle],[-int(quxiAngle),-int(quxiAngle)],pen=pg.mkPen('#7cbd27',width=5))
          self.lineList[10-int(float(quxiAngle))]=line
        elif self.lineList[10-int(float(quxiAngle))]!=0 and (10-int(float(quxiAngle)))%2 == 0:
          a = self.lineList[10-int(float(quxiAngle))].getData()
          if waifanAngle>a[0][1]:
            self.lineList[10-int(float(quxiAngle))].setData([0,waifanAngle],[-int(quxiAngle),-int(quxiAngle)])
      else:#屈膝角度是负值，外翻角度是负值
        if self.lineList2[10-int(float(quxiAngle))]==0 and (10-int(float(quxiAngle)))%2 == 0:#画线区间为2
          line=self.p1.plot([0,waifanAngle],[-int(quxiAngle),-int(quxiAngle)],pen=pg.mkPen('#7cbd27',width=5))
          self.lineList2[10-int(float(quxiAngle))]=line
        elif self.lineList2[10-int(float(quxiAngle))]!=0 and (10-int(float(quxiAngle)))%2 == 0:
          a = self.lineList2[10-int(float(quxiAngle))].getData()
          if waifanAngle<a[0][1]:
            self.lineList2[10-int(float(quxiAngle))].setData([0,waifanAngle],[-int(quxiAngle),-int(quxiAngle)])
    else:
      if Ifzf2>0:#屈膝角度是正值，外翻角度是正值
        if self.lineList[int(float(quxiAngle))-1]==0 and (int(float(quxiAngle))-1)%2 == 0:
          line=self.p1.plot([0,waifanAngle],[int(quxiAngle),int(quxiAngle)],pen=pg.mkPen('#7cbd27',width=5))
          self.lineList[int(float(quxiAngle))-1]=line
        elif self.lineList[int(float(quxiAngle))-1]!=0 and (int(float(quxiAngle))-1)%2 == 0:
          a = self.lineList[int(float(quxiAngle))-1].getData()
          if waifanAngle>a[0][1]:
            self.lineList[int(float(quxiAngle))-1].setData([0,waifanAngle],[int(quxiAngle),int(quxiAngle)])
      else:#屈膝角度是正值，外翻角度是负值
        if self.lineList2[int(float(quxiAngle))-1]==0 and (int(float(quxiAngle))-1)%2 == 0:
          line=self.p1.plot([0,waifanAngle],[int(quxiAngle),int(quxiAngle)],pen=pg.mkPen('#7cbd27',width=5))
          self.lineList2[int(float(quxiAngle))-1]=line
        elif self.lineList2[int(float(quxiAngle))-1]!=0 and (int(float(quxiAngle))-1)%2 == 0:
          a = self.lineList2[int(float(quxiAngle))-1].getData()
          if waifanAngle<a[0][1]:
            self.lineList2[int(float(quxiAngle))-1].setData([0,waifanAngle],[int(quxiAngle),int(quxiAngle)])
    
    self.ForceCamera1(self.view1)
    self.ForceCamera2(self.view2)
    self.currentX=quxiAngle
    self.currentY=waifanAngle



  #------------------------胫骨-----------------------------------------------
  #胫骨骨骼参数
  def onParameter2(self):
    self.TibiaButtonChecked(self.ui.Parameter2)
    self.HideAll()
    self.ui.OperationPlanWidget.setVisible(True)
    self.ui.GuGe.setVisible(True)
    ROIs = slicer.util.getNodesByClass('vtkMRMLMarkupsROINode')
    models = slicer.util.getNodesByClass('vtkMRMLModelNode')

    

    Name = []
    for i in range(0, len(models)):
      a = models[i]
      Name.append(a.GetName())
    if '胫骨近端' in Name:
      self.HidePart()
      self.ShowNode('胫骨近端')
    else:
      if len(ROIs) < 9:
        Tibia1 = slicer.util.getNode('Tibia')
        Tibia1.GetDisplayNode().SetOpacity(1)
        ras1, ras2, ras3= [0, 0, 0], [0, 0, 0], [0, 0, 0]
        slicer.util.getNode('胫骨隆凸').GetNthFiducialPosition(0, ras1)
        slicer.util.getNode('胫骨结节').GetNthFiducialPosition(0, ras2)
        slicer.util.getNode('踝穴中心').GetNthFiducialPosition(0, ras3)

        Tzb1 = [-ras1[0], -ras1[1], ras1[2]]  # 坐标1，原点，髌骨近端的点
        Tzb2 = [-ras2[0], -ras2[1], ras2[2]]  # 坐标2，髌骨中间的点
        Tzb3 = [-ras3[0], -ras3[1], ras3[2]]  # 坐标2，髌骨远端的点
        Tjxlz = [0, 0, 0]  # z轴基向量
        for i in range(0, 3):
            Tjxlz[i] = Tzb1[i] - Tzb3[i]
        moz = np.sqrt(np.square(Tjxlz[0]) + np.square(Tjxlz[1]) + np.square(Tjxlz[2]))  # 基向量z的模
        for i in range(0, 3):
            Tjxlz[i] = Tjxlz[i] / moz
        TcsD = Tjxlz[0] * Tzb1[0] + Tjxlz[1] * Tzb1[1] + Tjxlz[2] * Tzb1[2]  # 平面方程参数D
        TcsT2 = (Tjxlz[0] * Tzb2[0] + Tjxlz[1] * Tzb2[1] + Tjxlz[2] * Tzb2[2] - TcsD) / (
                Tjxlz[0] * Tjxlz[0] + Tjxlz[1] * Tjxlz[1] + Tjxlz[2] * Tjxlz[2])  # 坐标3平面方程参数T
        Tty2 = [0, 0, 0]  # 坐标2在XY平面的投影
        for i in range(0, 3):
            Tty2[i] = Tzb2[i] - Tjxlz[i] * TcsT2
        Tjxly = [0, 0, 0]  # y轴基向量
        for i in range(0, 3):
            Tjxly[i] = Tzb1[i] - Tty2[i]
        moy = np.sqrt(np.square(Tjxly[0]) + np.square(Tjxly[1]) + np.square(Tjxly[2]))  # 基向量y的模
        for i in range(0, 3):
            Tjxly[i] = Tjxly[i] / moy
        Tjxlx = [0, 0, 0]  # x轴基向量
        Tjxlx[0] = -(Tjxlz[1] * Tjxly[2] - Tjxlz[2] * Tjxly[1])
        Tjxlx[1] = -(Tjxlz[2] * Tjxly[0] - Tjxlz[0] * Tjxly[2])
        Tjxlx[2] = -(Tjxlz[0] * Tjxly[1] - Tjxlz[1] * Tjxly[0])
        mox = np.sqrt(np.square(Tjxly[0]) + np.square(Tjxly[1]) + np.square(Tjxly[2]))  # 基向量x的模
        for i in range(0, 3):
            Tjxlx[i] = Tjxlx[i] / mox
        Tzb3xz = []
        jd = 1
        jd = math.radians(jd)
        zhjz = np.array([[Tjxlx[0], Tjxly[0], Tjxlz[0], Tzb1[0]], [Tjxlx[1], Tjxly[1], Tjxlz[1], Tzb1[1]],
                [Tjxlx[2], Tjxly[2], Tjxlz[2], Tzb1[2]], [0, 0, 0, 1]])
        Tzb3xz3 = self.GetMarix(zhjz,1,Tzb3)
        # for i in range(0, 3):
        #     Tzb3[i] = Tzb3xz3[i]
        Tjxlz = [0, 0, 0]  # z轴基向量
        for i in range(0, 3):
            Tjxlz[i] = Tzb1[i] - Tzb3[i]
        moz = np.sqrt(np.square(Tjxlz[0]) + np.square(Tjxlz[1]) + np.square(Tjxlz[2]))  # 基向量z的模
        for i in range(0, 3):
            Tjxlz[i] = Tjxlz[i] / moz
        TcsD = Tjxlz[0] * Tzb1[0] + Tjxlz[1] * Tzb1[1] + Tjxlz[2] * Tzb1[2]  # 平面方程参数D
        TcsT2 = (Tjxlz[0] * Tzb2[0] + Tjxlz[1] * Tzb2[1] + Tjxlz[2] * Tzb2[2] - TcsD) / (
                Tjxlz[0] * Tjxlz[0] + Tjxlz[1] * Tjxlz[1] + Tjxlz[2] * Tjxlz[2])  # 坐标3平面方程参数T
        Tty2 = [0, 0, 0]  # 坐标2在XY平面的投影
        for i in range(0, 3):
            Tty2[i] = Tzb2[i] - Tjxlz[i] * TcsT2
        Tjxly = [0, 0, 0]  # y轴基向量
        for i in range(0, 3):
            Tjxly[i] = Tzb1[i] - Tty2[i]
        moy = np.sqrt(np.square(Tjxly[0]) + np.square(Tjxly[1]) + np.square(Tjxly[2]))  # 基向量y的模
        for i in range(0, 3):
            Tjxly[i] = Tjxly[i] / moy
        Tjxlx = [0, 0, 0]  # X轴基向量
        Tjxlx[0] = -(Tjxlz[1] * Tjxly[2] - Tjxlz[2] * Tjxly[1])
        Tjxlx[1] = -(Tjxlz[2] * Tjxly[0] - Tjxlz[0] * Tjxly[2])
        Tjxlx[2] = -(Tjxlz[0] * Tjxly[1] - Tjxlz[1] * Tjxly[0])
        mox = np.sqrt(np.square(Tjxlx[0]) + np.square(Tjxlx[1]) + np.square(Tjxlx[2]))  # 基向量x的模
        for i in range(0, 3):
            Tjxlx[i] = Tjxlx[i] / mox
        ccb = ([Tjxlx, Tjxly, Tjxlz])
        ccc = np.asarray(ccb)
        ccd = ccc.T
        np.savetxt(self.FilePath + "/Tibia-jxl.txt", ccd, fmt='%6f')
        Ttrans1 = np.array([[-1, 0, 0, 0],
                            [0, -1, 0, 0],
                            [0, 0, 1, 0],
                            [0, 0, 0, 1]])
        self.Ttrans2 = np.array([[float(Tjxlx[0]), float(Tjxly[0]), float(Tjxlz[0]), Tzb1[0]],
                                [float(Tjxlx[1]), float(Tjxly[1]), float(Tjxlz[1]), Tzb1[1]],
                                [float(Tjxlx[2]), float(Tjxly[2]), float(Tjxlz[2]), Tzb1[2]],
                                [0, 0, 0, 1]])
        Ttrans3 = np.array([[1, 0, 0, 0],
                            [0, 1, 0, 0],
                            [0, 0, 1, 0],
                            [0, 0, 0, 1]])
        Ttrans3_5 = np.array([[1, 0, 0, 9],
                            [0, 1, 0, -5],
                            [0, 0, 1, -10],
                            [0, 0, 0, 1]])
        Ttransform1 = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", '变换_3')
        Ttransform2 = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", '变换_4')
        Ttransform3 = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", '变换_5')
        TtransformTmp = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", "变换_胫骨")
        TtransformYueShu = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", "变换_约束")
        Ttransform1.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(Ttrans1))
        Ttransform2.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(self.Ttrans2))
        Ttransform3.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(Ttrans3))
        TtransformTmp.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(Ttrans3))
        TtransformYueShu.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(Ttrans3))
        Ttransform2.SetAndObserveTransformNodeID(Ttransform1.GetID())
        TtransformTmp.SetAndObserveTransformNodeID(Ttransform2.GetID())
        Ttransform3.SetAndObserveTransformNodeID(TtransformTmp.GetID())
        TtransformYueShu.SetAndObserveTransformNodeID(Ttransform3.GetID())
        self.EtctMove('变换_3', 'TibiaToTracker')
        self.addAxisTibia()
        self.SelectTibiaJiaTi()
        #self.TibiaJiaTiload.SetAndObserveTransformNodeID(Ttransform3.GetID())
        #self.TibiaJiaTiload.SetDisplayVisibility(False)
        roiNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsROINode', '胫骨近端切割')
        roiNode.SetCenter([0, 0, 0])
        roiNode.SetSize([100, 100, 140])
        roiNode.SetDisplayVisibility(False)
        roiNode.SetAndObserveTransformNodeID(TtransformYueShu.GetID())

        TibiaJinDuan = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLModelNode", "胫骨近端")
        inputModel = slicer.util.getNode('Tibia')
        inputROI = roiNode
        dynamicModelerNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLDynamicModelerNode")
        dynamicModelerNode.SetToolName("ROI cut")
        dynamicModelerNode.SetNodeReferenceID("ROICut.InputModel", inputModel.GetID())
        dynamicModelerNode.SetNodeReferenceID("ROICut.InputROI", inputROI.GetID())
        dynamicModelerNode.SetNodeReferenceID("ROICut.OutputPositiveModel", TibiaJinDuan.GetID())
        # dynamicModelerNode.SetContinuousUpdate(1)
        dynamicModelerNode.SetAttribute("ROICut.CapSurface", '1')
        slicer.modules.dynamicmodeler.logic().RunDynamicModelerTool(dynamicModelerNode)
        inputModel.SetDisplayVisibility(False)
        slicer.util.getNode('Tibia').SetDisplayVisibility(False)
        self.NodeMove('胫骨近端', 'TibiaToTracker')

    self.TCamera1(self.view1)
    self.TCamera2(self.view2)
    self.TCamera3(self.view3)
    self.ui.ModuleName.setText('手术规划')
  
  #确定胫骨截骨面，并正确放置假体位置     
  def TibiaJieGu(self):
    # 胫骨截骨面
    TibiaJieGu = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', '胫骨截骨面')
    TibiaJieGu.AddControlPoint(30, 0, 0)
    TibiaJieGu.AddControlPoint(0, 30, 0)
    TibiaJieGu.AddControlPoint(0, 0, 0)
    TtransformYueShu = slicer.util.getNode('变换_约束')
    TibiaJieGu.SetAndObserveTransformNodeID(TtransformYueShu.GetID())
    point = [0, 0, 0]
    point1 = [0, 0, 0]
    point3 = [0, 0, 0]
    slicer.util.getNode('内侧高点').GetNthControlPointPositionWorld(0, point)
    
    self.HideNode('胫骨截骨面')
    TibiaJGM = self.GetTransPoint('胫骨截骨面')
    pointTouYing = self.TouYing(TibiaJGM,point)
    xiangliang = [pointTouYing[0]-point[0],pointTouYing[1]-point[1],pointTouYing[2]-point[2]]

    z = [self.Ttrans2[0][2],self.Ttrans2[1][2],self.Ttrans2[2][2]]
    x = np.dot(xiangliang,z)
    print('x',x)
    d = self.point2area_distance(TibiaJGM, point)
    print('d:',d)
    if x < 0:
      d = -d
    distance = 6 + d

    slicer.util.getNode('外侧高点').GetNthControlPointPositionWorld(0, point1)
    slicer.util.getNode('胫骨隆凸').GetNthControlPointPositionWorld(0, point3)

    #胫骨变换
    transformNode3 = slicer.util.getNode('变换_3')
    trans3 = slicer.util.arrayFromTransformMatrix(transformNode3)
    transformNode4 = slicer.util.getNode('变换_4')
    trans4 = slicer.util.arrayFromTransformMatrix(transformNode4)
    transformNodeTib = slicer.util.getNode('变换_胫骨')
    transTib = slicer.util.arrayFromTransformMatrix(transformNodeTib)
    transformNode5 = slicer.util.getNode('变换_5')
    trans5 = slicer.util.arrayFromTransformMatrix(transformNode5)
    transformNodeYueShu = slicer.util.getNode('变换_约束')
    transYueShu = slicer.util.arrayFromTransformMatrix(transformNodeYueShu)
    rotationTransformNode = slicer.util.getNode('DianjiToTracker1')
    transdj = slicer.util.arrayFromTransformMatrix(rotationTransformNode)

    #胫骨变换相乘
    trans3=np.dot(transdj,trans3)

    #胫骨变换相乘
    TibiaTrans = np.dot(np.dot(np.dot(np.dot(trans3,trans4),transTib),trans5),transYueShu)
    # 胫骨变换矩阵求逆
    TibiaTrans_ni=np.linalg.inv(TibiaTrans)
    point=[point[0],point[1],point[2],1]
    point1 = [point1[0], point1[1], point1[2], 1]
    point3 = [point3[0], point3[1], point3[2], 1]
    point_ni=np.dot(TibiaTrans_ni,point)
    point1_ni = np.dot(TibiaTrans_ni, point1)
    point3_ni = np.dot(TibiaTrans_ni, point3)
    point2 = [(point_ni[0]+point1_ni[0]+point3_ni[0])/3,(point_ni[1]+point1_ni[1]+point3_ni[1])/3,(point_ni[2]+point1_ni[2]+point3_ni[2])/3]
    a = [point2[0] - point3_ni[0], point2[1] - point3_ni[1], point2[2] - point3_ni[2]]
    TransformTmp = slicer.util.getNode('变换_胫骨')
    TtransTmp = np.array([[1, 0, 0, -a[0]],
                    [0, 1, 0, -a[1]],
                    [0, 0, 1, -distance],
                    [0, 0, 0, 1]])

    print('TtransTmp',TtransTmp,'a',a)
    xzjz = self.GetMarix_z(-2)
    trans = np.dot(TtransTmp,xzjz)
    TransformTmp.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(trans))
    rotationTransformNode = slicer.util.getNode('DianjiToTracker1')
    try:
      rotationTransformNode.RemoveObserver(self.updataForceAngle)
    except:
      pass
    self.ZuiDiDian = rotationTransformNode.AddObserver(
      slicer.vtkMRMLTransformNode.TransformModifiedEvent, self.caculateLowPoint)
    self.onJieGuJianXi()

  #胫骨变换合集
  def TibiaTrans(self):
      transform3 = slicer.util.arrayFromTransformMatrix(slicer.util.getNode('变换_3'))
      transform4 = slicer.util.arrayFromTransformMatrix(slicer.util.getNode('变换_4'))
      transform5 = slicer.util.arrayFromTransformMatrix(slicer.util.getNode('变换_5'))
      transform_tmp = slicer.util.arrayFromTransformMatrix(slicer.util.getNode('变换_胫骨'))
      Trans = np.dot(np.dot(np.dot(transform3, transform4), transform_tmp),transform5)
      return Trans

  #推荐胫骨假体
  def SelectTibiaJiaTi(self):
    self.TibiaJieGu()
    PointPath = os.path.join(os.path.dirname(__file__), '假体库/a')
    trans = self.TibiaTrans()
    # lujing = os.path.join(self.jiatiPath, 'Tibia-1-5.stl')
    # self.TibiaJiaTiload = slicer.util.loadModel(lujing)
    # Transform5 = slicer.util.getNode('变换_5')
    # self.TibiaJiaTiload.SetAndObserveTransformNodeID(Transform5.GetID())
    judge1=[]
    index1=[]
    
    list = ['1-5', '2', '2-5', '3', '4', '5']
    for i in range(0, len(list)):
        inputPoints=[]
        name = 'Tibia-' + list[i]
        lujing = os.path.join(PointPath,name+'.txt')
        print('lujing',lujing)
        point  =  np.loadtxt(lujing)
        for j in range(0,11):
            b=np.array([float(point[j][0]),float(point[j][1]),point[j][2],1])
            a=np.dot(trans,b)
            inputPoints.append(a)
        inputModel=slicer.util.getNode('Tibia')
        surface_World = inputModel.GetPolyData()
        distanceFilter = vtk.vtkImplicitPolyDataDistance()
        distanceFilter.SetInput(surface_World)
        nOfFiducialPoints = 11
        distances = np.zeros(nOfFiducialPoints)
        for j in range(nOfFiducialPoints):
            point_World = np.asarray(inputPoints)[j,0:3] 
            closestPointOnSurface_World = np.zeros(3)
            closestPointDistance = distanceFilter.EvaluateFunctionAndGetClosestPoint(point_World, closestPointOnSurface_World)
            distances[j] = closestPointDistance
        a=0
        #if list[i] == list[self.select]:
        if list[i] == list[3]:
            tmp = distances
        print('distances:',distances)
        # for j in range(nOfFiducialPoints):
        #     if distances[j]<0:
        #         a = a + 1

        if distances[0]+distances[4]<0 and  distances[2]+distances[7]<0 and distances[5]+distances[10]<0:
            a=10
        
        if list[i] == '1-5':
            dis = distances
            TransformTmp = slicer.util.getNode('变换_胫骨')
            c = slicer.util.arrayFromTransformMatrix(TransformTmp)
            c[1][3] = c[1][3] - (distances[10] - distances[5]) / 2
            #c[0][3] = c[0][3] + (distances[0] - distances[4]) / 2
            TransformTmp.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(c))
                
        if a==10:
            sum = 0
            for j in range(nOfFiducialPoints):
                sum =sum + distances[j]
            judge1.append(sum)        
    
            index1.append(i)
          

      # if len(judge1)<1:
      #     TransformTmp = slicer.util.getNode('变换_胫骨')
      #     c =slicer.util.arrayFromTransformMatrix(TransformTmp)
      #     c[1][3]=c[1][3]+dis[10]
      #     TransformTmp.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(c))
      #     self.TibiaJtSelectNum=1
      #     self.SelectTibiaJiaTi()
      #     return 1
          

        if len(judge1)>=1:
          max_judge1=judge1.index(max(judge1))
          print('judge',judge1)
          print('index',index1)
          self.TibiaSelect = index1[max_judge1]
          if self.TibiaSelect - self.select >1:
              self.TibiaSelect = self.select+1
          elif self.select - self.TibiaSelect >1:
              self.TibiaSelect = self.TibiaSelect+1
        else:
          self.TibiaSelect = self.select

        Name = 'Tibia-'+list[self.TibiaSelect]
        self.ui.TibiaJiaTi.setCurrentText(Name)
        self.ui.JiaTiNameT.setText(Name)

      #self.AddKongBai()
      #设置表头文字颜色
      # self.ui.TibiaTableWidget.verticalHeaderItem(self.TibiaSelect).setForeground(qt.QBrush(qt.QColor(48,47,45)))
      # #设置表头背景颜色
      # self.ui.TibiaTableWidget.verticalHeaderItem(self.TibiaSelect).setBackground(qt.QColor(124,189,39))
      # self.ui.TibiaTableWidget.horizontalHeaderItem(self.select).setForeground(qt.QBrush(qt.QColor(48,47,45)))
      # self.ui.TibiaTableWidget.horizontalHeaderItem(self.select).setBackground(qt.QColor(124,189,39))
      # self.ui.TibiaTableWidget.item(self.TibiaSelect, self.select).setBackground(qt.QColor(124, 189, 39))
  
  #加载胫骨假体    
  def loadTibiaJiaTi(self, name):
    try:
      slicer.mrmlScene.RemoveNode(self.TibiaJiaTiload)
    except Exception as e:
      print(e) 
    lujing = os.path.join(self.jiatiPath, name + '.stl')
    print('name',name)
    
    self.TibiaJiaTiload = slicer.util.loadModel(lujing)
    self.TibiaJiaTiload.SetName(name)
    #将假体放在Transform约束变换下：
    TtransformYueShu = slicer.util.getNode('变换_约束')
    self.TibiaJiaTiload.SetAndObserveTransformNodeID(TtransformYueShu.GetID())

  #胫骨截骨调整
  def onAdjustment2(self):
    slicer.app.layoutManager().setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutTriple3DEndoscopyView)
    self.ThreeDViewAndImageWidget(2)
    self.loadTibiaJiaTi(self.ui.TibiaJiaTi.currentText)
    try:
      slicer.mrmlScene.RemoveNode(slicer.util.getNode("TibiaLine"))
      slicer.mrmlScene.RemoveNode(slicer.util.getNode("FemurLine"))      
    except:
      pass    
    try:
      self.ui.BoneButton.setChecked(False)
    except:
      pass 
    self.TibiaButtonChecked(self.ui.Adjustment2)

    self.HideAll()
    self.HidePart()
    self.ShowNode('胫骨近端')
    self.ui.PopupWidget.setVisible(True)
    self.ui.head1.setVisible(True)
    self.ui.Graph.setVisible(True)
    self.TibiaJiaTiload.SetDisplayVisibility(True) 
    self.InitHeadWidget()
    #初始化显示/隐藏，显示假体
    self.ui.JiaTiButton.setChecked(True)
    self.interactorStyle2.SetInteractor(None)
    self.interactorStyle3.SetInteractor(None)
    self.ui.PopupWidget.setMinimumHeight(470)
    slicer.modules.tibiapopup.widgetRepresentation().setParent(self.ui.PopupWidget)
    slicer.modules.tibiapopup.widgetRepresentation().setGeometry(0,0,450,500)
    slicer.modules.tibiapopup.widgetRepresentation().show()
    slicer.modules.TibiaPopupWidget.ui.xMoveButton1.click()
    segs = slicer.util.getNodesByClass('vtkMRMLMarkupsFiducialNode')
    Name = []
    for i in range(0, len(segs)):
      a = segs[i]
      Name.append(a.GetName())
    if '胫骨截骨面' in Name:
        pass

    else:
      # 截骨面
      TibiaJieGu = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', '胫骨截骨面')
      TibiaJieGu.AddControlPoint(30, 0, 0)
      TibiaJieGu.AddControlPoint(0, 30, 0)
      TibiaJieGu.AddControlPoint(0, 0, 0)
      Transform5 = slicer.util.getNode('变换_5')
      TibiaJieGu.SetAndObserveTransformNodeID(Transform5.GetID())
      slicer.util.getNode('胫骨截骨面').SetDisplayVisibility(False)

    #计算角度
    try:
      ras1, ras2, ras3, ras4 = [0, 0, 0], [0, 0, 0], [0, 0, 0], [0, 0, 0]
      slicer.util.getNode('胫骨隆凸').GetNthControlPointPositionWorld(0, ras1)
      slicer.util.getNode('内侧高点').GetNthControlPointPositionWorld(0, ras2)
      slicer.util.getNode('外侧高点').GetNthControlPointPositionWorld(0, ras3)
      slicer.util.getNode('踝穴中心').GetNthControlPointPositionWorld(0, ras4)
      #胫骨后倾角
      TibiaJGM =self.GetTransPoint('胫骨截骨面')
      NGTY = self.TouYing(TibiaJGM, ras2)
      WGTY = self.TouYing(TibiaJGM, ras3)
      xl1=np.array([NGTY[0]-WGTY[0], NGTY[1]-WGTY[1], NGTY[2]-WGTY[2]])
      xl2=np.array([ras4[0]-ras1[0], ras4[1]-ras1[1], ras4[2]-ras1[2]])
      houqingjiao =self.Angle(xl1, xl2)
      self.TibiaHouQingJiao = 90-houqingjiao
      # 胫骨外旋角
      xl4 = np.array([ras2[0]-ras3[0], ras2[1]-ras3[1], ras2[2]-ras3[2]])
      self.TibiaWaiXuanJiao = self.Angle(xl1, xl4)
      print('胫骨外旋角：', self.WaiXuanJiao)
    except Exception as e:
      print(e)


    self.SetTibiaCameraTip()
    self.ui.Adjustment.setChecked(0)
    try:
      self.ShowHide()#在View3显示不同的部分
    except:
      pass
    if len(self.ui.GraphImage.children()) < 2:
      self.PyQtGraph()
    else:
      self.ui.GraphImage.show()
    self.ui.ModuleName.setText('胫骨截骨')
  
  #设置胫骨三维视图注释
  def SetTibiaCameraTip(self):
    self.hideInformation()
    # 视图1按钮
    icon1A = qt.QIcon()
    icons1APath = os.path.join(self.iconsPath, '重置.png')
    icon1A.addPixmap(qt.QPixmap(icons1APath))
    self.TV1Button = qt.QPushButton(self.view1)
    self.TV1Button.setGeometry(5, 5, 41, 41)
    self.TV1Button.setIconSize(qt.QSize(41, 41))
    self.TV1Button.setIcon(icon1A)
    self.TV1Button.setFlat(True)
    self.TV1Button.setStyleSheet("QPushButton{border:none;background:transparent;color:rgba(0,0,0,0);}")
    self.TV1Button.connect('clicked(bool)', self.onTV1Button)
    self.TV1Button.setToolTip('锁定')
    self.TV1Button.show()
    # 视图2按钮
    icon2A = qt.QIcon()
    icons2APath = os.path.join(self.iconsPath, '箭头.png')
    icon2A.addPixmap(qt.QPixmap(icons2APath))
    self.TV2Button = qt.QPushButton(self.view2)
    self.TV2Button.setGeometry(5, 5, 41, 41)
    self.TV2Button.setIconSize(qt.QSize(41, 41))
    self.TV2Button.setIcon(icon2A)
    self.TV2Button.setFlat(True)
    self.TV2Button.setStyleSheet("QPushButton{border:none;background:transparent;}")
    self.TV2Button.connect('clicked(bool)', self.onTV2Button)
    self.TV2Button.show()
    # 视图3按钮
    icon3A = qt.QIcon()
    icons3APath = os.path.join(self.iconsPath, '箭头.png')
    icon3A.addPixmap(qt.QPixmap(icons3APath))
    self.TV3Button = qt.QPushButton(self.view3)
    self.TV3Button.setGeometry(5, 5, 41, 41)
    self.TV3Button.setIconSize(qt.QSize(41, 41))
    self.TV3Button.setIcon(icon3A)
    self.TV3Button.setFlat(True)
    self.TV3Button.setStyleSheet("QPushButton{border:none;background:transparent;}")
    self.TV3Button.connect('clicked(bool)', self.onTV3Button)
    self.TV3Button.show()

    # 设置相机
    self.TCamera1(self.view1)
    self.TCamera2(self.view2)
    self.TCamera3(self.view3)

    cameraNode = self.view1.cameraNode()
    cameraNode2 = self.view2.cameraNode()
    if slicer.modules.NoImageWelcomeWidget.judge == 'L':
      if (cameraNode.GetName() == 'TC1'):
          if (cameraNode2.GetName() == 'TC2'):
              self.TCamera1Tip(self.view1)
              self.TCamera2Tip(self.view2)
              self.TCamera3Tip(self.view3)
          else:
              self.TCamera1Tip(self.view1)
              self.TCamera2Tip(self.view3)
              self.TCamera3Tip(self.view2)

      elif (cameraNode.GetName() == 'TC2'):
          if (cameraNode2.GetName() == 'TC1'):
              self.TCamera1Tip(self.view2)
              self.TCamera2Tip(self.view1)
              self.TCamera3Tip(self.view3)
          else:
              self.TCamera1Tip(self.view2)
              self.TCamera2Tip(self.view3)
              self.TCamera3Tip(self.view1)
      else:
          if (cameraNode2.GetName() == 'TC2'):
              self.TCamera1Tip(self.view3)
              self.TCamera2Tip(self.view2)
              self.TCamera3Tip(self.view1)
          else:
              self.TCamera1Tip(self.view3)
              self.TCamera2Tip(self.view1)
              self.TCamera3Tip(self.view2)
    else:
      if (cameraNode.GetName() == 'TC1'):
          if (cameraNode2.GetName() == 'TC2'):
              self.TCamera1TipRight(self.view1)
              self.TCamera2Tip(self.view2)
              self.TCamera3TipRight(self.view3)
          else:
              self.TCamera1TipRight(self.view1)
              self.TCamera2Tip(self.view3)
              self.TCamera3TipRight(self.view2)
      elif (cameraNode.GetName() == 'TC2'):
          if (cameraNode2.GetName() == 'TC1'):
              self.TCamera1TipRight(self.view2)
              self.TCamera2Tip(self.view1)
              self.TCamera3TipRight(self.view3)
          else:
              self.TCamera1TipRight(self.view2)
              self.TCamera2Tip(self.view3)
              self.TCamera3TipRight(self.view1)
      else:
          if (cameraNode2.GetName() == 'TC2'):
              self.TCamera1TipRight(self.view3)
              self.TCamera2Tip(self.view2)
              self.TCamera3TipRight(self.view1)
          else:
              self.TCamera1TipRight(self.view3)
              self.TCamera2Tip(self.view1)
              self.TCamera3TipRight(self.view2)

  #胫骨视图选择
  def onTibiaViewSelect(self):
    self.HideAll()
    self.ui.PopupWidget.setVisible(True)
    self.hideInformation()
    self.ThreeDState()
    self.ui.PopupWidget.setMinimumHeight(650)
    slicer.modules.tibiaviewselect.widgetRepresentation().setParent(self.ui.PopupWidget)
    slicer.modules.tibiaviewselect.widgetRepresentation().setGeometry(-10,-10,624,624)
    slicer.modules.tibiaviewselect.widgetRepresentation().show()
    self.TibiaButtonChecked(self.ui.ViewChoose2)

  #胫骨重置
  def onReset2(self):
    message = qt.QMessageBox(qt.QMessageBox.Information,'重置',"是否要重置胫骨的方案？",qt.QMessageBox.Ok|qt.QMessageBox.Cancel)
    message.button(qt.QMessageBox().Ok).setText('是')
    message.button(qt.QMessageBox().Cancel).setText('否')
    c= message.exec()
    if c == qt.QMessageBox.Ok:
      self.TibiaButtonChecked(self.ui.ReSet2)
      slicer.app.setOverrideCursor(qt.Qt.WaitCursor)  # 光标变成圆圈
      self.DeleteNode('变换_3')
      self.DeleteNode('变换_4')
      self.DeleteNode('变换_5')
      self.DeleteNode('变换_胫骨')
      self.DeleteNode('变换_约束')
      self.DeleteNode(self.TibiaJiaTiload.GetName())
      self.DeleteNode('胫骨近端切割')
      self.DeleteNode('胫骨截骨面')
      self.DeleteNode('胫骨近端')
      self.DeleteNode('DynamicModeler_1')
      self.DeleteNode('胫骨切割')
      self.DeleteNode('切割6')
      self.DeleteNode('部件6')
      self.DeleteNode('动态切割6')
      slicer.app.restoreOverrideCursor()  # 变回光标原来的形状
      #显示点
      self.ShowNode('外侧高点')
      self.ShowNode('内侧高点')
      self.ShowNode('胫骨隆凸')
      self.ShowNode('胫骨结节')
      self.ShowNode('踝穴中心')
      #显示胫骨并将胫骨透明化
      Tibia = slicer.util.getNode('Tibia')
      Tibia.SetDisplayVisibility(1)
      Tibia.GetDisplayNode().SetOpacity(0.2)
      # 回到解剖标志
      self.ui.Parameter2.click()
    elif c == qt.QMessageBox.Canel:
      self.ui.ReSet2.setChecked(False)
  #--------------head-显示/隐藏---------------------------------------------
  # 截骨调整-截骨面
  def onBoneButton(self):
    if self.currentModel == 4:
      Bone = slicer.util.getNode('股骨切割')
      FemurYD = slicer.util.getNode('股骨远端')
      self.TibiaShowHide2()
      if self.ui.BoneButton.isChecked():
        self.ui.TransparentButton.setEnabled(False)
        Bone.SetDisplayVisibility(1)
        FemurYD.SetDisplayVisibility(0)
      else:
        self.ui.TransparentButton.setEnabled(True)
        Bone.SetDisplayVisibility(0)
        FemurYD.SetDisplayVisibility(1)

    elif self.currentModel == 5:
      Bone = slicer.util.getNode('胫骨切割')
      TibiaJD = slicer.util.getNode('胫骨近端')
      self.FemurShowHide2()
      if self.ui.BoneButton.isChecked():
        self.ui.TransparentButton.setEnabled(False)
        Bone.SetDisplayVisibility(1)
        TibiaJD.SetDisplayVisibility(0)
      else:
        self.ui.TransparentButton.setEnabled(True)
        Bone.SetDisplayVisibility(0)
        TibiaJD.SetDisplayVisibility(1)
  # 截骨调整-假体
  def onJiaTiButton(self):
    if self.currentModel == 4:
      self.TibiaJiaTiShowHide()
      if self.jiatiload.GetDisplayVisibility() == 1:
          self.jiatiload.SetDisplayVisibility(0)
      else:
          self.jiatiload.SetDisplayVisibility(1)
    elif self.currentModel == 5:
      self.FemurJiaTiShowHide()
      if self.TibiaJiaTiload.GetDisplayVisibility() == 1:
          self.TibiaJiaTiload.SetDisplayVisibility(0)
      else:
          self.TibiaJiaTiload.SetDisplayVisibility(1)
  # 截骨调整-标记点
  def onMarkerButton(self):
    if self.currentModel == 4:
      self.TibiaMarkShowHide()
      if self.ui.MarkerButton.isChecked():
        self.ShowNode('A点')
        self.ShowNode('内侧后髁')
        self.ShowNode('外侧后髁')
        self.ShowNode('外侧远端')
        self.ShowNode('内侧远端')
        self.ShowNode('外侧凸点')
        self.ShowNode('内侧凹点')
        self.ShowNode('开髓点')
        self.ShowNode('外侧皮质高点')

      else:
        self.HideNode('A点')
        self.HideNode('开髓点')
        self.HideNode('内侧后髁')
        self.HideNode('外侧后髁')
        self.HideNode('外侧远端')
        self.HideNode('内侧远端')
        self.HideNode('外侧凸点')
        self.HideNode('内侧凹点')
        self.HideNode('外侧皮质高点')

    elif self.currentModel == 5:
      self.FmeurMarkShowHide()
      if self.ui.MarkerButton.isChecked():
        self.ShowNode('外侧高点')
        self.ShowNode('内侧高点')
        self.ShowNode('胫骨隆凸')
        self.ShowNode('胫骨结节')
      else:
        self.HideNode('外侧高点')
        self.HideNode('内侧高点')
        self.HideNode('胫骨隆凸')
        self.HideNode('胫骨结节')
  # 截骨调整-透明显示
  def onTransparentButton(self):
    if self.currentModel == 4:
      self.TibiaTransparentShowHide()
      Femur = slicer.util.getNode('股骨远端')
      Femur.SetDisplayVisibility(1)
      if self.ui.TransparentButton.isChecked():
          self.ui.BoneButton.setEnabled(False)
          Femur.GetDisplayNode().SetOpacity(0.2)
      else:
          self.ui.BoneButton.setEnabled(True)
          Femur.GetDisplayNode().SetOpacity(1.0)


    elif self.currentModel == 5:
      self.FemurTransparentShowHide()
      Tibia = slicer.util.getNode('胫骨近端')
      Tibia.SetDisplayVisibility(1)
      if self.ui.TransparentButton.isChecked():
          self.ui.BoneButton.setEnabled(False)
          Tibia.GetDisplayNode().SetOpacity(0.2)
      else:
          self.ui.BoneButton.setEnabled(True)
          Tibia.GetDisplayNode().SetOpacity(1.0)
  
  def InitHeadWidget(self):
    self.ui.FemurSwitch.setVisible(False)
    self.ui.TibiaSwitch.setVisible(False)
    self.ui.FemurR.setVisible(False)
    self.ui.FemurL.setVisible(False)
    self.ui.TibiaJiaTi.setVisible(False)
    self.ui.FemurShowHide.setVisible(False)
    self.ui.TibiaShowHide.setVisible(False)
    if self.currentModel == 4:
      self.ui.TibiaSwitch.setVisible(True)
      self.ui.TibiaShowHide.setVisible(True)
      self.ui.TibiaJiaTi.setVisible(True)
      if slicer.modules.NoImageWelcomeWidget.judge == 'L':
        self.ui.FemurL.setVisible(True)
      else:
        self.ui.FemurR.setVisible(True)
    else:
      self.ui.FemurSwitch.setVisible(True)
      self.ui.TibiaJiaTi.setVisible(True)
      self.ui.FemurShowHide.setVisible(True)
      if slicer.modules.NoImageWelcomeWidget.judge == 'L':
        self.ui.FemurL.setVisible(True)
      else:
        self.ui.FemurR.setVisible(True)

  def onFemurR(self,index):
    print(index)
    print(self.ui.FemurR.currentText)
    name = self.ui.FemurR.currentText
    self.loadJiaTi(name)
    if self.ui.BoneButton.checked:
      slicer.modules.PopupWidget.ui.Confirm.click()
      self.ui.BoneButton.click()
      self.ui.BoneButton.click()
    self.ShowHide()

  def onFemurL(self,index):
    print(index)
    print(self.ui.FemurL.currentText) 
    name = self.ui.FemurL.currentText
    self.loadJiaTi(name)
    if self.ui.BoneButton.checked:
      slicer.modules.PopupWidget.ui.Confirm.click()
      self.ui.BoneButton.click()
      self.ui.BoneButton.click()
    self.ShowHide()
  
  def onTibiaJiaTi(self,index):
    print(index)
    print(self.ui.TibiaJiaTi.currentText)
    name = self.ui.TibiaJiaTi.currentText    
    self.loadTibiaJiaTi(name)
    if self.ui.BoneButton.checked:
      slicer.modules.TibiaPopupWidget.ui.xMoveButton1.click()
      self.ui.BoneButton.click()
      self.ui.BoneButton.click()
    self.ShowHide()

  def onFemurSwitch(self):
    self.currentModel = 4
    self.WidgetShow(self.currentModel)
    self.ui.Adjustment.click()
  
  def onTibiaSwitch(self):
    self.currentModel = 5
    self.WidgetShow(self.currentModel)
    self.ui.Adjustment2.click()
    
  def onFemurShowHide(self):
    self.FemurShowHide2()
    self.FemurJiaTiShowHide()
    self.FmeurMarkShowHide()
    self.FemurTransparentShowHide()
  
  def onTibiaShowHide(self):
    self.TibiaShowHide2()
    self.TibiaJiaTiShowHide()
    self.TibiaMarkShowHide()
    self.TibiaTransparentShowHide()

  
  def TibiaShowHide2(self):
    if self.ui.TibiaShowHide.checked:
      if self.ui.BoneButton.checked:
        self.ShowNode('胫骨切割')
        self.HideNode('胫骨近端')
      else:
        self.ShowNode('胫骨近端')
        self.HideNode('胫骨切割')
    else:
        self.HideNode('胫骨近端')
        self.HideNode('胫骨切割')
  
  def FemurShowHide2(self):
    if self.ui.FemurShowHide.checked:
      if self.ui.BoneButton.checked:
        self.ShowNode('股骨切割')
        self.HideNode('股骨远端')
      else:
        self.HideNode('股骨切割')
        self.ShowNode('股骨远端')
    else:
        self.HideNode('股骨切割')
        self.HideNode('股骨远端')

  def FemurJiaTiShowHide(self):
    if self.ui.FemurShowHide.checked:
      if self.ui.JiaTiButton.checked:
        self.jiatiload.SetDisplayVisibility(1)
      else:
        self.jiatiload.SetDisplayVisibility(0)
    else:
      self.jiatiload.SetDisplayVisibility(0)
  def TibiaJiaTiShowHide(self):
    if self.ui.TibiaShowHide.checked:
      if self.ui.JiaTiButton.checked:
        self.TibiaJiaTiload.SetDisplayVisibility(1)
      else:
        self.TibiaJiaTiload.SetDisplayVisibility(0)
    else:
      self.TibiaJiaTiload.SetDisplayVisibility(0)
  def FmeurMarkShowHide(self):
    if self.ui.FemurShowHide.checked:
      if self.ui.MarkerButton.checked:
        self.ShowNode('A点')
        self.ShowNode('内侧后髁')
        self.ShowNode('外侧后髁')
        self.ShowNode('外侧远端')
        self.ShowNode('内侧远端')
        self.ShowNode('外侧凸点')
        self.ShowNode('内侧凹点')
        self.ShowNode('开髓点')
        self.ShowNode('外侧皮质高点')
      else:
        self.HideNode('A点')
        self.HideNode('开髓点')
        self.HideNode('内侧后髁')
        self.HideNode('外侧后髁')
        self.HideNode('外侧远端')
        self.HideNode('内侧远端')
        self.HideNode('外侧凸点')
        self.HideNode('内侧凹点')
        self.HideNode('外侧皮质高点')
    else:
      self.HideNode('A点')
      self.HideNode('开髓点')
      self.HideNode('内侧后髁')
      self.HideNode('外侧后髁')
      self.HideNode('外侧远端')
      self.HideNode('内侧远端')
      self.HideNode('外侧凸点')
      self.HideNode('内侧凹点')
      self.HideNode('外侧皮质高点')
  def TibiaMarkShowHide(self):
    if self.ui.TibiaShowHide.checked:
      if self.ui.MarkerButton.checked:
        self.ShowNode('外侧高点')
        self.ShowNode('内侧高点')
        self.ShowNode('胫骨隆凸')
        self.ShowNode('胫骨结节')
      else:
        self.HideNode('外侧高点')
        self.HideNode('内侧高点')
        self.HideNode('胫骨隆凸')
        self.HideNode('胫骨结节')
    else:
      self.HideNode('外侧高点')
      self.HideNode('内侧高点')
      self.HideNode('胫骨隆凸')
      self.HideNode('胫骨结节')
  def FemurTransparentShowHide(self):
    Femur = slicer.util.getNode('股骨远端')
    Femur.SetDisplayVisibility(1)
    if self.ui.FemurShowHide.checked:
      if self.ui.TransparentButton.checked:
          Femur.GetDisplayNode().SetOpacity(0.2)
      else:
        Femur.GetDisplayNode().SetOpacity(1)
    else:
      Femur.SetDisplayVisibility(0)
  def TibiaTransparentShowHide(self):
    Tibia = slicer.util.getNode('胫骨近端')
    Tibia.SetDisplayVisibility(1)
    if self.ui.TibiaShowHide.checked:
      if self.ui.TransparentButton.checked:
          Tibia.GetDisplayNode().SetOpacity(0.2)
      else:
        Tibia.GetDisplayNode().SetOpacity(1)
    else:
      Tibia.SetDisplayVisibility(0)
  
  #设置模型在第三个三维视图中是否显示 0-不显示 1-显示
  def ThreeDViewShowHide(self,name,index):
    displayNode = slicer.util.getNode(name).GetDisplayNode()
    threeDViewIDs = [node.GetID() for node in slicer.util.getNodesByClass('vtkMRMLViewNode')]
    if index == 0:
      displayNode.SetViewNodeIDs(['vtkMRMLViewNode1','vtkMRMLViewNode2'])
    else:
      displayNode.SetViewNodeIDs(threeDViewIDs)
  
  def ShowHide(self):
    if self.ui.Adjustment.checked:
      self.ThreeDViewShowHide('胫骨切割',0)
      self.ThreeDViewShowHide('胫骨近端',0)
      self.ThreeDViewShowHide(self.TibiaJiaTiload.GetName(),0)
      self.ThreeDViewShowHide('股骨切割',1)
      self.ThreeDViewShowHide('股骨远端',1)
      self.ThreeDViewShowHide(self.jiatiload.GetName(),1)
    elif self.ui.Adjustment2.checked:
      self.ThreeDViewShowHide('胫骨切割',1)
      self.ThreeDViewShowHide('胫骨近端',1)
      self.ThreeDViewShowHide(self.TibiaJiaTiload.GetName(),1)
      self.ThreeDViewShowHide(self.jiatiload.GetName(),0)
      self.ThreeDViewShowHide('股骨切割',0)
      self.ThreeDViewShowHide('股骨远端',0)
    elif self.currentModel == 6:
      displayNode = slicer.util.getNode('胫骨近端').GetDisplayNode()
      displayNode.SetViewNodeIDs(['vtkMRMLViewNode1'])
      displayNode = slicer.util.getNode('股骨远端').GetDisplayNode()
      displayNode.SetViewNodeIDs(['vtkMRMLViewNode1'])
      displayNode = slicer.util.getNode('股骨切割').GetDisplayNode()
      displayNode.SetViewNodeIDs(['vtkMRMLViewNode2','vtkMRMLViewNode3'])
      displayNode = slicer.util.getNode('胫骨切割').GetDisplayNode()
      displayNode.SetViewNodeIDs(['vtkMRMLViewNode2','vtkMRMLViewNode4'])
      self.jiatiload.GetDisplayNode().SetViewNodeIDs(['vtkMRMLViewNode2','vtkMRMLViewNode3'])
      self.TibiaJiaTiload.GetDisplayNode().SetViewNodeIDs(['vtkMRMLViewNode2','vtkMRMLViewNode4'])
      self.ChenDian.GetDisplayNode().SetViewNodeIDs(['vtkMRMLViewNode2'])
    else:
      self.ThreeDViewShowHide('胫骨切割',1)
      self.ThreeDViewShowHide('胫骨近端',1)
      self.ThreeDViewShowHide('股骨切割',1)
      self.ThreeDViewShowHide('股骨远端',1)
      self.ThreeDViewShowHide(self.jiatiload.GetName(),1)
      self.ThreeDViewShowHide(self.TibiaJiaTiload.GetName(),1)

  #--------------------------报告-----------------------------------
  #加载衬垫
  def loadChenDian(self):
    if self.TibiaJiaTiload.GetName() =='Tibia-1-5':
      try:
          slicer.mrmlScene.RemoveNode(self.ChenDian)

      except Exception as e:
          print(e)
      lujing = os.path.join(self.jiatiPath, 'Insert-1-5.stl')
      self.ChenDian = slicer.util.loadModel(lujing)
      self.ChenDian.SetName('Insert-1-5')
    elif self.TibiaJiaTiload.GetName() =='Tibia-2':
      try:
        slicer.mrmlScene.RemoveNode(self.ChenDian)

      except Exception as e:
          print(e)
      lujing = os.path.join(self.jiatiPath, 'Insert-2.stl')
      self.ChenDian = slicer.util.loadModel(lujing)
      self.ChenDian.SetName('Insert-2')
    elif self.TibiaJiaTiload.GetName() =='Tibia-2-5':
      try:
          slicer.mrmlScene.RemoveNode(self.ChenDian)

      except Exception as e:
          print(e)
      lujing = os.path.join(self.jiatiPath, 'Insert-2-5.stl')
      self.ChenDian = slicer.util.loadModel(lujing)
      self.ChenDian.SetName('Insert-2-5')
    elif self.TibiaJiaTiload.GetName() =='Tibia-3':
      try:
          slicer.mrmlScene.RemoveNode(self.ChenDian)

      except Exception as e:
          print(e)
      lujing = os.path.join(self.jiatiPath, 'Insert-3.stl')
      self.ChenDian = slicer.util.loadModel(lujing)
      self.ChenDian.SetName('Insert-3')
    elif self.TibiaJiaTiload.GetName() =='Tibia-4':
      try:
          slicer.mrmlScene.RemoveNode(self.ChenDian)

      except Exception as e:
          print(e)
      lujing = os.path.join(self.jiatiPath, 'Insert-4.stl')
      self.ChenDian = slicer.util.loadModel(lujing)
      self.ChenDian.SetName('Insert-4')
    elif self.TibiaJiaTiload.GetName() =='Tibia-3':
      try:
          slicer.mrmlScene.RemoveNode(self.ChenDian)

      except Exception as e:
          print(e)
      lujing = os.path.join(self.jiatiPath, 'Insert-5.stl')
      self.ChenDian = slicer.util.loadModel(lujing)
      self.ChenDian.SetName('Insert-5')
    TtransformYueShu=slicer.util.getNode('变换_约束')
    self.ChenDian.SetAndObserveTransformNodeID(TtransformYueShu.GetID())
  #调直
  def onJieTu(self):
    self.ReportButtonChecked(self.ui.JieTu)
    import time
    slicer.app.setOverrideCursor(qt.Qt.WaitCursor)
    self.HidePart()
    shNode = slicer.vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNode(slicer.mrmlScene)
    slicer.util.getNode('胫骨近端').SetAndObserveTransformNodeID(shNode.GetID())
    self.ShowNode('股骨远端')
    self.ShowNode('胫骨近端')
    time.sleep(1)
    renderWindow = slicer.app.layoutManager().threeDWidget('View1').threeDView().renderWindow()
    renderWindow.SetAlphaBitPlanes(1)
    wti = vtk.vtkWindowToImageFilter()
    wti.SetInputBufferTypeToRGBA()
    wti.SetInput(renderWindow)
    writer = vtk.vtkPNGWriter()
    path = slicer.modules.report.path
    lujing1 = path[0:-9] + 'Resources/Icons/手术报告-3.png'
    writer.SetFileName(lujing1)
    writer.SetInputConnection(wti.GetOutputPort())
    writer.Write()
    print('手术报告-3已截图')
    self.HidePart()
    self.ShowNode('股骨切割')
    self.ShowNode('胫骨切割')
    self.jiatiload.SetDisplayVisibility(True)
    self.TibiaJiaTiload.SetDisplayVisibility(True)
    self.ChenDian.SetDisplayVisibility(True)
    #self.ShuZhi()
    time.sleep(1)
    renderWindow = slicer.app.layoutManager().threeDWidget('View1').threeDView().renderWindow()
    renderWindow.SetAlphaBitPlanes(1)
    wti = vtk.vtkWindowToImageFilter()
    wti.SetInputBufferTypeToRGBA()
    wti.SetInput(renderWindow)
    writer = vtk.vtkPNGWriter()
    path = slicer.modules.report.path
    lujing2 = path[0:-9] + 'Resources/Icons/手术报告-4.png'
    writer.SetFileName(lujing2)
    writer.SetInputConnection(wti.GetOutputPort())
    writer.Write()
    print('手术报告-4已截图')

    self.Camera3(self.view3)
    self.ShowNode('股骨切割')
    self.jiatiload.SetDisplayVisibility(True)
    self.ChenDian.SetDisplayVisibility(False)
    self.HideNode('胫骨切割')
    self.TibiaJiaTiload.SetDisplayVisibility(False)
    time.sleep(1)

    renderWindow = slicer.app.layoutManager().threeDWidget('View3').threeDView().renderWindow()
    renderWindow.SetAlphaBitPlanes(1)
    wti = vtk.vtkWindowToImageFilter()
    wti.SetInputBufferTypeToRGBA()
    wti.SetInput(renderWindow)
    writer = vtk.vtkPNGWriter()
    path = slicer.modules.report.path
    lujing2 = path[0:-9] + 'Resources/Icons/手术报告-1.png'
    writer.SetFileName(lujing2)
    writer.SetInputConnection(wti.GetOutputPort())
    writer.Write()
    print('手术报告-1已截图')

    view4 = slicer.app.layoutManager().threeDWidget('View4').threeDView()
    self.TCamera3(view4)
    self.HideNode('股骨切割')
    self.jiatiload.SetDisplayVisibility(False)
    self.ChenDian.SetDisplayVisibility(False)
    self.ShowNode('胫骨切割')
    self.TibiaJiaTiload.SetDisplayVisibility(True)
    time.sleep(1)

    renderWindow = slicer.app.layoutManager().threeDWidget('View4').threeDView().renderWindow()
    renderWindow.SetAlphaBitPlanes(1)
    wti = vtk.vtkWindowToImageFilter()
    wti.SetInputBufferTypeToRGBA()
    wti.SetInput(renderWindow)
    writer = vtk.vtkPNGWriter()
    path = slicer.modules.report.path
    lujing2 = path[0:-9] + 'Resources/Icons/手术报告-2.png'
    writer.SetFileName(lujing2)
    writer.SetInputConnection(wti.GetOutputPort())
    writer.Write()
    print('手术报告-2已截图')

    print('截图完成')
    self.ShowNode('股骨切割')
    self.jiatiload.SetDisplayVisibility(True)
    self.ShowNode('胫骨切割')
    self.TibiaJiaTiload.SetDisplayVisibility(True)
    self.ChenDian.SetDisplayVisibility(True)
    slicer.app.restoreOverrideCursor()
      
  #在组合中将股骨和胫骨调整为竖直状态
  def ShuZhi(self):
    #股骨变换
    try:
      slicer.util.getNode('变换_6')
    except:
      transformNode = slicer.util.getNode('变换')
      trans = slicer.util.arrayFromTransformMatrix(transformNode)
      transformNode1 = slicer.util.getNode('变换_1')
      trans1 = slicer.util.arrayFromTransformMatrix(transformNode1)
      transformNodeTmp = slicer.util.getNode('变换_临时')
      transTmp = slicer.util.arrayFromTransformMatrix(transformNodeTmp)
      transformNode2 = slicer.util.getNode('变换_2')
      trans2 = slicer.util.arrayFromTransformMatrix(transformNode2)
      transformNodeR = slicer.util.getNode('变换_R')
      transR = slicer.util.arrayFromTransformMatrix(transformNodeR)
      #胫骨变换
      transformNode3 = slicer.util.getNode('变换_3')
      trans3 = slicer.util.arrayFromTransformMatrix(transformNode3)
      transformNode4 = slicer.util.getNode('变换_4')
      trans4 = slicer.util.arrayFromTransformMatrix(transformNode4)
      transformNodeTib = slicer.util.getNode('变换_胫骨')
      transTib = slicer.util.arrayFromTransformMatrix(transformNodeTib)
      transformNode5 = slicer.util.getNode('变换_5')
      trans5 = slicer.util.arrayFromTransformMatrix(transformNode5)
      transformNodeYueShu = slicer.util.getNode('变换_约束')
      transYueShu = slicer.util.arrayFromTransformMatrix(transformNodeYueShu)

      #股骨变换相乘
      FemurTrans = np.dot(np.dot(np.dot(np.dot(trans,trans1),transTmp),trans2),transR)
      #胫骨变换相乘
      TibiaTrans = np.dot(np.dot(np.dot(np.dot(trans3,trans4),transTib),trans5),transYueShu)
      # 胫骨变换矩阵求逆
      TibiaTrans_ni=np.linalg.inv(TibiaTrans)
      trans_1 = np.array([[1, 0, 0, 0],
                            [0, 1, 0, -10],
                            [0, 0, 1, -18],
                            [0, 0, 0, 1]])
      TibiaTrans_ni=np.dot(trans_1,TibiaTrans_ni)
      #FemurTrans[2][3]=FemurTrans[2][3]-18
      # FemurTrans[1][3]=FemurTrans[1][3]+10

      xzjz=np.dot(FemurTrans,TibiaTrans_ni)
      Ttransform6 = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", '变换_6')
      Ttransform6.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(xzjz))
      #将变换6放到胫骨变换的顶层，即变换3的上一层
      transformNode3.SetAndObserveTransformNodeID(Ttransform6.GetID())
      slicer.util.getNode('胫骨切割').SetAndObserveTransformNodeID(Ttransform6.GetID())
      slicer.util.getNode('胫骨近端').SetAndObserveTransformNodeID(Ttransform6.GetID())

  #CT
  def onCTReport(self):
    if self.ui.CTReport.checked:
      self.HideAll()
      self.ReportButtonChecked(self.ui.CTReport)
      self.ui.OperationPlanWidget.setVisible(True)
      self.ui.ReportWidget.setVisible(True)
      self.ThreeDViewAndImageWidget(0)
      for i in range (0,len(self.noimageWidget.findChildren('QLabel'))):
        self.noimageWidget.findChildren('QLabel')[-1].delete()

      slicer.modules.report.widgetRepresentation().setParent(self.noimageWidget)
      # 将手术报告UI设置为与mywidget同宽高
      slicer.modules.report.widgetRepresentation().resize(self.noimageWidget.width, self.noimageWidget.height)
      slicer.modules.report.widgetRepresentation().show()
      Logo1 = slicer.modules.ReportWidget.ui.Logo
      pixmap = qt.QPixmap(self.iconsPath+'/Logo.png')
      Logo1.setPixmap(pixmap)
      try:
      #将姓名添加到姓名编辑处
        self.ui.NameEdit.setText(slicer.modules.NoImageWelcomeWidget.ui.NameEdit.text)
      except Exception as e:
        print(e)
    else:
      slicer.modules.report.widgetRepresentation().setParent(None)
      self.ui.ReportWidget.setVisible(False)
      self.ThreeDViewAndImageWidget(2)
      slicer.app.layoutManager().setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutFourOverFourView)

  #MRI
  def onMRIReport(self):
    self.HideAll()
    self.ReportButtonChecked(self.ui.MRIReport)
    slicer.modules.report.widgetRepresentation().hide()
  
  #获取路径
  def onPath(self):
    filePath = qt.QFileDialog.getExistingDirectory(self.ui.NoImage, '保存手术报告路径', 'D:/Data')
    self.ui.path.setText(filePath)
  
  #对手术报告截图保存+保存工程文件
  def onConfirmReport(self):
    # 保存截图
    self.ui.SaveTip.setText('请稍候......')
    from PIL import Image
    import os
    import os.path
    img = qt.QPixmap.grabWidget(slicer.modules.report.widgetRepresentation()).toImage()
    luJing = self.ui.path.text + '/' + self.ui.NameEdit.text + '.png'
    img.save(luJing)
    im = Image.open(luJing)
    out = im.transpose(Image.ROTATE_90)
    out.save(luJing)
    slicer.util.pip_install('reportlab==3.6.5')
    pdf_path = self.ui.path.text + '/' + self.ui.NameEdit.text + '.pdf'
    self.PNG_PDF(luJing, pdf_path)
    os.remove(luJing)

    if self.ui.checkBox.isChecked():
      #保存工程文件
      import zipfile
      import os
      #UID = slicer.modules.Case_mainWidget.UID
      UID = ""
      temppath = self.FilePath+'/'+str(UID)+'手术规划' + '/kneetmppath'
      # 新建目录
      try:
          os.makedirs(temppath)
      except Exception as e:
          print(e)
      #股骨变换
      transformNode = slicer.util.getNode('变换')
      trans = slicer.util.arrayFromTransformMatrix(transformNode)
      transformNode1 = slicer.util.getNode('变换_1')
      trans1 = slicer.util.arrayFromTransformMatrix(transformNode1)
      transformNodeTmp = slicer.util.getNode('变换_临时')
      transTmp = slicer.util.arrayFromTransformMatrix(transformNodeTmp)
      transformNode2 = slicer.util.getNode('变换_2')
      trans2 = slicer.util.arrayFromTransformMatrix(transformNode2)
      transformNodeR = slicer.util.getNode('变换_R')
      transR = slicer.util.arrayFromTransformMatrix(transformNodeR)
      #胫骨变换
      transformNode6 = slicer.util.getNode('变换_6')
      trans6 = slicer.util.arrayFromTransformMatrix(transformNode6)
      transformNode3 = slicer.util.getNode('变换_3')
      trans3 = slicer.util.arrayFromTransformMatrix(transformNode3)
      transformNode4 = slicer.util.getNode('变换_4')
      trans4 = slicer.util.arrayFromTransformMatrix(transformNode4)
      transformNodeTib = slicer.util.getNode('变换_胫骨')
      transTib = slicer.util.arrayFromTransformMatrix(transformNodeTib)
      transformNode5 = slicer.util.getNode('变换_5')
      trans5 = slicer.util.arrayFromTransformMatrix(transformNode5)
      transformNodeYueShu = slicer.util.getNode('变换_约束')
      transYueShu = slicer.util.arrayFromTransformMatrix(transformNodeYueShu)

      tr = np.vstack((trans, trans1, transTmp, trans2, transR, trans6, trans3, trans4, transTib, trans5, transYueShu))
      points = np.empty([17, 3])
      Femur = self.jiatiload.GetName()
      Tibia = self.TibiaJiaTiload.GetName()
      dian = ["股骨头球心", "开髓点", '外侧凸点', '内侧凹点', '胫骨隆凸', '胫骨结节', '踝穴中心', 'H点', 'A点', '内侧后髁', '外侧后髁', '内侧远端', '外侧远端', '内侧高点', '外侧高点', '内侧凸点','外侧皮质高点']

      with open(temppath + '/Femur.txt', 'w') as f:
        np.savetxt(f, tr, fmt='%6f')
        for j in range(17):
            slicer.util.getNode(dian[j]).GetNthFiducialPosition(0, points[j])
        np.savetxt(f, points, fmt='%6f', delimiter=' ')
        f.write(Femur + "\n")
        f.write(Tibia + "\n")
        f.write("外侧远端" + str(slicer.modules.PopupWidget.FemurWaiCeYuanDuan)+ "\n")
        f.write("内侧远端" + str(slicer.modules.PopupWidget.FemurNeiCeYuanDuan) + "\n")
        f.write("外侧后髁" + str(slicer.modules.PopupWidget.FemurWaiCeHouKe) + "\n")
        f.write("内侧后髁" + str(slicer.modules.PopupWidget.FemurNeiCeHouKe) + "\n")
        f.write("外侧平台" + str(slicer.modules.TibiaPopupWidget.TibiaWaiCeJieGu) + "\n")
        f.write("内侧平台" + str(slicer.modules.TibiaPopupWidget.TibiaNeiCeJieGu) + "\n")

      f.close()
      models = ['Femur', 'Tibia', '股骨远端', '胫骨近端', '部件1', '部件2', '部件3', '部件4', '部件5', '部件6', '股骨切割', '胫骨切割']
      for i in range(0, 12):
          saveVolumeNode = slicer.util.getNode(models[i])
          myStorageNode = saveVolumeNode.CreateDefaultStorageNode()
          myStorageNode.SetFileName(temppath + '/' + models[i] + '.stl')
          myStorageNode.WriteData(saveVolumeNode)

      # 生成压缩文件
      ysPath=self.FilePath +'/'+str(UID)+'手术规划'+'.ttkx'
      z = zipfile.ZipFile(ysPath, 'w', zipfile.ZIP_DEFLATED)
      for dirpath, dirnames, filenames in os.walk(temppath):
          fpath = dirpath.replace(temppath, '')  # 不replace的话，就从根目录开始复制
          fpath = fpath and fpath + os.sep or ''
          for filename in filenames:
              z.write(os.path.join(dirpath, filename), fpath + filename)

      #删除文件夹
      import shutil
      shutil.rmtree(temppath)
      #复制一份文件到指定目录下
      #shutil.copy2(ysPath, slicer.modules.recentfile.widgetRepresentation().self().FilePath)
    self.ui.SaveTip.setText('保存已完成')

    #png->pdf
    def PNG_PDF(self,png,pdf_path):
        #安装reportlab之后可以使用
        from reportlab.lib.pagesizes import portrait
        from reportlab.pdfgen import canvas
        from PIL import Image
        (w, h) = Image.open(png).size
        user = canvas.Canvas(pdf_path, pagesize=portrait((w, h)))
        user.drawImage(png, 0, 0, w, h)
        user.showPage()
        user.save()
  #----------------------导航-------------------------------------------
  def onNavigationSwitch(self):
    #图片widget
    slicer.app.layoutManager().setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutOneUp3DView)
    self.ThreeDViewAndImageWidget(self.SwitchState)
    if self.SwitchState == 0:
      self.SwitchState = 1
    elif self.SwitchState == 1:
      self.SwitchState = 2
    elif self.SwitchState == 2:            
      self.SwitchState = 0
  
  def DriveJZVisible(self):
    self.ui.MarkerWidget.setVisible(False)
    self.ui.AutoJiaoZhunWidget.setVisible(False)
    self.ui.FYAngleJiaoZhunWidget.setVisible(False)
  
  def DriveButtonChecked(self,button):
    self.ui.Marker.setChecked(False)
    self.ui.AutoJiaoZhun.setChecked(False)
    self.ui.FYAngleJiaoZhun.setChecked(False)
    button.setChecked(True)

  def DHButton(self):
    self.buttonMask("DH1",self.ui.Marker)
    self.buttonMask("DH2",self.ui.AutoJiaoZhun)
    self.buttonMask("DH3",self.ui.FYAngleJiaoZhun)

    self.ui.DHBackground.setPixmap(qt.QPixmap(os.path.join(self.iconsPath,'DHBackground.png')))
    self.ui.DHBackground.setScaledContents(True)

  # 电机校准
  def onDriveJZ(self):
    self.NavigationButtonChecked(self.ui.DriveJZ)
    self.HideAll()
    self.ui.NavigationWidget.setVisible(True)
    self.ui.DriveJZWidget.setVisible(True)
    self.DriveJZVisible()

  def onMarker(self):
    if self.ui.Marker.checked:
      self.DriveButtonChecked(self.ui.Marker)
      self.DriveJZVisible()
      self.ui.MarkerWidget.setVisible(True)
    else:
      self.DriveJZVisible()

  def onAutoJiaoZhun(self):
    if self.ui.AutoJiaoZhun.checked:
      self.DriveButtonChecked(self.ui.AutoJiaoZhun)
      self.DriveJZVisible()
      self.ui.AutoJiaoZhunWidget.setVisible(True)
    else:
      self.DriveJZVisible()
    self.CurrentTimes = [0, 0]
    self.guihua = np.array([[100, 100], [-100, -100], [100, 100], [-100, -100], [100, 100]])
    self.jiaozhun = np.array([[0, 90], [90, 0], [0, 0]])

  # def onstartDJ(self):
  #   self.ser1 = serial.Serial(  # 下面这些参数根据情况修改
  #     port='COM5',
  #     baudrate=115200
  #   )
  #   print("串口状态:" + str(self.ser1.is_open))
  # 
  #   self._received_thread_ = threading.Thread(target=self.__recv_func__, args=(self,))
  #   # print("thread")
  #   self._is_running_ = True
  #   # print("thread1")
  #   self._received_thread_.setDaemon(True)
  #   # print("thread2")
  #   self._received_thread_.setName("SerialPortRecvThread")
  #   self._received_thread_.start()
  #   print('开始监听信号')
  # 
  # def __recv_func__(self, ser1=None):
  #   while self.ser1.isOpen():
  #     try:
  #       print(self.ser1.in_waiting)
  #       if (self.ser1.in_waiting > 0):
  #         buffer = self.ser1.readline().decode('utf-8')
  #         buf_list = buffer.split(',')
  #         print('接收到的', buf_list)
  #         s1, s2, s3, s4 = int(buf_list[0]), int(buf_list[1]), int(buf_list[2]), int(buf_list[3])
  #         print(buffer, buf_list)
  #         if s1 == 0:
  #           if s2 == 0:
  #             s3 = 0
  #             s4 = 0
  #             self.ser1.write(f'{s1},{s2},{s3},{s4}'.encode())
  #           else:
  #             s1, s2, s3, s4 = buf_list[0], buf_list[1], self.jiaozhun[int(s2) - 1][0], self.jiaozhun[int(s2) - 1][1]
  #             self.ser1.write(f'{s1},{s2},{s3},{s4}'.encode())
  #         elif s1 == 1:
  #           s1, s2, s3, s4 = buf_list[0], buf_list[1], self.guihua[int(s2) - 1][0], self.guihua[int(s2) - 1][1]
  #           self.ser1.write(f'{s1},{s2},{s3},{s4}'.encode())
  # 
  #         print(f'{s1},{s2},{s3},{s4}')
  #         self.CurrentTimes = [s1, s2]
  #       elif (self.ser1.in_waiting <= 0):
  #         time.sleep(0.2)
  #     except:
  #       print('传输数据存在异常')

  def onDj1(self):
    T_Knife = slicer.util.getNode('KnifeToTracker')
    F1 = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', 'DJ-F1')
    self.FemurObserver = T_Knife.AddObserver(
      slicer.vtkMRMLTransformNode.TransformModifiedEvent, self.onTool2)
    self.SendMessegByPC([0, 2,-90,-90])

  def onDj1ensure(self):
    T_KnifeNode = slicer.util.getNode('KnifeToTracker')
    T_KnifeNode.RemoveObserver(self.FemurObserver)
    self.caculate_mian('DJ-F1')

  def onDj2(self):
    self.F = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', 'DJ-F')
    self.F.AddFiducial(12.310,-57.175,32.678)
    T_Knife = slicer.util.getNode('KnifeToTracker')
    # A_Knife = slicer.util.arrayFromTransformMatrix(T_Knife)
    # A_Knife_ni = np.linalg.inv(A_Knife)
    # T_Knife_ni = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLLinearTransformNode', 'T_Knife_ni')
    # T_Knife_ni.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(A_Knife_ni))
    self.F.SetAndObserveTransformNodeID(T_Knife.GetID())
    F1 = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', 'DJ-F2')
    self.FemurObserver = T_Knife.AddObserver(
      slicer.vtkMRMLTransformNode.TransformModifiedEvent, self.onTool1)
    self.SendMessegByPC([0,1,-90,0])
  def onDj2ensure(self):
    T_KnifeNode = slicer.util.getNode('KnifeToTracker')
    T_KnifeNode.RemoveObserver(self.FemurObserver)
    self.caculate_mian('DJ-F2')

  def FixAndCaculateAngle(self):
    #电机1、2回归原位置，计算工具点在电机1、2连线上的投影点，取向量投影点到工具点，计算出刀槽平面上的点，计算电机1、电机2、刀槽点角度，低于90°，顺时针旋转补偿，高于90°，逆时针去除。
    point1 = [0, 0, 0]
    point2 = [0, 0, 0]
    point3 = [0, 0, 0]
    slicer.util.getNode('DJ-F').GetNthControlPointPositionWorld(0, point1)
    slicer.util.getNode('DJ-F1').GetNthControlPointPositionWorld(0, point2)
    slicer.util.getNode('DJ-F2').GetNthControlPointPositionWorld(0, point3)
    dj1_p=np.array(point2)
    dj2_p = np.array(point3)
    gj_p=np.array(point1)
    move=self.Angle(dj1_p-dj2_p,gj_p-dj2_p)   #计算电机偏差角度
    print(move)
    print(move)
    print(move)
    print(move-90)



    #矫正
    if abs(move-90)<10:
      self.SendMessegByPC([0, 1, move-90,0])



    T_KnifeNode = slicer.util.getNode('KnifeToTracker')
    time.sleep(8)
    F1 = slicer.util.getNode('DJ-F2')
    T_Knife = slicer.util.arrayFromTransformMatrix(T_KnifeNode)
    T_Knife_ni = np.linalg.inv(T_Knife)
    points = slicer.util.arrayFromMarkupsControlPoints(F1)
    l = [points[-1][0], points[-1][1], points[-1][2], 1]
    mov = np.dot(T_Knife_ni, l)[0:3]
    F1.RemoveNthControlPoint(len(points) - 1)
    F1.AddControlPoint(mov)
    F1.SetAndObserveTransformNodeID(T_KnifeNode.GetID())


    # 将点电机1放入电机工具追踪工具。并建立电机坐标系.
    N_KnifeNode = slicer.util.getNode('KnifeToTracker')
    F1 = slicer.util.getNode('DJ-F1')
    T_Knife = slicer.util.arrayFromTransformMatrix(N_KnifeNode)
    T_Knife_ni = np.linalg.inv(T_Knife)
    points = slicer.util.arrayFromMarkupsControlPoints(F1)
    l = [points[-1][0], points[-1][1], points[-1][2], 1]
    mov = np.dot(T_Knife_ni, l)[0:3]
    F1.RemoveNthControlPoint(len(points) - 1)
    F1.AddControlPoint(mov)
    F1.SetAndObserveTransformNodeID(N_KnifeNode.GetID())
    point1 = [0, 0, 0]
    point2 = [0, 0, 0]
    point3 = [0, 0, 0]
    slicer.util.getNode('DJ-F').GetNthControlPointPositionWorld(0, point1)
    slicer.util.getNode('DJ-F1').GetNthControlPointPositionWorld(0, point2)
    slicer.util.getNode('DJ-F2').GetNthControlPointPositionWorld(0, point3)
    # point4 = self.getFootPoint(point1, point2, point3)
    dj1_p = np.array(point2)
    dj2_p = np.array(point3)
    gj_p = np.array(point1)
    # gj_ty_p=np.array(point4)
    dc_p = dj1_p + (gj_p - dj2_p)
    # 刀槽平行点（Yzhou）放入电机工具追踪工具
    F1 = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', 'DC-F')
    T_Knife = slicer.util.arrayFromTransformMatrix(N_KnifeNode)
    T_Knife_ni = np.linalg.inv(T_Knife)
    l = [dc_p[0], dc_p[1], dc_p[2], 1]
    mov = np.dot(T_Knife_ni, l)[0:3]
    F1.AddControlPoint(mov)
    F1.SetAndObserveTransformNodeID(N_KnifeNode.GetID())
    point1 = [0, 0, 0]
    point2 = [0, 0, 0]
    point3 = [0, 0, 0]
    slicer.util.getNode('DC-F').GetNthControlPointPosition(0, point1)
    slicer.util.getNode('DJ-F1').GetNthControlPointPosition(0, point2)
    slicer.util.getNode('DJ-F2').GetNthControlPointPosition(0, point3)
    dj1_p = np.array(point2)
    dj2_p = np.array(point3)
    dc_p = np.array(point1)
    jxlz = dj1_p - dj2_p
    moz = np.sqrt(np.square(jxlz[0]) + np.square(jxlz[1]) + np.square(jxlz[2]))  # 基向量z的模
    for i in range(0, 3):
      jxlz[i] = -jxlz[i] / moz
    jxly = dc_p - dj1_p
    moy = np.sqrt(np.square(jxly[0]) + np.square(jxly[1]) + np.square(jxly[2]))  # 基向量y的模
    for i in range(0, 3):
      jxly[i] = -jxly[i] / moy
    jxlx = [0, 0, 0]  # y轴基向量
    jxlx[0] = -(jxly[1] * jxlz[2] - jxly[2] * jxlz[1])
    jxlx[1] = -(jxly[2] * jxlz[0] - jxly[0] * jxlz[2])
    jxlx[2] = -(jxly[0] * jxlz[1] - jxly[1] * jxlz[0])
    mox = np.sqrt(np.square(jxlx[0]) + np.square(jxlx[1]) + np.square(jxlx[2]))  # 基向量x的模
    for i in range(0, 3):
      jxlx[i] = -jxlx[i] / mox
    zb2 = dj1_p
    trans1 = np.array([[float(jxlx[0]), float(jxly[0]), float(jxlz[0]), zb2[0]],
                       [float(jxlx[1]), float(jxly[1]), float(jxlz[1]), zb2[1]],
                       [float(jxlx[2]), float(jxly[2]), float(jxlz[2]), zb2[2]],
                       [0, 0, 0, 1]])
    F3 = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', 'DJ-F3')
    p3 = dj1_p + jxlx
    F3.AddControlPoint(p3)
    F3.SetAndObserveTransformNodeID(N_KnifeNode.GetID())
    # trans1_ni = np.linalg.inv(trans1)
    tannode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLLinearTransformNode', 'trans')
    tannode.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(trans1))

  def onTool1(self,unusedArg1=None, unusedArg2=None, unusedArg3=None):
    F1=slicer.util.getNode('DJ-F2')
    point1 = [0, 0, 0]
    slicer.util.getNode('DJ-F').GetNthControlPointPositionWorld(0, point1)
    F1.AddFiducial(point1[0],point1[1],point1[2])
    
  def onTool2(self,unusedArg1=None, unusedArg2=None, unusedArg3=None):
    F1=slicer.util.getNode('DJ-F1')
    point1 = [0, 0, 0]
    slicer.util.getNode('DJ-F').GetNthControlPointPositionWorld(0, point1)
    F1.AddFiducial(point1[0],point1[1],point1[2])

  def caculate_mian(self, P):
    # 平面拟合，将半圆的Z值变为0，只取X，y拟合计算圆心
    F1 = slicer.util.getNode(P)
    p3 = slicer.util.arrayFromMarkupsControlPoints(F1)
    x = p3[:, 0]
    y = p3[:, 1]
    z = p3[:, 2]
    a = 0
    A = np.ones((len(x), 3))
    for i in range(0, len(x)):
      A[i, 0] = x[a]
      A[i, 1] = y[a]
      a = a + 1

    # print(A)
    # 创建矩阵b
    b = np.zeros((len(x), 1))
    a = 0
    for i in range(0, len(x)):
      b[i, 0] = z[a]
      a = a + 1

    # 通过X=(AT*A)-1*AT*b直接求解
    A_T = A.T
    A1 = np.dot(A_T, A)
    A2 = np.linalg.inv(A1)
    A3 = np.dot(A2, A_T)
    X = np.dot(A3, b)
    n = [X[0, 0], X[1, 0], -1]
    p_1 = [0, 0, 0]
    jxlz = n
    moz = np.sqrt(np.square(jxlz[0]) + np.square(jxlz[1]) + np.square(jxlz[2]))  # 基向量x的模
    for i in range(0, 3):
      jxlz[i] = -jxlz[i] / moz

    p_f = np.array([np.average(x[0:10]), np.average(y[0:10]), np.average(z[0:10])])
    p_n = np.array([np.average(x[-10:-1]), np.average(y[-10:-1]), np.average(z[-10:-1])])
    jxlx = p_n - p_f
    mox = np.sqrt(np.square(jxlx[0]) + np.square(jxlx[1]) + np.square(jxlx[2]))  # 基向量x的模
    for i in range(0, 3):
      jxlx[i] = -jxlx[i] / mox

    jxly = [0, 0, 0]  # y轴基向量
    jxly[0] = -(jxlx[1] * jxlz[2] - jxlx[2] * jxlz[1])
    jxly[1] = -(jxlx[2] * jxlz[0] - jxlx[0] * jxlz[2])
    jxly[2] = -(jxlx[0] * jxlz[1] - jxlx[1] * jxlz[0])
    moy = np.sqrt(np.square(jxly[0]) + np.square(jxly[1]) + np.square(jxly[2]))  # 基向量x的模
    for i in range(0, 3):
      jxly[i] = -jxly[i] / moy

    zb2 = p_f
    trans1 = np.array([[float(jxlx[0]), float(jxly[0]), float(jxlz[0]), zb2[0]],
                       [float(jxlx[1]), float(jxly[1]), float(jxlz[1]), zb2[1]],
                       [float(jxlx[2]), float(jxly[2]), float(jxlz[2]), zb2[2]],
                       [0, 0, 0, 1]])
    trans1_ni = np.linalg.inv(trans1)
    x = []
    y = []
    for i in range(0, len(p3)):
      tmp = [p3[i][0], p3[i][1], p3[i][2], 1]
      x.append(np.dot(trans1_ni, tmp)[0])
      y.append(np.dot(trans1_ni, tmp)[1])

    # 圆心估计
    x_m = np.mean(x)
    y_m = np.mean(y)

    def calc_R(xc, yc):
      """ 计算数据点据圆心(xc, yc)的距离 """
      return np.sqrt((x - xc) ** 2 + (y - yc) ** 2)

    from scipy import odr
    def f_3b(beta, x):
      """ implicit definition of the circle """
      return (x[0] - beta[0]) ** 2 + (x[1] - beta[1]) ** 2 - beta[2] ** 2

    def jacb(beta, x):
      """ Jacobian function with respect to the parameters beta.
      return df_3b/dbeta
      """
      xc, yc, r = beta
      xi, yi = x
      df_db = np.empty((beta.size, x.shape[1]))
      df_db[0] = 2 * (xc - xi)  # d_f/dxc
      df_db[1] = 2 * (yc - yi)  # d_f/dyc
      df_db[2] = -2 * r  # d_f/dr
      return df_db

    def jacd(beta, x):
      """ Jacobian function with respect to the input x.
      return df_3b/dx
      """
      xc, yc, r = beta
      xi, yi = x
      df_dx = np.empty_like(x)
      df_dx[0] = 2 * (xi - xc)  # d_f/dxi
      df_dx[1] = 2 * (yi - yc)  # d_f/dyi
      return df_dx

    def calc_estimate(data):
      """ Return a first estimation on the parameter from the data  """
      xc0, yc0 = data.x.mean(axis=1)
      r0 = np.sqrt((data.x[0] - xc0) ** 2 + (data.x[1] - yc0) ** 2).mean()
      return xc0, yc0, r0

    # for implicit function :
    #       data.x contains both coordinates of the points
    #       data.y is the dimensionality of the response
    lsc_data = odr.Data(np.row_stack([x, y]), y=1)
    lsc_model = odr.Model(f_3b, implicit=True, estimate=calc_estimate, fjacd=jacd, fjacb=jacb)
    lsc_odr = odr.ODR(lsc_data, lsc_model)  # beta0 has been replaced by an estimate function
    lsc_odr.set_job(deriv=3)  # use user derivatives function without checking
    lsc_odr.set_iprint(iter=1, iter_step=1)  # print details for each iteration
    lsc_out = lsc_odr.run()

    xc_3b, yc_3b, R_3b = lsc_out.beta
    Ri_3b = calc_R(xc_3b, yc_3b)
    residu_3b = sum((Ri_3b - R_3b) ** 2)
    center = [xc_3b, yc_3b, 0, 1]
    center1 = np.dot(trans1, center)
    F1.RemoveAllControlPoints()
    F1.AddFiducial(center1[0], center1[1], center1[2])

  def SendMessegByPC(self, buf_list):
    s1, s2, s3, s4 = buf_list[0], buf_list[1], buf_list[2], buf_list[3]
    if s1 == 0:
      if s2 == 0:
        s3 = 0
        s4 = 0
      else:
        s1, s2, s3, s4 = buf_list[0], buf_list[1], buf_list[2], buf_list[3]
    else:
      s1, s2, s3, s4 = buf_list[0], buf_list[1], buf_list[2], buf_list[3]
    self.ser1.write(f'{s1},{s2},{s3},{s4}'.encode())
    print(f'{s1},{s2},{s3},{s4}')
    self.CurrentTimes = [s1, s2]

  def onDCMarker1(self):
    f = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', 'DCMarker')
    probeToTransformNode = slicer.util.getNode("StylusTipToStylus")
    slicer.modules.fiducialregistrationwizard.logic().AddFiducial(probeToTransformNode, f)
    self.ui.DCMarker1.setEnabled(False)
    self.ui.DCMarker2.setEnabled(True)

  def onDCMarker2(self):
    f = slicer.util.getNode('DCMarker')
    probeToTransformNode = slicer.util.getNode("StylusTipToStylus")
    slicer.modules.fiducialregistrationwizard.logic().AddFiducial(probeToTransformNode, f)
    self.ui.DCMarker2.setEnabled(False)
    self.ui.DCMarker3.setEnabled(True)

  def onDCMarker3(self):
    f = slicer.util.getNode('DCMarker')
    probeToTransformNode = slicer.util.getNode("StylusTipToStylus")
    slicer.modules.fiducialregistrationwizard.logic().AddFiducial(probeToTransformNode, f)
    self.ui.DCMarker3.setEnabled(False)
    self.ui.DCMarker4.setEnabled(True)

  def onDCMarker4(self):
    f = slicer.util.getNode('DCMarker')
    probeToTransformNode = slicer.util.getNode("StylusTipToStylus")
    slicer.modules.fiducialregistrationwizard.logic().AddFiducial(probeToTransformNode, f)
    self.ui.DCMarker4.setEnabled(False)

  def onFYAngleJiaoZhun(self):
    if self.ui.FYAngleJiaoZhun.checked:
      self.DriveButtonChecked(self.ui.FYAngleJiaoZhun)
      self.DriveJZVisible()
      self.ui.FYAngleJiaoZhunWidget.setVisible(True)
    else:
      self.DriveJZVisible()

  def FYJiaoZhun(self, step_n,PlaneName):
    if step_n == 1:
      PlaneNode = self.GetTransPoint(PlaneName)

      ras1 = [0, 0, 0]
      ras2 = [0, 0, 0]
      slicer.util.getNode('DJ-F1').GetNthControlPointPositionWorld(0, ras1)
      slicer.util.getNode('DJ-F2').GetNthControlPointPositionWorld(0, ras2)
      DjZAxis_YZ = np.array(self.TouYing(PlaneNode, ras2)) - np.array(self.TouYing(PlaneNode, ras1))
      ras1 = [0, 0, 0]
      ras2 = [0, 0, 0]
      slicer.util.getNode('假体点列').GetNthControlPointPositionWorld(0, ras1)
      slicer.util.getNode('假体点列').GetNthControlPointPositionWorld(1, ras2)
      jtZAxis = np.array(ras2) - np.array(ras1)
      ras1 = [0, 0, 0]
      ras2 = [0, 0, 0]
      slicer.util.getNode('假体竖直方向').GetNthControlPointPositionWorld(0, ras1)
      slicer.util.getNode('假体竖直方向').GetNthControlPointPositionWorld(2, ras2)
      jtYAxis = np.array(ras2) - np.array(ras1)
      if np.dot(jtYAxis, DjZAxis_YZ) < 0:
        angle1 = -self.angle(DjZAxis_YZ, jtZAxis)
      else:
        angle1 = self.angle(DjZAxis_YZ, jtZAxis)
    elif step_n == 2:
      ras1 = [0, 0, 0]
      ras2 = [0, 0, 0]
      slicer.util.getNode('DJ-F1').GetNthControlPointPositionWorld(0, ras1)
      slicer.util.getNode('DJ-F2').GetNthControlPointPositionWorld(0, ras2)
      DjZAxis = np.array(ras2) - np.array(ras1)
      ras1 = [0, 0, 0]
      ras2 = [0, 0, 0]
      slicer.util.getNode('假体点列').GetNthControlPointPositionWorld(0, ras1)
      slicer.util.getNode('假体点列').GetNthControlPointPositionWorld(1, ras2)
      jtZAxis = np.array(ras2) - np.array(ras1)
      ras1 = [0, 0, 0]
      ras2 = [0, 0, 0]
      slicer.util.getNode('假体竖直方向').GetNthControlPointPositionWorld(0, ras1)
      slicer.util.getNode('假体竖直方向').GetNthControlPointPositionWorld(1, ras2)
      jtXAxis = np.array(ras2) - np.array(ras1)
      if np.dot(jtXAxis, DjZAxis) < 0:
        angle1 = -self.angle(DjZAxis, jtZAxis)
      else:
        angle1 = self.angle(DjZAxis, jtZAxis)
    else:
      ras1 = [0, 0, 0]
      ras2 = [0, 0, 0]
      slicer.util.getNode('DJ-F1').GetNthControlPointPositionWorld(0, ras1)
      slicer.util.getNode('DJ-F3').GetNthControlPointPositionWorld(0, ras2)
      DjXAxis = np.array(ras2) - np.array(ras1)
      ras1 = [0, 0, 0]
      ras2 = [0, 0, 0]
      slicer.util.getNode('假体竖直方向').GetNthControlPointPositionWorld(0, ras1)
      slicer.util.getNode('假体竖直方向').GetNthControlPointPositionWorld(2, ras2)
      jtYAxis = np.array(ras2) - np.array(ras1)
      ras1 = [0, 0, 0]
      ras2 = [0, 0, 0]
      slicer.util.getNode('假体竖直方向').GetNthControlPointPositionWorld(0, ras1)
      slicer.util.getNode('假体竖直方向').GetNthControlPointPositionWorld(1, ras2)
      jtXAxis = np.array(ras2) - np.array(ras1)
      if np.dot(jtYAxis, DjXAxis) < 0:
        angle1 = -self.angle(DjXAxis, jtXAxis)
      else:
        angle1 = self.angle(DjXAxis, jtXAxis)

    return angle1

  # 股骨切割
  def onFemurQG(self):
    self.NavigationButtonChecked(self.ui.FemurQG)
    self.HideAll()
    self.ui.NavigationWidget.setVisible(True)
    self.ui.FemurQGWidget.setVisible(True)
    self.YLEnabled(self.ui.FirstPreview)
    self.QGEnabled(self.ui.FirstQG)

  # 胫骨切割
  def onTibiaQG(self):
    self.NavigationButtonChecked(self.ui.TibiaQG)
    self.HideAll()
    self.ui.NavigationWidget.setVisible(True)

  # --------------------------切割------------------------------------------
  # 获取点在线上的垂足
  def getFootPoint(self, point, line_p1, line_p2):
    """
    @point, line_p1, line_p2 : [x, y, z]
    """
    x0 = point[0]
    y0 = point[1]
    z0 = point[2]
    x1 = line_p1[0]
    y1 = line_p1[1]
    z1 = line_p1[2]
    x2 = line_p2[0]
    y2 = line_p2[1]
    z2 = line_p2[2]
    k = -((x1 - x0) * (x2 - x1) + (y1 - y0) * (y2 - y1) + (z1 - z0) * (z2 - z1)) / \
        ((x2 - x1) ** 2 + (y2 - y1) ** 2 + (z2 - z1) ** 2) * 1.0
    xn = k * (x2 - x1) + x1
    yn = k * (y2 - y1) + y1
    zn = k * (z2 - z1) + z1
    p = np.array([xn, yn, zn])
    return p

  # 开始工具校准，开启transform观察者
  def onOpenTool(self):
    # dianji
    # if self.ui.FemurQG.checked:
    JTVertical = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', '假体竖直方向')
    JTVertical.AddControlPoint(0, 0, 0)
    JTVertical.AddControlPoint(50, 0, 0)
    JTVertical.AddControlPoint(0, 50, 0)
    slicer.util.getNode('假体竖直方向').SetDisplayVisibility(False)
    # 假体
    JTPointNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', '假体点列')
    JTPointNode.AddControlPoint(0, 0, 0)
    JTPointNode.AddControlPoint(0, 0, 50)
    JTPointNode.AddControlPoint(50, 0, 0)
    slicer.util.getNode('假体点列').SetDisplayVisibility(False)
    Transform2 = slicer.util.getNode('股骨变换')
    JTPointNode.SetAndObserveTransformNodeID(Transform2.GetID())
    JTVertical.SetAndObserveTransformNodeID(Transform2.GetID())

    # rotationTransformNode = slicer.util.getNode('KnifeToTracker')
    # self.FemurObserver = rotationTransformNode.AddObserver(
    #     slicer.vtkMRMLTransformNode.TransformModifiedEvent, self.onTool)

  # 显示工具校准竖直和水平方向角度
  # 股骨上工具校准
  def onTool(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
    # 刀槽
    DJ = self.GetTransPoint('电机点列')
    tannode = slicer.util.getNode('KnifeToTracker')
    p2 = self.getFootPoint(DJ[0], DJ[1], DJ[2])
    jxlx = DJ[1] - DJ[2]
    jxly = p2 - DJ[0]
    jxlz = [0, 0, 0]  # y轴基向量
    jxlz[0] = -(jxlx[1] * jxly[2] - jxlx[2] * jxly[1])
    jxlz[1] = -(jxlx[2] * jxly[0] - jxlx[0] * jxly[2])
    jxlz[2] = -(jxlx[0] * jxly[1] - jxlx[1] * jxly[0])
    DJHF = jxly
    DJVF = np.array(jxlz)
    # 假体
    JT = self.GetTransPoint('假体点列')
    # 获取法向量
    JTVF = JT[1] - JT[0]
    JTHF = JT[2] - JT[0]
    # 计算夹角
    VAngle = 90 - float(self.angle(DJVF, JTVF))
    HAngle = 90 - float(self.angle(DJHF, JTHF))
    # self.ui.VAngleLabel.setText(str(VAngle) + '°')
    # self.ui.HAngleLabel.setText(str(HAngle) + '°')

  # 胫骨上工具校准
  def onTibiaTool(self, unusedArg1=None, unusedArg2=None, unusedArg3=None):
    # 刀槽
    DJ = self.GetTransPoint('胫骨电机点列')
    DJVF = DJ[2] - DJ[1]
    DJHF = DJ[0] - DJ[1]
    # 假体
    JT = self.GetTransPoint('胫骨假体点列')
    # 获取法向量
    JTVF = JT[1] - JT[0]
    JTHF = JT[2] - JT[0]
    # 计算夹角
    VAngle = 90 - float(self.angle(DJVF, JTVF))
    HAngle = 90 - float(self.angle(DJHF, JTHF))
    # self.ui.VAngleLabel.setText(str(VAngle) + '°')
    # self.ui.HAngleLabel.setText(str(HAngle) + '°')

  # 确认工具校准，关闭transform观察者
  def onConfirmTool(self):
    rotationTransformNode = slicer.util.getNode('KnifeToTracker')
    if self.ui.FemurQG.checked:
      rotationTransformNode.RemoveObserver(self.FemurObserver)
    else:
      rotationTransformNode.RemoveObserver(self.TibiaObserver)

  def cal_ang(self, point_1, point_2, point_3):
    xl1 = [point_1[0] - point_2[0], point_1[1] - point_2[1]]
    xl2 = [point_3[0] - point_2[0], point_3[1] - point_2[1]]

    a1 = np.dot(xl1, xl2)

    a2 = math.sqrt(np.dot(xl1, xl1)) * math.sqrt(np.dot(xl2, xl2))

    B = math.degrees(math.acos(a1 / a2))
    return B

  # 计算电机2所在位置
  def finddj2pos(self, dj1, jd, xian, n):
    r = 49.87
    A = -2 * dj1[0]
    B = -2 * dj1[1]
    C = dj1[0] ** 2 + dj1[1] ** 2 - r ** 2
    if jd == 90:
      k = 9999999
    elif jd == 0:
      k = 0
    else:
      k = math.tan(math.radians(jd))  # 线的参数
    if k == 9999999:  # k为无穷
      x = xian[0]
      a = 1
      b = B
      c = x * x + A * x + C
      y1 = None
      y2 = None
      discriminant = (b ** 2) - (4 * a * c)
      if discriminant == 0:
        y = -(b / (2 * a))

      elif discriminant < 0:
        print('90度，没有交点')
      else:
        root = math.sqrt(discriminant)
        y1 = (-b + root) / (2 * a)
        y2 = (-b - root) / (2 * a)
        x1 = x
        x2 = x
        dian1 = [x1, y1]
        dian2 = [x2, y2]
        if n == 5:
          if y2 < y1:
            y = y2
            x = x2
          else:
            y = y1
            x = x1
        else:
          if y2 > y1:
            y = y2
            x = x2
          else:
            y = y1
            x = x1

    elif k == 0:
      y = xian[1]
      a = 1
      b = A
      c = y * y + B * y + C
      x1 = None
      x2 = None
      discriminant = (b ** 2) - (4 * a * c)
      if discriminant == 0:
        x = -(b / (2 * a))

      elif discriminant < 0:
        print('没有交点')
      else:
        root = math.sqrt(discriminant)
        x1 = (-b + root) / (2 * a)
        x2 = (-b - root) / (2 * a)
        y1 = y
        y2 = y
        dian1 = [x1, y1]
        dian2 = [x2, y2]

    else:
      F = xian[1] - k * xian[0]
      a = k * k + 1
      b = 2 * k * F + A + B * k
      c = F * F + B * F + C
      x1 = None
      x2 = None
      discriminant = (b ** 2) - (4 * a * c)
      if discriminant == 0:
        x1 = -(b / (2 * a))
        y1 = k * x1 + F
        y = y1
        x = x1
      elif discriminant < 0:
        print('没有交点')
      else:
        root = math.sqrt(discriminant)
        x1 = (-b + root) / (2 * a)
        x2 = (-b - root) / (2 * a)
        y1 = k * x1 + F
        y2 = k * x2 + F
        dian1 = [x1, y1]
        dian2 = [x2, y2]
        if n == 5:
          if y2 < y1:
            y = y2
            x = x2
          else:
            y = y1
            x = x1
        else:
          if y2 > y1:
            y = y2
            x = x2
          else:
            y = y1
            x = x1

    dj2pos = [x, y]
    return dj2pos

  def finddj2pos1(self, dj1, jd, xian, xian1):
    r = 49.87
    A = -2 * dj1[0]
    B = -2 * dj1[1]
    C = dj1[0] ** 2 + dj1[1] ** 2 - r ** 2
    if jd == 90:
      k = 9999999
    elif jd == 0:
      k = 0
    else:
      k = math.tan(math.radians(jd))  # 线的参数
    if k == 9999999:
      x = xian[0]
      a = 1
      b = B
      c = x * x + A * x + C
      y1 = None
      y2 = None
      discriminant = (b ** 2) - (4 * a * c)
      if discriminant == 0:
        y1 = -(b / (2 * a))
        y = y1

      elif discriminant < 0:
        print('没有交点')
      else:
        root = math.sqrt(discriminant)
        y1 = (-b + root) / (2 * a)
        y2 = (-b - root) / (2 * a)
        x1 = x
        x2 = x
        n1 = np.array([xian1[0] - dj1[0], xian1[1] - dj1[1]])
        n2 = np.array([x1 - x2, y1 - y2])
        if np.dot(n1, n2) > 0:
          x = x1
          y = y1
        else:
          x = x2
          y = y2

    elif k == 0:
      y = xian[1]
      a = 1
      b = A
      c = y * y + B * y + C
      x1 = None
      x2 = None
      discriminant = (b ** 2) - (4 * a * c)
      if discriminant == 0:
        x = -(b / (2 * a))

      elif discriminant < 0:
        print('没有交点')
      else:
        root = math.sqrt(discriminant)
        x1 = (-b + root) / (2 * a)
        x2 = (-b - root) / (2 * a)
        y1 = y
        y2 = y
        n1 = np.array([xian1[0] - dj1[0], xian1[1] - dj1[1]])
        n2 = np.array([x1 - x2, y1 - y2])
        if np.dot(n1, n2) > 0:
          x = x1
          y = y1
        else:
          x = x2
          y = y2



    else:
      F = xian[1] - k * xian[0]
      a = k * k + 1
      b = 2 * k * F + A + B * k
      c = F * F + B * F + C
      x1 = None
      x2 = None
      discriminant = (b ** 2) - (4 * a * c)
      if discriminant == 0:
        x1 = -(b / (2 * a))
        y1 = k * x1 + F
        y = y1
        x = x1
      elif discriminant < 0:
        print('没有交点')
      else:
        root = math.sqrt(discriminant)
        x1 = (-b + root) / (2 * a)
        x2 = (-b - root) / (2 * a)
        y1 = k * x1 + F
        y2 = k * x2 + F
        if x1 > x2:
          x = x1
          y = y1
        else:
          x = x2
          y = y2

    dj2pos = [x, y]
    return dj2pos

  # 计算电机1与电机2坐标
  def DJAxis(self):
    # 初始化电机坐标
    # 得到变换后的坐标
    JTV = self.GetTransPoint('假体竖直方向')
    JTH = self.GetTransPoint('假体点列')
    point1, point2 = [0, 0, 0], [0, 0, 0]
    slicer.util.getNode('DJ-F1').GetNthControlPointPositionWorld(0, point1)
    slicer.util.getNode('DJ-F2').GetNthControlPointPositionWorld(0, point2)
    d2_OP = np.dot(JTV[0] - point1, JTV[2] - JTV[0])
    d4_OP = np.dot(JTV[0] - point2, JTV[2] - JTV[0])
    d1 = self.point2area_distance(JTV, point1)
    d2 = self.point2area_distance(JTH, point1)
    d3 = self.point2area_distance(JTV, point2)
    d4 = self.point2area_distance(JTH, point2)
    if d2_OP < 0:
      d2 = -d2
    if d4_OP < 0:
      d4 = -d4

    DJ1 = [-d1, d2]
    DJ2 = [d3, d4]
    print(DJ1, DJ2)
    return DJ1, DJ2

  # 初始化计算角度
  def onInit(self):
    # 计算电机配准后位置1
    # 先将电机配准至股骨附近，再开启后续工作

    dj1, self.dj2pre = self.DJAxis()
    # 根据假体加载数据和电机位置
    a = self.jiatiload.GetName()

    if a == 'femur-R1-5' or a == 'femur-l1-5':
      zuobiao = [[-25.499954, -0.135762, -0.019000],
                 [-25.149643, -14.354305, 5.265232],
                 [-27.852438, 12.519020, 3.365906],
                 [-21.377022, -23.346901, 24.886263],
                 [-27.514839, 16.021688, 13.103391]]
    elif a == 'femur-R2' or a == 'femur-l2':
      zuobiao = [[-28.450209, -1.561671, -0.020100],
                 [-26.150209, -16.200544, 6.580470],
                 [-29.108992, 13.373281, 3.709354],
                 [-22.788954, -24.413679, 22.864733],
                 [-29.272203, 17.748457, 14.366339]]
    elif a == 'femur-R2-5' or a == 'femur-l2-5':
      zuobiao = [[-29.203060, 0.230665, -0.022000],
                 [-27.077915, -17.746336, 7.415043],
                 [-30.480453, 13.752953, 3.367434],
                 [-23.839167, -26.293257, 25.511642],
                 [-30.759085, 18.555136, 15.014627]]

    elif a == 'femur-R3' or a == 'femur-l3':
      zuobiao = [[31.990156, 1.195784, -0.023500],
                 [28.628462, 20.361845, 9.097789],
                 [32.454105, -16.141815, 4.812799],
                 [24.988913, 27.960649, 24.804714],
                 [32.279945, -19.533432, 18.239368]]
    elif a == 'femur-R4' or a == 'femur-l4':
      zuobiao = [[-34.335312, -1.680947, -0.025500],
                 [-31.508013, -19.554201, 7.106586],
                 [-34.702728, 17.421394, 4.862123],
                 [-27.161606, -30.459152, 27.348459],
                 [-34.555763, 21.053728, 18.832129]]

    elif a == 'femur-R5' or a == 'femur-l5':
      zuobiao = [[-35.313412, 0.288476, -0.026500],
                 [-31.950918, -19.908022, 8.154259],
                 [-35.494095, 17.520115, 5.660280],
                 [-28.176050, -30.925489, 27.422649],
                 [-34.989296, 23.241940, 21.213896]]

    # #胫骨切割
    # xian = [0, 0]
    # Tibiajd =90
    # Tibia_dj2pos = self.finddj2pos(dj1,Tibiajd,xian,1)
    # print(Tibia_dj2pos)
    # self.Tibia_angle1 = self.jd1zhengfu(self.dj2pre, Tibia_dj2pos)*self.cal_ang(self.dj2pre, dj1, Tibia_dj2pos)
    # self.Tibia_angle2 = 90- self.cal_ang(dj1, one_dj2pos,xian)
    # print(round(self.Tibia_angle1, 1), round(self.Tibia_angle2, 1))

    # 第一刀
    xian1 = [-zuobiao[0][2], -zuobiao[0][1]]
    # xian1 = [0.866,-2.419]
    jd = 90
    one_dj2pos = self.finddj2pos(dj1, jd, xian1, 1)
    print('dj', dj1, self.dj2pre)
    print(one_dj2pos)
    self.one_angle1 = self.jd1zhengfu(self.dj2pre, one_dj2pos) * self.cal_ang(self.dj2pre, dj1, one_dj2pos)  # 顺时针
    self.one_angle2 = 90 - self.cal_ang(dj1, one_dj2pos, xian1)  # 逆时针
    print(self.one_angle1, self.one_angle2)
    print(round(self.one_angle1, 1), round(self.one_angle2, 1))

    # 第二刀
    # xian2tmp=[-zuobiao[1][2],-zuobiao[1][1]]
    jd1 = -6
    print('计算出来的点')
    # xian2 = self.findPointAxis(xian2tmp, 90+jd1)
    # xian2=[7.084,-17.184]
    xian2 = [-zuobiao[3][2], -zuobiao[3][1]]
    two_dj2pos = self.finddj2pos1(dj1, jd1, xian2, xian1)
    print('电机2的位置')
    print(two_dj2pos)
    self.two_angle1 = self.jd1zhengfu(one_dj2pos, two_dj2pos) * self.cal_ang(one_dj2pos, dj1, two_dj2pos)
    two_angle2_tmp = 90 + self.cal_ang(dj1, two_dj2pos, xian2)  # 相对于原位置的角度
    self.two_angle2 = two_angle2_tmp - self.one_angle2  # 实际转动角度
    print(self.two_angle1, self.two_angle2)
    print(round(self.two_angle1, 1), round(self.two_angle2, 1))

    # 第三刀
    # xian3tmp = [-zuobiao[2][2], -zuobiao[2][1]]
    jd2 = 1
    print('计算出来的点')
    # xian3 = self.findPointAxis(xian3tmp, jd2-90)
    # xian3=[4.269,14.369]
    xian3 = [-zuobiao[4][2], -zuobiao[4][1]]
    thr_dj2pos = self.finddj2pos1(dj1, jd2, xian3, xian1)
    self.thr_angle1 = self.jd1zhengfu(two_dj2pos, thr_dj2pos) * self.cal_ang(two_dj2pos, dj1, thr_dj2pos)
    thr_dj2pos = self.finddj2pos1(dj1, jd2, xian3, xian1)
    thr_angle2_tmp = 90 - self.cal_ang(dj1, thr_dj2pos, xian3)  # 相对于原位置的角度
    self.thr_angle2 = thr_angle2_tmp - two_angle2_tmp  # 实际转动角度
    print('电机2的位置')
    print(thr_dj2pos)
    print(self.thr_angle1, self.thr_angle2)
    print(round(self.thr_angle1, 1), round(self.thr_angle2, 1))

    # 第四刀
    # xian4tmp = [-zuobiao[3][2], -zuobiao[3][1]]
    jd3 = 45
    print('计算出来的点')
    # xian4 = self.findPointAxis(xian4tmp, 90+jd3)
    # xian4=[27.755,-26.179]
    xian4 = [-zuobiao[2][2], -zuobiao[2][1]]
    four_dj2pos = self.finddj2pos(dj1, jd3, xian4, 4)
    self.four_angle1 = self.jd1zhengfu(thr_dj2pos, four_dj2pos) * self.cal_ang(thr_dj2pos, dj1, four_dj2pos)
    four_angle2_tmp = 90 - self.cal_ang(dj1, four_dj2pos, xian4)  # 相对于原位置的角度
    self.four_angle2 = four_angle2_tmp - thr_angle2_tmp  # 实际转动角度
    print('电机2的位置')
    print(four_dj2pos)
    print(self.four_angle1, self.four_angle2)
    print(round(self.four_angle1, 1), round(self.four_angle2, 1))

    # 第五刀
    # xian5tmp = [-zuobiao[4][2], -zuobiao[4][1]]
    jd4 = -45
    print('计算出来的点')
    # xian5 = self.findPointAxis(xian5tmp, jd4-90)
    # xian5=[14.113,18.197]
    xian5 = [-zuobiao[1][2], -zuobiao[1][1]]
    five_dj2pos = self.finddj2pos(dj1, jd4, xian5, 5)
    self.five_angle1 = self.jd1zhengfu(four_dj2pos, five_dj2pos) * self.cal_ang(four_dj2pos, dj1, five_dj2pos)
    five_angle2_tmp = 90 + self.cal_ang(dj1, five_dj2pos, xian5)  # 相对于原位置的角度
    self.five_angle2 = five_angle2_tmp - four_angle2_tmp  # 实际转动角度

    self.num1 = 0
    self.num2 = 0
    self.fx1 = 0
    self.fx2 = 0
    print('电机2的位置')
    print(five_dj2pos)
    print(self.five_angle1, self.five_angle2)
    print(round(self.five_angle1, 1), round(self.five_angle2, 1))
    x1 = [self.one_angle1, self.two_angle1, self.thr_angle1, self.four_angle1, self.five_angle1]
    x2 = [-self.one_angle2, -self.two_angle2, -self.thr_angle2, -self.four_angle2, -self.five_angle2]
    self.dj1_position = [x1[0], x1[0] + x1[1], x1[0] + x1[1] + x1[2], x1[0] + x1[1] + x1[2] + x1[3],
                         x1[0] + x1[1] + x1[2] + x1[3] + x1[4]]
    self.dj2_position = [x2[0], x2[0] + x2[1], x2[0] + x2[1] + x2[2], x2[0] + x2[1] + x2[2] + x2[3],
                         x2[0] + x2[1] + x2[2] + x2[3] + x2[4]]

  # 轨迹预览Enabled
  def YLEnabled(self, checkBox):
    self.ui.FirstPreview.setEnabled(False)
    self.ui.SecondPreview.setEnabled(False)
    self.ui.ThirdPreview.setEnabled(False)
    self.ui.FourthPreview.setEnabled(False)
    self.ui.FifthPreview.setEnabled(False)
    checkBox.setEnabled(True)
    self.setPreviewBoxState(checkBox)

  def setPreviewBoxState(self, checkBox):
    for i in range(0, len(self.ui.PreviewBox.findChildren("QCheckBox"))):
      self.ui.PreviewBox.findChildren("QCheckBox")[i].setStyleSheet("None")
    checkBox.setStyleSheet("background:#515151;color:#7bcd27;font-weight:bold;")

  def setQGBoxState(self, checkBox):
    for i in range(0, len(self.ui.QGBox.findChildren("QCheckBox"))):
      self.ui.QGBox.findChildren("QCheckBox")[i].setStyleSheet("None")
    checkBox.setStyleSheet("background:#515151;color:#7bcd27;font-weight:bold;")

  def onFirstPreview(self):
    self.onGetMatrix(1)
    self.YLEnabled(self.ui.SecondPreview)

  def onSecondPreview(self):
    self.onGetMatrix(2)
    self.YLEnabled(self.ui.ThirdPreview)

  def onThirdPreview(self):
    self.onGetMatrix(3)
    self.YLEnabled(self.ui.FourthPreview)

  def onFourthPreview(self):
    self.onGetMatrix(4)
    self.YLEnabled(self.ui.FifthPreview)

  def onFifthPreview(self):
    self.onGetMatrix(5)
    self.ui.FifthPreview.setEnabled(False)

  def onGetMatrix(self, n):
    import time
    x1 = [-self.one_angle1, -self.two_angle1, -self.thr_angle1, -self.four_angle1, -self.five_angle1]
    x2 = [-self.one_angle2, -self.two_angle2, -self.thr_angle2, -self.four_angle2, -self.five_angle2]
    dj1 = [0, x1[0], x1[0] + x1[1], x1[0] + x1[1] + x1[2], x1[0] + x1[1] + x1[2] + x1[3],
           x1[0] + x1[1] + x1[2] + x1[3] + x1[4]]
    dj2 = [0, x2[0], x2[0] + x2[1], x2[0] + x2[1] + x2[2], x2[0] + x2[1] + x2[2] + x2[3],
           x2[0] + x2[1] + x2[2] + x2[3] + x2[4]]
    transformNode = slicer.util.getNode('xz1')
    for i in range(1, 11):
      print(i)
      Tjxlx = [0, 1, 0]
      jd1 = dj1[n - 1] + (dj1[n] - dj1[n - 1]) * i / 10
      print(jd1)
      jd = math.radians(jd1)
      xzjz = np.array([[math.cos(jd) + Tjxlx[0] * Tjxlx[0] * (1 - math.cos(jd)),
                        -Tjxlx[2] * math.sin(jd) + Tjxlx[0] * Tjxlx[1] * (1 - math.cos(jd)),
                        Tjxlx[1] * math.sin(jd) + Tjxlx[0] * Tjxlx[2] * (1 - math.cos(jd)), 0],
                       [Tjxlx[2] * math.sin(jd) + Tjxlx[0] * Tjxlx[1] * (1 - math.cos(jd)),
                        math.cos(jd) + Tjxlx[1] * Tjxlx[1] * (1 - math.cos(jd)),
                        -Tjxlx[0] * math.sin(jd) + Tjxlx[1] * Tjxlx[2] * (1 - math.cos(jd)), 0],
                       [-Tjxlx[1] * math.sin(jd) + Tjxlx[0] * Tjxlx[2] * (1 - math.cos(jd)),
                        Tjxlx[0] * math.sin(jd) + Tjxlx[1] * Tjxlx[2] * (1 - math.cos(jd)),
                        math.cos(jd) + Tjxlx[2] * Tjxlx[2] * (1 - math.cos(jd)), 0],
                       [0, 0, 0, 1]])
      transformNode.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(xzjz))
      slicer.app.processEvents()
      time.sleep(0.1)
    transformNode2 = slicer.util.getNode('xz2')
    for i in range(1, 11):
      Tjxlx = [0, 1, 0]
      jd11 = dj2[n - 1] + (dj2[n] - dj2[n - 1]) * i / 10
      print(jd11)
      jd2 = math.radians(jd11)
      xzjz1 = np.array([[math.cos(jd2) + Tjxlx[0] * Tjxlx[0] * (1 - math.cos(jd2)),
                         -Tjxlx[2] * math.sin(jd2) + Tjxlx[0] * Tjxlx[1] * (1 - math.cos(jd2)),
                         Tjxlx[1] * math.sin(jd2) + Tjxlx[0] * Tjxlx[2] * (1 - math.cos(jd2)), 0],
                        [Tjxlx[2] * math.sin(jd2) + Tjxlx[0] * Tjxlx[1] * (1 - math.cos(jd2)),
                         math.cos(jd2) + Tjxlx[1] * Tjxlx[1] * (1 - math.cos(jd2)),
                         -Tjxlx[0] * math.sin(jd2) + Tjxlx[1] * Tjxlx[2] * (1 - math.cos(jd2)), 0],
                        [-Tjxlx[1] * math.sin(jd2) + Tjxlx[0] * Tjxlx[2] * (1 - math.cos(jd2)),
                         Tjxlx[0] * math.sin(jd2) + Tjxlx[1] * Tjxlx[2] * (1 - math.cos(jd2)),
                         math.cos(jd2) + Tjxlx[2] * Tjxlx[2] * (1 - math.cos(jd2)), 0],
                        [0, 0, 0, 1]])
      transformNode2.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(xzjz1))
      slicer.app.processEvents()
      time.sleep(0.1)

  def onPreviewReSet(self):
    self.YLChecked(self.ui.PreviewReSet)
    dj1xz = slicer.util.getNode('xz1')
    dj2xz = slicer.util.getNode('xz2')
    dj1xz.SetMatrixTransformToParent(vtk.vtkMatrix4x4())
    dj2xz.SetMatrixTransformToParent(vtk.vtkMatrix4x4())
    self.YLChecked(self.ui.FirstPreview)
    self.ui.FirstPreview.setChecked(False)
    self.ui.SecondPreview.setChecked(False)
    self.ui.ThirdPreview.setChecked(False)
    self.ui.FourthPreview.setChecked(False)
    self.ui.FifthPreview.setChecked(False)

  # 切割setCheckedable
  def QGEnabled(self, checkBox):
    self.ui.FirstQG.setEnabled(False)
    self.ui.SecondQG.setEnabled(False)
    self.ui.ThirdQG.setEnabled(False)
    self.ui.FourthQG.setEnabled(False)
    self.ui.FifthQG.setEnabled(False)
    checkBox.setEnabled(True)
    self.setQGBoxState(checkBox)
    self.ViewTip(self.view1)

  def onFirstQG(self):
    print(self.dj1_position[0], self.dj2_position[0])
    self.ondianji(self.dj1_position[0], self.dj2_position[0], 1)
    self.QGEnabled(self.ui.SecondQG)

  def onSecondQG(self):
    print(self.dj1_position[1], self.dj2_position[1])
    self.ondianji(self.dj1_position[1], self.dj2_position[1], 2)
    self.QGEnabled(self.ui.ThirdQG)

  def onThirdQG(self):
    print(self.dj1_position[2], self.dj2_position[2])
    self.ondianji(self.dj1_position[2], self.dj2_position[2], 3)
    self.QGEnabled(self.ui.FourthQG)

  def onFourthQG(self):
    print(self.dj1_position[3], self.dj2_position[3])
    self.ondianji(self.dj1_position[3], self.dj2_position[3], 4)
    self.QGEnabled(self.ui.FifthQG)

  def onFifthQG(self):
    print(self.dj1_position[4], self.dj2_position[4])
    self.ondianji(self.dj1_position[4], self.dj2_position[4], 5)
    self.ui.FifthQG.setEnabled(False)

  def onQGReSet(self):
    self.ondianji(self.five_angle1, -self.five_angle2, 999)
    self.QGEnabled(self.ui.FirstQG)
    self.ui.FirstQG.setChecked(False)
    self.ui.SecondQG.setChecked(False)
    self.ui.ThirdQG.setChecked(False)
    self.ui.FourthQG.setChecked(False)
    self.ui.FifthQG.setChecked(False)

  def findPointAxis(self, zuobiao, jd):
    import math
    juli = 60
    x1 = juli * math.cos(math.radians(jd))
    y1 = juli * math.sin(math.radians(jd))
    zuobiao1 = [0, 0]
    zuobiao1[0] = zuobiao[0] - x1
    zuobiao1[1] = zuobiao[1] - y1
    print(zuobiao1)
    return zuobiao1

  def ViewTip(self, view):
    V11 = qt.QLabel(view)
    V12 = qt.QLabel(view)
    V13 = qt.QLabel(view)
    V14 = qt.QLabel(view)
    V15 = qt.QLabel(view)
    V16 = qt.QLabel(view)
    V17 = qt.QLabel(view)
    V18 = qt.QLabel(view)
    V11.setGeometry(view.contentsRect().width() - 100, 25, 100, 25)
    V11.setText(" 伸直夹角 ")
    V11.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V11.show()
    V12.setGeometry(view.contentsRect().width() - 100, 50, 100, 25)
    try:
      V12.setText(' ' + str(round(self.ShenZhiJiaJiao, 1)) + '°')
    except Exception as e:
      print(e)
    V12.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V12.show()
    V13.setGeometry(view.contentsRect().width() - 100, 75, 100, 25)
    V13.setText(" 伸直间隙 ")
    V13.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V13.show()
    V14.setGeometry(view.contentsRect().width() - 100, 100, 100, 25)
    try:
      V14.setText(' ' + str(round(self.ShenZhiJianXi, 1)) + 'mm')
    except Exception as e:
      print(e)
    V14.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V14.show()
    V15.setGeometry(0, 25, 100, 25)
    V15.setText(" 屈膝夹角 ")
    V15.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V15.show()
    V16.setGeometry(0, 50, 100, 25)
    try:
      V16.setText(' ' + str(round(self.QuXiJiaJiao, 1)) + 'mm')
    except Exception as e:
      print(e)
    V16.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V16.show()
    V17.setGeometry(0, 75, 100, 25)
    V17.setText(" 屈膝间隙 ")
    V17.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V17.show()
    V18.setGeometry(0, 100, 100, 25)
    try:
      V18.setText(' ' + str(round(self.QuXiJianXi, 1)) + 'mm')
    except Exception as e:
      print(e)
    V18.setStyleSheet('QLabel{background-color:transparent;color:#7BCD27;font:15px;}')
    V18.show()

  def CalculationAngle(self):
    # 伸直夹角
    # 获取世界坐标系下截骨面的坐标矩阵
    FJGM = self.GetTransPoint('股骨真实截骨面')
    # 胫骨真实截骨面
    TJGM = self.GetTransPoint('胫骨电机点列')
    # 获取截骨面的法向量
    FFXL = self.normal(FJGM)
    TFXL = self.normal(TJGM)
    self.ShenZhiJiaJiao = self.angle(FFXL, TFXL)
    # 屈膝夹角
    F3JGM = self.GetTransPoint('股骨第三真实截骨面')
    F3FXL = self.normal(F3JGM)
    self.QuXiJiaJiao = self.angle(F3FXL, TFXL)

  def CalculationDistance(self):
    # 伸直间隙
    point1, point2 = [0, 0, 0], [0, 0, 0]
    slicer.util.getNode('股骨真实截骨面').GetNthControlPointPositionWorld(0, point1)
    TJGM = self.GetTransPoint('胫骨电机点列')
    self.ShenZhiJianXi = self.point2area_distance(TJGM, point1)
    # 屈曲间隙
    slicer.util.getNode('内侧后髁').GetNthControlPointPositionWorld(0, point2)
    F3JGM = self.GetTransPoint('股骨第三真实截骨面')
    self.QuXiJianXi = self.point2area_distance(F3JGM, point2)

  # 判断角度正负
  def jd1zhengfu(self, point_pre, point_pos):
    a = -1
    xl1 = np.array([point_pos[0] - point_pre[0], point_pos[1] - point_pre[1]])
    xl2 = np.array([0, -1])
    if np.dot(xl1, xl2) > 0:
      a = 1
    return a

  # 判断这三个点是否能构成平面
  def isplane(self, a):
    coors = [[], [], []]  # 三个点的xyz坐标分别放在同一个列表用来比较
    for _point in a:  # 对于每个点
      coors[0].append(_point.x)
      coors[1].append(_point.y)
      coors[2].append(_point.z)
    for coor in coors:
      if coor[0] == coor[1] == coor[2]:  # 如果三个点的x或y或z坐标相等 则不能构成平面
        return print('Points:cannot form a plane')

  # 获得平面的法向量
  def normal(self, a):
    # self.isplane()  # 获得该平面的法向量前提是能构成平面
    A = a[0]
    B = a[1]
    C = a[2]  # 对应三个点
    AB = [B[0] - A[0], B[1] - A[1], B[2] - A[2]]  # 向量AB
    AC = [C[0] - A[0], C[1] - A[1], C[2] - A[2]]  # 向量AC
    B1 = AB[0]
    B2 = AB[1]
    B3 = AB[2]  # 向量AB的xyz坐标
    C1 = AC[0]
    C2 = AC[1]
    C3 = AC[2]  # 向量AC的xyz坐标
    n = [B2 * C3 - C2 * B3, B3 * C1 - C3 * B1, B1 * C2 - C1 * B2]  # 已知该平面的两个向量,求该平面的法向量的叉乘公式
    return n

  # 两个平面的夹角
  def angle(self, P1, P2):
    import math
    x1, y1, z1 = P1  # 该平面的法向量的xyz坐标
    x2, y2, z2 = P2  # 另一个平面的法向量的xyz坐标
    cosθ = ((x1 * x2) + (y1 * y2) + (z1 * z2)) / (
            ((x1 ** 2 + y1 ** 2 + z1 ** 2) ** 0.5) * ((x2 ** 2 + y2 ** 2 + z2 ** 2) ** 0.5))  # 平面向量的二面角公式
    degree = math.degrees(math.acos(cosθ))
    if degree > 90:  # 二面角∈[0°,180°] 但两个平面的夹角∈[0°,90°]
      degree = 180 - degree

    return str(round(degree, 5))

  # 删除节点
  def DeleteNode(self, node):
    try:
      slicer.mrmlScene.RemoveNode(slicer.util.getNode(node))
    except Exception as e:
      print(e)

  # 删除前面创建的所有节点
  def DeleteAllNode(self):
    self.DeleteNode('股骨远端切割')
    self.DeleteNode('DynamicModeler')
    self.DeleteNode('股骨远端')
    self.DeleteNode('切割1')
    self.DeleteNode('切割2')
    self.DeleteNode('切割3')
    self.DeleteNode('切割4')
    self.DeleteNode('切割5')
    self.DeleteNode('部件1')
    self.DeleteNode('部件2')
    self.DeleteNode('部件3')
    self.DeleteNode('部件4')
    self.DeleteNode('部件5')
    self.DeleteNode('动态切割1')
    self.DeleteNode('动态切割2')
    self.DeleteNode('动态切割3')
    self.DeleteNode('动态切割4')
    self.DeleteNode('动态切割5')
    self.DeleteNode('股骨切割')
    self.DeleteNode('胫骨近端切割')
    self.DeleteNode('胫骨近端')
    self.DeleteNode('DynamicModeler_1')
    self.DeleteNode('胫骨切割')
    self.DeleteNode('切割6')
    self.DeleteNode('部件6')
    self.DeleteNode('动态切割6')
    self.DeleteNode('内侧高点')
    self.DeleteNode('外侧高点')
    # 将变换相乘 得到股骨变换和胫骨变换
    try:
      transform = slicer.util.arrayFromTransformMatrix(slicer.util.getNode('变换'))
      transform1 = slicer.util.arrayFromTransformMatrix(slicer.util.getNode('变换_1'))
      transform2 = slicer.util.arrayFromTransformMatrix(slicer.util.getNode('变换_2'))
      transform3 = slicer.util.arrayFromTransformMatrix(slicer.util.getNode('变换_3'))
      transform4 = slicer.util.arrayFromTransformMatrix(slicer.util.getNode('变换_4'))
      transform5 = slicer.util.arrayFromTransformMatrix(slicer.util.getNode('变换_5'))
      transform6 = slicer.util.arrayFromTransformMatrix(slicer.util.getNode('变换_6'))
      transform_R = slicer.util.arrayFromTransformMatrix(slicer.util.getNode('变换_R'))
      transform_jg = slicer.util.arrayFromTransformMatrix(slicer.util.getNode('变换_胫骨'))
      transform_ys = slicer.util.arrayFromTransformMatrix(slicer.util.getNode('变换_约束'))
      transform_tmp = slicer.util.arrayFromTransformMatrix(slicer.util.getNode('变换_临时'))
      FemurTrans = np.dot(np.dot(np.dot(np.dot(transform, transform1), transform_tmp), transform2), transform_R)
      TibiaTrans = np.dot(np.dot(np.dot(np.dot(transform6, transform3), transform4), transform_jg), transform_ys)
      FemurTransform = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", '股骨变换')
      FemurTransform.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(FemurTrans))
      TibiaTransform = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", '胫骨变换')
      TibiaTransform.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(TibiaTrans))
      self.jiatiload.SetAndObserveTransformNodeID(FemurTransform.GetID())
      transNode1 = slicer.util.getNode('DianjiToTracker1')
      transNode2 = slicer.util.getNode('TibiaToTracker')
      FemurTransform.SetAndObserveTransformNodeID(transNode1.GetID())
      TibiaTransform.SetAndObserveTransformNodeID(transNode2.GetID())
      slicer.util.getNode('Femur').SetAndObserveTransformNodeID(FemurTransform.GetID())
      slicer.util.getNode('Tibia').SetAndObserveTransformNodeID(TibiaTransform.GetID())

    except Exception as e:
      print(e)

    self.DeleteNode('变换')
    self.DeleteNode('变换_1')
    self.DeleteNode('变换_2')
    self.DeleteNode('变换_3')
    self.DeleteNode('变换_4')
    self.DeleteNode('变换_5')
    self.DeleteNode('变换_6')
    self.DeleteNode('变换_胫骨')
    self.DeleteNode('变换_约束')
    self.DeleteNode('变换_临时')
    self.DeleteNode('股骨截骨面')
    self.DeleteNode('股骨第二截骨面')
    self.DeleteNode('股骨第三截骨面')
    self.DeleteNode('胫骨截骨面')
    self.DeleteNode('股骨头球心')
    self.DeleteNode('开髓点')
    self.DeleteNode('H点')
    self.DeleteNode('A点')
    self.DeleteNode('外侧远端')
    self.DeleteNode('内侧远端')
    self.DeleteNode('外侧皮质高点')
    self.DeleteNode('外侧后髁')
    self.DeleteNode('内侧后髁')
    self.DeleteNode('内侧凹点')
    self.DeleteNode('外侧凸点')
    self.DeleteNode('胫骨隆凸')
    self.DeleteNode('胫骨结节')
    self.DeleteNode('踝穴中心')
    self.DeleteNode('Femur_ZAxis')
    self.DeleteNode('Femur_XAxis')
    self.DeleteNode('Femur_ZJtAxis')
    self.DeleteNode('Femur_YJtAxis')
    self.DeleteNode('OutSide')
    self.DeleteNode('InSide')
    self.DeleteNode('Tibia_ZAxis')
    self.DeleteNode('Tibia_XAxis')
    self.DeleteNode('Tibia_YAxis')
    self.DeleteNode('Tibia_YZPlane')
    self.DeleteNode('Tibia_XZPlane')
    self.DeleteNode('Tibia_ZJtAxis')
    self.DeleteNode('InSide')

    try:
      self.jiatiload.SetDisplayVisibility(False)
      self.TibiaJiaTiload.SetDisplayVisibility(False)
      self.ChenDian.SetDisplayVisibility(False)
    except Exception as e:
      print(e)
    #获得真实截骨面
  def RealJGM(self,transform1,transform2,name):
    point1, point2, point3 = [0, 0, 0], [0, 0, 0], [0, 0, 0]
    slicer.util.getNode('dc').GetNthControlPointPositionWorld(0, point1)
    slicer.util.getNode('dc').GetNthControlPointPositionWorld(1, point2)
    slicer.util.getNode('dc').GetNthControlPointPositionWorld(2, point3)
    zb = np.array([[point1[0], point1[1], point1[2], 0],
                   [point2[0], point2[1], point2[2], 0],
                   [point3[0], point3[1], point3[2], 0],
                   [0,0,0,1]])
    transformNode = slicer.util.getNode(transform1)
    trans = slicer.util.arrayFromTransformMatrix(transformNode)
    transformNode1 = slicer.util.getNode(transform2)
    trans1 = slicer.util.arrayFromTransformMatrix(transformNode1)
    Trans_ni = np.linalg.inv(trans)
    Trans1_ni = np.linalg.inv(trans1)
    JGM = np.dot(np.dot(Trans_ni,Trans1_ni),zb)
    JieGu = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', name)
    JieGu.AddControlPoint(JGM[0][0], JGM[0][1], JGM[0][2])
    JieGu.AddControlPoint(JGM[1][0], JGM[1][1], JGM[1][2])
    JieGu.AddControlPoint(JGM[2][0], JGM[2][1], JGM[2][2])
    JieGu.SetAndObserveTransformNodeID(transformNode1.GetID())
  
  def PyQtGraph(self):
    import textwrap
    from PySide2.QtWidgets import QVBoxLayout
    import shiboken2
    from pyqtgraph.Qt import QtGui, QtCore
    import pyqtgraph as pg
    self.ui.PopupImage.setVisible(True)
    for i in range (0,len(self.ui.GraphImage.children())):
      a = self.ui.GraphImage.children()[-1]
      a.delete() 

    self.data1 = 0
    self.data2 = 0
    self.ui.GraphImage.setAutoFillBackground(True)
    viewLayout = qt.QVBoxLayout()
    self.ui.GraphImage.setLayout(viewLayout)
    pg.setConfigOption('background', '#454647')
    win = pg.GraphicsLayoutWidget(show=True)
    layout = shiboken2.wrapInstance(hash(viewLayout),QVBoxLayout)
    layout.addWidget(win)
    Y1 = np.array(self.pyqt_data_y1)
    X1 = np.array(self.pyqt_data_x)
    Y2 = np.array(self.pyqt_data_y2)
    X2 = np.array(self.pyqt_data_x)
    pg.setConfigOptions(antialias=True)
    self.p1 = win.addPlot()
    #self.p1.showGrid(True,True,0.1)#网格线
    #画线
    z1 = np.polyfit(X1,Y1,5) #拟合
    z2 = np.polyfit(X2,Y2,5)
    self.pp1 = np.poly1d(z1)#多项式显示
    self.pp2 = np.poly1d(z2)
    xx1 = np.linspace(-10,130,1000)
    yy1 = self.pp1(xx1)
    xx2 = np.linspace(-10,130,1000)
    yy2 = self.pp2(xx1)
    self.fill1=self.p1.plot(xx1,yy1,fillLevel=-0.3, brush=(50,50,200,100))
    self.fill2=self.p1.plot(xx2,yy2,fillLevel=-0.3, brush=(50,50,200,100))
    self.xian1 = self.p1.plot(xx1,yy1,pen="r")
    self.xian2 = self.p1.plot(xx2,yy2,pen="y")
    angle = self.p1.plot([0,0],[0,0],pen='g')

    self.currentX_line = pg.InfiniteLine(angle=90, movable=False, pen='g')
    self.p1.addItem(self.currentX_line, ignoreBounds=True)
    #显示图例
    legend = pg.LegendItem((20,5), offset=(10,5))
    legend.setParentItem(self.p1)
    legend.addItem(self.xian1, '内侧(mm)')
    legend2 = pg.LegendItem((20,5), offset=(10,16))
    legend2.setParentItem(self.p1)
    legend2.addItem(self.xian2, '外侧(mm)')
    legend3 = pg.LegendItem((20,5), offset=(10,27))
    legend3.setParentItem(self.p1)
    legend3.addItem(angle, '角度(°)')
    
    #Y轴
    ay = self.p1.getAxis('left')
    ticks1 = [-20,-10,-3,0,3,10,20]
    ay.setTicks([[(v, str(v)) for v in ticks1 ]])
    #X轴
    ax = self.p1.getAxis('bottom') 
    ticks = [-10,0,10,20,30,40,50,60,70,80,90,100,110,120,130]
    ax.setTicks([[(v, str(v)) for v in ticks ]])  
    #设置线
    #plot画线（x坐标[x1,x2]，y坐标[y1,y2]，线颜色，线宽度，线样式）
    self.l1=self.p1.plot([0,0], [-10,10], pen = '#c8c8c8', linewidth=1,linestyle='-') #90°线
    self.l2=self.p1.plot([90,90], [-10,10], pen = '#c8c8c8', linewidth=1,linestyle='-') #0°线
    l3=self.p1.plot([-10,130], [10,10], pen = '#c8c8c8', linewidth=1,linestyle='-') #10mm线
    l4=self.p1.plot([-10,130], [-10,-10], pen = '#c8c8c8', linewidth=1,linestyle='-') #-10mm线
    #设置文本，anchor用来设置偏移位置
    #显示文本
    text1 = pg.TextItem("%0.1f mm"%(self.pp2(0)))
    self.p1.addItem(text1)
    text1.setPos(0, self.pp2(0)+3)
    text2 = pg.TextItem("%0.1f mm"%(self.pp2(90)))
    self.p1.addItem(text2)
    text2.setPos(90, self.pp2(90)+3)
    self.text3 = pg.TextItem("%0.1f mm"%(self.pp2(0)))
    self.text4 = pg.TextItem("%0.1f mm"%(self.pp2(0)))
    self.p1.addItem(self.text3)
    self.p1.addItem(self.text4)


    # 设置十字线并显示坐标
    label = pg.LabelItem(justify='right',row = 0,column = 1)
    win.addItem(label)
    self.vLine = pg.InfiniteLine(angle=90, movable=False)
    # hLine = pg.InfiniteLine(angle=0, movable=False)
    self.p1.addItem(self.vLine, ignoreBounds=True)
    # p1.addItem(hLine, ignoreBounds=True)

    self.num_of_curves=2
    Colors_Set = [(31, 119, 180), (174, 199, 232), (255, 127, 14), (255, 187, 120),(44, 160, 44), (152, 223, 138)]
    self.plotcurves = ["%d" % x for x in np.arange(self.num_of_curves)]
    self.curvePoints = ["%d" % x for x in np.arange(self.num_of_curves)]
    self.texts = ["%d" % x for x in np.arange(self.num_of_curves)]
    self.arrows = ["%d" % x for x in np.arange(self.num_of_curves)]
    self.datas = ["%d" % x for x in np.arange(self.num_of_curves)]
 
    for k in range (self.num_of_curves):
      self.plotcurves[k] = pg.PlotCurveItem()

    self.datas[0] = [xx1,yy1]
    self.datas[1] = [xx2,yy2]
    for j in range(self.num_of_curves):
      pen = pg.mkPen(color=Colors_Set[j+2],width=1)
      self.plotcurves[j].setData(x=self.datas[j][0],y=self.datas[j][1],pen=pen,clickable=True)

    for k in range (self.num_of_curves):
      self.p1.addItem(self.plotcurves[k])
      self.curvePoints[k] = pg.CurvePoint(self.plotcurves[k])
      self.p1.addItem(self.curvePoints[k])
      self.texts[k] = pg.TextItem(str(k),color=Colors_Set[k+2],anchor=(0.5,0))
      # Here we require setParent on the TextItem
      self.texts[k].setParentItem(self.curvePoints[k])   
  
    #proxy = pg.SignalProxy(self.p1.scene().sigMouseMoved, rateLimit=60, slot=self.mouseMoved)
    self.p1.scene().sigMouseMoved.connect(self.mouseMoved)
    self.p1.hoverEvent = self.hoverEvent
    win.show()
    self.ui.GraphImage.show()

  def hoverEvent(self,event):
    if event.isExit():
      self.vLine.hide()
      for i in range(2):
        self.texts[i].hide()
        # self.arrows[i].hide()
        self.curvePoints[i].hide()
    else:
      self.vLine.show()
      for i in range(2):
        self.texts[i].show()
        # self.arrows[i].show()
        self.curvePoints[i].show()

  def mouseMoved(self,evt):
    import pyqtgraph as pg
    pos = self.p1.mapFromScene(evt)
    #pos = evt[0]
    majors = ["内侧","外侧"]
    if self.p1.sceneBoundingRect().contains(pos):
      mousePoint = self.p1.vb.mapSceneToView(pos)
      index = int(mousePoint.x())
      if index >-10 and index < 130:
        self.dataPosX = mousePoint.x()
        for m in range (self.num_of_curves):
          self.curvePoints[m].setPos((float(index)+10)/140)              
          T = majors[m] # Get the respective text string of the Legend
          if m:
            self.texts[m].setText(str(T) + ":[%0.1f,%0.1f]" % (self.dataPosX, self.pp2(self.dataPosX)))
          else:
            self.texts[m].setText(str(T)+":[%0.1f,%0.1f]"%(self.dataPosX,self.pp1(self.dataPosX)))
          self.arrows[m] = pg.ArrowItem(angle=-45)
          self.arrows[m].setParentItem(self.curvePoints[m])

      self.vLine.setPos(mousePoint.x())

  def updatePyqtgraph(self):
    Y1 = np.array(self.pyqt_data_y1)
    X1 = np.array(self.pyqt_data_x)
    Y2 = np.array(self.pyqt_data_y2)
    X2 = np.array(self.pyqt_data_x)
    z1 = np.polyfit(X1, Y1, 5)  # 拟合
    z2 = np.polyfit(X2, Y2, 5)
    self.pp1 = np.poly1d(z1)  # 多项式显示
    self.pp2 = np.poly1d(z2)
    xx1 = np.linspace(-10, 130, 1000)
    yy1 = self.pp1(xx1)
    yy2 = self.pp2(xx1)
    self.datas[0] = [xx1, yy1]
    self.datas[1] = [xx1, yy2]
    self.xian1.setData(xx1, yy1)
    self.xian2.setData(xx1, yy2)
    self.fill1.setData(xx1, yy1)
    self.fill2.setData(xx1, yy2)

    for j in range(2):
      self.plotcurves[j].setData(x=self.datas[j][0], y=self.datas[j][1])
    self.l1.setData([0, 0], [self.pp1(0), self.pp2(0)])
    self.l2.setData([90, 90], [self.pp1(90), self.pp2(90)])

    #添加当前角度
    self.currentX_line.setPos(self.currentX)
    self.text3.setText("%0.1f mm"%(self.pp1(self.currentX)))
    self.text4.setText("%0.1f mm"%(self.pp2(self.currentX)))
    self.text3.setPos(self.currentX, self.pp1(self.currentX)+1.5)
    self.text4.setPos(self.currentX, self.pp2(self.currentX)+1.5)

    if not self.ui.Graph.visible :
      self.ReturnGraph()
    line1 = slicer.util.getNode('InSide').GetLineLengthWorld()
    line2 = slicer.util.getNode('OutSide').GetLineLengthWorld()
    self.JieGuJianXi.SetNthControlPointLabel(0,str(round(line1,2))+'mm')
    self.JieGuJianXi.SetNthControlPointLabel(1,str(round(line2,2))+'mm')
    s1 = 1
    s2 = 1
    s3 = 0  
    s4 = 1
    s5 = round(line1,2)
    s6 = round(line1,2)
    s7 = 0
    s8 = '0@\n'
    self.ser1.write(f'{s1},{s2},{s3},{s4},{s5},{s6},{s7},{s8}'.encode())
    print('已发送', f'{s1},{s2},{s3},{s4},{s5},{s6},{s7},{s8}')
    time.sleep(0.15)
    

  def PopupGraph(self):
    if self.ui.PopupImage.text == '弹出图像':
      self.ui.Graph.setParent(None)
      self.ui.Graph.setWindowFlags(qt.Qt.WindowStaysOnTopHint)#置于顶层
      y = slicer.app.layoutManager().threeDWidget('View2').height
      x = slicer.app.layoutManager().threeDWidget('View1').width
      xx = slicer.app.layoutManager().threeDWidget('View3').width
      yy = slicer.app.layoutManager().threeDWidget('View3').height
      self.ui.Graph.setGeometry(x+460,y+20,xx,yy)
      self.ui.Graph.show()
      self.ui.PopupImage.setText('恢复图像')
    else:
      self.ReturnGraph()
      self.ui.PopupImage.setText('弹出图像')

  def ReturnGraph(self):
    self.ui.Graph.setParent(self.ui.PopupGraph)
    layout = self.ui.PopupGraph.layout()
    layout.addWidget(self.ui.Graph)
    self.ui.Graph.show()

  def DrawGraph(self):
    pass
  def ClearGraph(self):
    self.p1.clear()    

  def RecordGraph(self):
    pass

  #实时显示内外侧截骨间隙
  def onJieGuJianXi(self):
    self.JieGuJianXi = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode', 'JGJX')
    self.JieGuJianXi.AddControlPoint(-50, 10, 0)
    self.JieGuJianXi.AddControlPoint(50, 10, 0)
    self.JieGuJianXi.GetDisplayNode().PointLabelsVisibilityOn()
    self.JieGuJianXi.GetDisplayNode().SetGlyphScale(0)
    self.JieGuJianXi.GetDisplayNode().SetTextScale(3)
    Femur_ZJtAxis = slicer.util.getNode('变换_R')
    self.JieGuJianXi.SetAndObserveTransformNodeID(Femur_ZJtAxis.GetID())
    WaiCeLine = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsLineNode','OutSide')
    WaiCeLine.AddControlPoint(0, 0, 0)
    WaiCeLine.AddControlPoint(10, 0, 0)
    WaiCeLine.GetDisplayNode().SetPropertiesLabelVisibility(0)
    WaiCeLine.SetNthControlPointLocked(0,True)
    WaiCeLine.SetNthControlPointLocked(1,True)
    NeiCeLine = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsLineNode','InSide')
    NeiCeLine.AddControlPoint(0, 0, 0)
    NeiCeLine.AddControlPoint(10, 0, 0)
    NeiCeLine.GetDisplayNode().SetPropertiesLabelVisibility(0)
    NeiCeLine.SetNthControlPointLocked(0,True)
    NeiCeLine.SetNthControlPointLocked(1,True)

    

  def XuHao(self):
    Number = ["⑳","⑲","⑱","⑰","⑯","⑮","⑭","⑬","⑫","⑪","⑩","⑨","⑧","⑦","⑥","⑤","④","③","②","①"]

  def onstartDJ(self):

    self.ser1 = serial.Serial(  # 下面这些参数根据情况修改
      port='COM5',
      baudrate=115200,
      timeout=0.22,
      write_timeout=0.1
    )
    print("串口状态:" + str(self.ser1.is_open))

    self._received_thread_ = threading.Thread(target=self.__recv_func__, args=(self,))
    # print("thread")
    self._is_running_ = True
    # print("thread1")
    #self._received_thread_.setDaemon(True)
    # print("thread2")
    self._received_thread_.setName("SerialPortRecvThread")
    self._received_thread_.start()
    print('开始监听信号')

  def __recv_func__(self, ser1=None):
    while self.ser1.isOpen():
      #print('刷新:',self.ser1.in_waiting)
      try:
        if (self.ser1.in_waiting > 0):
          buffer = self.ser1.readline().decode('utf-8')
          self.handleCalc_do(buffer)
          print('接收到的', buffer)
          #s1, s2, s3, s4, s5, s6, s7, s8 = int(buf_list[0]), int(buf_list[1]), int(buf_list[2]), buf_list[3], int(buf_list[4]), int(buf_list[5]), buf_list[6], buf_list[7]
          # if s1 == 1:
          #   if s2 == 0:
          #     if s3<11:
          #         print('onSelect1')
          #         self.onSelect1()
          #     else:
          #         print('onNextArea')
          #         self.onNextArea()
          #   else:
          #     if s3<7:
          #       print('onSelect1')
          #       self.onSelect1()
          #     else:
          #       print('onNextArea')
          #       self.onNextArea()
        elif (self.ser1.in_waiting <= 0):
          time.sleep(0.05)

      except:
        print('传输数据存在异常')
      
      time.sleep(0.1)


  def handleCalc_do(self,s):
      buf_list = s.split('@')
      for i in range(len(buf_list)):
          buf_list1=buf_list[i].split(',')
          if len(buf_list1)==8:
              self.handleData(buf_list1)

  def handleData(self,buf):
      if int(buf[0]) == 1:
        if int(buf[1]) == 0:
          if int(buf[2])<12:
              print('onSelect1')
              self.onSelect1('xiaopingmu')
          else:
              print('onNextArea')
              self.onNextArea('xiaopingmu')
        else:
          if int(buf[2])<8:
            print('onSelect1')
            self.onSelect1('xiaopingmu')
          else:
            print('onNextArea')
            self.onNextArea('xiaopingmu')



class ReSizeEvent(qt.QObject):
  def eventFilter(self, object, event):
    if(event.type() == qt.QEvent.Resize):
      NoImage = slicer.modules.NoImageWidget
      if NoImage.currentModel == 4 and NoImage.ui.Adjustment.checked:
        NoImage.FemurCameraTip()
      elif NoImage.currentModel == 5 and NoImage.ui.Adjustment2.checked:
        NoImage.SetTibiaCameraTip()
      elif NoImage.ui.ForceLine.checked and NoImage.ui.ForceLine2.checked:
        NoImage.ForceLabel1.setGeometry(NoImage.view1.contentsRect().width()/2-50,5,200,40)
        NoImage.ForceLabel2.setGeometry(NoImage.view2.contentsRect().width()/2-50,5,200,40)
      return False
    return False