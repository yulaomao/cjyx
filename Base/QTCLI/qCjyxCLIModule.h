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

#ifndef __qCjyxCLIModule_h
#define __qCjyxCLIModule_h

// CTK includes
#include <ctkPimpl.h>

// Cjyx includes
#include "qCjyxAbstractModule.h"

// CjyxExecutionModel includes
#include <ModuleDescription.h>

#include "qCjyxBaseQTCLIExport.h"

class ModuleLogo;
class vtkDMMLCommandLineModuleNode;
class vtkCjyxCLIModuleLogic;

class qCjyxCLIModulePrivate;
class Q_CJYX_BASE_QTCLI_EXPORT qCjyxCLIModule : public qCjyxAbstractModule
{
  Q_OBJECT
public:

  typedef qCjyxAbstractModule Superclass;
  qCjyxCLIModule(QWidget *parent=nullptr);
  ~qCjyxCLIModule() override;

  ///
  /// Assign the module XML description.
  /// Note: That will also trigger the parsing of the XML structure
  void setXmlModuleDescription(const QString& xmlModuleDescription);

  /// Optionally set in the module XML description
  int index() const override;

  ///
  /// Return help/acknowledgement text
  QString helpText() const override;
  QString acknowledgementText() const override;

  /// Set temporary directory associated with the module
  void setTempDirectory(const QString& tempDirectory);
  QString tempDirectory() const;

  /// Set module entry point. Typically "cjyx:0x012345" for loadable CLI
  /// or "/home/user/work/Cjyx-Superbuild/../mycliexec" for executable CLI
  void setEntryPoint(const QString& entryPoint);
  QString entryPoint() const;

  /// SharedObjectModule for loadable modules or CommandLineModule for
  /// executable modules.
  void setModuleType(const QString& type);
  QString moduleType() const;

  /// This method allows to get a pointer to the ModuleLogic.
  /// If no moduleLogic already exists, one will be created calling
  /// 'createLogic' method.
  Q_INVOKABLE vtkCjyxCLIModuleLogic* cliModuleLogic();

  QString title() const override;

  /// Extracted from the "category" field
  QStringList categories() const override;

  /// Extracted from the "contributor" field
  QStringList contributors() const override;

  /// Specify editable node types
  QStringList associatedNodeTypes()const override;

  QImage logo() const override;
  void setLogo(const ModuleLogo& logo);

  /// Convert a ModuleLogo into a QIcon
  /// \todo: Find a better place for this util function
  static QImage moduleLogoToImage(const ModuleLogo& logo);

  /// Return the module description object used to store
  /// the module properties.
  ModuleDescription& moduleDescription();

protected:
  ///
  void setup() override;

  ///
  /// Create and return the widget representation associated to this module
  qCjyxAbstractModuleRepresentation * createWidgetRepresentation() override;

  ///
  /// Create and return the logic associated to this module
  vtkDMMLAbstractLogic* createLogic() override;

protected:
  QScopedPointer<qCjyxCLIModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qCjyxCLIModule);
  Q_DISABLE_COPY(qCjyxCLIModule);
};

#endif
