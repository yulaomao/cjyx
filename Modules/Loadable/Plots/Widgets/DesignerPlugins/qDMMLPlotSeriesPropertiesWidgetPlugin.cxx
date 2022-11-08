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

#include "qDMMLPlotSeriesPropertiesWidgetPlugin.h"
#include "qDMMLPlotSeriesPropertiesWidget.h"

//------------------------------------------------------------------------------
qDMMLPlotSeriesPropertiesWidgetPlugin
::qDMMLPlotSeriesPropertiesWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{

}

//------------------------------------------------------------------------------
QWidget *qDMMLPlotSeriesPropertiesWidgetPlugin
::createWidget(QWidget *_parent)
{
  qDMMLPlotSeriesPropertiesWidget* _widget
    = new qDMMLPlotSeriesPropertiesWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qDMMLPlotSeriesPropertiesWidgetPlugin
::domXml() const
{
  return "<widget class=\"qDMMLPlotSeriesPropertiesWidget\" \
          name=\"CjyxPlotChartPropertiesWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qDMMLPlotSeriesPropertiesWidgetPlugin
::includeFile() const
{
  return "qDMMLPlotSeriesPropertiesWidget.h";
}

//------------------------------------------------------------------------------
bool qDMMLPlotSeriesPropertiesWidgetPlugin
::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qDMMLPlotSeriesPropertiesWidgetPlugin
::name() const
{
  return "qDMMLPlotSeriesPropertiesWidget";
}
