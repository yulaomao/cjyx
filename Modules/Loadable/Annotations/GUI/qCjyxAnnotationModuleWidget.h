#ifndef __qCjyxAnnotationModuleWidget_h
#define __qCjyxAnnotationModuleWidget_h

// CTK includes
#include <ctkVTKObject.h>

// Cjyx includes
#include "qCjyxAbstractModuleWidget.h"
#include "../qCjyxAnnotationsModuleExport.h"
#include "vtkDMMLInteractionNode.h"
#include "vtkDMMLLayoutLogic.h"
#include "vtkDMMLSelectionNode.h"

class qCjyxAnnotationModulePropertyDialog;
class qCjyxAnnotationModuleReportDialog;
class qCjyxAnnotationModuleWidgetPrivate;
class vtkDMMLInteractionNode;
class vtkDMMLNode;

/// \ingroup Cjyx_QtModules_Annotation
class Q_CJYX_QTMODULES_ANNOTATIONS_EXPORT qCjyxAnnotationModuleWidget :
  public qCjyxAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
    typedef qCjyxAbstractModuleWidget Superclass;
    qCjyxAnnotationModuleWidget(QWidget *parent=nullptr);
    ~qCjyxAnnotationModuleWidget() override;
    vtkSmartPointer<vtkDMMLApplicationLogic> DMMLAppLogic;

    /// Different Annotation Types
    enum
      {
        TextNode = 1000,
        FiducialNode,
        AngleNode,
        StickyNode,
        SplineNode,
        RulerNode,
        ROINode,
        Screenshot,
      };

    /// the logic observes the interaction node, update the gui to keep in
    /// step with the mouse modes tool bar. If interactionNode is null, try to
    /// get it from the scene.
    void updateWidgetFromInteractionMode(vtkDMMLInteractionNode *interactionNode);

    bool setEditedNode(vtkDMMLNode* node, QString role = QString(), QString context = QString()) override;

protected:

public slots:
    /// a public slot that will refresh the tree view
    void refreshTree();
    /// a public slot that will expand a newly added hierarchy node item
    void onHierarchyNodeAddedEvent(vtkObject *caller, vtkObject *obj);

    /// a public slot allowing other modules to open up the screen capture
    /// dialog
    void grabSnapShot();

    /// Update the label showing the active annotation hierarchy, triggered from
    /// the logic modifying the active hierarchy node
    void updateActiveHierarchyLabel();

protected slots:

    /// User clicked on property edit button
    void propertyEditButtonClicked(QString dmmlId);

    // Table and Property Modify
    void moveDownSelected();
    void moveUpSelected();


  //------------------------------------------------------------------
  // Daniel's approved code starting here

  /// Create new annotation nodes
  void onCreateLineButtonClicked();
  void onCreateROIButtonClicked();

  /// Add a new hierarchy.
  void onAddHierarchyButtonClicked();

  /// Jump the 2d Slices to the first control point of the selected annotation
  void onJumpSlicesButtonClicked();

  /// Select all annotations
  void selectAllButtonClicked();

  /// Unselect all annotations
  void unselectAllButtonClicked();

  /// Toggle the visibility of selected Annotations
  void visibleSelectedButtonClicked();

  /// Un-/Lock selected Annotations
  void lockSelectedButtonClicked();

  /// Delete selected Annotations
  void deleteSelectedButtonClicked();

  /// Make active hierarchy annotations visible/invisible
  void invisibleHierarchyButtonClicked();
  void visibleHierarchyButtonClicked();
  /// Un/Lock annotations in active hierarchy
  void lockHierarchyButtonClicked();
  void unlockHierarchyButtonClicked();

  // Property dialog
  void propertyRestored();
  void propertyAccepted();

  // Report dialog
  void reportDialogRejected();
  void reportDialogAccepted();
  void onReportButtonClicked();

protected:
  QScopedPointer<qCjyxAnnotationModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qCjyxAnnotationModuleWidget);
  Q_DISABLE_COPY(qCjyxAnnotationModuleWidget);

  void setup() override;

  qCjyxAnnotationModulePropertyDialog* m_PropertyDialog;
  qCjyxAnnotationModuleReportDialog* m_ReportDialog;

  /// Type of current Annotations - described by enum
  int m_CurrentAnnotationType;

};

#endif
