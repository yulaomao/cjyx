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

  This file was originally developed by Laurent Chauvin, Brigham and Women's
  Hospital. The project was supported by grants 5P01CA067165,
  5R01CA124377, 5R01CA138586, 2R44DE019322, 7R01CA124377,
  5R42CA137886, 5R42CA137886
  It was then updated for the Markups module by Nicole Aucoin, BWH.

==============================================================================*/

#ifndef __qDMMLSequenceBrowserPlayWidget_h
#define __qDMMLSequenceBrowserPlayWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// Cjyx includes
#include "qDMMLWidget.h"

#include "qCjyxSequencesModuleWidgetsExport.h"

class qDMMLSequenceBrowserPlayWidgetPrivate;
class vtkDMMLNode;
class vtkDMMLSequenceBrowserNode;

/// \ingroup Cjyx_QtModules_Markups
class Q_CJYX_MODULE_SEQUENCES_WIDGETS_EXPORT qDMMLSequenceBrowserPlayWidget
: public qDMMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

  /// Enable displaying recording control buttons (record and snapshot).
  /// The buttons are only visible if this flag is enabled and there is at least one sequence that
  /// has recording enabled.
  Q_PROPERTY(bool RecordingControlsVisible READ recordingControlsVisible WRITE setRecordingControlsVisible)


public:
  typedef qDMMLWidget Superclass;
  qDMMLSequenceBrowserPlayWidget(QWidget *newParent = 0);
  ~qDMMLSequenceBrowserPlayWidget() override;

  /// Add a keyboard shortcut for play/pause button
  void setPlayPauseShortcut(QString keySequence);

  /// Add a keyboard shortcut for previous frame button
  void setPreviousFrameShortcut(QString keySequence);

  /// Add a keyboard shortcut for next frame button
  void setNextFrameShortcut(QString keySequence);

  /// Returns true if recording controls (record and snapshot buttons) are allowed to be shown.
  ///
  /// \note Regardless of this flag, recording controls are not shown if recording is not enabled
  /// for any of the browsed sequences.
  bool recordingControlsVisible() const;

public slots:
  void setDMMLSequenceBrowserNode(vtkDMMLSequenceBrowserNode* browserNode);
  void setDMMLSequenceBrowserNode(vtkDMMLNode* browserNode);
  void setPlaybackEnabled(bool play);
  void setRecordingEnabled(bool play);
  void setPlaybackRateFps(double playbackRateFps);
  void setPlaybackLoopEnabled(bool loopEnabled);
  void setRecordingControlsVisible(bool show);
  void onVcrFirst();
  void onVcrPrevious();
  void onVcrNext();
  void onVcrLast();
  void onVcrPlayPause();
  void onRecordSnapshot();

protected slots:
  void updateWidgetFromDMML();

protected:
  QScopedPointer<qDMMLSequenceBrowserPlayWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLSequenceBrowserPlayWidget);
  Q_DISABLE_COPY(qDMMLSequenceBrowserPlayWidget);

};

#endif
