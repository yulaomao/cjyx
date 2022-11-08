/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/
///  vtkDMMLDisplayableManagerExport
///
/// The vtkDMMLDisplayableManagerExport captures some system differences between Unix
/// and Windows operating systems.

#ifndef __vtkDMMLDisplayableManagerExport_h
#define __vtkDMMLDisplayableManagerExport_h

#include <vtkDMMLDisplayableManagerConfigure.h>

#if defined(WIN32) && !defined(DMMLDisplayableManager_STATIC)
#if defined(DMMLDisplayableManager_EXPORTS)
#define VTK_DMML_DISPLAYABLEMANAGER_EXPORT __declspec( dllexport )
#else
#define VTK_DMML_DISPLAYABLEMANAGER_EXPORT __declspec( dllimport )
#endif
#else
#define VTK_DMML_DISPLAYABLEMANAGER_EXPORT
#endif

#if defined(DMMLDisplayableManager_AUTOINIT)
#include <vtkAutoInit.h>
VTK_AUTOINIT(DMMLDisplayableManager)
#endif

#endif
