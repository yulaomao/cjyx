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

/// QtCore includes
#include "qCjyxIO.h"

// DMML includes
#include <vtkDMMLMessageCollection.h>

// CTK includes
#include <ctkPimpl.h>

//-----------------------------------------------------------------------------
class qCjyxIOPrivate
{
  Q_DECLARE_PUBLIC(qCjyxIO);
protected:
  qCjyxIO* q_ptr;
public:
  qCjyxIOPrivate(qCjyxIO& object);
  virtual ~qCjyxIOPrivate();

  vtkDMMLMessageCollection* UserMessages;
};

//-----------------------------------------------------------------------------
// qCjyxIOPrivate methods

//-----------------------------------------------------------------------------
qCjyxIOPrivate::qCjyxIOPrivate(qCjyxIO& object)
  : q_ptr(&object)
{
  this->UserMessages = vtkDMMLMessageCollection::New();
}

//-----------------------------------------------------------------------------
qCjyxIOPrivate::~qCjyxIOPrivate()
{
  this->UserMessages->Delete();
  this->UserMessages = nullptr;
}

//-----------------------------------------------------------------------------
// qCjyxIO methods

CTK_GET_CPP(qCjyxIO, vtkDMMLMessageCollection*, userMessages, UserMessages);

//----------------------------------------------------------------------------
qCjyxIO::qCjyxIO(QObject* parentObject)
  : Superclass(parentObject)
  , d_ptr(new qCjyxIOPrivate(*this))
{
  qRegisterMetaType<qCjyxIO::IOFileType>("qCjyxIO::IOFileType");
  qRegisterMetaType<qCjyxIO::IOProperties>("qCjyxIO::IOProperties");
}

//----------------------------------------------------------------------------
qCjyxIO::~qCjyxIO() = default;

//----------------------------------------------------------------------------
qCjyxIOOptions* qCjyxIO::options()const
{
  return nullptr;
}
