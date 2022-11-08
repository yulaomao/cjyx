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

#include "qDMMLRangeWidgetPlugin.h"
#include "qDMMLRangeWidget.h"

qDMMLRangeWidgetPlugin::qDMMLRangeWidgetPlugin(QObject *parentWidget)
        : QObject(parentWidget)
{
}

QWidget *qDMMLRangeWidgetPlugin::createWidget(QWidget *parentWidget)
{
  qDMMLRangeWidget* newWidget = new qDMMLRangeWidget(parentWidget);
  return newWidget;
}

QString qDMMLRangeWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLRangeWidget\" \
          name=\"DMMLRangeWidget\">\n"
          "</widget>\n";
}

QIcon qDMMLRangeWidgetPlugin::icon() const
{
  return QIcon(":/Icons/sliderspinbox.png");
}

QString qDMMLRangeWidgetPlugin::includeFile() const
{
  return "qDMMLRangeWidget.h";
}

bool qDMMLRangeWidgetPlugin::isContainer() const
{
  return false;
}

QString qDMMLRangeWidgetPlugin::name() const
{
  return "qDMMLRangeWidget";
}
