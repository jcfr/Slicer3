#ifndef __vtkEMSegmentTclConnector_h
#define __vtkEMSegmentTclConnector_h

#include "vtkEMSegment.h"
#include "vtkHTTPHandler.h"

// needed to get the CMake variables
#include <vtkSlicerConfigure.h>

#ifdef Slicer3_USE_KWWIDGETS

// Slicer3 includes
#include  "vtkSlicerApplication.h"

#else

// Slicer4 includes
#include "qSlicerApplication.h"
  typedef void Tcl_Interp;
#endif

#include "vtkDataIOManagerLogic.h"
#include "vtkHTTPHandler.h"

// the Tcl connector accesses Tcl in Slicer3 and Slicer4
class VTK_EMSEGMENT_EXPORT vtkSlicerCommonInterface: public vtkObject
{
public:
  static vtkSlicerCommonInterface *New();
  vtkTypeMacro(vtkSlicerCommonInterface, vtkObject);

//BTX
  Tcl_Interp* GetTclInterpeter(int argc, char *argv[], ostream *err = 0);

  int SourceTclFile(const char *tclFile);
  const char* EvaluateTcl(const char* command);
  const char* GetTclNameFromPointer(vtkObject *obj);
  const char* GetApplicationTclName();

  const char* GetTemporaryDirectory();
  const char* GetBinDirectory();
  const char* GetRepositoryRevision();
  void SetApplicationBinDir(const char*);
  vtkHTTPHandler* GetHTTPHandler(vtkMRMLScene* mrmlScene);
  void PromptBeforeExitOff();
  void DestroySlicerApplication();

  void AddDataIOToScene(vtkMRMLScene* mrmlScene, vtkSlicerApplicationLogic *appLogic, vtkDataIOManagerLogic *dataIOManagerLogic);
  void RemoveDataIOFromScene(vtkMRMLScene* mrmlScene, vtkDataIOManagerLogic *dataIOManagerLogic);
//ETX

private:
  vtkSlicerCommonInterface();
  ~vtkSlicerCommonInterface();
  vtkSlicerCommonInterface(const vtkSlicerCommonInterface&);
  void operator=(const vtkSlicerCommonInterface&);

};

#endif
