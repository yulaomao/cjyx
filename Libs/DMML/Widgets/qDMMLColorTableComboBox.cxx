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

// CTK includes
#include <ctkTreeComboBox.h>

// qDMML includes
#include "qDMMLColorTableComboBox.h"
#include "qDMMLSceneColorTableModel.h"

//-----------------------------------------------------------------------------
class qDMMLColorTableComboBoxPrivate
{
  Q_DECLARE_PUBLIC(qDMMLColorTableComboBox);
protected:
  qDMMLColorTableComboBox* const q_ptr;
public:
  qDMMLColorTableComboBoxPrivate(qDMMLColorTableComboBox& object);
  void init();
};

// -----------------------------------------------------------------------------
qDMMLColorTableComboBoxPrivate
::qDMMLColorTableComboBoxPrivate(qDMMLColorTableComboBox& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
void qDMMLColorTableComboBoxPrivate::init()
{
  Q_Q(qDMMLColorTableComboBox);
  q->rootModel()->setParent(q);
  ctkTreeComboBox* comboBox = new ctkTreeComboBox;
  // only the first column is visible
  comboBox->setVisibleModelColumn(0);
  q->setComboBox(comboBox);
  q->setShowHidden(true);

  QStringList nodeTypes;
  nodeTypes << QString("vtkDMMLColorTableNode");
  nodeTypes << QString("vtkDMMLProceduralColorNode");
  q->setNodeTypes(nodeTypes);
  q->setAddEnabled(false);
  q->setRemoveEnabled(false);

  QIcon defaultIcon(":blankLUT");
  QList<QSize> iconSizes(defaultIcon.availableSizes());
  if (iconSizes.size() > 0)
    {
    comboBox->setIconSize(iconSizes[0]);
    }
}

// --------------------------------------------------------------------------
qDMMLColorTableComboBox::qDMMLColorTableComboBox(QWidget* parentWidget)
  : Superclass(this->createSceneModel(), parentWidget)
  , d_ptr(new qDMMLColorTableComboBoxPrivate(*this))
{
  Q_D(qDMMLColorTableComboBox);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLColorTableComboBox::~qDMMLColorTableComboBox() = default;

// --------------------------------------------------------------------------
QAbstractItemModel* qDMMLColorTableComboBox::createSceneModel()
{
  return new qDMMLSceneColorTableModel;
}

// --------------------------------------------------------------------------
void qDMMLColorTableComboBox::setDMMLScene(vtkDMMLScene* scene)
{
  this->Superclass::setDMMLScene(scene);
  QModelIndex sceneIndex = this->comboBox()->model()->index(0,0);
  // index(0,0) is the scene.
  this->comboBox()->setRootModelIndex(sceneIndex);
}
