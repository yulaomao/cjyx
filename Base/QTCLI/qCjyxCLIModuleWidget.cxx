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
#include <QDebug>
#include <QFormLayout>
#include <QMenu>

// Cjyx includes
#include "qCjyxCLIModule.h"
#include "qCjyxCLIModuleWidget_p.h"
#include "vtkCjyxCLIModuleLogic.h"
#include "qCjyxCLIModuleUIHelper.h"

// CLIDMML includes
#include "vtkDMMLCommandLineModuleNode.h"

//-----------------------------------------------------------------------------
// qCjyxCLIModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qCjyxCLIModuleWidgetPrivate::qCjyxCLIModuleWidgetPrivate(qCjyxCLIModuleWidget& object)
  :q_ptr(&object)
{
  this->CLIModuleUIHelper = nullptr;
  this->CommandLineModuleNode = nullptr;
  this->AutoRunWhenParameterChanged = nullptr;
  this->AutoRunWhenInputModified = nullptr;
  this->AutoRunOnOtherInputEvents = nullptr;
  this->AutoRunCancelsRunningProcess = nullptr;
}

//-----------------------------------------------------------------------------
vtkCjyxCLIModuleLogic* qCjyxCLIModuleWidgetPrivate::logic()const
{
  Q_Q(const qCjyxCLIModuleWidget);
  return vtkCjyxCLIModuleLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
vtkDMMLCommandLineModuleNode* qCjyxCLIModuleWidgetPrivate::commandLineModuleNode()const
{
  return vtkDMMLCommandLineModuleNode::SafeDownCast(
    this->DMMLCommandLineModuleNodeSelector->currentNode());
}

//-----------------------------------------------------------------------------
qCjyxCLIModule * qCjyxCLIModuleWidgetPrivate::module()const
{
  Q_Q(const qCjyxCLIModuleWidget);
  qCjyxAbstractCoreModule* coreModule = const_cast<qCjyxAbstractCoreModule*>(q->module());
  return qobject_cast<qCjyxCLIModule*>(coreModule);
}

//-----------------------------------------------------------------------------
void qCjyxCLIModuleWidgetPrivate::setupUi(qCjyxWidget* widget)
{
  Q_Q(qCjyxCLIModuleWidget);

  this->Ui_qCjyxCLIModuleWidget::setupUi(widget);

  QString title = QString::fromStdString(
    this->logic()->GetDefaultModuleDescription().GetTitle());
  this->ModuleCollapsibleButton->setText(title);

  this->DMMLCommandLineModuleNodeSelector->setBaseName(title);
  /// Use the title of the CLI to filter all the command line module node
  /// It is not very robust but there shouldn't be twice the same title.
  this->DMMLCommandLineModuleNodeSelector->addAttribute(
    "vtkDMMLCommandLineModuleNode", "CommandLineModule", title);

  this->DMMLCommandLineModuleNodeSelector->setNodeTypeLabel("Parameter set", "vtkDMMLCommandLineModuleNode");

  this->addParameterGroups();

  // Setup AutoRun menu
  QMenu* autoRunMenu = new QMenu(qCjyxCLIModuleWidget::tr("AutoRun"), this->AutoRunPushButton);

  this->AutoRunWhenParameterChanged =
    new QAction(qCjyxCLIModuleWidget::tr("AutoRun on changed parameter"), autoRunMenu);
  this->AutoRunWhenParameterChanged->setToolTip(
    qCjyxCLIModuleWidget::tr("As long as the AutoRun button is down, the module "
          "is run anytime a parameter value is changed."));
  this->AutoRunWhenParameterChanged->setCheckable(true);
  this->connect(this->AutoRunWhenParameterChanged, SIGNAL(toggled(bool)),
                q, SLOT(setAutoRunWhenParameterChanged(bool)));

  this->AutoRunWhenInputModified =
    new QAction(qCjyxCLIModuleWidget::tr("AutoRun on modified input"), autoRunMenu);
  this->AutoRunWhenInputModified->setToolTip(
    qCjyxCLIModuleWidget::tr("As long as the AutoRun button is down, the module is run anytime an "
          "input node is modified."));
  this->AutoRunWhenInputModified->setCheckable(true);
  this->connect(this->AutoRunWhenInputModified, SIGNAL(toggled(bool)),
                q, SLOT(setAutoRunWhenInputModified(bool)));

  this->AutoRunOnOtherInputEvents =
    new QAction(qCjyxCLIModuleWidget::tr("AutoRun on other input events"), autoRunMenu);
  this->AutoRunOnOtherInputEvents->setToolTip(
    qCjyxCLIModuleWidget::tr("As long as the AutoRun button is down, the module is run anytime an "
          "input node fires an event other than a modified event."));
  this->AutoRunOnOtherInputEvents->setCheckable(true);
  this->connect(this->AutoRunOnOtherInputEvents, SIGNAL(toggled(bool)),
                q, SLOT(setAutoRunOnOtherInputEvents(bool)));

  this->AutoRunCancelsRunningProcess =
    new QAction(qCjyxCLIModuleWidget::tr("AutoRun cancels running process"),autoRunMenu);
  this->AutoRunCancelsRunningProcess->setToolTip(
    qCjyxCLIModuleWidget::tr("When checked, on apply, the module cancels/stops the existing "
          "running instance if any, otherwise it waits the completion to start "
          "a new run."));
  this->AutoRunCancelsRunningProcess->setCheckable(true);
  this->connect(this->AutoRunCancelsRunningProcess, SIGNAL(toggled(bool)),
                q, SLOT(setAutoRunCancelsRunningProcess(bool)));

  autoRunMenu->addAction(this->AutoRunWhenParameterChanged);
  autoRunMenu->addAction(this->AutoRunWhenInputModified);
  autoRunMenu->addAction(this->AutoRunOnOtherInputEvents);
  autoRunMenu->addAction(this->AutoRunCancelsRunningProcess);
  this->AutoRunPushButton->setMenu(autoRunMenu);

  // Connect buttons
  this->connect(this->ApplyPushButton, SIGNAL(clicked()),
                q, SLOT(apply()));

  this->connect(this->CancelPushButton, SIGNAL(clicked()),
                q, SLOT(cancel()));

  this->connect(this->DefaultPushButton, SIGNAL(clicked()),
                q, SLOT(reset()));

  this->connect(this->AutoRunPushButton, SIGNAL(toggled(bool)),
                q, SLOT(setAutoRun(bool)));

  this->connect(this->DMMLCommandLineModuleNodeSelector,
                SIGNAL(currentNodeChanged(vtkDMMLNode*)),
                q, SLOT(setCurrentCommandLineModuleNode(vtkDMMLNode*)));

  this->connect(this->DMMLCommandLineModuleNodeSelector,
                SIGNAL(nodeAddedByUser(vtkDMMLNode*)),
                SLOT(setDefaultNodeValue(vtkDMMLNode*)));

  // Scene must be set in node selector widgets before the DMMLCommandLineModuleNodeSelector widget
  // because when the scene is set in DMMLCommandLineModuleNodeSelector the first available module node
  // is automatically selected and all widgets are updated.
  // Node selector widgets can only be updated if the scene is already set, therefore
  // we set the scene here for all widgets, before DMMLCommandLineModuleNodeSelector has a chance to trigger
  // an update. Scene in DMMLCommandLineModuleNodeSelector will be set later by qCjyxAbstractCoreModule.
  emit q->dmmlSceneChanged(this->module()->dmmlScene());
  this->connect(q, SIGNAL(dmmlSceneChanged(vtkDMMLScene*)),
              this->DMMLCommandLineModuleNodeSelector, SLOT(setDMMLScene(vtkDMMLScene*)));
}

//-----------------------------------------------------------------------------
void qCjyxCLIModuleWidgetPrivate::updateUiFromCommandLineModuleNode(
  vtkObject* commandLineModuleNode)
{
  this->AutoRunPushButton->setEnabled(commandLineModuleNode != nullptr);
  if (!commandLineModuleNode)
    {
    this->ApplyPushButton->setEnabled(false);
    this->CancelPushButton->setEnabled(false);
    this->DefaultPushButton->setEnabled(false);
    this->CLIModuleUIHelper->updateUi(nullptr); // disable widgets
    return;
    }

  vtkDMMLCommandLineModuleNode * node =
    vtkDMMLCommandLineModuleNode::SafeDownCast(commandLineModuleNode);
  Q_ASSERT(node);

  // Update parameters except if the module is running, it would prevent the
  // the user to keep the focus into the widgets each time a progress
  // is reported. (try to select text in the label list line edit of the
  // ModelMaker while running)
  if (!(node->GetStatus() & vtkDMMLCommandLineModuleNode::Running))
    {
    this->CLIModuleUIHelper->updateUi(node);
    }

  this->ApplyPushButton->setEnabled(!node->IsBusy());
  this->DefaultPushButton->setEnabled(!node->IsBusy());
  this->CancelPushButton->setEnabled(node->IsBusy());

  this->AutoRunWhenParameterChanged->setChecked(
    node->GetAutoRunMode() & vtkDMMLCommandLineModuleNode::AutoRunOnChangedParameter);
  this->AutoRunWhenInputModified->setChecked(
    node->GetAutoRunMode() & vtkDMMLCommandLineModuleNode::AutoRunOnModifiedInputEvent);
  this->AutoRunOnOtherInputEvents->setChecked(
    node->GetAutoRunMode() & vtkDMMLCommandLineModuleNode::AutoRunOnOtherInputEvents);
  this->AutoRunCancelsRunningProcess->setChecked(
    node->GetAutoRunMode() & vtkDMMLCommandLineModuleNode::AutoRunCancelsRunningProcess);
  if (this->AutoRunPushButton->isChecked() != node->GetAutoRun())
    {
    this->AutoRunPushButton->setChecked(node->GetAutoRun());
    }
}

//-----------------------------------------------------------------------------
void qCjyxCLIModuleWidgetPrivate::updateCommandLineModuleNodeFromUi(
  vtkObject* commandLineModuleNode)
{
  if (!commandLineModuleNode)
    {
    return;
    }
  vtkDMMLCommandLineModuleNode * node =
    vtkDMMLCommandLineModuleNode::SafeDownCast(commandLineModuleNode);
  Q_ASSERT(node);
  this->CLIModuleUIHelper->updateDMMLCommandLineModuleNode(node);
}

//-----------------------------------------------------------------------------
void qCjyxCLIModuleWidgetPrivate::setDefaultNodeValue(vtkDMMLNode* commandLineModuleNode)
{
  vtkDMMLCommandLineModuleNode * node =
    vtkDMMLCommandLineModuleNode::SafeDownCast(commandLineModuleNode);
  Q_ASSERT(node);
  // Note that node will fire a ModifyEvent.
  node->SetModuleDescription(this->logic()->GetDefaultModuleDescription());
}

//-----------------------------------------------------------------------------
void qCjyxCLIModuleWidgetPrivate::addParameterGroups()
{
  // iterate over each parameter group
  const ModuleDescription& moduleDescription =
    this->logic()->GetDefaultModuleDescription();
  for (ParameterGroupConstIterator pgIt = moduleDescription.GetParameterGroups().begin();
       pgIt != moduleDescription.GetParameterGroups().end(); ++pgIt)
    {
    this->addParameterGroup(this->VerticalLayout, *pgIt);
    }
}

//-----------------------------------------------------------------------------
void qCjyxCLIModuleWidgetPrivate::addParameterGroup(QBoxLayout* _layout,
                                                     const ModuleParameterGroup& parameterGroup)
{
  Q_ASSERT(_layout);

  ctkCollapsibleButton * collapsibleWidget = new ctkCollapsibleButton();
  collapsibleWidget->setText(QString::fromStdString(parameterGroup.GetLabel()));
  collapsibleWidget->setCollapsed(parameterGroup.GetAdvanced() == "true");

  // Create a vertical layout and add parameter to it
  QFormLayout *vbox = new QFormLayout;
  this->addParameters(vbox, parameterGroup);
  //vbox->addStretch(1);
  vbox->setVerticalSpacing(1);
  collapsibleWidget->setLayout(vbox);

  _layout->addWidget(collapsibleWidget);
}

//-----------------------------------------------------------------------------
void qCjyxCLIModuleWidgetPrivate::addParameters(QFormLayout* _layout,
                                                const ModuleParameterGroup& parameterGroup)
{
  Q_ASSERT(_layout);
  // iterate over each parameter in this group
  ParameterConstIterator pBeginIt = parameterGroup.GetParameters().begin();
  ParameterConstIterator pEndIt = parameterGroup.GetParameters().end();

  for (ParameterConstIterator pIt = pBeginIt; pIt != pEndIt; ++pIt)
    {
    this->addParameter(_layout, *pIt);
    }
}

//-----------------------------------------------------------------------------
void qCjyxCLIModuleWidgetPrivate::addParameter(QFormLayout* _layout,
                                               const ModuleParameter& moduleParameter)
{
  Q_ASSERT(_layout);

  if (moduleParameter.GetHidden() == "true")
    {
    return;
    }

  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString description = QString::fromStdString(moduleParameter.GetDescription());

  // TODO Parameters with flags can support the None node because they are optional
  //int noneEnabled = 0;
  //if (moduleParameter.GetLongFlag() != "" || moduleParameter.GetFlag() != "")
  //  {
  //  noneEnabled = 1;
  //  }

  QLabel* widgetLabel = new QLabel(_label);
  widgetLabel->setToolTip(description);

  QWidget * widget = this->CLIModuleUIHelper->createTagWidget(moduleParameter);

  _layout->addRow(widgetLabel, widget);
}

//-----------------------------------------------------------------------------
void qCjyxCLIModuleWidgetPrivate::onValueChanged(const QString& name, const QVariant& value)
{
  Q_Q(qCjyxCLIModuleWidget);
  // but if the scene is closing, then nevermind, values are changing
  // because nodes are getting removed
  if (!q->dmmlScene() ||
      q->dmmlScene()->IsClosing())
    {
    return;
    }
  // Make sure a command line module node is created
  if (this->CommandLineModuleNode == nullptr)
    {
    // if not, then create a default node
    this->DMMLCommandLineModuleNodeSelector->addNode();
    Q_ASSERT(this->CommandLineModuleNode);
    }
  this->CLIModuleUIHelper->setCommandLineModuleParameter(
    this->CommandLineModuleNode, name, value);
}

//-----------------------------------------------------------------------------
// qCjyxCLIModuleWidget methods

//-----------------------------------------------------------------------------
qCjyxCLIModuleWidget::qCjyxCLIModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qCjyxCLIModuleWidgetPrivate(*this))
{
  Q_D(qCjyxCLIModuleWidget);

  d->CLIModuleUIHelper = new qCjyxCLIModuleUIHelper(this);
  this->connect(d->CLIModuleUIHelper,
                SIGNAL(valueChanged(QString,QVariant)),
                d,
                SLOT(onValueChanged(QString,QVariant)));
}

//-----------------------------------------------------------------------------
qCjyxCLIModuleWidget::~qCjyxCLIModuleWidget() = default;

//-----------------------------------------------------------------------------
void qCjyxCLIModuleWidget::setup()
{
  Q_D(qCjyxCLIModuleWidget);

  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qCjyxCLIModuleWidget::enter()
{
  Q_D(qCjyxCLIModuleWidget);

  this->Superclass::enter();

  // Make sure a command line module node is available when the module widget
  // is activated. If no CLI node is available then create a new one.
  if (d->DMMLCommandLineModuleNodeSelector->currentNode() == nullptr)
    {
    bool wasBlocked = d->DMMLCommandLineModuleNodeSelector->blockSignals(true);
    vtkDMMLCommandLineModuleNode* node = vtkDMMLCommandLineModuleNode::SafeDownCast(d->DMMLCommandLineModuleNodeSelector->addNode());
    Q_ASSERT(node);
    // Initialize module description (just to avoid warnings
    // when the node is set as current node and GUI is attempted to be updated from the node)
    d->setDefaultNodeValue(node);
    d->DMMLCommandLineModuleNodeSelector->blockSignals(wasBlocked);
    this->setCurrentCommandLineModuleNode(node);
    Q_ASSERT(d->CommandLineModuleNode);
    }
}

//-----------------------------------------------------------------------------
vtkDMMLCommandLineModuleNode * qCjyxCLIModuleWidget::currentCommandLineModuleNode()const
{
  Q_D(const qCjyxCLIModuleWidget);
  return d->CommandLineModuleNode;
}

//-----------------------------------------------------------------------------
void qCjyxCLIModuleWidget::setCurrentCommandLineModuleNode(
  vtkDMMLNode* commandLineModuleNode)
{
  Q_D(qCjyxCLIModuleWidget);
  vtkDMMLCommandLineModuleNode * node =
    vtkDMMLCommandLineModuleNode::SafeDownCast(commandLineModuleNode);
  if (node == d->CommandLineModuleNode)
    {
    return;
    }

  // Update the selector if this slot was called programmatically
  Q_ASSERT(d->DMMLCommandLineModuleNodeSelector);
  if (d->DMMLCommandLineModuleNodeSelector->currentNode()
      != commandLineModuleNode)
    {
    d->DMMLCommandLineModuleNodeSelector->setCurrentNode(commandLineModuleNode);
    return;
    }

  // Connect node modified event to updateUi that synchronize the values of the
  // nodes with the Ui
  this->qvtkReconnect(d->CommandLineModuleNode, node,
    vtkCommand::ModifiedEvent,
    d, SLOT(updateUiFromCommandLineModuleNode(vtkObject*)));

  // After we disconnected the Modified event from the old CommandLineModuleNode
  // we can save the parameters of the command line module node so they could be
  // retrieved later on when it becomes current again
  //d->updateCommandLineModuleNodeFromUi(d->CommandLineModuleNode);

  d->CommandLineModuleNode = node;
  d->CLIProgressBar->setCommandLineModuleNode(d->CommandLineModuleNode);
  d->updateUiFromCommandLineModuleNode(d->CommandLineModuleNode);
}

//-----------------------------------------------------------------------------
void qCjyxCLIModuleWidget::apply(bool wait)
{
  Q_D(qCjyxCLIModuleWidget);
  vtkDMMLCommandLineModuleNode* node = d->commandLineModuleNode();
  Q_ASSERT(node);
  d->CLIModuleUIHelper->updateDMMLCommandLineModuleNode(node);
  this->run(node, /* waitForCompletion= */ wait);
}

//-----------------------------------------------------------------------------
void qCjyxCLIModuleWidget::cancel()
{
  Q_D(qCjyxCLIModuleWidget);
  vtkDMMLCommandLineModuleNode* node = d->commandLineModuleNode();
  Q_ASSERT(node);
  this->cancel(node);
}

//-----------------------------------------------------------------------------
void qCjyxCLIModuleWidget::reset()
{
  Q_D(qCjyxCLIModuleWidget);
  vtkDMMLCommandLineModuleNode* node = d->commandLineModuleNode();
  Q_ASSERT(node);
  d->setDefaultNodeValue(node);
}

//-----------------------------------------------------------------------------
void qCjyxCLIModuleWidget::run(vtkDMMLCommandLineModuleNode* parameterNode, bool waitForCompletion)
{
  Q_D(qCjyxCLIModuleWidget);
  Q_ASSERT(d->logic());

  if (waitForCompletion)
    {
    d->logic()->ApplyAndWait(parameterNode);
    }
  else
    {
    d->logic()->Apply(parameterNode);
    }
}

//-----------------------------------------------------------------------------
void qCjyxCLIModuleWidget::cancel(vtkDMMLCommandLineModuleNode* node)
{
  if (!node)
    {
    return;
    }
  node->Cancel();
}

//-----------------------------------------------------------------------------
void qCjyxCLIModuleWidget::setAutoRun(bool enable)
{
  Q_D(qCjyxCLIModuleWidget);
  d->commandLineModuleNode()->SetAutoRun(enable);
}

//-----------------------------------------------------------------------------
void qCjyxCLIModuleWidget::setAutoRunWhenParameterChanged(bool autoRun)
{
  Q_D(qCjyxCLIModuleWidget);
  int newAutoRunMode = d->commandLineModuleNode()->GetAutoRunMode();
  if (autoRun)
    {
    newAutoRunMode |= vtkDMMLCommandLineModuleNode::AutoRunOnChangedParameter;
    }
  else
    {
    newAutoRunMode &= ~vtkDMMLCommandLineModuleNode::AutoRunOnChangedParameter;
    }
  d->commandLineModuleNode()->SetAutoRunMode(newAutoRunMode);
}

//-----------------------------------------------------------------------------
void qCjyxCLIModuleWidget::setAutoRunWhenInputModified(bool autoRun)
{
  Q_D(qCjyxCLIModuleWidget);
  int newAutoRunMode = d->commandLineModuleNode()->GetAutoRunMode();
  if (autoRun)
    {
    newAutoRunMode |= vtkDMMLCommandLineModuleNode::AutoRunOnModifiedInputEvent;
    }
  else
    {
    newAutoRunMode &= ~vtkDMMLCommandLineModuleNode::AutoRunOnModifiedInputEvent;
    }
  d->commandLineModuleNode()->SetAutoRunMode(newAutoRunMode);
}

//-----------------------------------------------------------------------------
void qCjyxCLIModuleWidget::setAutoRunOnOtherInputEvents(bool autoRun)
{
  Q_D(qCjyxCLIModuleWidget);
  int newAutoRunMode = d->commandLineModuleNode()->GetAutoRunMode();
  if (autoRun)
    {
    newAutoRunMode |= vtkDMMLCommandLineModuleNode::AutoRunOnOtherInputEvents;
    }
  else
    {
    newAutoRunMode &= ~vtkDMMLCommandLineModuleNode::AutoRunOnOtherInputEvents;
    }
  d->commandLineModuleNode()->SetAutoRunMode(newAutoRunMode);
}

//-----------------------------------------------------------------------------
void qCjyxCLIModuleWidget::setAutoRunCancelsRunningProcess(bool autoRun)
{
  Q_D(qCjyxCLIModuleWidget);
  int newAutoRunMode = d->commandLineModuleNode()->GetAutoRunMode();
  if (autoRun)
    {
    newAutoRunMode |= vtkDMMLCommandLineModuleNode::AutoRunCancelsRunningProcess;
    }
  else
    {
    newAutoRunMode &= ~vtkDMMLCommandLineModuleNode::AutoRunCancelsRunningProcess;
    }
  d->commandLineModuleNode()->SetAutoRunMode(newAutoRunMode);
}

//-----------------------------------------------------------
bool qCjyxCLIModuleWidget::setEditedNode(vtkDMMLNode* node,
                                           QString role /* = QString()*/,
                                           QString context /* = QString()*/)
{
  Q_D(qCjyxCLIModuleWidget);
  Q_UNUSED(role);
  Q_UNUSED(context);
  vtkDMMLCommandLineModuleNode* cmdLineModuleNode = vtkDMMLCommandLineModuleNode::SafeDownCast(node);
  if (!cmdLineModuleNode)
    {
    qWarning() << Q_FUNC_INFO << " failed: invalid input node";
    return false;
    }
  const char* moduleTitle = cmdLineModuleNode->GetAttribute("CommandLineModule");
  if (!moduleTitle)
    {
    qWarning() << Q_FUNC_INFO << " failed: CommandLineModule attribute of node is not set";
    return false;
    }
  if (moduleTitle != this->module()->title())
    {
    qWarning() << Q_FUNC_INFO << " failed: mismatch of module title in CommandLineModule attribute of node";
    return false;
    }
  d->DMMLCommandLineModuleNodeSelector->setCurrentNode(node);
  return true;
}

//-----------------------------------------------------------
double qCjyxCLIModuleWidget::nodeEditable(vtkDMMLNode* node)
{
  if (vtkDMMLCommandLineModuleNode::SafeDownCast(node))
    {
    vtkDMMLCommandLineModuleNode* cmdLineModuleNode = vtkDMMLCommandLineModuleNode::SafeDownCast(node);
    const char* moduleTitle = cmdLineModuleNode->GetAttribute("CommandLineModule");
    if (!moduleTitle)
      {
      // node is not associated to any module
      return 0.0;
      }
    if (moduleTitle != this->module()->title())
      {
      return 0.0;
      }
    // Module title matches, probably this module owns this node
    return 0.5;
    }
  else
    {
    return 0.0;
    }
}
