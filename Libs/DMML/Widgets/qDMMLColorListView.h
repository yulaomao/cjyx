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

#ifndef __qDMMLColorListView_h
#define __qDMMLColorListView_h

// Qt includes
#include <QListView>

// qDMML includes
#include "qDMMLWidgetsExport.h"

class QSortFilterProxyModel;
class qDMMLColorListViewPrivate;
class qDMMLColorModel;
class vtkDMMLColorNode;
class vtkDMMLNode;

class QDMML_WIDGETS_EXPORT qDMMLColorListView : public QListView
{
  Q_OBJECT
  Q_PROPERTY(bool showOnlyNamedColors READ showOnlyNamedColors WRITE setShowOnlyNamedColors)
public:
  qDMMLColorListView(QWidget *parent=nullptr);
  ~qDMMLColorListView() override;

  vtkDMMLColorNode* dmmlColorNode()const;
  qDMMLColorModel* colorModel()const;
  QSortFilterProxyModel* sortFilterProxyModel()const;

  bool showOnlyNamedColors()const;

public slots:
  void setDMMLColorNode(vtkDMMLColorNode* colorNode);
  /// Utility function to simply connect signals/slots with Qt Designer
  void setDMMLColorNode(vtkDMMLNode* colorNode);

  void setShowOnlyNamedColors(bool);

signals:
  /// Colors are selected when there is a new current item
  /// (not fired on the activated signal).
  void colorSelected(int index);
  void colorSelected(const QColor& color);
  void colorSelected(const QString& name);

protected slots:
  void currentChanged(const QModelIndex&, const QModelIndex&) override;

protected:
  QScopedPointer<qDMMLColorListViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLColorListView);
  Q_DISABLE_COPY(qDMMLColorListView);
};

#endif
