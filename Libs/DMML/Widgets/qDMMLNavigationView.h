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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qDMMLNavigationView_h
#define __qDMMLNavigationView_h

// CTK includes
#include <ctkVTKObject.h>
#include <ctkVTKThumbnailView.h>

#include "qDMMLWidgetsExport.h"

class qDMMLNavigationViewPrivate;
class vtkDMMLScene;
class vtkDMMLViewNode;

/// Specialized ctkVTKThumbnailView that recomputes its bounds when the dmml
/// scene is updated and changes its background color based on a dmml view node
/// TODO: Add utility function setThreeDView to extract automatically the
/// renderer, dmml scene and dmml view node.
class QDMML_WIDGETS_EXPORT qDMMLNavigationView : public ctkVTKThumbnailView
{
  Q_OBJECT
  QVTK_OBJECT

public:
  /// Superclass typedef
  typedef ctkVTKThumbnailView Superclass;

  /// Constructors
  explicit qDMMLNavigationView(QWidget* parent = nullptr);
  ~qDMMLNavigationView() override;

public slots:

  /// Set the DMML \a scene that should be listened for events
  void setDMMLScene(vtkDMMLScene* newScene);

  /// Set/Get \a viewNode
  void setDMMLViewNode(vtkDMMLViewNode* newViewNode);
  vtkDMMLViewNode* dmmlViewNode()const;

protected slots:
  void updateFromDMMLViewNode();
  void updateFromDMMLScene();

protected:
  QScopedPointer<qDMMLNavigationViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLNavigationView);
  Q_DISABLE_COPY(qDMMLNavigationView);
};

#endif
