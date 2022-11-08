// Qt includes
#include <QDebug>
#include <QMessageBox>

// CTK includes
#include <ctkFlowLayout.h>

// VTK includes
#include <vtkNew.h>
#include <vtkMatrix4x4.h>
#include <vtkImageData.h>

// Cjyx includes
#include <qCjyxAbstractCoreModule.h>
#include <qCjyxApplication.h>
#include <qCjyxLayoutManager.h>

// CropVolume includes
#include "qCjyxCropVolumeModuleWidget.h"
#include "ui_qCjyxCropVolumeModuleWidget.h"

// CropVolume Logic includes
#include <vtkCjyxCropVolumeLogic.h>

// qDMML includes
#include <qDMMLNodeFactory.h>
#include <qDMMLSliceWidget.h>

// DMML includes
#include <vtkDMMLAnnotationROINode.h>
#include <vtkDMMLApplicationLogic.h>
#include <vtkDMMLCropVolumeParametersNode.h>
#include <vtkDMMLMarkupsROINode.h>
#include <vtkDMMLSelectionNode.h>
#include <vtkDMMLSliceCompositeNode.h>
#include <vtkDMMLSliceLogic.h>
#include <vtkDMMLTransformNode.h>
#include <vtkDMMLVolumeNode.h>

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_CropVolume
class qCjyxCropVolumeModuleWidgetPrivate: public Ui_qCjyxCropVolumeModuleWidget
{
  Q_DECLARE_PUBLIC(qCjyxCropVolumeModuleWidget);
protected:
  qCjyxCropVolumeModuleWidget* const q_ptr;
public:

  qCjyxCropVolumeModuleWidgetPrivate(qCjyxCropVolumeModuleWidget& object);
  ~qCjyxCropVolumeModuleWidgetPrivate();

  vtkCjyxCropVolumeLogic* logic() const;

  /// Return true if inputs are correct, cropping may be enabled
  bool checkInputs(bool& autoFixAvailable, QString& message, bool autoFixProblems);

  vtkWeakPointer<vtkDMMLCropVolumeParametersNode> ParametersNode;
  vtkWeakPointer<vtkDMMLVolumeNode> InputVolumeNode;
  vtkWeakPointer<vtkDMMLTransformableNode> InputROINode;
};

//-----------------------------------------------------------------------------
// qCjyxCropVolumeModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qCjyxCropVolumeModuleWidgetPrivate::qCjyxCropVolumeModuleWidgetPrivate(qCjyxCropVolumeModuleWidget& object) : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qCjyxCropVolumeModuleWidgetPrivate::~qCjyxCropVolumeModuleWidgetPrivate() = default;

//-----------------------------------------------------------------------------
vtkCjyxCropVolumeLogic* qCjyxCropVolumeModuleWidgetPrivate::logic() const
{
  Q_Q(const qCjyxCropVolumeModuleWidget);
  return vtkCjyxCropVolumeLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
bool qCjyxCropVolumeModuleWidgetPrivate::checkInputs(bool& autoFixAvailable, QString& message, bool autoFixProblems)
{
  message.clear();
  autoFixAvailable = false;

  if (this->ParametersNode == nullptr)
    {
    message = qCjyxCropVolumeModuleWidget::tr("Select or create a new parameter node.");
    autoFixAvailable = true;
    if (autoFixProblems)
      {
      this->ParametersNodeComboBox->addNode();
      }
    return false;
    }

  if (this->ParametersNode->GetInputVolumeNode() == nullptr)
    {
    message = qCjyxCropVolumeModuleWidget::tr("Select an input volume.");
    return false;
    }

  if (this->ParametersNode->GetOutputVolumeNode() && this->ParametersNode->GetInputVolumeNode()
    && strcmp(this->ParametersNode->GetOutputVolumeNode()->GetClassName(), this->ParametersNode->GetInputVolumeNode()->GetClassName()) != 0)
    {
    message = qCjyxCropVolumeModuleWidget::tr("Output volume type does not match input volume type.");
    autoFixAvailable = true;
    if (autoFixProblems)
      {
      // create new node automatically
      this->ParametersNode->SetOutputVolumeNodeID(nullptr);
      }
    return false;
    }

  bool roiExists = true;
  bool inputVolumeTransformValid = true;
  bool roiTransformValid = true;
  bool outputVolumeTransformValid = true;
  QStringList problemsDescription;

  // Common cropping problems
  if (this->ParametersNode->GetROINode())
    {
    if (this->ParametersNode->GetROINode()->GetParentTransformNode()
      && !this->ParametersNode->GetROINode()->GetParentTransformNode()->IsTransformToWorldLinear())
      {
      roiTransformValid = false;
      problemsDescription << qCjyxCropVolumeModuleWidget::tr("Input ROI is under a non-linear tansform.");
      }
    }
  else
    {
    roiExists = false;
    problemsDescription << qCjyxCropVolumeModuleWidget::tr("Select or create a new input ROI.");
    }
  if (this->ParametersNode->GetOutputVolumeNode()
    && this->ParametersNode->GetOutputVolumeNode()->GetParentTransformNode()
    && !this->ParametersNode->GetOutputVolumeNode()->GetParentTransformNode()->IsTransformToWorldLinear())
    {
    outputVolumeTransformValid = false;
    problemsDescription << qCjyxCropVolumeModuleWidget::tr("Output volume is under a non-linear tansform.");
    }

  // Non-interpolated cropping problem
  if (this->ParametersNode->GetVoxelBased())
    {
    if (this->ParametersNode->GetInputVolumeNode()
      && this->ParametersNode->GetInputVolumeNode()->GetParentTransformNode()
      && !this->ParametersNode->GetInputVolumeNode()->GetParentTransformNode()->IsTransformToWorldLinear())
      {
      inputVolumeTransformValid = false;
      problemsDescription << qCjyxCropVolumeModuleWidget::tr("Interpolation is disabled and input volume is under a non-linear tansform");
      }
    // Only report ROI errors, if ROI is valid (avoid overloading the user with too much info)
    if (roiExists && roiTransformValid && !vtkCjyxCropVolumeLogic::IsROIAlignedWithInputVolume(this->ParametersNode))
      {
      roiTransformValid = false;
      problemsDescription += qCjyxCropVolumeModuleWidget::tr("Interpolation is disabled and input ROI is not aligned with input volume axes.");
      }
    }

  if (!autoFixProblems || problemsDescription.isEmpty())
    {
    // nothing to fix or fix is not requested
    autoFixAvailable = !problemsDescription.isEmpty();
    message = problemsDescription.join(" ");
    return problemsDescription.isEmpty();
    }

  if (!roiExists)
    {
    this->InputROIComboBox->addNode();
    }

  // Non-interpolated cropping problem
  if (!inputVolumeTransformValid)
    {
    this->ParametersNode->GetInputVolumeNode()->SetAndObserveTransformNodeID(nullptr);
    if (this->ParametersNode->GetVoxelBased())
      {
      this->logic()->SnapROIToVoxelGrid(this->ParametersNode);
      roiTransformValid = true;
      }
    }
  if (!roiTransformValid)
    {
    this->logic()->SnapROIToVoxelGrid(this->ParametersNode);
    }
  if (!outputVolumeTransformValid)
    {
    const char* newParentTransformNodeID = nullptr;
    if (this->ParametersNode->GetInputVolumeNode())
      {
      newParentTransformNodeID = this->ParametersNode->GetInputVolumeNode()->GetTransformNodeID();
      }
    this->ParametersNode->GetOutputVolumeNode()->SetAndObserveTransformNodeID(newParentTransformNodeID);
    }

  message = problemsDescription.join(" ");
  return problemsDescription.isEmpty();
}
//-----------------------------------------------------------------------------
// qCjyxCropVolumeModuleWidget methods

//-----------------------------------------------------------------------------
qCjyxCropVolumeModuleWidget::qCjyxCropVolumeModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qCjyxCropVolumeModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qCjyxCropVolumeModuleWidget::~qCjyxCropVolumeModuleWidget() = default;

//-----------------------------------------------------------------------------
void qCjyxCropVolumeModuleWidget::setup()
{
  Q_D(qCjyxCropVolumeModuleWidget);

  d->setupUi(this);

  this->Superclass::setup();

  connect(d->ParametersNodeComboBox, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
    this, SLOT(setParametersNode(vtkDMMLNode*)));

  connect(d->InputVolumeComboBox, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
    this, SLOT(setInputVolume(vtkDMMLNode*)));

  connect(d->InputROIComboBox, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
          this, SLOT(setInputROI(vtkDMMLNode*)));
  connect(d->InputROIComboBox, SIGNAL(nodeAddedByUser(vtkDMMLNode*)),
          this, SLOT(initializeInputROI(vtkDMMLNode*)));
  connect(d->InputROIComboBox, SIGNAL(nodeAdded(vtkDMMLNode*)),
          this, SLOT(onInputROIAdded(vtkDMMLNode*)));

  connect(d->OutputVolumeComboBox, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
    this, SLOT(setOutputVolume(vtkDMMLNode*)));

  connect(d->VisibilityButton, SIGNAL(toggled(bool)),
          this, SLOT(onROIVisibilityChanged(bool)));
  connect(d->ROIFitPushButton, SIGNAL(clicked()),
    this, SLOT(onROIFit()));

  connect(d->InterpolationEnabledCheckBox, SIGNAL(toggled(bool)),
    this, SLOT(onInterpolationEnabled(bool)));
  connect(d->SpacingScalingSpinBox, SIGNAL(valueChanged(double)),
    this, SLOT(onSpacingScalingValueChanged(double)));
  connect(d->IsotropicCheckbox, SIGNAL(toggled(bool)),
    this, SLOT(onIsotropicModeChanged(bool)));

  connect(d->LinearRadioButton, SIGNAL(toggled(bool)),
          this, SLOT(onInterpolationModeChanged()));
  connect(d->NNRadioButton, SIGNAL(toggled(bool)),
          this, SLOT(onInterpolationModeChanged()));
  connect(d->WSRadioButton, SIGNAL(toggled(bool)),
          this, SLOT(onInterpolationModeChanged()));
  connect(d->BSRadioButton, SIGNAL(toggled(bool)),
          this, SLOT(onInterpolationModeChanged()));

  connect(d->FillValueSpinBox, SIGNAL(valueChanged(double)),
    this, SLOT(onFillValueChanged(double)));

  // Observe info section, only update content if opened
  this->connect(d->VolumeInformationCollapsibleButton,
    SIGNAL(clicked(bool)),
    SLOT(onVolumeInformationSectionClicked(bool)));

  d->InputErrorLabel->setVisible(false);
  d->InputErrorFixButton->setVisible(false);
  connect(d->InputErrorFixButton, SIGNAL(clicked()),
    this, SLOT(onFixAlignment()));

  connect(d->CropButton, SIGNAL(clicked()),
    this, SLOT(onApply()));

}

//-----------------------------------------------------------------------------
void qCjyxCropVolumeModuleWidget::enter()
{
  Q_D(qCjyxCropVolumeModuleWidget);

  // For user's convenience, create a default ROI parameter node if
  // none exists yet.
  vtkDMMLScene* scene = this->dmmlScene();
  if (!scene)
    {
    qWarning("qCjyxCropVolumeModuleWidget::enter: invalid scene");
    return;
    }
  if (scene->GetNumberOfNodesByClass("vtkDMMLCropVolumeParametersNode") == 0)
    {
    vtkNew<vtkDMMLCropVolumeParametersNode> parametersNode;
    scene->AddNode(parametersNode.GetPointer());

    // Use first background volume node in any of the displayed slice views as input volume
    qCjyxApplication * app = qCjyxApplication::application();
    if (app && app->layoutManager())
      {
      foreach(QString sliceViewName, app->layoutManager()->sliceViewNames())
        {
        qDMMLSliceWidget* sliceWidget = app->layoutManager()->sliceWidget(sliceViewName);
        const char* backgroundVolumeNodeID = sliceWidget->sliceLogic()->GetSliceCompositeNode()->GetBackgroundVolumeID();
        if (backgroundVolumeNodeID != nullptr)
          {
          parametersNode->SetInputVolumeNodeID(backgroundVolumeNodeID);
          break;
          }
        }
      }

    // Use first visible ROI node (or last ROI node, if all are invisible)
    vtkDMMLDisplayableNode* foundROINode = nullptr;
    bool foundROINodeVisible = false;
    std::vector<vtkDMMLNode *> roiNodes;

    scene->GetNodesByClass("vtkDMMLMarkupsROINode", roiNodes);
    for (unsigned int i = 0; i < roiNodes.size(); ++i)
      {
      vtkDMMLDisplayableNode* roiNode = vtkDMMLDisplayableNode::SafeDownCast(roiNodes[i]);
      if (!roiNode)
        {
        continue;
        }
      foundROINode = roiNode;
      if (foundROINode->GetDisplayVisibility())
        {
        foundROINodeVisible = true;
        break;
        }
      }

    if (!foundROINodeVisible)
      {
      roiNodes.clear();
      scene->GetNodesByClass("vtkDMMLAnnotationROINode", roiNodes);
      for (unsigned int i = 0; i < roiNodes.size(); ++i)
        {
        vtkDMMLAnnotationROINode* roiNode = vtkDMMLAnnotationROINode::SafeDownCast(roiNodes[i]);
        if (!roiNode)
          {
          continue;
          }
        foundROINode = roiNode;
        if (foundROINode->GetDisplayVisibility())
          {
          break;
          }
        }
      }
    if (foundROINode)
      {
      parametersNode->SetROINodeID(foundROINode->GetID());
      }

    d->ParametersNodeComboBox->setCurrentNode(parametersNode.GetPointer());
    }
  else
    {
    // There is at least one parameter node.
    // If none is selected then select the first one.
    if (d->ParametersNodeComboBox->currentNode() == nullptr)
      {
      d->ParametersNodeComboBox->setCurrentNode(scene->GetFirstNodeByClass("vtkDMMLCropVolumeParametersNode"));
      }
    }

  this->Superclass::enter();

  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qCjyxCropVolumeModuleWidget::setDMMLScene(vtkDMMLScene* scene)
{
  this->Superclass::setDMMLScene(scene);
  // observe close event so can re-add a parameters node if necessary
  qvtkReconnect(this->dmmlScene(), vtkDMMLScene::EndImportEvent, this, SLOT(onDMMLSceneEndBatchProcessEvent()));
  qvtkReconnect(this->dmmlScene(), vtkDMMLScene::EndBatchProcessEvent, this, SLOT(onDMMLSceneEndBatchProcessEvent()));
  qvtkReconnect(this->dmmlScene(), vtkDMMLScene::EndCloseEvent, this, SLOT(onDMMLSceneEndBatchProcessEvent()));
  qvtkReconnect(this->dmmlScene(), vtkDMMLScene::EndRestoreEvent, this, SLOT(onDMMLSceneEndBatchProcessEvent()));
  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qCjyxCropVolumeModuleWidget::initializeInputROI(vtkDMMLNode *roiNode)
{
  Q_D(const qCjyxCropVolumeModuleWidget);
  if (!d->ParametersNode || !d->ParametersNode->GetInputVolumeNode())
    {
    return;
    }

  this->setInputROI(roiNode);
  this->onROIFit();
  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qCjyxCropVolumeModuleWidget::onApply()
{
  Q_D(qCjyxCropVolumeModuleWidget);

  if(!d->ParametersNode.GetPointer() ||
    !d->ParametersNode->GetInputVolumeNode() ||
    !d->ParametersNode->GetROINode())
    {
    qWarning() << Q_FUNC_INFO << ": invalid inputs";
    return;
    }

  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  vtkDMMLNode* oldOutputNode = d->ParametersNode->GetOutputVolumeNode();
  if (!d->logic()->Apply(d->ParametersNode))
    {
    // no errors
    if (d->ParametersNode->GetOutputVolumeNode() != oldOutputNode)
      {
      // New output volume is created, show it in slice viewers
      vtkCjyxApplicationLogic *appLogic = this->module()->appLogic();
      vtkDMMLSelectionNode *selectionNode = appLogic->GetSelectionNode();
      selectionNode->SetActiveVolumeID(d->ParametersNode->GetOutputVolumeNodeID());
      appLogic->PropagateVolumeSelection();
      }
    }
  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
void qCjyxCropVolumeModuleWidget::onFixAlignment()
{
  Q_D(qCjyxCropVolumeModuleWidget);
  bool autoFixAvailable = false;
  QString errorMessages;
  d->checkInputs(autoFixAvailable, errorMessages, true /* auto-fix problems */ );
  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qCjyxCropVolumeModuleWidget::setInputVolume(vtkDMMLNode* volumeNode)
{
  Q_D(qCjyxCropVolumeModuleWidget);

  if (!d->ParametersNode.GetPointer())
    {
    // setInputVolume may be triggered by calling setScene on InputVolumeComboBox
    // before ParametersNodeComboBox is initialized, so don't log a warning here
    return;
    }

  qvtkReconnect(d->InputVolumeNode, volumeNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromDMML()));
  d->InputVolumeNode = vtkDMMLVolumeNode::SafeDownCast(volumeNode);
  d->ParametersNode->SetInputVolumeNodeID(volumeNode ? volumeNode->GetID() : nullptr);
}

//-----------------------------------------------------------------------------
void qCjyxCropVolumeModuleWidget::setOutputVolume(vtkDMMLNode* volumeNode)
{
  Q_D(qCjyxCropVolumeModuleWidget);

  vtkDMMLCropVolumeParametersNode *parametersNode = vtkDMMLCropVolumeParametersNode::SafeDownCast(d->ParametersNodeComboBox->currentNode());
  if (!parametersNode)
    {
    if (volumeNode != nullptr)
      {
      qWarning() << Q_FUNC_INFO << ": invalid parameter node";
      }
    return;
    }

  parametersNode->SetOutputVolumeNodeID(volumeNode ? volumeNode->GetID() : nullptr);
}

//-----------------------------------------------------------------------------
void qCjyxCropVolumeModuleWidget::setInputROI(vtkDMMLNode* node)
{
  Q_D(qCjyxCropVolumeModuleWidget);

  if (!d->ParametersNode.GetPointer())
    {
    if (node != nullptr)
      {
      qWarning() << Q_FUNC_INFO << ": invalid parameter node";
      }
    return;
    }
  vtkDMMLTransformableNode* roiNode = nullptr;
  if (vtkDMMLAnnotationROINode::SafeDownCast(node) || vtkDMMLMarkupsROINode::SafeDownCast(node))
    {
    roiNode = vtkDMMLTransformableNode::SafeDownCast(node);
    }
  qvtkReconnect(d->InputROINode, roiNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromDMML()));
  d->InputROINode = roiNode;
  d->ParametersNode->SetROINodeID(roiNode ? roiNode->GetID() : nullptr);
}

//-----------------------------------------------------------------------------
void qCjyxCropVolumeModuleWidget::onInputROIAdded(vtkDMMLNode* node)
{
  Q_D(qCjyxCropVolumeModuleWidget);

  if (!d->ParametersNode.GetPointer())
    {
    return;
    }
  if (!d->ParametersNode->GetROINode() && node)
    {
    // There was no ROI selected and the user just added one
    // use that for cropping.
    d->ParametersNode->SetROINodeID(node->GetID());
    }

  // Turn off filling and enable interactive resizing. Semi-transparent actors may introduce volume rendering artifacts in certain
  // configurations and the filling also makes the views a bit more complex.
  vtkDMMLMarkupsROINode* markupsRoi = vtkDMMLMarkupsROINode::SafeDownCast(node);
  if (markupsRoi)
    {
    markupsRoi->CreateDefaultDisplayNodes();
    vtkDMMLMarkupsDisplayNode* displayNode = vtkDMMLMarkupsDisplayNode::SafeDownCast(markupsRoi->GetDisplayNode());
    if (displayNode)
      {
      displayNode->SetHandlesInteractive(true);
      displayNode->SetFillVisibility(false);
      }
    }
}

//-----------------------------------------------------------------------------
void qCjyxCropVolumeModuleWidget::onROIVisibilityChanged(bool visible)
{
  Q_D(qCjyxCropVolumeModuleWidget);
  if (!d->ParametersNode || !d->ParametersNode->GetROINode())
    {
    return;
    }
  d->ParametersNode->GetROINode()->SetDisplayVisibility(visible);
}

//-----------------------------------------------------------------------------
void qCjyxCropVolumeModuleWidget::onROIFit()
{
  Q_D(qCjyxCropVolumeModuleWidget);
  d->logic()->SnapROIToVoxelGrid(d->ParametersNode);
  d->logic()->FitROIToInputVolume(d->ParametersNode);
}

//-----------------------------------------------------------------------------
void qCjyxCropVolumeModuleWidget::onInterpolationModeChanged()
{
  Q_D(qCjyxCropVolumeModuleWidget);
  if (!d->ParametersNode)
    {
    return;
    }
  if(d->NNRadioButton->isChecked())
    {
    d->ParametersNode->SetInterpolationMode(vtkDMMLCropVolumeParametersNode::InterpolationNearestNeighbor);
    }
  if(d->LinearRadioButton->isChecked())
    {
    d->ParametersNode->SetInterpolationMode(vtkDMMLCropVolumeParametersNode::InterpolationLinear);
    }
  if(d->WSRadioButton->isChecked())
    {
    d->ParametersNode->SetInterpolationMode(vtkDMMLCropVolumeParametersNode::InterpolationWindowedSinc);
    }
  if(d->BSRadioButton->isChecked())
    {
    d->ParametersNode->SetInterpolationMode(vtkDMMLCropVolumeParametersNode::InterpolationBSpline);
    }
}

//-----------------------------------------------------------------------------
void qCjyxCropVolumeModuleWidget::onSpacingScalingValueChanged(double s)
{
  Q_D(qCjyxCropVolumeModuleWidget);
  if (!d->ParametersNode)
    {
    return;
    }
  d->ParametersNode->SetSpacingScalingConst(s);
}

//-----------------------------------------------------------------------------
void qCjyxCropVolumeModuleWidget::onFillValueChanged(double s)
{
  Q_D(qCjyxCropVolumeModuleWidget);
  if (!d->ParametersNode)
  {
    return;
  }
  d->ParametersNode->SetFillValue(s);
}

//-----------------------------------------------------------------------------
void qCjyxCropVolumeModuleWidget::onIsotropicModeChanged(bool isotropic)
{
  Q_D(qCjyxCropVolumeModuleWidget);
  if (!d->ParametersNode)
    {
    return;
    }
  d->ParametersNode->SetIsotropicResampling(isotropic);
}

//-----------------------------------------------------------------------------
void
qCjyxCropVolumeModuleWidget::onInterpolationEnabled(bool interpolationEnabled)
{
  Q_D(qCjyxCropVolumeModuleWidget);

  if (!d->ParametersNode)
    {
    return;
    }
  d->ParametersNode->SetVoxelBased(!interpolationEnabled);
}

//-----------------------------------------------------------------------------
void qCjyxCropVolumeModuleWidget::onDMMLSceneEndBatchProcessEvent()
{
  if (!this->dmmlScene() || this->dmmlScene()->IsBatchProcessing())
    {
    return;
    }
  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qCjyxCropVolumeModuleWidget::updateWidgetFromDMML()
{
  Q_D(qCjyxCropVolumeModuleWidget);
  if (!this->dmmlScene())
    {
    return;
    }

  QString inputCheckErrorMessage;
  bool autoFixAvailable = false;
  if (d->checkInputs(autoFixAvailable, inputCheckErrorMessage, false))
    {
    d->InputErrorLabel->setVisible(false);
    d->InputErrorFixButton->setVisible(false);
    }
  else
    {
    inputCheckErrorMessage.prepend("<span style = \"color:#FF0000;\">");
    inputCheckErrorMessage.append("</span>");
    d->InputErrorLabel->setText(inputCheckErrorMessage);
    d->InputErrorLabel->setVisible(true);
    d->InputErrorFixButton->setVisible(autoFixAvailable);
    }

  if (!d->ParametersNode)
    {
    // reset widget to defaults from node class
    d->InputVolumeComboBox->setCurrentNode(nullptr);
    d->InputROIComboBox->setCurrentNode(nullptr);
    d->OutputVolumeComboBox->setCurrentNode(nullptr);

    d->InterpolationEnabledCheckBox->setChecked(true);
    d->VisibilityButton->setChecked(true);

    d->IsotropicCheckbox->setChecked(false);
    d->SpacingScalingSpinBox->setValue(1.0);
    d->LinearRadioButton->setChecked(true);
    d->FillValueSpinBox->setValue(0.0);

    this->updateVolumeInfo();

    d->CropButton->setEnabled(false);
    return;
    }

  d->CropButton->setEnabled(inputCheckErrorMessage.isEmpty());

  d->InputVolumeComboBox->setCurrentNode(d->ParametersNode->GetInputVolumeNode());
  d->InputROIComboBox->setCurrentNode(d->ParametersNode->GetROINode());
  d->OutputVolumeComboBox->setCurrentNode(d->ParametersNode->GetOutputVolumeNode());
  d->InterpolationEnabledCheckBox->setChecked(!d->ParametersNode->GetVoxelBased());

  switch (d->ParametersNode->GetInterpolationMode())
    {
    case vtkDMMLCropVolumeParametersNode::InterpolationNearestNeighbor: d->NNRadioButton->setChecked(true); break;
    case vtkDMMLCropVolumeParametersNode::InterpolationLinear: d->LinearRadioButton->setChecked(true); break;
    case vtkDMMLCropVolumeParametersNode::InterpolationWindowedSinc: d->WSRadioButton->setChecked(true); break;
    case vtkDMMLCropVolumeParametersNode::InterpolationBSpline: d->BSRadioButton->setChecked(true); break;
    }
  d->IsotropicCheckbox->setChecked(d->ParametersNode->GetIsotropicResampling());
  d->VisibilityButton->setChecked(d->ParametersNode->GetROINode() && (d->ParametersNode->GetROINode()->GetDisplayVisibility() != 0));

  d->SpacingScalingSpinBox->setValue(d->ParametersNode->GetSpacingScalingConst());

  d->FillValueSpinBox->setValue(d->ParametersNode->GetFillValue());

  this->updateVolumeInfo();
}

//-----------------------------------------------------------
bool qCjyxCropVolumeModuleWidget::setEditedNode(vtkDMMLNode* node,
                                                  QString role /* = QString()*/,
                                                  QString context /* = QString()*/)
{
  Q_D(qCjyxCropVolumeModuleWidget);
  Q_UNUSED(role);
  Q_UNUSED(context);

  if (vtkDMMLCropVolumeParametersNode::SafeDownCast(node))
    {
    d->ParametersNodeComboBox->setCurrentNode(node);
    return true;
    }
  return false;
}

//------------------------------------------------------------------------------
void qCjyxCropVolumeModuleWidget::setParametersNode(vtkDMMLNode* node)
{
  vtkDMMLCropVolumeParametersNode* parametersNode = vtkDMMLCropVolumeParametersNode::SafeDownCast(node);
  Q_D(qCjyxCropVolumeModuleWidget);
  qvtkReconnect(d->ParametersNode, parametersNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromDMML()));
  d->ParametersNode = parametersNode;
  this->updateWidgetFromDMML();
}

//------------------------------------------------------------------------------
void qCjyxCropVolumeModuleWidget::updateVolumeInfo()
{
  Q_D(qCjyxCropVolumeModuleWidget);
  if (d->VolumeInformationCollapsibleButton->collapsed())
    {
    return;
    }

  vtkDMMLVolumeNode* inputVolumeNode = nullptr;
  if (d->ParametersNode != nullptr)
    {
    inputVolumeNode = d->ParametersNode->GetInputVolumeNode();
    }
  if (inputVolumeNode != nullptr && inputVolumeNode->GetImageData() != nullptr)
    {
    int *dimensions = inputVolumeNode->GetImageData()->GetDimensions();
    d->InputDimensionsWidget->setCoordinates(dimensions[0], dimensions[1], dimensions[2]);
    d->InputSpacingWidget->setCoordinates(inputVolumeNode->GetSpacing());
    }
  else
    {
    d->InputDimensionsWidget->setCoordinates(0, 0, 0);
    d->InputSpacingWidget->setCoordinates(0, 0, 0);
    }

  int outputExtent[6] = { 0, -1, -0, -1, 0, -1 };
  double outputSpacing[3] = { 0 };
  if (d->ParametersNode != nullptr && d->ParametersNode->GetInputVolumeNode())
    {
    if (d->ParametersNode->GetVoxelBased())
      {
      d->logic()->GetVoxelBasedCropOutputExtent(d->ParametersNode->GetROINode(), d->ParametersNode->GetInputVolumeNode(), outputExtent);
      d->ParametersNode->GetInputVolumeNode()->GetSpacing(outputSpacing);
      }
    else
      {
      d->logic()->GetInterpolatedCropOutputGeometry(d->ParametersNode->GetROINode(), d->ParametersNode->GetInputVolumeNode(),
        d->ParametersNode->GetIsotropicResampling(), d->ParametersNode->GetSpacingScalingConst(),
        outputExtent, outputSpacing);
      }
    }

  d->CroppedDimensionsWidget->setCoordinates(outputExtent[1] - outputExtent[0] + 1,
    outputExtent[3] - outputExtent[2] + 1, outputExtent[5] - outputExtent[4] + 1);
  d->CroppedSpacingWidget->setCoordinates(outputSpacing);
}

//------------------------------------------------------------------------------
void qCjyxCropVolumeModuleWidget::onVolumeInformationSectionClicked(bool isOpen)
{
  if (isOpen)
  {
    this->updateVolumeInfo();
  }
}
