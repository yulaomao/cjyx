#include "qDMMLVolumeThresholdWidgetPlugin.h"
#include "qDMMLVolumeThresholdWidget.h"

qDMMLVolumeThresholdWidgetPlugin::qDMMLVolumeThresholdWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qDMMLVolumeThresholdWidgetPlugin::createWidget(QWidget *_parent)
{
  qDMMLVolumeThresholdWidget* _widget = new qDMMLVolumeThresholdWidget(_parent);
  return _widget;
}

QString qDMMLVolumeThresholdWidgetPlugin::domXml() const
{
  return "<widget class=\"qDMMLVolumeThresholdWidget\" \
          name=\"DMMLVolumeThresholdWidget\">\n"
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

QIcon qDMMLVolumeThresholdWidgetPlugin::icon() const
{
  return QIcon(":/Icons/combobox.png");
}

QString qDMMLVolumeThresholdWidgetPlugin::includeFile() const
{
  return "qDMMLVolumeThresholdWidget.h";
}

bool qDMMLVolumeThresholdWidgetPlugin::isContainer() const
{
  return false;
}

QString qDMMLVolumeThresholdWidgetPlugin::name() const
{
  return "qDMMLVolumeThresholdWidget";
}
