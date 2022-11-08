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

#include "qDMMLSliderWidgetPlugin.h"
#include "qDMMLSliderWidget.h"

// --------------------------------------------------------------------------
qDMMLSliderWidgetPlugin::qDMMLSliderWidgetPlugin(QObject *_parent)
: QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qDMMLSliderWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLSliderWidget* _widget = new qDMMLSliderWidget(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qDMMLSliderWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLSliderWidget\" \
                  name=\"DMMLSliderWidget\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qDMMLSliderWidgetPlugin::icon() const
{
  return QIcon(":/Icons/sliderspinbox.png");
}

// --------------------------------------------------------------------------
QString qDMMLSliderWidgetPlugin::includeFile() const
{
  return "qDMMLSliderWidget.h";
}

// --------------------------------------------------------------------------
bool qDMMLSliderWidgetPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qDMMLSliderWidgetPlugin::name() const
{
  return "qDMMLSliderWidget";
}
