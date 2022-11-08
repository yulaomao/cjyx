#include "qDMMLThreeDViewInformationWidgetPlugin.h"
#include "qDMMLThreeDViewInformationWidget.h"

// --------------------------------------------------------------------------
qDMMLThreeDViewInformationWidgetPlugin::qDMMLThreeDViewInformationWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qDMMLThreeDViewInformationWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLThreeDViewInformationWidget* _widget = new qDMMLThreeDViewInformationWidget(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qDMMLThreeDViewInformationWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLThreeDViewInformationWidget\" \
          name=\"DMMLThreeDViewInformationWidget\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qDMMLThreeDViewInformationWidgetPlugin::icon() const
{
  return QIcon();
}

// --------------------------------------------------------------------------
QString qDMMLThreeDViewInformationWidgetPlugin::includeFile() const
{
  return "qDMMLThreeDViewInformationWidget.h";
}

// --------------------------------------------------------------------------
bool qDMMLThreeDViewInformationWidgetPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qDMMLThreeDViewInformationWidgetPlugin::name() const
{
  return "qDMMLThreeDViewInformationWidget";
}
