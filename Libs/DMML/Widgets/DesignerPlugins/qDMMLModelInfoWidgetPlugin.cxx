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

#include "qDMMLModelInfoWidgetPlugin.h"
#include "qDMMLModelInfoWidget.h"

//------------------------------------------------------------------------------
qDMMLModelInfoWidgetPlugin::qDMMLModelInfoWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{
}

//------------------------------------------------------------------------------
QWidget *qDMMLModelInfoWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLModelInfoWidget* _widget = new qDMMLModelInfoWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qDMMLModelInfoWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLModelInfoWidget\" \
          name=\"DMMLModelInfoWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QIcon qDMMLModelInfoWidgetPlugin::icon() const
{
  return QIcon(":/Icons/combobox.png");
}

//------------------------------------------------------------------------------
QString qDMMLModelInfoWidgetPlugin::includeFile() const
{
  return "qDMMLModelInfoWidget.h";
}

//------------------------------------------------------------------------------
bool qDMMLModelInfoWidgetPlugin::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qDMMLModelInfoWidgetPlugin::name() const
{
  return "qDMMLModelInfoWidget";
}
