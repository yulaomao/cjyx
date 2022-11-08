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

// Texts widgets includes
#include "qDMMLTextWidget.h"

// DMML includes
#include <vtkDMMLTextNode.h>

// CTK includes
#include <ctkMessageBox.h>

// Qt includes
#include <QStyle>

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_CreateModels
class qDMMLTextWidgetPrivate
  : public Ui_qDMMLTextWidget
{
  Q_DECLARE_PUBLIC(qDMMLTextWidget);
protected:
  qDMMLTextWidget* const q_ptr;

public:
  qDMMLTextWidgetPrivate( qDMMLTextWidget& object);
  ~qDMMLTextWidgetPrivate();
  virtual void setupUi(qDMMLTextWidget*);

  vtkSmartPointer<vtkDMMLTextNode> CurrentTextNode;
  bool TextEditModified;
  bool TextNodeContentsModified;

protected:
  bool Editing;

public:
  bool isEditing();
  void setEditing(bool editing);
};

// --------------------------------------------------------------------------
qDMMLTextWidgetPrivate::qDMMLTextWidgetPrivate(qDMMLTextWidget& object)
  : q_ptr(&object)
  , TextEditModified(false)
  , TextNodeContentsModified(false)
  , Editing(false)
{
}

//-----------------------------------------------------------------------------
qDMMLTextWidgetPrivate::~qDMMLTextWidgetPrivate() = default;

// --------------------------------------------------------------------------
void qDMMLTextWidgetPrivate::setupUi(qDMMLTextWidget* widget)
{
  this->Ui_qDMMLTextWidget::setupUi(widget);
}

// --------------------------------------------------------------------------
bool qDMMLTextWidgetPrivate::isEditing()
{
  return this->Editing;
}

// --------------------------------------------------------------------------
void qDMMLTextWidgetPrivate::setEditing(bool editing)
{
  Q_Q(qDMMLTextWidget);
  if (this->Editing == editing)
    {
    return;
    }

  this->Editing = editing;
  q->editingChanged(this->Editing);
}

//-----------------------------------------------------------------------------
// qDMMLTextWidget methods

//-----------------------------------------------------------------------------
qDMMLTextWidget::qDMMLTextWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr(new qDMMLTextWidgetPrivate(*this))
{
  this->setup();
}

//-----------------------------------------------------------------------------
qDMMLTextWidget::~qDMMLTextWidget()
{
  this->setDMMLTextNode(nullptr);
}

//-----------------------------------------------------------------------------
void qDMMLTextWidget::setup()
{
  Q_D(qDMMLTextWidget);

  d->setupUi(this);

  connect(d->TextEdit, SIGNAL(textChanged()), this, SLOT(onTextEditChanged()));
  connect(d->EditButton, SIGNAL(clicked()), this, SLOT(startEdits()));
  connect(d->CancelButton, SIGNAL(clicked()), this, SLOT(cancelEdits()));
  connect(d->SaveButton, SIGNAL(clicked()), this, SLOT(saveEdits()));
  connect(this, SIGNAL(updateWidgetFromDMMLRequested()), this, SLOT(updateWidgetFromDMML()));
  connect(this, SIGNAL(updateDMMLFromWidgetRequested()), this, SLOT(updateDMMLFromWidget()));

  this->updateWidgetFromDMMLRequested();
}

//------------------------------------------------------------------------------
void qDMMLTextWidget::setDMMLNode(vtkDMMLNode* node)
{
  this->setDMMLTextNode(vtkDMMLTextNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qDMMLTextWidget::setDMMLTextNode(vtkDMMLTextNode* node)
{
  Q_D(qDMMLTextWidget);
  if (node == d->CurrentTextNode)
    {
    // not changed
    return;
    }

  // Reconnect the appropriate nodes
  this->qvtkReconnect(d->CurrentTextNode, node, vtkDMMLTextNode::TextModifiedEvent, this, SLOT(onTextNodeContentsModified()));
  d->CurrentTextNode = node;
  d->Editing = false;
  this->updateWidgetFromDMMLRequested();
  this->dmmlNodeChanged(node);
}

//------------------------------------------------------------------------------
vtkDMMLTextNode* qDMMLTextWidget::dmmlTextNode()const
{
  Q_D(const qDMMLTextWidget);
  return d->CurrentTextNode;
}

//------------------------------------------------------------------------------
vtkDMMLNode* qDMMLTextWidget::dmmlNode()const
{
  Q_D(const qDMMLTextWidget);
  return d->CurrentTextNode;
}

//-----------------------------------------------------------------------------
void qDMMLTextWidget::onTextNodeContentsModified()
{
  Q_D(qDMMLTextWidget);
  d->TextNodeContentsModified = true;
  this->updateWidgetFromDMMLRequested();
}

//-----------------------------------------------------------------------------
void qDMMLTextWidget::updateWidgetFromDMML()
{
  Q_D(qDMMLTextWidget);

  bool updateText = true;
  bool editing = d->isEditing();
  if (this->ReadOnly)
    {
    updateText = true;
    d->TextEdit->setReadOnly(true);
    d->EditButton->setVisible(false);
    d->CancelButton->setVisible(false);
    d->SaveButton->setVisible(false);
    }
  else if (this->AutoSave)
    {
    updateText = true;
    d->TextEdit->setReadOnly(false);
    d->EditButton->setVisible(false);
    d->CancelButton->setVisible(false);
    d->SaveButton->setVisible(false);
    }
  else if (editing)
    {
    updateText = false;
    d->TextEdit->setReadOnly(false);
    d->EditButton->setVisible(false);
    d->CancelButton->setVisible(true);
    d->SaveButton->setVisible(true);
    }
  else
    {
    updateText = true;
    d->TextEdit->setReadOnly(true);
    d->EditButton->setVisible(true);
    d->CancelButton->setVisible(false);
    d->SaveButton->setVisible(false);
    }

  d->EditButton->setEnabled(!editing);
  d->CancelButton->setEnabled(editing);
  d->SaveButton->setEnabled(editing);

  bool wasBlocking = d->TextEdit->blockSignals(true);
  if (!d->CurrentTextNode)
    {
    d->TextEdit->setReadOnly(true);
    d->TextEdit->setText("");
    d->TextNodeContentsModified = false;
    d->EditButton->setEnabled(false);
    d->CancelButton->setEnabled(false);
    d->SaveButton->setEnabled(false);
    }
  else if (updateText)
    {
    int position = d->TextEdit->textCursor().position();
    std::string text;
    if (d->CurrentTextNode)
      {
      text = d->CurrentTextNode->GetText();
      }
    d->TextEdit->setText(text.c_str());
    d->TextNodeContentsModified = false;
    QTextCursor cursor = d->TextEdit->textCursor();
    position = std::min(position, d->TextEdit->toPlainText().length());
    cursor.setPosition(position);
    d->TextEdit->setTextCursor(cursor);
    }
  d->TextEdit->blockSignals(wasBlocking);

  if (d->TextNodeContentsModified)
    {
    d->SaveButton->setToolTip("The original text has been modified since editing was started");
    QIcon warningIcon = this->style()->standardIcon(QStyle::SP_MessageBoxWarning);
    d->SaveButton->setIcon(warningIcon);
    d->SaveButton->setIconSize(QSize(12, 12));
    }
  else
    {
    d->SaveButton->setToolTip("The original text has been modified since editing was started");
    d->SaveButton->setIcon(QIcon());
    }

  this->updateWidgetFromDMMLFinished();
}

//-----------------------------------------------------------------------------
void qDMMLTextWidget::updateDMMLFromWidget()
{
  Q_D(qDMMLTextWidget);
  if (!d->CurrentTextNode)
    {
    return;
    }
  std::string text = d->TextEdit->toPlainText().toStdString();
  d->CurrentTextNode->SetText(text.c_str(), VTK_ENCODING_UTF_8);
  this->updateDMMLFromWidgetFinished();
}

//------------------------------------------------------------------------------
void qDMMLTextWidget::setDMMLScene(vtkDMMLScene* scene)
{
  this->Superclass::setDMMLScene(scene);
  this->updateWidgetFromDMMLRequested();
}

//------------------------------------------------------------------------------
bool qDMMLTextWidget::isReadOnly()
{
  return this->ReadOnly;
}

//------------------------------------------------------------------------------
bool qDMMLTextWidget::isEditing()
{
  Q_D(qDMMLTextWidget);
  return d->isEditing();
}

//------------------------------------------------------------------------------
void qDMMLTextWidget::setReadOnly(bool readOnly)
{
  Q_D(qDMMLTextWidget);
  if (this->ReadOnly == readOnly)
    {
    return;
    }
  this->ReadOnly = readOnly;

  this->updateDMMLFromWidgetRequested();
  this->readOnlyChanged(this->ReadOnly);
  this->updateWidgetFromDMMLRequested();
}

//------------------------------------------------------------------------------
bool qDMMLTextWidget::isAutoSave()
{
  Q_D(qDMMLTextWidget);
  return this->AutoSave;
}

//------------------------------------------------------------------------------
void qDMMLTextWidget::setAutoSave(bool autoSave)
{
  Q_D(qDMMLTextWidget);
  if (this->AutoSave == autoSave)
    {
    return;
    }

  this->AutoSave = autoSave;
  if (this->AutoSave)
    {
    d->setEditing(false);
    }

  this->updateDMMLFromWidgetRequested();
  this->autoSaveChanged(this->AutoSave);
  this->updateWidgetFromDMMLRequested();
}

//------------------------------------------------------------------------------
bool qDMMLTextWidget::wordWrap()
{
  Q_D(qDMMLTextWidget);
  if (d->TextEdit->wordWrapMode() == QTextOption::NoWrap)
    {
    return false;
    }
  return true;
}

//------------------------------------------------------------------------------
void qDMMLTextWidget::setWordWrap(bool wordWrap)
{
  Q_D(qDMMLTextWidget);
  d->TextEdit->setWordWrapMode(wordWrap ? QTextOption::WrapAtWordBoundaryOrAnywhere : QTextOption::NoWrap);
}

//------------------------------------------------------------------------------
void qDMMLTextWidget::onTextEditChanged()
{
  Q_D(qDMMLTextWidget);

  d->TextEditModified = true;
  if (!d->CurrentTextNode || !this->AutoSave)
    {
    return;
    }

  this->updateDMMLFromWidgetRequested();
}

//------------------------------------------------------------------------------
void qDMMLTextWidget::startEdits()
{
  Q_D(qDMMLTextWidget);
  d->TextEditModified = false;
  d->TextEdit->setFocus();
  d->setEditing(true);
  this->updateWidgetFromDMMLRequested();
}

//------------------------------------------------------------------------------
void qDMMLTextWidget::cancelEdits()
{
  Q_D(qDMMLTextWidget);
  d->setEditing(false);
  this->updateWidgetFromDMMLRequested();
}

//------------------------------------------------------------------------------
void qDMMLTextWidget::saveEdits()
{
  Q_D(qDMMLTextWidget);
  this->updateDMMLFromWidgetRequested();
  d->setEditing(false);
  this->updateWidgetFromDMMLRequested();
}

//------------------------------------------------------------------------------
QTextEdit* qDMMLTextWidget::textEditWidget()
{
  Q_D(qDMMLTextWidget);
  return d->TextEdit;
}
