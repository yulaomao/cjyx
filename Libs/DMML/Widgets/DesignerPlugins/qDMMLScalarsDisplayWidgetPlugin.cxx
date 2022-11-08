#include "qDMMLScalarsDisplayWidgetPlugin.h"
#include "qDMMLScalarsDisplayWidget.h"

qDMMLScalarsDisplayWidgetPlugin::qDMMLScalarsDisplayWidgetPlugin(QObject *_parent)
 : QObject(_parent)
{
}

QWidget *qDMMLScalarsDisplayWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLScalarsDisplayWidget* _widget = new qDMMLScalarsDisplayWidget(_parent);
  return _widget;
}

QString qDMMLScalarsDisplayWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLScalarsDisplayWidget\" \
          name=\"DMMLScalarsDisplayWidget\">\n"
          " <property name=\"geometry\">\n"
          "  <rect>\n"
          "   <x>0</x>\n"
          "   <y>0</y>\n"
          "   <width>200</width>\n"
          "   <height>20</height>\n"
          "  </rect>\n"
          " </property>\n"
          "</widget>\n";
}

QIcon qDMMLScalarsDisplayWidgetPlugin::icon() const
{
  return QIcon(":/Icons/table.png");
}

QString qDMMLScalarsDisplayWidgetPlugin::includeFile() const
{
  return "qDMMLScalarsDisplayWidget.h";
}

bool qDMMLScalarsDisplayWidgetPlugin::isContainer() const
{
  return false;
}

QString qDMMLScalarsDisplayWidgetPlugin::name() const
{
  return "qDMMLScalarsDisplayWidget";
}
