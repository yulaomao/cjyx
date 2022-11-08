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
#include "qDMMLTreeViewPlugin.h"
#include "qDMMLTreeView.h"

// --------------------------------------------------------------------------
qDMMLTreeViewPlugin::qDMMLTreeViewPlugin(QObject *_parent):QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qDMMLTreeViewPlugin::createWidget(QWidget *_parent)
{
  qDMMLTreeView* _widget = new qDMMLTreeView(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qDMMLTreeViewPlugin::domXml() const
{
  return "<widget class=\"qDMMLTreeView\" \
          name=\"DMMLTreeView\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qDMMLTreeViewPlugin::icon() const
{
  return QIcon(":/Icons/listview.png");
}

// --------------------------------------------------------------------------
QString qDMMLTreeViewPlugin::includeFile() const
{
  return "qDMMLTreeView.h";
}

// --------------------------------------------------------------------------
bool qDMMLTreeViewPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qDMMLTreeViewPlugin::name() const
{
  return "qDMMLTreeView";
}
