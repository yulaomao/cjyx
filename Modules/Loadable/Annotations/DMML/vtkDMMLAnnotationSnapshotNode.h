// .NAME vtkDMMLAnnotationSnapshotNode - DMML node to represent scene snapshot including description and screenshot
// .SECTION Description
// n/A
//

#ifndef __vtkDMMLAnnotationSnapshotNode_h
#define __vtkDMMLAnnotationSnapshotNode_h

#include "vtkCjyxAnnotationsModuleDMMLExport.h"
#include "vtkDMMLAnnotationControlPointsNode.h"
#include "vtkDMMLAnnotationNode.h"

#include <vtkStdString.h>
class vtkImageData;
class vtkStringArray;
class vtkDMMLStorageNode;

/// \ingroup Cjyx_QtModules_Annotation
class  VTK_CJYX_ANNOTATIONS_MODULE_DMML_EXPORT vtkDMMLAnnotationSnapshotNode : public vtkDMMLAnnotationNode
{
public:
  static vtkDMMLAnnotationSnapshotNode *New();
  vtkTypeMacro(vtkDMMLAnnotationSnapshotNode,vtkDMMLAnnotationNode);

  //--------------------------------------------------------------------------
  // DMMLNode methods
  //--------------------------------------------------------------------------

  vtkDMMLNode* CreateNodeInstance() override;
  // Description:
  // Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "AnnotationSnapshot";}

  const char* GetIcon() override {return ":/Icons/ViewCamera.png";}

  void SetSnapshotDescription(const vtkStdString& newDescription);
  vtkGetMacro(SnapshotDescription, vtkStdString)

  void WriteXML(ostream& of, int nIndent) override;
  void ReadXMLAttributes(const char** atts) override;

  /// The attached screenshot
  virtual void SetScreenShot(vtkImageData* );
  vtkGetObjectMacro(ScreenShot, vtkImageData);

  /// The ScaleFactor of the Screenshot
  vtkGetMacro(ScaleFactor, double);
  vtkSetMacro(ScaleFactor, double);

  /// The screenshot type
  /// 0: 3D View
  /// 1: Red Slice View
  /// 2: Yellow Slice View
  /// 3: Green Slice View
  /// 4: Full layout
  // TODO use an enum for the types
  void SetScreenShotType(int type);
  vtkGetMacro(ScreenShotType, int);

  /// Create default storage node or nullptr if does not have one
  vtkDMMLStorageNode* CreateDefaultStorageNode() override;

  enum
  {
    SnapshotNodeAddedEvent = 0,
    ValueModifiedEvent,
  };

protected:
  vtkDMMLAnnotationSnapshotNode();
  ~vtkDMMLAnnotationSnapshotNode() override;
  vtkDMMLAnnotationSnapshotNode(const vtkDMMLAnnotationSnapshotNode&);
  void operator=(const vtkDMMLAnnotationSnapshotNode&);

  /// The associated Description
  vtkStdString SnapshotDescription;

  /// The vtkImageData of the screenshot
  vtkImageData* ScreenShot;

  /// The type of the screenshot
  /// 0: 3D View
  /// 1: Red Slice View
  /// 2: Yellow Slice View
  /// 3: Green Slice View
  /// 4: Full layout
  int ScreenShotType;

  double ScaleFactor;

};

#endif
