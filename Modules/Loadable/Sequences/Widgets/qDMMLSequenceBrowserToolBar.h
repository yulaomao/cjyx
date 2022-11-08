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

==============================================================================*/

#ifndef __qDMMLSequenceBrowserToolBar_h
#define __qDMMLSequenceBrowserToolBar_h

// Qt includes
//#include <QSignalMapper>
#include <QToolBar>

// CTK includes
#include <ctkPimpl.h>
// no ui begin
#include <ctkVTKObject.h>
// no ui end

// qDMMLWidget includes
#include "qDMMLWidget.h"
#include "qCjyxSequencesModuleWidgetsExport.h"

class qDMMLSequenceBrowserToolBarPrivate;
class vtkDMMLNode;
class vtkDMMLScene;
class vtkDMMLSequenceBrowserNode;

class Q_CJYX_MODULE_SEQUENCES_WIDGETS_EXPORT qDMMLSequenceBrowserToolBar : public QToolBar
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef QToolBar Superclass;

  /// Constructor
  /// Title is the name of the toolbar (can appear using right click on the toolbar area)
  qDMMLSequenceBrowserToolBar(const QString& title, QWidget* parent = 0);
  qDMMLSequenceBrowserToolBar(QWidget* parent = 0);
  ~qDMMLSequenceBrowserToolBar() override;

  Q_INVOKABLE vtkDMMLSequenceBrowserNode* activeBrowserNode();

public slots:
  virtual void setDMMLScene(vtkDMMLScene* newScene);
  void setActiveBrowserNode(vtkDMMLSequenceBrowserNode* newActiveBrowserNode);

signals:
  void dmmlSceneChanged(vtkDMMLScene*);
  void activeBrowserNodeChanged(vtkDMMLNode* activeBrowserNode);

protected:
  QScopedPointer<qDMMLSequenceBrowserToolBarPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLSequenceBrowserToolBar);
  Q_DISABLE_COPY(qDMMLSequenceBrowserToolBar);
};

#endif
