#include "qDMMLLinearTransformSliderPlugin.h"
#include "qDMMLLinearTransformSlider.h"

qDMMLLinearTransformSliderPlugin::qDMMLLinearTransformSliderPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qDMMLLinearTransformSliderPlugin::createWidget(QWidget *_parent)
{
  qDMMLLinearTransformSlider* _widget = new qDMMLLinearTransformSlider(_parent);
  return _widget;
}

QString qDMMLLinearTransformSliderPlugin::domXml() const
{
  return "<widget class=\"qDMMLLinearTransformSlider\" \
          name=\"DMMLLinearTransformSlider\">\n"
          " <property name=\"geometry\">\n"
          "  <rect>\n"
          "   <x>0</x>\n"
          "   <y>0</y>\n"
          "   <width>200</width>\n"
          "   <height>32</height>\n"
          "  </rect>\n"
          " </property>\n"
          "</widget>\n";
}

QIcon qDMMLLinearTransformSliderPlugin::icon() const
{
  return QIcon(":/Icons/sliderspinbox.png");
}

QString qDMMLLinearTransformSliderPlugin::includeFile() const
{
  return "qDMMLLinearTransformSlider.h";
}

bool qDMMLLinearTransformSliderPlugin::isContainer() const
{
  return false;
}

QString qDMMLLinearTransformSliderPlugin::name() const
{
  return "qDMMLLinearTransformSlider";
}
