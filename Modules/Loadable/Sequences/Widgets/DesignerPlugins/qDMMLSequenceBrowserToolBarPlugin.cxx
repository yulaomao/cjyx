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

#include "qDMMLSequenceBrowserToolBarPlugin.h"
#include "qDMMLSequenceBrowserToolBar.h"

// --------------------------------------------------------------------------
qDMMLSequenceBrowserToolBarPlugin::qDMMLSequenceBrowserToolBarPlugin(QObject *_parent)
        : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qDMMLSequenceBrowserToolBarPlugin::createWidget(QWidget *_parent)
{
  qDMMLSequenceBrowserToolBar* _widget = new qDMMLSequenceBrowserToolBar(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qDMMLSequenceBrowserToolBarPlugin::domXml() const
{
  return "<widget class=\"qDMMLSequenceBrowserToolBar\" \
          name=\"DMMLSequenceBrowserToolBar\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qDMMLSequenceBrowserToolBarPlugin::icon() const
{
  return QIcon();
}

// --------------------------------------------------------------------------
QString qDMMLSequenceBrowserToolBarPlugin::includeFile() const
{
  return "qDMMLSequenceBrowserToolBar.h";
}

// --------------------------------------------------------------------------
bool qDMMLSequenceBrowserToolBarPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qDMMLSequenceBrowserToolBarPlugin::name() const
{
  return "qDMMLSequenceBrowserToolBar";
}
