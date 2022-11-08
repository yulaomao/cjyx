// .NAME vtkDMMLAnnotationControlPointsNode - DMML node to represent a fiber bundle from tractography in DTI data.
// .SECTION Description
// Annotation nodes contains control points, internally represented as vtkPolyData.
// A Annotation node contains many control points  and forms the smallest logical unit of tractography
// that DMML will manage/read/write. Each control point has accompanying data.
// Visualization parameters for these nodes are controlled by the vtkDMMLAnnotationPointDisplayNode class.
//

#ifndef __vtkDMMLAnnotationControlPointsNode_h
#define __vtkDMMLAnnotationControlPointsNode_h

#include "vtkDMMLAnnotationNode.h"

class vtkDMMLAnnotationPointDisplayNode;

/// \ingroup Cjyx_QtModules_Annotation
class  VTK_CJYX_ANNOTATIONS_MODULE_DMML_EXPORT vtkDMMLAnnotationControlPointsNode
  : public vtkDMMLAnnotationNode
{
public:
  static vtkDMMLAnnotationControlPointsNode *New();
  vtkTypeMacro(vtkDMMLAnnotationControlPointsNode,vtkDMMLAnnotationNode);

  // void PrintSelf(ostream& os, vtkIndent indent) override;
  // Description:
  // Just prints short summary
  void PrintAnnotationInfo(ostream& os, vtkIndent indent, int titleFlag = 1) override;

  //--------------------------------------------------------------------------
  // DMMLNode methods
  //--------------------------------------------------------------------------

  vtkDMMLNode* CreateNodeInstance() override;
  // Description:
  // Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "AnnotationControlPoints";}

  // Description:
  // Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  // Description:
  // Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Write this node's information to a vector of strings for passing to a CLI,
  /// precede each datum with the prefix if not an empty string
  /// coordinateSystemFlag = 0 for RAS, 1 for LPS
  /// multipleFlag = 1 for the whole list, 1 for the first point
  void WriteCLI(std::vector<std::string>& commandLine,
                        std::string prefix, int coordinateSystem = 0,
                        int multipleFlag = 1) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkDMMLNode::CopyContent
  vtkDMMLCopyContentDefaultMacro(vtkDMMLAnnotationControlPointsNode);

  void UpdateScene(vtkDMMLScene *scene) override;

  // Description:
  // alternative method to propagate events generated in Display nodes
  void ProcessDMMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;


  enum
    {
      ControlPointModifiedEvent = 19010,
    };

  void Modified() override
    {
    Superclass::Modified();

    if (!this->GetDisableModifiedEvent())
      {
      this->InvokeEvent(vtkDMMLAnnotationControlPointsNode::ControlPointModifiedEvent);
      }
    }

  ///
  /// Invokes any modified events that are 'pending', meaning they were generated
  /// while the DisableModifiedEvent flag was nonzero.
  /// Returns the old flag state.
  int InvokePendingModifiedEvent () override
    {
    if ( this->GetModifiedEventPending() )
      {
      this->InvokeEvent(vtkDMMLAnnotationControlPointsNode::ControlPointModifiedEvent);
      }
    return Superclass::InvokePendingModifiedEvent();
    }

  // Description:
  // get associated display node or nullptr if not set
  vtkDMMLAnnotationPointDisplayNode* GetAnnotationPointDisplayNode();

  // Description:
  // Create default storage node or nullptr if does not have one
  vtkDMMLStorageNode* CreateDefaultStorageNode() override;

  int  AddControlPoint(double newControl[3],int selectedFlag, int visibleFlag);

  int  SetControlPoint(int id, double newControl[3],int selectedFlag, int visibleFlag);

  int  SetControlPointWorldCoordinates(int id, double newControl[3], int selectedFlag, int visibleFlag);

  int  SetControlPoint(int id, double newControl[3]);

  int  SetControlPointWorldCoordinates(int id, double newControl[3]);

  void DeleteControlPoint(int id);
  /// Return the RAS coordinates of point ID.
  /// Try to limit calling this function because it is performance critical.
  /// Also, when queried again it resets all former pointers. Copying is therefore necessary.
  double* GetControlPointCoordinates(vtkIdType id);

  void GetControlPointWorldCoordinates(vtkIdType id, double *point);

  int GetNumberOfControlPoints();

  enum
  {
    CP_SELECTED =  vtkDMMLAnnotationNode::NUM_TEXT_ATTRIBUTE_TYPES,
    CP_VISIBLE,
    NUM_CP_ATTRIBUTE_TYPES
  };

  const char *GetAttributeTypesEnumAsString(int val) override;


  // Description:
  // Initializes all variables associated with annotations
  void ResetAnnotations() override;

  // Description:
  // add display node if not already present
  void CreateAnnotationPointDisplayNode();

  /// flags to determine how the next fiducial added to the list is labelled
  enum NumberingSchemes
  {
      SchemeMin = 0,
      UseID = SchemeMin,
      UseIndex,
      UsePrevious,
      SchemeMax = UsePrevious,
  };


  /// Flag determining how to number the next added fiducial
  virtual void SetNumberingScheme(int numberingScheme);
  vtkGetMacro(NumberingScheme, int);

  /// Return a string representing the numbering scheme, set it from a string
  const char* GetNumberingSchemeAsString();
  const char* GetNumberingSchemeAsString(int g);
  void SetNumberingSchemeFromString(const char *schemeString);

  void Initialize(vtkDMMLScene* dmmlScene) override;

protected:
  vtkDMMLAnnotationControlPointsNode();
  ~vtkDMMLAnnotationControlPointsNode() override  = default;
  vtkDMMLAnnotationControlPointsNode(const vtkDMMLAnnotationControlPointsNode&);
  void operator=(const vtkDMMLAnnotationControlPointsNode&);

  // Description:
  // Create Poly data with substructures necessary for this class
  void CreatePolyData();

  // Description:
  // Initializes control points as well as attributes
  void ResetControlPoints();

  // Description:
  // Initializes all attributes
  void ResetControlPointsAttributesAll();

  /// How the next annotation will be numbered in it's Text field
  int NumberingScheme;
};

#endif
