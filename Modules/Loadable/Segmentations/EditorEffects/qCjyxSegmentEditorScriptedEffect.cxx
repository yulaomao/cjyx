/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// SubjectHierarchy includes
#include "qCjyxSegmentEditorScriptedEffect.h"

// Qt includes
#include <QDebug>
#include <QDir>
#include <QFileInfo>

// Cjyx includes
#include "qCjyxScriptedUtils_p.h"
#include "qCjyxUtils.h"

// PythonQt includes
#include <PythonQt.h>
#include <PythonQtConversion.h>

// DMML includes
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkPythonUtil.h>

//-----------------------------------------------------------------------------
class qCjyxSegmentEditorScriptedEffectPrivate
{
public:
  typedef qCjyxSegmentEditorScriptedEffectPrivate Self;
  qCjyxSegmentEditorScriptedEffectPrivate();
  virtual ~qCjyxSegmentEditorScriptedEffectPrivate();

  enum {
    IconMethod = 0,
    HelpTextMethod,
    CloneMethod,
    ActivateMethod,
    DeactivateMethod,
    SetupOptionsFrameMethod,
    CreateCursorMethod,
    ProcessInteractionEventsMethod,
    ProcessViewNodeEventsMethod,
    SetDMMLDefaultsMethod,
    ReferenceGeometryChangedMethod,
    MasterVolumeNodeChangedMethod,
    LayoutChangedMethod,
    InteractionNodeModifiedMethod,
    UpdateGUIFromDMMLMethod,
    UpdateDMMLFromGUIMethod,
    };

  mutable qCjyxPythonCppAPI PythonCppAPI;

  QString PythonSource;
};

//-----------------------------------------------------------------------------
// qCjyxSegmentEditorScriptedEffectPrivate methods

//-----------------------------------------------------------------------------
qCjyxSegmentEditorScriptedEffectPrivate::qCjyxSegmentEditorScriptedEffectPrivate()
{
  this->PythonCppAPI.declareMethod(Self::IconMethod, "icon");
  this->PythonCppAPI.declareMethod(Self::HelpTextMethod, "helpText");
  this->PythonCppAPI.declareMethod(Self::CloneMethod, "clone");
  this->PythonCppAPI.declareMethod(Self::ActivateMethod, "activate");
  this->PythonCppAPI.declareMethod(Self::DeactivateMethod, "deactivate");
  this->PythonCppAPI.declareMethod(Self::SetupOptionsFrameMethod, "setupOptionsFrame");
  this->PythonCppAPI.declareMethod(Self::CreateCursorMethod, "createCursor");
  this->PythonCppAPI.declareMethod(Self::ProcessInteractionEventsMethod, "processInteractionEvents");
  this->PythonCppAPI.declareMethod(Self::ProcessViewNodeEventsMethod, "processViewNodeEvents");
  this->PythonCppAPI.declareMethod(Self::SetDMMLDefaultsMethod, "setDMMLDefaults");
  this->PythonCppAPI.declareMethod(Self::ReferenceGeometryChangedMethod, "referenceGeometryChanged");
  this->PythonCppAPI.declareMethod(Self::MasterVolumeNodeChangedMethod, "masterVolumeNodeChanged");
  this->PythonCppAPI.declareMethod(Self::LayoutChangedMethod, "layoutChanged");
  this->PythonCppAPI.declareMethod(Self::InteractionNodeModifiedMethod, "interactionNodeModified");
  this->PythonCppAPI.declareMethod(Self::UpdateGUIFromDMMLMethod, "updateGUIFromDMML");
  this->PythonCppAPI.declareMethod(Self::UpdateDMMLFromGUIMethod, "updateDMMLFromGUI");
}

//-----------------------------------------------------------------------------
qCjyxSegmentEditorScriptedEffectPrivate::~qCjyxSegmentEditorScriptedEffectPrivate() = default;

//-----------------------------------------------------------------------------
// qCjyxSegmentEditorScriptedEffect methods

//-----------------------------------------------------------------------------
qCjyxSegmentEditorScriptedEffect::qCjyxSegmentEditorScriptedEffect(QObject *parent)
  : Superclass(parent)
  , d_ptr(new qCjyxSegmentEditorScriptedEffectPrivate)
{
  this->m_Name = QString("UnnamedScriptedEffect");
}

//-----------------------------------------------------------------------------
qCjyxSegmentEditorScriptedEffect::~qCjyxSegmentEditorScriptedEffect() = default;

//-----------------------------------------------------------------------------
QString qCjyxSegmentEditorScriptedEffect::pythonSource()const
{
  Q_D(const qCjyxSegmentEditorScriptedEffect);
  return d->PythonSource;
}

//-----------------------------------------------------------------------------
bool qCjyxSegmentEditorScriptedEffect::setPythonSource(const QString newPythonSource)
{
  Q_D(qCjyxSegmentEditorScriptedEffect);

  if (!Py_IsInitialized())
    {
    return false;
    }

  if (!newPythonSource.endsWith(".py") && !newPythonSource.endsWith(".pyc"))
    {
    return false;
    }

  // Use parent directory name as module name
  QDir sourceDir(newPythonSource);
  sourceDir.cdUp();
  QString moduleName = sourceDir.dirName();

  // Use filename as class name
  QString className = QFileInfo(newPythonSource).baseName();
  if (!className.endsWith("Effect"))
    {
    className.append("Effect");
    }

  // Get a reference to the main module and global dictionary
  PyObject * main_module = PyImport_AddModule("__main__");
  PyObject * global_dict = PyModule_GetDict(main_module);

  // Get a reference (or create if needed) the <moduleName> python module
  PyObject * module = PyImport_AddModule(moduleName.toUtf8());

  // Get a reference to the python module class to instantiate
  PythonQtObjectPtr classToInstantiate;
  if (PyObject_HasAttrString(module, className.toUtf8()))
    {
    classToInstantiate.setNewRef(PyObject_GetAttrString(module, className.toUtf8()));
    }
  if (!classToInstantiate)
    {
    PythonQtObjectPtr local_dict;
    local_dict.setNewRef(PyDict_New());
    if (!qCjyxScriptedUtils::loadSourceAsModule(moduleName, newPythonSource, global_dict, local_dict))
      {
      return false;
      }
    if (PyObject_HasAttrString(module, className.toUtf8()))
      {
      classToInstantiate.setNewRef(PyObject_GetAttrString(module, className.toUtf8()));
      }
    }

  if (!classToInstantiate)
    {
    PythonQt::self()->handleError();
    PyErr_SetString(PyExc_RuntimeError,
                    QString("qCjyxSegmentEditorScriptedEffect::setPythonSource - "
                            "Failed to load segment editor scripted effect: "
                            "class %1 was not found in %2").arg(className).arg(newPythonSource).toUtf8());
    PythonQt::self()->handleError();
    return false;
    }

  d->PythonCppAPI.setObjectName(className);

  PyObject* self = d->PythonCppAPI.instantiateClass(this, className, classToInstantiate);
  if (!self)
    {
    return false;
    }

  d->PythonSource = newPythonSource;

  if (!qCjyxScriptedUtils::setModuleAttribute(
        "cjyx", moduleName, self))
    {
    qCritical() << "Failed to set" << ("cjyx." + moduleName);
    }

  return true;
}

//-----------------------------------------------------------------------------
PyObject* qCjyxSegmentEditorScriptedEffect::self() const
{
  Q_D(const qCjyxSegmentEditorScriptedEffect);
  return d->PythonCppAPI.pythonSelf();
}

//-----------------------------------------------------------------------------
void qCjyxSegmentEditorScriptedEffect::setName(QString name)
{
  this->m_Name = name;
}

//-----------------------------------------------------------------------------
void qCjyxSegmentEditorScriptedEffect::setPerSegment(bool perSegment)
{
  this->m_PerSegment = perSegment;
}

//-----------------------------------------------------------------------------
void qCjyxSegmentEditorScriptedEffect::setRequireSegments(bool requireSegments)
{
  this->m_RequireSegments = requireSegments;
}

//-----------------------------------------------------------------------------
QIcon qCjyxSegmentEditorScriptedEffect::icon()
{
  Q_D(const qCjyxSegmentEditorScriptedEffect);
  PyObject* result = d->PythonCppAPI.callMethod(d->IconMethod);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::icon();
    }

  // Parse result
  QVariant resultVariant = PythonQtConv::PyObjToQVariant(result, QVariant::Icon);
  if (resultVariant.isNull())
    {
    return this->Superclass::icon();
    }
  return resultVariant.value<QIcon>();
}

//-----------------------------------------------------------------------------
const QString qCjyxSegmentEditorScriptedEffect::helpText()const
{
  Q_D(const qCjyxSegmentEditorScriptedEffect);
  PyObject* result = d->PythonCppAPI.callMethod(d->HelpTextMethod);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::helpText();
    }

  // Parse result
  if (!PyString_Check(result))
    {
    qWarning() << d->PythonSource << ": qCjyxSegmentEditorScriptedEffect: Function 'helpText' is expected to return a string!";
    return this->Superclass::helpText();
    }

  const char* role = PyString_AsString(result);
  return QString::fromLocal8Bit(role);
}

//-----------------------------------------------------------------------------
qCjyxSegmentEditorAbstractEffect* qCjyxSegmentEditorScriptedEffect::clone()
{
  Q_D(const qCjyxSegmentEditorScriptedEffect);
  PyObject* result = d->PythonCppAPI.callMethod(d->CloneMethod);
  if (!result)
    {
    qCritical() << d->PythonSource << ": clone: Failed to call mandatory clone method! If it is implemented, please see python output for errors.";
    return nullptr;
    }

  // Parse result
  QVariant resultVariant = PythonQtConv::PyObjToQVariant(result);
  qCjyxSegmentEditorAbstractEffect* clonedEffect = qobject_cast<qCjyxSegmentEditorAbstractEffect*>(
    resultVariant.value<QObject*>() );
  if (!clonedEffect)
    {
    qCritical() << d->PythonSource << ": clone: Invalid cloned effect object returned from python!";
    return nullptr;
    }
  return clonedEffect;
}

//-----------------------------------------------------------------------------
void qCjyxSegmentEditorScriptedEffect::activate()
{
  // Base class implementation needs to be called before the effect-specific one
  this->Superclass::activate();

  Q_D(const qCjyxSegmentEditorScriptedEffect);
  d->PythonCppAPI.callMethod(d->ActivateMethod);
}

//-----------------------------------------------------------------------------
void qCjyxSegmentEditorScriptedEffect::deactivate()
{
  // Base class implementation needs to be called before the effect-specific one
  this->Superclass::deactivate();

  Q_D(const qCjyxSegmentEditorScriptedEffect);
  d->PythonCppAPI.callMethod(d->DeactivateMethod);
}

//-----------------------------------------------------------------------------
void qCjyxSegmentEditorScriptedEffect::setupOptionsFrame()
{
  // Base class implementation needs to be called before the effect-specific one
  this->Superclass::setupOptionsFrame();

  Q_D(const qCjyxSegmentEditorScriptedEffect);
  d->PythonCppAPI.callMethod(d->SetupOptionsFrameMethod);
}

//-----------------------------------------------------------------------------
QCursor qCjyxSegmentEditorScriptedEffect::createCursor(qDMMLWidget* viewWidget)
{
  Q_D(const qCjyxSegmentEditorScriptedEffect);
  PyObject* arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, PythonQtConv::QVariantToPyObject(QVariant::fromValue<QObject*>((QObject*)viewWidget)));
  PyObject* result = d->PythonCppAPI.callMethod(d->CreateCursorMethod, arguments);
  Py_DECREF(arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::createCursor(viewWidget);
    }

  // Parse result
  QVariant resultVariant = PythonQtConv::PyObjToQVariant(result, QVariant::Cursor);
  return resultVariant.value<QCursor>();
}

//-----------------------------------------------------------------------------
bool qCjyxSegmentEditorScriptedEffect::processInteractionEvents(vtkRenderWindowInteractor* callerInteractor, unsigned long eid, qDMMLWidget* viewWidget)
{
  Q_D(const qCjyxSegmentEditorScriptedEffect);
  PyObject* arguments = PyTuple_New(3);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer((vtkObject*)callerInteractor));
  PyTuple_SET_ITEM(arguments, 1, PyInt_FromLong(eid));
  PyTuple_SET_ITEM(arguments, 2, PythonQtConv::QVariantToPyObject(QVariant::fromValue<QObject*>((QObject*)viewWidget)));
  PyObject* result = d->PythonCppAPI.callMethod(d->ProcessInteractionEventsMethod, arguments);
  Py_DECREF(arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::processInteractionEvents(callerInteractor, eid, viewWidget);
    }
  if (!PyBool_Check(result))
    {
    qWarning() << d->PythonSource
               << " - function 'processInteractionEvents' "
               << "is expected to return a boolean";
    return false;
    }
  return result == Py_True;
}

//-----------------------------------------------------------------------------
void qCjyxSegmentEditorScriptedEffect::processViewNodeEvents(vtkDMMLAbstractViewNode* callerViewNode, unsigned long eid, qDMMLWidget* viewWidget)
{
  Q_D(const qCjyxSegmentEditorScriptedEffect);
  PyObject* arguments = PyTuple_New(3);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer((vtkObject*)callerViewNode));
  PyTuple_SET_ITEM(arguments, 1, PyInt_FromLong(eid));
  PyTuple_SET_ITEM(arguments, 2, PythonQtConv::QVariantToPyObject(QVariant::fromValue<QObject*>((QObject*)viewWidget)));
  PyObject* result = d->PythonCppAPI.callMethod(d->ProcessViewNodeEventsMethod, arguments);
  Py_DECREF(arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::processViewNodeEvents(callerViewNode, eid, viewWidget);
    }
}

//-----------------------------------------------------------------------------
void qCjyxSegmentEditorScriptedEffect::setDMMLDefaults()
{
  // Base class implementation needs to be called before the effect-specific one
  // Note: Left here as comment in case this class is used as template for adaptor
  //  classes of effect base classes that have default implementation of this method
  //  (such as LabelEffect, etc.)
  //this->Superclass::setDMMLDefaults();

  Q_D(const qCjyxSegmentEditorScriptedEffect);
  d->PythonCppAPI.callMethod(d->SetDMMLDefaultsMethod);
}

//-----------------------------------------------------------------------------
void qCjyxSegmentEditorScriptedEffect::referenceGeometryChanged()
{
  // Base class implementation needs to be called before the effect-specific one
  this->Superclass::referenceGeometryChanged();

  Q_D(const qCjyxSegmentEditorScriptedEffect);
  d->PythonCppAPI.callMethod(d->ReferenceGeometryChangedMethod);
}

//-----------------------------------------------------------------------------
void qCjyxSegmentEditorScriptedEffect::masterVolumeNodeChanged()
{
  // Base class implementation needs to be called before the effect-specific one
  this->Superclass::masterVolumeNodeChanged();

  Q_D(const qCjyxSegmentEditorScriptedEffect);
  d->PythonCppAPI.callMethod(d->MasterVolumeNodeChangedMethod);
}

//-----------------------------------------------------------------------------
void qCjyxSegmentEditorScriptedEffect::layoutChanged()
{
  // Base class implementation needs to be called before the effect-specific one
  this->Superclass::layoutChanged();

  Q_D(const qCjyxSegmentEditorScriptedEffect);
  d->PythonCppAPI.callMethod(d->LayoutChangedMethod);
}

//-----------------------------------------------------------------------------
void qCjyxSegmentEditorScriptedEffect::interactionNodeModified(vtkDMMLInteractionNode* interactionNode)
{
  // Do not call base class implementation by default.
  // This way the effect may decide to not deactivate itself when markups place mode
  // is activated.

  Q_D(const qCjyxSegmentEditorScriptedEffect);
  PyObject* arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer((vtkObject*)interactionNode));
  PyObject* result = d->PythonCppAPI.callMethod(d->InteractionNodeModifiedMethod, arguments);
  Py_DECREF(arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    this->Superclass::interactionNodeModified(interactionNode);
    }
}

//-----------------------------------------------------------------------------
void qCjyxSegmentEditorScriptedEffect::updateGUIFromDMML()
{
  if (!this->active())
  {
    // updateGUIFromDMML is called when the effect is activated
    return;
  }

  // Base class implementation needs to be called before the effect-specific one
  // Note: Left here as comment in case this class is used as template for adaptor
  //  classes of effect base classes that have default implementation of this method
  //  (such as LabelEffect, etc.)
  //this->Superclass::updateGUIFromDMML();

  Q_D(const qCjyxSegmentEditorScriptedEffect);
  d->PythonCppAPI.callMethod(d->UpdateGUIFromDMMLMethod);
}

//-----------------------------------------------------------------------------
void qCjyxSegmentEditorScriptedEffect::updateDMMLFromGUI()
{
  // Base class implementation needs to be called before the effect-specific one
  // Note: Left here as comment in case this class is used as template for adaptor
  //  classes of effect base classes that have default implementation of this method
  //  (such as LabelEffect, etc.)
  //this->Superclass::updateDMMLFromGUI();

  Q_D(const qCjyxSegmentEditorScriptedEffect);
  d->PythonCppAPI.callMethod(d->UpdateDMMLFromGUIMethod);
}
