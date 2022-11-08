/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Cjyx
 Module:    $RCSfile: vtkDMMLAnnotationClickCounter.h,v $
 Date:      $Date: Aug 4, 2010 10:44:52 AM $
 Version:   $Revision: 1.0 $

 =========================================================================auto=*/

#ifndef __vtkDMMLAnnotationClickCounter_h
#define __vtkDMMLAnnotationClickCounter_h

// Annotation includes
#include "vtkCjyxAnnotationsModuleDMMLDisplayableManagerExport.h"

// VTK include
#include <vtkObject.h>

/// \ingroup Cjyx_QtModules_Annotation
class VTK_CJYX_ANNOTATIONS_MODULE_DMMLDISPLAYABLEMANAGER_EXPORT
vtkDMMLAnnotationClickCounter
  : public vtkObject
{
public:

  static vtkDMMLAnnotationClickCounter *New();
  vtkTypeMacro(vtkDMMLAnnotationClickCounter, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Increase the click counter and return the number of clicks.
  /// \sa HasEnoughClicks()
  int Click();

  /// Check if enough clicks are counted and reset the click number if it
  /// is equal to \a clicks
  /// \sa Click(), Reset()
  bool HasEnoughClicks(int clicks);

  /// Reset the click counter
  void Reset();

protected:

  vtkDMMLAnnotationClickCounter();
  ~vtkDMMLAnnotationClickCounter() override;

private:

  vtkDMMLAnnotationClickCounter(const vtkDMMLAnnotationClickCounter&) = delete;
  void operator=(const vtkDMMLAnnotationClickCounter&) = delete;

  int m_Clicks;

};

#endif /* __vtkDMMLAnnotationClickCounter_h */
