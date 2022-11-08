/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Brigham and Women's Hospital

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// qDMML includes
#include "qDMMLSequenceBrowserPlayWidget.h"
#include "ui_qDMMLSequenceBrowserPlayWidget.h"

// DMML includes
#include <vtkDMMLSequenceBrowserNode.h>
#include <vtkDMMLSequenceNode.h>

// Qt includes
#include <QDebug>
#include <QShortcut>

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_Markups
class qDMMLSequenceBrowserPlayWidgetPrivate
: public Ui_qDMMLSequenceBrowserPlayWidget
{
  Q_DECLARE_PUBLIC(qDMMLSequenceBrowserPlayWidget);
protected:
  qDMMLSequenceBrowserPlayWidget* const q_ptr;
public:
  qDMMLSequenceBrowserPlayWidgetPrivate(qDMMLSequenceBrowserPlayWidget& object);
  bool RecordingControlsVisible{ true };
  void init();

  vtkWeakPointer<vtkDMMLSequenceBrowserNode> SequenceBrowserNode;
};

//-----------------------------------------------------------------------------
// qDMMLSequenceBrowserPlayWidgetPrivate methods

//-----------------------------------------------------------------------------
qDMMLSequenceBrowserPlayWidgetPrivate::qDMMLSequenceBrowserPlayWidgetPrivate(qDMMLSequenceBrowserPlayWidget& object)
: q_ptr(&object)
{
  this->SequenceBrowserNode = nullptr;
}

//-----------------------------------------------------------------------------
void qDMMLSequenceBrowserPlayWidgetPrivate::init()
{
  Q_Q(qDMMLSequenceBrowserPlayWidget);
  this->setupUi(q);

  QObject::connect( this->pushButton_VcrFirst, SIGNAL(clicked()), q, SLOT(onVcrFirst()) );
  QObject::connect( this->pushButton_VcrPrevious, SIGNAL(clicked()), q, SLOT(onVcrPrevious()) );
  QObject::connect( this->pushButton_VcrNext, SIGNAL(clicked()), q, SLOT(onVcrNext()) );
  QObject::connect( this->pushButton_VcrLast, SIGNAL(clicked()), q, SLOT(onVcrLast()) );
  QObject::connect( this->pushButton_VcrPlayPause, SIGNAL(toggled(bool)), q, SLOT(setPlaybackEnabled(bool)) );
  QObject::connect(this->pushButton_VcrLoop, SIGNAL(toggled(bool)), q, SLOT(setPlaybackLoopEnabled(bool)));
  QObject::connect( this->doubleSpinBox_VcrPlaybackRate, SIGNAL(valueChanged(double)), q, SLOT(setPlaybackRateFps(double)) );
  QObject::connect(this->pushButton_VcrRecord, SIGNAL(toggled(bool)), q, SLOT(setRecordingEnabled(bool)));
  QObject::connect(this->pushButton_Snapshot, SIGNAL(clicked()), q, SLOT(onRecordSnapshot()));

  q->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
// qDMMLSequenceBrowserPlayWidget methods

//-----------------------------------------------------------------------------
qDMMLSequenceBrowserPlayWidget::qDMMLSequenceBrowserPlayWidget(QWidget *newParent)
: Superclass(newParent)
, d_ptr(new qDMMLSequenceBrowserPlayWidgetPrivate(*this))
{
  Q_D(qDMMLSequenceBrowserPlayWidget);
  d->init();
}

//-----------------------------------------------------------------------------
qDMMLSequenceBrowserPlayWidget::~qDMMLSequenceBrowserPlayWidget() = default;

//-----------------------------------------------------------------------------
void qDMMLSequenceBrowserPlayWidget::setDMMLSequenceBrowserNode(vtkDMMLNode* browserNode)
{
  setDMMLSequenceBrowserNode(vtkDMMLSequenceBrowserNode::SafeDownCast(browserNode));
}

//-----------------------------------------------------------------------------
void qDMMLSequenceBrowserPlayWidget::setDMMLSequenceBrowserNode(vtkDMMLSequenceBrowserNode* browserNode)
{
  Q_D(qDMMLSequenceBrowserPlayWidget);

  qvtkReconnect(d->SequenceBrowserNode, browserNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromDMML()));

  d->SequenceBrowserNode = browserNode;
  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qDMMLSequenceBrowserPlayWidget::updateWidgetFromDMML()
{
  Q_D(qDMMLSequenceBrowserPlayWidget);

  vtkDMMLSequenceNode* sequenceNode = d->SequenceBrowserNode.GetPointer() ? d->SequenceBrowserNode->GetMasterSequenceNode() : nullptr;
  this->setEnabled(sequenceNode != nullptr);
  if (!sequenceNode)
    {
    return;
    }

  QObjectList vcrPlaybackControls; // Note we don't include the recording, because we want to be able to record when no data is available
  vcrPlaybackControls
    << d->pushButton_VcrFirst << d->pushButton_VcrLast << d->pushButton_VcrLoop
    << d->pushButton_VcrNext << d->pushButton_VcrPlayPause << d->pushButton_VcrPrevious;
  bool vcrControlsEnabled=false;

  int numberOfDataNodes=sequenceNode->GetNumberOfDataNodes();
  if (numberOfDataNodes>0 && !d->SequenceBrowserNode->GetRecordingActive())
  {
    vcrControlsEnabled=true;

    bool pushButton_VcrPlayPauseBlockSignals = d->pushButton_VcrPlayPause->blockSignals(true);
    d->pushButton_VcrPlayPause->setChecked(d->SequenceBrowserNode->GetPlaybackActive());
    d->pushButton_VcrPlayPause->blockSignals(pushButton_VcrPlayPauseBlockSignals);

    bool pushButton_VcrLoopBlockSignals = d->pushButton_VcrLoop->blockSignals(true);
    d->pushButton_VcrLoop->setChecked(d->SequenceBrowserNode->GetPlaybackLooped());
    d->pushButton_VcrLoop->blockSignals(pushButton_VcrLoopBlockSignals);
  }

  bool signalsBlocked = d->doubleSpinBox_VcrPlaybackRate->blockSignals(true);
  d->doubleSpinBox_VcrPlaybackRate->setValue(d->SequenceBrowserNode->GetPlaybackRateFps());
  d->doubleSpinBox_VcrPlaybackRate->blockSignals(signalsBlocked);

  bool pushButton_VcrRecordingBlockSignals = d->pushButton_VcrRecord->blockSignals(true);
  d->pushButton_VcrRecord->setChecked(d->SequenceBrowserNode->GetRecordingActive());
  d->pushButton_VcrRecord->blockSignals(pushButton_VcrRecordingBlockSignals);

  bool recordingAllowed = d->SequenceBrowserNode->IsAnySequenceNodeRecording();
  bool playbackActive = d->SequenceBrowserNode->GetPlaybackActive();
  bool recordingActive = d->SequenceBrowserNode->GetRecordingActive();

  d->pushButton_VcrRecord->setVisible(recordingAllowed && d->RecordingControlsVisible);
  d->pushButton_VcrRecord->setEnabled(!playbackActive);
  d->pushButton_Snapshot->setVisible(recordingAllowed && d->RecordingControlsVisible);
  d->pushButton_Snapshot->setEnabled(!playbackActive && !recordingActive);

  foreach( QObject*w, vcrPlaybackControls ) { w->setProperty( "enabled", vcrControlsEnabled ); }
}

//-----------------------------------------------------------------------------
void qDMMLSequenceBrowserPlayWidget::onVcrFirst()
{
  Q_D(qDMMLSequenceBrowserPlayWidget);
  if (d->SequenceBrowserNode==nullptr)
    {
    qDebug() << "onVcrFirst failed: no active browser node is selected";
    updateWidgetFromDMML();
    return;
    }
  d->SequenceBrowserNode->SelectFirstItem();
}

//-----------------------------------------------------------------------------
void qDMMLSequenceBrowserPlayWidget::onVcrLast()
{
  Q_D(qDMMLSequenceBrowserPlayWidget);
  if (d->SequenceBrowserNode==nullptr)
    {
    qDebug() << "onVcrLast failed: no active browser node is selected";
    updateWidgetFromDMML();
    return;
    }
  d->SequenceBrowserNode->SelectLastItem();
}

//-----------------------------------------------------------------------------
void qDMMLSequenceBrowserPlayWidget::onVcrPrevious()
{
  Q_D(qDMMLSequenceBrowserPlayWidget);
  if (d->SequenceBrowserNode==nullptr)
    {
    qDebug() << "onVcrPrevious failed: no active browser node is selected";
    updateWidgetFromDMML();
    return;
    }
  d->SequenceBrowserNode->SelectNextItem(-1);
}

//-----------------------------------------------------------------------------
void qDMMLSequenceBrowserPlayWidget::onVcrNext()
{
  Q_D(qDMMLSequenceBrowserPlayWidget);
  if (d->SequenceBrowserNode==nullptr)
    {
    qDebug() << "onVcrNext failed: no active browser node is selected";
    updateWidgetFromDMML();
    return;
    }
  d->SequenceBrowserNode->SelectNextItem(1);
}

//-----------------------------------------------------------------------------
void qDMMLSequenceBrowserPlayWidget::onVcrPlayPause()
{
  Q_D(qDMMLSequenceBrowserPlayWidget);
  if (d->SequenceBrowserNode == nullptr)
    {
    qDebug() << "onVcrPlayPause failed: no active browser node is selected";
    updateWidgetFromDMML();
    return;
    }
  d->SequenceBrowserNode->SetRecordingActive(false);
  d->SequenceBrowserNode->SetPlaybackActive(!d->SequenceBrowserNode->GetPlaybackActive());
}

//-----------------------------------------------------------------------------
void qDMMLSequenceBrowserPlayWidget::onRecordSnapshot()
{
  Q_D(qDMMLSequenceBrowserPlayWidget);
  if (d->SequenceBrowserNode == nullptr)
    {
    qDebug() << "onRecordSnapshot failed: no active browser node is selected";
    updateWidgetFromDMML();
    return;
    }
  d->SequenceBrowserNode->SaveProxyNodesState();
}


//-----------------------------------------------------------------------------
void qDMMLSequenceBrowserPlayWidget::setPlaybackEnabled(bool play)
{
  Q_D(qDMMLSequenceBrowserPlayWidget);
  if (d->SequenceBrowserNode==nullptr)
    {
    qDebug() << "onVcrPlayPauseStateChanged failed: no active browser node is selected";
    updateWidgetFromDMML();
    return;
    }
  d->SequenceBrowserNode->SetRecordingActive(false);
  if (play!=d->SequenceBrowserNode->GetPlaybackActive())
    {
    d->SequenceBrowserNode->SetPlaybackActive(play);
    }
}

//-----------------------------------------------------------------------------
void qDMMLSequenceBrowserPlayWidget::setRecordingEnabled(bool record)
{
  Q_D(qDMMLSequenceBrowserPlayWidget);
  if (d->SequenceBrowserNode==nullptr)
    {
    qDebug() << "onVcrRecordStateChanged failed: no active browser node is selected";
    updateWidgetFromDMML();
    return;
    }
  d->SequenceBrowserNode->SetPlaybackActive(false);
  if (record!=d->SequenceBrowserNode->GetRecordingActive())
    {
    d->SequenceBrowserNode->SetRecordingActive(record);
    }
}

//-----------------------------------------------------------------------------
void qDMMLSequenceBrowserPlayWidget::setPlaybackLoopEnabled(bool loopEnabled)
{
  Q_D(qDMMLSequenceBrowserPlayWidget);
  if (d->SequenceBrowserNode==nullptr)
    {
    qDebug() << "onVcrPlaybackLoopStateChanged failed: no active browser node is selected";
    this->updateWidgetFromDMML();
    return;
    }
  if (loopEnabled!=d->SequenceBrowserNode->GetPlaybackLooped())
    {
    d->SequenceBrowserNode->SetPlaybackLooped(loopEnabled);
    }
}

//-----------------------------------------------------------------------------
void qDMMLSequenceBrowserPlayWidget::setPlaybackRateFps(double playbackRateFps)
{
  Q_D(qDMMLSequenceBrowserPlayWidget);
  if (d->SequenceBrowserNode==nullptr)
    {
    qDebug() << "setPlaybackRateFps failed: no active browser node is selected";
    this->updateWidgetFromDMML();
    return;
    }
  if (playbackRateFps!=d->SequenceBrowserNode->GetPlaybackRateFps())
    {
    d->SequenceBrowserNode->SetPlaybackRateFps(playbackRateFps);
    }
}

//-----------------------------------------------------------------------------
void qDMMLSequenceBrowserPlayWidget::setPlayPauseShortcut(QString keySequence)
{
  Q_D(qDMMLSequenceBrowserPlayWidget);
  QObject::connect(new QShortcut(QKeySequence(keySequence), this), SIGNAL(activated()), SLOT(onVcrPlayPause()));
  d->pushButton_VcrPlayPause->setToolTip(d->pushButton_VcrPlayPause->toolTip()+" ("+keySequence+")");
}

//-----------------------------------------------------------------------------
void qDMMLSequenceBrowserPlayWidget::setPreviousFrameShortcut(QString keySequence)
{
  Q_D(qDMMLSequenceBrowserPlayWidget);
  QObject::connect(new QShortcut(QKeySequence(keySequence), this), SIGNAL(activated()), SLOT(onVcrPrevious()));
  d->pushButton_VcrPrevious->setToolTip(d->pushButton_VcrPrevious->toolTip() + " (" + keySequence + ")");
}

//-----------------------------------------------------------------------------
void qDMMLSequenceBrowserPlayWidget::setNextFrameShortcut(QString keySequence)
{
  Q_D(qDMMLSequenceBrowserPlayWidget);
  QObject::connect(new QShortcut(QKeySequence(keySequence), this), SIGNAL(activated()), SLOT(onVcrNext()));
  d->pushButton_VcrNext->setToolTip(d->pushButton_VcrNext->toolTip() + " (" + keySequence + ")");
}

//---------------------------------------------------------------------------
void qDMMLSequenceBrowserPlayWidget::setRecordingControlsVisible(bool show)
{
  Q_D(qDMMLSequenceBrowserPlayWidget);
  d->RecordingControlsVisible = show;
  this->updateWidgetFromDMML();
}

//---------------------------------------------------------------------------
bool qDMMLSequenceBrowserPlayWidget::recordingControlsVisible() const
{
  Q_D(const qDMMLSequenceBrowserPlayWidget);
  return d->RecordingControlsVisible;
}

