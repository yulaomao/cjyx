/*=auto=========================================================================

  Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLProceduralColorNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.0 $

=========================================================================auto=*/

#ifndef __vtkDMMLProceduralColorNode_h
#define __vtkDMMLProceduralColorNode_h

#include "vtkDMMLColorNode.h"

class vtkColorTransferFunction;

/// \brief DMML node to represent procedurally defined color information.
///
/// Procedural nodes define methods that are used to map colors to scalar
/// values. Usually they will incorporate a custom subclass of a
/// vtkLookupTable, or a vtkColorTransferFunction.
class VTK_DMML_EXPORT vtkDMMLProceduralColorNode : public vtkDMMLColorNode
{
public:
  static vtkDMMLProceduralColorNode *New();
  vtkTypeMacro(vtkDMMLProceduralColorNode,vtkDMMLColorNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //--------------------------------------------------------------------------
  /// DMMLNode methods
  //--------------------------------------------------------------------------

  vtkDMMLNode* CreateNodeInstance() override;

  ///
  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkDMMLNode *node) override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "ProceduralColor";};

  ///
  ///
  void UpdateScene(vtkDMMLScene *scene) override;

  ///
  /// return a text string describing the color look up table type
  const char * GetTypeAsString() override;

  void ProcessDMMLEvents ( vtkObject *caller, unsigned long event, void *callData ) override;

  /// The list of valid procedural types
  //enum
  //{
  ///
  //};

  /// DisplayModifiedEvent is generated when display node parameters is changed
  enum
    {
      DisplayModifiedEvent = 20000
    };

  /// Get the color transfer function for this node
  /// \sa ColorTransferFunction, GetScalarsToColors(),
  /// SetAndObserveColorTransferFunction()
  vtkGetObjectMacro(ColorTransferFunction, vtkColorTransferFunction);

  /// Set the color transfer function
  /// \sa ColorTransferFunction, GetColorTransferFunction()
  virtual void SetAndObserveColorTransferFunction(vtkColorTransferFunction *ctf);

  /// Compare two color transfer functions
  /// Only compares the color map (x->RGB mapping)
  static bool IsColorMapEqual(vtkColorTransferFunction* tf1, vtkColorTransferFunction* tf2);

  /// Reimplemented vtkDMMLColorNode::GetLookupTable() to convert
  /// the continuous color transfer function to a look up table
  /// with a number of entries defined by NumberOfTableValues
  /// \sa ConvertedCTFtoLUT, SetNumberOfTableValues()
  vtkLookupTable * GetLookupTable() override;

  /// Reimplemented vtkDMMLColorNode::GetScalarsToColors() to return the
  /// transfer function instead of the empty lookuptable
  /// \sa ColorTransferFunction, GetColorTransferFunction()
  vtkScalarsToColors* GetScalarsToColors() override;

  /// set up some names, going from the points defined in the transfer function
  /// \sa vtkDMMLColorNode::SetColorName()
  bool SetNameFromColor(int index) override;

  /// Returns how many nodes define the color
  /// transfer function
  int GetNumberOfColors() override;

  /// Retrieve color transfer function entry value
  bool GetColor(int entry, double color[4]) override;

  /// Create default storage node or nullptr if does not have one
  vtkDMMLStorageNode* CreateDefaultStorageNode() override;

  /// Get number of entries used when discretizing
  /// the color transfer function into a lookup table
  /// \sa SetNumberOfTableValues(), GetLookupTable()
  vtkGetMacro(NumberOfTableValues, unsigned int)

  /// Set number of entries used when discretizing
  /// the color transfer function into a lookup table
  /// \sa GetNumberOfTableValues(), GetLookupTable()
  vtkSetMacro(NumberOfTableValues, unsigned int)

protected:
  vtkDMMLProceduralColorNode();
  ~vtkDMMLProceduralColorNode() override;
  vtkDMMLProceduralColorNode(const vtkDMMLProceduralColorNode&);
  void operator=(const vtkDMMLProceduralColorNode&);

  /// a color transfer function built up by calls to AddRGBPoint and Build
  /// \sa SetAndObserveColorTransferFunction(), GetColorTransferFunction()
  vtkColorTransferFunction *ColorTransferFunction;

  /// A lookup table created by discretizing
  /// the continuous color transfer function
  /// \sa GetLookupTable(), NumberOfTableValues
  vtkLookupTable *ConvertedCTFtoLUT;

  /// Number of entries to use when discretizing
  /// the color transfer function into a lookup table
  /// \sa GetNumberOfTableValues(), SetNumberOfTableValues(),
  /// GetLookupTable()
  unsigned int NumberOfTableValues;
};

#endif
