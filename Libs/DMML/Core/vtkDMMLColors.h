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

  This file was originally developed by Michael Jeulin-L, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkDMMLColors_h
#define __vtkDMMLColors_h

#include "vtkDMMLExport.h"

class vtkColor3d;

class VTK_DMML_EXPORT vtkDMMLColors
{
public:

  static vtkColor3d sliceRed();
  static vtkColor3d sliceYellow();
  static vtkColor3d sliceGreen();
  static vtkColor3d sliceOrange();
  static vtkColor3d sliceGray();

  static vtkColor3d threeDViewBlue();

  // Description
  // Utility function to convert vtkColor into double[3]
  static bool toRGBColor(const vtkColor3d& from, double to[3]);
  static bool toRGBColor(const char* fromHexadecimal, double to[3]);
};

#endif
