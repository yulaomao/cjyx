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

#include "qDMMLVolumePropertyNodeWidgetPlugin.h"
#include "qDMMLVolumePropertyNodeWidget.h"

//------------------------------------------------------------------------------
qDMMLVolumePropertyNodeWidgetPlugin
::qDMMLVolumePropertyNodeWidgetPlugin(QObject *parentObject)
  : QObject(parentObject)
{
}

//------------------------------------------------------------------------------
QWidget *qDMMLVolumePropertyNodeWidgetPlugin::createWidget(QWidget *parentWidget)
{
  qDMMLVolumePropertyNodeWidget* newWidget =
    new qDMMLVolumePropertyNodeWidget(parentWidget);
  return newWidget;
}

//------------------------------------------------------------------------------
QString qDMMLVolumePropertyNodeWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLVolumePropertyNodeWidget\" \
          name=\"DMMLVolumePropertyNodeWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QIcon qDMMLVolumePropertyNodeWidgetPlugin::icon() const
{
  return QIcon();
}

//------------------------------------------------------------------------------
QString qDMMLVolumePropertyNodeWidgetPlugin::includeFile() const
{
  return "qDMMLVolumePropertyNodeWidget.h";
}

//------------------------------------------------------------------------------
bool qDMMLVolumePropertyNodeWidgetPlugin::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qDMMLVolumePropertyNodeWidgetPlugin::name() const
{
  return "qDMMLVolumePropertyNodeWidget";
}
