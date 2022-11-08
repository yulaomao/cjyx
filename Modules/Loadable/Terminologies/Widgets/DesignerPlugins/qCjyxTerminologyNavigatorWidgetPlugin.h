/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __qCjyxTerminologyNavigatorWidgetPlugin_h
#define __qCjyxTerminologyNavigatorWidgetPlugin_h

#include "qCjyxTerminologiesModuleWidgetsAbstractPlugin.h"

class Q_CJYX_MODULE_TERMINOLOGIES_WIDGETS_PLUGINS_EXPORT qCjyxTerminologyNavigatorWidgetPlugin
  : public QObject
  , public qCjyxTerminologiesModuleWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qCjyxTerminologyNavigatorWidgetPlugin(QObject* parent = nullptr);

  QWidget *createWidget(QWidget* parent) override;
  QString  domXml() const override;
  QString  includeFile() const override;
  bool     isContainer() const override;
  QString  name() const override;

};

#endif
