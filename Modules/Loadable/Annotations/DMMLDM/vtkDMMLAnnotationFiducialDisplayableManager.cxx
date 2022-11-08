
// Annotation Logic includes
#include <vtkCjyxAnnotationModuleLogic.h>

// Annotation DMML includes
#include <vtkDMMLAnnotationFiducialNode.h>
#include <vtkDMMLAnnotationNode.h>
#include <vtkDMMLAnnotationDisplayNode.h>
#include <vtkDMMLAnnotationPointDisplayNode.h>
#include <vtkDMMLAnnotationTextDisplayNode.h>

// Annotation DMMLDisplayableManager includes
#include "vtkDMMLAnnotationDisplayableManagerHelper.h"
#include "vtkDMMLAnnotationFiducialDisplayableManager.h"

// Annotation VTKWidgets includes
#include <vtkAnnotationGlyphSource2D.h>

// DMMLDisplayableManager includes
#include <vtkDMMLSliceViewInteractorStyle.h>

// DMML includes
#include <vtkDMMLInteractionNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceNode.h>

// VTK includes
#include <vtkAbstractWidget.h>
#include <vtkFollower.h>
#include <vtkHandleRepresentation.h>
#include <vtkInteractorStyle.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkOrientedPolygonalHandleRepresentation3D.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkProperty2D.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSeedWidget.h>
#include <vtkSmartPointer.h>
#include <vtkSeedRepresentation.h>
#include <vtkSphereSource.h>

// STD includes
#include <string>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkDMMLAnnotationFiducialDisplayableManager);

//---------------------------------------------------------------------------
// vtkDMMLAnnotationFiducialDisplayableManager Callback
/// \ingroup Cjyx_QtModules_Annotation
class vtkAnnotationFiducialWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationFiducialWidgetCallback *New()
  { return new vtkAnnotationFiducialWidgetCallback; }

  vtkAnnotationFiducialWidgetCallback() = default;

  void Execute (vtkObject *vtkNotUsed(caller), unsigned long event, void*) override
  {

    // mark the Node with an attribute to indicate if it is currently being interacted with
    // so that other code can respond to changes only when it is not moving
    // Annodation.MovingInSliceView will be set to the layout name of
    // our slice node while it is being actively manipulated
    if (this->m_Widget && this->m_DisplayableManager && this->m_Node)
      {
      vtkDMMLSliceNode *sliceNode = this->m_DisplayableManager->GetSliceNode();
      if (sliceNode)
        {
        int modifiedWasDisabled = this->m_Node->GetDisableModifiedEvent();
        this->m_Node->DisableModifiedEventOn();
        if (this->m_Widget->GetWidgetState() == vtkSeedWidget::MovingSeed)
          {
          this->m_Node->SetAttribute("Annotations.MovingInSliceView", sliceNode->GetLayoutName());
          }
        else
          {
          const char *movingView = this->m_Node->GetAttribute("Annotations.MovingInSliceView");
          if (movingView && !strcmp(movingView, sliceNode->GetLayoutName()))
            {
            this->m_Node->RemoveAttribute("Annotations.MovingInSliceView");
            }
          }
        this->m_Node->SetDisableModifiedEvent(modifiedWasDisabled);
        }
      }

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


    if (this->m_DisplayableManager->Is2DDisplayableManager())
      {

      // if this is a 2D SliceView displayableManager, restrict the widget to the renderer

      // we need the widgetRepresentation
      vtkSeedRepresentation * representation = vtkSeedRepresentation::SafeDownCast(this->m_Widget->GetRepresentation());

      double displayCoordinates1[4];

      // first, we get the current displayCoordinates of the points
      representation->GetSeedDisplayPosition(0,displayCoordinates1);

      // second, we copy these to restrictedDisplayCoordinates
      double restrictedDisplayCoordinates1[4] = {displayCoordinates1[0], displayCoordinates1[1], displayCoordinates1[2], displayCoordinates1[3]};

      // modify restrictedDisplayCoordinates 1 and 2, if these are outside the viewport of the current renderer
      this->m_DisplayableManager->RestrictDisplayCoordinatesToViewport(restrictedDisplayCoordinates1);

      // only if we had to restrict the coordinates aka. if the coordinates changed, we update the positions
      if (this->m_DisplayableManager->GetDisplayCoordinatesChanged(displayCoordinates1,restrictedDisplayCoordinates1))
        {
        if (representation->GetRenderer() &&
            representation->GetRenderer()->GetActiveCamera())
          {
          representation->SetSeedDisplayPosition(0,restrictedDisplayCoordinates1);
          }
        }
      }

      if (event == vtkCommand::EndInteractionEvent)
        {
        // save the state of the node when done moving, then call
        // PropagateWidgetToDMML to update the node one last time
        if (this->m_Node->GetScene())
          {
          this->m_Node->GetScene()->SaveStateForUndo();
          }
        }

    // the interaction with the widget ended, now propagate the changes to DMML
    this->m_DisplayableManager->PropagateWidgetToDMML(this->m_Widget, this->m_Node);
  }

  void SetWidget(vtkAbstractWidget *w)
  {
    this->m_Widget = vtkSeedWidget::SafeDownCast(w);
  }
  void SetNode(vtkDMMLAnnotationNode *n)
  {
    this->m_Node = n;
  }
  void SetDisplayableManager(vtkDMMLAnnotationDisplayableManager * dm)
  {
    this->m_DisplayableManager = dm;
  }

  vtkSeedWidget * m_Widget;
  vtkDMMLAnnotationNode * m_Node;
  vtkDMMLAnnotationDisplayableManager * m_DisplayableManager;
  bool WasMoving;
  bool IsMoving;
};

//---------------------------------------------------------------------------
// vtkDMMLAnnotationFiducialDisplayableManager methods

//---------------------------------------------------------------------------
void vtkDMMLAnnotationFiducialDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
/// Create a new text widget.
vtkAbstractWidget * vtkDMMLAnnotationFiducialDisplayableManager::CreateWidget(vtkDMMLAnnotationNode* node)
{

  if (!node)
    {
    vtkErrorMacro("CreateWidget: Node not set!");
    return nullptr;
    }

  // 2d glyphs and text need to be scaled by 1/60 to show up properly in the 2d slice windows
  this->SetScaleFactor2D(0.01667);

  vtkDMMLAnnotationFiducialNode* fiducialNode = vtkDMMLAnnotationFiducialNode::SafeDownCast(node);

  if (!fiducialNode)
    {
    vtkErrorMacro("CreateWidget: Could not get fiducial node!");
    return nullptr;
    }

  vtkDMMLAnnotationPointDisplayNode *displayNode = fiducialNode->GetAnnotationPointDisplayNode();

  if (!displayNode)
    {
    std::cout<<"No DisplayNode!"<<std::endl;
    }

  // unset the glyph type which can be necessary when recreating a widget due to 2d/3d swap
  std::map<vtkDMMLNode*, int>::iterator iter  = this->NodeGlyphTypes.find(displayNode);
  if (iter != this->NodeGlyphTypes.end())
    {
    vtkDebugMacro("CreateWidget: found a glyph type already defined for this node: " << iter->second);
    this->NodeGlyphTypes[displayNode] = vtkDMMLAnnotationPointDisplayNode::GlyphMin - 1;
    }
  vtkNew<vtkSeedRepresentation> rep;
  if (!this->IsInLightboxMode())
    {
    vtkDebugMacro("CreateWidget: not in light box mode, making a 3d handle");
    vtkNew<vtkOrientedPolygonalHandleRepresentation3D> handle;

    // default to a sphere glyph, update in propagate dmml to widget
    vtkNew<vtkAnnotationGlyphSource2D> glyphSource;
    glyphSource->SetGlyphType(vtkDMMLAnnotationPointDisplayNode::Sphere3D);
    glyphSource->Update();
    glyphSource->SetScale(1.0);
    handle->SetHandle(glyphSource->GetOutput());
    rep->SetHandleRepresentation(handle.GetPointer());
    }
  else
    {
    vtkDebugMacro("CreateWidget: in light box mode, making a 2d handle");
    vtkNew<vtkPointHandleRepresentation2D> handle;
    rep->SetHandleRepresentation(handle.GetPointer());
    }




  //seed widget
  vtkSeedWidget * seedWidget = vtkSeedWidget::New();
  seedWidget->CreateDefaultRepresentation();

  seedWidget->SetRepresentation(rep.GetPointer());

  seedWidget->SetInteractor(this->GetInteractor());
  // set the renderer on the widget and representation
  if (!this->IsInLightboxMode())
    {
    seedWidget->SetCurrentRenderer(this->GetRenderer());
    seedWidget->GetRepresentation()->SetRenderer(this->GetRenderer());
    }
  else
    {
    int lightboxIndex = this->GetLightboxIndex(fiducialNode);
    seedWidget->SetCurrentRenderer(this->GetRenderer(lightboxIndex));
    seedWidget->GetRepresentation()->SetRenderer(this->GetRenderer(lightboxIndex));
    }

  //seedWidget->ProcessEventsOff();

  // create a new handle
  vtkHandleWidget* newhandle = seedWidget->CreateNewHandle();
  if (!newhandle)
    {
    vtkErrorMacro("CreateWidget: error creaing a new handle!");
    }

  // init the widget from the dmml node
  this->PropagateDMMLToWidget(fiducialNode, seedWidget);

  if (this->GetSliceNode())
    {

    bool showWidget = true;
    showWidget = this->IsWidgetDisplayable(this->GetSliceNode(), node);

    if (showWidget)
      {
      seedWidget->On();
      }

    }
  else
    {

    seedWidget->On();

    }

  seedWidget->CompleteInteraction();

  return seedWidget;

  }

//---------------------------------------------------------------------------
/// Tear down the widget creation
void vtkDMMLAnnotationFiducialDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkDMMLAnnotationNode * node)
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

  // add the callback
  vtkAnnotationFiducialWidgetCallback *myCallback = vtkAnnotationFiducialWidgetCallback::New();
  myCallback->SetNode(node);
  myCallback->SetWidget(widget);
  myCallback->SetDisplayableManager(this);
  widget->AddObserver(vtkCommand::StartInteractionEvent,myCallback);
  widget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  widget->AddObserver(vtkCommand::InteractionEvent,myCallback);
  myCallback->Delete();

  // store the current view
  node->SaveView();

}


//---------------------------------------------------------------------------
/// Propagate properties of DMML node to widget.
void vtkDMMLAnnotationFiducialDisplayableManager::PropagateDMMLToWidget(vtkDMMLAnnotationNode* node, vtkAbstractWidget * widget)
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

  // cast to the specific widget
  vtkSeedWidget* seedWidget = vtkSeedWidget::SafeDownCast(widget);

  if (!seedWidget)
    {
    vtkErrorMacro("PropagateDMMLToWidget: Could not get seed widget!");
    return;
    }

  // cast to the specific dmml node
  vtkDMMLAnnotationFiducialNode* fiducialNode = vtkDMMLAnnotationFiducialNode::SafeDownCast(node);

  if (!fiducialNode)
    {
    vtkErrorMacro("PropagateDMMLToWidget: Could not get fiducial node!");
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;

  // now get the widget properties (coordinates, measurement etc.) and if the dmml node has changed, propagate the changes
  vtkSeedRepresentation * seedRepresentation = vtkSeedRepresentation::SafeDownCast(seedWidget->GetRepresentation());
  if (!seedRepresentation)
    {
    vtkErrorMacro("PropagateDMMLToWidget: Could not get seed representation from widget!");
    return;
    }

  vtkDMMLAnnotationPointDisplayNode *displayNode = fiducialNode->GetAnnotationPointDisplayNode();

  if (!displayNode)
    {
    vtkDebugMacro("PropagateDMMLToWidget: Could not get display node for node " << (fiducialNode->GetID() ? fiducialNode->GetID() : "null id"));
    }

  // adjust the scale of the widget based on the current slice dimensions
  // so the widgets will maintain a fixed screen size as the window is resized
  // - this is used below to adjust text and fiducial size
  double scaleFor2D = 1;
  if (this->GetSliceNode())
    {
    int *dimensions = this->GetSliceNode()->GetDimensions();
    double windowDiagonal = sqrt(double(dimensions[0]*dimensions[0]+dimensions[1]*dimensions[1]));
    scaleFor2D = 707.1 / windowDiagonal; // assume a nominal screen of 500x500
    }

  vtkOrientedPolygonalHandleRepresentation3D *handleRep =
      vtkOrientedPolygonalHandleRepresentation3D::SafeDownCast(seedRepresentation->GetHandleRepresentation(0));
  // might be in lightbox mode where using a 2d point handle
  vtkPointHandleRepresentation2D *pointHandleRep = vtkPointHandleRepresentation2D::SafeDownCast(seedRepresentation->GetHandleRepresentation(0));
  // double check that if switch in and out of light box mode, the handle rep
  // is updated
  bool updateHandleType = false;
  if (this->IsInLightboxMode())
    {
    if (handleRep)
      {
      vtkDebugMacro("PropagateDMMLToWidget: have a 3d handle representation in 2d light box, resetting it.");
      updateHandleType = true;
      }
    }
  else
    {
    if (pointHandleRep)
      {
      vtkDebugMacro("PropagateDMMLToWidget: Not in light box, but have a point handle.");
      updateHandleType = true;
      }
    }
  if (updateHandleType)
    {
    vtkDebugMacro("PropagateDMMLToWidget: removing widget...");
    // clean it out
    this->Helper->RemoveWidgetAndNode(node);
    // recreate it
    vtkDMMLAnnotationNode *annotationNode = vtkDMMLAnnotationNode::SafeDownCast(node);
    if (annotationNode)
      {
      this->AddAnnotation(annotationNode);
      }
    // did it come back in here
    vtkDebugMacro("PropagateDMMLToWidget: did it end up calling this method already?");
    // probably not, so call and return
    vtkAbstractWidget * widget = this->Helper->GetWidget(annotationNode);
    if (widget)
      {
      this->PropagateDMMLToWidget(annotationNode, widget);
      }
    else
      {
      vtkWarningMacro("PropagateDMMLToWidget: failed to add a new widget for node " << node->GetName());
      return;
      }
    vtkDebugMacro("PropagateDMMLToWidget: NOW returning after calling self from self");
    return;
    }
  if (handleRep)
    {
    if (displayNode)
      {
      // set the glyph type
      std::map<vtkDMMLNode*, int>::iterator iter  = this->NodeGlyphTypes.find(displayNode);
      if (iter == this->NodeGlyphTypes.end() || iter->second != displayNode->GetGlyphType())
        {
        vtkDebugMacro("DisplayNode glyph type = " << displayNode->GetGlyphType() << " = " << displayNode->GetGlyphTypeAsString()
                      << ", is 3d glyph = " << (displayNode->GlyphTypeIs3D() ? "true" : "false")
                      << ", is 2d disp manager = " << this->Is2DDisplayableManager());
        if (displayNode->GlyphTypeIs3D())
          {
          if (this->Is2DDisplayableManager())
            {
            // map the 3d sphere to a filled circle, the 3d diamond to a filled
            // diamond
            vtkNew<vtkAnnotationGlyphSource2D> glyphSource;
            if (displayNode->GetGlyphType() == vtkDMMLAnnotationPointDisplayNode::Sphere3D)
              {
              glyphSource->SetGlyphType(vtkDMMLAnnotationPointDisplayNode::Circle2D);
              }
            else if (displayNode->GetGlyphType() == vtkDMMLAnnotationPointDisplayNode::Diamond3D)
              {
              glyphSource->SetGlyphType(vtkDMMLAnnotationPointDisplayNode::Diamond2D);
              }
            else
              {
              glyphSource->SetGlyphType(vtkDMMLAnnotationPointDisplayNode::StarBurst2D);
              }
            glyphSource->Update();
            glyphSource->SetScale(1.0);
            handleRep->SetHandle(glyphSource->GetOutput());
            } //if (this->Is2DDisplayableManager())
          else
            {
            if (displayNode->GetGlyphType() == vtkDMMLAnnotationPointDisplayNode::Sphere3D)
              {
              vtkNew<vtkSphereSource> sphereSource;
              sphereSource->SetRadius(0.5);
              sphereSource->SetPhiResolution(10);
              sphereSource->SetThetaResolution(10);
              sphereSource->Update();
              handleRep->SetHandle(sphereSource->GetOutput());
              }
            else
              {
              // the 3d diamond isn't supported yet, use a 2d diamond for now
              vtkNew<vtkAnnotationGlyphSource2D> glyphSource;
              glyphSource->SetGlyphType(vtkDMMLAnnotationPointDisplayNode::Diamond2D);
              glyphSource->Update();
              glyphSource->SetScale(1.0);
              handleRep->SetHandle(glyphSource->GetOutput());
              }
            }
          }//if (displayNode->GlyphTypeIs3D())
        else
          {
          // 2D
          vtkNew<vtkAnnotationGlyphSource2D> glyphSource;
          glyphSource->SetGlyphType(displayNode->GetGlyphType());
          glyphSource->Update();
          glyphSource->SetScale(1.0);
          handleRep->SetHandle(glyphSource->GetOutput());
          }
        this->NodeGlyphTypes[displayNode] = displayNode->GetGlyphType();
        } // if (iter == this->NodeGlyphTypes.end() || iter->second != displayNode->GetGlyphType())
      // end of glyph type

      if (fiducialNode->GetSelected())
        {
        // use the selected color
        handleRep->GetProperty()->SetColor(displayNode->GetSelectedColor());
        }
      else
        {
        // use the unselected color
        handleRep->GetProperty()->SetColor(displayNode->GetColor());
        }
      // material properties
      handleRep->GetProperty()->SetOpacity(displayNode->GetOpacity());
      handleRep->GetProperty()->SetAmbient(displayNode->GetAmbient());
      handleRep->GetProperty()->SetDiffuse(displayNode->GetDiffuse());
      handleRep->GetProperty()->SetSpecular(displayNode->GetSpecular());

//    handleRep->SetHandle(glyphSource->GetOutput());

      // the following check is only needed since we require a different uniform scale depending on 2D and 3D
      if (this->Is2DDisplayableManager())
        {
        handleRep->SetUniformScale(displayNode->GetGlyphScale()*this->GetScaleFactor2D()*scaleFor2D);
        }
      else
        {
        handleRep->SetUniformScale(displayNode->GetGlyphScale());
        }
      } // if point display node

    // update the text
    if (fiducialNode->GetName())
      {
      // create a string
      vtkStdString textString;
      textString = vtkStdString(fiducialNode->GetName());
      if (vtkStdString(handleRep->GetLabelText()) != textString)
        {
        handleRep->SetLabelText(textString.c_str());
        }

      // get the text display node
      vtkDMMLAnnotationTextDisplayNode *textDisplayNode = fiducialNode->GetAnnotationTextDisplayNode();
      if (textDisplayNode)
        {
        // scale the text
        double textscale[3] = {textDisplayNode->GetTextScale(), textDisplayNode->GetTextScale(), textDisplayNode->GetTextScale()};
        if (this->Is2DDisplayableManager())
          {
          // scale it down for the 2d windows
          textscale[0] *= this->GetScaleFactor2D() * scaleFor2D;
          textscale[1] *= this->GetScaleFactor2D() * scaleFor2D;
          textscale[2] *= this->GetScaleFactor2D() * scaleFor2D;
          }
        handleRep->SetLabelTextScale(textscale);
        if (handleRep->GetLabelTextActor())
          {
          // set the colors
          if (fiducialNode->GetSelected())
            {
            handleRep->GetLabelTextActor()->GetProperty()->SetColor(textDisplayNode->GetSelectedColor());
            }
          else
            {
            handleRep->GetLabelTextActor()->GetProperty()->SetColor(textDisplayNode->GetColor());
            }
          handleRep->GetLabelTextActor()->GetProperty()->SetOpacity(textDisplayNode->GetOpacity());
          }
        }//if (textDisplayNode)
      handleRep->LabelVisibilityOn();
      } // GetName
    else
      {
      handleRep->LabelVisibilityOff();
      }
    }//if (handleRep)
  else if (pointHandleRep)
    {
    if (displayNode)
      {
      // glyph type
      /*
      std::map<vtkDMMLNode*, int>::iterator iter  = this->NodeGlyphTypes.find(displayNode);
      if (iter == this->NodeGlyphTypes.end() || iter->second != displayNode->GetGlyphType())
        {
        // map the 3d sphere to a filled circle, the 3d diamond to a filled
        // diamond
        vtkNew<vtkAnnotationGlyphSource2D> glyphSource;
        if (displayNode->GetGlyphType() == vtkDMMLAnnotationPointDisplayNode::Sphere3D)
          {
          glyphSource->SetGlyphType(vtkDMMLAnnotationPointDisplayNode::Circle2D);
          }
        else if (displayNode->GetGlyphType() == vtkDMMLAnnotationPointDisplayNode::Diamond3D)
          {
          glyphSource->SetGlyphType(vtkDMMLAnnotationPointDisplayNode::Diamond2D);
          }
        else
          {
          glyphSource->SetGlyphType(displayNode->GetGlyphType());
          }
        glyphSource->Update();
        glyphSource->SetScale(1.0);
        std::cout << "PropagateDMMLToWidget: " << this->GetSliceNode()->GetName()
                  << ": setting point handle rep cursor shape " << glyphSource->GetOutput() << std::endl;
        pointHandleRep->SetCursorShape(glyphSource->GetOutput());
        this->NodeGlyphTypes[displayNode] = displayNode->GetGlyphType();
        }
      */
      // set the color
      if (fiducialNode->GetSelected())
        {
        // use the selected color
        pointHandleRep->GetProperty()->SetColor(displayNode->GetSelectedColor());
        }
      else
        {
        // use the unselected color
        pointHandleRep->GetProperty()->SetColor(displayNode->GetColor());
        }
      }
    }
  // now update the position
  this->UpdatePosition(widget, node);

  // update lock status
  this->Helper->UpdateLocked(node);

  // update visibility status
  bool displayableInViewer = true;
  // IsWidgetDisplayable is only defined on 2d slice viewers
  if (this->Is2DDisplayableManager())
    {
    displayableInViewer = this->IsWidgetDisplayable(this->GetSliceNode(), node);
    }
  this->Helper->UpdateVisible(node, displayableInViewer);

  seedRepresentation->NeedToRenderOn();
  seedWidget->Modified();

//  seedWidget->CompleteInteraction();

  // enable processing of modified events
  this->m_Updating = 0;


}

//---------------------------------------------------------------------------
/// Propagate properties of widget to DMML node.
void vtkDMMLAnnotationFiducialDisplayableManager::PropagateWidgetToDMML(vtkAbstractWidget * widget, vtkDMMLAnnotationNode* node)
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
  vtkSeedWidget* seedWidget = vtkSeedWidget::SafeDownCast(widget);

  if (!seedWidget)
   {
   vtkErrorMacro("PropagateWidgetToDMML: Could not get seed widget!");
   return;
   }

  if (seedWidget->GetWidgetState() != vtkSeedWidget::MovingSeed)
    {
    // ignore events not caused by seed movement,
    // but invoke any pending Modifieds, such as may have been
    // set in the callback command for changed attribute state
    node->InvokePendingModifiedEvent();
    return;
    }


  // cast to the specific dmml node
  vtkDMMLAnnotationFiducialNode* fiducialNode = vtkDMMLAnnotationFiducialNode::SafeDownCast(node);

  if (!fiducialNode)
   {
   vtkErrorMacro("PropagateWidgetToDMML: Could not get fiducial node!");
   return;
   }

  // disable processing of modified events
  this->m_Updating = 1;
  fiducialNode->DisableModifiedEventOn();

  // now get the widget properties (coordinates, measurement etc.) and if the dmml node has changed, propagate the changes
  vtkSeedRepresentation * seedRepresentation = vtkSeedRepresentation::SafeDownCast(seedWidget->GetRepresentation());

  double worldCoordinates1[4] = {0.0,0.0,0.0,1.0};

  if (this->Is2DDisplayableManager())
    {
    // 2D widget was changed

    double displayCoordinates1[4] = {0.0,0.0,0.0,1.0};
    seedRepresentation->GetSeedDisplayPosition(0,displayCoordinates1);
    vtkDebugMacro("PropagateWidgetToDMML: 2d displayable manager: widget display coords = "
                  << displayCoordinates1[0] << ", "
                  << displayCoordinates1[1] << ", "
                  << displayCoordinates1[2]);
    this->GetDisplayToWorldCoordinates(displayCoordinates1,worldCoordinates1);
    vtkDebugMacro("PropagateWidgetToDMML: 2d: widget world coords = " << worldCoordinates1[0] << ", " << worldCoordinates1[1] << ", "<< worldCoordinates1[2]);
    }
  else
    {

    seedRepresentation->GetSeedWorldPosition(0,worldCoordinates1);
vtkDebugMacro("PropagateWidgetToDMML: 3d: widget world coords = " << worldCoordinates1[0] << ", " << worldCoordinates1[1] << ", "<< worldCoordinates1[2]);
    }

  // was there a change?
  bool positionChanged = false;
  double currentCoordinates[4] = {0.0, 0.0, 0.0, 1.0};
  fiducialNode->GetFiducialWorldCoordinates(currentCoordinates);
  vtkDebugMacro("PropagateWidgetToDMML: fiducial current world coordinates = "
                << currentCoordinates[0] << ", "
                << currentCoordinates[1] << ", "
                << currentCoordinates[2]);

  double currentCoords[3];
  currentCoords[0] = currentCoordinates[0];
  currentCoords[1] = currentCoordinates[1];
  currentCoords[2] = currentCoordinates[2];
  double newCoords[3];
  newCoords[0] = worldCoordinates1[0];
  newCoords[1] = worldCoordinates1[1];
  newCoords[2] = worldCoordinates1[2];
  if (this->GetWorldCoordinatesChanged(currentCoords, newCoords))
    {
    vtkDebugMacro("PropagateWidgetToDMML: position changed.");
    positionChanged = true;
    }

  if (positionChanged)
    {
    vtkDebugMacro("PropagateWidgetToDMML: position changed, setting fiducial coordinates");
    fiducialNode->SetFiducialWorldCoordinates(worldCoordinates1);

    fiducialNode->SaveView();
    }
  //seedWidget->CompleteInteraction();

  // enable processing of modified events
  fiducialNode->DisableModifiedEventOff();

  if (positionChanged)
    {
    vtkDebugMacro("PropagateWidgetToDMML: position changed, calling modified on the fiducial node");
    fiducialNode->Modified();
    fiducialNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, fiducialNode);
    }
  // This displayableManager should now consider ModifiedEvent again
  this->m_Updating = 0;
}

//---------------------------------------------------------------------------
/// Create a annotationDMMLnode
void vtkDMMLAnnotationFiducialDisplayableManager::OnClickInRenderWindow(double x, double y, const char *associatedNodeID)
{
  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    vtkDebugMacro("OnClickInRenderWindow: x = " << x << ", y = " << y << ", incorrect displayable manager, focus = " << this->m_Focus << ", jumping out");
    return;
    }

  // place the seed where the user clicked
  vtkDebugMacro("OnClickInRenderWindow: placing seed at " << x << ", " << y);
  // switch to updating state to avoid events mess
  this->m_Updating = 1;

  double displayCoordinates1[2];
  displayCoordinates1[0] = x;
  displayCoordinates1[1] = y;


  double worldCoordinates1[4];

  this->GetDisplayToWorldCoordinates(displayCoordinates1,worldCoordinates1);

  // create the DMML node
  vtkDMMLAnnotationFiducialNode *fiducialNode = vtkDMMLAnnotationFiducialNode::New();

  fiducialNode->SetFiducialWorldCoordinates(worldCoordinates1);

  fiducialNode->SetName(this->GetDMMLScene()->GetUniqueNameByString("F"));

  // reset updating state
  this->m_Updating = 0;

  // if this was a one time place, go back to view transform mode
  vtkDMMLInteractionNode *interactionNode = this->GetInteractionNode();
  if (interactionNode && interactionNode->GetPlaceModePersistence() != 1)
    {
    vtkDebugMacro("End of one time place, place mode persistence = " << interactionNode->GetPlaceModePersistence());
    interactionNode->SetCurrentInteractionMode(vtkDMMLInteractionNode::ViewTransform);
    }

  // save for undo and add the node to the scene after any reset of the
  // interaction node so that don't end up back in place mode
  this->GetDMMLScene()->SaveStateForUndo();

  // is there a node associated with this?
  if (associatedNodeID)
    {
    fiducialNode->SetAttribute("AssociatedNodeID", associatedNodeID);
    }

  fiducialNode->Initialize(this->GetDMMLScene());

  fiducialNode->Delete();
}

//---------------------------------------------------------------------------
/// observe key press events
void vtkDMMLAnnotationFiducialDisplayableManager::AdditionnalInitializeStep()
{
  // don't add the key press event, as it triggers a crash on start up
  //vtkDebugMacro("Adding an observer on the key press event");
  this->AddInteractorStyleObservableEvent(vtkCommand::KeyPressEvent);
}


//---------------------------------------------------------------------------
void vtkDMMLAnnotationFiducialDisplayableManager::OnInteractorStyleEvent(int eventid)
{
  this->Superclass::OnInteractorStyleEvent(eventid);

  if (this->GetDisableInteractorStyleEventsProcessing())
    {
    vtkWarningMacro("OnInteractorStyleEvent: Processing of events was disabled.");
    return;
    }

  if (eventid == vtkCommand::KeyPressEvent)
    {
    char *keySym = this->GetInteractor()->GetKeySym();
    vtkDebugMacro("OnInteractorStyleEvent " << (this->Is2DDisplayableManager() ? this->GetSliceNode()->GetName() : "3D")
                  << ": key press event position = " << this->GetInteractor()->GetEventPosition()[0] << ", " << this->GetInteractor()->GetEventPosition()[1]
                  << ", key sym = " << (keySym == nullptr ? "null" : keySym));
    if (!keySym)
      {
      return;
      }
    if (strcmp(keySym, "p") == 0)
      {
      if (this->GetInteractionNode()->GetCurrentInteractionMode() == vtkDMMLInteractionNode::Place)
        {
        this->OnClickInRenderWindowGetCoordinates();
        }
      else
        {
        vtkDebugMacro("Fiducial DisplayableManager: key press p, but not in Place mode! Returning.");
        return;
        }
      }
    }
  else if (eventid == vtkCommand::KeyReleaseEvent)
    {
    vtkDebugMacro("Got a key release event");
    }
}


//---------------------------------------------------------------------------
void vtkDMMLAnnotationFiducialDisplayableManager::UpdatePosition(vtkAbstractWidget *widget, vtkDMMLNode *node)
{
//  vtkWarningMacro("UpdatePosition, node is " << (node == nullptr ? "null" : node->GetID()));
  if (!node)
    {
    return;
    }
  vtkDMMLAnnotationControlPointsNode *pointsNode = vtkDMMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!pointsNode)
    {
    vtkErrorMacro("UpdatePosition - Can not access control points node from node with id " << node->GetID());
    return;
    }
  // get the widget
  if (!widget)
    {
    vtkErrorMacro("UpdatePosition: no widget associated with points node " << pointsNode->GetID());
    return;
    }
  // cast to a seed widget
  vtkSeedWidget* seedWidget = vtkSeedWidget::SafeDownCast(widget);

  if (!seedWidget)
   {
   vtkErrorMacro("UpdatePosition: Could not get seed widget!");
   return;
   }

  // disable processing of modified events
  //this->m_Updating = 1;
  bool positionChanged = false;

  // now get the widget properties (coordinates, measurement etc.) and if the dmml node has changed, propagate the changes
  vtkSeedRepresentation * seedRepresentation = vtkSeedRepresentation::SafeDownCast(seedWidget->GetRepresentation());

  if (this->Is2DDisplayableManager())
    {
    // for 2d managers, compare the display positions
    double displayCoordinates1[4] = {0.0, 0.0, 0.0, 1.0};
    double displayCoordinatesBuffer1[4] = {0.0, 0.0, 0.0, 1.0};

    // get point in world coordinates using parent transforms
    double pointTransformed[4];
    pointsNode->GetControlPointWorldCoordinates(0, pointTransformed);

    this->GetWorldToDisplayCoordinates(pointTransformed,displayCoordinates1);

    seedRepresentation->GetSeedDisplayPosition(0,displayCoordinatesBuffer1);

    if (this->GetDisplayCoordinatesChanged(displayCoordinates1,displayCoordinatesBuffer1))
      {
      // only update when really changed
      vtkDebugMacro("UpdatePosition: " << this->GetSliceNode()->GetName()
                    << ": display coordinates changed:\n\tseed display = " << displayCoordinatesBuffer1[0] << ", " << displayCoordinatesBuffer1[1]
                    << "\n\tfid display =  " << displayCoordinates1[0] << ", " << displayCoordinates1[1] );
      // make sure the representation has a renderer and an active camera to
      // avoid a crash in vtkRenderer::ViewToWorld
      if (seedRepresentation->GetRenderer() &&
          seedRepresentation->GetRenderer()->GetActiveCamera())
        {
        seedRepresentation->SetSeedDisplayPosition(0,displayCoordinates1);
        }
      positionChanged = true;
      }
    else
      {
      vtkDebugMacro("UpdatePosition: " <<  this->GetSliceNode()->GetName() << ": display coordinates unchanged!");
      }
    }
  else
    {
    // transform fiducial point using parent transforms
    double fidWorldCoord[4];
    pointsNode->GetControlPointWorldCoordinates(0, fidWorldCoord);

    // for 3d managers, compare world positions
    double seedWorldCoord[4];
    seedRepresentation->GetSeedWorldPosition(0,seedWorldCoord);

    if (this->GetWorldCoordinatesChanged(seedWorldCoord, fidWorldCoord))
      {
      vtkDebugMacro("UpdatePosition: "
                    << (this->Is2DDisplayableManager() ? this->GetSliceNode()->GetName() : "3D")
                    << ": world coordinates changed:\n\tseed = " << seedWorldCoord[0] << ", " << seedWorldCoord[1] << ", " << seedWorldCoord[2]
                    << "\n\tfid =  " << fidWorldCoord[0] << ", " << fidWorldCoord[1] << ", " << fidWorldCoord[2]);
      seedRepresentation->GetHandleRepresentation(0)->SetWorldPosition(fidWorldCoord);
      positionChanged = true;
      }
    else
      {
      vtkDebugMacro("UpdatePosition: " << (this->Is2DDisplayableManager() ? this->GetSliceNode()->GetName() : "3D") << ": world coordinates unchanged!");
      }
    }
  if (positionChanged && this->m_Updating == 0)
    {
    // not already updating from propagate dmml to widget, so trigger a render
    seedRepresentation->NeedToRenderOn();
    seedWidget->Modified();
//    seedWidget->CompleteInteraction();
    }
  // enable processing of modified events
  //this->m_Updating = 0;
}

//---------------------------------------------------------------------------
void vtkDMMLAnnotationFiducialDisplayableManager::OnDMMLSceneEndClose()
{
  // clear out the map of glyph types
  this->NodeGlyphTypes.clear();
}
