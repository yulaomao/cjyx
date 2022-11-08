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

#ifndef __qDMMLSettingsUnitWidget_h
#define __qDMMLSettingsUnitWidget_h

// CTK includes
#include <ctkVTKObject.h>

// DMML includes
class vtkDMMLNode;

// qDMML includes
class qDMMLUnitWidget;
class qDMMLNodeComboBox;

// Qt includes
#include <QWidget>

// Unit includes
#include "qCjyxUnitsModuleWidgetsExport.h"
class qDMMLSettingsUnitWidgetPrivate;
class vtkCjyxUnitsLogic;

class Q_CJYX_MODULE_UNITS_WIDGETS_EXPORT qDMMLSettingsUnitWidget
  : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef QWidget Superclass;
  qDMMLSettingsUnitWidget(QWidget *parent=nullptr);
  ~qDMMLSettingsUnitWidget() override;

  /// Set the units logic scene for the preset comboxes
  virtual void setUnitsLogic(vtkCjyxUnitsLogic* logic);

  qDMMLNodeComboBox* unitComboBox();
  qDMMLUnitWidget* unitWidget();

protected:
  QScopedPointer<qDMMLSettingsUnitWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLSettingsUnitWidget);
  Q_DISABLE_COPY(qDMMLSettingsUnitWidget);
};

#endif
