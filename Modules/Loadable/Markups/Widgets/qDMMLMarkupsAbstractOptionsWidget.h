/*==============================================================================

  Copyright (c) The Intervention Centre
  Oslo University Hospital, Oslo, Norway. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Rafael Palomar (The Intervention Centre,
  Oslo University Hospital) and was supported by The Research Council of Norway
  through the ALive project (grant nr. 311393).

==============================================================================*/

#ifndef __qDMMLMarkupsAbstractOptionsWidget_h_
#define __qDMMLMarkupsAbstractOptionsWidget_h_

// Markups Widgets includes
#include "qCjyxMarkupsModuleWidgetsExport.h"

//DMML includes
#include <vtkDMMLScene.h>
#include <vtkDMMLMarkupsNode.h>

// Qt includes
#include <QWidget>

//-------------------------------------------------------------------------------
class vtkDMMLMarkupsNode;
class vtkDMMLNode;

//-------------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_Markups
/// \name qDMMLMarkupsAbstractOptionsWidget
/// \brief qDMMLMarkupsAbstractOptionsWidget is a base class for the
/// additional options widgets associated to some types of markups.
class Q_CJYX_MODULE_MARKUPS_WIDGETS_EXPORT qDMMLMarkupsAbstractOptionsWidget
  : public QWidget
{
  Q_OBJECT

  Q_PROPERTY(QString className READ className CONSTANT);

public:
  typedef QWidget Superclass;
  qDMMLMarkupsAbstractOptionsWidget(QWidget* parent=nullptr);
  ~qDMMLMarkupsAbstractOptionsWidget()=default;

  /// Updates the widget based on information from DMML.
  virtual void updateWidgetFromDMML() = 0;

  /// Gets the name of the additional options widget type
  virtual const QString className() const = 0;

  // Returns the associated markups node
  vtkDMMLMarkupsNode* dmmlMarkupsNode() const
    {return this->MarkupsNode.GetPointer();}

  // Returns the associated markups node
  vtkDMMLScene* dmmlScene() const
    {return this->DMMLScene.GetPointer();}

  /// Checks whether a given node can be handled by the widget. This allows
  /// using complex logics to determine whether the widget can manage a given
  /// markups node or not.
  virtual bool canManageDMMLMarkupsNode(vtkDMMLMarkupsNode *markupsNode) const = 0;

  /// Clone options widget . Override to return a new instance of the options widget
  virtual qDMMLMarkupsAbstractOptionsWidget* createInstance() const = 0;

public slots:
  /// Sets the vtkDMMLNode to operate on.
  void setDMMLMarkupsNode(vtkDMMLNode* markupsNode);
  /// Sets the vtkDMMLMarkupsNode to operate on.
  virtual void setDMMLMarkupsNode(vtkDMMLMarkupsNode* markupsNode) = 0;
  /// Sets the vtkDMMLNode to operate on.
  virtual void setDMMLScene(vtkDMMLScene* dmmlScene)
  {this->DMMLScene = dmmlScene;}

protected:
  vtkWeakPointer<vtkDMMLMarkupsNode> MarkupsNode;
  vtkWeakPointer<vtkDMMLScene> DMMLScene;

private:
  Q_DISABLE_COPY(qDMMLMarkupsAbstractOptionsWidget);
};

#endif // __qDMMLMarkupsAbstractOptionsWidget_h_
