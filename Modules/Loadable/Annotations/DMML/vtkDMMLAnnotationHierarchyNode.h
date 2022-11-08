// .NAME vtkDMMLAnnotationHierarchyNode - DMML node to represent hierarchy of Annotations.
// .SECTION Description
// n/a
//

#ifndef __vtkDMMLAnnotationHierarchyNode_h
#define __vtkDMMLAnnotationHierarchyNode_h

#include "vtkCjyxAnnotationsModuleDMMLExport.h"
#include "vtkDMMLDisplayableHierarchyNode.h"

class vtkAbstractTransform;

/// \ingroup Cjyx_QtModules_Annotation
class  VTK_CJYX_ANNOTATIONS_MODULE_DMML_EXPORT vtkDMMLAnnotationHierarchyNode : public vtkDMMLDisplayableHierarchyNode
{
public:
  static vtkDMMLAnnotationHierarchyNode *New();
  vtkTypeMacro(vtkDMMLAnnotationHierarchyNode,vtkDMMLDisplayableHierarchyNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  virtual const char* GetIcon() {return ":/Icons/Medium/CjyxHierarchy.png";};

  // Description:
  // Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  // Description:
  // Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  // Description:
  // Get node XML tag name (like Volume, Annotation)
  const char* GetNodeTagName() override;

  // Description:
  // Get all top level children associated to this node.
  virtual void GetDirectChildren(vtkCollection *children);

  /// Add into \a children all the children of the hierarchy node.
  /// \sa GetDirectChildren(), GetChildren()
  virtual void GetAllChildren(vtkCollection *children);

  /// Add into \a children all children of the hierarchy node of the first
  /// \a level nodes.
  /// If \a level is <0, then all levels are added.
  /// \sa GetAllChildren(), GetDirectChildren()
  virtual void GetChildren(vtkCollection *children, int level);

  // Description:
  // Delete all children of this node
  // If a child is another hierarchyNode, the parent of it gets set to this' parent
  virtual void DeleteDirectChildren();

  /// From Transformable superclass
  virtual bool CanApplyNonLinearTransforms()const;
  virtual void ApplyTransform(vtkAbstractTransform* transform);

protected:
  vtkDMMLAnnotationHierarchyNode();
  ~vtkDMMLAnnotationHierarchyNode() override;
  vtkDMMLAnnotationHierarchyNode(const vtkDMMLAnnotationHierarchyNode&);
  void operator=(const vtkDMMLAnnotationHierarchyNode&);

};

#endif
