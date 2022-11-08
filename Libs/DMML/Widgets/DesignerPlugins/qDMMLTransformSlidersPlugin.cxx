#include "qDMMLTransformSlidersPlugin.h"
#include "qDMMLTransformSliders.h"

qDMMLTransformSlidersPlugin::qDMMLTransformSlidersPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qDMMLTransformSlidersPlugin::createWidget(QWidget *_parent)
{
  qDMMLTransformSliders* _widget = new qDMMLTransformSliders(_parent);
  return _widget;
}

QString qDMMLTransformSlidersPlugin::domXml() const
{
  return "<widget class=\"qDMMLTransformSliders\" \
          name=\"DMMLTransformSliders\">\n"
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

QIcon qDMMLTransformSlidersPlugin::icon() const
{
  return QIcon(":/Icons/groupbox.png");
}

QString qDMMLTransformSlidersPlugin::includeFile() const
{
  return "qDMMLTransformSliders.h";
}

bool qDMMLTransformSlidersPlugin::isContainer() const
{
  return false;
}

QString qDMMLTransformSlidersPlugin::name() const
{
  return "qDMMLTransformSliders";
}
