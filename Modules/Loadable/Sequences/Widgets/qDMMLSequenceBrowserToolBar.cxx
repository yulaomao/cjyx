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

// qDMML includes
#include "qDMMLSequenceBrowserToolBar.h"
#include "qDMMLNodeComboBox.h"
#include "qDMMLSequenceBrowserPlayWidget.h"
#include "qDMMLSequenceBrowserSeekWidget.h"

// DMML includes
#include <vtkDMMLScene.h>
#include <vtkDMMLSequenceBrowserNode.h>

// VTK includes
#include <vtkWeakPointer.h>

//-----------------------------------------------------------------------------
class qDMMLSequenceBrowserToolBarPrivate
{
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qDMMLSequenceBrowserToolBar);
protected:
  qDMMLSequenceBrowserToolBar* const q_ptr;
public:
  qDMMLSequenceBrowserToolBarPrivate(qDMMLSequenceBrowserToolBar& object);
  void init();
  void setDMMLScene(vtkDMMLScene* newScene);
  qDMMLNodeComboBox* SequenceBrowserNodeSelector;
  qDMMLSequenceBrowserPlayWidget* SequenceBrowserPlayWidget;
  qDMMLSequenceBrowserSeekWidget* SequenceBrowserSeekWidget;
};

//--------------------------------------------------------------------------
// qDMMLSequenceBrowserToolBarPrivate methods

//---------------------------------------------------------------------------
qDMMLSequenceBrowserToolBarPrivate::qDMMLSequenceBrowserToolBarPrivate(qDMMLSequenceBrowserToolBar& object)
  : q_ptr(&object)
{
  this->SequenceBrowserNodeSelector = 0;
  this->SequenceBrowserPlayWidget = 0;
  this->SequenceBrowserSeekWidget = 0;
}

//---------------------------------------------------------------------------
void qDMMLSequenceBrowserToolBarPrivate::init()
{
  Q_Q(qDMMLSequenceBrowserToolBar);

  this->SequenceBrowserNodeSelector = new qDMMLNodeComboBox();
  this->SequenceBrowserNodeSelector->setNodeTypes(QStringList(QString("vtkDMMLSequenceBrowserNode")));
  this->SequenceBrowserNodeSelector->setNoneEnabled(false);
  this->SequenceBrowserNodeSelector->setAddEnabled(false);
  this->SequenceBrowserNodeSelector->setRenameEnabled(true);
  this->SequenceBrowserNodeSelector->setMaximumWidth(350);

  this->SequenceBrowserPlayWidget = new qDMMLSequenceBrowserPlayWidget();
  this->SequenceBrowserPlayWidget->setMaximumWidth(450);

  this->SequenceBrowserSeekWidget = new qDMMLSequenceBrowserSeekWidget();
  this->SequenceBrowserSeekWidget->setMinimumWidth(150);
  this->SequenceBrowserSeekWidget->setMaximumWidth(350);

  q->addWidget(this->SequenceBrowserPlayWidget);
  q->addWidget(this->SequenceBrowserSeekWidget);
  q->addWidget(this->SequenceBrowserNodeSelector);

  this->SequenceBrowserPlayWidget->setPlayPauseShortcut("Ctrl+Shift+Down");
  this->SequenceBrowserPlayWidget->setPreviousFrameShortcut("Ctrl+Shift+Left");
  this->SequenceBrowserPlayWidget->setNextFrameShortcut("Ctrl+Shift+Right");

  QObject::connect(this->SequenceBrowserNodeSelector, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
    this->SequenceBrowserPlayWidget, SLOT(setDMMLSequenceBrowserNode(vtkDMMLNode*)) );
  QObject::connect(this->SequenceBrowserNodeSelector, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
    this->SequenceBrowserSeekWidget, SLOT(setDMMLSequenceBrowserNode(vtkDMMLNode*)) );
  QObject::connect(this->SequenceBrowserNodeSelector, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
    q, SIGNAL(activeBrowserNodeChanged(vtkDMMLNode*)) );
}
// --------------------------------------------------------------------------
void qDMMLSequenceBrowserToolBarPrivate::setDMMLScene(vtkDMMLScene* newScene)
{
  Q_Q(qDMMLSequenceBrowserToolBar);

  this->SequenceBrowserNodeSelector->setDMMLScene(newScene);
}

// --------------------------------------------------------------------------
// qDMMLSequenceBrowserToolBar methods

// --------------------------------------------------------------------------
qDMMLSequenceBrowserToolBar::qDMMLSequenceBrowserToolBar(const QString& title, QWidget* parentWidget)
  :Superclass(title, parentWidget)
   , d_ptr(new qDMMLSequenceBrowserToolBarPrivate(*this))
{
  Q_D(qDMMLSequenceBrowserToolBar);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLSequenceBrowserToolBar::qDMMLSequenceBrowserToolBar(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qDMMLSequenceBrowserToolBarPrivate(*this))
{
  Q_D(qDMMLSequenceBrowserToolBar);
  d->init();
}

//---------------------------------------------------------------------------
qDMMLSequenceBrowserToolBar::~qDMMLSequenceBrowserToolBar() = default;

// --------------------------------------------------------------------------
void qDMMLSequenceBrowserToolBar::setDMMLScene(vtkDMMLScene* scene)
{
  Q_D(qDMMLSequenceBrowserToolBar);
  d->setDMMLScene(scene);
}

// --------------------------------------------------------------------------
vtkDMMLSequenceBrowserNode* qDMMLSequenceBrowserToolBar::activeBrowserNode()
{
  Q_D(qDMMLSequenceBrowserToolBar);
  return vtkDMMLSequenceBrowserNode::SafeDownCast(d->SequenceBrowserNodeSelector->currentNode());
}

// --------------------------------------------------------------------------
void qDMMLSequenceBrowserToolBar::setActiveBrowserNode(vtkDMMLSequenceBrowserNode * newActiveBrowserNode)
{
  Q_D(qDMMLSequenceBrowserToolBar);
  d->SequenceBrowserNodeSelector->setCurrentNode(newActiveBrowserNode);
}
