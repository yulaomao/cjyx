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

#ifndef __qCjyxScriptedLoadableModuleWidget_h
#define __qCjyxScriptedLoadableModuleWidget_h

// Cjyx includes
#include "qCjyxAbstractModuleWidget.h"

#include "qCjyxBaseQTGUIExport.h"

// Forward Declare PyObject*
#ifndef PyObject_HEAD
struct _object;
typedef _object PyObject;
#endif
class qCjyxScriptedLoadableModuleWidgetPrivate;

class Q_CJYX_BASE_QTGUI_EXPORT qCjyxScriptedLoadableModuleWidget
  :public qCjyxAbstractModuleWidget
{
  Q_OBJECT
  Q_PROPERTY(QString moduleName READ moduleName)
public:
  typedef qCjyxAbstractModuleWidget Superclass;
  typedef qCjyxScriptedLoadableModuleWidgetPrivate Pimpl;
  qCjyxScriptedLoadableModuleWidget(QWidget * parentWidget=nullptr);
  ~qCjyxScriptedLoadableModuleWidget() override;

  QString pythonSource()const;
  bool setPythonSource(const QString& newPythonSource, const QString& className = QLatin1String(""));

  /// Convenience method allowing to retrieve the associated scripted instance
  Q_INVOKABLE PyObject* self() const;

  void enter() override;
  void exit() override;

  bool setEditedNode(vtkDMMLNode* node, QString role = QString(), QString context = QString()) override;
  double nodeEditable(vtkDMMLNode* node) override;

protected:
  void setup() override;

protected:
  QScopedPointer<qCjyxScriptedLoadableModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qCjyxScriptedLoadableModuleWidget);
  Q_DISABLE_COPY(qCjyxScriptedLoadableModuleWidget);
};

#endif
