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

#include "qDMMLMarkupsAbstractOptionsWidget.h"

// Qt includes
#include <QDebug>

// DMML includes
#include "vtkDMMLMarkupsNode.h"

//-----------------------------------------------------------------------------
qDMMLMarkupsAbstractOptionsWidget::
qDMMLMarkupsAbstractOptionsWidget(QWidget* parent)
  : Superclass(parent), MarkupsNode(nullptr), DMMLScene(nullptr)
{

}

// --------------------------------------------------------------------------
void qDMMLMarkupsAbstractOptionsWidget::setDMMLMarkupsNode(vtkDMMLNode* node)
{
  this->setDMMLMarkupsNode(vtkDMMLMarkupsNode::SafeDownCast(node));
}
