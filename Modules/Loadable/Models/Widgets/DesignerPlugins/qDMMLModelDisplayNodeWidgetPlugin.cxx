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

#include "qDMMLModelDisplayNodeWidgetPlugin.h"
#include "qDMMLModelDisplayNodeWidget.h"

//-----------------------------------------------------------------------------
qDMMLModelDisplayNodeWidgetPlugin::
  qDMMLModelDisplayNodeWidgetPlugin(QObject *newParent) : QObject(newParent)
{
}

//-----------------------------------------------------------------------------
QWidget *qDMMLModelDisplayNodeWidgetPlugin::createWidget(QWidget *newParent)
{
  qDMMLModelDisplayNodeWidget* newWidget =
    new qDMMLModelDisplayNodeWidget(newParent);
  return newWidget;
}

//-----------------------------------------------------------------------------
QString qDMMLModelDisplayNodeWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLModelDisplayNodeWidget\" \
          name=\"EMSegmentInputChannelListWidget\">\n"
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

//-----------------------------------------------------------------------------
QIcon qDMMLModelDisplayNodeWidgetPlugin::icon() const
{
  return QIcon(":/Icons/table.png");
}

//-----------------------------------------------------------------------------
QString qDMMLModelDisplayNodeWidgetPlugin::includeFile() const
{
  return "qDMMLModelDisplayNodeWidget.h";
}

//-----------------------------------------------------------------------------
bool qDMMLModelDisplayNodeWidgetPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qDMMLModelDisplayNodeWidgetPlugin::name() const
{
  return "qDMMLModelDisplayNodeWidget";
}
