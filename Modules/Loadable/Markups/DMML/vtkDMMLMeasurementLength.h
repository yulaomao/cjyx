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

#ifndef __vtkDMMLMeasurementLength_h
#define __vtkDMMLMeasurementLength_h

// DMML includes
#include "vtkDMMLMeasurement.h"

// Markups includes
#include "vtkCjyxMarkupsModuleDMMLExport.h"

/// \brief Measurement class calculating curve length
/// \ingroup Cjyx_QtModules_Markups
class VTK_CJYX_MARKUPS_MODULE_DMML_EXPORT vtkDMMLMeasurementLength : public vtkDMMLMeasurement
{
public:
  static vtkDMMLMeasurementLength *New();
  vtkTypeMacro(vtkDMMLMeasurementLength, vtkDMMLMeasurement);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create a new instance of this measurement type.
  VTK_NEWINSTANCE
  vtkDMMLMeasurement* CreateInstance() const override { return vtkDMMLMeasurementLength::New(); }

  /// Calculate length of \sa InputDMMLNode curve markup node and store the result internally
  void Compute() override;

protected:
  vtkDMMLMeasurementLength();
  ~vtkDMMLMeasurementLength() override;
  vtkDMMLMeasurementLength(const vtkDMMLMeasurementLength&);
  void operator=(const vtkDMMLMeasurementLength&);
};

#endif
