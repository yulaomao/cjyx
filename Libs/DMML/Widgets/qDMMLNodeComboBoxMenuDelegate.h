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

  This file was originally developed by Benjamin Long, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qDMMLNodeComboBoxMenuDelegate_h
#define __qDMMLNodeComboBoxMenuDelegate_h

// Qt includes
#include <QAbstractItemDelegate>

// DMMLWidgets includes
#include "qDMMLWidgetsExport.h"

class QComboBox;

/// \brief Abstract Item delegate to show unselectable items highlighed when the mouse is
/// over them to give a feeling of selectable item.
///
/// This is used in qDMMLNodeComboBox for the extra items:
/// e.g. "Add/Remove/Edit node" actions
/// The highlight color is slightly more treansparent than the style palette
/// highlight color to indicates that the item is not really selectable
/// but correspond to an action.
///
/// We reimplemente the code from Qt "QComboMenuDelegate" used in the class QComboBox.
class QDMML_WIDGETS_EXPORT qDMMLNodeComboBoxMenuDelegate: public QAbstractItemDelegate
{
  Q_OBJECT
public:
  typedef QAbstractItemDelegate Superclass;
  qDMMLNodeComboBoxMenuDelegate(QObject *parent, QComboBox *cmb);

protected:
  void paint(QPainter *painter,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) const override;
  QSize sizeHint(const QStyleOptionViewItem &option,
                         const QModelIndex &index) const override;

private:
  /// Reimplemented to force the highlight in case the item is not selectable
  /// but current. The highlight color used is then slightly different from
  /// the default color
  QStyleOptionMenuItem getStyleOption(const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const;
  QComboBox *mCombo;
  Q_DISABLE_COPY(qDMMLNodeComboBoxMenuDelegate);
};

#endif // __qDMMLNodeComboBoxMenuDelegate_h
