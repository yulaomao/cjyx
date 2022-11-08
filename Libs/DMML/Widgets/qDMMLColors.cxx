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

// qDMML includes
#include "qDMMLColors.h"

// VTK includes
#include <vtkColor.h>
#include "vtkDMMLColors.h"
#include <vtkVersion.h>

//------------------------------------------------------------------------------
QColor qDMMLColors::sliceRed()
{
  return qDMMLColors::fromVTKColor(vtkDMMLColors::sliceRed());
}

//------------------------------------------------------------------------------
QColor qDMMLColors::sliceGreen()
{
  return qDMMLColors::fromVTKColor(vtkDMMLColors::sliceGreen());
}

//------------------------------------------------------------------------------
QColor qDMMLColors::sliceYellow()
{
  return qDMMLColors::fromVTKColor(vtkDMMLColors::sliceYellow());
}

//------------------------------------------------------------------------------
QColor qDMMLColors::sliceOrange()
{
  return qDMMLColors::fromVTKColor(vtkDMMLColors::sliceOrange());
}

//------------------------------------------------------------------------------
QColor qDMMLColors::threeDViewBlue()
{
  return qDMMLColors::fromVTKColor(vtkDMMLColors::threeDViewBlue());
}

//------------------------------------------------------------------------------
QColor qDMMLColors::sliceGray()
{
  return qDMMLColors::fromVTKColor(vtkDMMLColors::sliceGray());
}

//------------------------------------------------------------------------------
QColor qDMMLColors::fromVTKColor(const vtkColor3d& vtkColor)
{
  return QColor::fromRgb(vtkColor.GetRed(), vtkColor.GetGreen(), vtkColor.GetBlue());
}
