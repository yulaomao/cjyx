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

#ifndef __qDMMLNodeAttributeTableWidget_h
#define __qDMMLNodeAttributeTableWidget_h

// Qt includes
#include <QWidget>

// DMMLWidgets includes
#include "qDMMLWidgetsExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

class vtkDMMLNode;
class qDMMLNodeAttributeTableWidgetPrivate;
class qDMMLNodeAttributeTableView;

class QDMML_WIDGETS_EXPORT qDMMLNodeAttributeTableWidget : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  /// Constructors
  explicit qDMMLNodeAttributeTableWidget(QWidget* parent = nullptr);
  ~qDMMLNodeAttributeTableWidget() override;

  /// Get node attribute table view
  qDMMLNodeAttributeTableView* tableView();

  /// Get the inspected DMML node
  vtkDMMLNode* dmmlNode()const;

public slots:
  /// Set the inspected DMML node
  void setDMMLNode(vtkDMMLNode* node);

  /// Refreshes the widget contents (useful for keeping displayed contents up-to-date without invoking node modified event)
  void updateWidgetFromDMML();

protected:
  void showEvent(QShowEvent *) override;

  QScopedPointer<qDMMLNodeAttributeTableWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLNodeAttributeTableWidget);
  Q_DISABLE_COPY(qDMMLNodeAttributeTableWidget);
};

#endif
