/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLFiberBundleGlyphDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

  =========================================================================auto=*/

#ifndef __vtkDMMLDiffusionTensorVolumeSliceDisplayNode_h
#define __vtkDMMLDiffusionTensorVolumeSliceDisplayNode_h

#include "vtkDMMLGlyphableVolumeSliceDisplayNode.h"
class vtkDMMLDiffusionTensorDisplayPropertiesNode;

class vtkDiffusionTensorGlyph;
class vtkMatrix4x4;
class vtkPolyData;

/// \brief DMML node to represent display properties for tractography.
///
/// vtkDMMLDiffusionTensorVolumeSliceDisplayNode nodes store display properties of trajectories
/// from tractography in diffusion MRI data, including color type (by bundle, by fiber,
/// or by scalar invariants), display on/off for tensor glyphs and display of
/// trajectory as a line or tube.
class VTK_DMML_EXPORT vtkDMMLDiffusionTensorVolumeSliceDisplayNode
  : public vtkDMMLGlyphableVolumeSliceDisplayNode
{
 public:
  static vtkDMMLDiffusionTensorVolumeSliceDisplayNode *New (  );
  vtkTypeMacro ( vtkDMMLDiffusionTensorVolumeSliceDisplayNode,vtkDMMLGlyphableVolumeSliceDisplayNode );
  void PrintSelf ( ostream& os, vtkIndent indent ) override;

  //--------------------------------------------------------------------------
  /// DMMLNode methods
  //--------------------------------------------------------------------------

  vtkDMMLNode* CreateNodeInstance () override;

  ///
  /// Read node attributes from XML (DMML) file
  void ReadXMLAttributes ( const char** atts ) override;

  ///
  /// Write this node's information to a DMML file in XML format.
  void WriteXML ( ostream& of, int indent ) override;


  ///
  /// Copy the node's attributes to this object
  void Copy ( vtkDMMLNode *node ) override;

  ///
  /// Get node XML tag name (like Volume, UnstructuredGrid)
  const char* GetNodeTagName () override {return "DiffusionTensorVolumeSliceDisplayNode";}

  ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  void UpdateReferences() override;

  ///
  /// Finds the storage node and read the data
  void UpdateScene(vtkDMMLScene *scene) override;

  ///
  /// Update the stored reference to another node in the scene
  void UpdateReferenceID(const char *oldID, const char *newID) override;

  ///
  /// alternative method to propagate events generated in Display nodes
  void ProcessDMMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;

  /// Return the glyph producer output for the input image data.
  /// \sa GetOutputPolyData()
  vtkAlgorithmOutput* GetOutputMeshConnection() override;

  ///
  /// Update the pipeline based on this node attributes
  void UpdateAssignedAttribute() override;

  ///
  /// Set ImageData for a volume slice
  void SetSliceImagePort(vtkAlgorithmOutput *imagePort) override;

  ///
  /// Set slice to RAS transformation
  void SetSlicePositionMatrix(vtkMatrix4x4 *matrix) override;

  ///
  /// Set slice to IJK transformation
  void SetSliceGlyphRotationMatrix(vtkMatrix4x4 *matrix) override;

  //--------------------------------------------------------------------------
  /// Display Information: Geometry to display (not mutually exclusive)
  //--------------------------------------------------------------------------


  //--------------------------------------------------------------------------
  /// Display Information: Color Mode
  /// 0) solid color by group 1) color by scalar invariant
  /// 2) color by avg scalar invariant 3) color by other
  //--------------------------------------------------------------------------

  enum
  {
    colorModeSolid = 0,
    colorModeScalar = 1,
    colorModeFunctionOfScalar = 2,
    colorModeUseCellScalars = 3
  };

  //--------------------------------------------------------------------------
  /// Display Information: ColorMode for ALL nodes
  //--------------------------------------------------------------------------

  ///
  /// Color by solid color (for example the whole fiber bundle red. blue, etc.)
  void SetColorModeToSolid ( ) {
    this->SetColorMode ( this->colorModeSolid );
  };

  ///
  /// Color according to the tensors using various scalar invariants.
  void SetColorModeToScalar ( ) {
    this->SetColorMode ( this->colorModeScalar );
  };

  ///
  /// Color according to the tensors using a function of scalar invariants along the tract.
  /// This enables coloring by average FA, for example.
  void SetColorModeToFunctionOfScalar ( ) {
    this->SetColorMode ( this->colorModeFunctionOfScalar );
  };

  ///
  /// Use to color by the active cell scalars.  This is intended to support
  /// external processing of fibers, for example to label each with the distance
  /// of that fiber from an fMRI activation.  Then by making that information
  /// the active cell scalar field, this will allow coloring by that information.
  /// TO DO: make sure this information can be saved with the tract, save name of
  /// active scalar field if needed.
  void SetColorModeToUseCellScalars ( ) {
    this->SetColorMode ( this->colorModeUseCellScalars );
  };



  //--------------------------------------------------------------------------
  /// Display Information: ColorMode for glyphs
  //--------------------------------------------------------------------------


  //--------------------------------------------------------------------------
  /// DMML nodes that are observed
  //--------------------------------------------------------------------------


  /// Node reference to ALL DT nodes

  ///
  /// Get diffusion tensor display DMML object for fiber glyph.
  vtkDMMLDiffusionTensorDisplayPropertiesNode* GetDiffusionTensorDisplayPropertiesNode ( );

  ///
  /// Set diffusion tensor display DMML object for fiber glyph.
  void SetAndObserveDiffusionTensorDisplayPropertiesNodeID ( const char *ID );

  ///
  /// Get ID of diffusion tensor display DMML object for fiber glyph.
  vtkGetStringMacro(DiffusionTensorDisplayPropertiesNodeID);

  ///
  /// Get the number of selected scalar invariants to color a Slice
  static int GetNumberOfScalarInvariants();

  ///
  /// Get the nth scalar invariant to color a Slice
  static int GetNthScalarInvariant(int i);

 protected:
  vtkDMMLDiffusionTensorVolumeSliceDisplayNode ( );
  ~vtkDMMLDiffusionTensorVolumeSliceDisplayNode ( ) override;
  vtkDMMLDiffusionTensorVolumeSliceDisplayNode ( const vtkDMMLDiffusionTensorVolumeSliceDisplayNode& );
  void operator= ( const vtkDMMLDiffusionTensorVolumeSliceDisplayNode& );

  vtkDiffusionTensorGlyph  *DiffusionTensorGlyphFilter;

  /// ALL DMML nodes
  vtkDMMLDiffusionTensorDisplayPropertiesNode *DiffusionTensorDisplayPropertiesNode;
  char *DiffusionTensorDisplayPropertiesNodeID;

  void SetDiffusionTensorDisplayPropertiesNodeID(const char* id);

  static std::vector<int> GetSupportedColorModes();

};

#endif
