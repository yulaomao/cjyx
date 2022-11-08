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
#include "qDMMLCollapsibleButtonPlugin.h"
#include "qDMMLCollapsibleButton.h"

//-----------------------------------------------------------------------------
qDMMLCollapsibleButtonPlugin::qDMMLCollapsibleButtonPlugin(QObject *_parent):QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qDMMLCollapsibleButtonPlugin::createWidget(QWidget *_parent)
{
  qDMMLCollapsibleButton* _widget = new qDMMLCollapsibleButton(_parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qDMMLCollapsibleButtonPlugin::domXml() const
{
  return "<widget class=\"qDMMLCollapsibleButton\" \
          name=\"DMMLCollapsibleButton\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qDMMLCollapsibleButtonPlugin::includeFile() const
{
  return "qDMMLCollapsibleButton.h";
}

//-----------------------------------------------------------------------------
bool qDMMLCollapsibleButtonPlugin::isContainer() const
{
  return true;
}

//-----------------------------------------------------------------------------
QString qDMMLCollapsibleButtonPlugin::name() const
{
  return "qDMMLCollapsibleButton";
}
