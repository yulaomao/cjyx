#ifndef __qCjyxCropVolumeModuleWidget_h
#define __qCjyxCropVolumeModuleWidget_h

// Cjyx includes
#include "qCjyxAbstractModuleWidget.h"

#include "qCjyxCropVolumeModuleExport.h"

class qCjyxCropVolumeModuleWidgetPrivate;
class vtkDMMLNode;
class vtkDMMLCropVolumeParametersNode;

/// \ingroup Cjyx_QtModules_CropVolume
class Q_CJYX_QTMODULES_CROPVOLUME_EXPORT qCjyxCropVolumeModuleWidget :
  public qCjyxAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qCjyxAbstractModuleWidget Superclass;
  qCjyxCropVolumeModuleWidget(QWidget *parent=nullptr);
  ~qCjyxCropVolumeModuleWidget() override;

  bool setEditedNode(vtkDMMLNode* node, QString role = QString(), QString context = QString()) override;

public slots:
  void setParametersNode(vtkDMMLNode* node);

protected:
  QScopedPointer<qCjyxCropVolumeModuleWidgetPrivate> d_ptr;

  void setup() override;
  void enter() override;
  void setDMMLScene(vtkDMMLScene*) override;

protected slots:
  void setInputVolume(vtkDMMLNode*);
  void setOutputVolume(vtkDMMLNode* node);
  void setInputROI(vtkDMMLNode*);
  void initializeInputROI(vtkDMMLNode*);
  /// when ROIs get added to the node selector, if the selector doesn't
  /// have a current node, select it
  void onInputROIAdded(vtkDMMLNode* node);

  void onROIVisibilityChanged(bool);
  void onROIFit();
  void onInterpolationModeChanged();
  void onApply();
  void onFixAlignment();
  void updateWidgetFromDMML();
  void onSpacingScalingValueChanged(double);
  void onIsotropicModeChanged(bool);
  void onDMMLSceneEndBatchProcessEvent();
  void onInterpolationEnabled(bool interpolationEnabled);
  void onVolumeInformationSectionClicked(bool isOpen);
  void onFillValueChanged(double);

  void updateVolumeInfo();

private:
  Q_DECLARE_PRIVATE(qCjyxCropVolumeModuleWidget);
  Q_DISABLE_COPY(qCjyxCropVolumeModuleWidget);
};

#endif
