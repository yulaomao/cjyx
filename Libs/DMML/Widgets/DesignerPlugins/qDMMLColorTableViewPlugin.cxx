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

#include "qDMMLColorTableViewPlugin.h"
#include "qDMMLColorTableView.h"

qDMMLColorTableViewPlugin::qDMMLColorTableViewPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qDMMLColorTableViewPlugin::createWidget(QWidget *_parent)
{
  qDMMLColorTableView* _widget = new qDMMLColorTableView(_parent);
  return _widget;
}

QString qDMMLColorTableViewPlugin::domXml() const
{
  return "<widget class=\"qDMMLColorTableView\" \
          name=\"DMMLColorTableView\">\n"
          "</widget>\n";
}

QIcon qDMMLColorTableViewPlugin::icon() const
{
  return QIcon(":Icons/table.png");
}

QString qDMMLColorTableViewPlugin::includeFile() const
{
  return "qDMMLColorTableView.h";
}

bool qDMMLColorTableViewPlugin::isContainer() const
{
  return false;
}

QString qDMMLColorTableViewPlugin::name() const
{
  return "qDMMLColorTableView";
}
