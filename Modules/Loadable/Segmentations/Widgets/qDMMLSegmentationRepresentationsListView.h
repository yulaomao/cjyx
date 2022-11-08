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

#ifndef __qDMMLSegmentationRepresentationsListView_h
#define __qDMMLSegmentationRepresentationsListView_h

// Qt includes
#include <QWidget>

// Segmentations includes
#include "qCjyxSegmentationsModuleWidgetsExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

class vtkDMMLNode;
class qDMMLSegmentationRepresentationsListViewPrivate;

class Q_CJYX_MODULE_SEGMENTATIONS_WIDGETS_EXPORT qDMMLSegmentationRepresentationsListView : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  /// Constructor
  explicit qDMMLSegmentationRepresentationsListView(QWidget* parent = nullptr);
  /// Destructor
  ~qDMMLSegmentationRepresentationsListView() override;

  /// Get segmentation DMML node
  vtkDMMLNode* segmentationNode();

public slots:
  /// Set segmentation DMML node
  void setSegmentationNode(vtkDMMLNode* node);

protected slots:
  /// Populate representations list according to the segmentation node
  void populateRepresentationsList();

  /// Create selected representation using default parameters
  void createRepresentationDefault();

  /// Create selected representation using custom parameters (pops up parameters widget)
  void createRepresentationAdvanced();

  /// Remove selected representation
  void removeRepresentation();

  /// Make selected representation the master representation in segmentation
  void makeMaster();

protected:
  QScopedPointer<qDMMLSegmentationRepresentationsListViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLSegmentationRepresentationsListView);
  Q_DISABLE_COPY(qDMMLSegmentationRepresentationsListView);
};

#endif
