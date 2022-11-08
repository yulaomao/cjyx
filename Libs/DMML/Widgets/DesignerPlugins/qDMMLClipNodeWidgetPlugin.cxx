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

#include "qDMMLClipNodeWidgetPlugin.h"
#include "qDMMLClipNodeWidget.h"

//------------------------------------------------------------------------------
qDMMLClipNodeWidgetPlugin::qDMMLClipNodeWidgetPlugin(QObject* pluginParent)
  : QObject(pluginParent)
{
}

//------------------------------------------------------------------------------
QWidget *qDMMLClipNodeWidgetPlugin::createWidget(QWidget* widgetParent)
{
  qDMMLClipNodeWidget* newWidget = new qDMMLClipNodeWidget(widgetParent);
  return newWidget;
}

//------------------------------------------------------------------------------
QString qDMMLClipNodeWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLClipNodeWidget\" \
          name=\"DMMLClipNodeWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QIcon qDMMLClipNodeWidgetPlugin::icon() const
{
  return QIcon(":/Icons/combobox.png");
}

//------------------------------------------------------------------------------
QString qDMMLClipNodeWidgetPlugin::includeFile() const
{
  return "qDMMLClipNodeWidget.h";
}

//------------------------------------------------------------------------------
bool qDMMLClipNodeWidgetPlugin::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qDMMLClipNodeWidgetPlugin::name() const
{
  return "qDMMLClipNodeWidget";
}
