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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qDMMLMatrixWidgetPlugin.h"
#include "qDMMLMatrixWidget.h"

qDMMLMatrixWidgetPlugin::qDMMLMatrixWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qDMMLMatrixWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLMatrixWidget* _widget = new qDMMLMatrixWidget(_parent);
  return _widget;
}

QString qDMMLMatrixWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLMatrixWidget\" \
          name=\"DMMLMatrixWidget\">\n"
          " <property name=\"geometry\">\n"
          "  <rect>\n"
          "   <x>0</x>\n"
          "   <y>0</y>\n"
          "   <width>200</width>\n"
          "   <height>200</height>\n"
          "  </rect>\n"
          " </property>\n"
          "</widget>\n";
}

QIcon qDMMLMatrixWidgetPlugin::icon() const
{
  return QIcon(":/Icons/table.png");
}

QString qDMMLMatrixWidgetPlugin::includeFile() const
{
  return "qDMMLMatrixWidget.h";
}

bool qDMMLMatrixWidgetPlugin::isContainer() const
{
  return false;
}

QString qDMMLMatrixWidgetPlugin::name() const
{
  return "qDMMLMatrixWidget";
}
