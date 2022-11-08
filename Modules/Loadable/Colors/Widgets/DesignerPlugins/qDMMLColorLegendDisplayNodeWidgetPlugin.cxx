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

==============================================================================*/

#include "qDMMLColorLegendDisplayNodeWidgetPlugin.h"
#include "qDMMLColorLegendDisplayNodeWidget.h"

//-----------------------------------------------------------------------------
qDMMLColorLegendDisplayNodeWidgetPlugin::qDMMLColorLegendDisplayNodeWidgetPlugin(QObject *newParent)
  : QObject(newParent)
{
}

//-----------------------------------------------------------------------------
QString qDMMLColorLegendDisplayNodeWidgetPlugin::group() const
{
  return "Cjyx [DMML Widgets]";
}

//-----------------------------------------------------------------------------
QString qDMMLColorLegendDisplayNodeWidgetPlugin::toolTip() const
{
  return QString();
}

//-----------------------------------------------------------------------------
QString qDMMLColorLegendDisplayNodeWidgetPlugin::whatsThis() const
{
  return QString();
}

//-----------------------------------------------------------------------------
QWidget *qDMMLColorLegendDisplayNodeWidgetPlugin::createWidget(QWidget *newParent)
{
  qDMMLColorLegendDisplayNodeWidget* newWidget =
    new qDMMLColorLegendDisplayNodeWidget(newParent);
  return newWidget;
}

//-----------------------------------------------------------------------------
QString qDMMLColorLegendDisplayNodeWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLColorLegendDisplayNodeWidget\" \
          name=\"ColorLegendDisplayNodeWidget\">\n"
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
QIcon qDMMLColorLegendDisplayNodeWidgetPlugin::icon() const
{
  return QIcon(":/Icons/Colors.png");
}

//-----------------------------------------------------------------------------
QString qDMMLColorLegendDisplayNodeWidgetPlugin::includeFile() const
{
  return "qDMMLColorLegendDisplayNodeWidget.h";
}

//-----------------------------------------------------------------------------
bool qDMMLColorLegendDisplayNodeWidgetPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qDMMLColorLegendDisplayNodeWidgetPlugin::name() const
{
  return "qDMMLColorLegendDisplayNodeWidget";
}
