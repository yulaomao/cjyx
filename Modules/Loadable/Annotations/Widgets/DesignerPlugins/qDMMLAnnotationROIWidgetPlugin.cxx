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

#include "qDMMLAnnotationROIWidgetPlugin.h"
#include "qDMMLAnnotationROIWidget.h"

//-----------------------------------------------------------------------------
qDMMLAnnotationROIWidgetPlugin::qDMMLAnnotationROIWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qDMMLAnnotationROIWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLAnnotationROIWidget* _widget = new qDMMLAnnotationROIWidget(_parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qDMMLAnnotationROIWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLAnnotationROIWidget\" \
          name=\"DMMLAnnotationROIWidget\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qDMMLAnnotationROIWidgetPlugin::includeFile() const
{
  return "qDMMLAnnotationROIWidget.h";
}

//-----------------------------------------------------------------------------
bool qDMMLAnnotationROIWidgetPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qDMMLAnnotationROIWidgetPlugin::name() const
{
  return "qDMMLAnnotationROIWidget";
}
