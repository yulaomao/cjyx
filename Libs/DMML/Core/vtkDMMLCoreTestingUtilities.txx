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
  and was partially funded by NIH grant 1U24CA194354-01

==============================================================================*/

#ifndef __vtkDMMLCoreTestingUtilities_txx
#define __vtkDMMLCoreTestingUtilities_txx

// DMML includes
#include "vtkDMMLCoreTestingUtilities.h"

namespace vtkDMMLCoreTestingUtilities
{

//----------------------------------------------------------------------------
template<typename Type>
std::string ToString(Type value)
{
  std::ostringstream stream;
  stream << value;
  return stream.str();
}

} // namespace vtkDMMLCoreTestingUtilities

#endif
