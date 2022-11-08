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

#include "qDMMLPlotViewControllerWidgetPlugin.h"
#include "qDMMLPlotViewControllerWidget.h"

// --------------------------------------------------------------------------
qDMMLPlotViewControllerWidgetPlugin::qDMMLPlotViewControllerWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qDMMLPlotViewControllerWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLPlotViewControllerWidget* _widget = new qDMMLPlotViewControllerWidget(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qDMMLPlotViewControllerWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLPlotViewControllerWidget\" \
          name=\"DMMLPlotViewControllerWidget\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qDMMLPlotViewControllerWidgetPlugin::icon() const
{
  return QIcon();
}

// --------------------------------------------------------------------------
QString qDMMLPlotViewControllerWidgetPlugin::includeFile() const
{
  return "qDMMLPlotViewControllerWidget.h";
}

// --------------------------------------------------------------------------
bool qDMMLPlotViewControllerWidgetPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qDMMLPlotViewControllerWidgetPlugin::name() const
{
  return "qDMMLPlotViewControllerWidget";
}
