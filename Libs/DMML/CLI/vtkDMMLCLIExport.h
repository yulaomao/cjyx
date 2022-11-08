/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

/// vtkDMMLCLIExport
///
/// The vtkDMMLCLIExport captures some system differences between Unix
/// and Windows operating systems.

#ifndef __vtkDMMLCLIExport_h
#define __vtkDMMLCLIExport_h

#include <vtkDMMLCLIConfigure.h>

#if defined(WIN32) && !defined(VTKDMMLCLI_STATIC)
#if defined(DMMLCLI_EXPORTS)
#define VTK_DMML_CLI_EXPORT __declspec( dllexport )
#else
#define VTK_DMML_CLI_EXPORT __declspec( dllimport )
#endif
#else
#define VTK_DMML_CLI_EXPORT
#endif

#endif
