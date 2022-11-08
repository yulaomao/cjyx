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

#include "qDMMLSequenceBrowserSeekWidgetPlugin.h"
#include "qDMMLSequenceBrowserSeekWidget.h"

//------------------------------------------------------------------------------
qDMMLSequenceBrowserSeekWidgetPlugin
::qDMMLSequenceBrowserSeekWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{

}

//------------------------------------------------------------------------------
QWidget *qDMMLSequenceBrowserSeekWidgetPlugin
::createWidget(QWidget *_parent)
{
  qDMMLSequenceBrowserSeekWidget* _widget
    = new qDMMLSequenceBrowserSeekWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qDMMLSequenceBrowserSeekWidgetPlugin
::domXml() const
{
  return "<widget class=\"qDMMLSequenceBrowserSeekWidget\" \
          name=\"SequenceBrowserSeekWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qDMMLSequenceBrowserSeekWidgetPlugin
::includeFile() const
{
  return "qDMMLSequenceBrowserSeekWidget.h";
}

//------------------------------------------------------------------------------
bool qDMMLSequenceBrowserSeekWidgetPlugin
::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qDMMLSequenceBrowserSeekWidgetPlugin
::name() const
{
  return "qDMMLSequenceBrowserSeekWidget";
}
