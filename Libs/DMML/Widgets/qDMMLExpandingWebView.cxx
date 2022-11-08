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

// Qt includes
#include <QDebug>
#include <QEvent>
#include <QFileInfo>
#include <QHBoxLayout>

// qDMML includes
#include "qDMMLExpandingWebView_p.h"

// DMML includes
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

const char *htmlPreamble =
  "<!DOCTYPE html>"
  "<html>"
  "<head>"
  "<title>Scene</title>"
  "</head>"
  "<body>";

const char *htmlPostscript =
  "</body>"
  "</html>";



//--------------------------------------------------------------------------
// qDMMLExpandingWebViewPrivate methods

//---------------------------------------------------------------------------
qDMMLExpandingWebViewPrivate::qDMMLExpandingWebViewPrivate(qDMMLExpandingWebView& object)
  : q_ptr(&object)
{
  this->DMMLScene = nullptr;
}

//---------------------------------------------------------------------------
qDMMLExpandingWebViewPrivate::~qDMMLExpandingWebViewPrivate() = default;

//---------------------------------------------------------------------------
void qDMMLExpandingWebViewPrivate::init()
{
  Q_Q(qDMMLExpandingWebView);

  // Let the QWebView expand in both directions
  q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  q->setHtml("");
  q->show();
}

//---------------------------------------------------------------------------
void qDMMLExpandingWebViewPrivate::setDMMLScene(vtkDMMLScene* newScene)
{
  //Q_Q(qDMMLExpandingWebView);
  if (newScene == this->DMMLScene)
    {
    return;
    }

  this->qvtkReconnect(
    this->dmmlScene(), newScene,
    vtkDMMLScene::StartBatchProcessEvent, this, SLOT(startProcessing()));

  this->qvtkReconnect(
    this->dmmlScene(), newScene,
    vtkDMMLScene::EndBatchProcessEvent, this, SLOT(endProcessing()));

  this->DMMLScene = newScene;
}


// --------------------------------------------------------------------------
void qDMMLExpandingWebViewPrivate::startProcessing()
{
//  Q_Q(qDMMLExpandingWebView);
}

//
// --------------------------------------------------------------------------
void qDMMLExpandingWebViewPrivate::endProcessing()
{
  Q_Q(qDMMLExpandingWebView);
  q->updateWidgetFromDMML();
}

// --------------------------------------------------------------------------
vtkDMMLScene* qDMMLExpandingWebViewPrivate::dmmlScene()
{
  return this->DMMLScene;
}


// --------------------------------------------------------------------------
// qDMMLExpandingWebView methods

// --------------------------------------------------------------------------
qDMMLExpandingWebView::qDMMLExpandingWebView(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qDMMLExpandingWebViewPrivate(*this))
{
  Q_D(qDMMLExpandingWebView);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLExpandingWebView::~qDMMLExpandingWebView()
{
  this->setDMMLScene(nullptr);
}


//------------------------------------------------------------------------------
void qDMMLExpandingWebView::setDMMLScene(vtkDMMLScene* newScene)
{
  Q_D(qDMMLExpandingWebView);
  if (newScene == d->DMMLScene)
    {
    return;
    }

  d->setDMMLScene(newScene);

  emit dmmlSceneChanged(newScene);
}

//---------------------------------------------------------------------------
vtkDMMLScene* qDMMLExpandingWebView::dmmlScene()const
{
  Q_D(const qDMMLExpandingWebView);
  return d->DMMLScene;
}

// --------------------------------------------------------------------------
void qDMMLExpandingWebView::updateWidgetFromDMML()
{
  //qDebug() << "qDMMLExpandingWebView::updateWidgetFromDMML()";

  if (!this->dmmlScene())
    {
    return;
    }

  if (!this->isEnabled())
    {
    return;
    }

  /*
  // get a node
  char *nodeid = 0;

  if (!nodeid)
    {
    q->setHtml("");
    q->show();
    return;
    }
  */

  // Assemble the page
  //
  // 1. HTML page preamble
  // 2 to n-1: customise
  // n. HTML page poscript
  //
  QStringList html;
  html << htmlPreamble;       // 1. page header, css, javascript

  html << htmlPostscript;   // 5. page postscript, additional javascript

  //qDebug() << html.join("");

  // show the html
  this->setHtml(html.join(""));
  this->show();

}

//---------------------------------------------------------------------------
QSize qDMMLExpandingWebView::sizeHint()const
{
  // return a default size hint (invalid size)
  return QSize();
}
