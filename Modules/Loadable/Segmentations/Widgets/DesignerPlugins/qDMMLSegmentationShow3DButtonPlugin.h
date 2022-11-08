/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Cjyx

=========================================================================auto=*/

#ifndef __qDMMLSegmentationShow3DButtonPlugin_h
#define __qDMMLSegmentationShow3DButtonPlugin_h

#include "qCjyxSegmentationsModuleWidgetsAbstractPlugin.h"

class Q_CJYX_MODULE_SEGMENTATIONS_WIDGETS_PLUGINS_EXPORT qDMMLSegmentationShow3DButtonPlugin
  : public QObject
  , public qCjyxSegmentationsModuleWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qDMMLSegmentationShow3DButtonPlugin(QObject* parent = nullptr);

  QWidget *createWidget(QWidget* parent) override;
  QString  domXml() const override;
  QString  includeFile() const override;
  bool     isContainer() const override;
  QString  name() const override;

};

#endif
