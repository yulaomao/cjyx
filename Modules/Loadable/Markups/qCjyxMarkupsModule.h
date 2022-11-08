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

==============================================================================*/

#ifndef __qCjyxMarkupsModule_h
#define __qCjyxMarkupsModule_h

// CTK includes
#include <ctkVTKObject.h>

// Cjyx includes
#include "qCjyxLoadableModule.h"

#include "qCjyxMarkupsModuleExport.h"

#include "vtkCjyxConfigure.h" // For Cjyx_HAVE_QT5

class qDMMLMarkupsToolBar;
class vtkDMMLScene;
class vtkDMMLMarkupsNode;
class qCjyxMarkupsModulePrivate;
class vtkDMMLMarkupsDisplayNode;
class vtkObject;

/// \ingroup Cjyx_QtModules_Markups
class Q_CJYX_QTMODULES_MARKUPS_EXPORT qCjyxMarkupsModule :
  public qCjyxLoadableModule
{
  Q_OBJECT
  QVTK_OBJECT;
  Q_PLUGIN_METADATA(IID "org.cjyx.modules.loadable.qCjyxLoadableModule/1.0");
  Q_INTERFACES(qCjyxLoadableModule);
  /// Visibility of the markups toolbar
  Q_PROPERTY(bool toolBarVisible READ isToolBarVisible WRITE setToolBarVisible)
  Q_PROPERTY(bool autoShowToolBar READ autoShowToolBar WRITE setAutoShowToolBar)

public:

  typedef qCjyxLoadableModule Superclass;
  explicit qCjyxMarkupsModule(QObject *parent=nullptr);
  ~qCjyxMarkupsModule() override;

  qCjyxGetTitleMacro(QTMODULE_TITLE);

  /// Help to use the module
  QString helpText()const override;

  /// Return acknowledgements
  QString acknowledgementText()const override;

  /// Return the authors of the module
  QStringList  contributors()const override;

  /// Return a custom icon for the module
  QIcon icon()const override;

  /// Return the categories for the module
  QStringList categories()const override;

  /// Specify editable node types
  QStringList associatedNodeTypes()const override;

  void setDMMLScene(vtkDMMLScene* scene) override;

  static void readDefaultMarkupsDisplaySettings(vtkDMMLMarkupsDisplayNode* markupsDisplayNode);
  static void writeDefaultMarkupsDisplaySettings(vtkDMMLMarkupsDisplayNode* markupsDisplayNode);

  /// Indicates that markups toolbar should be showed when a new sequence is loaded.
  /// Adding a new markups node to the scene does not show the toolbar automatically
  /// but the importer must call showMarkups method.
  Q_INVOKABLE bool autoShowToolBar();
  Q_INVOKABLE bool isToolBarVisible();
  Q_INVOKABLE qDMMLMarkupsToolBar* toolBar();

  /// Utility function for showing the markupsNode in the application user interface (toolbar)
  /// if autoShowToolBar is enabled.
  Q_INVOKABLE static bool showMarkups(vtkDMMLMarkupsNode* markupsNode);

protected:

  /// Initialize the module. Register the volumes reader/writer
  void setup() override;

  /// Create and return the widget representation associated to this module
  qCjyxAbstractModuleRepresentation * createWidgetRepresentation() override;

  /// Create and return the logic associated to this module
  vtkDMMLAbstractLogic* createLogic() override;

public slots:
  void setToolBarVisible(bool visible);
  /// Enables automatic showing markups toolbar when a new markups node is loaded
  void setAutoShowToolBar(bool autoShow);
  //void onNodeAddedEvent(vtkObject*, vtkObject*);

protected:
  QScopedPointer<qCjyxMarkupsModulePrivate> d_ptr;


private:
  Q_DECLARE_PRIVATE(qCjyxMarkupsModule);
  Q_DISABLE_COPY(qCjyxMarkupsModule);

};

#endif
