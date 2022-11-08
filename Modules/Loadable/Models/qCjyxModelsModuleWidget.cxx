/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QScrollArea>

// CjyxQt includes
#include <qCjyxCoreApplication.h>
#include <qCjyxModuleManager.h>
#include <qCjyxAbstractCoreModule.h>

// Subject hierarchy widgets
#include "qDMMLSubjectHierarchyTreeView.h"
#include "qDMMLSubjectHierarchyModel.h"
#include "qDMMLSortFilterSubjectHierarchyProxyModel.h"

// Cjyx includes
#include "qCjyxModelsModuleWidget.h"
#include "ui_qCjyxModelsModuleWidget.h"

// Logic includes
#include "vtkDMMLDisplayableHierarchyLogic.h"
#include "vtkCjyxModelsLogic.h"

// Colors DMML and Logic includes
#include <vtkCjyxColorLogic.h>
#include <vtkDMMLColorLegendDisplayNode.h>

// DMML includes
#include "vtkDMMLFolderDisplayNode.h"
#include "vtkDMMLModelNode.h"
#include "vtkDMMLModelDisplayNode.h"
#include "vtkDMMLSelectionNode.h"
#include "vtkDMMLSubjectHierarchyNode.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkNew.h>


//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_Models
class qCjyxModelsModuleWidgetPrivate: public Ui_qCjyxModelsModuleWidget
{
public:
  qCjyxModelsModuleWidgetPrivate();

  QStringList HideChildNodeTypes;
  QString FiberDisplayClass;
  vtkSmartPointer<vtkCallbackCommand> CallBack;
};

//-----------------------------------------------------------------------------
// qCjyxModelsModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qCjyxModelsModuleWidgetPrivate::qCjyxModelsModuleWidgetPrivate()
{
  this->HideChildNodeTypes = (QStringList() << "vtkDMMLFiberBundleNode" << "vtkDMMLAnnotationNode");
  this->FiberDisplayClass = "vtkDMMLFiberBundleLineDisplayNode";
  this->CallBack = vtkSmartPointer<vtkCallbackCommand>::New();
}

//-----------------------------------------------------------------------------
// qCjyxModelsModuleWidget methods

//-----------------------------------------------------------------------------
qCjyxModelsModuleWidget::qCjyxModelsModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qCjyxModelsModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qCjyxModelsModuleWidget::~qCjyxModelsModuleWidget()
{
  Q_D(qCjyxModelsModuleWidget);

  // set the dmml scene to null so that stop observing it for events
  if (this->dmmlScene())
    {
    this->dmmlScene()->RemoveObserver(d->CallBack);
    }

  this->setDMMLScene(nullptr);
}

//-----------------------------------------------------------------------------
void qCjyxModelsModuleWidget::setup()
{
  Q_D(qCjyxModelsModuleWidget);

  d->setupUi(this);

  d->ModelDisplayWidget->setEnabled(false);

  d->ClipModelsNodeComboBox->setVisible(false);

  // Set up tree view
  qDMMLSortFilterSubjectHierarchyProxyModel* sortFilterProxyModel = d->SubjectHierarchyTreeView->sortFilterProxyModel();
  sortFilterProxyModel->setNodeTypes(QStringList() << "vtkDMMLModelNode" << "vtkDMMLFolderDisplayNode");
  d->SubjectHierarchyTreeView->setColumnHidden(d->SubjectHierarchyTreeView->model()->idColumn(), true);
  d->SubjectHierarchyTreeView->setColumnHidden(d->SubjectHierarchyTreeView->model()->transformColumn(), true);
  d->SubjectHierarchyTreeView->setPluginWhitelist(QStringList() << "Models" << "Folder" << "Opacity" << "Visibility");
  d->SubjectHierarchyTreeView->setSelectRoleSubMenuVisible(false);
  d->SubjectHierarchyTreeView->expandToDepth(4);
  d->SubjectHierarchyTreeView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

  connect( d->SubjectHierarchyTreeView, SIGNAL(currentItemsChanged(QList<vtkIdType>)),
    this, SLOT(setDisplaySelectionFromSubjectHierarchyItems(QList<vtkIdType>)) );

  connect( d->FilterModelSearchBox, SIGNAL(textChanged(QString)),
    sortFilterProxyModel, SLOT(setNameFilter(QString)) );

  connect( d->InformationButton, SIGNAL(contentsCollapsed(bool)),
    this, SLOT(onInformationSectionCollapsed(bool)) );

  connect(d->ModelDisplayWidget, SIGNAL(clippingToggled(bool)),
    this, SLOT(onClipSelectedModelToggled(bool)) );

  connect(d->ModelDisplayWidget, SIGNAL(clippingConfigurationButtonClicked()),
    this, SLOT(onClippingConfigurationButtonClicked()) );

  // add an add hierarchy right click action on the scene and hierarchy nodes
  connect(d->ModelDisplayWidget, SIGNAL(displayNodeChanged()),
    this, SLOT(onDisplayNodeChanged()) );

  connect(d->ClipSelectedModelCheckBox, SIGNAL(toggled(bool)),
    this, SLOT(onClipSelectedModelToggled(bool)) );

  connect(d->ColorLegendCollapsibleGroupBox, SIGNAL(toggled(bool)),
    this, SLOT(onColorLegendCollapsibleGroupBoxToggled(bool)));

  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
void qCjyxModelsModuleWidget::enter()
{
  Q_D(qCjyxModelsModuleWidget);

  // Set minimum models tree height so that it has a reasonable starting size
  int displayedItemCount = d->SubjectHierarchyTreeView->displayedItemCount();
  int headerHeight = d->SubjectHierarchyTreeView->header()->sizeHint().height();
  int treeViewHeight = headerHeight * 3; // Approximately three rows when empty (cannot get row height)
  if (displayedItemCount > 0)
    {
    // Calculate full tree view height based on number of displayed items and item height (and add 2 for the borders).
    treeViewHeight = headerHeight + displayedItemCount * d->SubjectHierarchyTreeView->sizeHintForRow(0) + 2;
    }

  // Get height of the whole Models module widget panel
  int panelHeight = this->sizeHint().height();
  // Set tree view minimum height to be the minimum of the calculated full height and half
  // of the panel height
  d->SubjectHierarchyTreeView->setMinimumHeight(qMin<int>(treeViewHeight, panelHeight / 2.0));

  // Connect SH item modified event so that widget state is updated when a display node is created
  // on the currently selected item (when color is set to a folder)
  vtkDMMLSubjectHierarchyNode* shNode = d->SubjectHierarchyTreeView->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
    }
  qvtkConnect( shNode, vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemModifiedEvent,
    this, SLOT( onSubjectHierarchyItemModified(vtkObject*,void*) ) );
  qvtkConnect( shNode, vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemDisplayModifiedEvent,
    this, SLOT( onSubjectHierarchyItemModified(vtkObject*,void*) ) );

  this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qCjyxModelsModuleWidget::exit()
{
  Q_D(qCjyxModelsModuleWidget);

  // Disconnect SH node modified when module is not active
  vtkDMMLSubjectHierarchyNode* shNode = d->SubjectHierarchyTreeView->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
    }
  qvtkDisconnect( shNode, vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemModifiedEvent,
    this, SLOT( onSubjectHierarchyItemModified(vtkObject*,void*) ) );
  qvtkDisconnect( shNode, vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemDisplayModifiedEvent,
    this, SLOT( onSubjectHierarchyItemModified(vtkObject*,void*) ) );

  this->Superclass::exit();
}

//-----------------------------------------------------------------------------
void qCjyxModelsModuleWidget::setDMMLScene(vtkDMMLScene* scene)
{
  Q_D(qCjyxModelsModuleWidget);

  if (scene == this->dmmlScene())
    {
    return;
    }
  this->Superclass::setDMMLScene(scene);

  if (scene)
    {
    scene->RemoveObserver(d->CallBack);
    }
  if (scene)
    {
    d->CallBack->SetClientData(this);

    d->CallBack->SetCallback(qCjyxModelsModuleWidget::onDMMLSceneEvent);

    scene->AddObserver(vtkDMMLScene::EndImportEvent, d->CallBack);
    this->onDMMLSceneEvent(scene, vtkDMMLScene::EndImportEvent, this, nullptr);
    }
}

//-----------------------------------------------------------------------------
void qCjyxModelsModuleWidget::onDMMLSceneEvent(vtkObject* vtk_obj, unsigned long event,
                                                void* client_data, void* call_data)
{
  vtkDMMLScene* scene = reinterpret_cast<vtkDMMLScene*>(vtk_obj);
  qCjyxModelsModuleWidget* widget = reinterpret_cast<qCjyxModelsModuleWidget*>(client_data);
  vtkDMMLNode* node = reinterpret_cast<vtkDMMLNode*>(call_data);
  Q_UNUSED(node);
  Q_ASSERT(scene);
  Q_UNUSED(scene);
  Q_ASSERT(widget);
  if (event == vtkDMMLScene::EndImportEvent)
    {
    //widget->updateWidgetFromSelectionNode();
    }
}

//-----------------------------------------------------------------------------
void qCjyxModelsModuleWidget::showAllModels()
{
  if (this->logic() == nullptr)
    {
    return;
    }
  vtkCjyxModelsLogic *modelsLogic = vtkCjyxModelsLogic::SafeDownCast(this->logic());
  if (modelsLogic)
    {
    modelsLogic->SetAllModelsVisibility(1);
    }
}

//-----------------------------------------------------------------------------
void qCjyxModelsModuleWidget::hideAllModels()
{
  if (this->logic() == nullptr)
    {
    return;
    }
  vtkCjyxModelsLogic *modelsLogic = vtkCjyxModelsLogic::SafeDownCast(this->logic());
  if (modelsLogic)
    {
    modelsLogic->SetAllModelsVisibility(0);
    }
}

//-----------------------------------------------------------
bool qCjyxModelsModuleWidget::setEditedNode(vtkDMMLNode* node,
                                              QString role /* = QString()*/,
                                              QString context /* = QString()*/)
{
  Q_D(qCjyxModelsModuleWidget);
  Q_UNUSED(role);
  Q_UNUSED(context);
  if (vtkDMMLModelNode::SafeDownCast(node) || vtkDMMLFolderDisplayNode::SafeDownCast(node))
    {
    d->SubjectHierarchyTreeView->setCurrentNode(node);
    return true;
    }

  if (vtkDMMLModelDisplayNode::SafeDownCast(node))
    {
    vtkDMMLModelDisplayNode* displayNode = vtkDMMLModelDisplayNode::SafeDownCast(node);
    vtkDMMLModelNode* displayableNode = vtkDMMLModelNode::SafeDownCast(displayNode->GetDisplayableNode());
    if (!displayableNode)
      {
      return false;
      }
    d->SubjectHierarchyTreeView->setCurrentNode(displayableNode);
    return true;
    }

  return false;
}

//-----------------------------------------------------------
void qCjyxModelsModuleWidget::onClippingConfigurationButtonClicked()
{
  Q_D(qCjyxModelsModuleWidget);
  d->ClippingButton->setCollapsed(false);
  // Make sure import/export is visible
  if (this->parent() && this->parent()->parent() && this->parent()->parent()->parent())
    {
    QScrollArea* scrollArea = qobject_cast<QScrollArea*>(this->parent()->parent()->parent());
    if (scrollArea)
      {
      scrollArea->ensureWidgetVisible(d->ClippingButton);
      }
    }
}

//-----------------------------------------------------------
void qCjyxModelsModuleWidget::onDisplayNodeChanged()
{
  Q_D(qCjyxModelsModuleWidget);
  vtkDMMLModelDisplayNode* displayNode = d->ModelDisplayWidget->dmmlModelDisplayNode();
  bool wasBlocked = d->ClipSelectedModelCheckBox->blockSignals(true);
  d->ClipSelectedModelLabel->setEnabled(displayNode != nullptr);
  d->ClipSelectedModelCheckBox->setEnabled(displayNode != nullptr);
  d->ClipSelectedModelCheckBox->setChecked(displayNode != nullptr && displayNode->GetClipping());
  d->ClipSelectedModelCheckBox->blockSignals(wasBlocked);

  // Color legend
  vtkDMMLColorLegendDisplayNode* colorLegendNode = nullptr;
  colorLegendNode = vtkCjyxColorLogic::GetColorLegendDisplayNode(displayNode);
  d->ColorLegendDisplayNodeWidget->setDMMLColorLegendDisplayNode(colorLegendNode);

  if (!colorLegendNode)
    {
    d->ColorLegendCollapsibleGroupBox->setCollapsed(true);
    }
  d->ColorLegendCollapsibleGroupBox->setEnabled(displayNode && displayNode->GetColorNode());}

//-----------------------------------------------------------
void qCjyxModelsModuleWidget::onClipSelectedModelToggled(bool toggled)
{
  Q_D(qCjyxModelsModuleWidget);
  vtkDMMLModelDisplayNode* displayNode = d->ModelDisplayWidget->dmmlModelDisplayNode();
  if (displayNode)
    {
    int wasModified = displayNode->StartModify();
    displayNode->SetClipping(toggled);
    // By enabling clipping, backfaces may become visible.
    // Therefore, it is better to render them (not cull them).
    if (toggled)
      {
      displayNode->BackfaceCullingOff();
      displayNode->FrontfaceCullingOff();
      if (d->DMMLClipNodeWidget->dmmlClipNode() != nullptr
        && d->DMMLClipNodeWidget->redSliceClipState() == vtkDMMLClipModelsNode::ClipOff
        && d->DMMLClipNodeWidget->greenSliceClipState() == vtkDMMLClipModelsNode::ClipOff
        && d->DMMLClipNodeWidget->yellowSliceClipState() == vtkDMMLClipModelsNode::ClipOff)
        {
        // All clipping planes are disabled.
        // Enable the first clipping plane to show a clipped model to the user.
        d->DMMLClipNodeWidget->setRedSliceClipState(vtkDMMLClipModelsNode::ClipPositiveSpace);
        }
      }
    displayNode->EndModify(wasModified);
    }
}

//-----------------------------------------------------------------------------
void qCjyxModelsModuleWidget::setDisplaySelectionFromSubjectHierarchyItems(QList<vtkIdType> itemIDs)
{
  Q_D(qCjyxModelsModuleWidget);

  vtkDMMLSubjectHierarchyNode* shNode = d->SubjectHierarchyTreeView->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
    }

  vtkDMMLNode* firstDataNode = nullptr;
  if ( itemIDs.size() > 0
    // In case of empty selection the only item in the list is the scene
    && !(itemIDs.size() == 1 && itemIDs[0] == shNode->GetSceneItemID()) )
    {
    firstDataNode = shNode->GetItemDataNode(itemIDs[0]);
    }
  // Only set model node to info widget if it's visible
  d->DMMLModelInfoWidget->setDMMLModelNode(d->InformationButton->collapsed() ? nullptr : firstDataNode);

  d->ModelDisplayWidget->setCurrentSubjectHierarchyItemIDs(itemIDs);
}

//---------------------------------------------------------------------------
void qCjyxModelsModuleWidget::onSubjectHierarchyItemModified(vtkObject* vtkNotUsed(caller), void* vtkNotUsed(callData))
{
  Q_D(qCjyxModelsModuleWidget);

  QList<vtkIdType> currentItemIDs = d->SubjectHierarchyTreeView->currentItems();
  this->setDisplaySelectionFromSubjectHierarchyItems(currentItemIDs);
}

//---------------------------------------------------------------------------
void qCjyxModelsModuleWidget::onInformationSectionCollapsed(bool collapsed)
{
  Q_D(qCjyxModelsModuleWidget);

  if (!collapsed)
    {
    QList<vtkIdType> currentItemIDs = d->SubjectHierarchyTreeView->currentItems();
    this->setDisplaySelectionFromSubjectHierarchyItems(currentItemIDs);
    }
}

//------------------------------------------------------------------------------
void qCjyxModelsModuleWidget::onColorLegendCollapsibleGroupBoxToggled(bool toggled)
{
  Q_D(qCjyxModelsModuleWidget);

  // Make sure a legend display node exists if the color legend section is opened
  if (!toggled)
    {
    return;
    }

  vtkDMMLModelDisplayNode* displayNode = d->ModelDisplayWidget->dmmlModelDisplayNode();
  vtkDMMLColorLegendDisplayNode* colorLegendNode = vtkCjyxColorLogic::GetColorLegendDisplayNode(displayNode);
  if (!colorLegendNode)
    {
    // color legend node does not exist, we need to create it now

    // Pause render to prevent the new Color legend displayed for a moment before it is hidden.
    vtkDMMLApplicationLogic* dmmlAppLogic = this->logic()->GetDMMLApplicationLogic();
    if (dmmlAppLogic)
      {
      dmmlAppLogic->PauseRender();
      }
    colorLegendNode = vtkCjyxColorLogic::AddDefaultColorLegendDisplayNode(displayNode);
    colorLegendNode->SetVisibility(false); // just because the groupbox is opened, don't show color legend yet
    if (dmmlAppLogic)
      {
      dmmlAppLogic->ResumeRender();
      }
    }
  d->ColorLegendDisplayNodeWidget->setDMMLColorLegendDisplayNode(colorLegendNode);
}
