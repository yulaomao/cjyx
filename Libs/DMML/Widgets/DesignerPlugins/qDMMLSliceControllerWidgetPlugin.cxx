#include "qDMMLSliceControllerWidgetPlugin.h"
#include "qDMMLSliceControllerWidget.h"

// --------------------------------------------------------------------------
qDMMLSliceControllerWidgetPlugin::qDMMLSliceControllerWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qDMMLSliceControllerWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLSliceControllerWidget* _widget = new qDMMLSliceControllerWidget(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qDMMLSliceControllerWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLSliceControllerWidget\" \
          name=\"DMMLSliceControllerWidget\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qDMMLSliceControllerWidgetPlugin::icon() const
{
  return QIcon();
}

// --------------------------------------------------------------------------
QString qDMMLSliceControllerWidgetPlugin::includeFile() const
{
  return "qDMMLSliceControllerWidget.h";
}

// --------------------------------------------------------------------------
bool qDMMLSliceControllerWidgetPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qDMMLSliceControllerWidgetPlugin::name() const
{
  return "qDMMLSliceControllerWidget";
}

