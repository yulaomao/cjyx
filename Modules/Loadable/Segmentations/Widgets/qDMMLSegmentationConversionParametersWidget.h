/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

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

#ifndef __qDMMLSegmentationConversionParametersWidget_h
#define __qDMMLSegmentationConversionParametersWidget_h

// Qt includes
#include <QWidget>

// Segmentations includes
#include "qCjyxSegmentationsModuleWidgetsExport.h"

#include "vtkSegmentationConverter.h"
#include "vtkSegmentationConverterRule.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

class vtkDMMLNode;
class qDMMLSegmentationConversionParametersWidgetPrivate;
class QTableWidgetItem;
class QItemSelectionModel;

class Q_CJYX_MODULE_SEGMENTATIONS_WIDGETS_EXPORT qDMMLSegmentationConversionParametersWidget : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

  Q_PROPERTY(QString targetRepresentationName READ targetRepresentationName WRITE setTargetRepresentationName)

public:
  /// Constructor
  explicit qDMMLSegmentationConversionParametersWidget(QWidget* parent = nullptr);
  /// Destructor
  ~qDMMLSegmentationConversionParametersWidget() override;

  /// Get segmentation DMML node
  vtkDMMLNode* segmentationNode();

  /// Get target representation name
  QString targetRepresentationName();

  /// Return selected path
  vtkSegmentationConverter::ConversionPathType selectedPath();

  /// Return chosen conversion parameters
  vtkSegmentationConverterRule::ConversionParameterListType conversionParameters();

signals:
  /// Emitted when conversion is done
  void conversionDone();

public slots:
  /// Set segmentation DMML node
  void setSegmentationNode(vtkDMMLNode* node);

  /// Set target representation name
  void setTargetRepresentationName(QString representationName);

protected slots:
  /// Populate paths table according to the conversion
  void populatePathsTable();

  /// Populate parameters table according to the selected path
  void populateParametersTable();

  /// Handle editing of generic conversation parameters
  void onParameterChanged(QTableWidgetItem* changedItem);

  /// Show segmentation geometry dialog to specify reference image geometry
  /// The button appears in the row of the reference image geometry conversion parameter, which is a special case.
  void onSpecifyGeometryButtonClicked();

  /// Create selected representation
  void applyConversion();

protected:
  QScopedPointer<qDMMLSegmentationConversionParametersWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLSegmentationConversionParametersWidget);
  Q_DISABLE_COPY(qDMMLSegmentationConversionParametersWidget);
};

#endif
