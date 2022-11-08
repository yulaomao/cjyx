#include "qDMMLSliceInformationWidgetPlugin.h"
#include "qDMMLSliceInformationWidget.h"

// --------------------------------------------------------------------------
qDMMLSliceInformationWidgetPlugin::qDMMLSliceInformationWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qDMMLSliceInformationWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLSliceInformationWidget* _widget = new qDMMLSliceInformationWidget(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qDMMLSliceInformationWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLSliceInformationWidget\" \
          name=\"DMMLSliceInformationWidget\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qDMMLSliceInformationWidgetPlugin::icon() const
{
  return QIcon();
}

// --------------------------------------------------------------------------
QString qDMMLSliceInformationWidgetPlugin::includeFile() const
{
  return "qDMMLSliceInformationWidget.h";
}

// --------------------------------------------------------------------------
bool qDMMLSliceInformationWidgetPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qDMMLSliceInformationWidgetPlugin::name() const
{
  return "qDMMLSliceInformationWidget";
}

