
// qDMML includes
#include "qDMMLThreeDViewPlugin.h"
#include "qDMMLThreeDView.h"

//-----------------------------------------------------------------------------
qDMMLThreeDViewPlugin::qDMMLThreeDViewPlugin(QObject *_parent):QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qDMMLThreeDViewPlugin::createWidget(QWidget *_parent)
{
  qDMMLThreeDView* _widget = new qDMMLThreeDView(_parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qDMMLThreeDViewPlugin::domXml() const
{
  return "<widget class=\"qDMMLThreeDView\" \
          name=\"DMMLThreeDView\">\n"
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
QString qDMMLThreeDViewPlugin::includeFile() const
{
  return "qDMMLThreeDView.h";
}

//-----------------------------------------------------------------------------
bool qDMMLThreeDViewPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qDMMLThreeDViewPlugin::name() const
{
  return "qDMMLThreeDView";
}
