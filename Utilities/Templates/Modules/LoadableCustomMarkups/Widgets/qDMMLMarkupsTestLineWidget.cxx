/*==============================================================================

  Copyright (c) The Intervention Centre
  Oslo University Hospital, Oslo, Norway. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Rafael Palomar (The Intervention Centre,
  Oslo University Hospital) and was supported by The Research Council of Norway
  through the ALive project (grant nr. 311393).

  ==============================================================================*/

#include "qDMMLMarkupsTestLineWidget.h"
#include "ui_qDMMLMarkupsTestLineWidget.h"

// DMML Nodes includes
#include "vtkDMMLMarkupsTestLineNode.h"

// VTK includes
#include <vtkWeakPointer.h>

// --------------------------------------------------------------------------
class qDMMLMarkupsTestLineWidgetPrivate:
  public Ui_qDMMLMarkupsTestLineWidget
{
  Q_DECLARE_PUBLIC(qDMMLMarkupsTestLineWidget);

protected:
  qDMMLMarkupsTestLineWidget* const q_ptr;

public:
  qDMMLMarkupsTestLineWidgetPrivate(qDMMLMarkupsTestLineWidget* object);
  void setupUi(qDMMLMarkupsTestLineWidget*);

  vtkWeakPointer<vtkDMMLMarkupsTestLineNode> MarkupsTestLineNode;
};

// --------------------------------------------------------------------------
qDMMLMarkupsTestLineWidgetPrivate::
qDMMLMarkupsTestLineWidgetPrivate(qDMMLMarkupsTestLineWidget* object)
  : q_ptr(object)
{

}

// --------------------------------------------------------------------------
void qDMMLMarkupsTestLineWidgetPrivate::setupUi(qDMMLMarkupsTestLineWidget* widget)
{
  Q_Q(qDMMLMarkupsTestLineWidget);

  this->Ui_qDMMLMarkupsTestLineWidget::setupUi(widget);
  this->lineTestCollapsibleButton->setVisible(false);
}

// --------------------------------------------------------------------------
qDMMLMarkupsTestLineWidget::
qDMMLMarkupsTestLineWidget(QWidget *parent)
  : Superclass(parent),
    d_ptr(new qDMMLMarkupsTestLineWidgetPrivate(this))
{
  this->setup();
}

// --------------------------------------------------------------------------
qDMMLMarkupsTestLineWidget::~qDMMLMarkupsTestLineWidget() = default;

// --------------------------------------------------------------------------
void qDMMLMarkupsTestLineWidget::setup()
{
  Q_D(qDMMLMarkupsTestLineWidget);
  d->setupUi(this);
}
// --------------------------------------------------------------------------
void qDMMLMarkupsTestLineWidget::updateWidgetFromDMML()
{
  Q_D(qDMMLMarkupsTestLineWidget);

  if (!this->canManageDMMLMarkupsNode(d->MarkupsTestLineNode))
    {
    d->lineTestCollapsibleButton->setVisible(false);
    return;
    }

  d->lineTestCollapsibleButton->setVisible(true);
}


//-----------------------------------------------------------------------------
bool qDMMLMarkupsTestLineWidget::canManageDMMLMarkupsNode(vtkDMMLMarkupsNode *markupsNode) const
{
  Q_D(const qDMMLMarkupsTestLineWidget);

  vtkDMMLMarkupsTestLineNode* testLineNode= vtkDMMLMarkupsTestLineNode::SafeDownCast(markupsNode);
  if (!testLineNode)
    {
    return false;
    }

  return true;
}

// --------------------------------------------------------------------------
void qDMMLMarkupsTestLineWidget::setDMMLMarkupsNode(vtkDMMLMarkupsNode* markupsNode)
{
  Q_D(qDMMLMarkupsTestLineWidget);

  d->MarkupsTestLineNode = vtkDMMLMarkupsTestLineNode::SafeDownCast(markupsNode);
  this->setEnabled(markupsNode != nullptr);
}
