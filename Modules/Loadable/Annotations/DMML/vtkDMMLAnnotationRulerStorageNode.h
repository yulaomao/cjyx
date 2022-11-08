// .NAME vtkDMMLAnnotationRulerStorageNode - DMML node for representing a volume storage
// .SECTION Description
// vtkDMMLAnnotationRulerStorageNode nodes describe the annotation storage
// node that allows to read/write point data from/to file.

#ifndef __vtkDMMLAnnotationRulerStorageNode_h
#define __vtkDMMLAnnotationRulerStorageNode_h

#include "vtkCjyxAnnotationsModuleDMMLExport.h"
#include "vtkDMMLAnnotationLinesStorageNode.h"

class vtkDMMLAnnotationRulerNode;

/// \ingroup Cjyx_QtModules_Annotation
class  VTK_CJYX_ANNOTATIONS_MODULE_DMML_EXPORT vtkDMMLAnnotationRulerStorageNode
  : public vtkDMMLAnnotationLinesStorageNode
{
public:
  static vtkDMMLAnnotationRulerStorageNode *New();
  vtkTypeMacro(vtkDMMLAnnotationRulerStorageNode,vtkDMMLAnnotationLinesStorageNode);
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
  const char* GetNodeTagName() override {return "AnnotationRulerStorage";}

  /// Read a single ruler from an open list file, called by the hierarchy
  /// storage node
  int ReadOneRuler(fstream & fstr, vtkDMMLAnnotationRulerNode *refNode);

  /// Return true if the node can be read in
  bool CanReadInReferenceNode(vtkDMMLNode* refNode) override;

protected:
  vtkDMMLAnnotationRulerStorageNode();
  ~vtkDMMLAnnotationRulerStorageNode() override;
  vtkDMMLAnnotationRulerStorageNode(const vtkDMMLAnnotationRulerStorageNode&);
  void operator=(const vtkDMMLAnnotationRulerStorageNode&);

  const char* GetAnnotationStorageType() { return "ruler"; }

  int WriteAnnotationRulerProperties(fstream & of, vtkDMMLAnnotationRulerNode *refNode);
  int WriteAnnotationRulerData(fstream& of, vtkDMMLAnnotationRulerNode *refNode);

  int ReadAnnotation(vtkDMMLAnnotationRulerNode *refNode);
  int ReadAnnotationRulerData(vtkDMMLAnnotationRulerNode *refNode, char line[1024], int typeColumn, int line1IDColumn, int selColumn,  int visColumn, int numColumns);
  int ReadAnnotationRulerProperties(vtkDMMLAnnotationRulerNode *refNode, char line[1024], int &typeColumn, int& line1IDColumn, int& selColumn, int& visColumn, int& numColumns);

  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkDMMLNode *refNode) override;

  /// Write data from the referenced node into the stream
  int WriteAnnotationDataInternal(vtkDMMLNode *refNode, fstream & of) override;
};

#endif
