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

#ifndef __qDMMLWidgetsAbstractPlugin_h
#define __qDMMLWidgetsAbstractPlugin_h

#include "qDMMLWidgetsConfigure.h" // For DMML_WIDGETS_HAVE_QT5

// Qt includes
#ifdef DMML_WIDGETS_HAVE_QT5
#include <QtUiPlugin/QDesignerCustomWidgetInterface>
#else
#include <QDesignerCustomWidgetInterface>
#endif

#include "qDMMLWidgetsPluginsExport.h"

class QDMML_WIDGETS_PLUGINS_EXPORT qDMMLWidgetsAbstractPlugin : public QDesignerCustomWidgetInterface
{
#ifdef DMML_WIDGETS_HAVE_QT5
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface")
#endif
  Q_INTERFACES(QDesignerCustomWidgetInterface);
public:

  qDMMLWidgetsAbstractPlugin();
  // Don't reimplement this method.
  QString group() const override;
  // You can reimplement these methods
  QIcon icon() const override;
  QString toolTip() const override;
  QString whatsThis() const override;

};

#endif
