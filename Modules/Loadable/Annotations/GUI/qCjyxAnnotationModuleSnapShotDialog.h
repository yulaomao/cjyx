#ifndef __qCjyxAnnotationModuleSnapShotDialog_h
#define __qCjyxAnnotationModuleSnapShotDialog_h

class vtkCjyxAnnotationModuleLogic;

#include "qDMMLScreenShotDialog.h"

/// \ingroup Cjyx_QtModules_Annotation
class qCjyxAnnotationModuleSnapShotDialog : public qDMMLScreenShotDialog
{
  Q_OBJECT
public:
  typedef qDMMLScreenShotDialog Superclass;
  qCjyxAnnotationModuleSnapShotDialog(QWidget* parent = nullptr);
  ~qCjyxAnnotationModuleSnapShotDialog() override;

  /// Set the Annotation module logic.
  void setLogic(vtkCjyxAnnotationModuleLogic* logic);

  /// Initialize this dialog with values from an existing annotation Snapshot node.
  void loadNode(const char* nodeId);
  /// Reset the dialog and give it a unique name.
  void reset();

  void accept() override;

private:
    vtkCjyxAnnotationModuleLogic* m_Logic;
};

#endif
