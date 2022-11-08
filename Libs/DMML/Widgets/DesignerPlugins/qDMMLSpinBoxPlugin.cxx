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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qDMMLSpinBoxPlugin.h"
#include "qDMMLSpinBox.h"

// --------------------------------------------------------------------------
qDMMLSpinBoxPlugin::qDMMLSpinBoxPlugin(QObject *_parent)
: QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qDMMLSpinBoxPlugin::createWidget(QWidget *_parent)
{
  qDMMLSpinBox* _widget = new qDMMLSpinBox(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qDMMLSpinBoxPlugin::domXml() const
{
  return "<widget class=\"qDMMLSpinBox\" \
                  name=\"DMMLSpinBox\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qDMMLSpinBoxPlugin::icon() const
{
  return QIcon(":/Icons/spinbox.png");
}

// --------------------------------------------------------------------------
QString qDMMLSpinBoxPlugin::includeFile() const
{
  return "qDMMLSpinBox.h";
}

// --------------------------------------------------------------------------
bool qDMMLSpinBoxPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qDMMLSpinBoxPlugin::name() const
{
  return "qDMMLSpinBox";
}
