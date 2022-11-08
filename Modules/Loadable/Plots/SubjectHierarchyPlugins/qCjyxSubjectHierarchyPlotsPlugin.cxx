/*==============================================================================

  Copyright (c) Kapteyn Astronomical Institute
  University of Groningen, Groningen, Netherlands. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Davide Punzo, Kapteyn Astronomical Institute,
  and was supported through the European Research Council grant nr. 291531.

==============================================================================*/

// SubjectHierarchy Plugins includes
#include "qCjyxSubjectHierarchyPluginHandler.h"
#include "qCjyxSubjectHierarchyPlotsPlugin.h"

// DMML includes
#include <vtkDMMLScene.h>
#include <vtkDMMLPlotChartNode.h>
#include <vtkDMMLLayoutNode.h>
#include <vtkDMMLPlotSeriesNode.h>
#include <vtkDMMLPlotViewNode.h>

#include <vtkCjyxPlotsLogic.h>
#include <qDMMLPlotWidget.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkCollection.h>

// Qt includes
#include <QDebug>
#include <QStandardItem>
#include <QAction>

// Cjyx includes
#include "qCjyxApplication.h"
#include "qCjyxLayoutManager.h"

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_SubjectHierarchy_Widgets
class qCjyxSubjectHierarchyPlotsPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qCjyxSubjectHierarchyPlotsPlugin);
protected:
  qCjyxSubjectHierarchyPlotsPlugin* const q_ptr;
public:
  qCjyxSubjectHierarchyPlotsPluginPrivate(qCjyxSubjectHierarchyPlotsPlugin& object);
  ~qCjyxSubjectHierarchyPlotsPluginPrivate() override;
  void init();
public:
  QIcon PlotChartIcon;
  QIcon PlotSeriesIcon;

  vtkWeakPointer<vtkCjyxPlotsLogic> PlotsLogic;

  QIcon VisibleIcon;
  QIcon HiddenIcon;
};

//-----------------------------------------------------------------------------
// qCjyxSubjectHierarchyPlotsPluginPrivate methods

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyPlotsPluginPrivate::qCjyxSubjectHierarchyPlotsPluginPrivate(qCjyxSubjectHierarchyPlotsPlugin& object)
: q_ptr(&object)
{
  this->PlotChartIcon = QIcon(":Icons/Medium/CjyxInteractivePlotting.png");
  this->PlotSeriesIcon = QIcon(":Icons/Medium/CjyxPlotSeries.png");

  this->VisibleIcon = QIcon(":Icons/VisibleOn.png");
  this->HiddenIcon = QIcon(":Icons/VisibleOff.png");
}

//------------------------------------------------------------------------------
void qCjyxSubjectHierarchyPlotsPluginPrivate::init()
{
}

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyPlotsPluginPrivate::~qCjyxSubjectHierarchyPlotsPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qCjyxSubjectHierarchyPlotsPlugin methods

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyPlotsPlugin::qCjyxSubjectHierarchyPlotsPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qCjyxSubjectHierarchyPlotsPluginPrivate(*this) )
{
  Q_D(qCjyxSubjectHierarchyPlotsPlugin);
  this->m_Name = QString("Plots");
  d->init();
}

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyPlotsPlugin::~qCjyxSubjectHierarchyPlotsPlugin() = default;

//-----------------------------------------------------------------------------
void qCjyxSubjectHierarchyPlotsPlugin::setPlotsLogic(vtkCjyxPlotsLogic* plotsLogic)
{
  Q_D(qCjyxSubjectHierarchyPlotsPlugin);
  d->PlotsLogic = plotsLogic;
}

//----------------------------------------------------------------------------
double qCjyxSubjectHierarchyPlotsPlugin::canAddNodeToSubjectHierarchy(
  vtkDMMLNode* node, vtkIdType parentItemID/*=vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID*/)const
{
  Q_UNUSED(parentItemID);
  if (!node)
    {
    qCritical() << Q_FUNC_INFO << ": Input node is nullptr!";
    return 0.0;
    }
  else if (node->IsA("vtkDMMLPlotChartNode"))
    {
    return 0.5;
    }
  else if (node->IsA("vtkDMMLPlotSeriesNode"))
    {
    return 0.5;
    }

  return 0.0;
}

//---------------------------------------------------------------------------
double qCjyxSubjectHierarchyPlotsPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID)const
{
  if (!itemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return 0.0;
    }
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return 0.0;
    }

  vtkDMMLNode* associatedNode = shNode->GetItemDataNode(itemID);
  if (associatedNode && associatedNode->IsA("vtkDMMLPlotChartNode"))
    {
    return 0.5; // There may be other plugins that can handle special PlotChart better
    }
  else if (associatedNode && associatedNode->IsA("vtkDMMLPlotSeriesNode"))
    {
    return 0.5; // There may be other plugins that can handle special PlotSeries better
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qCjyxSubjectHierarchyPlotsPlugin::roleForPlugin()const
{
  return "Plot";
}

//---------------------------------------------------------------------------
QIcon qCjyxSubjectHierarchyPlotsPlugin::icon(vtkIdType itemID)
{
  if (!itemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QIcon();
    }

  Q_D(qCjyxSubjectHierarchyPlotsPlugin);

  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return QIcon();
    }

  vtkDMMLNode* associatedNode = shNode->GetItemDataNode(itemID);
  if (associatedNode && associatedNode->IsA("vtkDMMLPlotChartNode"))
    {
    return d->PlotChartIcon;
    }
  else if (associatedNode && associatedNode->IsA("vtkDMMLPlotSeriesNode"))
    {
    return d->PlotSeriesIcon;
    }

  // Item unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qCjyxSubjectHierarchyPlotsPlugin::visibilityIcon(int visible)
{
  Q_D(qCjyxSubjectHierarchyPlotsPlugin);

  if (visible)
    {
    return d->VisibleIcon;
    }
  else
    {
    return d->HiddenIcon;
    }
}

//---------------------------------------------------------------------------
void qCjyxSubjectHierarchyPlotsPlugin::setDisplayVisibility(vtkIdType itemID, int visible)
{
  Q_D(qCjyxSubjectHierarchyPlotsPlugin);

  if (!itemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return;
    }
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkDMMLScene* scene = qCjyxSubjectHierarchyPluginHandler::instance()->dmmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid DMML scene!";
    return;
    }

  if (this->getDisplayVisibility(itemID) == visible)
    {
    return;
    }

  vtkDMMLPlotChartNode* plotChartNode = vtkDMMLPlotChartNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  vtkDMMLPlotSeriesNode* plotSeriesNode = vtkDMMLPlotSeriesNode::SafeDownCast(shNode->GetItemDataNode(itemID));

  if (!d->PlotsLogic)
    {
    qWarning() << Q_FUNC_INFO << ": plotsLogic is not set, cannot show plot in layout";
    return;
    }

  if (plotSeriesNode)
    {
    // A plot series node is selected.
    if (visible)
      {
      // Show series in current chart (if there is one), otherwise create a new chart
      vtkDMMLPlotChartNode* plotChartNode = nullptr;
      vtkDMMLPlotViewNode* plotViewNode = this->getPlotViewNode();
      if (plotViewNode)
        {
        plotChartNode = plotViewNode->GetPlotChartNode();
        }
      if (!plotChartNode)
        {
        plotChartNode = vtkDMMLPlotChartNode::SafeDownCast(scene->AddNewNodeByClass("vtkDMMLPlotChartNode"));
        }
      d->PlotsLogic->ShowChartInLayout(plotChartNode);
      if (!plotChartNode->HasPlotSeriesNodeID(plotSeriesNode->GetID()))
        {
        plotChartNode->AddAndObservePlotSeriesNodeID(plotSeriesNode->GetID());
        }
      }
    else
      {
      // Show series in current chart (if there is one), otherwise create a new chart
      vtkDMMLPlotViewNode* plotViewNode = this->getPlotViewNode();
      if (!plotViewNode)
        {
        // already hidden
        return;
        }
      vtkDMMLPlotChartNode* plotChartNode = plotViewNode->GetPlotChartNode();;
      if (!plotChartNode)
        {
        // already hidden
        return;
        }
      if (plotChartNode->HasPlotSeriesNodeID(plotSeriesNode->GetID()))
        {
        plotChartNode->RemovePlotSeriesNodeID(plotSeriesNode->GetID());
        }
      }
    }
  else if (plotChartNode)
    {
    vtkDMMLPlotChartNode* associatedPlotChartNode = vtkDMMLPlotChartNode::SafeDownCast(shNode->GetItemDataNode(itemID));
    d->PlotsLogic->ShowChartInLayout(visible ? associatedPlotChartNode : nullptr);
    }

  // Update icons of all charts (if we show this chart then we may have hidden other charts)
  if (scene->IsBatchProcessing())
    {
    return;
    }
  std::vector< vtkDMMLNode* > chartNodes;
  scene->GetNodesByClass("vtkDMMLPlotChartNode", chartNodes);
  for (std::vector< vtkDMMLNode* >::iterator chartIt = chartNodes.begin(); chartIt != chartNodes.end(); ++chartIt)
    {
    vtkDMMLPlotChartNode* chartNode = vtkDMMLPlotChartNode::SafeDownCast(*chartIt);
    vtkIdType chartNodeId = shNode->GetItemByDataNode(chartNode);
    shNode->ItemModified(chartNodeId);
    }
  std::vector< vtkDMMLNode* > seriesNodes;
  scene->GetNodesByClass("vtkDMMLPlotSeriesNode", seriesNodes);
  for (std::vector< vtkDMMLNode* >::iterator seriesIt = seriesNodes.begin(); seriesIt != seriesNodes.end(); ++seriesIt)
    {
    vtkDMMLPlotSeriesNode* seriesNode = vtkDMMLPlotSeriesNode::SafeDownCast(*seriesIt);
    vtkIdType seriesNodeId = shNode->GetItemByDataNode(seriesNode);
    shNode->ItemModified(seriesNodeId);
    }
}

//-----------------------------------------------------------------------------
int qCjyxSubjectHierarchyPlotsPlugin::getDisplayVisibility(vtkIdType itemID)const
{
  if (!itemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return -1;
    }
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return -1;
    }

  vtkDMMLPlotViewNode* plotViewNode = this->getPlotViewNode();
  if (!plotViewNode || !plotViewNode->GetPlotChartNode())
    {
    // No quantitative view has been set yet
    return 0;
    }

  vtkDMMLPlotSeriesNode* plotSeriesNode = vtkDMMLPlotSeriesNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (plotSeriesNode)
    {
    return (plotViewNode->GetPlotChartNode()->HasPlotSeriesNodeID(plotSeriesNode->GetID()) ? 1 : 0);
    }

  vtkDMMLPlotChartNode* plotChartNode = vtkDMMLPlotChartNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (plotChartNode)
    {
    // Return shown if plotLayout in plot view is the examined item's associated data node
    return (plotChartNode == plotViewNode->GetPlotChartNode()) ? 1 : 0;
    }

  return 0;
}

//---------------------------------------------------------------------------
vtkDMMLPlotViewNode* qCjyxSubjectHierarchyPlotsPlugin::getPlotViewNode()const
{
  vtkDMMLScene* scene = qCjyxSubjectHierarchyPluginHandler::instance()->dmmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid DMML scene!";
    return nullptr;
    }

  qDMMLLayoutManager* layoutManager = qCjyxApplication::application()->layoutManager();
  if (!layoutManager)
    {
    return nullptr;
    }

  for (int i=0; i<layoutManager->plotViewCount(); i++)
    {
    qDMMLPlotWidget* plotWidget = layoutManager->plotWidget(i);
    if (!plotWidget)
      {
      // invalid plot widget
      continue;
      }
    vtkDMMLPlotViewNode* plotView = plotWidget->dmmlPlotViewNode();
    if (plotView)
      {
      return plotView;
      }
    }

  // no valid plot view in current layout
  return nullptr;
}
