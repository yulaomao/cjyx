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

#ifndef __vtkDMMLMeasurementVolume_h
#define __vtkDMMLMeasurementVolume_h

// DMML includes
#include "vtkDMMLMeasurement.h"

// Markups includes
#include "vtkCjyxMarkupsModuleDMMLExport.h"

/// \brief Measurement class calculating Volume enclosed in a ROI
/// \ingroup Cjyx_QtModules_Markups
class VTK_CJYX_MARKUPS_MODULE_DMML_EXPORT vtkDMMLMeasurementVolume : public vtkDMMLMeasurement
{
public:
  static vtkDMMLMeasurementVolume *New();
  vtkTypeMacro(vtkDMMLMeasurementVolume, vtkDMMLMeasurement);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create a new instance of this measurement type.
  VTK_NEWINSTANCE
  vtkDMMLMeasurement* CreateInstance() const override { return vtkDMMLMeasurementVolume::New(); }

  /// Calculate Volume of \sa InputDMMLNode markups ROI node and store the result internally
  void Compute() override;

protected:
  vtkDMMLMeasurementVolume();
  ~vtkDMMLMeasurementVolume() override;
  vtkDMMLMeasurementVolume(const vtkDMMLMeasurementVolume&);
  void operator=(const vtkDMMLMeasurementVolume&);
};

#endif
