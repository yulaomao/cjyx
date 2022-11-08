/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#ifndef itkDMMLIDImageIO_h
#define itkDMMLIDImageIO_h

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif

#include <vtkVersion.h>

#include "itkDMMLIDIOExport.h"

#include "itkImageIOBase.h"

class vtkDMMLVolumeNode;
class vtkDMMLDiffusionWeightedVolumeNode;
class vtkDMMLDiffusionImageVolumeNode;
class vtkImageData;
class vtkDMMLNode;

namespace itk
{
/** \class DMMLIDImageIO
 * \brief ImageIO object for reading and writing imaegs from a DMML scene
 *
 * DMMLIDImageIO is an ImageIO object that allows you to
 * retrieve/store an image in a DMML node using a standard ITK
 * ImageFileReader or ImageFileWriter.  THis allows a plugin to be
 * written once and compiled into a shared object module that Cjyx
 * can communicate with directly or compiled into a command line
 * program that can be executed outside of Cjyx.  In the former, the
 * plugin will be provided with a DMML ID for the "file" to read/write
 * and the ImageFileReader/Writer will use the DMMLIDImageIO object to
 * perform "IO" operations directly on a DMML scene.  In the latter,
 * the plugin will be provided with a filename for its inputs/outputs
 * and other ITK ImageIO objects will be employed by the
 * ImageFileReader/ImageFileWriter to read and write the data.
 *
 * The "filename" specified will look like a URI:
 *     <code>cjyx:\<scene id\>#\<node id\></code>                    - local cjyx
 *     <code>cjyx://\<hostname\>/\<scene id\>#\<node id\></code>     - remote cjyx
 *
 * This code was written on the Massachusettes Turnpike with extreme
 * glare on the LCD.
 */
class DMMLIDImageIO_EXPORT DMMLIDImageIO : public ImageIOBase
{
public:
  /** Standard class typedefs. */
  typedef DMMLIDImageIO       Self;
  typedef ImageIOBase         Superclass;
  typedef SmartPointer<Self>  Pointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(DMMLIDImageIO, ImageIOBase);

  /** Determine the file type. Returns true if this ImageIO can read the
   * file specified. */
  bool CanReadFile(const char*) override;

  virtual bool CanUseOwnBuffer();
  virtual void ReadUsingOwnBuffer();
  virtual void * GetOwnBuffer();

  /** Set the spacing and dimension information for the set filename. */
  void ReadImageInformation() override;

  /** Reads the data from disk into the memory buffer provided. */
  void Read(void* buffer) override;

  /*-------- This part of the interfaces deals with writing data. ----- */

  /** Determine the file type. Returns true if this ImageIO can read the
   * file specified. */
  bool CanWriteFile(const char*) override;

  /** Writes the header of the image.
   * Assumes SetFileName has been called with a valid file name. */
  void WriteImageInformation() override;

  /** Writes the data to disk from the memory buffer provided. Make sure
   * that the IORegion has been set properly. */
  void Write(const void* buffer) override;

protected:
  DMMLIDImageIO();
  ~DMMLIDImageIO() override;
  void PrintSelf(std::ostream& os, Indent indent) const override;

  /** Write the image information to the node and specified image */
  virtual void WriteImageInformation(vtkDMMLVolumeNode *, vtkImageData*,
                                     int *scalarType, int *numberOfScalarComponents);

  /** Take information in a Cjyx node and transfer it the
   *  MetaDataDictionary in ITK */
  void SetDWDictionaryValues(MetaDataDictionary &dict,
                             vtkDMMLDiffusionWeightedVolumeNode *dw);

  /** Take information in a Cjyx node and transfer it the
   *  MetaDataDictionary in ITK */
  void SetDTDictionaryValues(MetaDataDictionary &dict,
                             vtkDMMLDiffusionImageVolumeNode *di);

  /** Take information from the MetaDataDictionary that is needed to
   *  transfer this volume to a Cjyx node */
  void SetDWNodeValues(vtkDMMLDiffusionWeightedVolumeNode *dw,
                       MetaDataDictionary &dict);

  /** Take information from the MetaDataDictionary that is needed to
   *  transfer this volume to a Cjyx node */
  void SetDTNodeValues(vtkDMMLDiffusionImageVolumeNode *di,
                       MetaDataDictionary &dict);

  void RequestModified(vtkDMMLNode* modifiedObject);

private:
  DMMLIDImageIO(const Self&) = delete;
  void operator=(const Self&) = delete;

  bool IsAVolumeNode(const char*);
  vtkDMMLVolumeNode* FileNameToVolumeNodePtr(const char*);

  std::string m_Scheme;
  std::string m_Authority;
  std::string m_SceneID;
  std::string m_NodeID;

};


} /// end namespace itk
#endif /// itkDMMLIDImageIO_h
