/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

/// vtkDMMLExport
///
/// The vtkDMMLExport captures some system differences between Unix
/// and Windows operating systems.

#ifndef __vtkDMMLExport_h
#define __vtkDMMLExport_h

#include <vtkDMMLConfigure.h>

#if defined(WIN32) && !defined(VTKDMML_STATIC)
#if defined(DMMLCore_EXPORTS)
#define VTK_DMML_EXPORT __declspec( dllexport )
#else
#define VTK_DMML_EXPORT __declspec( dllimport )
#endif
#else
#define VTK_DMML_EXPORT
#endif

#endif
