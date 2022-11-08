/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qDMMLListWidget_h
#define __qDMMLListWidget_h

// Qt includes
#include <QListView>

// CTK includes
#include <ctkPimpl.h>

#include "qDMMLWidgetsExport.h"

class qDMMLListWidgetPrivate;
class vtkDMMLScene;

class QDMML_WIDGETS_EXPORT qDMMLListWidget : public QListView
{
  Q_OBJECT
public:
  qDMMLListWidget(QWidget *parent=nullptr);
  ~qDMMLListWidget() override;

  vtkDMMLScene* dmmlScene()const;

public slots:
  void setDMMLScene(vtkDMMLScene* scene);

protected:
  QScopedPointer<qDMMLListWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLListWidget);
  Q_DISABLE_COPY(qDMMLListWidget);
};

#endif
