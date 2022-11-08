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

#ifndef __qCjyxVolumeRenderingReader_h
#define __qCjyxVolumeRenderingReader_h

// Cjyx includes
#include <qCjyxFileReader.h>

// Volume Rendering includes
class qCjyxVolumeRenderingReaderPrivate;
class vtkCjyxVolumeRenderingLogic;

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_VolumeRendering
class qCjyxVolumeRenderingReader
  : public qCjyxFileReader
{
  Q_OBJECT
public:
  typedef qCjyxFileReader Superclass;
  qCjyxVolumeRenderingReader(QObject* parent = nullptr);
  qCjyxVolumeRenderingReader(vtkCjyxVolumeRenderingLogic* logic, QObject* parent = nullptr);
  ~qCjyxVolumeRenderingReader() override;

  void setVolumeRenderingLogic(vtkCjyxVolumeRenderingLogic* logic);
  vtkCjyxVolumeRenderingLogic* volumeRenderingLogic()const;

  // Reimplemented for IO specific description
  QString description()const override;
  IOFileType fileType()const override;
  QStringList extensions()const override;

  bool load(const IOProperties& properties) override;

protected:
  QScopedPointer<qCjyxVolumeRenderingReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qCjyxVolumeRenderingReader);
  Q_DISABLE_COPY(qCjyxVolumeRenderingReader);
};

#endif
