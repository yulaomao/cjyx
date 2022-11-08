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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// CTK includes
#include <ctkComboBox.h>

// Logic includes
#include "vtkCjyxUnitsLogic.h"

// Qt includes
#include <QDebug>
#include <QWidget>

// Cjyx includes
#include "qDMMLSettingsUnitWidget.h"
#include "qDMMLUnitWidget.h"
#include "ui_qDMMLSettingsUnitWidget.h"

// DMML includes
#include "vtkDMMLNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSelectionNode.h"
#include "vtkDMMLUnitNode.h"

// STD
#include <vector>

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_ExtensionTemplate
class qDMMLSettingsUnitWidgetPrivate: public Ui_qDMMLSettingsUnitWidget
{
  Q_DECLARE_PUBLIC(qDMMLSettingsUnitWidget);
protected:
  qDMMLSettingsUnitWidget* const q_ptr;

public:
  qDMMLSettingsUnitWidgetPrivate(qDMMLSettingsUnitWidget& obj);
  void setupUi(qDMMLSettingsUnitWidget*);

  vtkCjyxUnitsLogic* Logic;
};

//-----------------------------------------------------------------------------
// qDMMLSettingsUnitWidgetPrivate methods

//-----------------------------------------------------------------------------
qDMMLSettingsUnitWidgetPrivate::qDMMLSettingsUnitWidgetPrivate(
  qDMMLSettingsUnitWidget& object)
  : q_ptr(&object)
{
  this->Logic = nullptr;
}

//-----------------------------------------------------------------------------
void qDMMLSettingsUnitWidgetPrivate::setupUi(qDMMLSettingsUnitWidget* q)
{
  this->Ui_qDMMLSettingsUnitWidget::setupUi(q);

  QObject::connect(this->UnitNodeComboBox,
    SIGNAL(currentNodeChanged(vtkDMMLNode*)),
    this->UnitInfoWidget, SLOT(setCurrentNode(vtkDMMLNode*)));

  // Hide unit label and combobox for now
  this->UnitLabel->setVisible(false);
  this->UnitNodeComboBox->setVisible(false);
}

//-----------------------------------------------------------------------------
// qDMMLSettingsUnitWidget methods

//-----------------------------------------------------------------------------
qDMMLSettingsUnitWidget::qDMMLSettingsUnitWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qDMMLSettingsUnitWidgetPrivate(*this) )
{
  Q_D(qDMMLSettingsUnitWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qDMMLSettingsUnitWidget::~qDMMLSettingsUnitWidget() = default;

//-----------------------------------------------------------------------------
void qDMMLSettingsUnitWidget::setUnitsLogic(vtkCjyxUnitsLogic* logic)
{
  Q_D(qDMMLSettingsUnitWidget);
  if (logic == d->Logic)
    {
    return;
    }

  d->Logic = logic;
  d->UnitInfoWidget->setDMMLScene(d->Logic ? d->Logic->GetUnitsScene() : nullptr);
}

//-----------------------------------------------------------------------------
qDMMLNodeComboBox* qDMMLSettingsUnitWidget::unitComboBox()
{
  Q_D(qDMMLSettingsUnitWidget);
  return d->UnitNodeComboBox;
}

//-----------------------------------------------------------------------------
qDMMLUnitWidget* qDMMLSettingsUnitWidget::unitWidget()
{
  Q_D(qDMMLSettingsUnitWidget);
  return d->UnitInfoWidget;
}
