#include "vtkObjectFactory.h"
#include "vtkEMSegmentTclConnector.h"


#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
#include "vtkKWTkUtilities.h"

#endif


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
}

//----------------------------------------------------------------------------
vtkEMSegmentTclConnector::~vtkEMSegmentTclConnector() {
}


//----------------------------------------------------------------------------
// New Task Specific Pipeline
//----------------------------------------------------------------------------

int vtkEMSegmentTclConnector::SourceTclFile(const char *tclFile)
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3

  // Load Tcl File defining the setting
  if (!vtkSlicerApplication::GetInstance()->LoadScript(tclFile))
    {
      vtkErrorMacro("Could not load in data for task. The following file does not exist: " << tclFile);
      return 1;
    }

#endif

  return 0;

}


//----------------------------------------------------------------------------
const char*  vtkEMSegmentTclConnector::GetTempDirectory()
{


#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  return vtkSlicerApplication::GetInstance()->GetTemporaryDirectory();

  //#else

  // TODO Slicer4

#endif

  vtkErrorMacro("Something went wrong.")
  return 0;

}

//-----------------------------------------------------------------------------
const char* vtkEMSegmentTclConnector::Script(const char* format)
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  return vtkSlicerApplication::GetInstance()->Script(format);
  //#else

  // TODO Slicer4

#endif

  vtkErrorMacro("Something went wrong.")
  return 0;
}

//-----------------------------------------------------------------------------
const char* vtkEMSegmentTclConnector::Script(const char* format, int row)
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  return vtkSlicerApplication::GetInstance()->Script(format,row);
  //#else

  // TODO Slicer4

#endif

  vtkErrorMacro("Something went wrong.")
  return 0;
}

//-----------------------------------------------------------------------------
const char* vtkEMSegmentTclConnector::Script(const char* format, int row, int column)
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  return vtkSlicerApplication::GetInstance()->Script(format,row,column);
  //#else

  // TODO Slicer4

#endif

  vtkErrorMacro("Something went wrong.")
  return 0;
}

//-----------------------------------------------------------------------------
const char* vtkEMSegmentTclConnector::GetTclNameFromPointer(vtkObject *obj)
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  return vtkKWTkUtilities::GetTclNameFromPointer(vtkSlicerApplication::GetInstance()->GetMainInterp(),obj);
  //#else

  // TODO Slicer4

#endif

  vtkErrorMacro("Something went wrong.")
  return 0;
}

