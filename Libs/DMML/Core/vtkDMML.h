/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   DMML
  Module:    $RCSfile: vtkDMML.h,v $
  Date:      $Date$
  Version:   $Rev$

=========================================================================auto=*/

/*
 * This is needed for loading dmml code as module.
 */

#include "vtkDMMLExport.h"

// Macro for DMML application version comparison in preprocessor macros.
// Example:
// #if DMML_APPLICATION_SUPPORT_VERSION < DMML_VERSION_CHECK(4, 4, 0)
// ...
// #endif
#define DMML_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))
