#ifndef __vtkDMMLROIListNode_h
#define __vtkDMMLROIListNode_h

#include "vtkDMMLNode.h"
class vtkDMMLROINode;

class vtkCollection;

/// \brief DMML list node to represent a list of ROINODE.
class VTK_DMML_EXPORT vtkDMMLROIListNode : public vtkDMMLNode
{
  public:
    static vtkDMMLROIListNode *New();
    vtkTypeMacro(vtkDMMLROIListNode,vtkDMMLNode);
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
    const char* GetNodeTagName() override {return "ROIList";}

    ///
    ///
    void UpdateScene(vtkDMMLScene *scene) override;

    ///
    /// update display node ids
    void UpdateReferences() override;

    ///
    /// Get the number of ROIs in the list
    int GetNumberOfROIs();

    ///
    /// Restrict access to the ROI, pass in a value via the list
    /// so that the appropriate events can be invoked. Returns 0 on success

    ///
    /// Get/Set for Nth ROI node Position in RAS coordinates
    /// Note: The ROI Position is the center of the ROI
    int SetNthROIXYZ(int n, double x, double y, double z);
    double *GetNthROIXYZ(int n);
    ///
    /// Get/Set for Nth radius of the ROI in RAS coordinates
    int SetNthROIRadiusXYZ(int n, double Radiusx, double Radiusy, double Radiusz);
    double *GetNthROIRadiusXYZ(int n);
    ///
    /// Get/Set for Nth ROI node Position in IJK coordinates
    /// Note: The ROI Position is the center of the ROI
    int SetNthROIIJK(int n, double i, double j, double k);
    double *GetNthROIIJK(int n);
    ///
    /// Get/Set for Nth radius of the ROI in IJK coordinates
    int SetNthROIRadiusIJK(int n, double Radiusi, double Radiusj, double Radiusk);
    double *GetNthROIRadiusIJK(int n);

    ///
    /// Get/Set for label text of the Nth ROI
    int SetNthROILabelText(int n, const char *text);
    const char *GetNthROILabelText(int n);
    ///
    /// Get/Set for selected flag of the Nth ROI
    int SetNthROISelected(int n, int flag);
    int GetNthROISelected(int n);
    ///
    /// Get/Set for ID of the Nth ROI
    int SetNthROIID(int n, const char *id);
    const char *GetNthROIID(int n);

    ///
    /// Add an ROI to the list with default values
    int AddROI( );
    ///
    /// Remove an ROI from the list
    void RemoveROI(int i);
    ///
    /// Remove all ROIs from the list
    void RemoveAllROIs();

    void ProcessDMMLEvents ( vtkObject *caller, unsigned long event, void *callData ) override;

    /// DisplayModifiedEvent is generated when display node parameters is changed
    /// PolyDataModifiedEvent is generated when something else is changed
    enum
      {
      DisplayModifiedEvent = 21000,
      PolyDataModifiedEvent = 21001,
      ROIModifiedEvent = 21002
      };

    ///
    /// Get/Set for list visibility
    void SetVisibility(int visible);
    vtkGetMacro(Visibility,int);

    ///
    /// Get/Set for Text scale
    void SetTextScale(double scale);
    vtkGetMacro(TextScale,double);

    ///
    /// Get/Set for ROI and Text color
    void SetColor(double r, double g, double b);
    void SetColor(double c[3]);
    vtkGetVectorMacro(Color,double,3);

    ///
    /// Get/Set for color for when an ROI is selected
    void SetSelectedColor(double r, double g, double b);
    void SetSelectedColor(double c[3]);
    vtkGetVectorMacro(SelectedColor,double,3);

    ///
    /// Opacity of the ROI expressed as a number from 0 to 1
    void SetOpacity(double opacity);
    vtkGetMacro(Opacity, double);

    ///
    /// Ambient of the ROI expressed as a number from 0 to 100
    vtkSetMacro(Ambient, double);
    vtkGetMacro(Ambient, double);

    ///
    /// Diffuse of the ROI expressed as a number from 0 to 100
    vtkSetMacro(Diffuse, double);
    vtkGetMacro(Diffuse, double);

    ///
    /// Specular of the ROI expressed as a number from 0 to 100
    vtkSetMacro(Specular, double);
    vtkGetMacro(Specular, double);

    ///
    /// Power of the ROI expressed as a number from 0 to 100
    vtkSetMacro(Power, double);
    vtkGetMacro(Power, double);

    ///
    /// Get/Set for the volume ID associated with the ROI
    vtkGetStringMacro(VolumeNodeID);
    vtkSetStringMacro(VolumeNodeID);

    ///
    /// Set the Volume node ID for each ROI node in the list
    void SetAllVolumeNodeID();

    ///
    /// Synchronize the ROI position and radius in IJK
    /// coordinates according RAS coordinates
    void UpdateIJK();

    ///
    /// disallow access to the ROI box by outside classes, have them use
    /// SetNthROI
    vtkDMMLROINode* GetNthROINode(int n);

  protected:
    vtkDMMLROIListNode();
    ~vtkDMMLROIListNode() override;

    vtkDMMLROIListNode(const vtkDMMLROIListNode&);
    void operator=(const vtkDMMLROIListNode&);

    ///
    /// The collection of ROI that make up this list
    vtkCollection *ROIList;

    double TextScale;
    int Visibility;
    double Color[3];
    double SelectedColor[3];


    ///
    /// Numbers relating to the 3D render of the ROI
    double Opacity;
    double Ambient;
    double Diffuse;
    double Specular;
    double Power;

    ///
    /// The ID of the volume node that is associated with the ROI
    char *VolumeNodeID;
};

#endif
