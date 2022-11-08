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

#include "qDMMLScalarInvariantComboBoxPlugin.h"
#include "qDMMLScalarInvariantComboBox.h"

//------------------------------------------------------------------------------
qDMMLScalarInvariantComboBoxPlugin
::qDMMLScalarInvariantComboBoxPlugin(QObject* parentObject)
  : QObject(parentObject)
{
}

//------------------------------------------------------------------------------
QWidget *qDMMLScalarInvariantComboBoxPlugin::createWidget(QWidget* parentWidget)
{
  qDMMLScalarInvariantComboBox* newWidget = new qDMMLScalarInvariantComboBox(parentWidget);
  return newWidget;
}

//------------------------------------------------------------------------------
QString qDMMLScalarInvariantComboBoxPlugin::domXml() const
{
  return "<widget class=\"qDMMLScalarInvariantComboBox\" \
          name=\"DMMLScalarInvariantComboBox\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QIcon qDMMLScalarInvariantComboBoxPlugin::icon() const
{
  return QIcon(":/Icons/combobox.png");
}

//------------------------------------------------------------------------------
QString qDMMLScalarInvariantComboBoxPlugin::includeFile() const
{
  return "qDMMLScalarInvariantComboBox.h";
}

//------------------------------------------------------------------------------
bool qDMMLScalarInvariantComboBoxPlugin::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qDMMLScalarInvariantComboBoxPlugin::name() const
{
  return "qDMMLScalarInvariantComboBox";
}
