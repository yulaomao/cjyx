/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Brigham and Women's Hospital

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __qDMMLSequenceBrowserSeekWidget_h
#define __qDMMLSequenceBrowserSeekWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// Cjyx includes
#include "qDMMLWidget.h"

#include "qCjyxSequencesModuleWidgetsExport.h"

class qDMMLSequenceBrowserSeekWidgetPrivate;
class vtkDMMLNode;
class vtkDMMLSequenceBrowserNode;
class QSlider;

/// \ingroup Cjyx_QtModules_Markups
class Q_CJYX_MODULE_SEQUENCES_WIDGETS_EXPORT qDMMLSequenceBrowserSeekWidget
: public qDMMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qDMMLWidget Superclass;
  qDMMLSequenceBrowserSeekWidget(QWidget *newParent = 0);
  ~qDMMLSequenceBrowserSeekWidget() override;

  /// Get access to the internal slider widget.
  /// This allows fine-tuning of parameters such as page step.
  Q_INVOKABLE QSlider* slider() const;

public slots:
  void setDMMLSequenceBrowserNode(vtkDMMLNode* browserNode);
  void setDMMLSequenceBrowserNode(vtkDMMLSequenceBrowserNode* browserNode);
  void setSelectedItemNumber(int itemNumber);

protected slots:
  void onIndexDisplayFormatModified();
  void updateWidgetFromDMML();

protected:
  QScopedPointer<qDMMLSequenceBrowserSeekWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLSequenceBrowserSeekWidget);
  Q_DISABLE_COPY(qDMMLSequenceBrowserSeekWidget);

};

#endif
