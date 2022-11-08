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

#ifndef __vtkDMMLMeasurementArea_h
#define __vtkDMMLMeasurementArea_h

// DMML includes
#include "vtkDMMLMeasurement.h"

// Markups includes
#include "vtkCjyxMarkupsModuleDMMLExport.h"

/// \brief Measurement class calculating area of a plane or enclosed by a closed curve
/// \ingroup Cjyx_QtModules_Markups
class VTK_CJYX_MARKUPS_MODULE_DMML_EXPORT vtkDMMLMeasurementArea : public vtkDMMLMeasurement
{
public:
  static vtkDMMLMeasurementArea *New();
  vtkTypeMacro(vtkDMMLMeasurementArea, vtkDMMLMeasurement);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create a new instance of this measurement type.
  VTK_NEWINSTANCE
  vtkDMMLMeasurement* CreateInstance() const override { return vtkDMMLMeasurementArea::New(); }

  /// Calculate area of \sa InputDMMLNode plane or closed curve markup node and store the result internally
  void Compute() override;

protected:
  vtkDMMLMeasurementArea();
  ~vtkDMMLMeasurementArea() override;
  vtkDMMLMeasurementArea(const vtkDMMLMeasurementArea&);
  void operator=(const vtkDMMLMeasurementArea&);
};

#endif
