// .NAME vtkDMMLAnnotationDisplayNode - DMML node to represent display properties for tractography.
// .SECTION Description
// vtkDMMLAnnotationDisplayNode nodes store display properties of trajectories
// from tractography in diffusion MRI data, including color type (by bundle, by fiber,
// or by scalar invariants), display on/off for tensor glyphs and display of
// trajectory as a line or tube.
//

#ifndef __vtkDMMLAnnotationDisplayNode_h
#define __vtkDMMLAnnotationDisplayNode_h

#include "vtkDMML.h"
#include "vtkDMMLModelDisplayNode.h"
#include "vtkDMMLDisplayableNode.h"
#include "vtkDMMLDisplayNode.h"
#include "vtkCjyxAnnotationsModuleDMMLExport.h"

#define vtkSetAndPropagateVector3Macro(name,type) \
virtual void SetAndPropagateSuper##name (type _arg1, type _arg2, type _arg3) \
  { \
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Super" << #name " to (" << _arg1 << "," << _arg2 << "," << _arg3 << ")"); \
  if ((this->Super##name[0] != _arg1)||(this->Super##name[1] != _arg2)||(this->Super##name[2] != _arg3)) \
    { \
    this->name[0] = _arg1; \
    this->name[1] = _arg2; \
    this->name[2] = _arg3; \
    this->Super##name[0] = _arg1; \
    this->Super##name[1] = _arg2; \
    this->Super##name[2] = _arg3; \
    vtkDMMLDisplayableNode *displayableNode = this->GetDisplayableNode(); \
    if (!displayableNode) \
      { \
      return; \
      } \
    for (int i=0; i<displayableNode->GetNumberOfDisplayNodes(); i++) \
      { \
      vtkDMMLDisplayNode *displayNode = displayableNode->GetNthDisplayNode(i); \
      if (!displayNode || displayNode==this) \
        { \
        continue; \
        } \
      displayNode->Set##name(this->Super##name); \
      } \
    this->Modified(); \
    } \
  }; \
virtual void SetAndPropagateSuper##name (type _arg[3]) \
  { \
  this->SetAndPropagateSuper##name (_arg[0], _arg[1], _arg[2]);\
  }

#define vtkSetAndPropagateMacro(name,type) \
virtual void SetAndPropagateSuper##name (type _arg) \
  { \
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Super" << #name " to (" << _arg << ")"); \
  if (this->Super##name != _arg) \
    { \
    this->name = _arg; \
    this->Super##name = _arg; \
    vtkDMMLDisplayableNode *displayableNode = this->GetDisplayableNode(); \
    if (!displayableNode) \
      { \
      return; \
      } \
    for (int i=0; i<displayableNode->GetNumberOfDisplayNodes(); i++) \
      { \
      vtkDMMLDisplayNode *displayNode = displayableNode->GetNthDisplayNode(i); \
      if (!displayNode || displayNode==this) \
        { \
        continue; \
        } \
      displayNode->Set##name(this->Super##name); \
      } \
    this->Modified(); \
    } \
  }; \


/// \ingroup Cjyx_QtModules_Annotation
class  VTK_CJYX_ANNOTATIONS_MODULE_DMML_EXPORT vtkDMMLAnnotationDisplayNode : public vtkDMMLModelDisplayNode
{
 public:
  static vtkDMMLAnnotationDisplayNode *New (  );
  vtkTypeMacro ( vtkDMMLAnnotationDisplayNode,vtkDMMLModelDisplayNode );
  void PrintSelf ( ostream& os, vtkIndent indent ) override;

  //--------------------------------------------------------------------------
  // DMMLNode methods
  //--------------------------------------------------------------------------

  vtkDMMLNode* CreateNodeInstance () override;

  // Description:
  // Read node attributes from XML (DMML) file
  void ReadXMLAttributes ( const char** atts ) override;

  // Description:
  // Write this node's information to a DMML file in XML format.
  void WriteXML ( ostream& of, int indent ) override;


  // Description:
  // Copy the node's attributes to this object
  void Copy ( vtkDMMLNode *node ) override;

  // Description:
  // Get node XML tag name (like Volume, Annotation)
  const char* GetNodeTagName() override {return "AnnotationDisplay";}

  // Description:
  // Finds the storage node and read the data
  void UpdateScene(vtkDMMLScene *scene) override;

  // Description:
  // alternative method to propagate events generated in Display nodes
  void ProcessDMMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;

  // Functionality for backups of this node
  /// Creates a backup of the current DMML state of this node and keeps a reference
  virtual void CreateBackup(){};
  virtual void RestoreBackup(){};
  void ClearBackup();
  /// Returns the associated backup of this node
  vtkDMMLAnnotationDisplayNode * GetBackup();

  //
  // Set and propagate attributes to all other displayNodes
  //

  vtkSetAndPropagateVector3Macro(Color, double);
  vtkGetVector3Macro(SuperColor, double);

  vtkSetAndPropagateVector3Macro(SelectedColor, double);
  vtkGetVector3Macro(SuperSelectedColor, double);

  vtkSetAndPropagateMacro(Opacity, double);
  vtkGetMacro(SuperOpacity, double);

  vtkSetAndPropagateMacro(Ambient, double);
  vtkGetMacro(SuperAmbient, double);

  vtkSetAndPropagateMacro(Diffuse, double);
  vtkGetMacro(SuperDiffuse, double);

  vtkSetAndPropagateMacro(Specular, double);
  vtkGetMacro(SuperSpecular, double);

  vtkSetAndPropagateMacro(Power, double);
  vtkGetMacro(SuperPower, double);

  vtkSetAndPropagateMacro(SelectedAmbient, double);
  vtkGetMacro(SuperSelectedAmbient, double);

  vtkSetAndPropagateMacro(SelectedSpecular, double);
  vtkGetMacro(SuperSelectedSpecular, double);

  /// Set SliceProjection flag
  /// Off by default
  /// Not all subclasses have projection behavior
  /// Please refer to subclasses for more information
  /// \sa SliceIntersectionVisibilty, ProjectedColor
  vtkSetMacro(SliceProjection, int);
  vtkGetMacro(SliceProjection, int);

  /// \enum ProjectionFlag
  enum ProjectionFlag
  {
  ProjectionOff = 0x00,
  ProjectionOn = 0x01
  };

  /// Set SliceProjection to On
  inline void SliceProjectionOn();

  /// Set SliceProjection to Off
  inline void SliceProjectionOff();

  /// Set color of the projection on the 2D viewers
  /// White (1.0, 1.0, 1.0) by default.
  vtkSetVector3Macro(ProjectedColor, double);
  vtkGetVector3Macro(ProjectedColor, double);

  /// Set opacity of projection on the 2D viewers
  /// 1.0 by default
  vtkSetMacro(ProjectedOpacity, double);
  vtkGetMacro(ProjectedOpacity, double);

 protected:
  vtkDMMLAnnotationDisplayNode();
  ~vtkDMMLAnnotationDisplayNode() override;
  vtkDMMLAnnotationDisplayNode( const vtkDMMLAnnotationDisplayNode& );
  void operator= ( const vtkDMMLAnnotationDisplayNode& );

  vtkDMMLAnnotationDisplayNode* m_Backup;

  double SuperColor[3];
  double SuperSelectedColor[3];

  double SuperOpacity;
  double SuperAmbient;
  double SuperDiffuse;
  double SuperSpecular;
  double SuperPower;
  double SuperSelectedAmbient;
  double SuperSelectedSpecular;


  int SliceProjection;
  double ProjectedColor[3];
  double ProjectedOpacity;
};

//----------------------------------------------------------------------------
void vtkDMMLAnnotationDisplayNode
::SliceProjectionOn()
{
  this->SetSliceProjection( this->GetSliceProjection() |
                            vtkDMMLAnnotationDisplayNode::ProjectionOn);
}

//----------------------------------------------------------------------------
void vtkDMMLAnnotationDisplayNode
::SliceProjectionOff()
{
  this->SetSliceProjection( this->GetSliceProjection() &
                            ~vtkDMMLAnnotationDisplayNode::ProjectionOn);
}

#endif
