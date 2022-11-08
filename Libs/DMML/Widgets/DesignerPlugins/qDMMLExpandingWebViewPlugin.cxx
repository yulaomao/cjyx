#include "qDMMLExpandingWebViewPlugin.h"
#include "qDMMLExpandingWebView.h"

//------------------------------------------------------------------------------
qDMMLExpandingWebViewPlugin::qDMMLExpandingWebViewPlugin(QObject *_parent)
        : QObject(_parent)
{

}

//------------------------------------------------------------------------------
QWidget *qDMMLExpandingWebViewPlugin::createWidget(QWidget *_parent)
{
  qDMMLExpandingWebView* _widget = new qDMMLExpandingWebView(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qDMMLExpandingWebViewPlugin::domXml() const
{
  return "<widget class=\"qDMMLExpandingWebView\" \
          name=\"DMMLExpandingWebView\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qDMMLExpandingWebViewPlugin::includeFile() const
{
  return "qDMMLExpandingWebView.h";
}

//------------------------------------------------------------------------------
bool qDMMLExpandingWebViewPlugin::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qDMMLExpandingWebViewPlugin::name() const
{
  return "qDMMLExpandingWebView";
}
