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

#include "qDMMLNodeComboBoxPlugin.h"
#include "qDMMLNodeComboBox.h"

qDMMLNodeComboBoxPlugin::qDMMLNodeComboBoxPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qDMMLNodeComboBoxPlugin::createWidget(QWidget *_parent)
{
  qDMMLNodeComboBox* _widget = new qDMMLNodeComboBox(_parent);
  return _widget;
}

QString qDMMLNodeComboBoxPlugin::domXml() const
{
  return "<widget class=\"qDMMLNodeComboBox\" \
                  name=\"DMMLNodeComboBox\">\n"
          "</widget>\n";
}

QIcon qDMMLNodeComboBoxPlugin::icon() const
{
  return QIcon(":/Icons/combobox.png");
}

QString qDMMLNodeComboBoxPlugin::includeFile() const
{
  return "qDMMLNodeComboBox.h";
}

bool qDMMLNodeComboBoxPlugin::isContainer() const
{
  return false;
}

QString qDMMLNodeComboBoxPlugin::name() const
{
  return "qDMMLNodeComboBox";
}
