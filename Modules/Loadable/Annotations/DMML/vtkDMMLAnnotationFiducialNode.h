#ifndef __vtkDMMLAnnotationFiducialNode_h
#define __vtkDMMLAnnotationFiducialNode_h

#include "vtkCjyxAnnotationsModuleDMMLExport.h"
#include "vtkDMMLAnnotationControlPointsNode.h"

#include <vtkStdString.h>
class vtkStringArray;
class vtkDMMLStorageNode;

/// \brief DMML node to represent a fiducial in the Annotations module - deprecated
///
/// Annotation nodes contains control points, internally represented as vtkPolyData.
/// A Annotation node contains many control points  and forms the smallest logical unit of tractography
/// that DMML will manage/read/write. Each control point has accompanying data.
/// Visualization parameters for these nodes are controlled by the
/// vtkDMMLAnnotationPointDisplayNode class.
/// \deprecated Use vtkDMMLMarkupsFiduicalNode
/// \ingroup Cjyx_QtModules_Annotation
class  VTK_CJYX_ANNOTATIONS_MODULE_DMML_EXPORT vtkDMMLAnnotationFiducialNode : public vtkDMMLAnnotationControlPointsNode
{
public:
  static vtkDMMLAnnotationFiducialNode *New();
  vtkTypeMacro(vtkDMMLAnnotationFiducialNode,vtkDMMLAnnotationControlPointsNode);

  //--------------------------------------------------------------------------
  // DMMLNode methods
  //--------------------------------------------------------------------------

  vtkDMMLNode* CreateNodeInstance() override;
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "AnnotationFiducials";}

  const char* GetIcon() override {return ":/Icons/AnnotationPoint.png";}

  int  SetFiducial(double newControl[3],int selectedFlag, int visibleFlag);

  /// Selected and visible are currently always set to 1 and are controlled by selected and visible flag
  void SetFiducialLabel(const char* newLabel)
    {
    this->SetText(0,newLabel,1,1);
    }

  vtkStdString GetFiducialLabel()
    {
    return this->GetText(0);
    }

  void SetFiducialValue(const char* newValue)
    {
    this->SetText(1,newValue,1,1);
    }

  /// return atoi(this->GetText(1).c_str());
  int GetFiducialValue()
    {
    return 0;
    }

  int SetFiducialCoordinates(double newCoord[3], int selFlag = 1, int visFlag = 1)
    {
    return this->SetControlPoint(0,newCoord,selFlag,visFlag);
    }

  int SetFiducialWorldCoordinates(double newCoord[3], int selFlag = 1, int visFlag = 1)
    {
    return this->SetControlPointWorldCoordinates(0,newCoord,selFlag,visFlag);
    }

  int SetFiducialCoordinates(double x, double y, double z)
    {
    double newCoord[3];
    newCoord[0] = x;
    newCoord[1] = y;
    newCoord[2] = z;
    return this->SetFiducialCoordinates(newCoord);
    }

  double* GetFiducialCoordinates()
    {
    return this->GetControlPointCoordinates(0);
    }

  void GetFiducialWorldCoordinates(double *point)
    {
    this->GetControlPointWorldCoordinates(0, point);
    }

  /// returns true and control point coordinate 0 on success, false and 0,0,0 on failure
  bool GetFiducialCoordinates(double coord[3]);

  enum
  {
    FiducialNodeAddedEvent = 0,
    ValueModifiedEvent,
  };

protected:
  /// \deprecated Use vtkDMMLMarkupsFiducialNode
  /// \sa vtkDMMLMarkupsFiducialNode::vtkDMMLMarkupsFiducialNode()
  vtkDMMLAnnotationFiducialNode();
  ~vtkDMMLAnnotationFiducialNode() override;
  vtkDMMLAnnotationFiducialNode(const vtkDMMLAnnotationFiducialNode&);
  void operator=(const vtkDMMLAnnotationFiducialNode&);

  void SetTextFromID();

};

#endif
