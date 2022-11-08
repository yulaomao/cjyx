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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __qDMMLNodeAttributeTableWidgetPlugin_h
#define __qDMMLNodeAttributeTableWidgetPlugin_h

#include "qDMMLWidgetsAbstractPlugin.h"

class QDMML_WIDGETS_PLUGINS_EXPORT qDMMLNodeAttributeTableWidgetPlugin
    : public QObject, public qDMMLWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qDMMLNodeAttributeTableWidgetPlugin(QObject *_parent = nullptr);

  QWidget *createWidget(QWidget *_parent) override;
  QString  domXml() const override;
  QString  includeFile() const override;
  bool     isContainer() const override;
  QString  name() const override;

};

#endif
