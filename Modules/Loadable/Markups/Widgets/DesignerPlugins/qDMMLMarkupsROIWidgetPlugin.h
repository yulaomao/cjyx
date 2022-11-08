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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

#ifndef __qDMMLMarkupsROIWidgetPlugin_h
#define __qDMMLMarkupsROIWidgetPlugin_h

#include "qCjyxMarkupsModuleWidgetsAbstractPlugin.h"

class Q_CJYX_MODULE_MARKUPS_WIDGETS_PLUGINS_EXPORT qDMMLMarkupsROIWidgetPlugin
    : public QObject, public qCjyxMarkupsModuleWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qDMMLMarkupsROIWidgetPlugin(QObject *_parent = nullptr);

  QWidget *createWidget(QWidget *_parent) override;
  QString  domXml() const override;
  QString  includeFile() const override;
  bool     isContainer() const override;
  QString  name() const override;

};

#endif
