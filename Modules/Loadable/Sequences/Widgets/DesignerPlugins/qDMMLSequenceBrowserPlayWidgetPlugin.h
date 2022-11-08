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

#ifndef __qDMMLSequenceBrowserPlayWidgetPlugin_h
#define __qDMMLSequenceBrowserPlayWidgetPlugin_h

#include "qCjyxSequencesModuleWidgetsAbstractPlugin.h"

class Q_CJYX_MODULE_SEQUENCES_WIDGETS_PLUGINS_EXPORT
qDMMLSequenceBrowserPlayWidgetPlugin
  : public QObject, public qCjyxSequencesModuleWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qDMMLSequenceBrowserPlayWidgetPlugin(QObject *_parent = 0);

  QWidget *createWidget(QWidget *_parent) override;
  QString domXml() const override;
  QString includeFile() const override;
  bool isContainer() const override;
  QString name() const override;

};

#endif
