/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Matthew Holden, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#include "qDMMLPlotChartPropertiesWidgetPlugin.h"
#include "qDMMLPlotChartPropertiesWidget.h"

//------------------------------------------------------------------------------
qDMMLPlotChartPropertiesWidgetPlugin
::qDMMLPlotChartPropertiesWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{

}

//------------------------------------------------------------------------------
QWidget *qDMMLPlotChartPropertiesWidgetPlugin
::createWidget(QWidget *_parent)
{
  qDMMLPlotChartPropertiesWidget* _widget
    = new qDMMLPlotChartPropertiesWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qDMMLPlotChartPropertiesWidgetPlugin
::domXml() const
{
  return "<widget class=\"qDMMLPlotChartPropertiesWidget\" \
          name=\"CjyxPlotChartPropertiesWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qDMMLPlotChartPropertiesWidgetPlugin
::includeFile() const
{
  return "qDMMLPlotChartPropertiesWidget.h";
}

//------------------------------------------------------------------------------
bool qDMMLPlotChartPropertiesWidgetPlugin
::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qDMMLPlotChartPropertiesWidgetPlugin
::name() const
{
  return "qDMMLPlotChartPropertiesWidget";
}
