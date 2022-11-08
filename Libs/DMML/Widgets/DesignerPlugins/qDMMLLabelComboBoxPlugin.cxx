
// qDMML includes
#include "qDMMLLabelComboBoxPlugin.h"
#include "qDMMLLabelComboBox.h"

//-----------------------------------------------------------------------------
qDMMLLabelComboBoxPlugin::qDMMLLabelComboBoxPlugin(QObject *_parent):QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qDMMLLabelComboBoxPlugin::createWidget(QWidget *_parent)
{
  qDMMLLabelComboBox* _widget = new qDMMLLabelComboBox(_parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qDMMLLabelComboBoxPlugin::domXml() const
{
  return "<widget class=\"qDMMLLabelComboBox\" \
          name=\"DMMLLabelComboBox\">\n"
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

//-----------------------------------------------------------------------------
QIcon qDMMLLabelComboBoxPlugin::icon() const
{
  return QIcon(":/Icons/combox.png");
}

//-----------------------------------------------------------------------------
QString qDMMLLabelComboBoxPlugin::includeFile() const
{
  return "qDMMLLabelComboBox.h";
}

//-----------------------------------------------------------------------------
bool qDMMLLabelComboBoxPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qDMMLLabelComboBoxPlugin::name() const
{
  return "qDMMLLabelComboBox";
}
