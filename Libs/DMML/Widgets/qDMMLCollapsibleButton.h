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

#ifndef __qDMMLCollapsibleButton_h
#define __qDMMLCollapsibleButton_h

// CTK includes
#include <ctkCollapsibleButton.h>

// qDMMLWidget includes
#include "qDMMLWidgetsExport.h"

class qDMMLCollapsibleButtonPrivate;
class vtkDMMLScene;

/// This class is intended to improve visual appearance and convenience of ctkCollapsibleButton.
///
/// Currently the visual appearance is the same as its base class.
///
/// The dmmlSceneChanged signal can be used to simplify scene settings in Qt Designer:
/// it allows drawing one long signal/slot arrow from the top-level widget to the collapsible button
/// and a short signal/slot arrow from the collapsible button to each child widget in it.
class QDMML_WIDGETS_EXPORT qDMMLCollapsibleButton : public ctkCollapsibleButton
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef ctkCollapsibleButton Superclass;

  /// Constructors
  explicit qDMMLCollapsibleButton(QWidget* parent = nullptr);
  ~qDMMLCollapsibleButton() override;

  /// Return a pointer on the DMML scene
  vtkDMMLScene* dmmlScene() const;

public slots:
  void setDMMLScene(vtkDMMLScene* scene);

signals:
  void dmmlSceneChanged(vtkDMMLScene*);

protected:
  QScopedPointer<qDMMLCollapsibleButtonPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLCollapsibleButton);
  Q_DISABLE_COPY(qDMMLCollapsibleButton);
};

#endif
