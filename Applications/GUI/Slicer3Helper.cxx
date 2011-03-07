#include "Slicer3Helper.h"


//-----------------------------------------------------------------------------
void Slicer3Helper::AddDataIOToScene(vtkMRMLScene* mrmlScene, vtkSlicerApplication *app, vtkSlicerApplicationLogic *appLogic, vtkDataIOManagerLogic *dataIOManagerLogic)
{
  if (!app || !appLogic)
    {
      // todo vtkWarningMacro("Parameter of DataIO are not set according to app or appLogic bc one of them is NULL - this might cause issues when downloading data form the web!");
    }
  // Create Remote I/O and Cache handling mechanisms
  // and configure them using Application registry values
  {
    vtkCacheManager *cacheManager = vtkCacheManager::New();
    
    if (app) 
      {
        cacheManager->SetRemoteCacheLimit ( app->GetRemoteCacheLimit() );
        cacheManager->SetRemoteCacheFreeBufferSize ( app->GetRemoteCacheFreeBufferSize() );
        cacheManager->SetEnableForceRedownload ( app->GetEnableForceRedownload() );
        cacheManager->SetRemoteCacheDirectory( app->GetRemoteCacheDirectory() );
      }
    cacheManager->SetMRMLScene ( mrmlScene );
    mrmlScene->SetCacheManager( cacheManager );
    cacheManager->Delete();
  }

  //cacheManager->SetEnableRemoteCacheOverwriting ( app->GetEnableRemoteCacheOverwriting() );
  //--- MRML collection of data transfers with access to cache manager
  {
    vtkDataIOManager *dataIOManager = vtkDataIOManager::New();
    dataIOManager->SetCacheManager ( mrmlScene->GetCacheManager() );
    if (app)
      {
        dataIOManager->SetEnableAsynchronousIO ( app->GetEnableAsynchronousIO () );
      }
    mrmlScene->SetDataIOManager ( dataIOManager );
    dataIOManager->Delete();
  }

  //--- Data transfer logic
  {
    // vtkDataIOManagerLogic *dataIOManagerLogic = vtkDataIOManagerLogic::New();
    dataIOManagerLogic->SetMRMLScene ( mrmlScene );
    if (appLogic)
      {
        dataIOManagerLogic->SetApplicationLogic ( appLogic );
      }
    dataIOManagerLogic->SetAndObserveDataIOManager ( mrmlScene->GetDataIOManager() );
  }

  {
    vtkCollection *URIHandlerCollection = vtkCollection::New();
    // add some new handlers
    mrmlScene->SetURIHandlerCollection( URIHandlerCollection );
    URIHandlerCollection->Delete();   
  }

#if !defined(REMOTEIO_DEBUG)
  // register all existing uri handlers (add to collection)
  vtkHTTPHandler *httpHandler = vtkHTTPHandler::New();
  httpHandler->SetPrefix ( "http://" );
  httpHandler->SetName ( "HTTPHandler");
  mrmlScene->AddURIHandler(httpHandler);
  httpHandler->Delete();

  vtkSRBHandler *srbHandler = vtkSRBHandler::New();
  srbHandler->SetPrefix ( "srb://" );
  srbHandler->SetName ( "SRBHandler" );
  mrmlScene->AddURIHandler(srbHandler);
  srbHandler->Delete();

  vtkXNATHandler *xnatHandler = vtkXNATHandler::New();
  vtkSlicerXNATPermissionPrompterWidget *xnatPermissionPrompter = vtkSlicerXNATPermissionPrompterWidget::New();
  if (app)
    {
      xnatPermissionPrompter->SetApplication ( app );
    }
  xnatPermissionPrompter->SetPromptTitle ("Permission Prompt");
  xnatHandler->SetPrefix ( "xnat://" );
  xnatHandler->SetName ( "XNATHandler" );
  xnatHandler->SetRequiresPermission (1);
  xnatHandler->SetPermissionPrompter ( xnatPermissionPrompter );
  mrmlScene->AddURIHandler(xnatHandler);
  xnatPermissionPrompter->Delete();
  xnatHandler->Delete();

  vtkHIDHandler *hidHandler = vtkHIDHandler::New();
  hidHandler->SetPrefix ( "hid://" );
  hidHandler->SetName ( "HIDHandler" );
  mrmlScene->AddURIHandler( hidHandler);
  hidHandler->Delete();

  vtkXNDHandler *xndHandler = vtkXNDHandler::New();
  xndHandler->SetPrefix ( "xnd://" );
  xndHandler->SetName ( "XNDHandler" );
  mrmlScene->AddURIHandler( xndHandler);
  xndHandler->Delete();

  //add something to hold user tags
  vtkTagTable *userTagTable = vtkTagTable::New();
  mrmlScene->SetUserTagTable( userTagTable );
  userTagTable->Delete();
#endif
}

void Slicer3Helper::RemoveDataIOFromScene(vtkMRMLScene* mrmlScene, vtkDataIOManagerLogic *dataIOManagerLogic)
{
  if ( dataIOManagerLogic != NULL )
    {
      dataIOManagerLogic->SetAndObserveDataIOManager ( NULL );
      dataIOManagerLogic->SetMRMLScene ( NULL );
    }

  if (mrmlScene->GetDataIOManager())
    {
      mrmlScene->GetDataIOManager()->SetCacheManager(NULL);
      mrmlScene->SetDataIOManager(NULL);
    }

  if ( mrmlScene->GetCacheManager())
    {
      mrmlScene->GetCacheManager()->SetMRMLScene ( NULL );
      mrmlScene->SetCacheManager(NULL);
    }

  mrmlScene->SetURIHandlerCollection(NULL);
  mrmlScene->SetUserTagTable( NULL );
}

