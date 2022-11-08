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

#ifndef __qDMMLMalformedWidget_h_
#define __qDMMLMalformedWidget_h_

// Markups widgets includes
#include "qDMMLMarkupsAbstractOptionsWidget.h"

//------------------------------------------------------------------------------
class qDMMLMarkupsMalformedWidgetPrivate;
class vtkDMMLMarkupsNode;

//------------------------------------------------------------------------------
class qDMMLMarkupsMalformedWidget : public qDMMLMarkupsAbstractOptionsWidget
{
  Q_OBJECT

  Q_PROPERTY(QString className READ className CONSTANT);

public:
  typedef qDMMLMarkupsAbstractOptionsWidget Superclass;
  qDMMLMarkupsMalformedWidget(QWidget* parent=nullptr);

  /// Gets the name of the additional options widget type
  const QString className() const override {return "";}

  void updateWidgetFromDMML() {}

  bool canManageDMMLMarkupsNode(vtkDMMLMarkupsNode *) const override {return false;}  /// Set the DMML node of interest

  void setDMMLMarkupsNode(vtkDMMLMarkupsNode* node) override;

  /// Returns an instance of the widget
  qDMMLMarkupsAbstractOptionsWidget* createInstance() const override
  { return new qDMMLMarkupsMalformedWidget(); }
};

#endif // __qDMMLMalformedWidget_h_
