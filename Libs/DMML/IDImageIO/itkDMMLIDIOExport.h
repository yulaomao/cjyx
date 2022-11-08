/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

/// itkDMMLIDIOExport
///
/// The itkDMMLIDIOExport captures some system differences between Unix
/// and Windows operating systems.

#ifndef itkDMMLIDIOExport_h
#define itkDMMLIDIOExport_h

#include <itkDMMLIDImageIOConfigure.h>

#if defined(WIN32) && !defined(DMMLIDIO_STATIC)
#if defined(DMMLIDIO_EXPORTS)
#define DMMLIDImageIO_EXPORT __declspec( dllexport )
#else
#define DMMLIDImageIO_EXPORT __declspec( dllimport )
#endif
#else
#define DMMLIDImageIO_EXPORT
#endif

#endif
