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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QAction>
#include <QFileDialog>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QStringBuilder>
#include <QTableWidgetItem>

// C++ includes
#include <cmath>

// Cjyx includes
#include "qCjyxTransformsModuleWidget.h"
#include "ui_qCjyxTransformsModuleWidget.h"

// vtkCjyxLogic includes
#include "vtkCjyxTransformLogic.h"

// DMMLWidgets includes
#include <qDMMLUtils.h>

// DMML includes
#include "vtkDMMLScalarVolumeNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLTransformNode.h"
#include "vtkDMMLTransformDisplayNode.h"
#include "vtkDMMLVectorVolumeNode.h"

// VTK includes
#include <vtkAddonMathUtilities.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>

//-----------------------------------------------------------------------------
class qCjyxTransformsModuleWidgetPrivate: public Ui_qCjyxTransformsModuleWidget
{
  Q_DECLARE_PUBLIC(qCjyxTransformsModuleWidget);
protected:
  qCjyxTransformsModuleWidget* const q_ptr;
public:
  qCjyxTransformsModuleWidgetPrivate(qCjyxTransformsModuleWidget& object);
  static QList<vtkSmartPointer<vtkDMMLTransformableNode> > getSelectedNodes(qDMMLTreeView* tree);
  vtkCjyxTransformLogic*      logic()const;
  vtkDMMLTransformNode*         DMMLTransformNode;
  QAction*                      CopyAction;
  QAction*                      PasteAction;
};

//-----------------------------------------------------------------------------
qCjyxTransformsModuleWidgetPrivate::qCjyxTransformsModuleWidgetPrivate(qCjyxTransformsModuleWidget& object)
  : q_ptr(&object)
{
  this->DMMLTransformNode = nullptr;
  this->CopyAction = nullptr;
  this->PasteAction = nullptr;
}
//-----------------------------------------------------------------------------
vtkCjyxTransformLogic* qCjyxTransformsModuleWidgetPrivate::logic()const
{
  Q_Q(const qCjyxTransformsModuleWidget);
  return vtkCjyxTransformLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
QList<vtkSmartPointer<vtkDMMLTransformableNode> > qCjyxTransformsModuleWidgetPrivate::getSelectedNodes(qDMMLTreeView* tree)
{
  QModelIndexList selectedIndexes =
    tree->selectionModel()->selectedRows();
  selectedIndexes = qDMMLTreeView::removeChildren(selectedIndexes);

  // Return the list of nodes
  QList<vtkSmartPointer<vtkDMMLTransformableNode> > selectedNodes;
  foreach(QModelIndex selectedIndex, selectedIndexes)
    {
    vtkDMMLTransformableNode* node = vtkDMMLTransformableNode::SafeDownCast(
      tree->sortFilterProxyModel()->
      dmmlNodeFromIndex( selectedIndex ));
    Q_ASSERT(node);
    selectedNodes << node;
    }
  return selectedNodes;
}

//-----------------------------------------------------------------------------
qCjyxTransformsModuleWidget::qCjyxTransformsModuleWidget(QWidget* _parentWidget)
  : Superclass(_parentWidget)
  , d_ptr(new qCjyxTransformsModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qCjyxTransformsModuleWidget::~qCjyxTransformsModuleWidget() = default;

//-----------------------------------------------------------------------------
void qCjyxTransformsModuleWidget::setup()
{
  Q_D(qCjyxTransformsModuleWidget);
  d->setupUi(this);

  // Add coordinate reference button to a button group
  d->CopyAction = new QAction(this);
  d->CopyAction->setIcon(QIcon(":Icons/Medium/CjyxEditCopy.png"));
  d->CopyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  d->CopyAction->setShortcuts(QKeySequence::Copy);
  d->CopyAction->setToolTip(tr("Copy"));
  this->addAction(d->CopyAction);
  d->PasteAction = new QAction(this);
  d->PasteAction->setIcon(QIcon(":Icons/Medium/CjyxEditPaste.png"));
  d->PasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  d->PasteAction->setShortcuts(QKeySequence::Paste);
  d->PasteAction->setToolTip(tr("Paste"));
  this->addAction(d->PasteAction);

  // Connect button group
  this->connect(d->TranslateFirstToolButton,
                SIGNAL(toggled(bool)),
                SLOT(onTranslateFirstButtonPressed(bool)));

  // Connect identity button
  this->connect(d->IdentityPushButton,
                SIGNAL(clicked()),
                SLOT(identity()));

  // Connect invert button
  this->connect(d->InvertPushButton,
                SIGNAL(clicked()),
                SLOT(invert()));

  // Connect split button
  this->connect(d->SplitPushButton,
                SIGNAL(clicked()),
                SLOT(split()));

  // Connect node selector with module itself
  this->connect(d->TransformNodeSelector,
                SIGNAL(currentNodeChanged(vtkDMMLNode*)),
                SLOT(onNodeSelected(vtkDMMLNode*)));

  // Set a static min/max range to let users freely enter values
  d->MatrixWidget->setRange(-1e10, 1e10);

  // Homogeneous transformation matrix is expected to have (0,0,0,1)
  // in its last row, so do not allow users to edit the last row.
  for (int col = 0; col < 4; col++)
    {
    QTableWidgetItem* item = d->MatrixWidget->widgetItem(3, col);
    if (!item)
      {
      continue;
      }
    item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
    }

  d->RotationSliders->setSingleStep(0.1);
  d->RotationSliders->setDecimals(1);

  // Transform nodes connection
  this->connect(d->TransformToolButton, SIGNAL(clicked()),
                SLOT(transformSelectedNodes()));
  this->connect(d->UntransformToolButton, SIGNAL(clicked()),
                SLOT(untransformSelectedNodes()));
  this->connect(d->HardenToolButton, SIGNAL(clicked()),
                SLOT(hardenSelectedNodes()));

  // Observe display section, if opened, then add display node
  this->connect(d->DisplayCollapsibleButton,
                SIGNAL(clicked(bool)),
                SLOT(onDisplaySectionClicked(bool)));

  // Observe Apply transform section to maintain a nice layout
  // even when the section is closed.
  this->connect(d->TransformedCollapsibleButton,
                SIGNAL(clicked(bool)),
                SLOT(onTransformableSectionClicked(bool)));

  // Connect copy and paste actions
  d->CopyTransformToolButton->setDefaultAction(d->CopyAction);
  this->connect(d->CopyAction,
                SIGNAL(triggered()),
                SLOT(copyTransform()));

  d->PasteTransformToolButton->setDefaultAction(d->PasteAction);
  this->connect(d->PasteAction,
                SIGNAL(triggered()),
                SLOT(pasteTransform()));

  // Icons
  QIcon rightIcon =
    QApplication::style()->standardIcon(QStyle::SP_ArrowRight);
  d->TransformToolButton->setIcon(rightIcon);

  QIcon leftIcon =
    QApplication::style()->standardIcon(QStyle::SP_ArrowLeft);
  d->UntransformToolButton->setIcon(leftIcon);

  // Connect convert button
  this->connect(d->ConvertPushButton,
    SIGNAL(clicked()),
    SLOT(convert()));

  // Connect node convert input/output node selectors
  this->connect(d->ConvertReferenceVolumeNodeComboBox,
    SIGNAL(currentNodeChanged(vtkDMMLNode*)),
    SLOT(updateConvertButtonState()));
  this->connect(d->ConvertOutputDisplacementFieldNodeComboBox,
    SIGNAL(currentNodeChanged(vtkDMMLNode*)),
    SLOT(updateConvertButtonState()));

  this->onTransformableSectionClicked(d->TransformedCollapsibleButton->isChecked());
  this->onNodeSelected(nullptr);
  this->updateConvertButtonState();
}

//-----------------------------------------------------------------------------
void qCjyxTransformsModuleWidget::onTranslateFirstButtonPressed(bool checked)
{
  Q_D(qCjyxTransformsModuleWidget);

  qDMMLTransformSliders::CoordinateReferenceType ref =
    checked ? qDMMLTransformSliders::LOCAL : qDMMLTransformSliders::GLOBAL;
  d->TranslationSliders->setCoordinateReference(ref);
  d->RotationSliders->setCoordinateReference(ref);
}

//-----------------------------------------------------------------------------
void qCjyxTransformsModuleWidget::onNodeSelected(vtkDMMLNode* node)
{
  Q_D(qCjyxTransformsModuleWidget);

  vtkDMMLTransformNode* transformNode = vtkDMMLTransformNode::SafeDownCast(node);

  bool isLinearTransform = (transformNode!=nullptr && transformNode->IsLinear());
  bool isCompositeTransform = (transformNode!=nullptr && transformNode->IsComposite());

  // Enable/Disable CoordinateReference, identity, split buttons, MatrixViewGroupBox, and
  // Min/Max translation inputs

  d->InvertPushButton->setEnabled(transformNode != nullptr);

  d->TranslateFirstToolButton->setEnabled(isLinearTransform);
  d->IdentityPushButton->setEnabled(isLinearTransform);
  d->MatrixViewGroupBox->setEnabled(isLinearTransform);

  d->TranslateFirstToolButton->setVisible(isLinearTransform);
  d->MatrixViewGroupBox->setVisible(isLinearTransform);
  d->TranslationSliders->setVisible(isLinearTransform);
  d->RotationSliders->setVisible(isLinearTransform);

  d->CopyTransformToolButton->setVisible(isLinearTransform);
  d->PasteTransformToolButton->setVisible(isLinearTransform);

  d->SplitPushButton->setVisible(isCompositeTransform);

  QStringList nodeTypes;
  // If no transform node, it would show the entire scene, lets shown none
  // instead.
  if (transformNode == nullptr)
    {
    nodeTypes << QString("vtkDMMLNotANode");
    }
  d->TransformedTreeView->setNodeTypes(nodeTypes);

  // Filter the current node in the transformed tree view
  d->TransformedTreeView->setRootNode(transformNode);

  // Hide the current node in the transformable tree view
  QStringList hiddenNodeIDs;
  if (transformNode)
    {
    hiddenNodeIDs << QString(transformNode->GetID());
    }
  d->TransformableTreeView->sortFilterProxyModel()
    ->setHiddenNodeIDs(hiddenNodeIDs);

  this->qvtkReconnect(d->DMMLTransformNode, transformNode,
                      vtkDMMLTransformableNode::TransformModifiedEvent,
                      this, SLOT(onDMMLTransformNodeModified(vtkObject*)));

  if (d->DMMLTransformNode == nullptr && transformNode != nullptr)
    {
    d->TransformedCollapsibleButton->setCollapsed(false);
    }

  d->DMMLTransformNode = transformNode;

  // If there is no display node then collapse the display section.
  // This allows creation of transform display nodes on request:
  // the display node is created if the user expands the display section.
  vtkDMMLTransformDisplayNode* dispNode = nullptr;
  if (transformNode)
    {
    dispNode = vtkDMMLTransformDisplayNode::SafeDownCast(transformNode->GetDisplayNode());
    }
  if (dispNode==nullptr)
    {
    d->DisplayCollapsibleButton->setCollapsed(true);
    }
}

//-----------------------------------------------------------------------------
void qCjyxTransformsModuleWidget::onDMMLTransformNodeModified(vtkObject* caller)
{
  Q_D(qCjyxTransformsModuleWidget);

  vtkDMMLTransformNode* transformNode = vtkDMMLTransformNode::SafeDownCast(caller);
  if (!transformNode)
    {
    return;
    }
  Q_ASSERT(d->DMMLTransformNode == transformNode);

  bool isLinearTransform = transformNode->IsLinear();
  bool isCompositeTransform = transformNode->IsComposite();

  d->TranslateFirstToolButton->setEnabled(isLinearTransform);
  d->IdentityPushButton->setEnabled(isLinearTransform);
  d->MatrixViewGroupBox->setEnabled(isLinearTransform);

  // This method may be called very frequently (when transform is changing
  // in real time). Due to some reason setVisible calls take time,
  // even if the visibility state does not change.
  // To save time, only call the set function if the visibility has to be changed.
  if (isLinearTransform!=d->TranslateFirstToolButton->isVisible())
    {
    d->TranslateFirstToolButton->setVisible(isLinearTransform);
    }
  if (isLinearTransform!=d->MatrixViewGroupBox->isVisible())
    {
    d->MatrixViewGroupBox->setVisible(isLinearTransform);
    }
  if (isLinearTransform!=d->TranslationSliders->isVisible())
    {
    d->TranslationSliders->setVisible(isLinearTransform);
    }
  if (isLinearTransform!=d->RotationSliders->isVisible())
    {
    d->RotationSliders->setVisible(isLinearTransform);
    }
  if (isLinearTransform!=d->CopyTransformToolButton->isVisible())
    {
    d->CopyTransformToolButton->setVisible(isLinearTransform);
    }
  if (isLinearTransform!=d->PasteTransformToolButton->isVisible())
    {
    d->PasteTransformToolButton->setVisible(isLinearTransform);
    }
  if (isCompositeTransform!=d->SplitPushButton->isVisible())
    {
    d->SplitPushButton->setVisible(isCompositeTransform);
    }
}

//-----------------------------------------------------------------------------
void qCjyxTransformsModuleWidget::identity()
{
  Q_D(qCjyxTransformsModuleWidget);

  if (d->DMMLTransformNode==nullptr || !d->DMMLTransformNode->IsLinear())
    {
    return;
    }

  d->TranslationSliders->resetUnactiveSliders();
  d->RotationSliders->resetUnactiveSliders();

  vtkNew<vtkMatrix4x4> matrix; // initialized to identity by default
  d->DMMLTransformNode->SetMatrixTransformToParent(matrix.GetPointer());
}

//-----------------------------------------------------------------------------
void qCjyxTransformsModuleWidget::invert()
{
  Q_D(qCjyxTransformsModuleWidget);

  if (!d->DMMLTransformNode) { return; }

  d->TranslationSliders->resetUnactiveSliders();
  d->RotationSliders->resetUnactiveSliders();

  DMMLNodeModifyBlocker blocker(d->DMMLTransformNode);
  d->DMMLTransformNode->Inverse();
  d->DMMLTransformNode->InverseName();
}

//-----------------------------------------------------------------------------
void qCjyxTransformsModuleWidget::split()
{
  Q_D(qCjyxTransformsModuleWidget);

  if (d->DMMLTransformNode==nullptr)
    {
    return;
    }

  d->DMMLTransformNode->Split();
}

//-----------------------------------------------------------------------------
void qCjyxTransformsModuleWidget::copyTransform()
{
  Q_D(qCjyxTransformsModuleWidget);

  vtkLinearTransform* linearTransform =
      vtkLinearTransform::SafeDownCast(d->DMMLTransformNode->GetTransformToParent());
  if (!linearTransform)
    {
    // Silent fail, no worries!
    qWarning() << "Unable to cast parent transform as a vtkLinearTransform";
    return;
    }

  vtkMatrix4x4* internalMatrix = linearTransform->GetMatrix();
  std::string delimiter = " ";
  std::string rowDelimiter = "\n";
  std::string output = vtkAddonMathUtilities::ToString(internalMatrix, delimiter, rowDelimiter);
  QApplication::clipboard()->setText(QString::fromStdString(output));
}

//-----------------------------------------------------------------------------
void qCjyxTransformsModuleWidget::pasteTransform()
{
  Q_D(qCjyxTransformsModuleWidget);

  vtkNew<vtkMatrix4x4> tempMatrix;

  std::string text = QApplication::clipboard()->text().toStdString();
  bool success = vtkAddonMathUtilities::FromString(tempMatrix.GetPointer(), text);
  if (!success)
    {
    qWarning() << "Cannot convert pasted string to matrix.";
    return;
    }
  // Homogeneous transformation matrix is expected to have (0,0,0,1)
  // in its last row.
  tempMatrix->SetElement(3, 0, 0.0);
  tempMatrix->SetElement(3, 1, 0.0);
  tempMatrix->SetElement(3, 2, 0.0);
  tempMatrix->SetElement(3, 3, 1.0);
  d->DMMLTransformNode->SetMatrixTransformToParent(tempMatrix.GetPointer());
}

//-----------------------------------------------------------------------------
int qCjyxTransformsModuleWidget::coordinateReference()const
{
  Q_D(const qCjyxTransformsModuleWidget);
  return (d->TranslateFirstToolButton->isChecked() ? qDMMLTransformSliders::LOCAL : qDMMLTransformSliders::GLOBAL);
}

//-----------------------------------------------------------------------------
void qCjyxTransformsModuleWidget::setDMMLScene(vtkDMMLScene* scene)
{
  Q_D(qCjyxTransformsModuleWidget);
  this->Superclass::setDMMLScene(scene);
  // If the root index is set before the scene, it will show the scene as
  // top-level item. Setting the root index to be the scene makes the nodes
  // top-level, and this can only be done once the scene is set.
  d->TransformableTreeView->setRootIndex(
    d->TransformableTreeView->sortFilterProxyModel()->dmmlSceneIndex());
}

//-----------------------------------------------------------------------------
void qCjyxTransformsModuleWidget::transformSelectedNodes()
{
  Q_D(qCjyxTransformsModuleWidget);
  QList<vtkSmartPointer<vtkDMMLTransformableNode> > nodesToTransform =
    qCjyxTransformsModuleWidgetPrivate::getSelectedNodes(d->TransformableTreeView);
  foreach(vtkSmartPointer<vtkDMMLTransformableNode> node, nodesToTransform)
    {
    node->SetAndObserveTransformNodeID(d->DMMLTransformNode->GetID());
    }
}

//-----------------------------------------------------------------------------
void qCjyxTransformsModuleWidget::untransformSelectedNodes()
{
  Q_D(qCjyxTransformsModuleWidget);
  QList<vtkSmartPointer<vtkDMMLTransformableNode> > nodesToTransform =
    qCjyxTransformsModuleWidgetPrivate::getSelectedNodes(d->TransformedTreeView);
  foreach(vtkSmartPointer<vtkDMMLTransformableNode> node, nodesToTransform)
    {
    node->SetAndObserveTransformNodeID(nullptr);
    }
}

//-----------------------------------------------------------------------------
void qCjyxTransformsModuleWidget::hardenSelectedNodes()
{
  Q_D(qCjyxTransformsModuleWidget);
  QList<vtkSmartPointer<vtkDMMLTransformableNode> > nodesToTransform =
    qCjyxTransformsModuleWidgetPrivate::getSelectedNodes(d->TransformedTreeView);
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  foreach(vtkSmartPointer<vtkDMMLTransformableNode> node, nodesToTransform)
    {
    d->logic()->hardenTransform(vtkDMMLTransformableNode::SafeDownCast(node));
    }
  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
void qCjyxTransformsModuleWidget::onDisplaySectionClicked(bool clicked)
{
  Q_D(qCjyxTransformsModuleWidget);
  // If the display section is opened and there is no display node then create one
  if (!clicked)
    {
    return;
    }
  if (d->DMMLTransformNode==nullptr)
    {
    return;
    }
  if (vtkDMMLTransformDisplayNode::SafeDownCast(d->DMMLTransformNode->GetDisplayNode())==nullptr)
    {
    d->DMMLTransformNode->CreateDefaultDisplayNodes();
    // Refresh the display node section
    d->TransformDisplayNodeWidget->setDMMLTransformNode(d->DMMLTransformNode);
    }
}

//-----------------------------------------------------------------------------
void qCjyxTransformsModuleWidget::onTransformableSectionClicked(bool clicked)
{
  Q_D(qCjyxTransformsModuleWidget);
  if (clicked)
    {
    // the transformable section is open, so no need for spacer
    d->BottomSpacer->changeSize(0,0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
  else
    {
    // the transformable section is open, add spacer to prevent stretching of
    // the remaining sections
    d->BottomSpacer->changeSize(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding);
    }
}

//-----------------------------------------------------------------------------
void qCjyxTransformsModuleWidget::convert()
{
  Q_D(qCjyxTransformsModuleWidget);
  if (d->DMMLTransformNode == nullptr)
    {
    qWarning("qCjyxTransformsModuleWidget::convert failed: DMMLTransformNode is invalid");
    return;
    }
  if (d->ConvertReferenceVolumeNodeComboBox->currentNode() == nullptr)
    {
    qWarning("qCjyxTransformsModuleWidget::convert failed: reference volume node is invalid");
    return;
    }
  if (d->ConvertOutputDisplacementFieldNodeComboBox->currentNode() == nullptr)
    {
    qWarning("qCjyxTransformsModuleWidget::convert failed: reference volume node is invalid");
    return;
    }
  vtkDMMLScalarVolumeNode* scalarOutputVolumeNode = vtkDMMLScalarVolumeNode::SafeDownCast(d->ConvertOutputDisplacementFieldNodeComboBox->currentNode());
  vtkDMMLVectorVolumeNode* vectorOutputVolumeNode = vtkDMMLVectorVolumeNode::SafeDownCast(d->ConvertOutputDisplacementFieldNodeComboBox->currentNode());
  vtkDMMLTransformNode* outputTransformNode = vtkDMMLTransformNode::SafeDownCast(d->ConvertOutputDisplacementFieldNodeComboBox->currentNode());
  vtkDMMLVolumeNode* referenceVolumeNode = vtkDMMLVolumeNode::SafeDownCast(d->ConvertReferenceVolumeNodeComboBox->currentNode());
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  vtkDMMLNode* resultNode = nullptr;
  if (vectorOutputVolumeNode)
    {
    // this must be checked before scalarOutputVolumeNode, as vtkDMMLVectorVolumeNode is a vtkDMMLScalarVolumeNode as well
    resultNode = d->logic()->CreateDisplacementVolumeFromTransform(d->DMMLTransformNode, referenceVolumeNode, false /*magnitudeOnly*/, vectorOutputVolumeNode);
    }
  else if (scalarOutputVolumeNode)
    {
    resultNode = d->logic()->CreateDisplacementVolumeFromTransform(d->DMMLTransformNode, referenceVolumeNode, true /*magnitudeOnly*/, scalarOutputVolumeNode);
    }
  else if (outputTransformNode)
    {
    resultNode = d->logic()->ConvertToGridTransform(d->DMMLTransformNode, referenceVolumeNode, outputTransformNode);
    }
  else
    {
    qWarning("qCjyxTransformsModuleWidget::convert failed: invalid output node type");
    }
  QApplication::restoreOverrideCursor();
  if (resultNode == nullptr)
    {
    QMessageBox::warning(this, tr("Conversion failed"), tr("Failed to convert transform. See application log for details."));
    }
}

//-----------------------------------------------------------------------------
void qCjyxTransformsModuleWidget::updateConvertButtonState()
{
  Q_D(qCjyxTransformsModuleWidget);
  bool enableConvert = (d->DMMLTransformNode != nullptr
    && d->ConvertReferenceVolumeNodeComboBox->currentNode() != nullptr
    && d->ConvertOutputDisplacementFieldNodeComboBox->currentNode() != nullptr);
  d->ConvertPushButton->setEnabled(enableConvert);
}

//-----------------------------------------------------------
bool qCjyxTransformsModuleWidget::setEditedNode(vtkDMMLNode* node,
                                                  QString role /* = QString()*/,
                                                  QString context /* = QString()*/)
{
  Q_D(qCjyxTransformsModuleWidget);
  Q_UNUSED(role);
  Q_UNUSED(context);
  if (vtkDMMLTransformNode::SafeDownCast(node))
    {
    d->TransformNodeSelector->setCurrentNode(node);
    return true;
    }

  if (vtkDMMLTransformDisplayNode::SafeDownCast(node))
    {
    vtkDMMLTransformDisplayNode* displayNode = vtkDMMLTransformDisplayNode::SafeDownCast(node);
    vtkDMMLTransformNode* displayableNode = vtkDMMLTransformNode::SafeDownCast(displayNode->GetDisplayableNode());
    if (!displayableNode)
      {
      return false;
      }
    d->TransformNodeSelector->setCurrentNode(displayableNode);
    return true;
    }

  return false;
}
