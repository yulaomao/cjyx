/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

#include "qDMMLTableViewPlugin.h"
#include "qDMMLTableView.h"

//------------------------------------------------------------------------------
qDMMLTableViewPlugin::qDMMLTableViewPlugin(QObject *_parent)
: QObject(_parent)
{
}

//------------------------------------------------------------------------------
QWidget *qDMMLTableViewPlugin::createWidget(QWidget *_parent)
{
qDMMLTableView* _widget = new qDMMLTableView(_parent);
return _widget;
}

//------------------------------------------------------------------------------
QString qDMMLTableViewPlugin::domXml() const
{
return "<widget class=\"qDMMLTableView\" \
name=\"DMMLTableView\">\n"
"</widget>\n";
}

//------------------------------------------------------------------------------
QString qDMMLTableViewPlugin::includeFile() const
{
return "qDMMLTableView.h";
}

//------------------------------------------------------------------------------
bool qDMMLTableViewPlugin::isContainer() const
{
return false;
}

//------------------------------------------------------------------------------
QString qDMMLTableViewPlugin::name() const
{
return "qDMMLTableView";
}
