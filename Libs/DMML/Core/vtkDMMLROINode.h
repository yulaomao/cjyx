#ifndef __vtkDMMLROINode_h
#define __vtkDMMLROINode_h

#include "vtkDMMLTransformableNode.h"

class vtkPlanes;

/// \brief DMML node to represent a 3D ROI.
///
/// Model nodes describe ROI data.  They indicate where the ROI is
/// located and the size of the ROI.
class VTK_DMML_EXPORT vtkDMMLROINode : public vtkDMMLTransformableNode
{
public:
  static vtkDMMLROINode *New();
  vtkTypeMacro(vtkDMMLROINode,vtkDMMLTransformableNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //--------------------------------------------------------------------------
  /// DMML methods
  //--------------------------------------------------------------------------
  vtkDMMLNode* CreateNodeInstance() override;

  ///
  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  virtual void ReadXMLString( const char *keyValuePairs);

  ///
  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkDMMLNode *node) override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "DMMLROINode";}

  ///
  ///
  void UpdateScene(vtkDMMLScene *scene) override;

  ///
  /// update display node ids
  void UpdateReferences() override;

  ///
  /// Indicates if the ROI is visible
  vtkBooleanMacro(Visibility, int);
  vtkGetMacro(Visibility, int);
  vtkSetMacro(Visibility, int);

  ///
  /// Indicates if the ROI is updated interactively
  vtkBooleanMacro(InteractiveMode, int);
  vtkGetMacro(InteractiveMode, int);
  vtkSetMacro(InteractiveMode, int);

  ///
  /// Get/Set for ROI Position in RAS coordinates
  /// Note: The ROI Position is the center of the ROI
  void SetXYZ(double X, double Y, double Z);
  void SetXYZ(double* XYZ);
  vtkGetVectorMacro(XYZ,double,3);

  ///
  /// Get/Set for radius of the ROI in RAS coordinates
  void SetRadiusXYZ(double RadiusX, double RadiusY, double RadiusZ);
  void SetRadiusXYZ(double* RadiusXYZ);
  vtkGetVectorMacro(RadiusXYZ,double,3);

  ///
  /// Get/Set for ROI Position in IJK coordinates
  void SetIJK(double I, double J, double K);
  void SetIJK(double* IJK);
  vtkGetVectorMacro(IJK,double,3);

  ///
  /// Get/Set for radius of the ROI in IJK coordinates
  void SetRadiusIJK(double RadiusI, double RadiusJ, double RadiusK);
  void SetRadiusIJK(double* RadiusIJK);
  vtkGetVectorMacro(RadiusIJK,double,3);

  ///
  /// Set/Get the InsideOut flag. This data member is used in conjunction
  /// with the GetPlanes() method. When off, the normals point out of the
  /// box. When on, the normals point into the hexahedron.  InsideOut is off
  /// by default.
  vtkSetMacro(InsideOut,int);
  vtkGetMacro(InsideOut,int);
  vtkBooleanMacro(InsideOut,int);

  ///
  /// Get/Set for LabelText
  vtkSetStringMacro(LabelText);
  vtkGetStringMacro(LabelText);

  void ProcessDMMLEvents ( vtkObject *caller, unsigned long event, void *callData ) override;

  vtkGetStringMacro(VolumeNodeID);
  vtkSetStringMacro(VolumeNodeID);

  ///
  /// transform utility functions
  bool CanApplyNonLinearTransforms()const override;
  void ApplyTransform(vtkAbstractTransform* transform) override;

  /// Description
  /// get transformed planes for the ROI region
  void GetTransformedPlanes(vtkPlanes *planes);

protected:
  vtkDMMLROINode();
  ~vtkDMMLROINode() override;
  vtkDMMLROINode(const vtkDMMLROINode&);
  void operator=(const vtkDMMLROINode&);
  int Visibility;
  int InteractiveMode;

  ///
  /// The location of the ROI centroid in RAS space
  /// Note: The ROI Position is the center of the ROI
  double XYZ[3];
  ///
  /// The raidus of  of the ROI box in RAS space
  double RadiusXYZ[3];
  ///
  /// The location of the ROI centroid in IJK space
  /// Note: The ROI Position is the center of the ROI
  double IJK[3];
  ///
  /// The radius of the ROI box in IJK space
  double RadiusIJK[3];

  /// Control the orientation of the normals
  int InsideOut;

  char *LabelText;

  ///
  /// The ID of the volume associated with the ROI
  char *VolumeNodeID;
};

#endif
