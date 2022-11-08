
// DMML includes
#include "vtkDMMLInteractionNode.h"

// VTK includes
#include <vtkObjectFactory.h>

// STD includes

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLInteractionNode);


//----------------------------------------------------------------------------
vtkDMMLInteractionNode::vtkDMMLInteractionNode()
{
  this->HideFromEditors = 1;

  this->SetSingletonTag("Singleton");

  this->CurrentInteractionMode = vtkDMMLInteractionNode::ViewTransform;
  this->LastInteractionMode = vtkDMMLInteractionNode::ViewTransform;
  this->PlaceModePersistence = 0;
  this->TransformModePersistence = 1;
  this->EnableFiberEdit = 0;
}

//----------------------------------------------------------------------------
vtkDMMLInteractionNode::~vtkDMMLInteractionNode() = default;

//----------------------------------------------------------------------------
int vtkDMMLInteractionNode::GetInteractionModeByString ( const char * modeString )
{
  if (modeString == nullptr)
    {
    return (-1);
    }

  if ( !strcmp (modeString, "Place" ))
    {
    return ( vtkDMMLInteractionNode::Place);
    }
  else if ( !strcmp (modeString, "ViewTransform" ))
    {
    return ( vtkDMMLInteractionNode::ViewTransform);
    }
  else if ( !strcmp (modeString, "Transform" ))
    {
    return ( vtkDMMLInteractionNode::ViewTransform);
    }
//  else if ( !strcmp (modeString, "SelectRegion" ))
//    {
//    return ( vtkDMMLInteractionNode::SelectRegion);
//    }
//  else if ( !strcmp (modeString, "LassoRegion" ))
//    {
//    return ( vtkDMMLInteractionNode::LassoRegion);
//    }
  else
    {
    return (-1);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLInteractionNode::NormalizeAllMouseModes()
{
  this->PlaceModePersistence = 0;
  this->TransformModePersistence = 1;
}

//----------------------------------------------------------------------------
void vtkDMMLInteractionNode::SetPlaceModePersistence ( int val )
{
  if (this->PlaceModePersistence != val)
    {
    this->PlaceModePersistence = val;
    this->InvokeEvent(this->InteractionModePersistenceChangedEvent, nullptr);
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkDMMLInteractionNode::SetTransformModePersistence ( int val )
{
  if (this->TransformModePersistence != val)
    {
    this->TransformModePersistence = val;
    this->InvokeEvent(this->InteractionModePersistenceChangedEvent, nullptr);
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkDMMLInteractionNode::SetLastInteractionMode ( int mode )
{
  if (this->LastInteractionMode != mode)
    {
    this->LastInteractionMode = mode;
    }
}

//----------------------------------------------------------------------------
void vtkDMMLInteractionNode::SetCurrentInteractionMode ( int mode )
{
  if (this->CurrentInteractionMode == mode)
    {
    return;
    }
  bool wasPlacing = (this->CurrentInteractionMode == vtkDMMLInteractionNode::Place);
  this->CurrentInteractionMode = mode;
  this->Modified();
  this->InvokeCustomModifiedEvent(vtkDMMLInteractionNode::InteractionModeChangedEvent);
  if (wasPlacing)
    {
    this->InvokeCustomModifiedEvent(vtkDMMLInteractionNode::EndPlacementEvent);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLInteractionNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  if ( this->GetCurrentInteractionMode() == vtkDMMLInteractionNode::Place )
    {
    of << " currentInteractionMode=\"" << "Place" << "\"";
    }
  else if ( this->GetCurrentInteractionMode() == vtkDMMLInteractionNode::ViewTransform )
    {
    of << " currentInteractionMode=\"" << "ViewTransform" << "\"";
    }
//  else if ( this->GetCurrentInteractionMode() == vtkDMMLInteractionNode::SelectRegion )
//    {
//    of << " currentInteractionMode=\"" << "SelectRegion" << "\"";
//    }
//  else if ( this->GetCurrentInteractionMode() == vtkDMMLInteractionNode::LassoRegion )
//    {
//    of << " currentInteractionMode=\"" << "LassoRegion" << "\"";
//    }

  of << " placeModePersistence=\"" << (this->PlaceModePersistence ? "true" : "false") << "\"";

  if ( this->GetLastInteractionMode() == vtkDMMLInteractionNode::Place )
    {
    of << " lastInteractionMode=\"" << "Place" << "\"";
    }
  else if ( this->GetLastInteractionMode() == vtkDMMLInteractionNode::ViewTransform )
    {
    of << " lastInteractionMode=\"" << "ViewTransform" << "\"";
    }
//  else if ( this->GetLastInteractionMode() == vtkDMMLInteractionNode::SelectRegion )
//    {
//    of << " lastInteractionMode=\"" << "SelectRegion" << "\"";
//    }
//  else if ( this->GetLastInteractionMode() == vtkDMMLInteractionNode::LassoRegion )
//    {
//    of << " lastInteractionMode=\"" << "LassoRegion" << "\"";
//    }
}

//----------------------------------------------------------------------------
void vtkDMMLInteractionNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "currentInteractionMode"))
      {
      if ( !strcmp (attValue, "Place" ))
        {
        this->CurrentInteractionMode = vtkDMMLInteractionNode::Place;
        }
      else if ( !strcmp (attValue, "ViewTransform" ))
        {
        this->CurrentInteractionMode = vtkDMMLInteractionNode::ViewTransform;
        }
//      else if ( !strcmp (attValue, "SelectRegion" ))
//        {
//        this->CurrentInteractionMode = vtkDMMLInteractionNode::SelectRegion;
//        }
//      else if ( !strcmp (attValue, "LassoRegion" ))
//        {
//        this->CurrentInteractionMode = vtkDMMLInteractionNode::LassoRegion;
//        }
      }
    else if (!strcmp(attName, "placeModePersistence"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->PlaceModePersistence = 1;
        }
      else
        {
        this->PlaceModePersistence = 0;
        }
      }
    else if (!strcmp(attName, "lastInteractionMode"))
      {
      if ( !strcmp (attValue, "Place" ))
        {
        this->LastInteractionMode = vtkDMMLInteractionNode::Place;
        }
      else if ( !strcmp (attValue, "ViewTransform" ))
        {
        this->LastInteractionMode = vtkDMMLInteractionNode::ViewTransform;
        }
//      else if ( !strcmp (attValue, "SelectRegion" ))
//        {
//        this->LastInteractionMode = vtkDMMLInteractionNode::SelectRegion;
//        }
//      else if ( !strcmp (attValue, "LassoRegion" ))
//        {
//        this->LastInteractionMode = vtkDMMLInteractionNode::LassoRegion;
//        }
      }
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkDMMLInteractionNode::Copy(vtkDMMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkDMMLInteractionNode *node = (vtkDMMLInteractionNode *) anode;

  this->SetCurrentInteractionMode (node->GetCurrentInteractionMode());
  this->SetPlaceModePersistence (node->GetPlaceModePersistence());

  this->SetLastInteractionMode ( node->GetLastInteractionMode());

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLInteractionNode::PrintSelf(ostream& os, vtkIndent indent)
{

  Superclass::PrintSelf(os,indent);
  os << indent << "CurrentInteractionMode:        " << this->GetInteractionModeAsString(this->CurrentInteractionMode) << "\n";
  os << indent << "LastInteractionMode:        " <<  this->GetInteractionModeAsString(this->LastInteractionMode) << "\n";

  os << indent << "PlaceModePersistence: " << this->GetPlaceModePersistence() << "\n";
  os << indent << "TransformModePersistence: " << this->GetTransformModePersistence() << "\n";
}

//---------------------------------------------------------------------------
const char * vtkDMMLInteractionNode::GetInteractionModeAsString(int mode)
{
  if (mode == this->Place)
    {
    return "Place";
    }
  else if (mode == this->ViewTransform)
    {
    return "ViewTransform";
    }
  //  else if (mode == this->SelectRegion)
  //    {
  //    return "SelectRegion";
  //    }
  //  else if (mode == this->LassoRegion)
  //    {
  //    return "LassoRegion";
  //    }
  else
    {
    return "(unknown)";
    }
}

//----------------------------------------------------------------------------
void vtkDMMLInteractionNode::SwitchToPersistentPlaceMode()
{
  if (this->GetCurrentInteractionMode() == vtkDMMLInteractionNode::Place &&
      this->GetPlaceModePersistence() == 1)
    {
    return;
    }
  int disabledModify = this->StartModify();
  this->NormalizeAllMouseModes();
  this->SetLastInteractionMode(this->GetCurrentInteractionMode());
  this->SetPlaceModePersistence(1);
  this->SetCurrentInteractionMode(vtkDMMLInteractionNode::Place);
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLInteractionNode::SwitchToSinglePlaceMode()
{
  if (this->GetCurrentInteractionMode() == vtkDMMLInteractionNode::Place &&
      this->GetPlaceModePersistence() == 0)
    {
    return;
    }
  int disabledModify = this->StartModify();
  this->NormalizeAllMouseModes();
  this->SetLastInteractionMode(this->GetCurrentInteractionMode());
  this->SetPlaceModePersistence(0);
  this->SetCurrentInteractionMode(vtkDMMLInteractionNode::Place);
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLInteractionNode::SwitchToViewTransformMode()
{
  if (this->GetCurrentInteractionMode() == vtkDMMLInteractionNode::ViewTransform &&
      this->GetTransformModePersistence() == 1)
    {
    return;
    }
  int disabledModify = this->StartModify();
  this->SetLastInteractionMode(this->GetCurrentInteractionMode());
  // only set transform mode persistence, keep the state of the pick and place
  // mode persistence
  this->SetTransformModePersistence(1);
  this->SetCurrentInteractionMode(vtkDMMLInteractionNode::ViewTransform);
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLInteractionNode::EditNode(vtkDMMLNode* node)
{
  // Observers in qCjyxCoreApplication listen for this event
  this->InvokeEvent(EditNodeEvent, node);
}

//----------------------------------------------------------------------------
void vtkDMMLInteractionNode::ShowViewContextMenu(vtkDMMLInteractionEventData* eventData)
{
  // Observers in qCjyxCoreApplication listen for this event
  this->InvokeEvent(ShowViewContextMenuEvent, eventData);
}
