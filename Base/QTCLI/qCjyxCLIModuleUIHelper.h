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

#ifndef __qCjyxCLIModuleUIHelper_h
#define __qCjyxCLIModuleUIHelper_h

// Qt includes
#include <QButtonGroup>
#include <QObject>
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>

/// ModuleDescriptionParser includes
#include <ModuleDescription.h>

#include "qCjyxBaseQTCLIExport.h"

class QWidget;
class qCjyxCLIModuleWidget;
class vtkDMMLCommandLineModuleNode;
class qCjyxCLIModuleUIHelperPrivate;

//-----------------------------------------------------------------------------
class Q_CJYX_BASE_QTCLI_EXPORT qCjyxWidgetValueWrapper: public QObject
{
  Q_OBJECT
public:
  qCjyxWidgetValueWrapper(const QString& _name, const QString& _label, QObject* parent);
  ~qCjyxWidgetValueWrapper() override;
  virtual QVariant value() = 0;
  QString label(){ return this->Label; }
  QString name(){ return this->Name; }

  virtual void setValue(const QString& _value) = 0;

  static QString toString(const QString& _value)
    {
    return _value;
    }

  static bool toBool(const QString& _value)
    {
    return (_value.compare("true", Qt::CaseInsensitive) == 0);
    }

  static int toInt(const QString& _value)
    {
    return _value.toInt();
    }

  static double toDouble(const QString& _value)
    {
    return _value.toDouble();
    }

signals:
  void valueChanged();

protected:
  QString Name;
  QString Label;
};

//-----------------------------------------------------------------------------
class Q_CJYX_BASE_QTCLI_EXPORT ButtonGroupWidgetWrapper: public QWidget
{
  Q_OBJECT
public:
  ButtonGroupWidgetWrapper(QWidget* parentWidget = nullptr);

  QButtonGroup* buttonGroup()const;
  QString checkedValue();

  void setCheckedValue(const QString& value);

signals:
  void valueChanged();

private:
  QButtonGroup* ButtonGroup;
};

//-----------------------------------------------------------------------------
class Q_CJYX_BASE_QTCLI_EXPORT qCjyxCLIModuleUIHelper: public QObject
{
  Q_OBJECT
public:

  qCjyxCLIModuleUIHelper(qCjyxCLIModuleWidget* cliModuleWidget);
  ~qCjyxCLIModuleUIHelper() override;

  /// Create the widget associated with the given \a moduleParameter
  /// The caller is responsible to delete the widget.
  /// Note also that if the widget is added to a layout, Qt will
  /// be responsible to delete the widget.
  QWidget* createTagWidget(const ModuleParameter& moduleParameter);

  ///
  /// Update \a commandLineModuleNode properties using value entered from the UI
  void updateDMMLCommandLineModuleNode(vtkDMMLCommandLineModuleNode* commandLineModuleNode);

  /// Update user interface using the given \a commandLineModuleNode parameters
  void updateUi(vtkDMMLCommandLineModuleNode* commandLineModuleNode);

  /// Set parameter to the command line module node
  void setCommandLineModuleParameter(vtkDMMLCommandLineModuleNode* node,
                                     const QString& name,
                                     const QVariant& value);
  void setValue(const QString& name, const QVariant& type);

signals:
  void valueChanged(const QString& tag, const QVariant& value);

protected slots:
  void onValueChanged();

protected:
  QScopedPointer<qCjyxCLIModuleUIHelperPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qCjyxCLIModuleUIHelper);
  Q_DISABLE_COPY(qCjyxCLIModuleUIHelper);
};

#endif
