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

#ifndef __qDMMLColorTableView_h
#define __qDMMLColorTableView_h

// Qt includes
#include <QTableView>
#include <QItemDelegate>

// qDMML includes
#include "qDMMLWidgetsExport.h"

class QSortFilterProxyModel;
class qDMMLColorTableViewPrivate;
class qDMMLColorModel;
class vtkDMMLColorNode;
class vtkDMMLNode;

/// \brief Table view for color table nodes.
/// Edition of color (opening dialog) and opacity (slider popup) is handled by
/// qDMMLItemDelegate.
class QDMML_WIDGETS_EXPORT qDMMLColorTableView : public QTableView
{
  Q_OBJECT
  /// This property show/hides the colors whose name are (none).
  /// false by default.
  /// \sa showOnlyNamedColors(), setShowOnlyNamedColors()
  Q_PROPERTY(bool showOnlyNamedColors READ showOnlyNamedColors WRITE setShowOnlyNamedColors)
public:
  qDMMLColorTableView(QWidget *parent=nullptr);
  ~qDMMLColorTableView() override;

  Q_INVOKABLE vtkDMMLColorNode* dmmlColorNode()const;
  Q_INVOKABLE qDMMLColorModel* colorModel()const;
  Q_INVOKABLE QSortFilterProxyModel* sortFilterProxyModel()const;

  /// Return the row of the color with name \a colorName.
  /// \sa rowFromColorIndex()
  Q_INVOKABLE int rowFromColorName(const QString& colorName)const;
  /// Return the row of the color of index \a colorIndex.
  /// \sa rowFromColorIndex()
  Q_INVOKABLE int rowFromColorIndex(int colorIndex)const;

  bool showOnlyNamedColors()const;

public slots:
  void setDMMLColorNode(vtkDMMLColorNode* colorNode);
  /// Utility function to simply connect signals/slots with Qt Designer
  void setDMMLColorNode(vtkDMMLNode* colorNode);

  void setShowOnlyNamedColors(bool);

protected:
  QScopedPointer<qDMMLColorTableViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLColorTableView);
  Q_DISABLE_COPY(qDMMLColorTableView);
};

#endif
