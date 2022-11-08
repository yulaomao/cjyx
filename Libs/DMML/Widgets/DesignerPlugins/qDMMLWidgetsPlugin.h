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

#ifndef __qDMMLWidgetsPlugin_h
#define __qDMMLWidgetsPlugin_h

#include "qDMMLWidgetsConfigure.h" // For DMML_WIDGETS_HAVE_QT5, DMML_WIDGETS_HAVE_WEBENGINE_SUPPORT

// Qt includes
#ifdef DMML_WIDGETS_HAVE_QT5
#include <QtUiPlugin/QDesignerCustomWidgetCollectionInterface>
#else
#include <QDesignerCustomWidgetCollectionInterface>
#endif

// DMMLWidgets includes
#include "qDMMLCheckableNodeComboBoxPlugin.h"
#include "qDMMLClipNodeWidgetPlugin.h"
#include "qDMMLCollapsibleButtonPlugin.h"
#include "qDMMLColorListViewPlugin.h"
#include "qDMMLColorTableComboBoxPlugin.h"
#include "qDMMLColorTableViewPlugin.h"
#include "qDMMLCoordinatesWidgetPlugin.h"
#include "qDMMLDisplayNodeViewComboBoxPlugin.h"
#include "qDMMLDisplayNodeWidgetPlugin.h"
#include "qDMMLEventBrokerWidgetPlugin.h"
#ifdef DMML_WIDGETS_HAVE_WEBENGINE_SUPPORT
#include "qDMMLExpandingWebViewPlugin.h"
#endif
#include "qDMMLLabelComboBoxPlugin.h"
#include "qDMMLLayoutWidgetPlugin.h"
#include "qDMMLLinearTransformSliderPlugin.h"
#include "qDMMLListWidgetPlugin.h"
#include "qDMMLMatrixWidgetPlugin.h"
#include "qDMMLModelInfoWidgetPlugin.h"
#include "qDMMLNavigationViewPlugin.h"
#include "qDMMLNodeAttributeTableViewPlugin.h"
#include "qDMMLNodeAttributeTableWidgetPlugin.h"
#include "qDMMLNodeComboBoxPlugin.h"
#include "qDMMLPlotWidgetPlugin.h"
#include "qDMMLPlotViewControllerWidgetPlugin.h"
#include "qDMMLRangeWidgetPlugin.h"
#include "qDMMLROIWidgetPlugin.h"
#include "qDMMLScalarInvariantComboBoxPlugin.h"
#include "qDMMLScalarsDisplayWidgetPlugin.h"
#include "qDMMLSliceControllerWidgetPlugin.h"
#include "qDMMLSliceInformationWidgetPlugin.h"
#include "qDMMLSliceWidgetPlugin.h"
#include "qDMMLSliderWidgetPlugin.h"
#include "qDMMLSpinBoxPlugin.h"
#include "qDMMLThreeDViewInformationWidgetPlugin.h"
#include "qDMMLThreeDViewPlugin.h"
#include "qDMMLTransformSlidersPlugin.h"
#include "qDMMLTreeViewPlugin.h"
#include "qDMMLVolumeInfoWidgetPlugin.h"
#include "qDMMLVolumeThresholdWidgetPlugin.h"
#include "qDMMLWidgetPlugin.h"
#include "qDMMLWindowLevelWidgetPlugin.h"
#include "qDMMLSceneFactoryWidgetPlugin.h"
#include "qDMMLTableViewPlugin.h"

// \class Group the plugins in one library
class QDMML_WIDGETS_PLUGINS_EXPORT qDMMLWidgetsPlugin
  : public QObject
  , public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
#ifdef DMML_WIDGETS_HAVE_QT5
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetCollectionInterface")
#endif
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface);

public:
  QList<QDesignerCustomWidgetInterface*> customWidgets() const override
    {
    QList<QDesignerCustomWidgetInterface *> plugins;
    plugins << new qDMMLCheckableNodeComboBoxPlugin
            << new qDMMLClipNodeWidgetPlugin
            << new qDMMLCollapsibleButtonPlugin
            << new qDMMLColorListViewPlugin
            << new qDMMLColorTableComboBoxPlugin
            << new qDMMLColorTableViewPlugin
            << new qDMMLColorTableViewPlugin
            << new qDMMLCoordinatesWidgetPlugin
            << new qDMMLDisplayNodeViewComboBoxPlugin
            << new qDMMLDisplayNodeWidgetPlugin
            << new qDMMLEventBrokerWidgetPlugin
#ifdef DMML_WIDGETS_HAVE_WEBENGINE_SUPPORT
            << new qDMMLExpandingWebViewPlugin
#endif
            << new qDMMLLabelComboBoxPlugin
            << new qDMMLLayoutWidgetPlugin
            << new qDMMLLinearTransformSliderPlugin
            << new qDMMLListWidgetPlugin
            << new qDMMLMatrixWidgetPlugin
            << new qDMMLModelInfoWidgetPlugin
            << new qDMMLNavigationViewPlugin
            << new qDMMLNodeAttributeTableViewPlugin
            << new qDMMLNodeAttributeTableWidgetPlugin
            << new qDMMLNodeComboBoxPlugin
            << new qDMMLPlotWidgetPlugin
            << new qDMMLPlotViewControllerWidgetPlugin
            << new qDMMLRangeWidgetPlugin
            << new qDMMLROIWidgetPlugin
            << new qDMMLScalarInvariantComboBoxPlugin
            << new qDMMLScalarsDisplayWidgetPlugin
            << new qDMMLSceneFactoryWidgetPlugin
            << new qDMMLSliceControllerWidgetPlugin
            << new qDMMLSliceInformationWidgetPlugin
            << new qDMMLSliceWidgetPlugin
            << new qDMMLSliderWidgetPlugin
            << new qDMMLSpinBoxPlugin
            << new qDMMLTableViewPlugin
            << new qDMMLThreeDViewInformationWidgetPlugin
            << new qDMMLThreeDViewPlugin
            << new qDMMLTransformSlidersPlugin
            << new qDMMLTreeViewPlugin
            << new qDMMLVolumeInfoWidgetPlugin
            << new qDMMLVolumeThresholdWidgetPlugin
            << new qDMMLWidgetPlugin
            << new qDMMLWindowLevelWidgetPlugin;
    return plugins;
    }
};

#endif
