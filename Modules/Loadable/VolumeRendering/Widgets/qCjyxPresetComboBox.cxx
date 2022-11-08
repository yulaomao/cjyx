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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QDesktopWidget>
#include <QListView>

// CTK includes
#include <ctkVTKWidgetsUtils.h>
#include <ctkWidgetsUtils.h>

/// Module logic includes
#include "vtkCjyxVolumeRenderingLogic.h"

// DMMLWidgets includes
#include <qDMMLSceneModel.h>
#include <qDMMLUtils.h>

// VolumeRenderingWidgets includes
#include "qCjyxPresetComboBox_p.h"

// DMML includes
#include <vtkDMMLVolumeNode.h>
#include <vtkDMMLVolumePropertyNode.h>

// VTK includes
#include <vtkColorTransferFunction.h>
#include <vtkVolumeProperty.h>

// STD includes
#include <cassert>

//-----------------------------------------------------------------------------
qCjyxIconComboBox::qCjyxIconComboBox(QWidget* parentWidget)
  :Superclass(parentWidget)
{
  QListView* listView = new QListView(nullptr);
  listView->setViewMode(QListView::IconMode);
  listView->setUniformItemSizes(true);
  listView->setWrapping(true);
  listView->setMovement(QListView::Static);
  listView->setFlow(QListView::LeftToRight);
  listView->setResizeMode(QListView::Adjust); // resize list view if widget width is changed
  listView->setSpacing(0);
  this->setView(listView);
}

//-----------------------------------------------------------------------------
void qCjyxIconComboBox::showPopup()
{
  QFrame* container = qobject_cast<QFrame*>(this->view()->parentWidget());
  QStyleOptionComboBox opt;
  initStyleOption(&opt);

  QRect listRect(this->style()->subControlRect(QStyle::CC_ComboBox, &opt,
                                               QStyle::SC_ComboBoxListBoxPopup, this));
  QRect screen = QApplication::desktop()->availableGeometry(QApplication::desktop()->screenNumber(this));
  QPoint below = mapToGlobal(listRect.bottomLeft());
  QPoint above = mapToGlobal(listRect.topLeft());

  // CustomSize
  //
  const int numberOfItems = this->count();
  QSize itemSize = QSize( this->view()->sizeHintForColumn(0), this->view()->sizeHintForRow(0));
  const int itemsPerRow = itemSize.width() ? listRect.width() / itemSize.width() : 0;
  const int itemsPerColumns = itemsPerRow ? static_cast<int>(0.999 + static_cast<double>(numberOfItems) / itemsPerRow) : 0;
  listRect.setWidth( itemsPerRow * itemSize.width() );
  listRect.setHeight( itemsPerColumns * itemSize.height() );

  QMargins margins = container->contentsMargins();
  listRect.setWidth( listRect.width() + margins.left() + margins.right());
  listRect.setWidth( listRect.width() + container->frameWidth());
  listRect.setHeight( listRect.height() + margins.top() + margins.bottom());
  listRect.setHeight( listRect.height() + container->frameWidth());

  // Position horizontally.
  listRect.moveLeft(above.x());

  // Position vertically so the currently selected item lines up
  // with the combo box.
  const QRect currentItemRect = view()->visualRect(view()->currentIndex());
  const int offset = listRect.top() - currentItemRect.top();
  listRect.moveTop(above.y() + offset - listRect.top());

  if (listRect.width() > screen.width())
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
  container->setGeometry(listRect);
  container->raise();
  container->show();
  this->view()->setFocus();
  this->view()->scrollTo( this->view()->currentIndex(),
    this->style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, this)
      ? QAbstractItemView::PositionAtCenter : QAbstractItemView::EnsureVisible);
  container->update();
}

//-----------------------------------------------------------------------------
qCjyxPresetComboBoxPrivate::qCjyxPresetComboBoxPrivate(
  qCjyxPresetComboBox& object)
  : q_ptr(&object)
  , ShowIcons(true)
{
}

//-----------------------------------------------------------------------------
void qCjyxPresetComboBoxPrivate::init()
{
  Q_Q(qCjyxPresetComboBox);

  q->setNodeTypes(QStringList("vtkDMMLVolumePropertyNode"));
  q->setSelectNodeUponCreation(false);
  q->setAddEnabled(false);
  q->setRemoveEnabled(false);
  q->setBaseName(qCjyxPresetComboBox::tr("Preset"));

  q->setShowIcons(true);

  QObject::connect(q, SIGNAL(nodeAdded(vtkDMMLNode*)), q, SLOT(setIconToPreset(vtkDMMLNode*)));
  QObject::connect(q, SIGNAL(currentNodeChanged(vtkDMMLNode*)), q, SLOT(updateComboBoxTitleAndIcon(vtkDMMLNode*)));
}

// --------------------------------------------------------------------------
qCjyxPresetComboBox::qCjyxPresetComboBox(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qCjyxPresetComboBoxPrivate(*this))
{
  Q_D(qCjyxPresetComboBox);
  d->init();
}

// --------------------------------------------------------------------------
qCjyxPresetComboBox::~qCjyxPresetComboBox() = default;

// --------------------------------------------------------------------------
void qCjyxPresetComboBox::setIconToPreset(vtkDMMLNode* presetNode)
{
  Q_D(qCjyxPresetComboBox);
  if (presetNode == nullptr)
    {
    return;
    }

  qDMMLSceneModel* sceneModel = qobject_cast<qDMMLSceneModel*>(this->sortFilterProxyModel()->sourceModel());

  if (d->ShowIcons)
    {
    // Search corresponding icon
    QIcon presetIcon;
    vtkDMMLVolumeNode* iconVolume = vtkDMMLVolumeNode::SafeDownCast(
      presetNode->GetNodeReference(vtkCjyxVolumeRenderingLogic::GetIconVolumeReferenceRole()));
    if (iconVolume && iconVolume->GetImageData()!=nullptr)
      {
      QImage qimage;
      qDMMLUtils::vtkImageDataToQImage(iconVolume->GetImageData(),qimage);
      presetIcon = QIcon(QPixmap::fromImage(qimage));
      }
    if (presetIcon.availableSizes().size() == 0)
      {
      // Check if an image is available for this preset in the stock preset images
      presetIcon = QIcon(QString(":/presets/") + presetNode->GetName());
      }
    if (presetIcon.availableSizes().size() == 0)
      {
      // Use generic icon (puzzle piece)
      presetIcon = QIcon(":/Icons/Extension.png");
      }

    // Set icon
    sceneModel->setData(sceneModel->indexFromNode(presetNode), presetIcon, Qt::DecorationRole);

    // Update icon size
    QList<QSize> iconSizes = presetIcon.availableSizes();
    if (iconSizes.count())
      {
      QSize iconSize = this->comboBox()->view()->iconSize();
      this->comboBox()->view()->setIconSize(QSize(qMax(iconSize.width(), iconSizes[0].width()),
                                                  qMax(iconSize.height(), iconSizes[0].height())));
      }
    }

  // Set toolTip
  vtkDMMLVolumePropertyNode* volumePropertyNode = vtkDMMLVolumePropertyNode::SafeDownCast(presetNode);
  if (volumePropertyNode)
    {
    int previewSize = this->style()->pixelMetric(QStyle::PM_SmallIconSize);
    vtkScalarsToColors* colors =
      volumePropertyNode->GetVolumeProperty() ? volumePropertyNode->GetVolumeProperty()->GetRGBTransferFunction() : nullptr;
    assert(colors && colors->GetRange()[1] > colors->GetRange()[0]);
    QImage img = ctk::scalarsToColorsImage(colors, QSize(previewSize, previewSize));
    QString imgSrc = ctk::base64HTMLImageTagSrc(img);
    QString toolTip = QString("<img src=\"%1\"> %2").arg(imgSrc).arg(presetNode->GetName());
    sceneModel->setData(sceneModel->indexFromNode(presetNode), toolTip, Qt::ToolTipRole);
    }
}

// --------------------------------------------------------------------------
void qCjyxPresetComboBox::updateComboBoxTitleAndIcon(vtkDMMLNode* node)
{
  Q_D(qCjyxPresetComboBox);
  ctkComboBox* combo = qobject_cast<ctkComboBox*>(this->comboBox());
  if (node)
    {
    combo->setDefaultText(node->GetName());
    combo->setDefaultIcon(d->ShowIcons ? combo->itemIcon(combo->currentIndex()) : QIcon());
    }
  else
    {
    combo->setDefaultText(tr("Select a Preset"));
    combo->setDefaultIcon(QIcon());
    }
}

// --------------------------------------------------------------------------
bool qCjyxPresetComboBox::showIcons()const
{
  Q_D(const qCjyxPresetComboBox);
  return d->ShowIcons;
}

// --------------------------------------------------------------------------
void qCjyxPresetComboBox::setShowIcons(bool show)
{
  Q_D(qCjyxPresetComboBox);
  d->ShowIcons = show;

  vtkDMMLScene* scene = this->dmmlScene();
  qDMMLSceneModel* sceneModel = qobject_cast<qDMMLSceneModel*>(this->sortFilterProxyModel()->sourceModel());

  if (d->ShowIcons)
    {
    qCjyxIconComboBox* comboBox = new qCjyxIconComboBox;
    comboBox->forceDefault(true);
    this->setComboBox(comboBox);
    sceneModel->setNameColumn(-1);
    }
  else
    {
    ctkComboBox* comboBox = new ctkComboBox;
    comboBox->forceDefault(false);
    this->setComboBox(comboBox);
    sceneModel->setNameColumn(0);
    }

  // Update from scene
  QString currentNodeID = this->currentNodeID();
  this->setDMMLScene(nullptr);
  this->setDMMLScene(scene);
  this->updateComboBoxTitleAndIcon(nullptr);
  this->setCurrentNodeID(currentNodeID);
}
