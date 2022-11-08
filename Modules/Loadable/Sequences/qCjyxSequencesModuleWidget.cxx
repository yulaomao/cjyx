/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QCheckBox>
#include <QDebug>
#include <QListWidgetItem>
#include <QtPlugin>

// Cjyx includes
#include "qDMMLSequenceBrowserToolBar.h"
#include "qCjyxApplication.h"
#include "qCjyxLayoutManager.h"
#include "qCjyxSequencesModuleWidget.h"
#include "qCjyxSequencesModule.h"
#include "ui_qCjyxSequencesModuleWidget.h"

// DMML includes
#include "vtkDMMLCrosshairNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLScalarVolumeNode.h"
#include "vtkDMMLTransformNode.h"

// Sequence includes
#include "vtkCjyxSequencesLogic.h"
#include "vtkDMMLSequenceNode.h"
#include "vtkDMMLSequenceBrowserNode.h"

// VTK includes
#include <vtkAbstractTransform.h>
#include <vtkAxis.h>
#include <vtkChartXY.h>
#include <vtkFloatArray.h>
#include <vtkGeneralTransform.h>
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPlot.h>
#include <vtkTable.h>
#include <vtkWeakPointer.h>


enum
{
  SYNCH_NODES_NAME_COLUMN=0,
  SYNCH_NODES_PROXY_COLUMN,
  SYNCH_NODES_PLAYBACK_COLUMN,
  SYNCH_NODES_RECORDING_COLUMN,
  SYNCH_NODES_OVERWRITE_PROXY_NAME_COLUMN,
  SYNCH_NODES_SAVE_CHANGES_COLUMN,
  SYNCH_NODES_NUMBER_OF_COLUMNS // this must be the last line in this enum
};

#define FROM_STD_STRING_SAFE(unsafeString) QString::fromStdString( unsafeString==nullptr?"":unsafeString )
#define FROM_ATTRIBUTE_SAFE(unsafeString) ( unsafeString==nullptr?"":unsafeString )

enum
{
//  DATA_NODE_VIS_COLUMN=0,
  DATA_NODE_VALUE_COLUMN,
  DATA_NODE_NAME_COLUMN,
  DATA_NODE_NUMBER_OF_COLUMNS // this must be the last line in this enum
};

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_ExtensionTemplate
class qCjyxSequencesModuleWidgetPrivate: public Ui_qCjyxSequencesModuleWidget
{
  Q_DECLARE_PUBLIC( qCjyxSequencesModuleWidget );

protected:
  qCjyxSequencesModuleWidget* const q_ptr;
public:
  qCjyxSequencesModuleWidgetPrivate( qCjyxSequencesModuleWidget& object );
  ~qCjyxSequencesModuleWidgetPrivate();

  vtkCjyxSequencesLogic* logic() const;

  void init();
  void resetInteractiveCharting();
  void updateInteractiveCharting();
  void setAndObserveCrosshairNode();

  qDMMLSequenceBrowserToolBar* toolBar();

  /// Using this flag prevents overriding the parameter set node contents when the
  ///   QDMMLCombobox selects the first instance of the specified node type when initializing
  bool ModuleWindowInitialized;

  vtkWeakPointer<vtkDMMLSequenceBrowserNode> ActiveBrowserNode;

  vtkChartXY* ChartXY;
  vtkTable* ChartTable;
  vtkFloatArray* ArrayX;
  vtkFloatArray* ArrayY1;
  vtkFloatArray* ArrayY2;
  vtkFloatArray* ArrayY3;

  /// Get a list of MLRML nodes that are in the scene but not added to the sequences data node at the chosen index value
  void GetDataNodeCandidates(vtkCollection* foundNodes, vtkDMMLSequenceNode* sequenceNode);

  vtkWeakPointer<vtkDMMLSequenceNode> ActiveSequenceNode;
  QString DataNodeCandidatesClassName; // data node class name that was used for populating the candidate node list

  vtkWeakPointer<vtkDMMLCrosshairNode> CrosshairNode;

  QStringList SupportedProxyNodeTypes;
};


//-----------------------------------------------------------------------------
// qCjyxSequencesModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qCjyxSequencesModuleWidgetPrivate::qCjyxSequencesModuleWidgetPrivate( qCjyxSequencesModuleWidget& object )
: q_ptr(&object)
, ModuleWindowInitialized(false)
, ChartXY(0)
, ChartTable(0)
, ArrayX(0)
, ArrayY1(0)
, ArrayY2(0)
, ArrayY3(0)
, DataNodeCandidatesClassName("(uninitialized)") // this will force an initial update
{
}

//-----------------------------------------------------------------------------
qCjyxSequencesModuleWidgetPrivate::~qCjyxSequencesModuleWidgetPrivate()
{
  if (this->ChartTable)
    {
    this->ChartTable->Delete();
    }
  if (this->ArrayX)
    {
    this->ArrayX->Delete();
    }
  if (this->ArrayY1)
    {
    this->ArrayY1->Delete();
    }
  if (this->ArrayY2)
    {
    this->ArrayY2->Delete();
    }
  if (this->ArrayY3)
    {
    this->ArrayY3->Delete();
    }
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidgetPrivate::setAndObserveCrosshairNode()
{
  Q_Q(qCjyxSequencesModuleWidget);

  vtkDMMLCrosshairNode* crosshairNode = 0;
  if (q->dmmlScene())
    {
    crosshairNode = vtkDMMLCrosshairNode::SafeDownCast(q->dmmlScene()->GetNthNodeByClass(0, "vtkDMMLCrosshairNode"));
    }

  q->qvtkReconnect(this->CrosshairNode.GetPointer(), crosshairNode,
    vtkDMMLCrosshairNode::CursorPositionModifiedEvent,
    q, SLOT(updateChart()));
  this->CrosshairNode = crosshairNode;
}

//-----------------------------------------------------------------------------
vtkCjyxSequencesLogic* qCjyxSequencesModuleWidgetPrivate::logic() const
{
  Q_Q( const qCjyxSequencesModuleWidget );
  return vtkCjyxSequencesLogic::SafeDownCast( q->logic() );
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidgetPrivate::GetDataNodeCandidates(vtkCollection* foundNodes, vtkDMMLSequenceNode* sequenceNode)
{
  Q_Q( const qCjyxSequencesModuleWidget );

  if (foundNodes==nullptr)
    {
    qCritical() << "GetDataNodeCandidates failed, invalid output collection";
    return;
    }
  foundNodes->RemoveAllItems();
  if (sequenceNode==nullptr)
    {
    qCritical() << "GetDataNodeCandidates failed, invalid sequence node";
    return;
    }

  std::string dataNodeClassName=sequenceNode->GetDataNodeClassName();

  for ( int i = 0; i < sequenceNode->GetScene()->GetNumberOfNodes(); i++ )
    {
    vtkDMMLNode* currentNode = vtkDMMLNode::SafeDownCast( sequenceNode->GetScene()->GetNthNode( i ) );
    if (currentNode->GetHideFromEditors())
      {
      // don't show hidden nodes, they would clutter the view
      continue;
      }
    if (currentNode->GetSingletonTag()!=nullptr)
      {
      // don't allow adding singletons (mainly because we can only store one singleton node in a scene, so we couldn't store it)
      continue;
      }
    if (currentNode==sequenceNode)
      {
      // don't allow adding itself as data node
      continue;
      }
    if (!dataNodeClassName.empty())
      {
      if (dataNodeClassName.compare(currentNode->GetClassName())!=0)
        {
        // class is not compatible with elements already in the sequence, don't show it
        continue;
        }
      }
    foundNodes->AddItem( currentNode );
    }
}

//-----------------------------------------------------------------------------
qDMMLSequenceBrowserToolBar* qCjyxSequencesModuleWidgetPrivate::toolBar()
{
  Q_Q(const qCjyxSequencesModuleWidget);
  qCjyxSequencesModule* module = dynamic_cast<qCjyxSequencesModule*>(q->module());
  if (!module)
    {
    qWarning("qCjyxSequencesModuleWidget::toolBar failed: module is not set");
    return nullptr;
    }
  return module->toolBar();
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidgetPrivate::init()
{
  this->ChartXY = this->ChartView_iCharting->chart();
  this->ChartTable = vtkTable::New();
  this->ArrayX = vtkFloatArray::New();
  this->ArrayY1 = vtkFloatArray::New();
  this->ArrayY2 = vtkFloatArray::New();
  this->ArrayY3 = vtkFloatArray::New();
  this->ArrayX->SetName("X axis");
  this->ArrayY1->SetName("Y1 axis");
  this->ArrayY2->SetName("Y2 axis");
  this->ArrayY3->SetName("Y3 axis");
  this->ChartTable->AddColumn(this->ArrayX);
  this->ChartTable->AddColumn(this->ArrayY1);
  this->ChartTable->AddColumn(this->ArrayY2);
  this->ChartTable->AddColumn(this->ArrayY3);

  this->resetInteractiveCharting();
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidgetPrivate::resetInteractiveCharting()
{
  this->ChartXY->RemovePlot(0);
  this->ChartXY->RemovePlot(0);
  this->ChartXY->RemovePlot(0);
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidgetPrivate::updateInteractiveCharting()
{
  Q_Q(qCjyxSequencesModuleWidget);

  if (this->CrosshairNode.GetPointer()==nullptr)
    {
    qWarning() << "qCjyxSequencesModuleWidgetPrivate::updateInteractiveCharting failed: crosshair node is not available";
    resetInteractiveCharting();
    return;
    }
  vtkDMMLSequenceNode* sequenceNode = this->ActiveBrowserNode ? this->ActiveBrowserNode->GetMasterSequenceNode() : nullptr;
  if (sequenceNode==nullptr)
    {
    resetInteractiveCharting();
    return;
    }
  double croshairPosition_RAS[4]={0,0,0,1}; // homogeneous coordinate to allow transform by matrix multiplication
  bool validPosition = this->CrosshairNode->GetCursorPositionRAS(croshairPosition_RAS);
  if (!validPosition)
    {
    resetInteractiveCharting();
    return;
    }
  vtkDMMLNode* proxyNode = this->ActiveBrowserNode->GetProxyNode(sequenceNode);
  vtkDMMLTransformableNode* transformableProxyNode = vtkDMMLTransformableNode::SafeDownCast(proxyNode);

  int numberOfDataNodes = sequenceNode->GetNumberOfDataNodes();
  this->ChartTable->SetNumberOfRows(numberOfDataNodes);

  vtkDMMLScalarVolumeNode *vNode = vtkDMMLScalarVolumeNode::SafeDownCast(sequenceNode->GetNthDataNode(0));
  if (vNode)
    {
    int numOfScalarComponents = 0;
    numOfScalarComponents = vNode->GetImageData()->GetNumberOfScalarComponents();
    if (numOfScalarComponents > 3)
      {
      return;
      }
    vtkNew<vtkGeneralTransform> worldTransform;
    worldTransform->Identity();
    vtkDMMLTransformNode *transformNode = transformableProxyNode ? transformableProxyNode->GetParentTransformNode() : nullptr;
    if ( transformNode )
      {
      transformNode->GetTransformFromWorld(worldTransform.GetPointer());
      }

    int numberOfValidPoints = 0;
    for (int i = 0; i<numberOfDataNodes; i++)
      {
      vNode = vtkDMMLScalarVolumeNode::SafeDownCast(sequenceNode->GetNthDataNode(i));
      this->ChartTable->SetValue(i, 0, i);

      vtkNew<vtkGeneralTransform> worldToIjkTransform;
      worldToIjkTransform->PostMultiply();
      worldToIjkTransform->Identity();
      vtkNew<vtkMatrix4x4> rasToIjkMatrix;
      vNode->GetRASToIJKMatrix(rasToIjkMatrix.GetPointer());
      worldToIjkTransform->Concatenate(rasToIjkMatrix.GetPointer());
      worldToIjkTransform->Concatenate(worldTransform.GetPointer());

      double *crosshairPositionDouble_IJK = worldToIjkTransform->TransformDoublePoint(croshairPosition_RAS);
      int croshairPosition_IJK[3]={vtkMath::Round(crosshairPositionDouble_IJK[0]),
        vtkMath::Round(crosshairPositionDouble_IJK[1]), vtkMath::Round(crosshairPositionDouble_IJK[2])};
      int* imageExtent = vNode->GetImageData()->GetExtent();
      bool isCrosshairInsideImage = imageExtent[0]<=croshairPosition_IJK[0] && croshairPosition_IJK[0]<=imageExtent[1]
          && imageExtent[2]<=croshairPosition_IJK[1] && croshairPosition_IJK[1]<=imageExtent[3]
          && imageExtent[4]<=croshairPosition_IJK[2] && croshairPosition_IJK[2]<=imageExtent[5];
      if (isCrosshairInsideImage)
        {
        numberOfValidPoints++;
        }
      for (int c = 0; c<numOfScalarComponents; c++)
        {
        double val = isCrosshairInsideImage ? vNode->GetImageData()->GetScalarComponentAsDouble(croshairPosition_IJK[0],
          croshairPosition_IJK[1], croshairPosition_IJK[2], c) : 0;
        this->ChartTable->SetValue(i, c+1, val);
        }
      }
    //this->ChartTable->Update();
    this->ChartXY->RemovePlot(0);
    this->ChartXY->RemovePlot(0);
    this->ChartXY->RemovePlot(0);

    if (numberOfValidPoints>0)
      {
      this->ChartXY->GetAxis(0)->SetTitle("Signal Intensity");
      this->ChartXY->GetAxis(1)->SetTitle("Time");
      for (int c = 0; c<numOfScalarComponents; c++)
        {
        vtkPlot* line = this->ChartXY->AddPlot(vtkChart::LINE);
        line->SetInputData(this->ChartTable, 0, c+1);
        //line->SetColor(255,0,0,255);
        }
      }
    }

  vtkDMMLTransformNode *tNode = vtkDMMLTransformNode::SafeDownCast(sequenceNode->GetNthDataNode(0));
  if (tNode)
    {
    for (int i = 0; i<numberOfDataNodes; i++)
      {
      tNode = vtkDMMLTransformNode::SafeDownCast(sequenceNode->GetNthDataNode(i));
      vtkAbstractTransform* trans2Parent = tNode->GetTransformToParent();

      double* transformedcroshairPosition_RAS = trans2Parent->TransformDoublePoint(croshairPosition_RAS);

      this->ChartTable->SetValue(i, 0, i);
      this->ChartTable->SetValue(i, 1, transformedcroshairPosition_RAS[0]-croshairPosition_RAS[0]);
      this->ChartTable->SetValue(i, 2, transformedcroshairPosition_RAS[1]-croshairPosition_RAS[1]);
      this->ChartTable->SetValue(i, 3, transformedcroshairPosition_RAS[2]-croshairPosition_RAS[2]);
      }
    //this->ChartTable->Update();
    this->ChartXY->RemovePlot(0);
    this->ChartXY->RemovePlot(0);
    this->ChartXY->RemovePlot(0);

    this->ChartXY->GetAxis(0)->SetTitle("Displacement");
    this->ChartXY->GetAxis(1)->SetTitle("Time");
    vtkPlot* line_X = this->ChartXY->AddPlot(vtkChart::LINE);
    vtkPlot* line_Y = this->ChartXY->AddPlot(vtkChart::LINE);
    vtkPlot* line_Z = this->ChartXY->AddPlot(vtkChart::LINE);

    line_X->SetInputData(this->ChartTable, 0, 1);
    line_Y->SetInputData(this->ChartTable, 0, 2);
    line_Z->SetInputData(this->ChartTable, 0, 3);

    line_X->SetColor(255,0,0,255);
    line_Y->SetColor(0,255,0,255);
    line_Z->SetColor(0,0,255,255);
    }
}


//-----------------------------------------------------------------------------
// qCjyxSequencesModuleWidget methods

//-----------------------------------------------------------------------------
qCjyxSequencesModuleWidget::qCjyxSequencesModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qCjyxSequencesModuleWidgetPrivate( *this ) )
{
}

//-----------------------------------------------------------------------------
qCjyxSequencesModuleWidget::~qCjyxSequencesModuleWidget() = default;

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::setup ()
{
  Q_D(qCjyxSequencesModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  d->init();

  if (d->ComboBox_IndexType->count() == 0)
    {
    for (int indexType=0; indexType<vtkDMMLSequenceNode::NumberOfIndexTypes; indexType++)
      {
      d->ComboBox_IndexType->addItem(vtkDMMLSequenceNode::GetIndexTypeAsString(indexType).c_str());
      }
    }

  d->TableWidget_DataNodes->setColumnWidth( DATA_NODE_VALUE_COLUMN, 30 );
  d->TableWidget_DataNodes->setColumnWidth( DATA_NODE_NAME_COLUMN, 100 );

  d->PushButton_AddDataNode->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowLeft));
  d->PushButton_RemoveDataNode->setIcon(QIcon(":/Icons/DataNodeDelete.png"));

  d->ExpandButton_DataNodes->setChecked(false);

  d->pushButton_AddSequenceNode->setIcon(QIcon(":/Icons/Add.png"));
  d->pushButton_RemoveSequenceNode->setIcon(QIcon(":/Icons/Remove.png"));

  QHeaderView* tableWidget_SynchronizedSequenceNodes_HeaderView = d->tableWidget_SynchronizedSequenceNodes->horizontalHeader();

  tableWidget_SynchronizedSequenceNodes_HeaderView-> setSectionResizeMode(SYNCH_NODES_NAME_COLUMN, QHeaderView::Interactive);
  tableWidget_SynchronizedSequenceNodes_HeaderView-> setSectionResizeMode(SYNCH_NODES_PROXY_COLUMN, QHeaderView::Interactive);
  tableWidget_SynchronizedSequenceNodes_HeaderView-> setSectionResizeMode(SYNCH_NODES_PLAYBACK_COLUMN, QHeaderView::ResizeToContents);
  tableWidget_SynchronizedSequenceNodes_HeaderView-> setSectionResizeMode(SYNCH_NODES_RECORDING_COLUMN, QHeaderView::ResizeToContents);
  tableWidget_SynchronizedSequenceNodes_HeaderView-> setSectionResizeMode(SYNCH_NODES_OVERWRITE_PROXY_NAME_COLUMN, QHeaderView::ResizeToContents);
  tableWidget_SynchronizedSequenceNodes_HeaderView-> setSectionResizeMode(SYNCH_NODES_SAVE_CHANGES_COLUMN, QHeaderView::ResizeToContents);

  tableWidget_SynchronizedSequenceNodes_HeaderView->setStretchLastSection(false);

  d->tableWidget_SynchronizedSequenceNodes->setColumnWidth(SYNCH_NODES_NAME_COLUMN, 200);
  d->tableWidget_SynchronizedSequenceNodes->setColumnWidth(SYNCH_NODES_PROXY_COLUMN, 200);

}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::enter()
{
  Q_D(qCjyxSequencesModuleWidget);

  if (this->dmmlScene() != 0)
    {
    // set up dmml scene observations so that the GUI gets updated
    this->qvtkConnect(this->dmmlScene(), vtkDMMLScene::NodeAddedEvent,
      this, SLOT(onNodeAddedEvent(vtkObject*, vtkObject*)));
    this->qvtkConnect(this->dmmlScene(), vtkDMMLScene::NodeRemovedEvent,
      this, SLOT(onNodeRemovedEvent(vtkObject*, vtkObject*)));
    this->qvtkConnect(this->dmmlScene(), vtkDMMLScene::EndImportEvent,
      this, SLOT(onDMMLSceneEndImportEvent()));
    this->qvtkConnect(this->dmmlScene(), vtkDMMLScene::EndBatchProcessEvent,
      this, SLOT(onDMMLSceneEndBatchProcessEvent()));
    this->qvtkConnect(this->dmmlScene(), vtkDMMLScene::EndCloseEvent,
      this, SLOT(onDMMLSceneEndCloseEvent()));
    this->qvtkConnect(this->dmmlScene(), vtkDMMLScene::EndRestoreEvent,
      this, SLOT(onDMMLSceneEndRestoreEvent()));
    this->setActiveSequenceNode(vtkDMMLSequenceNode::SafeDownCast(d->DMMLNodeComboBox_Sequence->currentNode()));
    this->updateCandidateNodesWidgetFromDMML(true);

    if (!d->ModuleWindowInitialized)
      {
      // Connect events after the module is entered to make sure that initial events that are triggered when scene is set
      // are not mistaken for user clicks.
      // Edit
      connect(d->DMMLNodeComboBox_Sequence, SIGNAL(currentNodeChanged(vtkDMMLNode*)), this, SLOT(onSequenceNodeSelectionChanged()));
      connect(d->LineEdit_IndexName, SIGNAL(textEdited(const QString&)), this, SLOT(onIndexNameEdited()));
      connect(d->LineEdit_IndexUnit, SIGNAL(textEdited(const QString&)), this, SLOT(onIndexUnitEdited()));
      connect(d->ComboBox_IndexType, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onIndexTypeEdited(QString)));
      //connect( d->TableWidget_DataNodes, SIGNAL( currentCellChanged( int, int, int, int ) ), this, SLOT( onDataNodeChanged() ) );
      connect(d->TableWidget_DataNodes, SIGNAL(cellChanged(int, int)), this, SLOT(onDataNodeEdited(int, int)));
      connect(d->PushButton_AddDataNode, SIGNAL(clicked()), this, SLOT(onAddDataNodeButtonClicked()));
      connect(d->PushButton_RemoveDataNode, SIGNAL(clicked()), this, SLOT(onRemoveDataNodeButtonClicked()));
      // Browse
      connect(d->DMMLNodeComboBox_ActiveBrowser, SIGNAL(currentNodeChanged(vtkDMMLNode*)), this, SLOT(activeBrowserNodeChanged(vtkDMMLNode*)));
      connect(d->DMMLNodeComboBox_MasterSequence, SIGNAL(currentNodeChanged(vtkDMMLNode*)), this, SLOT(sequenceNodeChanged(vtkDMMLNode*)));
      connect(d->checkBox_PlaybackItemSkippingEnabled, SIGNAL(toggled(bool)), this, SLOT(playbackItemSkippingEnabledChanged(bool)));
      connect(d->checkBox_RecordMasterOnly, SIGNAL(toggled(bool)), this, SLOT(recordMasterOnlyChanged(bool)));
      connect(d->comboBox_RecordingSamplingMode, SIGNAL(currentIndexChanged(int)), this, SLOT(recordingSamplingModeChanged(int)));
      connect(d->comboBox_IndexDisplayMode, SIGNAL(currentIndexChanged(int)), this, SLOT(indexDisplayModeChanged(int)));
      connect(d->lineEdit_IndexDisplayFormat, SIGNAL(textEdited(const QString)), this, SLOT(indexDisplayFormatChanged(const QString)));
      connect(d->pushButton_AddSequenceNode, SIGNAL(clicked()), this, SLOT(onAddSequenceNodeButtonClicked()));
      connect(d->pushButton_RemoveSequenceNode, SIGNAL(clicked()), this, SLOT(onRemoveSequenceNodesButtonClicked()));
      // Toolbar
      qDMMLSequenceBrowserToolBar* toolBar = d->toolBar();
      if (toolBar)
        {
        connect(toolBar, SIGNAL(activeBrowserNodeChanged(vtkDMMLNode*)), this, SLOT(activeBrowserNodeChanged(vtkDMMLNode*)));
        }
      d->ModuleWindowInitialized = true;
      }

    // For the user's convenience, create a browser node by default, when entering to the module and no browser node exists in the scene yet
    vtkDMMLNode* node = this->dmmlScene()->GetNthNodeByClass(0, "vtkDMMLSequenceBrowserNode");
    if (node == nullptr)
      {
      vtkSmartPointer<vtkDMMLSequenceBrowserNode> newBrowserNode = vtkSmartPointer<vtkDMMLSequenceBrowserNode>::New();
      this->dmmlScene()->AddNode(newBrowserNode);
      this->activeBrowserNodeChanged(newBrowserNode);
      }
    else
      {
      this->activeBrowserNodeChanged(d->DMMLNodeComboBox_ActiveBrowser->currentNode());
      }
    }
  else
    {
    qCritical() << "Entering the Sequences module failed, scene is invalid";
    }

  d->setAndObserveCrosshairNode();

  this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::exit()
{
  Q_D(qCjyxSequencesModuleWidget);
  this->Superclass::exit();

  // remove dmml scene observations, don't need to update the GUI while the
  // module is not showing
  this->qvtkDisconnectAll();

  d->ActiveSequenceNode = nullptr;
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::setDMMLScene(vtkDMMLScene* scene)
{
  Q_D(qCjyxSequencesModuleWidget);

  if (this->dmmlScene() == scene)
    {
    return;
    }

  this->Superclass::setDMMLScene(scene);

  // Only those nodes can be proxy nodes that support content copy.
  // Retrieve that information from the scene now.
  d->SupportedProxyNodeTypes.clear();
  if (scene)
    {
    int numberOfRegisteredNodeClasses = scene->GetNumberOfRegisteredNodeClasses();
    for (int nodeClassIndex = 0; nodeClassIndex < numberOfRegisteredNodeClasses; ++nodeClassIndex)
      {
      vtkDMMLNode* registeredNode = scene->GetNthRegisteredNodeClass(nodeClassIndex);
      if (registeredNode->HasCopyContent())
        {
        d->SupportedProxyNodeTypes << registeredNode->GetClassName();
        }
      }
    }

  d->setAndObserveCrosshairNode();
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::onNodeAddedEvent(vtkObject* scene, vtkObject* node)
{
  Q_UNUSED(scene);
  Q_UNUSED(node);
  if (!this->dmmlScene() || this->dmmlScene()->IsBatchProcessing())
    {
    return;
    }
  this->updateCandidateNodesWidgetFromDMML(true);
  this->refreshSynchronizedSequenceNodesTable();
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::onNodeRemovedEvent(vtkObject* scene, vtkObject* node)
{
  Q_UNUSED(scene);
  Q_UNUSED(node);
  if (!this->dmmlScene() || this->dmmlScene()->IsBatchProcessing())
    {
    return;
    }
  this->updateCandidateNodesWidgetFromDMML(true);
  this->refreshSynchronizedSequenceNodesTable();
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::onDMMLSceneEndImportEvent()
{
  this->updateCandidateNodesWidgetFromDMML(true);
  this->refreshSynchronizedSequenceNodesTable();
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::onDMMLSceneEndRestoreEvent()
{
  this->updateCandidateNodesWidgetFromDMML(true);
  this->refreshSynchronizedSequenceNodesTable();
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::onDMMLSceneEndBatchProcessEvent()
{
  if (!this->dmmlScene())
    {
    return;
    }
  this->updateCandidateNodesWidgetFromDMML(true);
  this->refreshSynchronizedSequenceNodesTable();
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::onDMMLSceneEndCloseEvent()
{
  if (!this->dmmlScene() || this->dmmlScene()->IsBatchProcessing())
    {
    return;
    }
  this->updateCandidateNodesWidgetFromDMML(true);
  this->updateSequenceItemWidgetFromDMML();
  this->refreshSynchronizedSequenceNodesTable();
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::onSequenceNodeSelectionChanged()
{
  Q_D(qCjyxSequencesModuleWidget);
  d->LineEdit_NewDataNodeIndexValue->setText("0");
  this->setActiveSequenceNode(vtkDMMLSequenceNode::SafeDownCast(d->DMMLNodeComboBox_Sequence->currentNode()));
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::onSequenceNodeModified()
{
  Q_D(qCjyxSequencesModuleWidget);
  this->updateSequenceItemWidgetFromDMML();
  this->updateCandidateNodesWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::onIndexNameEdited()
{
  Q_D(qCjyxSequencesModuleWidget);

  vtkDMMLSequenceNode* currentSequence = vtkDMMLSequenceNode::SafeDownCast( d->DMMLNodeComboBox_Sequence->currentNode() );
  if ( currentSequence == nullptr )
    {
    return;
    }

  currentSequence->SetIndexName(d->LineEdit_IndexName->text().toLatin1().constData());
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::onIndexUnitEdited()
{
  Q_D(qCjyxSequencesModuleWidget);

  vtkDMMLSequenceNode* currentSequence = vtkDMMLSequenceNode::SafeDownCast( d->DMMLNodeComboBox_Sequence->currentNode() );
  if ( currentSequence == nullptr )
    {
    return;
    }

  currentSequence->SetIndexUnit( d->LineEdit_IndexUnit->text().toLatin1().constData() );
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::onIndexTypeEdited(QString indexTypeString)
{
  Q_D(qCjyxSequencesModuleWidget);

  vtkDMMLSequenceNode* currentSequence = vtkDMMLSequenceNode::SafeDownCast( d->DMMLNodeComboBox_Sequence->currentNode() );
  if ( currentSequence == nullptr )
    {
    return;
    }

  currentSequence->SetIndexTypeFromString( indexTypeString.toLatin1().constData() );
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::onDataNodeEdited( int row, int column )
{
  Q_D(qCjyxSequencesModuleWidget);

  // Ensure that the user is editing, not the index changed programmatically
  if ( d->TableWidget_DataNodes->currentRow() != row || d->TableWidget_DataNodes->currentColumn() != column )
    {
    return;
    }

  vtkDMMLSequenceNode* currentSequence = vtkDMMLSequenceNode::SafeDownCast( d->DMMLNodeComboBox_Sequence->currentNode() );
  if ( currentSequence == nullptr )
    {
    return;
    }

  std::string currentIndexValue = currentSequence->GetNthIndexValue( d->TableWidget_DataNodes->currentRow() );
  if ( currentIndexValue.empty() )
    {
    return;
    }

  vtkDMMLNode* currentDataNode = currentSequence->GetDataNodeAtValue(currentIndexValue.c_str() );
  if ( currentDataNode == nullptr )
    {
    return;
    }

  // Grab the text from the modified item
  QTableWidgetItem* qItem = d->TableWidget_DataNodes->item( row, column );
  QString qText = qItem->text();

  if ( column == DATA_NODE_VALUE_COLUMN )
    {
    currentSequence->UpdateIndexValue( currentIndexValue.c_str(), qText.toLatin1().constData() );
    }

  if ( column == DATA_NODE_NAME_COLUMN )
    {
    currentDataNode->SetName( qText.toLatin1().constData() );
    }

  this->updateSequenceItemWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::onAddDataNodeButtonClicked()
{
  Q_D(qCjyxSequencesModuleWidget);

  vtkDMMLSequenceNode* currentSequence = vtkDMMLSequenceNode::SafeDownCast( d->DMMLNodeComboBox_Sequence->currentNode() );
  if ( currentSequence == nullptr )
    {
    return;
    }

  std::string currentIndexValue = d->LineEdit_NewDataNodeIndexValue->text().toLatin1().constData();
  if ( currentIndexValue.empty() )
    {
    qCritical() << "Cannot add new data node, as Index value is not specified";
    return;
    }

  // Get the selected node
  QListWidgetItem* currentItem = d->ListWidget_CandidateDataNodes->currentItem();
  if (currentItem == 0)
    {
    qCritical() << "Cannot add new data node, as current data item selection is invalid";
    return;
    }

  QString currentCandidateNodeId = currentItem->data(Qt::UserRole).toString();
  vtkDMMLNode* currentCandidateNode = currentSequence->GetScene()->GetNodeByID(currentCandidateNodeId.toLatin1().constData());
  if (currentCandidateNode == 0)
    {
    qCritical() << "Cannot add new data node, as current data item is invalid";
    return;
    }
  int wasModified = currentSequence->StartModify();
  currentSequence->SetDataNodeAtValue(currentCandidateNode, currentIndexValue.c_str() );

  // Auto-increment the Index value in the new data textbox

  QString oldIndexValue=d->LineEdit_NewDataNodeIndexValue->text();
  bool isIndexValueNumeric=false;
  double oldIndexNumber = oldIndexValue.toDouble(&isIndexValueNumeric);
  if (isIndexValueNumeric)
    {
    double incrementValue=d->DoubleSpinBox_IndexValueAutoIncrement->value();
    QString newIndexValue=QString::number(oldIndexNumber+incrementValue);
    d->LineEdit_NewDataNodeIndexValue->setText(newIndexValue);
    }
  currentSequence->EndModify(wasModified);

  // Restore candidate node selection / auto-advance to the next node
  int selectionOffset=0; // can be 0 or 1, the selection in the data nodes list moves forward by this number of elements
  if (d->CheckBox_AutoAdvanceDataSelection->checkState()==Qt::Checked)
    {
    selectionOffset=1;
    }
  for ( int i = 0; i < d->ListWidget_CandidateDataNodes->count(); i++ )
    {
    QListWidgetItem * item = d->ListWidget_CandidateDataNodes->item(i);
    if (item->data(Qt::UserRole).toString() == currentCandidateNodeId)
      {
      if (i+selectionOffset<d->ListWidget_CandidateDataNodes->count())
        {
        // not at the end of the list, so select the next item
        d->ListWidget_CandidateDataNodes->setCurrentRow(i+selectionOffset);
        }
      else
        {
        // we are at the end of the list (already added the last element),
        // so unselect the item to prevent duplicate adding of the last element
        d->ListWidget_CandidateDataNodes->setCurrentRow(-1);
        }
      break;
      }
    }
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::onRemoveDataNodeButtonClicked()
{
  Q_D(qCjyxSequencesModuleWidget);

  vtkDMMLSequenceNode* currentSequence = vtkDMMLSequenceNode::SafeDownCast( d->DMMLNodeComboBox_Sequence->currentNode() );
  if ( currentSequence == nullptr )
    {
    return;
    }

  std::string currentIndexValue = currentSequence->GetNthIndexValue( d->TableWidget_DataNodes->currentRow() );
  if ( currentIndexValue.empty() )
    {
    return;
    }
  currentSequence->RemoveDataNodeAtValue( currentIndexValue.c_str() );
}

// --------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::setActiveSequenceNode(vtkDMMLSequenceNode* sequenceNode)
{
  Q_D(qCjyxSequencesModuleWidget);
  if (d->ActiveSequenceNode == sequenceNode)
    {
    return; // no change
    }
  if (d->ActiveSequenceNode != sequenceNode)
    {
    // Reconnect the input node's Modified() event observer
    this->qvtkReconnect(d->ActiveSequenceNode, sequenceNode, vtkCommand::ModifiedEvent,
      this, SLOT(onSequenceNodeModified()));
    d->ActiveSequenceNode = sequenceNode;
    }

  this->updateSequenceItemWidgetFromDMML();
  this->updateCandidateNodesWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::updateSequenceItemWidgetFromDMML()
{
  Q_D(qCjyxSequencesModuleWidget);

  vtkDMMLSequenceNode* currentSequence = d->ActiveSequenceNode;

  if ( currentSequence == nullptr )
    {
    d->Label_DataNodeTypeValue->setText( FROM_STD_STRING_SAFE( "undefined" ) );
    d->LineEdit_IndexName->setText( FROM_STD_STRING_SAFE( "" ) );
    d->LineEdit_IndexUnit->setText( FROM_STD_STRING_SAFE( "" ) );
    d->ComboBox_IndexType->setCurrentIndex(-1);
    d->TableWidget_DataNodes->clear();
    d->TableWidget_DataNodes->setRowCount( 0 );
    d->TableWidget_DataNodes->setColumnCount( 0 );
    d->ListWidget_CandidateDataNodes->clear();
    setEnableWidgets(false);
    return;
    }

  setEnableWidgets(true);

  // Put the correct properties in the sequence node table
  QString typeName=currentSequence->GetDataNodeTagName().c_str();
  d->Label_DataNodeTypeValue->setText( typeName.toLatin1().constData() ); // TODO: maybe use the node->tag instead?

  d->LineEdit_IndexName->setText( currentSequence->GetIndexName().c_str() );
  d->LineEdit_IndexUnit->setText(currentSequence->GetIndexUnit().c_str());
  d->ComboBox_IndexType->setCurrentIndex( d->ComboBox_IndexType->findText(currentSequence->GetIndexTypeAsString().c_str()) );

  // Display all of the sequence nodes
  d->TableWidget_DataNodes->clear();
  d->TableWidget_DataNodes->setRowCount( currentSequence->GetNumberOfDataNodes() );
  d->TableWidget_DataNodes->setColumnCount( DATA_NODE_NUMBER_OF_COLUMNS );
  std::stringstream valueHeader;
  valueHeader << currentSequence->GetIndexName();
  valueHeader << " (" << currentSequence->GetIndexUnit() << ")";
  QStringList SequenceNodesTableHeader;
  //SequenceNodesTableHeader.insert( DATA_NODE_VIS_COLUMN, "Vis" );
  SequenceNodesTableHeader.insert( DATA_NODE_VALUE_COLUMN, valueHeader.str().c_str() );
  SequenceNodesTableHeader.insert( DATA_NODE_NAME_COLUMN, "Name" );
  d->TableWidget_DataNodes->setHorizontalHeaderLabels( SequenceNodesTableHeader );

  int numberOfDataNodes=currentSequence->GetNumberOfDataNodes();
  for ( int dataNodeIndex = 0; dataNodeIndex < numberOfDataNodes; dataNodeIndex++ )
    {
    std::string currentValue = currentSequence->GetNthIndexValue( dataNodeIndex );
    vtkDMMLNode* currentDataNode = currentSequence->GetNthDataNode( dataNodeIndex );

    if (currentDataNode==nullptr)
      {
      qCritical() << "qCjyxSequencesModuleWidget::updateSequenceItemWidgetFromDMML invalid data node";
      continue;
      }

    QTableWidgetItem* valueItem = new QTableWidgetItem( FROM_STD_STRING_SAFE( currentValue.c_str() ) );
    valueItem->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

    QTableWidgetItem* nameItem = new QTableWidgetItem( FROM_STD_STRING_SAFE( currentDataNode->GetName() ) );

    d->TableWidget_DataNodes->setItem( dataNodeIndex, DATA_NODE_VALUE_COLUMN, valueItem );
    d->TableWidget_DataNodes->setItem( dataNodeIndex, DATA_NODE_NAME_COLUMN, nameItem );
    }

  //d->TableWidget_DataNodes->resizeColumnsToContents();
  d->TableWidget_DataNodes->resizeRowsToContents();

  // Open the data node adding section if there are no data nodes yet
  // to make it easier to see how to add new nodes.
  if (numberOfDataNodes==0)
    {
    d->ExpandButton_DataNodes->setChecked(true);
    }
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::updateCandidateNodesWidgetFromDMML(bool forceUpdate /* =false */)
{
  Q_D(qCjyxSequencesModuleWidget);

  vtkDMMLSequenceNode* currentSequence = vtkDMMLSequenceNode::SafeDownCast( d->DMMLNodeComboBox_Sequence->currentNode() );
  if ( currentSequence == nullptr )
    {
    d->ListWidget_CandidateDataNodes->clear();
    return;
    }

  QString currentDataNodeCandidatesClassName(currentSequence->GetDataNodeClassName().c_str());
  if (!forceUpdate && d->DataNodeCandidatesClassName == currentDataNodeCandidatesClassName)
    {
    // already up-to-date
    return;
    }
  d->DataNodeCandidatesClassName = currentDataNodeCandidatesClassName;

  // Display the candidate data nodes
  vtkNew<vtkCollection> candidateNodes;
  d->GetDataNodeCandidates( candidateNodes.GetPointer(), currentSequence );

  d->ListWidget_CandidateDataNodes->clear();

  for ( int i = 0; i < candidateNodes->GetNumberOfItems(); i++ )
    {
    vtkDMMLNode* currentCandidateNode = vtkDMMLNode::SafeDownCast( candidateNodes->GetItemAsObject( i ));
    QListWidgetItem *qlwi = new QListWidgetItem();
    qlwi->setText(currentCandidateNode->GetName());
    qlwi->setData(Qt::UserRole, QString(currentCandidateNode->GetID()));
    d->ListWidget_CandidateDataNodes->addItem(qlwi);
    }
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::setEnableWidgets(bool enable)
{
  Q_D(qCjyxSequencesModuleWidget);
  d->Label_DataNodeTypeValue->setEnabled(enable);
  d->LineEdit_IndexName->setEnabled(enable);
  d->LineEdit_IndexUnit->setEnabled(enable);
  d->ComboBox_IndexType->setEnabled(enable);
  d->TableWidget_DataNodes->setEnabled(enable);
  d->ListWidget_CandidateDataNodes->setEnabled(enable);
  d->LineEdit_NewDataNodeIndexValue->setEnabled(enable);
  d->PushButton_AddDataNode->setEnabled(enable);
  d->PushButton_RemoveDataNode->setEnabled(enable);
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::activeBrowserNodeChanged(vtkDMMLNode* node)
{
  Q_D(qCjyxSequencesModuleWidget);
  vtkDMMLSequenceBrowserNode* browserNode = vtkDMMLSequenceBrowserNode::SafeDownCast(node);
  this->setActiveBrowserNode(browserNode);
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::sequenceNodeChanged(vtkDMMLNode* inputNode)
{
  vtkDMMLSequenceNode* sequenceNode = vtkDMMLSequenceNode::SafeDownCast(inputNode);
  this->setMasterSequenceNode(sequenceNode);
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::playbackItemSkippingEnabledChanged(bool enabled)
{
  Q_D(qCjyxSequencesModuleWidget);
  if (d->ActiveBrowserNode == nullptr)
    {
    return; // no active node, nothing to update
    }
  d->ActiveBrowserNode->SetPlaybackItemSkippingEnabled(enabled);
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::recordMasterOnlyChanged(bool enabled)
{
  Q_D(qCjyxSequencesModuleWidget);
  if (d->ActiveBrowserNode == nullptr)
    {
    return; // no active node, nothing to update
    }
  d->ActiveBrowserNode->SetRecordMasterOnly(enabled);
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::recordingSamplingModeChanged(int index)
{
  Q_D(qCjyxSequencesModuleWidget);
  if (d->ActiveBrowserNode == nullptr)
    {
    return; // no active node, nothing to update
    }
  d->ActiveBrowserNode->SetRecordingSamplingMode(index);
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::indexDisplayModeChanged(int index)
{
  Q_D(qCjyxSequencesModuleWidget);
  if (d->ActiveBrowserNode == nullptr)
    {
    return; // no active node, nothing to update
    }
  d->ActiveBrowserNode->SetIndexDisplayMode(index);
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::indexDisplayFormatChanged(const QString& format)
{
  Q_D(qCjyxSequencesModuleWidget);
  if (d->ActiveBrowserNode == nullptr)
    {
    return; // no active node, nothing to update
    }
  d->ActiveBrowserNode->SetIndexDisplayFormat(format.toStdString());
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::onActiveBrowserNodeModified(vtkObject* caller)
{
  Q_UNUSED(caller);
  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::onDMMLInputSequenceInputNodeModified(vtkObject* inputNode)
{
  Q_UNUSED(inputNode);
  this->updateWidgetFromDMML();
}

// --------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::setActiveBrowserNode(vtkDMMLSequenceBrowserNode* browserNode)
{
  Q_D(qCjyxSequencesModuleWidget);
  if (!this->dmmlScene() || this->dmmlScene()->IsBatchProcessing())
    {
    return;
    }
  if (d->ActiveBrowserNode == browserNode
    && browserNode != nullptr) // always update if browserNode is nullptr (needed for proper update during scene close)
    {
    // no change
    return;
    }

  this->qvtkReconnect(d->ActiveBrowserNode, browserNode, vtkCommand::ModifiedEvent,
    this, SLOT(onActiveBrowserNodeModified(vtkObject*)));

  d->ActiveBrowserNode = browserNode;

  d->DMMLNodeComboBox_ActiveBrowser->setCurrentNode(browserNode);
  d->sequenceBrowserPlayWidget->setDMMLSequenceBrowserNode(browserNode);
  d->sequenceBrowserSeekWidget->setDMMLSequenceBrowserNode(browserNode);
  qDMMLSequenceBrowserToolBar* toolBar = d->toolBar();
  if (toolBar)
    {
    toolBar->setActiveBrowserNode(browserNode);
    }

  this->updateWidgetFromDMML();
}

// --------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::setMasterSequenceNode(vtkDMMLSequenceNode* sequenceNode)
{
  Q_D(qCjyxSequencesModuleWidget);
  if (d->ActiveBrowserNode==nullptr)
    {
    // this happens when entering the module (the node selector already finds a suitable sequence node so it selects it, but
    // no browser node is selected yet)
    this->updateWidgetFromDMML();
    return;
    }
  if (sequenceNode!=d->ActiveBrowserNode->GetMasterSequenceNode())
    {
    bool oldModify=d->ActiveBrowserNode->StartModify();

    // Reconnect the input node's Modified() event observer
    this->qvtkReconnect(d->ActiveBrowserNode->GetMasterSequenceNode(), sequenceNode, vtkCommand::ModifiedEvent,
      this, SLOT(onDMMLInputSequenceInputNodeModified(vtkObject*)));

    char* sequenceNodeId = sequenceNode==nullptr ? nullptr : sequenceNode->GetID();

    d->ActiveBrowserNode->SetAndObserveMasterSequenceNodeID(sequenceNodeId);

    // Update d->ActiveBrowserNode->SetAndObserveSelectedSequenceNodeID
    if (sequenceNode!=nullptr && sequenceNode->GetNumberOfDataNodes()>0)
      {
      d->ActiveBrowserNode->SetSelectedItemNumber(0);
      }
    else
      {
      d->ActiveBrowserNode->SetSelectedItemNumber(-1);
      }

    d->ActiveBrowserNode->EndModify(oldModify);
    }
}

// --------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::onAddSequenceNodeButtonClicked()
{
  Q_D(qCjyxSequencesModuleWidget);
  vtkDMMLSequenceBrowserNode* browserNode = vtkDMMLSequenceBrowserNode::SafeDownCast(d->DMMLNodeComboBox_ActiveBrowser->currentNode());
  if (!browserNode)
    {
    qWarning() << Q_FUNC_INFO << " failed: no browser node is selected";
    return;
    }
  vtkDMMLSequenceNode* sequenceNode = vtkDMMLSequenceNode::SafeDownCast(d->DMMLNodeComboBox_SynchronizeSequenceNode->currentNode());
  vtkDMMLSequenceNode* addedSequenceNode = d->logic()->AddSynchronizedNode(sequenceNode, nullptr, browserNode);
  if (addedSequenceNode)
    {
    if (browserNode->GetNumberOfSynchronizedSequenceNodes() == 0)
      {
      // master node is added - if it's a new (empty) sequence node, enable recording by default
      if (sequenceNode == nullptr)
        {
        browserNode->SetRecording(addedSequenceNode, true);
        }
      }
    else
      {
      // synchronized node is added - copy the recording setting from the master sequence node
      browserNode->SetRecording(addedSequenceNode, browserNode->GetRecording(browserNode->GetMasterSequenceNode()));
      }
    }
}

// --------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::onRemoveSequenceNodesButtonClicked()
{
  Q_D(qCjyxSequencesModuleWidget);
  // First, grab all of the selected rows
  QModelIndexList modelIndexList = d->tableWidget_SynchronizedSequenceNodes->selectionModel()->selectedIndexes();
  std::vector<std::string> selectedSequenceIDs;
  for (QModelIndexList::iterator index = modelIndexList.begin(); index!=modelIndexList.end(); index++)
    {
    QWidget* proxyNodeComboBox = d->tableWidget_SynchronizedSequenceNodes->cellWidget((*index).row(), SYNCH_NODES_PROXY_COLUMN);
    std::string currSelectedSequenceID = proxyNodeComboBox->property("DMMLNodeID").toString().toLatin1().constData();
    selectedSequenceIDs.push_back(currSelectedSequenceID);
    disconnect(proxyNodeComboBox, SIGNAL(currentNodeChanged(vtkDMMLNode*)), this,
      SLOT(onProxyNodeChanged(vtkDMMLNode*))); // No need to reconnect - the entire row is going to be removed
    }
  // Now, use the DMML ID stored by the proxy node combo box to determine the sequence nodes to remove from the browser
  std::vector<std::string>::iterator sequenceIDItr;
  for (sequenceIDItr = selectedSequenceIDs.begin(); sequenceIDItr != selectedSequenceIDs.end(); sequenceIDItr++)
    {
    d->ActiveBrowserNode->RemoveSynchronizedSequenceNode((*sequenceIDItr).c_str());
    }
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::updateWidgetFromDMML()
{
  Q_D(qCjyxSequencesModuleWidget);

  d->SynchronizedBrowsingSection->setEnabled(d->ActiveBrowserNode != nullptr);
  d->PlottingSection->setEnabled(d->ActiveBrowserNode != nullptr);
  d->AdvancedSection->setEnabled(d->ActiveBrowserNode != nullptr);

  if (d->ActiveBrowserNode==nullptr)
    {
    this->refreshSynchronizedSequenceNodesTable();
    return;
    }

  vtkDMMLSequenceNode* sequenceNode = d->ActiveBrowserNode->GetMasterSequenceNode();

  bool wasBlocked = d->DMMLNodeComboBox_MasterSequence->blockSignals(true);
  d->DMMLNodeComboBox_MasterSequence->setCurrentNode(sequenceNode);
  d->DMMLNodeComboBox_MasterSequence->blockSignals(wasBlocked);

  wasBlocked = d->checkBox_PlaybackItemSkippingEnabled->blockSignals(true);
  d->checkBox_PlaybackItemSkippingEnabled->setChecked(d->ActiveBrowserNode->GetPlaybackItemSkippingEnabled());
  d->checkBox_PlaybackItemSkippingEnabled->blockSignals(wasBlocked);

  wasBlocked = d->checkBox_RecordMasterOnly->blockSignals(true);
  d->checkBox_RecordMasterOnly->setChecked(d->ActiveBrowserNode->GetRecordMasterOnly());
  d->checkBox_RecordMasterOnly->blockSignals(wasBlocked);

  wasBlocked = d->comboBox_RecordingSamplingMode->blockSignals(true);
  d->comboBox_RecordingSamplingMode->setCurrentIndex(d->ActiveBrowserNode->GetRecordingSamplingMode());
  d->comboBox_RecordingSamplingMode->blockSignals(wasBlocked);

  wasBlocked = d->comboBox_IndexDisplayMode->blockSignals(true);
  d->comboBox_IndexDisplayMode->setCurrentIndex(d->ActiveBrowserNode->GetIndexDisplayMode());
  d->comboBox_IndexDisplayMode->blockSignals(wasBlocked);

  wasBlocked = d->lineEdit_IndexDisplayFormat->blockSignals(true);
  int position = d->lineEdit_IndexDisplayFormat->cursorPosition();
  d->lineEdit_IndexDisplayFormat->setText(QString::fromStdString(d->ActiveBrowserNode->GetIndexDisplayFormat()));
  d->lineEdit_IndexDisplayFormat->setCursorPosition(position);
  d->lineEdit_IndexDisplayFormat->blockSignals(wasBlocked);

  this->refreshSynchronizedSequenceNodesTable();
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::refreshSynchronizedSequenceNodesTable()
{
  Q_D(qCjyxSequencesModuleWidget);

  if (d->ActiveBrowserNode != nullptr &&
    (d->ActiveBrowserNode->GetRecordingActive() || d->ActiveBrowserNode->GetPlaybackActive()))
    {
    // this is an expensive operation, we cannot afford to do it while recording or replaying
    // TODO: make this update method much more efficient
    return;
    }


  // Clear the table
  for (int row=0; row<d->tableWidget_SynchronizedSequenceNodes->rowCount(); row++)
    {
    QCheckBox* playbackCheckbox = dynamic_cast<QCheckBox*>(d->tableWidget_SynchronizedSequenceNodes->cellWidget(row, SYNCH_NODES_PLAYBACK_COLUMN));
    disconnect(playbackCheckbox, SIGNAL(stateChanged(int)), this, SLOT(synchronizedSequenceNodePlaybackStateChanged(int)));
    QCheckBox* recordingCheckbox = dynamic_cast<QCheckBox*>(d->tableWidget_SynchronizedSequenceNodes->cellWidget(row, SYNCH_NODES_RECORDING_COLUMN));
    disconnect(recordingCheckbox, SIGNAL(stateChanged(int)), this, SLOT(synchronizedSequenceNodeRecordingStateChanged(int)));
    QCheckBox* overwriteProxyNameCheckbox = dynamic_cast<QCheckBox*>(
      d->tableWidget_SynchronizedSequenceNodes->cellWidget(row, SYNCH_NODES_OVERWRITE_PROXY_NAME_COLUMN));
    disconnect(overwriteProxyNameCheckbox, SIGNAL(stateChanged(int)), this, SLOT(synchronizedSequenceNodeOverwriteProxyNameStateChanged(int)));
    QCheckBox* saveChangesCheckbox = dynamic_cast<QCheckBox*>(d->tableWidget_SynchronizedSequenceNodes->cellWidget(row, SYNCH_NODES_SAVE_CHANGES_COLUMN));
    disconnect(saveChangesCheckbox, SIGNAL(stateChanged(int)), this, SLOT(synchronizedSequenceNodeSaveChangesStateChanged(int)));
    qDMMLNodeComboBox* proxyNodeComboBox = dynamic_cast<qDMMLNodeComboBox*>(
      d->tableWidget_SynchronizedSequenceNodes->cellWidget(row, SYNCH_NODES_PROXY_COLUMN));
    disconnect(proxyNodeComboBox, SIGNAL(currentNodeChanged(vtkDMMLNode*)), this, SLOT(onProxyNodeChanged(vtkDMMLNode*)));
    }

  if (d->ActiveBrowserNode==nullptr)
    {
    d->tableWidget_SynchronizedSequenceNodes->setRowCount(0); // clear() would not actually remove the rows
    return;
    }
  // A valid active browser node is selected
  vtkDMMLSequenceNode* sequenceNode = d->ActiveBrowserNode->GetMasterSequenceNode();
  if (sequenceNode==nullptr)
    {
    d->tableWidget_SynchronizedSequenceNodes->setRowCount(0); // clear() would not actually remove the rows
    return;
    }

  disconnect(d->tableWidget_SynchronizedSequenceNodes, SIGNAL(cellChanged(int, int)), this, SLOT(sequenceNodeNameEdited(int, int)));

  vtkNew<vtkCollection> syncedNodes;
  d->ActiveBrowserNode->GetSynchronizedSequenceNodes(syncedNodes.GetPointer(), true);
  d->tableWidget_SynchronizedSequenceNodes->setRowCount(syncedNodes->GetNumberOfItems()); // +1 because we add the master as well

  // Create line for the compatible nodes
  for (int i=0; i<syncedNodes->GetNumberOfItems(); ++i)
    {
    vtkDMMLSequenceNode* syncedNode = vtkDMMLSequenceNode::SafeDownCast( syncedNodes->GetItemAsObject(i) );
    if (!syncedNode)
      {
      continue;
      }

    QTableWidgetItem* verticalHeaderItem = new QTableWidgetItem();
    if (!strcmp(syncedNode->GetID(), d->ActiveBrowserNode->GetMasterSequenceNode()->GetID()))
      {
      verticalHeaderItem->setText("M");
      verticalHeaderItem->setToolTip("Master sequence");
      }
    else
      {
      verticalHeaderItem->setText(QString::number(i));
      verticalHeaderItem->setToolTip("Synchronized sequence");
      }
    d->tableWidget_SynchronizedSequenceNodes->setVerticalHeaderItem(i, verticalHeaderItem);

    // Create checkboxes
    QCheckBox* playbackCheckbox = new QCheckBox(d->tableWidget_SynchronizedSequenceNodes);
    playbackCheckbox->setToolTip(tr("Include this node in synchronized playback"));
    playbackCheckbox->setProperty("DMMLNodeID", QString(syncedNode->GetID()));

    QCheckBox* overwriteProxyNameCheckbox = new QCheckBox(d->tableWidget_SynchronizedSequenceNodes);
    overwriteProxyNameCheckbox->setToolTip(tr("Overwrite the associated node's name during playback"));
    overwriteProxyNameCheckbox->setProperty("DMMLNodeID", QString(syncedNode->GetID()));

    QCheckBox* saveChangesCheckbox = new QCheckBox(d->tableWidget_SynchronizedSequenceNodes);
    saveChangesCheckbox->setToolTip(tr("Save changes to the node into the sequence"));
    saveChangesCheckbox->setProperty("DMMLNodeID", QString(syncedNode->GetID()));

    QCheckBox* recordingCheckbox = new QCheckBox(d->tableWidget_SynchronizedSequenceNodes);
    recordingCheckbox->setToolTip(tr("Include this node in synchronized recording"));
    recordingCheckbox->setProperty("DMMLNodeID", QString(syncedNode->GetID()));

    // Set previous checked state of the checkbox
    bool playbackChecked = d->ActiveBrowserNode->GetPlayback(syncedNode);
    playbackCheckbox->setChecked(playbackChecked);

    bool overwriteProxyNameChecked = d->ActiveBrowserNode->GetOverwriteProxyName(syncedNode);
    overwriteProxyNameCheckbox->setChecked(overwriteProxyNameChecked);

    bool saveChangesChecked = d->ActiveBrowserNode->GetSaveChanges(syncedNode);
    saveChangesCheckbox->setChecked(saveChangesChecked);

    bool recordingChecked = d->ActiveBrowserNode->GetRecording(syncedNode);
    recordingCheckbox->setChecked(recordingChecked);

    connect(playbackCheckbox, SIGNAL(stateChanged(int)), this, SLOT(synchronizedSequenceNodePlaybackStateChanged(int)));
    connect(overwriteProxyNameCheckbox, SIGNAL(stateChanged(int)), this, SLOT(synchronizedSequenceNodeOverwriteProxyNameStateChanged(int)));
    connect(saveChangesCheckbox, SIGNAL(stateChanged(int)), this, SLOT(synchronizedSequenceNodeSaveChangesStateChanged(int)));
    connect(recordingCheckbox, SIGNAL(stateChanged(int)), this, SLOT(synchronizedSequenceNodeRecordingStateChanged(int)));

    d->tableWidget_SynchronizedSequenceNodes->setCellWidget(i, SYNCH_NODES_PLAYBACK_COLUMN, playbackCheckbox);
    d->tableWidget_SynchronizedSequenceNodes->setCellWidget(i, SYNCH_NODES_RECORDING_COLUMN, recordingCheckbox);
    d->tableWidget_SynchronizedSequenceNodes->setCellWidget(i, SYNCH_NODES_OVERWRITE_PROXY_NAME_COLUMN, overwriteProxyNameCheckbox);
    d->tableWidget_SynchronizedSequenceNodes->setCellWidget(i, SYNCH_NODES_SAVE_CHANGES_COLUMN, saveChangesCheckbox);

    QTableWidgetItem* nameItem = new QTableWidgetItem( QString(syncedNode->GetName()) );
    d->tableWidget_SynchronizedSequenceNodes->setItem(i, SYNCH_NODES_NAME_COLUMN, nameItem);

    vtkDMMLNode* proxyNode = d->ActiveBrowserNode->GetProxyNode(syncedNode);
    qDMMLNodeComboBox* proxyNodeComboBox = new qDMMLNodeComboBox();
    if (!syncedNode->GetDataNodeClassName().empty())
      {
      proxyNodeComboBox->setNodeTypes(QStringList() << syncedNode->GetDataNodeClassName().c_str());
      }
    else
      {
      proxyNodeComboBox->setNodeTypes(d->SupportedProxyNodeTypes);
      }
    proxyNodeComboBox->setAddEnabled(false);
    proxyNodeComboBox->setNoneEnabled(true);
    proxyNodeComboBox->setRemoveEnabled(true);
    proxyNodeComboBox->setRenameEnabled(true);
    proxyNodeComboBox->setShowChildNodeTypes(false); // all supported node types are explicitly listed
    proxyNodeComboBox->setShowHidden(true); // display nodes are hidden by default
    proxyNodeComboBox->setDMMLScene(this->dmmlScene());
    proxyNodeComboBox->setCurrentNode(proxyNode);
    proxyNodeComboBox->setProperty("DMMLNodeID", QString(syncedNode->GetID()));
    d->tableWidget_SynchronizedSequenceNodes->setCellWidget(i, SYNCH_NODES_PROXY_COLUMN, proxyNodeComboBox);

    connect(proxyNodeComboBox, SIGNAL(currentNodeChanged(vtkDMMLNode*)), this, SLOT(onProxyNodeChanged(vtkDMMLNode*)));
    }

  connect(d->tableWidget_SynchronizedSequenceNodes, SIGNAL(cellChanged(int, int)), this, SLOT(sequenceNodeNameEdited(int, int)));
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::sequenceNodeNameEdited(int row, int column)
{
  Q_D(qCjyxSequencesModuleWidget);

  if (d->ActiveBrowserNode==nullptr)
    {
    qCritical() << "qCjyxSequencesModuleWidget::synchronizedSequenceNodePlaybackStateChanged: Invalid activeBrowserNode";
    return;
    }
  if (column!=SYNCH_NODES_NAME_COLUMN)
    {
    return;
    }

  std::string newSequenceNodeName = d->tableWidget_SynchronizedSequenceNodes->item(row, column)->text().toStdString();

  QWidget* proxyNodeComboBox = d->tableWidget_SynchronizedSequenceNodes->cellWidget(row, SYNCH_NODES_PROXY_COLUMN);
  std::string synchronizedNodeID = proxyNodeComboBox->property("DMMLNodeID").toString().toLatin1().constData();
  vtkDMMLSequenceNode* synchronizedNode = vtkDMMLSequenceNode::SafeDownCast( this->dmmlScene()->GetNodeByID(synchronizedNodeID) );

  synchronizedNode->SetName(newSequenceNodeName.c_str());
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::synchronizedSequenceNodePlaybackStateChanged(int aState)
{
  Q_D(qCjyxSequencesModuleWidget);

  if (d->ActiveBrowserNode==nullptr)
    {
    qCritical() << "qCjyxSequencesModuleWidget::synchronizedSequenceNodePlaybackStateChanged: Invalid activeBrowserNode";
    return;
    }

  QCheckBox* senderCheckbox = dynamic_cast<QCheckBox*>(sender());
  if (!senderCheckbox)
    {
    qCritical() << "qCjyxSequencesModuleWidget::synchronizedSequenceNodePlaybackStateChanged: Invalid sender checkbox";
    return;
    }

  std::string synchronizedNodeID = senderCheckbox->property("DMMLNodeID").toString().toLatin1().constData();
  vtkDMMLSequenceNode* synchronizedNode = vtkDMMLSequenceNode::SafeDownCast( this->dmmlScene()->GetNodeByID(synchronizedNodeID) );
  d->ActiveBrowserNode->SetPlayback(synchronizedNode, aState);
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::synchronizedSequenceNodeRecordingStateChanged(int aState)
{
  Q_D(qCjyxSequencesModuleWidget);

  if (d->ActiveBrowserNode==nullptr)
    {
    qCritical() << "qCjyxSequencesModuleWidget::synchronizedSequenceNodeRecordingStateChanged: Invalid activeBrowserNode";
    return;
    }

  QCheckBox* senderCheckbox = dynamic_cast<QCheckBox*>(sender());
  if (!senderCheckbox)
    {
    qCritical() << "qCjyxSequencesModuleWidget::synchronizedSequenceNodeRecordingStateChanged: Invalid sender checkbox";
    return;
    }

  std::string synchronizedNodeID = senderCheckbox->property("DMMLNodeID").toString().toLatin1().constData();
  vtkDMMLSequenceNode* synchronizedNode = vtkDMMLSequenceNode::SafeDownCast( this->dmmlScene()->GetNodeByID(synchronizedNodeID) );
  d->ActiveBrowserNode->SetRecording(synchronizedNode, aState);
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::synchronizedSequenceNodeOverwriteProxyNameStateChanged(int aState)
{
  Q_D(qCjyxSequencesModuleWidget);

  if (d->ActiveBrowserNode==nullptr)
    {
    qCritical() << "qCjyxSequencesModuleWidget::synchronizedSequenceNodeOverwriteProxyNameStateChanged: Invalid activeBrowserNode";
    return;
    }

  QCheckBox* senderCheckbox = dynamic_cast<QCheckBox*>(sender());
  if (!senderCheckbox)
    {
    qCritical() << "qCjyxSequencesModuleWidget::synchronizedSequenceNodeOverwriteProxyNameStateChanged: Invalid sender checkbox";
    return;
    }

  std::string synchronizedNodeID = senderCheckbox->property("DMMLNodeID").toString().toLatin1().constData();
  vtkDMMLSequenceNode* synchronizedNode = vtkDMMLSequenceNode::SafeDownCast( this->dmmlScene()->GetNodeByID(synchronizedNodeID) );
  d->ActiveBrowserNode->SetOverwriteProxyName(synchronizedNode, aState);
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::synchronizedSequenceNodeSaveChangesStateChanged(int aState)
{
  Q_D(qCjyxSequencesModuleWidget);

  if (d->ActiveBrowserNode==nullptr)
    {
    qCritical() << "qCjyxSequencesModuleWidget::synchronizedSequenceNodeSaveChangesStateChanged: Invalid activeBrowserNode";
    return;
    }

  QCheckBox* senderCheckbox = dynamic_cast<QCheckBox*>(sender());
  if (!senderCheckbox)
    {
    qCritical() << "qCjyxSequencesModuleWidget::synchronizedSequenceNodeSaveChangesStateChanged: Invalid sender checkbox";
    return;
    }

  std::string synchronizedNodeID = senderCheckbox->property("DMMLNodeID").toString().toLatin1().constData();
  vtkDMMLSequenceNode* synchronizedNode = vtkDMMLSequenceNode::SafeDownCast( this->dmmlScene()->GetNodeByID(synchronizedNodeID) );
  d->ActiveBrowserNode->SetSaveChanges(synchronizedNode, aState);
}

//-----------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::onProxyNodeChanged(vtkDMMLNode* newProxyNode)
{
  Q_D(qCjyxSequencesModuleWidget);

  if (d->ActiveBrowserNode==nullptr)
    {
    qCritical() << "qCjyxSequencesModuleWidget::onProxyNodeChanged: Invalid activeBrowserNode";
    return;
    }

  qDMMLNodeComboBox* senderComboBox = dynamic_cast<qDMMLNodeComboBox*>(sender());
  if (!senderComboBox)
    {
    qCritical() << "qCjyxSequencesModuleWidget::onProxyNodeChanged: Invalid sender checkbox";
    return;
    }

  std::string synchronizedNodeID = senderComboBox->property("DMMLNodeID").toString().toLatin1().constData();
  vtkDMMLSequenceNode* synchronizedNode = vtkDMMLSequenceNode::SafeDownCast( this->dmmlScene()->GetNodeByID(synchronizedNodeID) );

  // If name sync is enabled between sequence and proxy node then update the sequence node name based on the proxy node
  if (newProxyNode && newProxyNode->GetName() != nullptr
    && synchronizedNode && d->ActiveBrowserNode->GetOverwriteProxyName(synchronizedNode))
    {
    std::string baseName = "Data";
    if (newProxyNode->GetAttribute("Sequences.BaseName") != 0)
      {
      baseName = newProxyNode->GetAttribute("Sequences.BaseName");
      }
    else if (newProxyNode->GetName() != 0)
      {
      baseName = newProxyNode->GetName();
      }
    baseName += " sequence";
    std::string proxyNodeName = this->dmmlScene()->GetUniqueNameByString(baseName.c_str());
    synchronizedNode->SetName(proxyNodeName.c_str());
    }

  d->logic()->AddSynchronizedNode(synchronizedNode, newProxyNode, d->ActiveBrowserNode);
}

//------------------------------------------------------------------------------
void qCjyxSequencesModuleWidget::updateChart()
{
  Q_D(qCjyxSequencesModuleWidget);
  if (  d->pushButton_iCharting->isChecked())
    {
    d->updateInteractiveCharting();
    }
}

//-----------------------------------------------------------
bool qCjyxSequencesModuleWidget::setEditedNode(vtkDMMLNode* node, QString role /* = QString()*/, QString context /* = QString() */)
{
  Q_UNUSED(context);
  Q_D(qCjyxSequencesModuleWidget);
  if (vtkDMMLSequenceBrowserNode::SafeDownCast(node))
    {
    if (role == "toolbar")
      {
      qDMMLSequenceBrowserToolBar* toolBar = d->toolBar();
      if (toolBar)
        {
        toolBar->setActiveBrowserNode(vtkDMMLSequenceBrowserNode::SafeDownCast(node));
        }
      }
    else
      {
      d->mainTabWidget->setCurrentIndex(0); // browse tab
      d->DMMLNodeComboBox_ActiveBrowser->setCurrentNode(node);
      }
    return true;
    }

  if (vtkDMMLSequenceNode::SafeDownCast(node))
    {
    d->mainTabWidget->setCurrentIndex(1); // edit tab
    d->DMMLNodeComboBox_Sequence->setCurrentNode(node);
    return true;
    }

  return false;
}
