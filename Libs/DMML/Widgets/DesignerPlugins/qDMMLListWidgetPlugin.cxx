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

#include "qDMMLListWidgetPlugin.h"
#include "qDMMLListWidget.h"

qDMMLListWidgetPlugin::qDMMLListWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qDMMLListWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLListWidget* _widget = new qDMMLListWidget(_parent);
  return _widget;
}

QString qDMMLListWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLListWidget\" \
          name=\"DMMLListWidget\">\n"
          "</widget>\n";
}

QIcon qDMMLListWidgetPlugin::icon() const
{
  return QIcon(":Icons/listbox.png");
}

QString qDMMLListWidgetPlugin::includeFile() const
{
  return "qDMMLListWidget.h";
}

bool qDMMLListWidgetPlugin::isContainer() const
{
  return false;
}

QString qDMMLListWidgetPlugin::name() const
{
  return "qDMMLListWidget";
}
