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

#ifndef __qCjyxVolumesModule_h
#define __qCjyxVolumesModule_h

// Cjyx includes
#include "qCjyxLoadableModule.h"

#include "qCjyxVolumesModuleExport.h"

class qCjyxAbstractModuleWidget;
class qCjyxVolumesModulePrivate;

/// \ingroup Cjyx_QtModules_Volumes
class Q_CJYX_QTMODULES_VOLUMES_EXPORT qCjyxVolumesModule :
  public qCjyxLoadableModule
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.cjyx.modules.loadable.qCjyxLoadableModule/1.0");
  Q_INTERFACES(qCjyxLoadableModule);

public:

  typedef qCjyxLoadableModule Superclass;
  qCjyxVolumesModule(QObject *parent=nullptr);
  ~qCjyxVolumesModule() override;

  QString helpText()const override;
  QString acknowledgementText()const override;
  QStringList contributors()const override;
  QIcon icon()const override;
  QStringList categories()const override;
  QStringList dependencies()const override;
  qCjyxGetTitleMacro(QTMODULE_TITLE);

protected:
  /// Initialize the module. Register the volumes reader/writer
  void setup() override;

  /// Create and return the widget representation associated to this module
  qCjyxAbstractModuleRepresentation* createWidgetRepresentation() override;

  /// Create and return the logic associated to this module
  vtkDMMLAbstractLogic* createLogic() override;

  /// Specify editable node types
  QStringList associatedNodeTypes()const override;

protected:
  QScopedPointer<qCjyxVolumesModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qCjyxVolumesModule);
  Q_DISABLE_COPY(qCjyxVolumesModule);
};

#endif
