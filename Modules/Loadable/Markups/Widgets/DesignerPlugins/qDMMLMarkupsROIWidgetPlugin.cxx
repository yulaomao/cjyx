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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

#include "qDMMLMarkupsROIWidgetPlugin.h"
#include "qDMMLMarkupsROIWidget.h"

//-----------------------------------------------------------------------------
qDMMLMarkupsROIWidgetPlugin::qDMMLMarkupsROIWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qDMMLMarkupsROIWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLMarkupsROIWidget* _widget = new qDMMLMarkupsROIWidget(_parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qDMMLMarkupsROIWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLMarkupsROIWidget\" \
          name=\"DMMLMarkupsROIWidget\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qDMMLMarkupsROIWidgetPlugin::includeFile() const
{
  return "qDMMLMarkupsROIWidget.h";
}

//-----------------------------------------------------------------------------
bool qDMMLMarkupsROIWidgetPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qDMMLMarkupsROIWidgetPlugin::name() const
{
  return "qDMMLMarkupsROIWidget";
}
