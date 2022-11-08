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

#ifndef __qDMMLModelDisplayNodeWidgetPlugin_h
#define __qDMMLModelDisplayNodeWidgetPlugin_h

#include "qCjyxModelsWidgetsAbstractPlugin.h"

class Q_CJYX_QTMODULES_MODELS_WIDGETS_PLUGINS_EXPORT qDMMLModelDisplayNodeWidgetPlugin
  : public QObject
  , public qCjyxModelsWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qDMMLModelDisplayNodeWidgetPlugin(QObject * newParent = nullptr);

  QWidget *createWidget(QWidget *newParent) override;
  QString  domXml() const override;
  QIcon    icon() const override;
  QString  includeFile() const override;
  bool     isContainer() const override;
  QString  name() const override;

};

#endif
