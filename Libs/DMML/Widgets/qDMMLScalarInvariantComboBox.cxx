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
#include <QComboBox>
#include <QHBoxLayout>

// qDMML includes
#include "qDMMLScalarInvariantComboBox.h"

// DMML includes
#include <vtkDMMLDiffusionTensorDisplayPropertiesNode.h>
#include <vtkDMMLDiffusionTensorVolumeDisplayNode.h>

// VTK includes

//------------------------------------------------------------------------------
class qDMMLScalarInvariantComboBoxPrivate
{
  Q_DECLARE_PUBLIC(qDMMLScalarInvariantComboBox);

protected:
  qDMMLScalarInvariantComboBox* const q_ptr;

public:
  qDMMLScalarInvariantComboBoxPrivate(qDMMLScalarInvariantComboBox& object);
  void init();
  void setScalarInvariantToComboBox(int scalarInvariant);

  QComboBox*                                   ComboBox;
  vtkDMMLDiffusionTensorDisplayPropertiesNode* DisplayPropertiesNode;

private:
  void populateComboBox();
};

//------------------------------------------------------------------------------
qDMMLScalarInvariantComboBoxPrivate::qDMMLScalarInvariantComboBoxPrivate(
  qDMMLScalarInvariantComboBox& object)
  : q_ptr(&object)
{
  this->ComboBox = nullptr;
  this->DisplayPropertiesNode = nullptr;
}

//------------------------------------------------------------------------------
void qDMMLScalarInvariantComboBoxPrivate::init()
{
  Q_Q(qDMMLScalarInvariantComboBox);

  this->ComboBox = new QComboBox(q);
  QHBoxLayout* layout = new QHBoxLayout(q);
  layout->addWidget(this->ComboBox);
  layout->setContentsMargins(0,0,0,0);
  q->setLayout(layout);
  q->setSizePolicy(this->ComboBox->sizePolicy());

  this->populateComboBox();
  QObject::connect(this->ComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onCurrentScalarInvariantChanged(int)));
}

//------------------------------------------------------------------------------
void qDMMLScalarInvariantComboBoxPrivate::populateComboBox()
{
  this->ComboBox->clear();
  for (int i = 0;
        i < vtkDMMLDiffusionTensorVolumeDisplayNode::GetNumberOfScalarInvariants();
       ++i)
    {
    const int scalarInvariant = vtkDMMLDiffusionTensorVolumeDisplayNode::GetNthScalarInvariant(i);
    this->ComboBox->addItem(
      vtkDMMLDiffusionTensorDisplayPropertiesNode::GetScalarEnumAsString(scalarInvariant),
      QVariant(scalarInvariant));
    }

}

//------------------------------------------------------------------------------
void qDMMLScalarInvariantComboBoxPrivate::setScalarInvariantToComboBox(int scalarInvariant)
{ // The combobox has been populated on the assumption that all the scalar
  // invariant were comprised between GetFirstScalarInvariant() and
  // GetLastScalarInvariant().
  Q_ASSERT(scalarInvariant >=
           vtkDMMLDiffusionTensorDisplayPropertiesNode::GetFirstScalarInvariant());
  Q_ASSERT(scalarInvariant <=
           vtkDMMLDiffusionTensorDisplayPropertiesNode::GetLastScalarInvariant());
  int index = this->ComboBox->findData(QVariant(scalarInvariant));
  Q_ASSERT(index >= 0);
  this->ComboBox->setCurrentIndex(index);
}

//------------------------------------------------------------------------------
qDMMLScalarInvariantComboBox::qDMMLScalarInvariantComboBox(QWidget* parentWidget)
  : QWidget(parentWidget)
  , d_ptr(new qDMMLScalarInvariantComboBoxPrivate(*this))
{
  Q_D(qDMMLScalarInvariantComboBox);
  d->init();
}

//------------------------------------------------------------------------------
qDMMLScalarInvariantComboBox::~qDMMLScalarInvariantComboBox() = default;

//------------------------------------------------------------------------------
vtkDMMLDiffusionTensorDisplayPropertiesNode* qDMMLScalarInvariantComboBox::displayPropertiesNode()const
{
  Q_D(const qDMMLScalarInvariantComboBox);
  return d->DisplayPropertiesNode;
}

//------------------------------------------------------------------------------
void qDMMLScalarInvariantComboBox::setDisplayPropertiesNode(vtkDMMLNode* node)
{
  this->setDisplayPropertiesNode(
    vtkDMMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qDMMLScalarInvariantComboBox::setDisplayPropertiesNode(
  vtkDMMLDiffusionTensorDisplayPropertiesNode* displayNode)
{
  Q_D(qDMMLScalarInvariantComboBox);
  qvtkReconnect(d->DisplayPropertiesNode, displayNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromDMML()));
  d->DisplayPropertiesNode = displayNode;
  this->updateWidgetFromDMML();
}

//------------------------------------------------------------------------------
void qDMMLScalarInvariantComboBox::updateWidgetFromDMML()
{
  Q_D(qDMMLScalarInvariantComboBox);
  if (!d->DisplayPropertiesNode)
    {
    return;
    }
  d->setScalarInvariantToComboBox(d->DisplayPropertiesNode->GetColorGlyphBy());
}

//------------------------------------------------------------------------------
void qDMMLScalarInvariantComboBox::onCurrentScalarInvariantChanged(int index)
{
  Q_D(qDMMLScalarInvariantComboBox);
  int scalarInvariant = d->ComboBox->itemData(index).toInt();
  this->setScalarInvariant(scalarInvariant);
  emit scalarInvariantChanged(scalarInvariant);
}

//------------------------------------------------------------------------------
int qDMMLScalarInvariantComboBox::scalarInvariant()const
{
  Q_D(const qDMMLScalarInvariantComboBox);
  QVariant scalarInvariant = d->ComboBox->itemData(d->ComboBox->currentIndex());
  return scalarInvariant.isValid() ? scalarInvariant.toInt() : -1;
}

//------------------------------------------------------------------------------
void qDMMLScalarInvariantComboBox::setScalarInvariant(int value)
{
  Q_D(qDMMLScalarInvariantComboBox);
  if (!d->DisplayPropertiesNode)
    {
    d->setScalarInvariantToComboBox(value);
    }
  else
    {
    // SetColorGlyphBy will eventually call updateWidgetFromDMML
    d->DisplayPropertiesNode->SetColorGlyphBy(value);
    }
}
