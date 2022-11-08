/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Cjyx

=========================================================================auto=*/

#include "qDMMLSegmentationShow3DButtonPlugin.h"
#include "qDMMLSegmentationShow3DButton.h"

//-----------------------------------------------------------------------------
qDMMLSegmentationShow3DButtonPlugin::qDMMLSegmentationShow3DButtonPlugin(QObject* pluginParent)
  : QObject(pluginParent)
{
}

//-----------------------------------------------------------------------------
QWidget *qDMMLSegmentationShow3DButtonPlugin::createWidget(QWidget* parentWidget)
{
  qDMMLSegmentationShow3DButton* pluginWidget =
    new qDMMLSegmentationShow3DButton(parentWidget);
  return pluginWidget;
}

//-----------------------------------------------------------------------------
QString qDMMLSegmentationShow3DButtonPlugin::domXml() const
{
  return "<widget class=\"qDMMLSegmentationShow3DButton\" \
          name=\"SegmentationShow3DButton\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qDMMLSegmentationShow3DButtonPlugin::includeFile() const
{
  return "qDMMLSegmentationShow3DButton.h";
}

//-----------------------------------------------------------------------------
bool qDMMLSegmentationShow3DButtonPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qDMMLSegmentationShow3DButtonPlugin::name() const
{
  return "qDMMLSegmentationShow3DButton";
}
