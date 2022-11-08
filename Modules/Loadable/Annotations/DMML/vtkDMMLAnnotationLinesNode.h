// .NAME vtkDMMLAnnotationLinesNode - DMML node to represent a fiber bundle from tractography in DTI data.
// .SECTION Description
// Annotation nodes contains control points, internally represented as vtkPolyData.
// A Annotation node contains many control points  and forms the smallest logical unit of tractography
// that DMML will manage/read/write. Each control point has accompanying data.
// Visualization parameters for these nodes are controlled by the vtkDMMLAnnotationLineDisplayNode class.
//

#ifndef __vtkDMMLAnnotationLinesNode_h
#define __vtkDMMLAnnotationLinesNode_h

#include "vtkDMMLAnnotationControlPointsNode.h"

class vtkDMMLAnnotationLineDisplayNode;

/// \ingroup Cjyx_QtModules_Annotation
class  VTK_CJYX_ANNOTATIONS_MODULE_DMML_EXPORT vtkDMMLAnnotationLinesNode
  : public vtkDMMLAnnotationControlPointsNode
{
public:
  static vtkDMMLAnnotationLinesNode *New();
  vtkTypeMacro(vtkDMMLAnnotationLinesNode,vtkDMMLAnnotationControlPointsNode);
  // void PrintSelf(ostream& os, vtkIndent indent) override;
  // Description:
  // Just prints short summary
  void PrintAnnotationInfo(ostream& os, vtkIndent indent, int titleFlag = 1) override;

  //--------------------------------------------------------------------------
  // DMMLNode methods
  //--------------------------------------------------------------------------

  vtkDMMLNode* CreateNodeInstance() override;
  // Description:
  // Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "AnnotationLines";}

  // Description:
  // Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  // Description:
  // Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkDMMLNode::CopyContent
  vtkDMMLCopyContentDefaultMacro(vtkDMMLAnnotationLinesNode);

  void UpdateScene(vtkDMMLScene *scene) override;

  // Description:
  // alternative method to propagate events generated in Display nodes
  void ProcessDMMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;


  // Description:
  // get associated display node or nullptr if not set
  vtkDMMLAnnotationLineDisplayNode* GetAnnotationLineDisplayNode();

  // Description:
  // Create default storage node or nullptr if does not have one
  vtkDMMLStorageNode* CreateDefaultStorageNode() override;

  // Define line between control points
  int  AddLine(int ctrlPtIdStart, int ctrlPtIdEnd,int selectedFlag, int visibleFlag);
  int SetLine(int id, int ctrlPtIdStart, int ctrlPtIdEnd, int selectedFlag, int visibleFlag);

  int SetControlPoint(int id, double newControl[3],int selectedFlag, int visibleFlag);

  int SetControlPointWorldCoordinates(int id, double newControl[3],int selectedFlag, int visibleFlag)
    {
    double localPoint[4]={0,0,0,1};
    this->TransformPointFromWorld(newControl, localPoint);
    return this->SetControlPoint(id, localPoint, selectedFlag, visibleFlag);
    }

  void DeleteLine(int id);
  int GetEndPointsId(vtkIdType lineID, vtkIdType ctrlPtID[2]);
  int GetNumberOfLines();

  // Description:
  // add line display node if not already present
  void CreateAnnotationLineDisplayNode();

  enum
  {
    LINE_SELECTED = vtkDMMLAnnotationControlPointsNode::NUM_CP_ATTRIBUTE_TYPES,
    LINE_VISIBLE,
    NUM_LINE_ATTRIBUTE_TYPES
  };

  const char *GetAttributeTypesEnumAsString(int val) override;

  // Description:
  // Initializes all variables associated with annotations
  void ResetAnnotations() override;

  void Initialize(vtkDMMLScene* dmmlScene) override;

protected:
  vtkDMMLAnnotationLinesNode();
  ~vtkDMMLAnnotationLinesNode() override;
  vtkDMMLAnnotationLinesNode(const vtkDMMLAnnotationLinesNode&);
  void operator=(const vtkDMMLAnnotationLinesNode&);

  // Description:
  // Create Poly data with substructures necessary for this class
  void CreatePolyData();

  // Description:
  // Initializes control points as well as attributes
  void ResetLines();

  // Description:
  // Initializes all attributes
  void ResetLinesAttributesAll();

  bool InitializeLinesFlag;
};

#endif
