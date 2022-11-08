#ifndef __qDMMLSliceWidgetPlugin_h
#define __qDMMLSliceWidgetPlugin_h

#include "qDMMLWidgetsAbstractPlugin.h"

class QDMML_WIDGETS_PLUGINS_EXPORT qDMMLSliceWidgetPlugin :
  public QObject,
  public qDMMLWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qDMMLSliceWidgetPlugin(QObject *_parent = nullptr);

  QWidget *createWidget(QWidget *_parent) override;
  QString  domXml() const override;
//   QIcon    icon() const;
  QString  includeFile() const override;
  bool     isContainer() const override;
  QString  name() const override;

};

#endif
