// .NAME vtkDMMLAnnotationControlPointsStorageNode - DMML node for representing a volume storage
// .SECTION Description
// vtkDMMLAnnotationControlPointsStorageNode nodes describe the annotation storage
// node that allows to read/write point data from/to file.

#ifndef __vtkDMMLAnnotationControlPointsStorageNode_h
#define __vtkDMMLAnnotationControlPointsStorageNode_h

#include "vtkCjyxAnnotationsModuleDMMLExport.h"
#include "vtkDMMLAnnotationStorageNode.h"

class vtkDMMLAnnotationPointDisplayNode;
class vtkDMMLAnnotationControlPointsNode;

/// \ingroup Cjyx_QtModules_Annotation
class  VTK_CJYX_ANNOTATIONS_MODULE_DMML_EXPORT vtkDMMLAnnotationControlPointsStorageNode : public vtkDMMLAnnotationStorageNode
{
public:
  static vtkDMMLAnnotationControlPointsStorageNode *New();
  vtkTypeMacro(vtkDMMLAnnotationControlPointsStorageNode,vtkDMMLAnnotationStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  // Description:
  // Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "AnnotationControlPointsStorage";}

  // Initialize all the supported write file types
  bool CanReadInReferenceNode(vtkDMMLNode* refNode) override;

protected:
  vtkDMMLAnnotationControlPointsStorageNode();
  ~vtkDMMLAnnotationControlPointsStorageNode() override;
  vtkDMMLAnnotationControlPointsStorageNode(const vtkDMMLAnnotationControlPointsStorageNode&);
  void operator=(const vtkDMMLAnnotationControlPointsStorageNode&);

  const char* GetAnnotationStorageType() { return "point"; }

  int WriteAnnotationPointDisplayProperties(fstream & of, vtkDMMLAnnotationPointDisplayNode *refNode, std::string preposition);
  int WriteAnnotationControlPointsProperties(fstream & of, vtkDMMLAnnotationControlPointsNode *refNode);
  int WriteAnnotationControlPointsData(fstream& of, vtkDMMLAnnotationControlPointsNode *refNode);

  int ReadAnnotation(vtkDMMLAnnotationControlPointsNode *refNode);
  int ReadAnnotationControlPointsData(vtkDMMLAnnotationControlPointsNode *refNode, char line[1024], int typeColumn, int xColumn, int yColumn, int zColumn,
                      int selColumn,  int visColumn, int numColumns);
  int ReadAnnotationPointDisplayProperties(vtkDMMLAnnotationPointDisplayNode *refNode, std::string lineString, std::string preposition);
  int ReadAnnotationControlPointsProperties(vtkDMMLAnnotationControlPointsNode *refNode, char line[1024], int &typeColumn,
                        int& xColumn,    int& yColumn,     int& zColumn, int& selColumn, int& visColumn, int& numColumns);

  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkDMMLNode *refNode) override;

  int WriteAnnotationDataInternal(vtkDMMLNode *refNode, fstream &of) override;
};

#endif
