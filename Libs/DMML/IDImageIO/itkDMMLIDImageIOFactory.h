/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef itkDMMLIDImageIOFactory_h
#define itkDMMLIDImageIOFactory_h

#include "itkObjectFactoryBase.h"
#include "itkImageIOBase.h"

#include "itkDMMLIDImageIO.h"

#include "itkDMMLIDIOExport.h"

namespace itk
{
/** \class DMMLIDImageIOFactory
 * \brief Create instances of DMMLIDImageIO objects using an object factory.
 */
class DMMLIDImageIO_EXPORT DMMLIDImageIOFactory : public ObjectFactoryBase
{
public:
  /** Standard class typedefs. */
  typedef DMMLIDImageIOFactory      Self;
  typedef ObjectFactoryBase         Superclass;
  typedef SmartPointer<Self>        Pointer;
  typedef SmartPointer<const Self>  ConstPointer;

  /** Class methods used to interface with the registered factories. */
  const char* GetITKSourceVersion() const override;
  const char* GetDescription() const override;

  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self);
  static DMMLIDImageIOFactory* FactoryNew() { return new DMMLIDImageIOFactory;}

  /** Run-time type information (and related methods). */
  itkTypeMacro(DMMLIDImageIOFactory, ObjectFactoryBase);

  /** Register one factory of this type  */
  static void RegisterOneFactory()
  {
    DMMLIDImageIOFactory::Pointer nrrdFactory = DMMLIDImageIOFactory::New();
    ObjectFactoryBase::RegisterFactory(nrrdFactory);
  }

protected:
  DMMLIDImageIOFactory();
  ~DMMLIDImageIOFactory() override;

private:
  DMMLIDImageIOFactory(const Self&) = delete;
  void operator=(const Self&) = delete;

};


} /// end namespace itk

#endif
