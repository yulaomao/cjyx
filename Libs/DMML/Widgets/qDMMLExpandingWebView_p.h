#ifndef __qDMMLExpandingWebView_p_h
#define __qDMMLExpandingWebView_p_h

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

// VTK includes
#include <vtkWeakPointer.h>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qDMML includes
#include "qDMMLExpandingWebView.h"

class vtkObject;

//-----------------------------------------------------------------------------
class qDMMLExpandingWebViewPrivate: public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qDMMLExpandingWebView);
protected:
  qDMMLExpandingWebView* const q_ptr;
public:
  qDMMLExpandingWebViewPrivate(qDMMLExpandingWebView& object);
  ~qDMMLExpandingWebViewPrivate() override;

  virtual void init();

  void setDMMLScene(vtkDMMLScene* scene);
  vtkDMMLScene *dmmlScene();

public slots:
  /// Handle DMML scene events
  void startProcessing();
  void endProcessing();

protected:

  vtkDMMLScene*                      DMMLScene;
};

#endif
