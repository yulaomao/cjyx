/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

#ifndef __vtkDMMLLogic_h
#define __vtkDMMLLogic_h

// DMML includes
#include "vtkDMML.h"
class vtkDMMLScene;

// VTK includes
#include <vtkObject.h>

/// \brief Class that manages adding and deleting of observers with events.
///
/// Class that manages adding and deleting of obserevers with events
/// This class keeps track of obserevers and events added to each vtk object.
/// It caches tags returned by AddObserver method so that observers can be removed properly.
class VTK_DMML_EXPORT vtkDMMLLogic : public vtkObject
{
public:
  /// The Usual vtk class functions
  static vtkDMMLLogic *New();
  vtkTypeMacro(vtkDMMLLogic,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override { this->Superclass::PrintSelf(os, indent); }

  vtkDMMLScene* GetScene() {return this->Scene;};
  void SetScene(vtkDMMLScene* scene) {this->Scene = scene;};

  void RemoveUnreferencedStorageNodes();

  void RemoveUnreferencedDisplayNodes();

  /// Get application home directory.
  /// The path is retrieved from the environment variable defined by DMML_APPLICATION_HOME_DIR_ENV.
  static std::string GetApplicationHomeDirectory();

  /// Get application share subdirectory.
  /// The path is constructed by concatenating application home directory and DMML_APPLICATION_SHARE_SUBDIR.
  static std::string GetApplicationShareDirectory();

protected:
  vtkDMMLLogic();
  ~vtkDMMLLogic() override;
  vtkDMMLLogic(const vtkDMMLLogic&);
  void operator=(const vtkDMMLLogic&);

  vtkDMMLScene *Scene;
};

#endif
