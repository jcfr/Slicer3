#ifndef __vtkEMSegmentTclConnector_h
#define __vtkEMSegmentTclConnector_h

#include "vtkEMSegment.h"
#include "vtkHTTPHandler.h"

// needed to get the CMake variables
#include <vtkSlicerConfigure.h>

#ifdef Slicer3_USE_KWWIDGETS

// Slicer3 includes
#include  "vtkSlicerApplication.h"
#include "vtkDataIOManagerLogic.h"
#include "vtkHTTPHandler.h"

#else

// Slicer4 includes
#include "qSlicerApplication.h"

#endif

// the Tcl connector accesses Tcl in Slicer3 and Slicer4
class VTK_EMSEGMENT_EXPORT vtkSlicerCommonInterface: public vtkObject
{
public:
  static vtkSlicerCommonInterface *New();
  vtkTypeMacro(vtkSlicerCommonInterface, vtkObject);

//BTX
  static int SourceTclFile(const char *tclFile);
  static const char* EvaluateTcl(const char* command);
  static const char* GetTclNameFromPointer(vtkObject *obj);

  static const char* GetTemporaryDirectory();
  static const char* GetRepositoryRevision();
  static vtkHTTPHandler* GetHTTPHandler(vtkMRMLScene* mrmlScene);

  static void AddDataIOToScene(vtkMRMLScene* mrmlScene, vtkSlicerApplicationLogic *appLogic, vtkDataIOManagerLogic *dataIOManagerLogic);
  static void RemoveDataIOFromScene(vtkMRMLScene* mrmlScene, vtkDataIOManagerLogic *dataIOManagerLogic);
//ETX

private:
  vtkSlicerCommonInterface();
  ~vtkSlicerCommonInterface();
  vtkSlicerCommonInterface(const vtkSlicerCommonInterface&);
  void operator=(const vtkSlicerCommonInterface&);

};

#endif
