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

#include "qDMMLMarkupsInteractionHandleWidgetPlugin.h"
#include "qDMMLMarkupsInteractionHandleWidget.h"

//-----------------------------------------------------------------------------
qDMMLMarkupsInteractionHandleWidgetPlugin::qDMMLMarkupsInteractionHandleWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qDMMLMarkupsInteractionHandleWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLMarkupsInteractionHandleWidget* _widget = new qDMMLMarkupsInteractionHandleWidget(_parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qDMMLMarkupsInteractionHandleWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLMarkupsInteractionHandleWidget\" \
          name=\"DMMLMarkupsInteractionHandleWidget\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qDMMLMarkupsInteractionHandleWidgetPlugin::includeFile() const
{
  return "qDMMLMarkupsInteractionHandleWidget.h";
}

//-----------------------------------------------------------------------------
bool qDMMLMarkupsInteractionHandleWidgetPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qDMMLMarkupsInteractionHandleWidgetPlugin::name() const
{
  return "qDMMLMarkupsInteractionHandleWidget";
}
