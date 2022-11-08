#ifndef itkDMMLIDIOPlugin_h
#define itkDMMLIDIOPlugin_h

#include "itkObjectFactoryBase.h"

#ifdef WIN32
#ifdef DMMLIDIOPlugin_EXPORTS
#define DMMLIDIOPlugin_EXPORT __declspec(dllexport)
#else
#define DMMLIDIOPlugin_EXPORT __declspec(dllimport)
#endif
#else
#define DMMLIDIOPlugin_EXPORT
#endif

/**
 * Routine that is called when the shared library is loaded by
 * itk::ObjectFactoryBase::LoadDynamicFactories().
 *
 * itkLoad() is C (not C++) function.
 */
extern "C" {
    DMMLIDIOPlugin_EXPORT itk::ObjectFactoryBase* itkLoad();
}
#endif
