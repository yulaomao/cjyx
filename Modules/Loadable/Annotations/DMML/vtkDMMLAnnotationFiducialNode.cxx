#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkDMMLAnnotationFiducialNode.h"
#include <vtkPolyData.h>

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLAnnotationFiducialNode);

//----------------------------------------------------------------------------
vtkDMMLAnnotationFiducialNode::vtkDMMLAnnotationFiducialNode() = default;

//----------------------------------------------------------------------------
vtkDMMLAnnotationFiducialNode::~vtkDMMLAnnotationFiducialNode() = default;

//---------------------------------------------------------------------------
int vtkDMMLAnnotationFiducialNode::SetFiducial(double newControl[3],int selectedFlag, int visibleFlag)
{
  if (!this->SetFiducialCoordinates(newControl, selectedFlag, visibleFlag))
    {
    return 0;
    }

  return 1;
}

//---------------------------------------------------------------------------
bool vtkDMMLAnnotationFiducialNode::GetFiducialCoordinates(double coord[3])
{
  coord[0] = coord[1] = coord[2] = 0.0;
  if (this->GetPoints())
    {
    this->GetPoints()->GetPoint(0, coord);
    return true;
    }
  return false;
}

//---------------------------------------------------------------------------
void vtkDMMLAnnotationFiducialNode::SetTextFromID()
{
  std::string idLabel = this->GetID();
  std::string textLabel;

  if (this->NumberingScheme == vtkDMMLAnnotationControlPointsNode::UseID)
    {
      textLabel = idLabel;
    }
  else
    {
    size_t pos = idLabel.find_last_not_of("0123456789");
    std::string strippedID = idLabel.substr(0, pos+1);
    std::stringstream ss;
    ss << strippedID;

    if (this->NumberingScheme == vtkDMMLAnnotationControlPointsNode::UseIndex)
      {
    // use the fid's index
    ss << '0';
      }
    else if (this->NumberingScheme == vtkDMMLAnnotationControlPointsNode::UsePrevious)
      {
    vtkErrorMacro("Currently option vtkDMMLAnnotationControlPointsNode::UsePrevious is not installed");
    return;
      }
      // use the number from the previous fiducial
      // int lastNumber = 0;
      // if (id > 0)
      //   {
      //       std::string previousLabel = this->GetText(id-1);
      //   size_t prevpos = previousLabel.find_last_not_of("0123456789");
      //   std::string suffixPreviousLabel = previousLabel.substr(prevpos+1, std::string::npos);
      //   lastNumber = atoi(suffixPreviousLabel.c_str());
      //   lastNumber++;
      //   }
      // ss << lastNumber;
      // }
    textLabel = ss.str();
    }
  this->SetFiducialLabel(textLabel.c_str());
}

