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

#include "qDMMLSequenceBrowserPlayWidgetPlugin.h"
#include "qDMMLSequenceBrowserPlayWidget.h"

//------------------------------------------------------------------------------
qDMMLSequenceBrowserPlayWidgetPlugin
::qDMMLSequenceBrowserPlayWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{

}

//------------------------------------------------------------------------------
QWidget *qDMMLSequenceBrowserPlayWidgetPlugin
::createWidget(QWidget *_parent)
{
  qDMMLSequenceBrowserPlayWidget* _widget
    = new qDMMLSequenceBrowserPlayWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qDMMLSequenceBrowserPlayWidgetPlugin
::domXml() const
{
  return "<widget class=\"qDMMLSequenceBrowserPlayWidget\" \
          name=\"SequenceBrowserPlayWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qDMMLSequenceBrowserPlayWidgetPlugin
::includeFile() const
{
  return "qDMMLSequenceBrowserPlayWidget.h";
}

//------------------------------------------------------------------------------
bool qDMMLSequenceBrowserPlayWidgetPlugin
::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qDMMLSequenceBrowserPlayWidgetPlugin
::name() const
{
  return "qDMMLSequenceBrowserPlayWidget";
}
