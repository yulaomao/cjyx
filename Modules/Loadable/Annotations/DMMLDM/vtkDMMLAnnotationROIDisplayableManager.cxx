// Annotation Logic includes
#include <vtkCjyxAnnotationModuleLogic.h>

// Annotation DMML includes
#include <vtkDMMLAnnotationROINode.h>

// Annotation DisplayableManager includes
#include "vtkDMMLAnnotationClickCounter.h"
#include "vtkDMMLAnnotationDisplayableManagerHelper.h"
#include "vtkDMMLAnnotationROIDisplayableManager.h"

// Annotation VTKWidgets includes
#include <vtkAnnotationROIRepresentation2D.h>
#include <vtkAnnotationROIRepresentation.h>
#include <vtkAnnotationROIWidget2D.h>
#include <vtkAnnotationROIWidget.h>

// DMML includes
#include <vtkDMMLInteractionNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLTransformNode.h>

// VTK includes
#include <vtkAbstractWidget.h>
#include <vtkActor2D.h>
#include <vtkCubeSource.h>
#include <vtkCutter.h>
#include <vtkHandleRepresentation.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkObject.h>
#include <vtkPickingManager.h>
#include <vtkPlane.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
// for picking
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

// std includes
#include <string>
#include <math.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkDMMLAnnotationROIDisplayableManager);

//---------------------------------------------------------------------------
// vtkDMMLAnnotationROIDisplayableManager Callback
/// \ingroup Cjyx_QtModules_Annotation
class vtkAnnotationROIWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationROIWidgetCallback *New()
  { return new vtkAnnotationROIWidgetCallback; }

  vtkAnnotationROIWidgetCallback() = default;


  void Execute (vtkObject *vtkNotUsed(caller), unsigned long event, void*) override
  {
    if ((event == vtkCommand::EndInteractionEvent) || (event == vtkCommand::InteractionEvent))
      {

      // sanity checks
      if (!this->m_DisplayableManager)
        {
        return;
        }
      if (!this->m_Node)
        {
        return;
        }
      if (!this->m_Widget)
        {
        return;
        }
      // sanity checks end

      // the interaction with the widget ended, now propagate the changes to DMML
      vtkDMMLAnnotationROINode *roiNode = vtkDMMLAnnotationROINode::SafeDownCast(m_Node);
      if (roiNode->GetInteractiveMode() ||
           (!roiNode->GetInteractiveMode() && event == vtkCommand::EndInteractionEvent))
        {
        this->m_DisplayableManager->PropagateWidgetToDMML(this->m_Widget, this->m_Node);
        }
    }
  }

  void SetWidget(vtkAbstractWidget *w)
  {
    this->m_Widget = w;
  }
  void SetNode(vtkDMMLAnnotationNode *n)
  {
    this->m_Node = n;
  }
  void SetDisplayableManager(vtkDMMLAnnotationDisplayableManager * dm)
  {
    this->m_DisplayableManager = dm;
  }

  vtkAbstractWidget * m_Widget;
  vtkDMMLAnnotationNode * m_Node;
  vtkDMMLAnnotationDisplayableManager * m_DisplayableManager;
};

//---------------------------------------------------------------------------
// vtkDMMLAnnotationROIDisplayableManager methods

//---------------------------------------------------------------------------
vtkDMMLAnnotationROIDisplayableManager::~vtkDMMLAnnotationROIDisplayableManager() = default;
//---------------------------------------------------------------------------
void vtkDMMLAnnotationROIDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
/// Create a new text widget.
vtkAbstractWidget * vtkDMMLAnnotationROIDisplayableManager::CreateWidget(vtkDMMLAnnotationNode* node)
{

  if (!node)
    {
    vtkErrorMacro("CreateWidget: Node not set!");
    return nullptr;
    }

  vtkDMMLAnnotationROINode* roiNode = vtkDMMLAnnotationROINode::SafeDownCast(node);
  if (!roiNode)
    {
    vtkErrorMacro("CreateWidget: Could not get ROI node!");
    return nullptr;
    }

  vtkAnnotationROIWidget* roiWidget = nullptr;
  if (this->Is2DDisplayableManager())
    {
    roiWidget = vtkAnnotationROIWidget2D::New();
    }
  else
    {
    roiWidget = vtkAnnotationROIWidget::New();
    }

  if (this->GetInteractor()->GetPickingManager())
    {
    if (!(this->GetInteractor()->GetPickingManager()->GetEnabled()))
      {
      // if the picking manager is not already turned on for this
      // interactor, enable it
      this->GetInteractor()->GetPickingManager()->EnabledOn();
      }
    }

  roiWidget->SetInteractor(this->GetInteractor());
  roiWidget->SetRotationEnabled(0);
  roiWidget->CreateDefaultRepresentation();

  vtkAnnotationROIRepresentation *boxRepresentation =
      vtkAnnotationROIRepresentation::SafeDownCast(roiWidget->GetRepresentation());
  boxRepresentation->SetPlaceFactor(1.0);

  this->PropagateDMMLToWidget(node, roiWidget);

  if (this->Is2DDisplayableManager())
    {
    vtkNew<vtkPropCollection> actors;
    boxRepresentation->GetActors2D(actors.GetPointer());
    for (int i=0; i<actors->GetNumberOfItems(); i++)
      {
      this->GetRenderer()->AddActor2D(vtkProp::SafeDownCast(actors->GetItemAsObject(i)));
      }
    }
  else
    {
    roiWidget->SetCurrentRenderer(this->GetRenderer());

    boxRepresentation->NeedToRenderOn();

    }

  roiWidget->On();

  return roiWidget;

}

//---------------------------------------------------------------------------
/// Tear down the widget creation
void vtkDMMLAnnotationROIDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkDMMLAnnotationNode * node)
{

  if (!widget)
    {
    vtkErrorMacro("OnWidgetCreated: Widget was null!");
    return;
    }

  if (!node)
    {
    vtkErrorMacro("OnWidgetCreated: DMML node was null!");
    return;
    }

  vtkAnnotationROIWidgetCallback *myCallback = vtkAnnotationROIWidgetCallback::New();
  myCallback->SetNode(node);
  myCallback->SetWidget(widget);
  myCallback->SetDisplayableManager(this);
  widget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  widget->AddObserver(vtkCommand::InteractionEvent,myCallback);
  myCallback->Delete();

  node->SaveView();

}

void vtkDMMLAnnotationROIDisplayableManager::UpdatePosition(vtkAbstractWidget *widget, vtkDMMLNode *node)
{
  this->PropagateDMMLToWidget(vtkDMMLAnnotationNode::SafeDownCast(node), widget);
}


//---------------------------------------------------------------------------
void vtkDMMLAnnotationROIDisplayableManager::OnDMMLSceneNodeRemoved(vtkDMMLNode* node)
{

  vtkDMMLAnnotationROINode *annotationNode = vtkDMMLAnnotationROINode::SafeDownCast(node);
  if (!annotationNode)
    {
    return;
    }

  if (this->Is2DDisplayableManager())
    {
    vtkAnnotationROIWidget2D* roiWidget = vtkAnnotationROIWidget2D::SafeDownCast(this->Helper->GetWidget(annotationNode));
    vtkAnnotationROIRepresentation2D* rep = vtkAnnotationROIRepresentation2D::SafeDownCast(
      roiWidget ? roiWidget->GetRepresentation() : nullptr);

    if (rep)
      {
      // update actor's visbility from dmml
      vtkNew<vtkPropCollection> actors;
      rep->GetActors2D(actors.GetPointer());
      for (int i=0; i<actors->GetNumberOfItems(); i++)
        {
        this->GetRenderer()->RemoveActor2D(vtkProp::SafeDownCast(actors->GetItemAsObject(i)));
        }
      }
    }

  this->Superclass::OnDMMLSceneNodeRemoved(node);
}

//---------------------------------------------------------------------------
void vtkDMMLAnnotationROIDisplayableManager::OnDMMLSliceNodeModifiedEvent(vtkDMMLSliceNode* sliceNode)
{

  if (!sliceNode)
    {
    vtkErrorMacro("OnDMMLSliceNodeModifiedEvent: Could not get the sliceNode.");
    return;
    }

  // run through all associated nodes
  vtkDMMLAnnotationDisplayableManagerHelper::AnnotationNodeListIt it;
  it = this->Helper->AnnotationNodeList.begin();
  while(it != this->Helper->AnnotationNodeList.end())
    {

    // we loop through all nodes
    vtkDMMLAnnotationNode* annotationNode = *it;

    vtkAbstractWidget* widget = this->Helper->GetWidget(annotationNode);
    if (!widget)
      {
      vtkErrorMacro("OnDMMLSliceNodeModifiedEvent: We could not get the widget to the node: " << annotationNode->GetID());
      return;
      }
    this->PropagateDMMLToWidget(annotationNode, widget);
    it++;
    }
}

//---------------------------------------------------------------------------
/// Propagate properties of DMML node to widget.
void vtkDMMLAnnotationROIDisplayableManager::PropagateDMMLToWidget(vtkDMMLAnnotationNode* node, vtkAbstractWidget * widget)
{

  if (!widget)
    {
    vtkErrorMacro("PropagateDMMLToWidget: Widget was null!");
    return;
    }

  if (!node)
    {
    vtkErrorMacro("PropagateDMMLToWidget: DMML node was null!");
    return;
    }

  if (this->Is2DDisplayableManager())
    {
    this->PropagateDMMLToWidget2D(node, widget);
    return;
    }


  // cast to the specific widget
  vtkAnnotationROIWidget* roiWidget = vtkAnnotationROIWidget::SafeDownCast(widget);

  if (!roiWidget)
    {
    vtkErrorMacro("PropagateDMMLToWidget: Could not get box widget!");
    return;
    }

  // cast to the specific dmml node
  vtkDMMLAnnotationROINode* roiNode = vtkDMMLAnnotationROINode::SafeDownCast(node);

  if (!roiNode)
    {
    vtkErrorMacro("PropagateDMMLToWidget: Could not get ROI node!");
    return;
    }

  if (this->m_Updating)
    {
    vtkDebugMacro("PropagateDMMLToWidget: Updating in progress.. Exit now.");
    return;
    }

  // now get the widget properties (coordinates, measurement etc.) and if the dmml node has changed, propagate the changes
  vtkAnnotationROIRepresentation * rep = vtkAnnotationROIRepresentation::SafeDownCast(roiWidget->GetRepresentation());

  if (!rep)
    {
    vtkErrorMacro("PropagateDMMLToWidget: Could not get widget representation!");
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;


  // handle ROI transform to world space
  vtkNew<vtkMatrix4x4> transformToWorld;
  transformToWorld->Identity();

  vtkDMMLTransformNode* tnode = roiNode->GetParentTransformNode();
  if (tnode != nullptr && tnode->IsTransformToWorldLinear())
    {
    tnode->GetMatrixTransformToWorld(transformToWorld.GetPointer());
    }
  transformToWorld->Invert();

  rep->SetWorldToLocalMatrix(transformToWorld.GetPointer());

  // update widget from dmml
  double xyz[3];
  double rxyz[3];
  roiNode->GetXYZ(xyz);
  roiNode->GetRadiusXYZ(rxyz);

  double bounds[6];
  for (int i=0; i<3; i++)
    {
    bounds[  i] = xyz[i]-rxyz[i];
    bounds[3+i] = xyz[i]+rxyz[i];
    }
  double b[6];
  b[0] = bounds[0];
  b[1] = bounds[3];
  b[2] = bounds[1];
  b[3] = bounds[4];
  b[4] = bounds[2];
  b[5] = bounds[5];

  if (roiNode->GetLocked())
    {
    rep->HandlesOff();
    }
  else
    {
    rep->HandlesOn();
    }

  rep->PlaceWidget(b);

  this->SetParentTransformToWidget(roiNode, roiWidget);

  if (roiNode->GetDisplayVisibility())
    {
    widget->EnabledOn();
    }
  else
    {
    widget->EnabledOff();
    }


  // re-render the widget
  rep->NeedToRenderOn();
  roiWidget->Modified();

  // enable processing of modified events
  this->m_Updating = 0;

}

//---------------------------------------------------------------------------
/// Propagate properties of DMML node to widget.
void vtkDMMLAnnotationROIDisplayableManager::PropagateDMMLToWidget2D(vtkDMMLAnnotationNode* node, vtkAbstractWidget * widget)
{
  if (!this->GetRenderer() || !this->GetRenderer()->GetRenderWindow() || this->GetRenderer()->GetRenderWindow()->GetNeverRendered())
    {
    return;
    }

  vtkDMMLSliceNode *sliceNode = this->GetSliceNode();
  if (!sliceNode)
    {
    return;
    }

  // cast to the specific widget
  vtkAnnotationROIWidget2D* roiWidget = vtkAnnotationROIWidget2D::SafeDownCast(widget);

  if (!roiWidget)
    {
    // commented out by Fedorov: fails too often in ChangeTracker workflow
    //vtkErrorMacro("PropagateDMMLToWidget: Could not get box widget!");
    return;
    }

  // cast to the specific dmml node
  vtkDMMLAnnotationROINode* roiNode = vtkDMMLAnnotationROINode::SafeDownCast(node);

  if (!roiNode)
    {
    vtkErrorMacro("PropagateDMMLToWidget: Could not get ROI node!");
    return;
    }

  if (this->m_Updating)
    {
    vtkDebugMacro("PropagateDMMLToWidget: Updating in progress.. Exit now.");
    return;
    }


  // now get the widget properties (coordinates, measurement etc.) and if the dmml node has changed, propagate the changes
  vtkAnnotationROIRepresentation2D * rep = vtkAnnotationROIRepresentation2D::SafeDownCast(roiWidget->GetRepresentation());

  if (!rep)
    {
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;

  // update widget from dmml
  double xyz[3];
  double rxyz[3];
  roiNode->GetXYZ(xyz);
  roiNode->GetRadiusXYZ(rxyz);

  double bounds[6];
  for (int i=0; i<3; i++)
    {
    bounds[  i] = xyz[i]-rxyz[i];
    bounds[3+i] = xyz[i]+rxyz[i];
    }
  double b[6];
  b[0] = bounds[0];
  b[1] = bounds[3];
  b[2] = bounds[1];
  b[3] = bounds[4];
  b[4] = bounds[2];
  b[5] = bounds[5];

  //this->SetParentTransformToWidget(roiNode, roiWidget);

  // handle ROI transform to world space
  vtkNew<vtkMatrix4x4> transformToWorld;
  transformToWorld->Identity();

  vtkDMMLTransformNode* tnode = roiNode->GetParentTransformNode();
  if (tnode != nullptr && tnode->IsTransformToWorldLinear())
    {
    tnode->GetMatrixTransformToWorld(transformToWorld.GetPointer());
    }

  // update the transform from world to screen space
  // for the extracted cut plane
  vtkNew<vtkMatrix4x4> rasToXY;
  rasToXY->DeepCopy(sliceNode->GetXYToRAS());
  rasToXY->Invert();

  vtkNew<vtkMatrix4x4> XYToWorld;
  XYToWorld->Identity();
  XYToWorld->Multiply4x4(rasToXY.GetPointer(), transformToWorld.GetPointer(), XYToWorld.GetPointer());

  vtkSmartPointer<vtkTransform> transform = rep->GetIntersectionPlaneTransform();

  transform->SetMatrix(XYToWorld.GetPointer());

  //
  // update the plane equation for the current slice cutting plane
  // - extract from the slice matrix
  // - normalize the normal
  transformToWorld->Invert();
  rasToXY->DeepCopy(sliceNode->GetXYToRAS());

  vtkNew<vtkMatrix4x4> mat;
  mat->Identity();
  mat->Multiply4x4(transformToWorld.GetPointer(), rasToXY.GetPointer(), mat.GetPointer());
  rasToXY->DeepCopy(mat.GetPointer());

  double normal[4]={0,0,0,1};
  double origin[4]={0,0,0,1};
  double sum = 0;
  int i;

  for (i=0; i<3; i++)
    {
    normal[i] = rasToXY->GetElement(i, 2);
    origin[i] = rasToXY->GetElement(i, 3);
    }
  for (i=0; i<3; i++)
    {
    sum += normal[i]*normal[i];
    }

  double lenInv = 1./sqrt(sum);
  for (i=0; i<3; i++)
    {
    normal[i] = normal[i]*lenInv;
    }

  vtkPlane *plane = rep->GetIntersectionPlane();

  plane->SetNormal(normal);
  plane->SetOrigin(origin);

  rep->SetSliceIntersectionVisibility(roiNode->GetDisplayVisibility() ? 1:0);
  rep->SetHandlesVisibility(roiNode->GetLocked()==0 && roiNode->GetDisplayVisibility() ? 1:0);

  rep->PlaceWidget(b);

  // re-render the widget
  rep->NeedToRenderOn();

  roiWidget->Modified();

  // enable processing of modified events
  this->m_Updating = 0;



  return;
}

//---------------------------------------------------------------------------
/// Propagate properties of widget to DMML node.
void vtkDMMLAnnotationROIDisplayableManager::PropagateWidgetToDMML(vtkAbstractWidget * widget, vtkDMMLAnnotationNode* node)
{
  if (!widget)
    {
    vtkErrorMacro("PropagateWidgetToDMML: Widget was null!");
    return;
    }

  if (!node)
    {
    vtkErrorMacro("PropagateWidgetToDMML: DMML node was null!");
    return;
    }

  // cast to the specific widget
  vtkAnnotationROIWidget* roiWidget = vtkAnnotationROIWidget::SafeDownCast(widget);

  if (!roiWidget)
    {
    vtkErrorMacro("PropagateWidgetToDMML: Could not get box widget!");
    return;
    }

  // cast to the specific dmml node
  vtkDMMLAnnotationROINode* roiNode = vtkDMMLAnnotationROINode::SafeDownCast(node);

  if (!roiNode)
    {
    vtkErrorMacro("PropagateWidgetToDMML: Could not get ROI node!");
    return;
    }

  if (this->m_Updating)
    {
    vtkDebugMacro("PropagateWidgetToDMML: Updating in progress.. Exit now.");
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;
  int disabledModify = roiNode->StartModify();

  // now get the widget properties (coordinates, measurement etc.) and save it to the dmml node
  vtkAnnotationROIRepresentation * rep = vtkAnnotationROIRepresentation::SafeDownCast(roiWidget->GetRepresentation());

  double extents[3];
  double center[3];
  rep->GetExtents(extents);
  rep->GetCenter(center);

  //this->GetWorldToLocalCoordinates(roiNode, center, center);

  roiNode->SetXYZ(center[0], center[1], center[2] );
  roiNode->SetRadiusXYZ(0.5*extents[0], 0.5*extents[1], 0.5*extents[2] );

  // save the current view
  roiNode->SaveView();

  // enable processing of modified events
  roiNode->EndModify(disabledModify);

  //roiNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, roiNode);

  // This displayableManager should now consider ModifiedEvent again
  this->m_Updating = 0;
}

//---------------------------------------------------------------------------
/// Create a annotationDMMLnode
void vtkDMMLAnnotationROIDisplayableManager::OnClickInRenderWindow(double x, double y, const char *associatedNodeID)
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  // place the seed where the user clicked
  this->PlaceSeed(x,y);


  if (!this->m_ClickCounter->HasEnoughClicks(2))
    {
    this->GetDisplayToWorldCoordinates(x, y, this->LastClickWorldCoordinates);
    }
  else
    {
    // switch to updating state to avoid events mess
    //this->m_Updating = 1;

    //vtkHandleWidget *h1 = this->GetSeed(0);
    vtkHandleWidget *h2 = this->GetSeed(1);

    // convert the coordinates
    //double* displayCoordinates1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetDisplayPosition();
    double* displayCoordinates2 = vtkHandleRepresentation::SafeDownCast(h2->GetRepresentation())->GetDisplayPosition();

    double origin[4] = {this->LastClickWorldCoordinates[0],
                        this->LastClickWorldCoordinates[1],
                        this->LastClickWorldCoordinates[2],
                        1};
    double radius[4] = {0,0,0,0};

    //this->GetDisplayToWorldCoordinates(displayCoordinates1[0],displayCoordinates1[1],origin);
    this->GetDisplayToWorldCoordinates(displayCoordinates2[0],displayCoordinates2[1],radius);

    double rx = sqrt((origin[0]-radius[0])*(origin[0]-radius[0]));
    if (rx==0)
      {
      rx = 100;
      }
    double ry = sqrt((origin[1]-radius[1])*(origin[1]-radius[1]));
    if (ry==0)
      {
      ry = rx;
      }
    double rz = sqrt((origin[2]-radius[2])*(origin[2]-radius[2]));
    if (rz==0)
      {
      rz = rx;
      }

    // done calculating the bounds

    // create the DMML node
    vtkDMMLAnnotationROINode *roiNode = vtkDMMLAnnotationROINode::New();

    roiNode->SetXYZ(origin);
    roiNode->SetRadiusXYZ(rx,ry,rz);

    roiNode->SetName(this->GetDMMLScene()->GetUniqueNameByString("R"));

    // if this was a one time place, go back to view transform mode
    vtkDMMLInteractionNode *interactionNode = this->GetInteractionNode();
    if (interactionNode && interactionNode->GetPlaceModePersistence() != 1)
      {
      interactionNode->SetCurrentInteractionMode(vtkDMMLInteractionNode::ViewTransform);
      }

    this->GetDMMLScene()->SaveStateForUndo();

    // is there a node associated with this?
    if (associatedNodeID)
      {
      roiNode->SetAttribute("AssociatedNodeID", associatedNodeID);
      }

    roiNode->Initialize(this->GetDMMLScene());

    roiNode->Delete();

    // reset updating state
    this->m_Updating = 0;
    }

  }

void vtkDMMLAnnotationROIDisplayableManager::SetParentTransformToWidget(vtkDMMLAnnotationNode *node, vtkAbstractWidget *widget)
{
  if (!node || !widget)
    {
    return;
    }

  vtkAnnotationROIRepresentation *rep = vtkAnnotationROIRepresentation::SafeDownCast(widget->GetRepresentation());

  vtkNew<vtkMatrix4x4> transformToWorld;
  transformToWorld->Identity();

  // get the nodes's transform node
  vtkDMMLTransformNode* tnode = node->GetParentTransformNode();
  if (rep != nullptr && tnode != nullptr && tnode->IsTransformToWorldLinear())
    {
    tnode->GetMatrixTransformToWorld(transformToWorld.GetPointer());
    }

  vtkNew<vtkPropCollection> actors;
  rep->GetActors(actors.GetPointer());

  for (int i=0; i<actors->GetNumberOfItems(); i++)
    {
    vtkActor *actor = vtkActor::SafeDownCast(actors->GetItemAsObject(i));
    actor->SetUserMatrix(transformToWorld.GetPointer());
    }
  //rep->SetTransform(xform);

  widget->InvokeEvent(vtkCommand::EndInteractionEvent);

}

//---------------------------------------------------------------------------
bool vtkDMMLAnnotationROIDisplayableManager::IsWidgetDisplayable(vtkDMMLSliceNode* vtkNotUsed(sliceNode),
                                                                 vtkDMMLAnnotationNode* vtkNotUsed(node))
{
  if (this->Is2DDisplayableManager())
    {
    return true;
    }
  else
    {
    return false;
    }
}
