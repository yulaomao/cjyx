// .NAME vtkDMMLAnnotationTextNode - DMML node to represent a text.
// .SECTION Description
// Annotation nodes contains control points, internally represented as vtkPolyData.
// A Annotation node contains many control points  and forms the smallest logical unit of tractography
// that DMML will manage/read/write. Each control point has accompanying data.
// Visualization parameters for these nodes are controlled by the vtkDMMLAnnotationPointDisplayNode class.
//

#ifndef __vtkDMMLAnnotationTextNode_h
#define __vtkDMMLAnnotationTextNode_h

#include "vtkCjyxAnnotationsModuleDMMLExport.h"
#include "vtkDMMLAnnotationControlPointsNode.h"

#include <vtkStdString.h>
class vtkStringArray;

/// \ingroup Cjyx_QtModules_Annotation
class  VTK_CJYX_ANNOTATIONS_MODULE_DMML_EXPORT vtkDMMLAnnotationTextNode : public vtkDMMLAnnotationControlPointsNode
{
public:
  static vtkDMMLAnnotationTextNode *New();
  vtkTypeMacro(vtkDMMLAnnotationTextNode,vtkDMMLAnnotationControlPointsNode);

  //--------------------------------------------------------------------------
  // DMMLNode methods
  //--------------------------------------------------------------------------

  vtkDMMLNode* CreateNodeInstance() override;
  // Description:
  // Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "AnnotationText";}

  const char* GetIcon() override {return ":/Icons/AnnotationText.png";}

  int SetTextCoordinates(double newCoord[3]) {return this->SetControlPoint(0,newCoord,1,1);}
  double* GetTextCoordinates() {return this->GetControlPointCoordinates(0);}

  // normalized viewport coordinates of the lower left of the text box for the caption widget
  int SetCaptionCoordinates(double x, double y);
  int SetCaptionCoordinates(double newCoord[2]);
  double* GetCaptionCoordinates() {return this->GetControlPointCoordinates(1);}

  void SetTextLabel(const char* newLabel) {this->SetText(0, newLabel, 1, 1);}
  vtkStdString GetTextLabel() {return this->GetText(0);}

  enum
  {
    TextNodeAddedEvent = 0,
    ValueModifiedEvent,
  };

protected:
  vtkDMMLAnnotationTextNode();
  ~vtkDMMLAnnotationTextNode() override  = default;
  vtkDMMLAnnotationTextNode(const vtkDMMLAnnotationTextNode&);
  void operator=(const vtkDMMLAnnotationTextNode&);

};

#endif
