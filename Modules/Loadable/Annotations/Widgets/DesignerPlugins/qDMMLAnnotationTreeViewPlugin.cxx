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

#include "qDMMLAnnotationTreeViewPlugin.h"
#include "qDMMLAnnotationTreeView.h"

//-----------------------------------------------------------------------------
qDMMLAnnotationTreeViewPlugin::qDMMLAnnotationTreeViewPlugin(QObject* pluginParent)
  : QObject(pluginParent)
{
}

//-----------------------------------------------------------------------------
QWidget *qDMMLAnnotationTreeViewPlugin::createWidget(QWidget* parentWidget)
{
  qDMMLAnnotationTreeView* pluginWidget
    = new qDMMLAnnotationTreeView(parentWidget);
  return pluginWidget;
}

//-----------------------------------------------------------------------------
QString qDMMLAnnotationTreeViewPlugin::domXml() const
{
  return "<widget class=\"qDMMLAnnotationTreeView\" \
          name=\"AnnotationTreeView\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qDMMLAnnotationTreeViewPlugin::includeFile() const
{
  return "qDMMLAnnotationTreeView.h";
}

//-----------------------------------------------------------------------------
bool qDMMLAnnotationTreeViewPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qDMMLAnnotationTreeViewPlugin::name() const
{
  return "qDMMLAnnotationTreeView";
}
