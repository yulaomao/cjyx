/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

==============================================================================*/


#ifndef __qDMMLTransformInfoWidget_h
#define __qDMMLTransformInfoWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// Cjyx includes
#include "qDMMLWidget.h"

#include "qCjyxTransformsModuleWidgetsExport.h"

class qDMMLTransformInfoWidgetPrivate;
class vtkDMMLTransformNode;
class vtkDMMLNode;

/// \ingroup Cjyx_QtModules_Transforms
class Q_CJYX_MODULE_TRANSFORMS_WIDGETS_EXPORT
qDMMLTransformInfoWidget
  : public qDMMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qDMMLWidget Superclass;
  qDMMLTransformInfoWidget(QWidget *parent=nullptr);
  ~qDMMLTransformInfoWidget() override;

  vtkDMMLTransformNode* dmmlTransformNode()const;

  vtkDMMLScene* dmmlScene()const;

public slots:

  /// Set the DMML node of interest
  /// Note that setting transformNode to 0 will disable the widget
  void setDMMLTransformNode(vtkDMMLTransformNode* transformNode);

  void setDMMLScene(vtkDMMLScene* scene) override;

  /// Utility function that calls setDMMLTransformNode(vtkDMMLTransformNode* transformNode)
  /// It's useful to connect to vtkDMMLNode* signals
  void setDMMLTransformNode(vtkDMMLNode* node);

  /// Process event function
  void processEvent(vtkObject* sender, void* callData, unsigned long eventId, void* clientData);

protected slots:
  void updateWidgetFromDMML();
  void updateTransformVectorDisplayFromDMML();

protected:

  void showEvent(QShowEvent *) override;

  QScopedPointer<qDMMLTransformInfoWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLTransformInfoWidget);
  Q_DISABLE_COPY(qDMMLTransformInfoWidget);
};

#endif
