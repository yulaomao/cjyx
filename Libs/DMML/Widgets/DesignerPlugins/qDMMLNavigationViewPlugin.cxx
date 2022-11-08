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

// qDMML includes
#include "qDMMLNavigationViewPlugin.h"
#include "qDMMLNavigationView.h"

//-----------------------------------------------------------------------------
qDMMLNavigationViewPlugin::qDMMLNavigationViewPlugin(QObject *_parent):QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qDMMLNavigationViewPlugin::createWidget(QWidget *_parent)
{
  qDMMLNavigationView* _widget = new qDMMLNavigationView(_parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qDMMLNavigationViewPlugin::domXml() const
{
  return "<widget class=\"qDMMLNavigationView\" \
          name=\"DMMLNavigationView\">\n"
          " <property name=\"geometry\">\n"
          "  <rect>\n"
          "   <x>0</x>\n"
          "   <y>0</y>\n"
          "   <width>200</width>\n"
          "   <height>200</height>\n"
          "  </rect>\n"
          " </property>\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qDMMLNavigationViewPlugin::includeFile() const
{
  return "qDMMLNavigationView.h";
}

//-----------------------------------------------------------------------------
bool qDMMLNavigationViewPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qDMMLNavigationViewPlugin::name() const
{
  return "qDMMLNavigationView";
}
