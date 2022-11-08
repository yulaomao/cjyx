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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Qt includes

// qDMML includes
#include "qDMMLNodeAttributeTableWidget.h"
#include "ui_qDMMLNodeAttributeTableWidget.h"

// DMML includes
#include <vtkDMMLNode.h>

// STD includes
#include <sstream>

// --------------------------------------------------------------------------
class qDMMLNodeAttributeTableWidgetPrivate: public Ui_qDMMLNodeAttributeTableWidget
{
  Q_DECLARE_PUBLIC(qDMMLNodeAttributeTableWidget);
protected:
  qDMMLNodeAttributeTableWidget* const q_ptr;
public:
  qDMMLNodeAttributeTableWidgetPrivate(qDMMLNodeAttributeTableWidget& object);
  void init();

  vtkWeakPointer<vtkDMMLNode> DMMLNode;
};

// --------------------------------------------------------------------------
qDMMLNodeAttributeTableWidgetPrivate::qDMMLNodeAttributeTableWidgetPrivate(qDMMLNodeAttributeTableWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qDMMLNodeAttributeTableWidgetPrivate::init()
{
  Q_Q(qDMMLNodeAttributeTableWidget);
  this->setupUi(q);

  QObject::connect(this->AddAttributeButton, SIGNAL(clicked()),
          this->DMMLNodeAttributeTableView, SLOT(addAttribute()));
  QObject::connect(this->RemoveAttributeButton, SIGNAL(clicked()),
          this->DMMLNodeAttributeTableView, SLOT(removeSelectedAttributes()));
}

// --------------------------------------------------------------------------
// qDMMLNodeAttributeTableWidget methods

// --------------------------------------------------------------------------
qDMMLNodeAttributeTableWidget::qDMMLNodeAttributeTableWidget(QWidget* _parent)
  : QWidget(_parent)
  , d_ptr(new qDMMLNodeAttributeTableWidgetPrivate(*this))
{
  Q_D(qDMMLNodeAttributeTableWidget);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLNodeAttributeTableWidget::~qDMMLNodeAttributeTableWidget() = default;

// --------------------------------------------------------------------------
vtkDMMLNode* qDMMLNodeAttributeTableWidget::dmmlNode()const
{
  Q_D(const qDMMLNodeAttributeTableWidget);
  return d->DMMLNode.GetPointer();
}

// --------------------------------------------------------------------------
void qDMMLNodeAttributeTableWidget::setDMMLNode(vtkDMMLNode* node)
{
  Q_D(qDMMLNodeAttributeTableWidget);
  d->DMMLNodeAttributeTableView->setInspectedNode(node);

  qvtkReconnect(d->DMMLNode.GetPointer(), node, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromDMML()));
  qvtkReconnect(d->DMMLNode.GetPointer(), node, vtkDMMLNode::ReferenceAddedEvent,
                this, SLOT(updateWidgetFromDMML()));
  qvtkReconnect(d->DMMLNode.GetPointer(), node, vtkDMMLNode::ReferenceModifiedEvent,
                this, SLOT(updateWidgetFromDMML()));
  qvtkReconnect(d->DMMLNode.GetPointer(), node, vtkDMMLNode::ReferenceRemovedEvent,
                this, SLOT(updateWidgetFromDMML()));
  d->DMMLNode = node;

  this->updateWidgetFromDMML();
}

// --------------------------------------------------------------------------
qDMMLNodeAttributeTableView* qDMMLNodeAttributeTableWidget::tableView()
{
  Q_D(qDMMLNodeAttributeTableWidget);
  return d->DMMLNodeAttributeTableView;
}

//------------------------------------------------------------------------------
void qDMMLNodeAttributeTableWidget::showEvent(QShowEvent *)
{
  // Update the widget, now that it becomes becomes visible
  // (we might have missed some updates, because widget contents is not updated
  // if the widget is not visible).
  this->updateWidgetFromDMML();
}

//------------------------------------------------------------------------------
void qDMMLNodeAttributeTableWidget::updateWidgetFromDMML()
{
  Q_D(qDMMLNodeAttributeTableWidget);
  if (!this->isVisible())
    {
    // Getting the node information may be expensive,
    // so if the widget is not visible then do not update
    return;
    }

  if (d->DMMLNode.GetPointer())
    {
    d->NodeInformationGroupBox->setVisible(true);
    std::stringstream infoStream;
    d->DMMLNode->PrintSelf(infoStream, vtkIndent(0));
    d->DMMLNodeInfoLabel->setText(infoStream.str().c_str());
    }
  else
    {
    d->NodeInformationGroupBox->setVisible(false);
    d->DMMLNodeInfoLabel->clear();
    }
}
