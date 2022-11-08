// Qt includes
#include <QVariant>

// QCjyx includes
#include "qCjyxApplication.h"
#include "qCjyxLayoutManager.h"

// SceneViewWidget includes
#include "qCjyxSceneViewsModuleDialog.h"

// SceneViewLogic includes
#include <vtkCjyxSceneViewsModuleLogic.h>

// DMML includes
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkStdString.h>

// STD includes
#include <vector>

//-----------------------------------------------------------------------------
qCjyxSceneViewsModuleDialog::qCjyxSceneViewsModuleDialog(QWidget* parent/*=nullptr*/)
  : qDMMLScreenShotDialog(parent)
{
  this->m_Logic = nullptr;
  this->setLayoutManager(qCjyxApplication::application()->layoutManager());
  this->setShowScaleFactorSpinBox(false);
  this->setWindowTitle(tr("3D Cjyx SceneView"));

  // default name
  QString name("SceneView");
  this->setNameEdit(name);
}

//-----------------------------------------------------------------------------
qCjyxSceneViewsModuleDialog::~qCjyxSceneViewsModuleDialog()
{

  if(this->m_Logic)
    {
    this->m_Logic = nullptr;
    }
}

//-----------------------------------------------------------------------------
void qCjyxSceneViewsModuleDialog::setLogic(vtkCjyxSceneViewsModuleLogic* logic)
{
  if (!logic)
    {
    qErrnoWarning("setLogic: We need the SceneViews module logic here!");
    return;
    }
  this->m_Logic = logic;
}

//-----------------------------------------------------------------------------
void qCjyxSceneViewsModuleDialog::loadNode(const QString& nodeId)
{
  if (!this->m_Logic)
    {
    qErrnoWarning("initialize: We need the SceneViews module logic here!");
    return;
    }
  this->setLayoutManager(qCjyxApplication::application()->layoutManager());
  this->setData(QVariant(nodeId));

  // get the name..
  vtkStdString name = this->m_Logic->GetSceneViewName(nodeId.toUtf8());

  // ..and set it in the GUI
  this->setNameEdit(QString::fromStdString(name));

  // get the description..
  vtkStdString description = this->m_Logic->GetSceneViewDescription(nodeId.toUtf8());

  // ..and set it in the GUI
  this->setDescription(QString::fromStdString(description));

  // get the screenshot type..
  int screenshotType = this->m_Logic->GetSceneViewScreenshotType(nodeId.toUtf8());

  // ..and set it in the GUI
  this->setWidgetType((qDMMLScreenShotDialog::WidgetType)screenshotType);

  vtkImageData* imageData = this->m_Logic->GetSceneViewScreenshot(nodeId.toUtf8());
  this->setImageData(imageData);
}

//-----------------------------------------------------------------------------
void qCjyxSceneViewsModuleDialog::reset()
{
  QString name = this->nameEdit();
  if (name.length() == 0)
    {
    name = QString("SceneView");
    }
  // check to see if it's an already used name for a node (redrawing the
  // dialog causes it to reset and calling GetUniqueNameByString increments
  // the number each time).
  QByteArray nameBytes = name.toUtf8();
  vtkCollection* col = this->m_Logic->GetDMMLScene()->GetNodesByName(nameBytes.data());
  if (col->GetNumberOfItems() > 0)
    {
    // get a new unique name
    name = this->m_Logic->GetDMMLScene()->GetUniqueNameByString(name.toUtf8());
    }
  col->Delete();
  this->resetDialog();
  this->setNameEdit(name);
}

//-----------------------------------------------------------------------------
void qCjyxSceneViewsModuleDialog::accept()
{
  // name
  QString name = this->nameEdit();
  QByteArray nameBytes = name.toUtf8();

  // description
  QString description = this->description();
  QByteArray descriptionBytes = description.toUtf8();

  // we need to know of which type the screenshot is
  int screenshotType = static_cast<int>(this->widgetType());

  if (this->data().toString().isEmpty())
    {
    // this is a new SceneView
    this->m_Logic->CreateSceneView(nameBytes.data(),descriptionBytes.data(),
                                   screenshotType,this->imageData());
    //QMessageBox::information(this, "3D Cjyx SceneView created",
    //             "A new SceneView was created and the current scene was attached.");
    }
  else
    {
    // this SceneView already exists
    this->m_Logic->ModifySceneView(vtkStdString(this->data().toString().toUtf8()),nameBytes.data(),descriptionBytes.data()
                                   ,screenshotType,this->imageData());
    //QMessageBox::information(this, "3D Cjyx SceneView updated",
    //             The SceneView was updated without changing the attached scene.");
    }
  this->Superclass::accept();
}

