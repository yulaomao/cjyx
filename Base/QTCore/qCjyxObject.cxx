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

// Qt includes

// QTCore includes
#include "qCjyxObject.h"

// VTK includes
#include "vtkDMMLScene.h"
#include "vtkSmartPointer.h"

//-----------------------------------------------------------------------------
class qCjyxObjectPrivate
{
public:
  vtkSmartPointer<vtkDMMLScene>              DMMLScene;
};

//-----------------------------------------------------------------------------
qCjyxObject::qCjyxObject(): d_ptr(new qCjyxObjectPrivate)
{
}

//-----------------------------------------------------------------------------
qCjyxObject::~qCjyxObject() = default;

//-----------------------------------------------------------------------------
void qCjyxObject::setDMMLScene(vtkDMMLScene* scene)
{
  Q_D(qCjyxObject);
  if (scene == d->DMMLScene)
    {
    return ;
    }
  d->DMMLScene = scene;
}

//-----------------------------------------------------------------------------
vtkDMMLScene* qCjyxObject::dmmlScene()const
{
  Q_D(const qCjyxObject);
  return d->DMMLScene;
}
