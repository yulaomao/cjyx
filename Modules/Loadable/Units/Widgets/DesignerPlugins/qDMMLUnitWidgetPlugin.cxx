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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qDMMLUnitWidgetPlugin.h"
#include "qDMMLUnitWidget.h"

//------------------------------------------------------------------------------
qDMMLUnitWidgetPlugin::qDMMLUnitWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{
}

//------------------------------------------------------------------------------
QWidget *qDMMLUnitWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLUnitWidget* _widget = new qDMMLUnitWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qDMMLUnitWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLUnitWidget\" \
          name=\"DMMLUnitWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qDMMLUnitWidgetPlugin::includeFile() const
{
  return "qDMMLUnitWidget.h";
}

//------------------------------------------------------------------------------
bool qDMMLUnitWidgetPlugin::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qDMMLUnitWidgetPlugin::name() const
{
  return "qDMMLUnitWidget";
}
