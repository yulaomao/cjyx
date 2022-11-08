
// qDMML includes
#include "qDMMLSliceWidgetPlugin.h"
#include "qDMMLSliceWidget.h"

//-----------------------------------------------------------------------------
qDMMLSliceWidgetPlugin::qDMMLSliceWidgetPlugin(QObject *_parent):QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qDMMLSliceWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLSliceWidget* _widget = new qDMMLSliceWidget(_parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qDMMLSliceWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLSliceWidget\" \
          name=\"DMMLSliceViewWidget\">\n"
          " <property name=\"geometry\">\n"
          "  <rect>\n"
          "   <x>0</x>\n"
          "   <y>0</y>\n"
          "   <width>200</width>\n"
          "   <height>200</height>\n"
          "  </rect>\n"
          " </property>\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qDMMLSliceWidgetPlugin::includeFile() const
{
  return "qDMMLSliceWidget.h";
}

//-----------------------------------------------------------------------------
bool qDMMLSliceWidgetPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qDMMLSliceWidgetPlugin::name() const
{
  return "qDMMLSliceWidget";
}
