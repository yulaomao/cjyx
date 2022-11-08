#include "qDMMLLayoutWidgetPlugin.h"
#include "qDMMLLayoutWidget.h"

qDMMLLayoutWidgetPlugin::qDMMLLayoutWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{
}

QWidget *qDMMLLayoutWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLLayoutWidget* _widget = new qDMMLLayoutWidget(_parent);
  return _widget;
}

QString qDMMLLayoutWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLLayoutWidget\" \
          name=\"DMMLLayoutWidget\">\n"
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

QIcon qDMMLLayoutWidgetPlugin::icon() const
{
  return QIcon(":/Icons/table.png");
}

QString qDMMLLayoutWidgetPlugin::includeFile() const
{
  return "qDMMLLayoutWidget.h";
}

bool qDMMLLayoutWidgetPlugin::isContainer() const
{
  return false;
}

QString qDMMLLayoutWidgetPlugin::name() const
{
  return "qDMMLLayoutWidget";
}
