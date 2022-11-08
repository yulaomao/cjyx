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

#include "qDMMLCaptureToolBarPlugin.h"
#include "qDMMLCaptureToolBar.h"

// --------------------------------------------------------------------------
qDMMLCaptureToolBarPlugin::qDMMLCaptureToolBarPlugin(QObject *_parent)
        : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qDMMLCaptureToolBarPlugin::createWidget(QWidget *_parent)
{
  qDMMLCaptureToolBar* _widget = new qDMMLCaptureToolBar(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qDMMLCaptureToolBarPlugin::domXml() const
{
  return "<widget class=\"qDMMLCaptureToolBar\" \
          name=\"DMMLCaptureToolBar\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qDMMLCaptureToolBarPlugin::icon() const
{
  return QIcon();
}

// --------------------------------------------------------------------------
QString qDMMLCaptureToolBarPlugin::includeFile() const
{
  return "qDMMLCaptureToolBar.h";
}

// --------------------------------------------------------------------------
bool qDMMLCaptureToolBarPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qDMMLCaptureToolBarPlugin::name() const
{
  return "qDMMLCaptureToolBar";
}

