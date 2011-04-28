#include "vtkObjectFactory.h"
#include "vtkEMSegmentTclConnector.h"

//----------------------------------------------------------------------------
vtkEMSegmentTclConnector* vtkEMSegmentTclConnector::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret =
    vtkObjectFactory::CreateInstance("vtkEMSegmentTclConnector");
  if(ret)
    {
    return (vtkEMSegmentTclConnector*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkEMSegmentTclConnector;
}


//----------------------------------------------------------------------------
vtkEMSegmentTclConnector::vtkEMSegmentTclConnector() {
    this->SlicerApp =NULL;
    this->EMSLogic = NULL;
}

//----------------------------------------------------------------------------
vtkEMSegmentTclConnector::~vtkEMSegmentTclConnector() {
  this->SetSlicerApp(NULL);
  this->SetEMSLogic(NULL);
}


//----------------------------------------------------------------------------
// New Task Specific Pipeline
//----------------------------------------------------------------------------

int vtkEMSegmentTclConnector::SourceTclFile(const char *tclFile)
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3

  // Load Tcl File defining the setting
  if (!this->SlicerApp->LoadScript(tclFile))
    {
      vtkErrorMacro("Could not load in data for task. The following file does not exist: " << tclFile);
      return 1;
    }

#endif

  return 0;

}
