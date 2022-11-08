#ifndef __qDMMLThreeDViewPlugin_h
#define __qDMMLThreeDViewPlugin_h

#include "qDMMLWidgetsAbstractPlugin.h"

class QDMML_WIDGETS_PLUGINS_EXPORT qDMMLThreeDViewPlugin :
  public QObject,
  public qDMMLWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qDMMLThreeDViewPlugin(QObject *_parent = nullptr);

  QWidget *createWidget(QWidget *_parent) override;
  QString  domXml() const override;
//   QIcon    icon() const;
  QString  includeFile() const override;
  bool     isContainer() const override;
  QString  name() const override;

};

#endif
