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

// SubjectHierarchy plugins includes
#include "qCjyxSubjectHierarchyPluginHandler.h"
#include "qCjyxSubjectHierarchyPluginLogic.h"
#include "qCjyxSubjectHierarchyAbstractPlugin.h"
#include "qCjyxSubjectHierarchyDefaultPlugin.h"

// Subject hierarchy logic includes
#include "vtkCjyxSubjectHierarchyModuleLogic.h"

// Qt includes
#include <QAction>
#include <QDebug>
#include <QStringList>
#include <QInputDialog>
#include <QSettings>

// Cjyx includes
#include "qCjyxApplication.h"

// VTK includes
#include <vtkCallbackCommand.h>

//----------------------------------------------------------------------------
qCjyxSubjectHierarchyPluginHandler *qCjyxSubjectHierarchyPluginHandler::m_Instance = nullptr;

//----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_SubjectHierarchy_Widgets
class qCjyxSubjectHierarchyPluginHandlerCleanup
{
public:
  inline void use()   {   }

  ~qCjyxSubjectHierarchyPluginHandlerCleanup()
    {
    if (qCjyxSubjectHierarchyPluginHandler::m_Instance)
      {
      qCjyxSubjectHierarchyPluginHandler::cleanup();
      }
    }
};
static qCjyxSubjectHierarchyPluginHandlerCleanup qCjyxSubjectHierarchyPluginHandlerCleanupGlobal;

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyPluginHandler* qCjyxSubjectHierarchyPluginHandler::instance()
{
  if(!qCjyxSubjectHierarchyPluginHandler::m_Instance)
    {
    qCjyxSubjectHierarchyPluginHandlerCleanupGlobal.use();
    qCjyxSubjectHierarchyPluginHandler::m_Instance = new qCjyxSubjectHierarchyPluginHandler();
    }
  // Return the instance
  return qCjyxSubjectHierarchyPluginHandler::m_Instance;
}

//-----------------------------------------------------------------------------
void qCjyxSubjectHierarchyPluginHandler::cleanup()
{
  if (qCjyxSubjectHierarchyPluginHandler::m_Instance)
    {
    delete qCjyxSubjectHierarchyPluginHandler::m_Instance;
    qCjyxSubjectHierarchyPluginHandler::m_Instance = nullptr;
    }
}

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyPluginHandler::qCjyxSubjectHierarchyPluginHandler(QObject* parent)
  : QObject(parent)
  , m_DMMLScene(nullptr)
{
  this->m_CurrentItems.clear();

  this->m_RegisteredPlugins.clear();
  this->m_DefaultPlugin = new qCjyxSubjectHierarchyDefaultPlugin();

  this->m_CallBack = vtkSmartPointer<vtkCallbackCommand>::New();
  this->m_CallBack->SetClientData(this);
  this->m_CallBack->SetCallback(qCjyxSubjectHierarchyPluginHandler::onSubjectHierarchyNodeEvent);
}

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyPluginHandler::~qCjyxSubjectHierarchyPluginHandler()
{
  if (m_DMMLScene != nullptr)
    {
    m_DMMLScene->RemoveObserver(m_CallBack);
    }

  QList<qCjyxSubjectHierarchyAbstractPlugin*>::iterator pluginIt;
  for (pluginIt = this->m_RegisteredPlugins.begin(); pluginIt != this->m_RegisteredPlugins.end(); ++pluginIt)
    {
    delete (*pluginIt);
    }
  this->m_RegisteredPlugins.clear();

  delete this->m_DefaultPlugin;
}

//---------------------------------------------------------------------------
bool qCjyxSubjectHierarchyPluginHandler::registerPlugin(qCjyxSubjectHierarchyAbstractPlugin* pluginToRegister)
{
  if (pluginToRegister == nullptr)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid plugin to register";
    return false;
    }
  if (pluginToRegister->name().isEmpty())
    {
    qCritical() << Q_FUNC_INFO << ": SubjectHierarchy plugin cannot be registered with empty name";
    return false;
    }

  // Check if the same plugin has already been registered
  qCjyxSubjectHierarchyAbstractPlugin* currentPlugin = nullptr;
  foreach (currentPlugin, this->m_RegisteredPlugins)
    {
    if (pluginToRegister->name().compare(currentPlugin->name()) == 0)
      {
      qDebug() << Q_FUNC_INFO << ": SubjectHierarchy plugin " << pluginToRegister->name() << " is already registered";
      return false;
      }
    }

  // Add view menu actions from plugin to plugin logic
  if (this->m_PluginLogic)
    {
    foreach(QAction* action, pluginToRegister->viewContextMenuActions())
      {
      if (action != nullptr && action->objectName().isEmpty())
        {
        qWarning() << Q_FUNC_INFO << ": view context menu action name is not set for "
          << action->text() << ", provided subject hierarchy by plugin " << pluginToRegister->name();
        }
      this->m_PluginLogic->registerViewContextMenuAction(action);
      }
    }

  // Add the plugin to the list
  this->m_RegisteredPlugins << pluginToRegister;

  // Update timestamp
  this->LastPluginRegistrationTime = QDateTime::currentDateTimeUtc();

  return true;
}

//---------------------------------------------------------------------------
qCjyxSubjectHierarchyDefaultPlugin* qCjyxSubjectHierarchyPluginHandler::defaultPlugin()
{
  return this->m_DefaultPlugin;
}

//---------------------------------------------------------------------------
QList<qCjyxSubjectHierarchyAbstractPlugin*> qCjyxSubjectHierarchyPluginHandler::allPlugins()
{
  QList<qCjyxSubjectHierarchyAbstractPlugin*> allPlugins = this->m_RegisteredPlugins;
  allPlugins << this->m_DefaultPlugin;
  return allPlugins;
}

//---------------------------------------------------------------------------
qCjyxSubjectHierarchyAbstractPlugin* qCjyxSubjectHierarchyPluginHandler::pluginByName(QString name)
{
  // Return default plugin if requested
  if (name.compare("Default") == 0)
    {
    return this->m_DefaultPlugin;
    }
  else if (name.isEmpty())
    {
    return nullptr;
    }

  // Find plugin with name
  qCjyxSubjectHierarchyAbstractPlugin* currentPlugin = nullptr;
  foreach (currentPlugin, this->m_RegisteredPlugins)
    {
    if (currentPlugin->name().compare(name) == 0)
      {
      return currentPlugin;
      }
    }

  qWarning() << Q_FUNC_INFO << ": Plugin named '" << name << "' cannot be found";
  return nullptr;
}

//---------------------------------------------------------------------------
QList<qCjyxSubjectHierarchyAbstractPlugin*> qCjyxSubjectHierarchyPluginHandler::pluginsForAddingNodeToSubjectHierarchy(
  vtkDMMLNode* node, vtkIdType parentItemID/*=vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID*/ )
{
  QList<qCjyxSubjectHierarchyAbstractPlugin*> mostSuitablePlugins;
  double bestConfidence = 0.0;
  qCjyxSubjectHierarchyAbstractPlugin* currentPlugin = nullptr;
  foreach (currentPlugin, this->m_RegisteredPlugins)
    {
    double currentConfidence = currentPlugin->canAddNodeToSubjectHierarchy(node, parentItemID);
    if (currentConfidence > bestConfidence)
      {
      bestConfidence = currentConfidence;

      // Set only the current plugin as most suitable plugin
      mostSuitablePlugins.clear();
      mostSuitablePlugins << currentPlugin;
      }
    else if (currentConfidence > 0.0 && currentConfidence == bestConfidence)
      {
      // Add current plugin to most suitable plugins
      mostSuitablePlugins << currentPlugin;
      }
    }

  return mostSuitablePlugins;
}

//---------------------------------------------------------------------------
QList<qCjyxSubjectHierarchyAbstractPlugin*>
qCjyxSubjectHierarchyPluginHandler::pluginsForReparentingItemInSubjectHierarchy(vtkIdType itemID, vtkIdType parentItemID)
{
  QList<qCjyxSubjectHierarchyAbstractPlugin*> mostSuitablePlugins;
  double bestConfidence = 0.0;
  qCjyxSubjectHierarchyAbstractPlugin* currentPlugin = nullptr;
  foreach (currentPlugin, this->m_RegisteredPlugins)
    {
    double currentConfidence = currentPlugin->canReparentItemInsideSubjectHierarchy(itemID, parentItemID);
    if (currentConfidence > bestConfidence)
      {
      bestConfidence = currentConfidence;

      // Set only the current plugin as most suitable plugin
      mostSuitablePlugins.clear();
      mostSuitablePlugins << currentPlugin;
      }
    else if (currentConfidence > 0.0 && currentConfidence == bestConfidence)
      {
      // Add current plugin to most suitable plugins
      mostSuitablePlugins << currentPlugin;
      }
    }

  return mostSuitablePlugins;
}

//---------------------------------------------------------------------------
qCjyxSubjectHierarchyAbstractPlugin*
qCjyxSubjectHierarchyPluginHandler::findOwnerPluginForSubjectHierarchyItem(vtkIdType itemID)
{
  if (this->m_DMMLScene != nullptr && this->m_DMMLScene->GetSubjectHierarchyNode() == nullptr)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy node";
    return nullptr;
    }

  QList<qCjyxSubjectHierarchyAbstractPlugin*> mostSuitablePlugins;
  double bestConfidence = 0.0;
  qCjyxSubjectHierarchyAbstractPlugin* currentPlugin = nullptr;
  foreach (currentPlugin, this->m_RegisteredPlugins)
    {
    double currentConfidence = currentPlugin->canOwnSubjectHierarchyItem(itemID);
    if (currentConfidence > bestConfidence)
      {
      bestConfidence = currentConfidence;

      // Set only the current plugin as most suitable plugin
      mostSuitablePlugins.clear();
      mostSuitablePlugins << currentPlugin;
      }
    else if (currentConfidence > 0.0 && currentConfidence == bestConfidence)
      {
      // Add current plugin to most suitable plugins
      mostSuitablePlugins << currentPlugin;
      }
    }

  // Determine owner plugin based on plugins returning the highest non-zero confidence values for the input item
  qCjyxSubjectHierarchyAbstractPlugin* ownerPlugin = nullptr;
  if (mostSuitablePlugins.size() > 1)
    {
    // Let the user choose a plugin if more than one returned the same non-zero confidence value
    vtkDMMLNode* dataNode = this->m_DMMLScene->GetSubjectHierarchyNode()->GetItemDataNode(itemID);
    QString textToDisplay = QString("Equal confidence number found for more than one subject hierarchy plugin.\n\n"
                                    "Select plugin to own node named\n'%1'\n(type %2):").arg(
                                    dataNode?dataNode->GetName():"NULL").arg(dataNode?dataNode->GetNodeTagName():"None");
    ownerPlugin = this->selectPluginFromDialog(textToDisplay, mostSuitablePlugins);
    }
  else if (mostSuitablePlugins.size() == 1)
    {
    // One plugin found
    ownerPlugin = mostSuitablePlugins[0];
    }
  else
    {
    // Choose default plugin if all registered plugins returned confidence value 0
    ownerPlugin = m_DefaultPlugin;
    }

  return ownerPlugin;
}

//---------------------------------------------------------------------------
qCjyxSubjectHierarchyAbstractPlugin* qCjyxSubjectHierarchyPluginHandler::findAndSetOwnerPluginForSubjectHierarchyItem(vtkIdType itemID)
{
  if (this->m_DMMLScene != nullptr && this->m_DMMLScene->GetSubjectHierarchyNode() == nullptr)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy node";
    return nullptr;
    }

  qCjyxSubjectHierarchyAbstractPlugin* ownerPlugin = this->findOwnerPluginForSubjectHierarchyItem(itemID);
  this->m_DMMLScene->GetSubjectHierarchyNode()->SetItemOwnerPluginName(itemID, ownerPlugin->name().toUtf8().constData());
  return ownerPlugin;
}

//---------------------------------------------------------------------------
qCjyxSubjectHierarchyAbstractPlugin* qCjyxSubjectHierarchyPluginHandler::getOwnerPluginForSubjectHierarchyItem(vtkIdType itemID)
{
  if (this->m_DMMLScene != nullptr && this->m_DMMLScene->GetSubjectHierarchyNode() == nullptr)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy node";
    return nullptr;
    }

  std::string ownerPluginName = this->m_DMMLScene->GetSubjectHierarchyNode()->GetItemOwnerPluginName(itemID);
  if (ownerPluginName.empty())
    {
    if (itemID != this->m_DMMLScene->GetSubjectHierarchyNode()->GetSceneItemID())
      {
      qCritical() << Q_FUNC_INFO << ": Item '" << this->m_DMMLScene->GetSubjectHierarchyNode()->GetItemName(itemID).c_str() << "' is not owned by any plugin";
      }
    return nullptr;
    }

  return this->pluginByName(ownerPluginName.c_str());
}

//---------------------------------------------------------------------------
qCjyxSubjectHierarchyAbstractPlugin* qCjyxSubjectHierarchyPluginHandler::selectPluginFromDialog(
  QString textToDisplay, QList<qCjyxSubjectHierarchyAbstractPlugin*> candidatePlugins)
{
  if (candidatePlugins.empty())
    {
    qCritical() << Q_FUNC_INFO << ": Empty candidate plugin list! Returning default plugin.";
    return m_DefaultPlugin;
    }

  // Convert list of plugin objects to string list for the dialog
  QStringList candidatePluginNames;
  foreach (qCjyxSubjectHierarchyAbstractPlugin* plugin, candidatePlugins)
    {
    candidatePluginNames << plugin->name();
    }

  // Show dialog with a combobox containing the plugins in the input list
  bool ok = false;
  QString selectedPluginName = QInputDialog::getItem(nullptr, "Select subject hierarchy plugin", textToDisplay, candidatePluginNames, 0, false, &ok);
  if (ok && !selectedPluginName.isEmpty())
    {
    // The user pressed OK, find the object for the selected plugin [1]
    foreach (qCjyxSubjectHierarchyAbstractPlugin* plugin, candidatePlugins)
      {
      if (!selectedPluginName.compare(plugin->name()))
        {
        return plugin;
        }
      }
    }

  // User pressed cancel (or [1] failed to find the plugin)
  qWarning() << Q_FUNC_INFO << ": Plugin selection failed! Returning first available plugin";
  return candidatePlugins[0];
}

//----------------------------------------------------------------------------
void qCjyxSubjectHierarchyPluginHandler::observeSubjectHierarchyNode(vtkDMMLSubjectHierarchyNode* shNode)
{
  if (m_DMMLScene != nullptr)
  {
    m_DMMLScene->GetSubjectHierarchyNode()->RemoveObserver(m_CallBack);
  }

  if (shNode != nullptr)
  {
    this->setDMMLScene(shNode->GetScene());

    shNode->AddObserver(vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemAddedEvent, m_CallBack);
    shNode->AddObserver(vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemOwnerPluginSearchRequested, m_CallBack);
    shNode->AddObserver(vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemsShowInViewRequestedEvent, m_CallBack);
  }
}

//-----------------------------------------------------------------------------
vtkDMMLSubjectHierarchyNode* qCjyxSubjectHierarchyPluginHandler::subjectHierarchyNode()const
{
  return m_DMMLScene == nullptr ? nullptr : m_DMMLScene->GetSubjectHierarchyNode();
}

//-----------------------------------------------------------------------------
void qCjyxSubjectHierarchyPluginHandler::setDMMLScene(vtkDMMLScene* scene)
{
  if (scene == this->m_DMMLScene)
    {
    return;
    }

  if (m_DMMLScene != nullptr)
    {
    m_DMMLScene->RemoveObserver(m_CallBack);
    }

  m_DMMLScene = scene;

  if (m_DMMLScene != nullptr)
    {
    scene->AddObserver(vtkDMMLScene::NodeRemovedEvent, m_CallBack);
    this->observeSubjectHierarchyNode(m_DMMLScene->GetSubjectHierarchyNode());
    }
}

//-----------------------------------------------------------------------------
vtkDMMLScene* qCjyxSubjectHierarchyPluginHandler::dmmlScene()const
{
  return m_DMMLScene;
}

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyPluginLogic* qCjyxSubjectHierarchyPluginHandler::pluginLogic()
{
  return m_PluginLogic;
}

//-----------------------------------------------------------------------------
void qCjyxSubjectHierarchyPluginHandler::setPluginLogic(qCjyxSubjectHierarchyPluginLogic* pluginLogic)
{
  if (m_PluginLogic && pluginLogic != m_PluginLogic)
    {
    qCritical() << Q_FUNC_INFO << ": Subject hierarchy plugin logic is a singleton and must not change";
    return;
    }

  m_PluginLogic = pluginLogic;

  // Register view menu actions of those plugins that were registered before the PluginLogic was set.
  if (this->m_PluginLogic)
    {
    foreach(qCjyxSubjectHierarchyAbstractPlugin* pluginToRegister, this->m_RegisteredPlugins)
      {
      foreach(QAction * action, pluginToRegister->viewContextMenuActions())
        {
        if (action != nullptr && action->objectName().isEmpty())
          {
          qWarning() << Q_FUNC_INFO << ": action name is not set for menu item "
            << action->text() << " provided subject hierarchy by plugin " << pluginToRegister->name();
          }
        this->m_PluginLogic->registerViewContextMenuAction(action);
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qCjyxSubjectHierarchyPluginHandler::setCurrentItem(vtkIdType itemID)
{
  this->m_CurrentItems.clear();
  this->m_CurrentItems.append(itemID);
}

//-----------------------------------------------------------------------------
vtkIdType qCjyxSubjectHierarchyPluginHandler::currentItem()
{
  if (this->m_CurrentItems.size() != 1)
    {
    return vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }
  return this->m_CurrentItems.at(0);
}

//-----------------------------------------------------------------------------
void qCjyxSubjectHierarchyPluginHandler::setCurrentItems(QList<vtkIdType> items)
{
  this->m_CurrentItems = items;
}

//-----------------------------------------------------------------------------
QList<vtkIdType> qCjyxSubjectHierarchyPluginHandler::currentItems()
{
  return this->m_CurrentItems;
}

//------------------------------------------------------------------------------
void qCjyxSubjectHierarchyPluginHandler::currentItems(vtkIdList* selectedItems)
{
  if (!selectedItems)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid item list";
    return;
    }

  foreach (vtkIdType item, this->m_CurrentItems)
    {
    selectedItems->InsertNextId(item);
    }
}

//-----------------------------------------------------------------------------
bool qCjyxSubjectHierarchyPluginHandler::autoDeleteSubjectHierarchyChildren()const
{
  QSettings* settings = qCjyxApplication::application()->settingsDialog()->settings();
  if (settings->contains("SubjectHierarchy/AutoDeleteSubjectHierarchyChildren"))
    {
    return settings->value("SubjectHierarchy/AutoDeleteSubjectHierarchyChildren").toBool();
    }

  return false; // Default value
}

//-----------------------------------------------------------------------------
void qCjyxSubjectHierarchyPluginHandler::setAutoDeleteSubjectHierarchyChildren(bool on)
{
  QSettings* settings = qCjyxApplication::application()->settingsDialog()->settings();
  settings->setValue("SubjectHierarchy/AutoDeleteSubjectHierarchyChildren", on);
}

//-----------------------------------------------------------------------------
bool qCjyxSubjectHierarchyPluginHandler::displayPatientIDInSubjectHierarchyItemName()const
{
  QSettings* settings = qCjyxApplication::application()->settingsDialog()->settings();
  if (settings->contains("SubjectHierarchy/DisplayPatientIDInSubjectHierarchyItemName"))
    {
    return settings->value("SubjectHierarchy/DisplayPatientIDInSubjectHierarchyItemName").toBool();
    }

  return true; // Default value
}

//-----------------------------------------------------------------------------
void qCjyxSubjectHierarchyPluginHandler::setDisplayPatientIDInSubjectHierarchyItemName(bool on)
{
  QSettings* settings = qCjyxApplication::application()->settingsDialog()->settings();
  settings->setValue("SubjectHierarchy/DisplayPatientIDInSubjectHierarchyItemName", on);
}

//-----------------------------------------------------------------------------
bool qCjyxSubjectHierarchyPluginHandler::displayPatientBirthDateInSubjectHierarchyItemName()const
{
  QSettings* settings = qCjyxApplication::application()->settingsDialog()->settings();
  if (settings->contains("SubjectHierarchy/DisplayPatientBirthDateInSubjectHierarchyItemName"))
    {
    return settings->value("SubjectHierarchy/DisplayPatientBirthDateInSubjectHierarchyItemName").toBool();
    }

  return false; // Default value
}

//-----------------------------------------------------------------------------
void qCjyxSubjectHierarchyPluginHandler::setDisplayPatientBirthDateInSubjectHierarchyItemName(bool on)
{
  QSettings* settings = qCjyxApplication::application()->settingsDialog()->settings();
  settings->setValue("SubjectHierarchy/DisplayPatientBirthDateInSubjectHierarchyItemName", on);
}

//-----------------------------------------------------------------------------
bool qCjyxSubjectHierarchyPluginHandler::displayStudyIDInSubjectHierarchyItemName()const
{
  QSettings* settings = qCjyxApplication::application()->settingsDialog()->settings();
  if (settings->contains("SubjectHierarchy/DisplayStudyIDInSubjectHierarchyItemName"))
    {
    return settings->value("SubjectHierarchy/DisplayStudyIDInSubjectHierarchyItemName").toBool();
    }

  return false; // Default value
}

//-----------------------------------------------------------------------------
void qCjyxSubjectHierarchyPluginHandler::setDisplayStudyIDInSubjectHierarchyItemName(bool on)
{
  QSettings* settings = qCjyxApplication::application()->settingsDialog()->settings();
  settings->setValue("SubjectHierarchy/DisplayStudyIDInSubjectHierarchyItemName", on);
}

//-----------------------------------------------------------------------------
bool qCjyxSubjectHierarchyPluginHandler::displayStudyDateInSubjectHierarchyItemName()const
{
  QSettings* settings = qCjyxApplication::application()->settingsDialog()->settings();
  if (settings->contains("SubjectHierarchy/DisplayStudyDateInSubjectHierarchyItemName"))
    {
    return settings->value("SubjectHierarchy/DisplayStudyDateInSubjectHierarchyItemName").toBool();
    }

  return true; // Default value
}

//-----------------------------------------------------------------------------
void qCjyxSubjectHierarchyPluginHandler::setDisplayStudyDateInSubjectHierarchyItemName(bool on)
{
  QSettings* settings = qCjyxApplication::application()->settingsDialog()->settings();
  settings->setValue("SubjectHierarchy/DisplayStudyDateInSubjectHierarchyItemName", on);
}

//-----------------------------------------------------------------------------
void qCjyxSubjectHierarchyPluginHandler::onSubjectHierarchyNodeEvent(
  vtkObject* caller, unsigned long event, void* clientData, void* callData )
{
  vtkDMMLSubjectHierarchyNode* shNode = reinterpret_cast<vtkDMMLSubjectHierarchyNode*>(caller);
  vtkDMMLScene* scene = reinterpret_cast<vtkDMMLScene*>(caller);
  qCjyxSubjectHierarchyPluginHandler* pluginHandler = reinterpret_cast<qCjyxSubjectHierarchyPluginHandler*>(clientData);
  if (!pluginHandler || (!shNode && !scene))
    {
    qCritical() << Q_FUNC_INFO << ": Invalid event parameters";
    return;
    }

  // Handle subject hierarchy node events
  if ( ( event == vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemAddedEvent
      || event == vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemOwnerPluginSearchRequested )
      && shNode->GetScene() && !shNode->GetScene()->IsImporting() )
    {
    // Get item ID
    vtkIdType itemID = vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID;
    if (callData)
      {
      vtkIdType* itemIdPtr = reinterpret_cast<vtkIdType*>(callData);
      if (itemIdPtr)
        {
        itemID = *itemIdPtr;
        }
      }

      // Find plugin for added subject hierarchy item and "claim" it
      pluginHandler->findAndSetOwnerPluginForSubjectHierarchyItem(itemID);
    }
  // Handle scene events
  else if (event == vtkDMMLScene::NodeRemovedEvent)
    {
    // Get node (if any)
    vtkDMMLNode* node = vtkDMMLNode::SafeDownCast(reinterpret_cast<vtkObject*>(callData));

    if (!scene->IsClosing() && node->IsA("vtkDMMLSubjectHierarchyNode"))
      {
      // Make sure there is one subject hierarchy node in the scene
      vtkDMMLSubjectHierarchyNode* newSubjectHierarchyNode = vtkDMMLSubjectHierarchyNode::ResolveSubjectHierarchy(scene);
      if (!newSubjectHierarchyNode)
        {
        qCritical() << Q_FUNC_INFO << ": No subject hierarchy node could be retrieved from the scene";
        }
      }
    }
  // Handle scene events
  else if (event == vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemsShowInViewRequestedEvent)
  {
    if (!callData)
      {
      qCritical() << Q_FUNC_INFO << ": SubjectHierarchyItemsShowInViewEvent processing failed, invalid event data";
      return;
      }
    vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemsShowInViewRequestedEventData* showNodesEventData
      = reinterpret_cast<vtkDMMLSubjectHierarchyNode::SubjectHierarchyItemsShowInViewRequestedEventData*>(callData);
    pluginHandler->showItemsInView(showNodesEventData->itemIDsToShow, showNodesEventData->viewNode);
  }
}

//-----------------------------------------------------------------------------
void qCjyxSubjectHierarchyPluginHandler::showItemsInView(vtkIdList* itemIDsToShow, vtkDMMLAbstractViewNode* viewNode)
{
  if (!itemIDsToShow)
    {
    return;
    }
  if (!this->m_DMMLScene || !this->m_DMMLScene->GetSubjectHierarchyNode())
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy node";
    return;
    }

  // Collect all items that will be shown
  vtkDMMLSubjectHierarchyNode* shNode = m_DMMLScene->GetSubjectHierarchyNode();
  QSet<vtkIdType> allItemIDsSet;
  for (int index = 0; index < itemIDsToShow->GetNumberOfIds(); ++index)
    {
    vtkIdType currentItemID = itemIDsToShow->GetId(index);
    // Add item itself
    allItemIDsSet << currentItemID;
    // Add child items recursively
    std::vector<vtkIdType> childItemIDs;
    shNode->GetItemChildren(currentItemID, childItemIDs, true);
    for (auto childItem : childItemIDs)
      {
      allItemIDsSet << childItem;
      }
    }
  vtkNew<vtkIdList> allItemIDsToShow;
  foreach (vtkIdType itemID, allItemIDsSet)
    {
    allItemIDsToShow->InsertNextId(itemID);
    }

  // Show each dropped item and their children
  for (int index = 0; index < allItemIDsToShow->GetNumberOfIds(); ++index)
    {
    vtkIdType itemID = allItemIDsToShow->GetId(index);
    qCjyxSubjectHierarchyAbstractPlugin* ownerPlugin = this->getOwnerPluginForSubjectHierarchyItem(itemID);
    if (ownerPlugin)
      {
      ownerPlugin->showItemInView(itemID, viewNode, allItemIDsToShow);
      }
    }
}

//-----------------------------------------------------------------------------
QString qCjyxSubjectHierarchyPluginHandler::dumpContextMenuActions()
{
  QString info;
  QList< QAction* > actions;

  info.append("=== Item context menu ===\n");
  actions.clear();
  foreach(qCjyxSubjectHierarchyAbstractPlugin* plugin, this->m_RegisteredPlugins)
    {
    actions << plugin->itemContextMenuActions();
    }
  info.append(qCjyxSubjectHierarchyPluginLogic::buildMenuFromActions(nullptr, actions));

  info.append("\n=== Scene context menu ===\n");
  actions.clear();
  foreach(qCjyxSubjectHierarchyAbstractPlugin* plugin, this->m_RegisteredPlugins)
    {
    actions << plugin->sceneContextMenuActions();
    }
  info.append(qCjyxSubjectHierarchyPluginLogic::buildMenuFromActions(nullptr, actions));

  info.append("\n=== Visibility context menu ===\n");
  actions.clear();
  foreach(qCjyxSubjectHierarchyAbstractPlugin* plugin, this->m_RegisteredPlugins)
    {
    actions << plugin->visibilityContextMenuActions();
    }
  info.append(qCjyxSubjectHierarchyPluginLogic::buildMenuFromActions(nullptr, actions));

  info.append("\n=== View context menu ===\n");
  actions.clear();
  foreach(qCjyxSubjectHierarchyAbstractPlugin* plugin, this->m_RegisteredPlugins)
    {
    actions << plugin->viewContextMenuActions();
    }
  info.append(qCjyxSubjectHierarchyPluginLogic::buildMenuFromActions(nullptr, actions));

  return info;
}
