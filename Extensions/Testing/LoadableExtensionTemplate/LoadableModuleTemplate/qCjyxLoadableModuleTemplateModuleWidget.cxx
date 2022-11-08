/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

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

// Cjyx includes
#include "qCjyxLoadableModuleTemplateModuleWidget.h"
#include "ui_qCjyxLoadableModuleTemplateModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_ExtensionTemplate
class qCjyxLoadableModuleTemplateModuleWidgetPrivate: public Ui_qCjyxLoadableModuleTemplateModuleWidget
{
public:
  qCjyxLoadableModuleTemplateModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qCjyxLoadableModuleTemplateModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qCjyxLoadableModuleTemplateModuleWidgetPrivate::qCjyxLoadableModuleTemplateModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qCjyxLoadableModuleTemplateModuleWidget methods

//-----------------------------------------------------------------------------
qCjyxLoadableModuleTemplateModuleWidget::qCjyxLoadableModuleTemplateModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qCjyxLoadableModuleTemplateModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qCjyxLoadableModuleTemplateModuleWidget::~qCjyxLoadableModuleTemplateModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qCjyxLoadableModuleTemplateModuleWidget::setup()
{
  Q_D(qCjyxLoadableModuleTemplateModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}
