/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLGlyphableVolumeDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkDMMLGlyphableVolumeDisplayNode_h
#define __vtkDMMLGlyphableVolumeDisplayNode_h

// DMML includes
#include "vtkDMMLScalarVolumeDisplayNode.h"
class vtkDMMLColorNode;
class vtkDMMLGlyphableVolumeSliceDisplayNode;
class vtkDMMLVolumeNode;

// STD includes
#include <vector>

/// \brief DMML node for representing a volume display attributes.
///
/// vtkDMMLGlyphableVolumeDisplayNode nodes describe how volume is displayed.
class VTK_DMML_EXPORT vtkDMMLGlyphableVolumeDisplayNode : public vtkDMMLScalarVolumeDisplayNode
{
  public:
  static vtkDMMLGlyphableVolumeDisplayNode *New();
  vtkTypeMacro(vtkDMMLGlyphableVolumeDisplayNode,vtkDMMLScalarVolumeDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  ///
  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkDMMLNode *node) override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "GlyphableVolumeDisplay";}

  ///
  /// Update the stored reference to another node in the scene
  void UpdateReferenceID(const char *oldID, const char *newID) override;

  //--------------------------------------------------------------------------
  /// Display Information
  //--------------------------------------------------------------------------

  /// Set/Get visualization Mode
  enum
    {
    visModeScalar = 0,
    visModeGlyph = 1,
    visModeBoth = 2
    };

  vtkGetMacro(VisualizationMode, int);
  vtkSetMacro(VisualizationMode, int);

  //virtual vtkPolyData* ExecuteGlyphPipeLineAndGetPolyData( vtkImageData* );

  void SetVisualizationModeToScalarVolume() {
    this->SetVisualizationMode(this->visModeScalar);
  };
  void SetVisualizationModeToGlyphs() {
    this->SetVisualizationMode(this->visModeGlyph);
  };
  void SetVisualizationModeToBoth() {
    this->SetVisualizationMode(this->visModeBoth);
  };

  /// Set Glyph color node ID as reference to the scene
  void SetSceneReferences() override;

  ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  void UpdateReferences() override;

  ///
  /// Finds the storage node and read the data
  void UpdateScene(vtkDMMLScene *scene) override;

  ///
  /// String ID of the color DMML node
  void SetAndObserveGlyphColorNodeID(const char *GlyphColorNodeID);
  void SetAndObserveGlyphColorNodeID(std::string GlyphColorNodeID);
  vtkGetStringMacro(GlyphColorNodeID);

  ///
  /// Get associated color DMML node
  vtkDMMLColorNode* GetGlyphColorNode();

  ///
  /// alternative method to propagate events generated in Display nodes
  void ProcessDMMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;
  ///
  /// set gray colormap
  void SetDefaultColorMap(/*int isLabelMap*/) override;

  ///
  /// get associated slice glyph display node or nullptr if not set
  virtual std::vector< vtkDMMLGlyphableVolumeSliceDisplayNode*>
    GetSliceGlyphDisplayNodes( vtkDMMLVolumeNode* vtkNotUsed(node) )
    {
    vtkErrorMacro("Shouldn't be calling this");
    return std::vector< vtkDMMLGlyphableVolumeSliceDisplayNode*>();
    }


  ///
  /// add slice glyph display nodes if not already present and return it
  virtual void AddSliceGlyphDisplayNodes( vtkDMMLVolumeNode* vtkNotUsed(node) )
    {
    vtkErrorMacro("Shouldn't be calling this");
    }

  ///
  /// Defines the expected range of the output data for given imageData after
  /// having been mapped through the current display options
  void GetDisplayScalarRange(double range[2]) override
    {
    this->Superclass::GetDisplayScalarRange(range);
    }

protected:
  vtkDMMLGlyphableVolumeDisplayNode();
  ~vtkDMMLGlyphableVolumeDisplayNode() override;
  vtkDMMLGlyphableVolumeDisplayNode(const vtkDMMLGlyphableVolumeDisplayNode&);
  void operator=(const vtkDMMLGlyphableVolumeDisplayNode&);

  char *GlyphColorNodeID;

  void SetGlyphColorNodeID(const char* id);

  vtkDMMLColorNode *GlyphColorNode;

  int VisualizationMode;

};

#endif

