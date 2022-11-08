/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

==============================================================================*/

#include "qDMMLTransformInfoWidgetPlugin.h"
#include "qDMMLTransformInfoWidget.h"

//------------------------------------------------------------------------------
qDMMLTransformInfoWidgetPlugin::qDMMLTransformInfoWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{
}

//------------------------------------------------------------------------------
QWidget *qDMMLTransformInfoWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLTransformInfoWidget* _widget = new qDMMLTransformInfoWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qDMMLTransformInfoWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLTransformInfoWidget\" \
          name=\"DMMLTransformInfoWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qDMMLTransformInfoWidgetPlugin::includeFile() const
{
  return "qDMMLTransformInfoWidget.h";
}

//------------------------------------------------------------------------------
bool qDMMLTransformInfoWidgetPlugin::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qDMMLTransformInfoWidgetPlugin::name() const
{
  return "qDMMLTransformInfoWidget";
}
