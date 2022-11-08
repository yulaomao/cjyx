// .NAME vtkDMMLAnnotationNode - DMML node to represent a fiber bundle from tractography in DTI data.
// .SECTION Description
// Annotation nodes contains control points, internally represented as vtkPolyData.
// A Annotation node contains many control points  and forms the smallest logical unit of tractography
// that DMML will manage/read/write. Each control point has accompanying data.
// Visualization parameters for these nodes are controlled by the vtkDMMLAnnotationTextDisplayNode class.
//

#ifndef __vtkDMMLAnnotationNode_h
#define __vtkDMMLAnnotationNode_h

// DMML includes
#include "vtkDMMLModelNode.h"
class vtkDMMLCameraNode;
class vtkDMMLSliceNode;

// Annotations includes
#include "vtkCjyxAnnotationsModuleDMMLExport.h"
class vtkDMMLAnnotationTextDisplayNode;

// VTK includes
#include <vtkSmartPointer.h>
class vtkCellArray;
class vtkPoints;
class vtkStringArray;

/// \ingroup Cjyx_QtModules_Annotation
class  VTK_CJYX_ANNOTATIONS_MODULE_DMML_EXPORT vtkDMMLAnnotationNode : public vtkDMMLModelNode
{
public:
  static vtkDMMLAnnotationNode *New();
  vtkTypeMacro(vtkDMMLAnnotationNode,vtkDMMLModelNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  // Description:
  // Just prints short summary
  virtual void PrintAnnotationInfo(ostream& os, vtkIndent indent, int titleFlag = 1);

  virtual const char* GetIcon() {return "";}

  //--------------------------------------------------------------------------
  // DMMLNode methods
  //--------------------------------------------------------------------------

  vtkDMMLNode* CreateNodeInstance() override;
  // Description:
  // Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "Annotation";}

  // Description:
  // Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  // Description:
  // Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;


  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkDMMLNode::CopyContent
  vtkDMMLCopyContentMacro(vtkDMMLAnnotationNode);

  void UpdateScene(vtkDMMLScene *scene) override;

  // Description:
  // alternative method to propagate events generated in Display nodes
  void ProcessDMMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;

  // vtkDMMLModelNode overrides it and it handles models only, while in annotations
  // we have all kinds of nodes (e.g., screenshot), so we need to revert to the generic
  // storable node implementation.
  std::string GetDefaultStorageNodeClassName(const char* filename /* =nullptr */) override;

  // Description:
  // Create default storage node or nullptr if does not have one
  vtkDMMLStorageNode* CreateDefaultStorageNode() override;

  /// Returns false since in general cannot apply non linear transforms
  /// \sa ApplyTransformMatrix, ApplyTransform
  bool CanApplyNonLinearTransforms()const override {return false;}

  int AddText(const char *newText,int selectedFlag, int visibleFlag);
  void SetText(int id, const char *newText,int selectedFlag, int visibleFlag);
  vtkStdString GetText(int id);
  int DeleteText(int id);

  int GetNumberOfTexts();

  enum
  {
    TEXT_SELECTED = 0,
    TEXT_VISIBLE,
    NUM_TEXT_ATTRIBUTE_TYPES,
     LockModifiedEvent,
  };
  virtual const char *GetAttributeTypesEnumAsString(int val);
  vtkDataArray* GetAnnotationAttributes(int att);
  int GetAnnotationAttribute(vtkIdType id, int att);
  void SetAnnotationAttribute(vtkIdType id, int att, double value);
  int DeleteAttribute(vtkIdType idAttEntry,  vtkIdType idAttType);

  // Description:
  // Initializes all variables associated with annotations
  virtual void ResetAnnotations();

  // Description:
  // add display node if not already present
  void CreateAnnotationTextDisplayNode();

  vtkDMMLAnnotationTextDisplayNode* GetAnnotationTextDisplayNode();

  /// Set the text scale of the associated text.
  void SetTextScale(double textScale);
  /// Get the text scale of the associated text.
  double GetTextScale();


  // Description:
  // Reference of this annotation - can be an image, model, scene ,  ...
  vtkGetStringMacro (ReferenceNodeID);
  vtkSetStringMacro (ReferenceNodeID);

  /// If set to 1 then parameters should not be changed.
  /// Fires vtkDMMLAnnotationNode::LockModifiedEvent if changed except if
  /// GetDisableModifiedEvent() is true.
  vtkGetMacro(Locked, int);
  void SetLocked(int init);

  virtual void Initialize(vtkDMMLScene* dmmlScene);

  // Functionality for backups of this node
  /// Creates a backup of the current DMML state of this node and keeps a reference
  void CreateBackup();
  void ClearBackup();
  /// Returns the associated backup of this node
  vtkDMMLAnnotationNode * GetBackup();
  /// Restore an attached backup of this node.
  void RestoreBackup();

  // Functionality to save the current view
  /// Saves the current view.
  void SaveView();
  /// Restores a previously saved view.
  void RestoreView();

protected:
  vtkDMMLAnnotationNode();
  ~vtkDMMLAnnotationNode() override;
  vtkDMMLAnnotationNode(const vtkDMMLAnnotationNode&);
  void operator=(const vtkDMMLAnnotationNode&);

  // Description:
  // Initializes Text as  well as attributes
  // void ResetAnnotations();

  // Description:
  // Only initializes attributes with ID
  void ResetAttributes(int id);
  // Description:
  // Initializes all attributes
  void ResetTextAttributesAll();
  void SetAttributeSize(vtkIdType  id, vtkIdType n);

  void CreatePolyData();
  vtkPoints* GetPoints();
  vtkCellArray* GetLines();

  vtkStringArray *TextList;
  char *ReferenceNodeID;

  int Locked;

  vtkDMMLAnnotationNode * m_Backup;

  vtkSmartPointer<vtkDMMLSliceNode> m_RedSliceNode;
  vtkSmartPointer<vtkDMMLSliceNode> m_YellowSliceNode;
  vtkSmartPointer<vtkDMMLSliceNode> m_GreenSliceNode;
  vtkSmartPointer<vtkDMMLCameraNode> m_CameraNode;

};

#endif
