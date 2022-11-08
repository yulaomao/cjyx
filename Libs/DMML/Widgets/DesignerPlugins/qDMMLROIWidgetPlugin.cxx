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

#include "qDMMLROIWidgetPlugin.h"
#include "qDMMLROIWidget.h"

qDMMLROIWidgetPlugin::qDMMLROIWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qDMMLROIWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLROIWidget* _widget = new qDMMLROIWidget(_parent);
  return _widget;
}

QString qDMMLROIWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLROIWidget\" \
          name=\"DMMLROIWidget\">\n"
          "</widget>\n";
}

QIcon qDMMLROIWidgetPlugin::icon() const
{
  return QIcon(":/Icons/sliderspinbox.png");
}

QString qDMMLROIWidgetPlugin::includeFile() const
{
  return "qDMMLROIWidget.h";
}

bool qDMMLROIWidgetPlugin::isContainer() const
{
  return false;
}

QString qDMMLROIWidgetPlugin::name() const
{
  return "qDMMLROIWidget";
}
