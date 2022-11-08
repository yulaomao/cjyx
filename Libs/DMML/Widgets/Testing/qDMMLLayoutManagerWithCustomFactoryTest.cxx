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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Common test driver includes
#include "qDMMLWidgetCxxTests.h"
#include "qDMMLLayoutManagerTestHelper.cxx"

// Qt includes
#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

// Cjyx includes
#include "qDMMLLayoutManager.h"
#include "qDMMLLayoutManager_p.h"
#include "qDMMLSliceControllerWidget.h"
#include "qDMMLSliceWidget.h"
#include "vtkCjyxConfigure.h"

// DMML includes
#include <vtkDMMLApplicationLogic.h>
#include <vtkDMMLLayoutLogic.h>
#include <vtkDMMLLayoutNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSliceLogic.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLSliceViewDisplayableManagerFactory.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkWeakPointer.h>
#include "qDMMLWidget.h"

//------------------------------------------------------------------------------
class qCjyxLayoutCustomSliceViewFactory
 : public qDMMLLayoutSliceViewFactory
{
  Q_OBJECT
public:
  typedef qDMMLLayoutSliceViewFactory Superclass;
  qCjyxLayoutCustomSliceViewFactory(QObject* parent):Superclass(parent)
  {
    this->LastNode = nullptr;
  }

  ~qCjyxLayoutCustomSliceViewFactory() override = default;

  vtkWeakPointer<vtkDMMLSliceNode> LastNode;

protected:
  QWidget* createViewFromNode(vtkDMMLAbstractViewNode* viewNode) override
  {
    if (!this->layoutManager() || !viewNode)
      {// can't create a slice widget if there is no parent widget
      Q_ASSERT(viewNode);
      return nullptr;
      }

    // there is a unique slice widget per node
    Q_ASSERT(!this->viewWidget(viewNode));

    this->LastNode = vtkDMMLSliceNode::SafeDownCast(viewNode);

    qDMMLSliceWidget * sliceWidget = new qDMMLSliceWidget(this->layoutManager()->viewport());
    sliceWidget->sliceController()->setControllerButtonGroup(this->SliceControllerButtonGroup);
    QString sliceLayoutName(viewNode->GetLayoutName());
    QString sliceViewLabel(viewNode->GetLayoutLabel());
    vtkDMMLSliceNode* sliceNode = vtkDMMLSliceNode::SafeDownCast(viewNode);
    QColor sliceLayoutColor = QColor::fromRgbF(sliceNode->GetLayoutColor()[0],
                                               sliceNode->GetLayoutColor()[1],
                                               sliceNode->GetLayoutColor()[2]);
    sliceWidget->setSliceViewName(sliceLayoutName);
    sliceWidget->setObjectName(QString("qDMMLSliceWidget" + sliceLayoutName));
    sliceWidget->setSliceViewLabel(sliceViewLabel);
    sliceWidget->setSliceViewColor(sliceLayoutColor);
    sliceWidget->setDMMLScene(this->dmmlScene());
    sliceWidget->setDMMLSliceNode(sliceNode);
    sliceWidget->setSliceLogics(this->sliceLogics());

    this->sliceLogics()->AddItem(sliceWidget->sliceLogic());

    QLabel* label = new QLabel();
    label->setText("This is a SliceView customization");

    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(sliceWidget);
    layout->addWidget(label);

    QWidget * widget = new QWidget(this->layoutManager()->viewport());
    widget->setObjectName("CustomSliceWidget");
    widget->setLayout(layout);

    return widget;
  }
};

//------------------------------------------------------------------------------
class vtkDMMLCustomViewNode
  : public vtkDMMLAbstractViewNode
{
public:
  static vtkDMMLCustomViewNode *New();
  vtkTypeMacro(vtkDMMLCustomViewNode, vtkDMMLAbstractViewNode);

  //--------------------------------------------------------------------------
  /// DMMLNode methods
  //--------------------------------------------------------------------------

  vtkDMMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override
  {
    return "CustomView";
  }

protected:
  vtkDMMLCustomViewNode() = default;
  ~vtkDMMLCustomViewNode() override = default;
  vtkDMMLCustomViewNode(const vtkDMMLCustomViewNode&);
  void operator=(const vtkDMMLCustomViewNode&);
};

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLCustomViewNode);

//------------------------------------------------------------------------------
class qDMMLLayoutCustomViewFactory
 : public qDMMLLayoutViewFactory
{
  Q_OBJECT
public:
  typedef qDMMLLayoutViewFactory Superclass;
  qDMMLLayoutCustomViewFactory(QObject* parent) : Superclass(parent)
  {
    this->LastNode = nullptr;
  }
  ~qDMMLLayoutCustomViewFactory() override = default;

  QString viewClassName()const override
  {
    return "vtkDMMLCustomViewNode";
  }

  vtkWeakPointer<vtkDMMLCustomViewNode> LastNode;

protected:

  QWidget* createViewFromNode(vtkDMMLAbstractViewNode* viewNode) override
  {
    if (!viewNode || !this->layoutManager() || !this->layoutManager()->viewport())
      {
      Q_ASSERT(viewNode);
      return nullptr;
      }

    // There must be a unique Custom widget per node
    Q_ASSERT(!this->viewWidget(viewNode));

    this->LastNode = vtkDMMLCustomViewNode::SafeDownCast(viewNode);

    QLabel* label = new QLabel();
    label->setText("This is a custom view");

    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(label);

    QWidget * widget = new QWidget(this->layoutManager()->viewport());
    widget->setObjectName("CustomWidget");
    widget->setLayout(layout);

    return widget;
  }
};

//------------------------------------------------------------------------------
int qDMMLLayoutManagerWithCustomFactoryTest(int argc, char * argv[] )
{
  (void)checkViewArrangement; // Fix -Wunused-function warning

  qDMMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qDMMLWidget::postInitializeApplication();

  QWidget w;
  w.show();

  qDMMLLayoutManager layoutManager(&w, &w);

  vtkNew<vtkDMMLApplicationLogic> applicationLogic;
  vtkDMMLSliceViewDisplayableManagerFactory::GetInstance()->SetDMMLApplicationLogic(applicationLogic);

  vtkNew<vtkDMMLScene> scene;
  vtkNew<vtkDMMLLayoutNode> layoutNode;

  scene->RegisterNodeClass(vtkSmartPointer<vtkDMMLCustomViewNode>::New());

  scene->AddNode(layoutNode.GetPointer());

  applicationLogic->SetDMMLScene(scene.GetPointer());
  layoutManager.setDMMLScene(scene.GetPointer());

  // Unregister regular SliceView factory and register a custom one
  qDMMLLayoutSliceViewFactory* dmmlSliceViewFactory =
      qobject_cast<qDMMLLayoutSliceViewFactory*>(
        layoutManager.dmmlViewFactory("vtkDMMLSliceNode"));

  qCjyxLayoutCustomSliceViewFactory* customSliceViewFactory =
      new qCjyxLayoutCustomSliceViewFactory(&layoutManager);

  layoutManager.unregisterViewFactory(dmmlSliceViewFactory);
  layoutManager.registerViewFactory(customSliceViewFactory);

  // Register a factory for vtkDMMLCustomViewNode
  qDMMLLayoutCustomViewFactory* customViewFactory =
      new qDMMLLayoutCustomViewFactory(&layoutManager);
  layoutManager.registerViewFactory(customViewFactory);

  int customLayout = vtkDMMLLayoutNode::CjyxLayoutUserView + 1;
  const char* customLayoutDescription =
      "<layout type=\"horizontal\">"
      "      <item>"
      "        <view class=\"vtkDMMLSliceNode\" singletontag=\"CustomSliceView\">"
      "          <property name=\"HideFromEditors\" action=\"default\">true</property>"
      "        </view>"
      "      </item>"
      "      <item>"
      "        <view class=\"vtkDMMLCustomViewNode\" singletontag=\"CustomView\">"
      "          <property name=\"HideFromEditors\" action=\"default\">true</property>"
      "        </view>"
      "      </item>"
      "</layout>";
  layoutNode->AddLayoutDescription(customLayout, customLayoutDescription);


  layoutNode->SetViewArrangement(vtkDMMLLayoutNode::CjyxLayoutOneUpRedSliceView);

  QWidget* sliceWidget = layoutManager.viewWidget(customSliceViewFactory->LastNode);

  if (!sliceWidget)
    {
    std::cerr << "Line " << __LINE__
              << " - Problem with qDMMLLayoutManager::viewWidget function: "
              << "Non null sliceWidget is expected."
              << std::endl;
    return EXIT_FAILURE;
    }

  if (sliceWidget->objectName() != "CustomSliceWidget")
    {
    std::cerr << "Line " << __LINE__
              << " - Problem with qDMMLLayoutManager::viewWidget function: "
              << "Widget with 'CustomSliceWidget' as object name is expected."
              << std::endl;
    return EXIT_FAILURE;
    }

  layoutNode->SetViewArrangement(customLayout);

  sliceWidget = layoutManager.viewWidget(customSliceViewFactory->LastNode);

  if (!sliceWidget)
    {
    std::cerr << "Line " << __LINE__
              << " - Problem with qDMMLLayoutManager::viewWidget function: "
              << "Non null sliceWidget is expected."
              << std::endl;
    return EXIT_FAILURE;
    }

  if (sliceWidget->objectName() != "CustomSliceWidget")
    {
    std::cerr << "Line " << __LINE__
              << " - Problem with qDMMLLayoutManager::viewWidget function: "
              << "Widget with 'CustomSliceWidget' as object name is expected."
              << std::endl;
    return EXIT_FAILURE;
    }

  QWidget* customWidget = layoutManager.viewWidget(customViewFactory->LastNode);

  if (!customWidget)
    {
    std::cerr << "Line " << __LINE__
              << " - Problem with qDMMLLayoutManager::viewWidget function: "
              << "Non null customWidget is expected."
              << std::endl;
    return EXIT_FAILURE;
    }

  if (customWidget->objectName() != "CustomWidget")
    {
    std::cerr << "Line " << __LINE__
              << " - Problem with qDMMLLayoutManager::viewWidget function: "
              << "Widget with 'CustomWidget' as object name is expected."
              << std::endl;
    return EXIT_FAILURE;
    }

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    return safeApplicationQuit(&app);
    }
  else
    {
    return app.exec();
    }
}

#include "moc_qDMMLLayoutManagerWithCustomFactoryTest.cxx"
