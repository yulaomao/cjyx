/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __qCjyxTerminologiesModuleWidget_h
#define __qCjyxTerminologiesModuleWidget_h

// Cjyx includes
#include "qCjyxAbstractModuleWidget.h"

#include "qCjyxTerminologiesModuleExport.h"

class qCjyxTerminologiesModuleWidgetPrivate;
class vtkDMMLNode;

/// \ingroup CjyxRt_QtModules_DicomRtImport
class Q_CJYX_QTMODULES_TERMINOLOGIES_EXPORT qCjyxTerminologiesModuleWidget :
  public qCjyxAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qCjyxAbstractModuleWidget Superclass;
  qCjyxTerminologiesModuleWidget(QWidget *parent=nullptr);
  ~qCjyxTerminologiesModuleWidget() override;

protected:
  QScopedPointer<qCjyxTerminologiesModuleWidgetPrivate> d_ptr;

  void setup() override;

private:
  Q_DECLARE_PRIVATE(qCjyxTerminologiesModuleWidget);
  Q_DISABLE_COPY(qCjyxTerminologiesModuleWidget);
};

#endif
