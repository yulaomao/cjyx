/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/
///  vtkDMMLLogicExport
///
/// The vtkDMMLLogicExport captures some system differences between Unix
/// and Windows operating systems.


#ifndef __vtkDMMLLogicExport_h
#define __vtkDMMLLogicExport_h

#include <vtkDMMLLogicConfigure.h>

#if defined(WIN32) && !defined(VTKDMMLLogic_STATIC)
#if defined(DMMLLogic_EXPORTS)
#define VTK_DMML_LOGIC_EXPORT __declspec( dllexport )
#else
#define VTK_DMML_LOGIC_EXPORT __declspec( dllimport )
#endif
#else
#define VTK_DMML_LOGIC_EXPORT
#endif

#endif
