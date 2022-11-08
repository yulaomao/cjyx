#ifndef __qDMMLLinearTransformSliderPlugin_h
#define __qDMMLLinearTransformSliderPlugin_h

#include "qDMMLWidgetsAbstractPlugin.h"

class QDMML_WIDGETS_PLUGINS_EXPORT qDMMLLinearTransformSliderPlugin : public QObject,
                                         public qDMMLWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qDMMLLinearTransformSliderPlugin(QObject *_parent = nullptr);

  QWidget *createWidget(QWidget *_parent) override;
  QString domXml() const override;
  QIcon   icon() const override;
  QString includeFile() const override;
  bool isContainer() const override;
  QString name() const override;

};

#endif
