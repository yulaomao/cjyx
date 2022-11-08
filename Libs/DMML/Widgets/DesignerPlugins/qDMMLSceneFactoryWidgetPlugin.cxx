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

#include "qDMMLSceneFactoryWidgetPlugin.h"
#include "qDMMLSceneFactoryWidget.h"

// qDMML includes

// Qt includes

// --------------------------------------------------------------------------
qDMMLSceneFactoryWidgetPlugin::qDMMLSceneFactoryWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qDMMLSceneFactoryWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLSceneFactoryWidget* _widget = new qDMMLSceneFactoryWidget(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qDMMLSceneFactoryWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLSceneFactoryWidget\" \
          name=\"DMMLSceneFactoryWidget\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QString qDMMLSceneFactoryWidgetPlugin::includeFile() const
{
  return "qDMMLSceneFactoryWidgetPlugin.h";
}

// --------------------------------------------------------------------------
bool qDMMLSceneFactoryWidgetPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qDMMLSceneFactoryWidgetPlugin::name() const
{
  return "qDMMLSceneFactoryWidget";
}
