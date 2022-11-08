#ifndef __vtkDMMLLayoutNode_h
#define __vtkDMMLLayoutNode_h

// DMML includes
#include "vtkDMMLAbstractLayoutNode.h"

class vtkXMLDataElement;
class vtkDMMLAbstractViewNode;

/// \brief Node that describes the view layout of the application.
///
/// When the scene is closing (vtkDMMLScene::Clear), the view arrangement is
/// set to none due to the Copy() call on an empty node.
class VTK_DMML_EXPORT vtkDMMLLayoutNode : public vtkDMMLAbstractLayoutNode
{
public:
  static vtkDMMLLayoutNode *New();
  vtkTypeMacro(vtkDMMLLayoutNode,vtkDMMLAbstractLayoutNode);
  vtkDMMLNode* CreateNodeInstance() override;
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //--------------------------------------------------------------------------
  /// DMML methods
  //--------------------------------------------------------------------------

  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object
  void Copy(vtkDMMLNode *node) override;

  /// \brief Reimplemented to reset maximized view node.
  void Reset(vtkDMMLNode* defaultNode) override;

  /// Get/Set for Current layout
  vtkGetMacro(ViewArrangement, int);
  virtual void SetViewArrangement(int);

  vtkGetMacro(GUIPanelVisibility, int);
  vtkSetMacro(GUIPanelVisibility, int);

  vtkGetMacro(BottomPanelVisibility, int);
  vtkSetMacro(BottomPanelVisibility, int);

  /// 0 is left side, 1 is right side
  vtkGetMacro(GUIPanelLR, int);
  vtkSetMacro(GUIPanelLR, int);

  /// Control the collapse state of the SliceControllers
  vtkGetMacro(CollapseSliceControllers, int);
  vtkSetMacro(CollapseSliceControllers, int);

  /// CompareView configuration Get/Set methods
  vtkGetMacro(NumberOfCompareViewRows, int);
  vtkSetClampMacro(NumberOfCompareViewRows, int, 1, 50);
  vtkGetMacro(NumberOfCompareViewColumns, int);
  vtkSetClampMacro(NumberOfCompareViewColumns, int, 1, 50);

  /// CompareView lightbox configuration Get/Set methods
  vtkGetMacro(NumberOfCompareViewLightboxRows, int);
  vtkSetClampMacro(NumberOfCompareViewLightboxRows, int, 1, 50);
  vtkGetMacro(NumberOfCompareViewLightboxColumns, int);
  vtkSetClampMacro(NumberOfCompareViewLightboxColumns, int, 1, 50);

  /// Set/Get the size of the main and secondary panels (size of Frame1
  /// in each panel)
  vtkGetMacro(MainPanelSize, int);
  vtkSetMacro(MainPanelSize, int);
  vtkGetMacro(SecondaryPanelSize, int);
  vtkSetMacro(SecondaryPanelSize, int);

  /// Set/Get the size of the last selected module
  vtkGetStringMacro(SelectedModule);
  vtkSetStringMacro(SelectedModule);

  /// Set/Get the layout name of the view that is temporarily shown maximized.
  vtkDMMLAbstractViewNode* GetMaximizedViewNode();
  void SetMaximizedViewNode(vtkDMMLAbstractViewNode* maximizedViewNode);

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "Layout";}

  enum CjyxLayout
    {
    CjyxLayoutInitialView = 0,
    CjyxLayoutDefaultView = 1,
    CjyxLayoutConventionalView = 2,
    CjyxLayoutFourUpView = 3,
    CjyxLayoutOneUp3DView = 4,
    CjyxLayoutOneUpRedSliceView = 6,
    CjyxLayoutOneUpYellowSliceView = 7,
    CjyxLayoutOneUpGreenSliceView = 8,
    CjyxLayoutTabbed3DView = 9,
    CjyxLayoutTabbedSliceView = 10,
    CjyxLayoutCompareView = 12,
    CjyxLayoutNone = 14,
    CjyxLayoutDual3DView = 15,
    CjyxLayoutConventionalWidescreenView = 16,
    CjyxLayoutCompareWidescreenView = 17,
    CjyxLayoutTriple3DEndoscopyView = 19, // Up to here, all layouts are Cjyx 3 compatible
    CjyxLayoutThreeOverThreeView = 21,
    CjyxLayoutFourOverFourView = 22,
    CjyxLayoutCompareGridView = 23,
    CjyxLayoutTwoOverTwoView = 27,
    CjyxLayoutSideBySideView = 29,
    CjyxLayoutFourByThreeSliceView = 30,
    CjyxLayoutFourByTwoSliceView = 31,
    CjyxLayoutFiveByTwoSliceView = 32,
    CjyxLayoutThreeByThreeSliceView = 33,
    CjyxLayoutFourUpTableView = 34,
    CjyxLayout3DTableView = 35,
    CjyxLayoutConventionalPlotView = 36,
    CjyxLayoutFourUpPlotView = 37,
    CjyxLayoutFourUpPlotTableView = 38,
    CjyxLayoutOneUpPlotView = 39,
    CjyxLayoutThreeOverThreePlotView = 40,
    CjyxLayoutDicomBrowserView = 41,
    CjyxLayoutFinalView, // special value, must be placed after the last standard view (used for iterating through all the views)

    CjyxLayoutMaximizedView = 98,
    CjyxLayoutCustomView = 99,
    CjyxLayoutUserView = 100
    };

  /// Adds a layout description with integer identifier
  /// "layout". Returns false without making any modifications if the
  /// integer identifier "layout" has already been added.
  bool AddLayoutDescription(int layout, const char* layoutDescription);

  /// Modifies a layout description for integer identifier
  /// "layout". Returns false without making any modifications if the
  /// integer identifier "layout" has NOT already been added.
  bool SetLayoutDescription(int layout, const char* layoutDescription);

  /// Query whether a layout exists with a specified integer identifier
  bool IsLayoutDescription(int layout);

  /// Get the layout description associated with a specified integer
  /// identifier. The empty string is returned if the layout does not exist.
  std::string GetLayoutDescription(int layout);

  // Get the layout description currently displayed. Used
  // internally. This is XML description corresponding to the ivar
  // ViewArrangement which is the integer identifier for the
  // layout. ViewArrangement and CurrentViewDescription may not
  // correspond while a view is being switched.
  vtkGetStringMacro(CurrentLayoutDescription);

  // Get the XML data model of the CurrentViewDescription
  vtkGetObjectMacro(LayoutRootElement, vtkXMLDataElement);

  // You are responsible to delete the returned dataElement.
  static vtkXMLDataElement* ParseLayout(const char* description);

protected:
  void UpdateCurrentLayoutDescription();
  void SetAndParseCurrentLayoutDescription(const char* description);
  vtkSetStringMacro(CurrentLayoutDescription);

protected:
  vtkDMMLLayoutNode();
  ~vtkDMMLLayoutNode() override;
  vtkDMMLLayoutNode(const vtkDMMLLayoutNode&);
  void operator=(const vtkDMMLLayoutNode&);

  int GUIPanelVisibility;
  int BottomPanelVisibility;
  int GUIPanelLR;
  int CollapseSliceControllers;
  int ViewArrangement;
  int NumberOfCompareViewRows;
  int NumberOfCompareViewColumns;
  int NumberOfCompareViewLightboxRows;
  int NumberOfCompareViewLightboxColumns;

  char *SelectedModule;

  int MainPanelSize;
  int SecondaryPanelSize;

  std::map<int, std::string> Layouts;
  char*                      CurrentLayoutDescription;
  vtkXMLDataElement*         LayoutRootElement;
};

#endif
