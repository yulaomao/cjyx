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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qCjyxPlotsModuleWidgetsPlugin_h
#define __qCjyxPlotsModuleWidgetsPlugin_h

// Qt includes
#include <QtUiPlugin/QDesignerCustomWidgetCollectionInterface>

// Plots includes
#include "qDMMLPlotSeriesPropertiesWidgetPlugin.h"
#include "qDMMLPlotChartPropertiesWidgetPlugin.h"

// \class Group the plugins in one library
class Q_CJYX_MODULE_PLOTS_WIDGETS_PLUGINS_EXPORT qCjyxPlotsModuleWidgetsPlugin
  : public QObject
  , public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetCollectionInterface")
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface);

public:
  QList<QDesignerCustomWidgetInterface*> customWidgets() const override
    {
    QList<QDesignerCustomWidgetInterface *> plugins;
    plugins << new qDMMLPlotChartPropertiesWidgetPlugin;
    plugins << new qDMMLPlotSeriesPropertiesWidgetPlugin;

    return plugins;
    }
};

#endif
