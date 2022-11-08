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

#include "qDMMLDisplayNodeWidgetPlugin.h"
#include "qDMMLDisplayNodeWidget.h"

//------------------------------------------------------------------------------
qDMMLDisplayNodeWidgetPlugin::qDMMLDisplayNodeWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

//------------------------------------------------------------------------------
QWidget *qDMMLDisplayNodeWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLDisplayNodeWidget* _widget = new qDMMLDisplayNodeWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qDMMLDisplayNodeWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLDisplayNodeWidget\" \
          name=\"DMMLDisplayNodeWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QIcon qDMMLDisplayNodeWidgetPlugin::icon() const
{
  return QIcon(":/Icons/combobox.png");
}

//------------------------------------------------------------------------------
QString qDMMLDisplayNodeWidgetPlugin::includeFile() const
{
  return "qDMMLDisplayNodeWidget.h";
}

//------------------------------------------------------------------------------
bool qDMMLDisplayNodeWidgetPlugin::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qDMMLDisplayNodeWidgetPlugin::name() const
{
  return "qDMMLDisplayNodeWidget";
}
