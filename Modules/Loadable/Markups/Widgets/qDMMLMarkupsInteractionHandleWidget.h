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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

#ifndef __qDMMLMarkupsInteractionHandleWidget_h
#define __qDMMLMarkupsInteractionHandleWidget_h

// Qt includes
#include <QWidget>

// AnnotationWidgets includes
#include "qCjyxMarkupsModuleWidgetsExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

#include <qDMMLWidget.h>

class vtkDMMLNode;
class vtkDMMLMarkupsDisplayNode;
class qDMMLMarkupsInteractionHandleWidgetPrivate;

class Q_CJYX_MODULE_MARKUPS_WIDGETS_EXPORT qDMMLMarkupsInteractionHandleWidget : public qDMMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  /// Constructors
  explicit qDMMLMarkupsInteractionHandleWidget(QWidget* parent = nullptr);
  ~qDMMLMarkupsInteractionHandleWidget() override;

  /// Returns the current DMML display node
  vtkDMMLMarkupsDisplayNode* dmmlDisplayNode() const;

public slots:
  /// Set the DMML display node
  void setDMMLDisplayNode(vtkDMMLMarkupsDisplayNode* node);

protected slots:
  /// Internal function to update the widgets based on the node/display node
  void updateWidgetFromDMML();

  /// Internal function to update the node based on the widget
  void updateDMMLFromWidget();

protected:
  QScopedPointer<qDMMLMarkupsInteractionHandleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLMarkupsInteractionHandleWidget);
  Q_DISABLE_COPY(qDMMLMarkupsInteractionHandleWidget);
};

#endif
