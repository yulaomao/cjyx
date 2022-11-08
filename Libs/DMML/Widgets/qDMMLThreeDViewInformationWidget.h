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

#ifndef __qDMMLThreeDViewInformationWidget_h
#define __qDMMLThreeDViewInformationWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>

// qDMMLWidget includes
#include "qDMMLWidget.h"

#include "qDMMLWidgetsExport.h"

class qDMMLThreeDViewInformationWidgetPrivate;
class vtkDMMLNode;
class vtkDMMLViewNode;

class QDMML_WIDGETS_EXPORT qDMMLThreeDViewInformationWidget : public qDMMLWidget
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef qDMMLWidget Superclass;

  /// Constructors
  explicit qDMMLThreeDViewInformationWidget(QWidget* parent = nullptr);
  ~qDMMLThreeDViewInformationWidget() override;

  /// Get \a viewNode
  vtkDMMLViewNode* dmmlViewNode()const;

public slots:

  /// Set a new viewNode.
  void setDMMLViewNode(vtkDMMLNode* newNode);

  /// Set a new SliceNode.
  void setDMMLViewNode(vtkDMMLViewNode* newSliceNode);

  /// Set view group
  void setViewGroup(int viewGroup);

protected:
  QScopedPointer<qDMMLThreeDViewInformationWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLThreeDViewInformationWidget);
  Q_DISABLE_COPY(qDMMLThreeDViewInformationWidget);
};

#endif
