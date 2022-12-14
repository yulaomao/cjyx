// .NAME vtkDMMLAnnotationPointDisplayNode - DMML node to represent display properties for tractography.
// .SECTION Description
// vtkDMMLAnnotationPointDisplayNode nodes store display properties of trajectories
// from tractography in diffusion MRI data, including color type (by bundle, by fiber,
// or by scalar invariants), display on/off for tensor glyphs and display of
// trajectory as a line or tube.
//

#ifndef __vtkDMMLAnnotationPointDisplayNode_h
#define __vtkDMMLAnnotationPointDisplayNode_h

#include "vtkDMML.h"
#include "vtkDMMLAnnotationDisplayNode.h"
#include "vtkCjyxAnnotationsModuleDMMLExport.h"

/// \ingroup Cjyx_QtModules_Annotation
class  VTK_CJYX_ANNOTATIONS_MODULE_DMML_EXPORT vtkDMMLAnnotationPointDisplayNode : public vtkDMMLAnnotationDisplayNode
{
 public:
  static vtkDMMLAnnotationPointDisplayNode *New (  );
  vtkTypeMacro ( vtkDMMLAnnotationPointDisplayNode,vtkDMMLAnnotationDisplayNode );
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
  const char* GetNodeTagName() override {return "AnnotationPointDisplay";}

  // Description:
  // Finds the storage node and read the data
  void UpdateScene(vtkDMMLScene *scene) override;

  // Description:
  // alternative method to propagate events generated in Display nodes
  void ProcessDMMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;

  /// Which kind of glyph should be used to display this fiducial?
  /// Vertex2D is supposed to start at 1
  enum GlyphShapes
  {
    GlyphMin = 1,
    Vertex2D = GlyphMin,
    Dash2D,
    Cross2D,
    ThickCross2D,
    Triangle2D,
    Square2D,
    Circle2D,
    Diamond2D,
    Arrow2D,
    ThickArrow2D,
    HookedArrow2D,
    StarBurst2D,
    Sphere3D,
    Diamond3D,
    GlyphMax = Sphere3D,
  };
  /// Return the min/max glyph types, for iterating over them in tcl
  int GetMinimumGlyphType() { return vtkDMMLAnnotationPointDisplayNode::GlyphMin; };
  int GetMaximumGlyphType() { return vtkDMMLAnnotationPointDisplayNode::GlyphMax; };

  /// The glyph type used to display this fiducial
  void SetGlyphType(int type);
  vtkGetMacro(GlyphType, int);
  /// Returns 1 if the type is a 3d one, 0 else
  int GlyphTypeIs3D(int glyphType);
  int GlyphTypeIs3D() { return this->GlyphTypeIs3D(this->GlyphType); };

  /// Return a string representing the glyph type, set it from a string
  const char* GetGlyphTypeAsString();
  const char* GetGlyphTypeAsString(int g);
  void SetGlyphTypeFromString(const char *glyphString);

  /// Get/Set for Symbol scale
  ///  vtkSetMacro(GlyphScale,double);
  void SetGlyphScale(double scale);
  vtkGetMacro(GlyphScale,double);

  /// Create a backup of this node and attach it.
  void CreateBackup() override;
  /// Restore an attached backup of this node.
  void RestoreBackup() override;

  /// Set projection color as fiducial color
  ///\sa SetProjectedColor
  inline void SliceProjectionUseFiducialColorOn();

  /// Manually set projection color
  ///\sa SetProjectedColor
  inline void SliceProjectionUseFiducialColorOff();

  /// Set projection's view different
  /// if under/over/in the plane
  ///\sa SetProjectedColor
  inline void SliceProjectionOutlinedBehindSlicePlaneOn();

  /// Set projection's view the same
  /// if under/over/in the plane
  ///\sa SetProjectedColor
  inline void SliceProjectionOutlinedBehindSlicePlaneOff();

  /// ProjectionUseFiducialColor : Set projection color as fiducial color
  /// ProjectionOutlinedBehindSlicePlane : Different shape and opacity when fiducial
  /// is on top of the slice plane, or under
  /// Projection Off, UseFiducialColor, OutlinedBehindSlicePlane by default
  ///\enum ProjectionFlag
  enum ProjectionFlag
  {
  ProjectionUseFiducialColor = 0x02,
  ProjectionOutlinedBehindSlicePlane = 0x04
  };

 protected:
  vtkDMMLAnnotationPointDisplayNode();
  ~vtkDMMLAnnotationPointDisplayNode() override  = default;
  vtkDMMLAnnotationPointDisplayNode( const vtkDMMLAnnotationPointDisplayNode& );
  void operator= ( const vtkDMMLAnnotationPointDisplayNode& );

  double GlyphScale;
  int GlyphType;
  static const char* GlyphTypesNames[GlyphMax+2];
};

//----------------------------------------------------------------------------
void vtkDMMLAnnotationPointDisplayNode
::SliceProjectionUseFiducialColorOn()
{
  this->SetSliceProjection( this->GetSliceProjection() |
                            vtkDMMLAnnotationPointDisplayNode::ProjectionUseFiducialColor);
}

//----------------------------------------------------------------------------
void vtkDMMLAnnotationPointDisplayNode
::SliceProjectionUseFiducialColorOff()
{
  this->SetSliceProjection( this->GetSliceProjection() &
                            ~vtkDMMLAnnotationPointDisplayNode::ProjectionUseFiducialColor);
}

//----------------------------------------------------------------------------
void vtkDMMLAnnotationPointDisplayNode
::SliceProjectionOutlinedBehindSlicePlaneOn()
{
  this->SetSliceProjection( this->GetSliceProjection() |
                            vtkDMMLAnnotationPointDisplayNode::ProjectionOutlinedBehindSlicePlane);
}

//----------------------------------------------------------------------------
void vtkDMMLAnnotationPointDisplayNode
::SliceProjectionOutlinedBehindSlicePlaneOff()
{
  this->SetSliceProjection( this->GetSliceProjection() &
                            ~vtkDMMLAnnotationPointDisplayNode::ProjectionOutlinedBehindSlicePlane);
}

#endif
