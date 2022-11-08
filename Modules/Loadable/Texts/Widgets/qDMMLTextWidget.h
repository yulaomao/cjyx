/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

#ifndef __qDMMLTextWidget_h
#define __qDMMLTextWidget_h

// Cjyx includes
#include "qCjyxWidget.h"

// Text widgets includes
#include "qCjyxTextsModuleWidgetsExport.h"
#include "ui_qDMMLTextWidget.h"

class vtkDMMLNode;
class vtkDMMLTextNode;
class qDMMLTextWidgetPrivate;
class QTextEdit;

/// \ingroup Cjyx_QtModules_Texts
class Q_CJYX_MODULE_TEXTS_WIDGETS_EXPORT qDMMLTextWidget : public qCjyxWidget
{
  Q_OBJECT

public:
  typedef qCjyxWidget Superclass;
  qDMMLTextWidget(QWidget *parent=nullptr);
  ~qDMMLTextWidget() override;

  Q_PROPERTY(bool autoSave READ isAutoSave WRITE setAutoSave);
  Q_PROPERTY(bool editing READ isEditing);
  Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly);
  Q_PROPERTY(bool wordWrap READ wordWrap WRITE setWordWrap);

  /// Get the text node
  Q_INVOKABLE vtkDMMLTextNode* dmmlTextNode() const;
  Q_INVOKABLE vtkDMMLNode* dmmlNode() const;

  /// Returns true if the text editor is read only
  /// If read only is enabled, only the text edit will be shown, and the user will be unable to type in the text edit.
  /// \sa setReadOnly()
  bool isReadOnly();

  /// Returns true if text changes made in the widget must be immediately saved into the text node.
  /// If true, the text editor will propagate text changes to the vtkDMMLTextNode after each keypress.
  /// If false, the text editor will only update the node when "Save" is clicked, and changes from the
  /// text node will not be shown in the widget while the text is being edited.
  /// Save and Cancel buttons are only shown if auto-save is disabled.
  /// \sa setAutoSave()
  bool isAutoSave();

  /// Returns true if the text box is in edit mode
  bool isEditing();

  /// Returns the word wrap mode used in the text editor
  /// \sa setWordWrap()
  bool wordWrap();

  /// Returns the internal text editor widget to allow low-level access and customization.
  Q_INVOKABLE QTextEdit* textEditWidget();

public slots:
  /// Reimplemented from qCjyxWidget
  void setDMMLScene(vtkDMMLScene* scene) override;

  /// Set the read only property of the text editor.
  /// If read only is enabled, only the text edit will be shown, and the user will be unable to edit the text.
  /// \sa isReadOnly()
  void setReadOnly(bool readOnly);

  /// Set the continuous update property of the text editor
  /// If true, the text editor will propagate the text to the vtkDMMLTextNode as it is modified, and vice versa.
  /// If false, the text editor will only update the node when "Save" is clicked, and changes from the vtkDMMLTextNode will not be propagated
  /// if the text is being edited.
  /// When auto update is enabled, only the text edit will be shown.
  /// \sa isAutoSave()
  void setAutoSave(bool autoSave);

  /// Set the word wrap mode to be used by the text editor
  /// \sa wordWrap()
  void setWordWrap(bool wordWrap);

  /// Set the currently observed text node
  /// \sa dmmlTextNode()
  void setDMMLTextNode(vtkDMMLTextNode* textNode);

  /// Utility function to simply connect signals/slots with Qt Designer
  /// \sa dmmlNode()
  void setDMMLNode(vtkDMMLNode* textNode);

public slots:
  /// Start editing mode
  void startEdits();

  /// Finish editing, discarding all changes.
  void cancelEdits();

  /// Finish editing, saving edited contents to the text node.
  void saveEdits();

signals:
  /// This signal is emitted if updates to the widget from the DMML node have been requested.
  /// \sa updateWidgetFromDMMLFinished()
  void updateWidgetFromDMMLRequested();

  /// This signal is emitted if updates to the DMML node from the widget have been requested.
  /// \sa updateDMMLFromWidgetFinished()
  void updateDMMLFromWidgetRequested();

  /// This signal is emitted if updates to the widget from the DMML node have finished.
  /// \sa updateWidgetFromDMMLRequested()
  void updateWidgetFromDMMLFinished();

  /// This signal is emitted if updates to the DMML node from the widget have finished.
  /// \sa updateDMMLFromWidgetRequested()
  void updateDMMLFromWidgetFinished();

  /// This signal is emitted if the node changes
  /// \sa setDMMLNode(), setDMMLTextNode(), dmmlNode(), dmmlTextNode()
  void dmmlNodeChanged(vtkDMMLNode*);

  /// This signal is emitted if the read only property is changed
  void readOnlyChanged(bool);

  /// This signal is emitted if the autoSave property is changed
  bool autoSaveChanged(bool);

  /// This signal is emitted when the user starts/stops the widget edit mode
  void editingChanged(bool);

protected slots:
  /// Update the GUI to reflect the currently selected text node.
  void updateWidgetFromDMML();

  /// Update the DMML node to reflect the currently state of the GUI.
  void updateDMMLFromWidget();

  /// Method invoked when the contents of the text node is modified
  void onTextNodeContentsModified();

  /// Method invoked when the contents of the text edit is changed
  void onTextEditChanged();

protected:
  QScopedPointer<qDMMLTextWidgetPrivate> d_ptr;

  // Setup the UI and establish connections
  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qDMMLTextWidget);
  Q_DISABLE_COPY(qDMMLTextWidget);

protected:
  bool AutoSave{false};
  bool ReadOnly{false};

};

#endif
