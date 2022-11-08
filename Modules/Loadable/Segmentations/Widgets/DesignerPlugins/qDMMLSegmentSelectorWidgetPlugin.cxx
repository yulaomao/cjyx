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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#include "qDMMLSegmentSelectorWidgetPlugin.h"
#include "qDMMLSegmentSelectorWidget.h"

//-----------------------------------------------------------------------------
qDMMLSegmentSelectorWidgetPlugin::qDMMLSegmentSelectorWidgetPlugin(QObject* pluginParent)
  : QObject(pluginParent)
{
}

//-----------------------------------------------------------------------------
QWidget *qDMMLSegmentSelectorWidgetPlugin::createWidget(QWidget* parentWidget)
{
  qDMMLSegmentSelectorWidget* pluginWidget =
    new qDMMLSegmentSelectorWidget(parentWidget);
  return pluginWidget;
}

//-----------------------------------------------------------------------------
QString qDMMLSegmentSelectorWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLSegmentSelectorWidget\" \
          name=\"SegmentSelectorWidget\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qDMMLSegmentSelectorWidgetPlugin::includeFile() const
{
  return "qDMMLSegmentSelectorWidget.h";
}

//-----------------------------------------------------------------------------
bool qDMMLSegmentSelectorWidgetPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qDMMLSegmentSelectorWidgetPlugin::name() const
{
  return "qDMMLSegmentSelectorWidget";
}
