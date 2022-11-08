#ifndef __qCjyxScalarVolumeDisplayWidget_h
#define __qCjyxScalarVolumeDisplayWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// Cjyx includes
#include <qCjyxWidget.h>

#include "qCjyxVolumesModuleWidgetsExport.h"

class vtkDMMLNode;
class vtkDMMLScalarVolumeDisplayNode;
class vtkDMMLScalarVolumeNode;
class qCjyxScalarVolumeDisplayWidgetPrivate;

/// \ingroup Cjyx_QtModules_Volumes
class Q_CJYX_QTMODULES_VOLUMES_WIDGETS_EXPORT qCjyxScalarVolumeDisplayWidget
  : public qCjyxWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(bool enableColorTableComboBox READ isColorTableComboBoxEnabled WRITE setColorTableComboBoxEnabled )
  Q_PROPERTY(bool enableDMMLWindowLevelWidget READ isDMMLWindowLevelWidgetEnabled WRITE setDMMLWindowLevelWidgetEnabled )
public:
  /// Constructors
  typedef qCjyxWidget Superclass;
  explicit qCjyxScalarVolumeDisplayWidget(QWidget* parent);
  ~qCjyxScalarVolumeDisplayWidget() override;

  Q_INVOKABLE vtkDMMLScalarVolumeNode* volumeNode()const;
  Q_INVOKABLE vtkDMMLScalarVolumeDisplayNode* volumeDisplayNode()const;

  bool isColorTableComboBoxEnabled()const;
  void setColorTableComboBoxEnabled(bool);

  bool isDMMLWindowLevelWidgetEnabled()const;
  void setDMMLWindowLevelWidgetEnabled(bool);

public slots:

  ///
  /// Set the DMML node of interest
  void setDMMLVolumeNode(vtkDMMLScalarVolumeNode* volumeNode);
  void setDMMLVolumeNode(vtkDMMLNode* node);

  void setInterpolate(bool interpolate);
  void setColorNode(vtkDMMLNode* colorNode);
  void setPreset(const QString& presetName);

protected slots:
  void updateWidgetFromDMML();
  void updateHistogram();
  void onPresetButtonClicked();
  void onLockWindowLevelButtonClicked();
  void onHistogramSectionExpanded(bool);

protected:
  void showEvent(QShowEvent * event) override;
protected:
  QScopedPointer<qCjyxScalarVolumeDisplayWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qCjyxScalarVolumeDisplayWidget);
  Q_DISABLE_COPY(qCjyxScalarVolumeDisplayWidget);
};

#endif
