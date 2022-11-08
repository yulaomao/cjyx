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

#include "qDMMLCoordinatesWidgetPlugin.h"
#include "qDMMLCoordinatesWidget.h"

// --------------------------------------------------------------------------
qDMMLCoordinatesWidgetPlugin::qDMMLCoordinatesWidgetPlugin(QObject *_parent)
: QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qDMMLCoordinatesWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLCoordinatesWidget* _widget = new qDMMLCoordinatesWidget(_parent);
  return _widget;
}


// --------------------------------------------------------------------------
QString qDMMLCoordinatesWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLCoordinatesWidget\" \
                  name=\"DMMLCoordinatesWidget\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qDMMLCoordinatesWidgetPlugin::icon() const
{
  return QIcon(":/Icons/spinbox.png");
}

// --------------------------------------------------------------------------
QString qDMMLCoordinatesWidgetPlugin::includeFile() const
{
  return "qDMMLCoordinatesWidget.h";
}

// --------------------------------------------------------------------------
bool qDMMLCoordinatesWidgetPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qDMMLCoordinatesWidgetPlugin::name() const
{
  return "qDMMLCoordinatesWidget";
}
