/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __qCjyxSegmentEditorScriptedEffect_h
#define __qCjyxSegmentEditorScriptedEffect_h

// Segmentations Editor Effects includes
#include "qCjyxSegmentEditorAbstractEffect.h"

#include "qCjyxSegmentationsEditorEffectsExport.h"

// Forward Declare PyObject*
#ifndef PyObject_HEAD
struct _object;
typedef _object PyObject;
#endif
class qCjyxSegmentEditorScriptedEffectPrivate;

/// \ingroup CjyxRt_QtModules_Segmentations
/// \brief Scripted abstract effect for implementing segment editor effects in python
///
/// This class provides an interface to effects implemented in python.
/// USAGE: Subclass AbstractScriptedSegmentEditorEffect in EditorEffects/Python subfolder,
///   and register effect by creating this class and setting python source to implemented
///   effect subclass. One example is the ThresholdEffect.
///
class Q_CJYX_SEGMENTATIONS_EFFECTS_EXPORT qCjyxSegmentEditorScriptedEffect
  : public qCjyxSegmentEditorAbstractEffect
{
  Q_OBJECT

public:
  typedef qCjyxSegmentEditorAbstractEffect Superclass;
  qCjyxSegmentEditorScriptedEffect(QObject* parent = nullptr);
  ~qCjyxSegmentEditorScriptedEffect() override;

  Q_INVOKABLE QString pythonSource()const;

  /// Set python source for the implemented effect
  /// \param newPythonSource Python file path
  Q_INVOKABLE bool setPythonSource(const QString newPythonSource);

  /// Convenience method allowing to retrieve the associated scripted instance
  Q_INVOKABLE PyObject* self() const;

  /// Set the name property value.
  /// \sa name
  void setName(QString name) override;

  /// Set flag indicating whether effect operates on segments (true) or the whole segmentation (false).
  void setPerSegment(bool perSegment) override;

  /// If this property is set to true then this effect is enabled only when the segmentation has segment(s) in it.
  void setRequireSegments(bool requireSegments) override;

// API: Methods that are to be reimplemented in the effect subclasses
public:
  /// Get icon for effect to be displayed in segment editor
  QIcon icon() override;

  /// Get help text for effect to be displayed in the help box
  const QString helpText()const override;

  /// Clone editor effect. Override to return a new instance of the effect sub-class
  qCjyxSegmentEditorAbstractEffect* clone() override;

  /// Perform actions to activate the effect (show options frame, etc.)
  void activate() override;

  /// Perform actions to deactivate the effect (hide options frame, destroy actors, etc.)
  void deactivate() override;

  /// Create options frame widgets, make connections, and add them to the main options frame using \sa addOptionsWidget
  void setupOptionsFrame() override;

  /// Create a cursor customized for the given effect, potentially for each view
  QCursor createCursor(qDMMLWidget* viewWidget) override;

  /// Callback function invoked when interaction happens
  /// \param callerInteractor Interactor object that was observed to catch the event
  /// \param eid Event identifier
  /// \param viewWidget Widget of the Cjyx layout view. Can be \sa qDMMLSliceWidget or \sa qDMMLThreeDWidget
  bool processInteractionEvents(vtkRenderWindowInteractor* callerInteractor, unsigned long eid, qDMMLWidget* viewWidget) override;

  /// Callback function invoked when view node is modified
  /// \param callerViewNode View node that was observed to catch the event. Can be either \sa vtkDMMLSliceNode or \sa vtkDMMLViewNode
  /// \param eid Event identifier
  /// \param viewWidget Widget of the Cjyx layout view. Can be \sa qDMMLSliceWidget or \sa qDMMLThreeDWidget
  void processViewNodeEvents(vtkDMMLAbstractViewNode* callerViewNode, unsigned long eid, qDMMLWidget* viewWidget) override;

  /// Set default parameters in the parameter DMML node
  void setDMMLDefaults() override;

  /// Simple mechanism to let the effects know that reference geometry change has changed
  void referenceGeometryChanged() override;
  /// Simple mechanism to let the effects know that master volume has changed
  void masterVolumeNodeChanged() override;
  /// Simple mechanism to let the effects know that the layout has changed
  void layoutChanged() override;
  /// Let the effect know that the interaction node is modified
  void interactionNodeModified(vtkDMMLInteractionNode* interactionNode) override;

public slots:
  /// Update user interface from parameter set node
  void updateGUIFromDMML() override;

  /// Update parameter set node from user interface
  void updateDMMLFromGUI() override;

protected:
  QScopedPointer<qCjyxSegmentEditorScriptedEffectPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qCjyxSegmentEditorScriptedEffect);
  Q_DISABLE_COPY(qCjyxSegmentEditorScriptedEffect);
};

#endif
