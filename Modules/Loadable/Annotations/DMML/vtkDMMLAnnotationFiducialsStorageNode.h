// .NAME vtkDMMLAnnotationFiducialsStorageNode - DMML node for representing a volume storage
// .SECTION Description
// vtkDMMLAnnotationFiducialsStorageNode can be used to read in the old style storage files ending with fcsv

#ifndef __vtkDMMLAnnotationFiducialsStorageNode_h
#define __vtkDMMLAnnotationFiducialsStorageNode_h

#include "vtkCjyxAnnotationsModuleDMMLExport.h"
#include "vtkDMMLAnnotationControlPointsStorageNode.h"

class vtkDMMLAnnotationPointDisplayNode;
class vtkDMMLAnnotationFiducialNode;

/// \ingroup Cjyx_QtModules_Annotation
class  VTK_CJYX_ANNOTATIONS_MODULE_DMML_EXPORT vtkDMMLAnnotationFiducialsStorageNode
  : public vtkDMMLAnnotationControlPointsStorageNode
{
  public:
  static vtkDMMLAnnotationFiducialsStorageNode *New();
  vtkTypeMacro(vtkDMMLAnnotationFiducialsStorageNode,vtkDMMLAnnotationControlPointsStorageNode);

  vtkDMMLNode* CreateNodeInstance() override;

  // Description:
  // Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "AnnotationFiducialsStorage";}

  /// utility method called by the annotation hierarchy node to let this
  /// storage node read a single fiducial's data from an already open file
  int ReadOneFiducial(fstream & fstr, vtkDMMLAnnotationFiducialNode *fiducialNode);

  /// Return true if the node can be read in
  bool CanReadInReferenceNode(vtkDMMLNode* refNode) override;

protected:
  vtkDMMLAnnotationFiducialsStorageNode()  = default;
  ~vtkDMMLAnnotationFiducialsStorageNode() override  = default;
  vtkDMMLAnnotationFiducialsStorageNode(const vtkDMMLAnnotationFiducialsStorageNode&);
  void operator=(const vtkDMMLAnnotationFiducialsStorageNode&);

  int ReadAnnotation(vtkDMMLAnnotationFiducialNode *refNode);
  int ReadAnnotationFiducialsData(vtkDMMLAnnotationFiducialNode *refNode, char line[1024], int labelColumn, int xColumn, int yColumn, int zColumn,
                      int selColumn,  int visColumn, int numColumns);
  int ReadAnnotationFiducialsProperties(vtkDMMLAnnotationFiducialNode *refNode, char line[1024], int &labelColumn,
                        int& xColumn,    int& yColumn,     int& zColumn, int& selColumn, int& visColumn, int& numColumns);

  // Description:
  // Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  // Read data and set it in the referenced node
  int ReadDataInternal(vtkDMMLNode *refNode) override;

};

#endif
