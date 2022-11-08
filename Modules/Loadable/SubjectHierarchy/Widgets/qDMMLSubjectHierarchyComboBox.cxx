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

// Qt includes
#include <QDebug>
#include <QFrame>
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QLabel>

// SubjectHierarchy includes
#include "qDMMLSubjectHierarchyComboBox.h"

#include "qDMMLSubjectHierarchyTreeView.h"
#include "qDMMLSubjectHierarchyModel.h"
#include "qDMMLSortFilterSubjectHierarchyProxyModel.h"

#include "qCjyxSubjectHierarchyPluginHandler.h"
#include "qCjyxSubjectHierarchyAbstractPlugin.h"

// DMML includes
#include <vtkDMMLScene.h>

//------------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_SubjectHierarchy
class qDMMLSubjectHierarchyComboBoxPrivate
{
  Q_DECLARE_PUBLIC(qDMMLSubjectHierarchyComboBox);

protected:
  qDMMLSubjectHierarchyComboBox* const q_ptr;

public:
  qDMMLSubjectHierarchyComboBoxPrivate(qDMMLSubjectHierarchyComboBox& object);
  virtual void init();

public:
  int MaximumNumberOfShownItems;
  bool AlignPopupVertically;
  bool ShowCurrentItemParents;

  qDMMLSubjectHierarchyTreeView* TreeView;
  QLabel* NoItemLabel;
};

//------------------------------------------------------------------------------
qDMMLSubjectHierarchyComboBoxPrivate::qDMMLSubjectHierarchyComboBoxPrivate(qDMMLSubjectHierarchyComboBox& object)
  : q_ptr(&object)
  , MaximumNumberOfShownItems(20)
  , AlignPopupVertically(true)
  , ShowCurrentItemParents(true)
  , TreeView(nullptr)
  , NoItemLabel(nullptr)
{
}

// --------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBoxPrivate::init()
{
  Q_Q(qDMMLSubjectHierarchyComboBox);

  q->forceDefault(true);

  q->setDefaultText("Select subject hierarchy item");
  q->setDefaultIcon(QIcon());
  q->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

  // Setup tree view
  this->TreeView = new qDMMLSubjectHierarchyTreeView(q);
  this->TreeView->setMultiSelection(false);
  this->TreeView->setColumnHidden(this->TreeView->model()->visibilityColumn(), true);
  this->TreeView->setColumnHidden(this->TreeView->model()->transformColumn(), true);
  this->TreeView->setColumnHidden(this->TreeView->model()->idColumn(), true);
  this->TreeView->setHeaderHidden(true);
  this->TreeView->setContextMenuEnabled(false);
  this->TreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  this->TreeView->setDragDropMode(QAbstractItemView::NoDragDrop);

  // No item label
  this->NoItemLabel = new QLabel("No items");
  this->NoItemLabel->setMargin(4);

  // Add tree view to container
  QFrame* container = qobject_cast<QFrame*>(q->view()->parentWidget());
  container->layout()->addWidget(this->NoItemLabel);
  container->layout()->addWidget(this->TreeView);

  // Make connections
  QObject::connect(this->TreeView, SIGNAL(currentItemChanged(vtkIdType)),
                   q, SLOT(updateComboBoxTitleAndIcon(vtkIdType)));
  QObject::connect(this->TreeView, SIGNAL(currentItemChanged(vtkIdType)),
                   q, SLOT(hidePopup()));
  QObject::connect(this->TreeView, SIGNAL(currentItemModified(vtkIdType)),
                   q, SLOT(updateComboBoxTitleAndIcon(vtkIdType)));
  QObject::connect(this->TreeView, SIGNAL(currentItemChanged(vtkIdType)),
                   q, SIGNAL(currentItemChanged(vtkIdType)));
  QObject::connect(this->TreeView, SIGNAL(currentItemModified(vtkIdType)),
                   q, SIGNAL(currentItemModified(vtkIdType)));
}

// --------------------------------------------------------------------------
// qDMMLSubjectHierarchyComboBox

// --------------------------------------------------------------------------
qDMMLSubjectHierarchyComboBox::qDMMLSubjectHierarchyComboBox(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qDMMLSubjectHierarchyComboBoxPrivate(*this))
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLSubjectHierarchyComboBox::~qDMMLSubjectHierarchyComboBox() = default;

//------------------------------------------------------------------------------
vtkDMMLSubjectHierarchyNode* qDMMLSubjectHierarchyComboBox::subjectHierarchyNode()const
{
  Q_D(const qDMMLSubjectHierarchyComboBox);
  return d->TreeView->subjectHierarchyNode();
}

//------------------------------------------------------------------------------
vtkDMMLScene* qDMMLSubjectHierarchyComboBox::dmmlScene()const
{
  Q_D(const qDMMLSubjectHierarchyComboBox);
  return d->TreeView->dmmlScene();
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::setDMMLScene(vtkDMMLScene* scene)
{
  Q_D(const qDMMLSubjectHierarchyComboBox);

  if (this->dmmlScene() == scene)
    {
    return;
    }

  d->TreeView->setDMMLScene(scene);

  vtkDMMLSubjectHierarchyNode* shNode = d->TreeView->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
    }

  // Connect scene events so that title can be updated
  qvtkReconnect( scene, vtkDMMLScene::EndCloseEvent, this, SLOT( onDMMLSceneCloseEnded(vtkObject*) ) );

  // Set tree root item to be the new scene, and disable showing it
  d->TreeView->setRootItem(shNode->GetSceneItemID());
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::clearSelection()
{
  Q_D(const qDMMLSubjectHierarchyComboBox);
  d->TreeView->clearSelection();

  // Clear title and icon
  this->updateComboBoxTitleAndIcon(vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID);
}

//------------------------------------------------------------------------------
vtkIdType qDMMLSubjectHierarchyComboBox::currentItem()const
{
  Q_D(const qDMMLSubjectHierarchyComboBox);
  return d->TreeView->currentItem();
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::setCurrentItem(vtkIdType itemID)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  return d->TreeView->setCurrentItem(itemID);
}

//--------------------------------------------------------------------------
qDMMLSortFilterSubjectHierarchyProxyModel* qDMMLSubjectHierarchyComboBox::sortFilterProxyModel()const
{
  Q_D(const qDMMLSubjectHierarchyComboBox);
  return d->TreeView->sortFilterProxyModel();
}

//--------------------------------------------------------------------------
qDMMLSubjectHierarchyModel* qDMMLSubjectHierarchyComboBox::model()const
{
  Q_D(const qDMMLSubjectHierarchyComboBox);
  return d->TreeView->model();
}

//--------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::setShowRootItem(bool show)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  d->TreeView->setShowRootItem(show);
}

//--------------------------------------------------------------------------
bool qDMMLSubjectHierarchyComboBox::showRootItem()const
{
  Q_D(const qDMMLSubjectHierarchyComboBox);
  return d->TreeView->showRootItem();
}

//--------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::setRootItem(vtkIdType rootItemID)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  d->TreeView->setRootItem(rootItemID);
}

//--------------------------------------------------------------------------
vtkIdType qDMMLSubjectHierarchyComboBox::rootItem()const
{
  Q_D(const qDMMLSubjectHierarchyComboBox);
  return d->TreeView->rootItem();
}

//--------------------------------------------------------------------------
bool qDMMLSubjectHierarchyComboBox::highlightReferencedItems()const
{
  Q_D(const qDMMLSubjectHierarchyComboBox);
  return d->TreeView->highlightReferencedItems();
}

//--------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::setHighlightReferencedItems(bool highlightOn)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  d->TreeView->setHighlightReferencedItems(highlightOn);
}

//--------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::setPluginWhitelist(QStringList whitelist)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  d->TreeView->setPluginWhitelist(whitelist);
}

//--------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::setPluginBlacklist(QStringList blacklist)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  d->TreeView->setPluginBlacklist(blacklist);
}

//--------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::disablePlugin(QString plugin)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  d->TreeView->disablePlugin(plugin);
}

//-----------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::setIncludeItemAttributeNamesFilter(QStringList filter)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  this->sortFilterProxyModel()->setIncludeItemAttributeNamesFilter(filter);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  d->TreeView->setRootItem(d->TreeView->rootItem());
}

//-----------------------------------------------------------------------------
QStringList qDMMLSubjectHierarchyComboBox::includeItemAttributeNamesFilter()const
{
  return this->sortFilterProxyModel()->includeItemAttributeNamesFilter();
}

//-----------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::setIncludeNodeAttributeNamesFilter(QStringList filter)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  this->sortFilterProxyModel()->setIncludeNodeAttributeNamesFilter(filter);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  d->TreeView->setRootItem(d->TreeView->rootItem());
}

//-----------------------------------------------------------------------------
QStringList qDMMLSubjectHierarchyComboBox::includeNodeAttributeNamesFilter()const
{
  return this->sortFilterProxyModel()->includeNodeAttributeNamesFilter();
}

//-----------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::setExcludeItemAttributeNamesFilter(QStringList filter)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  this->sortFilterProxyModel()->setExcludeItemAttributeNamesFilter(filter);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  d->TreeView->setRootItem(d->TreeView->rootItem());
}

//-----------------------------------------------------------------------------
QStringList qDMMLSubjectHierarchyComboBox::excludeItemAttributeNamesFilter()const
{
  return this->sortFilterProxyModel()->excludeItemAttributeNamesFilter();
}

//-----------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::setExcludeNodeAttributeNamesFilter(QStringList filter)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  this->sortFilterProxyModel()->setExcludeNodeAttributeNamesFilter(filter);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  d->TreeView->setRootItem(d->TreeView->rootItem());
}

//-----------------------------------------------------------------------------
QStringList qDMMLSubjectHierarchyComboBox::excludeNodeAttributeNamesFilter()const
{
  return this->sortFilterProxyModel()->excludeNodeAttributeNamesFilter();
}

//-----------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::setAttributeNameFilter(QString& filter)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  this->sortFilterProxyModel()->setAttributeNameFilter(filter);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  d->TreeView->setRootItem(d->TreeView->rootItem());
}

//-----------------------------------------------------------------------------
QString qDMMLSubjectHierarchyComboBox::attributeNameFilter()const
{
  return this->sortFilterProxyModel()->attributeNameFilter();
}

//-----------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::setAttributeValueFilter(QString& filter)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  this->sortFilterProxyModel()->setAttributeValueFilter(filter);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  d->TreeView->setRootItem(d->TreeView->rootItem());
}

//-----------------------------------------------------------------------------
QString qDMMLSubjectHierarchyComboBox::attributeValueFilter()const
{
  return this->sortFilterProxyModel()->attributeValueFilter();
}

//-----------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::addItemAttributeFilter(QString attributeName, QVariant attributeValue/*=QString()*/, bool include/*=true*/)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  this->sortFilterProxyModel()->addItemAttributeFilter(attributeName, attributeValue, include);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  d->TreeView->setRootItem(d->TreeView->rootItem());
}

//-----------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::removeItemAttributeFilter(QString attributeName, QVariant attributeValue, bool include)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  this->sortFilterProxyModel()->removeItemAttributeFilter(attributeName, attributeValue, include);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  d->TreeView->setRootItem(d->TreeView->rootItem());
}

//-----------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::removeItemAttributeFilter(QString attributeName, bool include)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  this->sortFilterProxyModel()->removeItemAttributeFilter(attributeName, include);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  d->TreeView->setRootItem(d->TreeView->rootItem());
}

//-----------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::addNodeAttributeFilter(
  QString attributeName, QVariant attributeValue/*=QString()*/, bool include/*=true*/, QString className/*=QString()*/)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  this->sortFilterProxyModel()->addNodeAttributeFilter(attributeName, attributeValue, include, className);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  d->TreeView->setRootItem(d->TreeView->rootItem());
}

//-----------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::removeNodeAttributeFilter(QString attributeName, QVariant attributeValue, bool include, QString className)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  this->sortFilterProxyModel()->removeNodeAttributeFilter(attributeName, attributeValue, include, className);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  d->TreeView->setRootItem(d->TreeView->rootItem());
}

//-----------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::removeNodeAttributeFilter(QString attributeName, bool include)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  this->sortFilterProxyModel()->removeNodeAttributeFilter(attributeName, include);

  // Reset root item, as it may have been corrupted, when tree became empty due to the filter
  d->TreeView->setRootItem(d->TreeView->rootItem());
}

//--------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::setLevelFilter(QStringList &levelFilter)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  d->TreeView->setLevelFilter(levelFilter);
}

//--------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::setNodeTypes(const QStringList& types)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  d->TreeView->setNodeTypes(types);
}

//--------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::setHideChildNodeTypes(const QStringList& types)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  d->TreeView->setHideChildNodeTypes(types);
}

//--------------------------------------------------------------------------
int qDMMLSubjectHierarchyComboBox::maximumNumberOfShownItems()const
{
  Q_D(const qDMMLSubjectHierarchyComboBox);
  return d->MaximumNumberOfShownItems;
}

//--------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::setMaximumNumberOfShownItems(int maxNumberOfShownItems)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  d->MaximumNumberOfShownItems = maxNumberOfShownItems;
}

//--------------------------------------------------------------------------
bool qDMMLSubjectHierarchyComboBox::alignPopupVertically()const
{
  Q_D(const qDMMLSubjectHierarchyComboBox);
  return d->AlignPopupVertically;
}

//--------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::setAlignPopupVertically(bool align)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  d->AlignPopupVertically = align;
}

//--------------------------------------------------------------------------
bool qDMMLSubjectHierarchyComboBox::noneEnabled()const
{
  if (!this->model())
    {
    return false;
    }
  return this->model()->noneEnabled();
}

//--------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::setNoneEnabled(bool enable)
{
  if (!this->model())
    {
    return;
    }
  this->model()->setNoneEnabled(enable);
}

//--------------------------------------------------------------------------
QString qDMMLSubjectHierarchyComboBox::noneDisplay()const
{
  if (!this->model())
    {
    return QString();
    }
  return this->model()->noneDisplay();
}

//--------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::setNoneDisplay(const QString& displayName)
{
  if (!this->model())
    {
    return;
    }
  this->model()->setNoneDisplay(displayName);
}

//--------------------------------------------------------------------------
bool qDMMLSubjectHierarchyComboBox::showCurrentItemParents()const
{
  Q_D(const qDMMLSubjectHierarchyComboBox);
  return d->ShowCurrentItemParents;
}

//--------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::setShowCurrentItemParents(bool show)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  d->ShowCurrentItemParents = show;
}

//--------------------------------------------------------------------------
bool qDMMLSubjectHierarchyComboBox::showEmptyHierarchyItems()const
{
  Q_D(const qDMMLSubjectHierarchyComboBox);
  return this->sortFilterProxyModel()->showEmptyHierarchyItems();
}

//--------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::setShowEmptyHierarchyItems(bool show)
{
  Q_D(qDMMLSubjectHierarchyComboBox);
  this->sortFilterProxyModel()->setShowEmptyHierarchyItems(show);
}

//-----------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::showPopup()
{
  Q_D(qDMMLSubjectHierarchyComboBox);

  QFrame* container = qobject_cast<QFrame*>(this->view()->parentWidget());
  QStyleOptionComboBox opt;
  this->initStyleOption(&opt);

  QRect listRect(this->style()->subControlRect(QStyle::CC_ComboBox, &opt,
                                               QStyle::SC_ComboBoxListBoxPopup, this));
  QRect screen = QApplication::desktop()->availableGeometry(QApplication::desktop()->screenNumber(this));
  QPoint below = mapToGlobal(listRect.bottomLeft());
  QPoint above = mapToGlobal(listRect.topLeft());

  // Custom Height
  int popupHeight = 0;
  int displayedItemCount = d->TreeView->displayedItemCount();
  if (displayedItemCount == 0)
    {
    // If there is no items, find what message to show instead
    vtkDMMLSubjectHierarchyNode* shNode = d->TreeView->subjectHierarchyNode();
    vtkIdType rootItem = d->TreeView->rootItem();
    std::vector<vtkIdType> childItemIDs;
    shNode->GetItemChildren(rootItem, childItemIDs, false);
    if (childItemIDs.empty())
      {
      if (rootItem != shNode->GetSceneItemID())
        {
        std::string rootName = shNode->GetItemName(rootItem);
        QString label = QString("No items in branch: ") + QString::fromStdString(rootName);
        d->NoItemLabel->setText(label);
        }
      else
        {
        d->NoItemLabel->setText("No items in scene");
        }
      }
    else
      {
      d->NoItemLabel->setText("No items accepted by current filters");
      }

      // Show no item label instead of tree view
      d->NoItemLabel->show();
      d->TreeView->hide();
      popupHeight = d->NoItemLabel->sizeHint().height();
    }
  else
    {
    // Height based on the number of items
    const int numberOfShownShItems = qMin(displayedItemCount, d->MaximumNumberOfShownItems);
    const int numberOfRows = (this->noneEnabled() ? numberOfShownShItems + 1 : numberOfShownShItems);
    const int referenceRowHeight = (this->noneEnabled() ? d->TreeView->sizeHintForRow(1) : d->TreeView->sizeHintForRow(0));
    popupHeight = numberOfRows * referenceRowHeight;

    // Add tree view margins for the height
    // NB: not needed for the width as the item labels will be cropped
    // without displaying an horizontal scroll bar
    QMargins tvMargins = d->TreeView->contentsMargins();
    popupHeight += tvMargins.top() + tvMargins.bottom();

    d->NoItemLabel->hide();
    d->TreeView->show();
    }

  // Add container margins for the height
  QMargins margins = container->contentsMargins();
  popupHeight += margins.top() + margins.bottom();

  // Position of the container
  if(d->AlignPopupVertically)
    {
    // Position horizontally
    listRect.moveLeft(above.x());

    // Position vertically so the currently selected item lines up with the combo box
    const QRect currentItemRect = d->TreeView->visualRect(d->TreeView->currentIndex());
    const int offset = listRect.top() - currentItemRect.top();
    listRect.moveTop(above.y() + offset - listRect.top());

    if (listRect.width() > screen.width() )
      {
      listRect.setWidth(screen.width());
      }
    if (mapToGlobal(listRect.bottomRight()).x() > screen.right())
      {
      below.setX(screen.x() + screen.width() - listRect.width());
      above.setX(screen.x() + screen.width() - listRect.width());
      }
    if (mapToGlobal(listRect.topLeft()).x() < screen.x() )
      {
      below.setX(screen.x());
      above.setX(screen.x());
      }
    }
  else
    {
    // Position below the combobox
    listRect.moveTo(below);
    }

  container->move(listRect.topLeft());
  container->setFixedHeight(popupHeight);
  container->setFixedWidth(this->width());
  container->raise();
  container->show();

  this->view()->setFocus();
  this->view()->scrollTo( this->view()->currentIndex(),
                          this->style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, this)
                             ? QAbstractItemView::PositionAtCenter
                             : QAbstractItemView::EnsureVisible );
  container->update();
}

//-----------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::hidePopup()
{
  // Hide popup
  QFrame* container = qobject_cast<QFrame*>(this->view()->parentWidget());
  if (container)
    {
    container->hide();
    }
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::mousePressEvent(QMouseEvent* e)
{
  // Disable context menu altogether
  if (e->button() == Qt::RightButton)
    {
    return;
    }

  // Perform default mouse press event (make selections etc.)
  this->Superclass::mousePressEvent(e);
}

//------------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::updateComboBoxTitleAndIcon(vtkIdType selectedShItemID)
{
  Q_D(qDMMLSubjectHierarchyComboBox);

  vtkDMMLSubjectHierarchyNode* shNode = d->TreeView->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    this->setDefaultText("Error: Invalid subject hierarchy");
    this->setDefaultIcon(QIcon());
    return;
    }
  if (!selectedShItemID)
    {
    if (this->noneEnabled())
      {
      this->setDefaultText(this->noneDisplay());
      }
    else
      {
      this->setDefaultText("Select subject hierarchy item");
      }
    this->setDefaultIcon(QIcon());
    return;
    }

  // Assemble title for selected item
  QString titleText(shNode->GetItemName(selectedShItemID).c_str());
  if (d->ShowCurrentItemParents)
    {
    vtkIdType parentItemID = shNode->GetItemParent(selectedShItemID);
    while (parentItemID != shNode->GetSceneItemID() && parentItemID != vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
      {
      titleText.prepend(" / ");
      QString parentItemName(shNode->GetItemName(parentItemID).c_str());
      if (parentItemName.length() > 21)
        {
        // Truncate item name if too long
        parentItemName = parentItemName.left(9) + "..." + parentItemName.right(9);
        }
      titleText.prepend(parentItemName);
      parentItemID = shNode->GetItemParent(parentItemID);
      }
    }
  this->setDefaultText(titleText);

  // Get icon for selected item
  std::string ownerPluginName = shNode->GetItemOwnerPluginName(selectedShItemID);
  qCjyxSubjectHierarchyAbstractPlugin* ownerPlugin =
    qCjyxSubjectHierarchyPluginHandler::instance()->pluginByName(ownerPluginName.c_str());
  if (ownerPlugin)
    {
    this->setDefaultIcon(ownerPlugin->icon(selectedShItemID));
    }
  else
    {
    qCritical() << Q_FUNC_INFO << ": No owner plugin for subject hierarchy item " << shNode->GetItemName(selectedShItemID).c_str();
    this->setDefaultIcon(QIcon());
    }
}

//-----------------------------------------------------------------------------
void qDMMLSubjectHierarchyComboBox::onDMMLSceneCloseEnded(vtkObject* sceneObject)
{
  vtkDMMLScene* scene = vtkDMMLScene::SafeDownCast(sceneObject);
  if (!scene)
    {
    return;
    }

  // Make sure the title generated from previous selection is cleared when closing the scene.
  this->updateComboBoxTitleAndIcon(vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID);
}
