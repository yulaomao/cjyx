/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLRemoteIOLogic.cxx,v $
  Date:      $Date: 2011-04-06 17:26:19 -0400 (Wed, 06 Apr 2011) $
  Version:   $Revision: 16232 $

=========================================================================auto=*/

// DMML includes
#include <vtkCacheManager.h>
#include <vtkDataIOManager.h>
#include <vtkDMMLScene.h>
#include <vtkTagTable.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkObjectFactory.h>

// RemoteIO includes
#include <vtkHTTPHandler.h>

#include "vtkDMMLRemoteIOLogic.h"

vtkStandardNewMacro(vtkDMMLRemoteIOLogic);

vtkCxxSetObjectMacro(vtkDMMLRemoteIOLogic, CacheManager, vtkCacheManager);
vtkCxxSetObjectMacro(vtkDMMLRemoteIOLogic, DataIOManager, vtkDataIOManager);

//----------------------------------------------------------------------------
vtkDMMLRemoteIOLogic::vtkDMMLRemoteIOLogic()
{
  this->CacheManager = vtkCacheManager::New();
  this->DataIOManager = vtkDataIOManager::New();
  this->DataIOManager->SetCacheManager(this->CacheManager);
}

//----------------------------------------------------------------------------
vtkDMMLRemoteIOLogic::~vtkDMMLRemoteIOLogic()
{
  if (this->DataIOManager)
    {
    this->DataIOManager->SetCacheManager(nullptr);
    this->SetDataIOManager(nullptr);
    }
  if (this->CacheManager)
    {
    this->SetCacheManager(nullptr);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLRemoteIOLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

}

//-----------------------------------------------------------------------------
void vtkDMMLRemoteIOLogic::AddDataIOToScene()
{
  // TODO more of the cache and DataIOManager code
  // from qCjyxCoreApplication::setDMMLScene(vtkDMMLScene* newDMMLScene)
  // should be moved to here so they can be used outside of the
  // context of a qCjyx based application
  // Update 2015/03/20: split qCjyxCoreApplication::setDMMLScene so that
  // a user can call vtkCjyxApplicationLogic::SetDMMLSceneDataIO to trigger
  /// this method on a independent scene with separate remote io logic and data
  /// io manager logic
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("Cannot add DataIOHandlers -- scene not set");
    return;
    }

  // hook our cache and dataIO managers into the DMML scene
  this->CacheManager->SetDMMLScene(this->GetDMMLScene());
  this->GetDMMLScene()->SetCacheManager(this->CacheManager);
  this->GetDMMLScene()->SetDataIOManager(this->DataIOManager);

  vtkCollection *uriHandlerCollection = vtkCollection::New();
  // add some new handlers
  this->GetDMMLScene()->SetURIHandlerCollection( uriHandlerCollection );
  uriHandlerCollection->Delete();

#if !defined(REMOTEIO_DEBUG)
  // register all existing uri handlers (add to collection)
  vtkHTTPHandler *httpHandler = vtkHTTPHandler::New();
  httpHandler->SetPrefix ( "http://" );
  httpHandler->SetName ( "HTTPHandler");
  this->GetDMMLScene()->AddURIHandler(httpHandler);
  httpHandler->Delete();

  //add something to hold user tags
  vtkTagTable *userTagTable = vtkTagTable::New();
  this->GetDMMLScene()->SetUserTagTable( userTagTable );
  userTagTable->Delete();
#endif
}

void vtkDMMLRemoteIOLogic::RemoveDataIOFromScene()
{
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("Cannot remove DataIOHandlers -- scene not set");
    }
  this->GetDMMLScene()->SetURIHandlerCollection(nullptr);
  this->GetDMMLScene()->SetUserTagTable( nullptr );
}

