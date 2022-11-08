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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#include "qDMMLNodeAttributeTableWidgetPlugin.h"
#include "qDMMLNodeAttributeTableWidget.h"

//-----------------------------------------------------------------------------
qDMMLNodeAttributeTableWidgetPlugin::qDMMLNodeAttributeTableWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qDMMLNodeAttributeTableWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLNodeAttributeTableWidget* _widget = new qDMMLNodeAttributeTableWidget(_parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qDMMLNodeAttributeTableWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLNodeAttributeTableWidget\" \
          name=\"DMMLNodeAttributeTableWidget\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qDMMLNodeAttributeTableWidgetPlugin::includeFile() const
{
  return "qDMMLNodeAttributeTableWidget.h";
}

//-----------------------------------------------------------------------------
bool qDMMLNodeAttributeTableWidgetPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qDMMLNodeAttributeTableWidgetPlugin::name() const
{
  return "qDMMLNodeAttributeTableWidget";
}
