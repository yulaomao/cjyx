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

#ifndef __qCjyxSequenceBrowserModuleWidgetsPlugin_h
#define __qCjyxSequenceBrowserModuleWidgetsPlugin_h

// Qt includes
#include "vtkCjyxConfigure.h" // For Cjyx_HAVE_QT5
#ifdef Cjyx_HAVE_QT5
#include <QtUiPlugin/QDesignerCustomWidgetCollectionInterface>
#else
#include <QDesignerCustomWidgetCollectionInterface>
#endif

// SequenceBrowser includes
#include "qDMMLSequenceBrowserPlayWidgetPlugin.h"
#include "qDMMLSequenceBrowserSeekWidgetPlugin.h"
#include "qDMMLSequenceBrowserToolBarPlugin.h"

// \class Group the plugins in one library
class Q_CJYX_MODULE_SEQUENCES_WIDGETS_PLUGINS_EXPORT qCjyxSequenceBrowserModuleWidgetsPlugin
  : public QObject
  , public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface")
#endif
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface);

public:
  QList<QDesignerCustomWidgetInterface*> customWidgets() const override
    {
    QList<QDesignerCustomWidgetInterface *> plugins;
    plugins << new qDMMLSequenceBrowserPlayWidgetPlugin;
    plugins << new qDMMLSequenceBrowserSeekWidgetPlugin;
    plugins << new qDMMLSequenceBrowserToolBarPlugin;
    return plugins;
    }
};

#endif
