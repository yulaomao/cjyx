#ifndef __qCjyxVolumeDisplayWidget_h
#define __qCjyxVolumeDisplayWidget_h

// Cjyx  includes
#include <qDMMLWidget.h>

// CTK includes
#include <ctkVTKObject.h>

// Volumes includes
#include "qCjyxVolumesModuleWidgetsExport.h"

class vtkDMMLNode;
class qCjyxVolumeDisplayWidgetPrivate;

/// \ingroup Cjyx_QtModules_Volumes
class Q_CJYX_QTMODULES_VOLUMES_WIDGETS_EXPORT qCjyxVolumeDisplayWidget : public qDMMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  /// Constructors
  typedef qDMMLWidget Superclass;
  explicit qCjyxVolumeDisplayWidget(QWidget* parent=nullptr);
  ~qCjyxVolumeDisplayWidget() override;

public slots:
  /// Set the DMML node of interest
  void setDMMLVolumeNode(vtkDMMLNode* node);

protected slots:
  /// Internally use in case the current display widget should change when the
  /// volume node changes
  void updateFromDMML(vtkObject* volume);
protected:
  QScopedPointer<qCjyxVolumeDisplayWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qCjyxVolumeDisplayWidget);
  Q_DISABLE_COPY(qCjyxVolumeDisplayWidget);
};

#endif
