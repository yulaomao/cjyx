#include "qDMMLWindowLevelWidgetPlugin.h"
#include "qDMMLWindowLevelWidget.h"

qDMMLWindowLevelWidgetPlugin::qDMMLWindowLevelWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qDMMLWindowLevelWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLWindowLevelWidget* _widget = new qDMMLWindowLevelWidget(_parent);
  return _widget;
}

QString qDMMLWindowLevelWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLWindowLevelWidget\" \
          name=\"DMMLWindowLevelWidget\">\n"
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

QIcon qDMMLWindowLevelWidgetPlugin::icon() const
{
  return QIcon(":/Icons/combobox.png");
}

QString qDMMLWindowLevelWidgetPlugin::includeFile() const
{
  return "qDMMLWindowLevelWidget.h";
}

bool qDMMLWindowLevelWidgetPlugin::isContainer() const
{
  return false;
}

QString qDMMLWindowLevelWidgetPlugin::name() const
{
  return "qDMMLWindowLevelWidget";
}
