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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// qDMML includes
#include "qDMMLWidgetPlugin.h"
#include "qDMMLWidget.h"

// --------------------------------------------------------------------------
qDMMLWidgetPlugin::qDMMLWidgetPlugin(QObject *_parent):QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qDMMLWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLWidget* _widget = new qDMMLWidget(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qDMMLWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLWidget\" \
          name=\"DMMLWidget\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qDMMLWidgetPlugin::icon() const
{
  return QIcon(":/Icons/widget.png");
}

// --------------------------------------------------------------------------
QString qDMMLWidgetPlugin::includeFile() const
{
  return "qDMMLWidget.h";
}

// --------------------------------------------------------------------------
bool qDMMLWidgetPlugin::isContainer() const
{
  return true;
}

// --------------------------------------------------------------------------
QString qDMMLWidgetPlugin::name() const
{
  return "qDMMLWidget";
}
