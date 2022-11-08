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
  Oslo University Hospital), based on qCjyxSegmentEditorEffectFactory.cxx by
  Csaba Pinter (Perklab, Queen's University), and was supported by The Research
  Council of Norway through the ALive project (grant nr. 311393).

  ==============================================================================*/

#include "qDMMLMarkupsOptionsWidgetsFactory.h"

// Qt includes
#include <QDebug>
#include <QString>

//----------------------------------------------------------------------------
qDMMLMarkupsOptionsWidgetsFactory *qDMMLMarkupsOptionsWidgetsFactory::Instance = nullptr;

//----------------------------------------------------------------------------
/// \ingroup CjyxRt_QtModules_Segmentations
class qDMMLMarkupsOptionsWidgetsFactoryCleanup
{
public:
  inline void use()   {   }

  ~qDMMLMarkupsOptionsWidgetsFactoryCleanup()
  {
    if (qDMMLMarkupsOptionsWidgetsFactory::Instance)
      {
      qDMMLMarkupsOptionsWidgetsFactory::cleanup();
      }
  }

  void cleanup()
  {
    qDMMLMarkupsOptionsWidgetsFactory::cleanup();
  }
};

//-----------------------------------------------------------------------------
static qDMMLMarkupsOptionsWidgetsFactoryCleanup qDMMLMarkupsOptionsWidgetsFactoryCleanupGlobal;

//-----------------------------------------------------------------------------
qDMMLMarkupsOptionsWidgetsFactory* qDMMLMarkupsOptionsWidgetsFactory::instance()
{
  if(!qDMMLMarkupsOptionsWidgetsFactory::Instance)
    {
    qDMMLMarkupsOptionsWidgetsFactoryCleanupGlobal.use();
    qDMMLMarkupsOptionsWidgetsFactory::Instance = new qDMMLMarkupsOptionsWidgetsFactory();
    }
  // Return the instance
  return qDMMLMarkupsOptionsWidgetsFactory::Instance;
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsOptionsWidgetsFactory::cleanup()
{
  if (qDMMLMarkupsOptionsWidgetsFactory::Instance)
    {
    delete qDMMLMarkupsOptionsWidgetsFactory::Instance;
    qDMMLMarkupsOptionsWidgetsFactory::Instance = nullptr;
    }
}

//-----------------------------------------------------------------------------
qDMMLMarkupsOptionsWidgetsFactory::qDMMLMarkupsOptionsWidgetsFactory(QObject* parent)
  : QObject(parent)
{
  this->RegisteredWidgets.clear();
}

//-----------------------------------------------------------------------------
qDMMLMarkupsOptionsWidgetsFactory::~qDMMLMarkupsOptionsWidgetsFactory()
{
  this->unregisterAll();
}

//-----------------------------------------------------------------------------
bool qDMMLMarkupsOptionsWidgetsFactory::registerOptionsWidget(qDMMLMarkupsAbstractOptionsWidget* widget)
{
  // Check for nullptr
  if (!widget)
    {
    qCritical() << Q_FUNC_INFO << ":  Invalid widget.";
    return false;
    }

  // Check for empty type name
  if (widget->className().isEmpty())
    {
    qCritical() << Q_FUNC_INFO << ":  options widget has no class name.";
    return false;
    }

  // Check for already registered widget
  if (this->RegisteredWidgets.contains(widget->className()))
    {
    qWarning() << Q_FUNC_INFO << ":  options widget type "
               << widget->className()
               << " already registered.";
    delete widget;
    return false;
    }

  // Register the additional options widget
  this->RegisteredWidgets[widget->className()] = widget;

  // Emit registered signal
  emit optionsWidgetRegistered();

  return true;
}

//-----------------------------------------------------------------------------
bool qDMMLMarkupsOptionsWidgetsFactory::unregisterOptionsWidget(qDMMLMarkupsAbstractOptionsWidget* widget)
{
  // Check for empty type name
  if (!widget)
    {
    qCritical() << Q_FUNC_INFO << ":  Invalid widget.";
    return false;
    }

  auto className = widget->className();

  delete widget;

  if(this->unregisterOptionsWidget(className))
    {
    // Emit registered signal
    emit optionsWidgetUnregistered();
    return true;
    }

  return false;
}

//-----------------------------------------------------------------------------
bool qDMMLMarkupsOptionsWidgetsFactory::unregisterOptionsWidget(const QString& className)
{
  // Check for empty type name
  if (className.isEmpty())
    {
    qCritical() << Q_FUNC_INFO << ":  options widget has no tpye name.";
    return false;
    }

  // Check for already registered widget
  if (!this->RegisteredWidgets.remove(className))
    {
    qWarning() << Q_FUNC_INFO << ":  options widget type "
               << className << " not registered.";
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
qDMMLMarkupsAbstractOptionsWidget* qDMMLMarkupsOptionsWidgetsFactory::createWidget(const QString& className)
{
  // Check for empty type name
  if (className.isEmpty())
    {
    qCritical() << Q_FUNC_INFO << ":  options widget has no class name.";
    return nullptr;
    }

  // Check for already registered widget
  if (!this->RegisteredWidgets.contains(className))
    {
    qWarning() << Q_FUNC_INFO << ":  options widget type "
               << className
               << " not registered.";
    return nullptr;
    }

  return this->RegisteredWidgets[className]->createInstance();

}

//-----------------------------------------------------------------------------
void qDMMLMarkupsOptionsWidgetsFactory::unregisterAll()
{
  foreach(auto widget, this->RegisteredWidgets)
    {
    if (widget)
      {
      delete widget.data();
      }
    }

  this->RegisteredWidgets.clear();
}
