/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLRemoteIOLogic.h,v $
  Date:      $Date: 2011-01-07 10:39:33 -0500 (Fri, 07 Jan 2011) $
  Version:   $Revision: 15750 $

=========================================================================auto=*/

///  vtkDMMLRemoteIOLogic - DMML logic class for color manipulation
///
/// This class manages the logic associated with instrumenting
/// a DMML scene instance with RemoteIO functionality (so vtkDMMLStorableNodes
/// can access data by URL)

#ifndef __vtkDMMLRemoteIOLogic_h
#define __vtkDMMLRemoteIOLogic_h

// DMMLLogic includes
#include "vtkDMMLAbstractLogic.h"

// STD includes
#include <cstdlib>

class vtkCacheManager;
class vtkDataIOManager;

class VTK_DMML_LOGIC_EXPORT vtkDMMLRemoteIOLogic : public vtkDMMLAbstractLogic
{
public:
  /// The Usual vtk class functions
  static vtkDMMLRemoteIOLogic *New();
  vtkTypeMacro(vtkDMMLRemoteIOLogic,vtkDMMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void AddDataIOToScene();
  void RemoveDataIOFromScene();

  ///
  /// Accessors for the comonents of the remote IO infrascucture
  /// Note that the internal instances are created in the constructor
  /// and used when calling AddDataIOToScene
  /// and RemoveDataIOFromScene
  /// The Get methods can be used elsewhere, but the set methods
  /// should only be used for debuggin
  vtkGetObjectMacro (CacheManager, vtkCacheManager);
  virtual void SetCacheManager(vtkCacheManager*);
  vtkGetObjectMacro (DataIOManager, vtkDataIOManager);
  virtual void SetDataIOManager(vtkDataIOManager*);

protected:
  vtkDMMLRemoteIOLogic();
  ~vtkDMMLRemoteIOLogic() override;
  // disable copy constructor and operator
  vtkDMMLRemoteIOLogic(const vtkDMMLRemoteIOLogic&);
  void operator=(const vtkDMMLRemoteIOLogic&);

  vtkCacheManager *          CacheManager;
  vtkDataIOManager *         DataIOManager;
};

#endif
