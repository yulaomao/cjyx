#ifndef __qCjyxAnnotationModulePropertyDialog_h
#define __qCjyxAnnotationModulePropertyDialog_h

#include <QLineEdit>
#include "ui_qCjyxAnnotationModulePropertyDialogNew.h"

#include "qCjyxAbstractModuleWidget.h"

#include "vtkStdString.h"

class vtkCjyxAnnotationModuleLogic;
class vtkDMMLAnnotationLinesNode;
class vtkDMMLAnnotationControlPointsNode;
class vtkDMMLAnnotationNode;
class vtkDMMLNode;
class vtkDMMLAnnotationDisplayNode;
class vtkDMMLAnnotationLineDisplayNode;
class vtkDMMLAnnotationPointDisplayNode;
class vtkDMMLAnnotationRulerNode;
class vtkDMMLAnnotationFiducialNode;

/// \ingroup Cjyx_QtModules_Annotation
class qCjyxAnnotationModulePropertyDialog : public QDialog
{
  Q_OBJECT

public:
  qCjyxAnnotationModulePropertyDialog(const char * id, vtkCjyxAnnotationModuleLogic* logic);
  ~qCjyxAnnotationModulePropertyDialog();

  const char * GetID() { return this->m_id; }


  void updateTextFromTable(QString text);
  void updateValue(QString valueString);

  void SaveStateForUndo(vtkDMMLNode* node);
  void Undo(vtkDMMLNode* node);
  void SetButtonText(int type);
  void UpdateLockUnlockStatus(bool isLock);

  static void TurnColorArrayToQColor(double* color, QColor &qcolor);
  static void TurnQColorToColorArray(double* color, QColor &qcolor);
  static void FormatValueToChar(const char* format, std::vector<double> vv, QString &valueString );


protected:

protected slots:

    void onCoordinateChanged(QString text);

    //void onPointsTableWidgetChanged(QTableWidgetItem *tableItem);
    void onPointColorChanged(QColor qcolor);
    void onPointSelectedColorChanged(QColor qcolor);
    void onPointSizeChanged(double value);
    void onPointOpacityChanged(double value);
    void onPointAmbientChanged(double value);
    void onPointDiffuseChanged(double value);
    void onPointSpecularChanged(double value);
    void onPointGlyphChanged(QString value);
    void onLineColorChanged(QColor qcolor);
    void onLineSelectedColorChanged(QColor qcolor);
    void onLineWidthChanged(double value);
    void onLineLabelPositionChanged(double value);
    void onLineLabelVisibilityStateChanged(int state);
    void onLineTickSpacingChanged();
    void onLineMaxTicksChanged(double value);
    void onLineOpacityChanged(double value);
    void onLineAmbientChanged(double value);
    void onLineDiffuseChanged(double value);
    void onLineSpecularChanged(double value);

  void onTextChanged();
  void onDialogRejected();
  void onDialogAccepted();
  void onTextUnselectedColorChanged(QColor qcolor);
  void onTextSelectedColorChanged(QColor qcolor);
  void onTextScaleChanged(double value);
  void onTextOpacityChanged(double value);

  void onLockUnlockButtonClicked();
  void onVisibleInvisibleButtonClicked();


signals:
    void coordinateChanged(QString, char*);
    void textChanged();
    void textChanged(const QString &);
    void dialogRejected();
    void dialogAccepted();
    void doublePropertyChanged(double, char*, int);
    void colorPropertyChanged(QColor, char*, int);
    //void itemChanged(QTableWidgetItem *);

private:

    void SaveLinesNode(vtkDMMLAnnotationLinesNode* node);
    void SaveControlPoints(vtkDMMLAnnotationControlPointsNode* node);
    void SaveAnnotationNode(vtkDMMLAnnotationNode* node);
    void UndoLinesNode(vtkDMMLAnnotationLinesNode* node);
    void UndoControlPoints(vtkDMMLAnnotationControlPointsNode* node);
    void UndoAnnotationNode(vtkDMMLAnnotationNode* node);

    // approved code starts here
    Ui::qCjyxAnnotationModulePropertyDialogNew ui;

    vtkStdString m_id;
    vtkCjyxAnnotationModuleLogic* m_logic;

    // create the slot and signal connections
    void createConnection();

    // initialize the GUI in respect to values from the logic
    void initialize();

    // lock/unlock the complete interface
    void lockUnlockInterface(bool lock);
};

#endif
