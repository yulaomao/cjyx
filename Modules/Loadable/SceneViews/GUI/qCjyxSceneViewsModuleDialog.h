#ifndef __qCjyxSceneViewsModuleDialog_h
#define __qCjyxSceneViewsModuleDialog_h

#include <vtkCjyxSceneViewsModuleLogic.h>

#include "qDMMLScreenShotDialog.h"

/// \ingroup Cjyx_QtModules_SceneViews
class qCjyxSceneViewsModuleDialog : public qDMMLScreenShotDialog
{
  Q_OBJECT
public:
  typedef qDMMLScreenShotDialog Superclass;

  qCjyxSceneViewsModuleDialog(QWidget* parent=nullptr);
  ~qCjyxSceneViewsModuleDialog() override;

  /// Set the SceneViews module logic.
  void setLogic(vtkCjyxSceneViewsModuleLogic* logic);

  /// Initialize this dialog with values from an existing annotation Snapshot node.
  void loadNode(const QString& nodeId);
  /// Reset the dialog and give it a unique name.
  void reset();

  void accept() override;

private:
    vtkCjyxSceneViewsModuleLogic* m_Logic;
};

#endif
