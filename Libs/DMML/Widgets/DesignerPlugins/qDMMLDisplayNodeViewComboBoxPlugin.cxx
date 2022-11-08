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

#include "qDMMLDisplayNodeViewComboBoxPlugin.h"
#include "qDMMLDisplayNodeViewComboBox.h"

//-----------------------------------------------------------------------------
qDMMLDisplayNodeViewComboBoxPlugin
::qDMMLDisplayNodeViewComboBoxPlugin(QObject* pluginParent)
  : QObject(pluginParent)
{
}

//-----------------------------------------------------------------------------
QWidget *qDMMLDisplayNodeViewComboBoxPlugin::createWidget(QWidget *parentWidget)
{
  qDMMLDisplayNodeViewComboBox* widget =
    new qDMMLDisplayNodeViewComboBox(parentWidget);
  return widget;
}

//-----------------------------------------------------------------------------
QString qDMMLDisplayNodeViewComboBoxPlugin::domXml() const
{
  return "<widget class=\"qDMMLDisplayNodeViewComboBox\" \
          name=\"DisplayNodeViewComboBox\">\n"
          " <property name=\"geometry\">\n"
          "  <rect>\n"
          "   <x>0</x>\n"
          "   <y>0</y>\n"
          "   <width>200</width>\n"
          "   <height>20</height>\n"
          "  </rect>\n"
          " </property>\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QIcon qDMMLDisplayNodeViewComboBoxPlugin::icon() const
{
  return QIcon(":/Icons/combobox.png");
}

//-----------------------------------------------------------------------------
QString qDMMLDisplayNodeViewComboBoxPlugin::includeFile() const
{
  return "qDMMLDisplayNodeViewComboBox.h";
}

//-----------------------------------------------------------------------------
bool qDMMLDisplayNodeViewComboBoxPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qDMMLDisplayNodeViewComboBoxPlugin::name() const
{
  return "qDMMLDisplayNodeViewComboBox";
}
