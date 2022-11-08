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

#ifndef __qCjyxVolumeRenderingModuleWidgetsPlugin_h
#define __qCjyxVolumeRenderingModuleWidgetsPlugin_h

// Qt includes
#include <QtUiPlugin/QDesignerCustomWidgetCollectionInterface>

// VolumeRendering includes
#include "qDMMLVolumePropertyNodeWidgetPlugin.h"
#include "qCjyxPresetComboBoxPlugin.h"
#include "qCjyxVolumeRenderingPresetComboBoxPlugin.h"
#include "qCjyxGPUMemoryComboBoxPlugin.h"

// \class Group the plugins in one library
class Q_CJYX_MODULE_VOLUMERENDERING_WIDGETS_PLUGINS_EXPORT qCjyxVolumeRenderingModuleWidgetsPlugin
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
    plugins << new qDMMLVolumePropertyNodeWidgetPlugin
            << new qCjyxPresetComboBoxPlugin
            << new qCjyxVolumeRenderingPresetComboBoxPlugin
            << new qCjyxGPUMemoryComboBoxPlugin;
    return plugins;
    }
};

#endif
