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

#include "qDMMLNodeAttributeTableViewPlugin.h"
#include "qDMMLNodeAttributeTableView.h"

//-----------------------------------------------------------------------------
qDMMLNodeAttributeTableViewPlugin::qDMMLNodeAttributeTableViewPlugin(QObject *_parent)
  : QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qDMMLNodeAttributeTableViewPlugin::createWidget(QWidget *_parent)
{
  qDMMLNodeAttributeTableView* _widget = new qDMMLNodeAttributeTableView(_parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qDMMLNodeAttributeTableViewPlugin::domXml() const
{
  return "<widget class=\"qDMMLNodeAttributeTableView\" \
          name=\"DMMLNodeAttributeTableView\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qDMMLNodeAttributeTableViewPlugin::includeFile() const
{
  return "qDMMLNodeAttributeTableView.h";
}

//-----------------------------------------------------------------------------
bool qDMMLNodeAttributeTableViewPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qDMMLNodeAttributeTableViewPlugin::name() const
{
  return "qDMMLNodeAttributeTableView";
}
