/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Brigham and Women's Hospital

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#include "qDMMLMarkupsDisplayNodeWidgetPlugin.h"
#include "qDMMLMarkupsDisplayNodeWidget.h"

//------------------------------------------------------------------------------
qDMMLMarkupsDisplayNodeWidgetPlugin
::qDMMLMarkupsDisplayNodeWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{

}

//------------------------------------------------------------------------------
QWidget *qDMMLMarkupsDisplayNodeWidgetPlugin
::createWidget(QWidget *_parent)
{
  qDMMLMarkupsDisplayNodeWidget* _widget
    = new qDMMLMarkupsDisplayNodeWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qDMMLMarkupsDisplayNodeWidgetPlugin
::domXml() const
{
  return "<widget class=\"qDMMLMarkupsDisplayNodeWidget\" \
          name=\"MarkupsDisplayNodeWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qDMMLMarkupsDisplayNodeWidgetPlugin
::includeFile() const
{
  return "qDMMLMarkupsDisplayNodeWidget.h";
}

//------------------------------------------------------------------------------
bool qDMMLMarkupsDisplayNodeWidgetPlugin
::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qDMMLMarkupsDisplayNodeWidgetPlugin
::name() const
{
  return "qDMMLMarkupsDisplayNodeWidget";
}
