#ifndef __qDMMLEventLogger_p_h
#define __qDMMLEventLogger_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Cjyx API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

/// Qt includes
#include <QHash>
#include <QObject>
#include <QString>

/// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qDMML includes
#include "qDMMLEventLogger.h"

/// VTK includes
#include <vtkWeakPointer.h>

class vtkDMMLScene;

//------------------------------------------------------------------------------
class qDMMLEventLoggerPrivate: public QObject
{
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qDMMLEventLogger);
protected:
  qDMMLEventLogger* const q_ptr;
public:
  qDMMLEventLoggerPrivate(qDMMLEventLogger& object);
  typedef QObject Superclass;

  void init();

  void setDMMLScene(vtkDMMLScene* scene);

private:
  vtkWeakPointer<vtkDMMLScene> DMMLScene;

  QList<QString>          EventToListen;
  QHash<QString, QString> EventNameToConnectionIdMap;

  bool ConsoleOutputEnabled;
};

#endif
