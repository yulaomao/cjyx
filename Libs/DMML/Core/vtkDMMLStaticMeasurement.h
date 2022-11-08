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

#ifndef __vtkDMMLStaticMeasurement_h
#define __vtkDMMLStaticMeasurement_h

// DMML includes
#include "vtkDMMLMeasurement.h"

/// \brief Measurement class storing a constant measurement.
///
/// Typically all measurements calculate their own value from its input
/// DMML node. This class is to be able to store constant measurements.
///
/// \ingroup Cjyx_QtModules_Markups
class VTK_DMML_EXPORT vtkDMMLStaticMeasurement : public vtkDMMLMeasurement
{
public:
  static vtkDMMLStaticMeasurement *New();
  vtkTypeMacro(vtkDMMLStaticMeasurement, vtkDMMLMeasurement);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create a new instance of this measurement type.
  VTK_NEWINSTANCE
  vtkDMMLMeasurement* CreateInstance() const override { return vtkDMMLStaticMeasurement::New(); }

  /// Do nothing to compute the measurement as it is static
  void Compute() override;

protected:
  vtkDMMLStaticMeasurement();
  ~vtkDMMLStaticMeasurement() override;
  vtkDMMLStaticMeasurement(const vtkDMMLStaticMeasurement&);
  void operator=(const vtkDMMLStaticMeasurement&);
};

#endif
