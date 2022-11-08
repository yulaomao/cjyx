/*==============================================================================

  Copyright (c) Kapteyn Astronomical Institute
  University of Groningen, Groningen, Netherlands. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Davide Punzo, Kapteyn Astronomical Institute,
  and was supported through the European Research Council grant nr. 291531.

==============================================================================*/

// qDMML includes
#include "qDMMLPlotWidgetPlugin.h"
#include "qDMMLPlotWidget.h"

//-----------------------------------------------------------------------------
qDMMLPlotWidgetPlugin::qDMMLPlotWidgetPlugin(QObject *_parent):QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qDMMLPlotWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLPlotWidget* _widget = new qDMMLPlotWidget(_parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qDMMLPlotWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLPlotWidget\" \
          name=\"DMMLPlotViewWidget\">\n"
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
QString qDMMLPlotWidgetPlugin::includeFile() const
{
  return "qDMMLPlotWidget.h";
}

//-----------------------------------------------------------------------------
bool qDMMLPlotWidgetPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qDMMLPlotWidgetPlugin::name() const
{
  return "qDMMLPlotWidget";
}
