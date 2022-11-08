// .NAME vtkDMMLAnnotationLinesStorageNode - DMML node for representing a volume storage
// .SECTION Description
// vtkDMMLAnnotationLinesStorageNode nodes describe the annotation storage
// node that allows to read/write point data from/to file.

#ifndef __vtkDMMLAnnotationLinesStorageNode_h
#define __vtkDMMLAnnotationLinesStorageNode_h

#include "vtkCjyxAnnotationsModuleDMMLExport.h"
#include "vtkDMMLAnnotationControlPointsStorageNode.h"

class vtkDMMLAnnotationLineDisplayNode;
class vtkDMMLAnnotationLinesNode;

/// \ingroup Cjyx_QtModules_Annotation
class  VTK_CJYX_ANNOTATIONS_MODULE_DMML_EXPORT vtkDMMLAnnotationLinesStorageNode
  : public vtkDMMLAnnotationControlPointsStorageNode
{
  public:
  static vtkDMMLAnnotationLinesStorageNode *New();
  vtkTypeMacro(vtkDMMLAnnotationLinesStorageNode,vtkDMMLAnnotationControlPointsStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  // Description:
  // Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  // Description:
  // Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  // Description:
  // Copy the node's attributes to this object
  void Copy(vtkDMMLNode *node) override;

  // Description:
  // Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "AnnotationLinesStorage";}

  /// Return true if the node can be read in
  bool CanReadInReferenceNode(vtkDMMLNode* refNode) override;

protected:
  vtkDMMLAnnotationLinesStorageNode();
  ~vtkDMMLAnnotationLinesStorageNode() override;
  vtkDMMLAnnotationLinesStorageNode(const vtkDMMLAnnotationLinesStorageNode&);
  void operator=(const vtkDMMLAnnotationLinesStorageNode&);

  const char* GetAnnotationStorageType() { return "line"; }

  int WriteAnnotationLineDisplayProperties(fstream & of, vtkDMMLAnnotationLineDisplayNode *refNode, std::string preposition);
  int WriteAnnotationLinesProperties(fstream & of, vtkDMMLAnnotationLinesNode *refNode);
  int WriteAnnotationLinesData(fstream& of, vtkDMMLAnnotationLinesNode *refNode);

  int ReadAnnotation(vtkDMMLAnnotationLinesNode *refNode);
  int ReadAnnotationLinesData(vtkDMMLAnnotationLinesNode *refNode, char line[1024], int typeColumn, int startIDColumn, int endIDColumn, int selColumn,  int visColumn, int numColumns);
  int ReadAnnotationLineDisplayProperties(vtkDMMLAnnotationLineDisplayNode *refNode, std::string lineString, std::string preposition);
  int ReadAnnotationLinesProperties(vtkDMMLAnnotationLinesNode *refNode, char line[1024], int &typeColumn, int& startIDColumn,    int& endIDColumn, int& selColumn, int& visColumn, int& numColumns);

  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkDMMLNode *refNode) override;

  // Description:
  int WriteAnnotationDataInternal(vtkDMMLNode *refNode, fstream & of) override;
};

#endif



