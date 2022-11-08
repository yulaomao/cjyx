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

#ifndef __qDMMLColors_h
#define __qDMMLColors_h

/// Qt includes
#include <QColor>

/// qDMMLWidget includes
#include "qDMMLWidgetsExport.h"

class vtkColor3d;

class QDMML_WIDGETS_EXPORT qDMMLColors
{
public:

  static QColor sliceRed();
  static QColor sliceYellow();
  static QColor sliceGreen();
  static QColor sliceOrange();
  static QColor sliceGray();

  static QColor threeDViewBlue();

  // Description
  // Utility function to convert vtkColor into QColor
  static QColor fromVTKColor(const vtkColor3d&);
};

#endif
