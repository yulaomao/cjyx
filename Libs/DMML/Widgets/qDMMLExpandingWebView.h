/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __qDMMLExpandingWebView_h
#define __qDMMLExpandingWebView_h

// Qt includes
#include <QWebEngineView>

#include "qDMMLWidgetsExport.h"

class qDMMLExpandingWebViewPrivate;

// DMML includes
class vtkDMMLScene;

/// \brief qDMMLExpandingWebView is the display canvas for some or all of a DMML scene.
///
/// qDMMLExpandingWebView is currently implemented as a subclass of QWebView
class QDMML_WIDGETS_EXPORT qDMMLExpandingWebView :
    public QWebEngineView
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef QWebEngineView Superclass;

  /// Constructors
  explicit qDMMLExpandingWebView(QWidget* parent = nullptr);
  ~qDMMLExpandingWebView() override;

  /// Return a pointer on the current DMML scene
  vtkDMMLScene* dmmlScene() const;

  // Redefine the sizeHint so layouts work properly.
  QSize sizeHint() const override;

public slots:

  /// Set the DMML \a scene that should be listened for events
  void setDMMLScene(vtkDMMLScene* newScene);

  /// subclasses reimplement this to handle updates in the dmml scene
  virtual void updateWidgetFromDMML();

signals:

  /// When designing custom qDMMLWidget in the designer, you can connect the
  /// dmmlSceneChanged signal directly to the aggregated DMML widgets that
  /// have a setDMMLScene slot.
  void dmmlSceneChanged(vtkDMMLScene*);

protected:
  QScopedPointer<qDMMLExpandingWebViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLExpandingWebView);
  Q_DISABLE_COPY(qDMMLExpandingWebView);
};

#endif
