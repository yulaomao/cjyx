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

#ifndef __qCjyxSequencesModuleWidget_h
#define __qCjyxSequencesModuleWidget_h

// Cjyx includes
#include "qCjyxAbstractModuleWidget.h"

#include "qCjyxSequencesModuleExport.h"

#include <QtGui>

class qCjyxSequencesModuleWidgetPrivate;
class vtkDMMLNode;
class vtkDMMLSequenceNode;
class vtkDMMLSequenceBrowserNode;

/// \ingroup Cjyx_QtModules_ExtensionTemplate
class Q_CJYX_QTMODULES_SEQUENCES_EXPORT qCjyxSequencesModuleWidget :
  public qCjyxAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qCjyxAbstractModuleWidget Superclass;
  qCjyxSequencesModuleWidget(QWidget *parent=0);
  ~qCjyxSequencesModuleWidget() override;

  /// Set up the GUI from dmml when entering
  void enter() override;
  /// Disconnect from scene when exiting
  void exit() override;

  Q_INVOKABLE void setActiveBrowserNode(vtkDMMLSequenceBrowserNode* browserNode);
  Q_INVOKABLE void setMasterSequenceNode(vtkDMMLSequenceNode* sequenceNode);

  Q_INVOKABLE bool setEditedNode(vtkDMMLNode* node, QString role = QString(), QString context = QString()) override;

public slots:

  void setActiveSequenceNode(vtkDMMLSequenceNode* newActiveSequenceNode);
  void onSequenceNodeSelectionChanged();
  void onSequenceNodeModified();

  void onIndexNameEdited();
  void onIndexUnitEdited();
  void onIndexTypeEdited(QString indexTypeString);

  void onDataNodeEdited( int row, int column );

  void onAddDataNodeButtonClicked();
  void onRemoveDataNodeButtonClicked();

  /// Respond to the scene events
  void onNodeAddedEvent(vtkObject* scene, vtkObject* node);
  void onNodeRemovedEvent(vtkObject* scene, vtkObject* node);
  void onDMMLSceneEndImportEvent();
  void onDMMLSceneEndRestoreEvent();
  void onDMMLSceneEndBatchProcessEvent();
  void onDMMLSceneEndCloseEvent();

protected:
  void updateWidgetFromDMML();

  /// Refresh synchronized sequence nodes table from DMML
  void refreshSynchronizedSequenceNodesTable();

  QScopedPointer<qCjyxSequencesModuleWidgetPrivate> d_ptr;

  void setup() override;

  void setEnableWidgets(bool enable);

public slots:
  void setDMMLScene(vtkDMMLScene* scene) override;

protected slots:
  void activeBrowserNodeChanged(vtkDMMLNode* node);
  void sequenceNodeChanged(vtkDMMLNode*);
  void playbackItemSkippingEnabledChanged(bool enabled);
  void recordMasterOnlyChanged(bool enabled);
  void recordingSamplingModeChanged(int index);
  void indexDisplayModeChanged(int index);
  void indexDisplayFormatChanged(const QString& format);
  void onDMMLInputSequenceInputNodeModified(vtkObject* caller);
  void onActiveBrowserNodeModified(vtkObject* caller);
  void updateChart();

  void sequenceNodeNameEdited(int row, int column);

  void onAddSequenceNodeButtonClicked();
  void onRemoveSequenceNodesButtonClicked();

  void synchronizedSequenceNodePlaybackStateChanged(int aState);
  void synchronizedSequenceNodeRecordingStateChanged(int aState);
  void synchronizedSequenceNodeOverwriteProxyNameStateChanged(int aState);
  void synchronizedSequenceNodeSaveChangesStateChanged(int aState);

  void onProxyNodeChanged(vtkDMMLNode* newProxyNode);

  void updateSequenceItemWidgetFromDMML();
  void updateCandidateNodesWidgetFromDMML(bool forceUpdate = false);

private:
  Q_DECLARE_PRIVATE(qCjyxSequencesModuleWidget);
  Q_DISABLE_COPY(qCjyxSequencesModuleWidget);
};

#endif
