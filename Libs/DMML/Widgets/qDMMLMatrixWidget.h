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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qDMMLMatrixWidget_h
#define __qDMMLMatrixWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>
#include <ctkVTKAbstractMatrixWidget.h>

#include "qDMMLWidgetsExport.h"

class vtkDMMLNode;
class vtkDMMLTransformNode;
class vtkMatrix4x4;
class qDMMLMatrixWidgetPrivate;

class QDMML_WIDGETS_EXPORT qDMMLMatrixWidget : public ctkVTKAbstractMatrixWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(CoordinateReferenceType coordinateReference READ coordinateReference WRITE setCoordinateReference)
  Q_ENUMS(CoordinateReferenceType)

public:

  /// Constructors
  typedef ctkVTKAbstractMatrixWidget Superclass;
  explicit qDMMLMatrixWidget(QWidget* parent);
  ~qDMMLMatrixWidget() override;

  ///
  /// Set/Get Coordinate system
  /// By default, the selector coordinate system will be set to GLOBAL
  enum CoordinateReferenceType { GLOBAL, LOCAL };
  void setCoordinateReference(CoordinateReferenceType coordinateReference);
  CoordinateReferenceType coordinateReference() const;

  vtkDMMLTransformNode* dmmlTransformNode()const;

public slots:
  ///
  /// Set the DMML node of interest
  void setDMMLTransformNode(vtkDMMLTransformNode* transformNode);
  void setDMMLTransformNode(vtkDMMLNode* node);

protected slots:
  ///
  /// Triggered upon DMML transform node updates
  void updateMatrix();

  ///
  /// Triggered when the user modifies the cells of the matrix.
  /// Synchronize with the node.
  void updateTransformNode();

protected:
  QScopedPointer<qDMMLMatrixWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLMatrixWidget);
  Q_DISABLE_COPY(qDMMLMatrixWidget);
};

#endif
