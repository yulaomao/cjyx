#include "itkDMMLIDIOPlugin.h"
#include "itkDMMLIDImageIOFactory.h"

/**
 * Routine that is called when the shared library is loaded by
 * itk::ObjectFactoryBase::LoadDynamicFactories().
 *
 * itkLoad() is C (not C++) function.
 */
itk::ObjectFactoryBase* itkLoad()
{
  static itk::DMMLIDImageIOFactory::Pointer f
    = itk::DMMLIDImageIOFactory::New();
  return f;
}
