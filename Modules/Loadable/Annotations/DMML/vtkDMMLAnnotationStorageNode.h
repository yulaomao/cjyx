// .NAME vtkDMMLAnnotationStorageNode - DMML node for representing a volume storage
// .SECTION Description
// vtkDMMLAnnotationStorageNode nodes describe the annotation storage
// node that allows to read/write point data from/to file.

#ifndef __vtkDMMLAnnotationStorageNode_h
#define __vtkDMMLAnnotationStorageNode_h

#include "vtkCjyxAnnotationsModuleDMMLExport.h"
#include "vtkDMMLStorageNode.h"

class vtkDMMLAnnotationTextDisplayNode;
class vtkDMMLAnnotationDisplayNode;
class vtkDMMLAnnotationNode;

/// \ingroup Cjyx_QtModules_Annotation
class  VTK_CJYX_ANNOTATIONS_MODULE_DMML_EXPORT vtkDMMLAnnotationStorageNode : public vtkDMMLStorageNode
{
public:
  static vtkDMMLAnnotationStorageNode *New();
  vtkTypeMacro(vtkDMMLAnnotationStorageNode,vtkDMMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  // Description:
  // Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "AnnotationStorage";}

  /// Return true if the node can be read in
  bool CanReadInReferenceNode(vtkDMMLNode* refNode) override;

protected:
  vtkDMMLAnnotationStorageNode();
  ~vtkDMMLAnnotationStorageNode() override;
  vtkDMMLAnnotationStorageNode(const vtkDMMLAnnotationStorageNode&);
  void operator=(const vtkDMMLAnnotationStorageNode&);

  int WriteAnnotationDisplayProperties(fstream & of, vtkDMMLAnnotationDisplayNode *refNode, std::string preposition);
  int WriteAnnotationTextDisplayProperties(fstream & of, vtkDMMLAnnotationTextDisplayNode *refNode, std::string preposition);

  int WriteAnnotationTextProperties(fstream & of, vtkDMMLAnnotationNode *refNode);
  int WriteAnnotationData(fstream& of, vtkDMMLAnnotationNode *refNode);
  int OpenFileToWrite(fstream& of);

  // Description:
  // Read data related to vtkDMMLAnnotationDisplayNode
  int ReadAnnotationDisplayProperties(vtkDMMLAnnotationDisplayNode *annotationDisplayNode, std::string lineString, std::string preposition);
  int ReadAnnotationTextDisplayProperties(vtkDMMLAnnotationTextDisplayNode *annotationDisplayNode, std::string lineString, std::string preposition);

  int ReadAnnotationTextData(vtkDMMLAnnotationNode *refNode, char line[1024], int typeColumn, int textColumn,  int selColumn,
              int visColumn, int numColumns);
  int ReadAnnotationTextProperties(vtkDMMLAnnotationNode *annotationNode, char line[1024], int &typeColumn, int& annotationColumn, int& selColumn, int& visColumn, int& columnNumber);
  // Description:
  // assumes that ResetAnnotations is executed
  int ReadAnnotation(vtkDMMLAnnotationNode *refNode);

  int OpenFileToRead(fstream& of, vtkDMMLNode *refNode);
  const char* GetAnnotationStorageType() { return "text"; }

  /// Initialize all the supported read file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkDMMLNode *refNode) override;

  /// Write data from a referenced node
  int WriteDataInternal(vtkDMMLNode *refNode) override;
  /// Write data from a referenced node into a passed stream
  virtual int WriteAnnotationDataInternal(vtkDMMLNode *refNode, fstream &of);

};

#endif
