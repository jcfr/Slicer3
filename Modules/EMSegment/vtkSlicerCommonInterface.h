#ifndef __vtkEMSegmentTclConnector_h
#define __vtkEMSegmentTclConnector_h

#include "vtkEMSegment.h"
#include "vtkHTTPHandler.h"

// needed to get the CMake variables
#include <vtkSlicerConfigure.h>

#ifdef Slicer3_USE_KWWIDGETS

// Slicer3 includes
#include  "vtkSlicerApplication.h"

  // Slicer3 python
  #ifdef Slicer3_USE_PYTHON

  #include "slicerPython.h"

  extern "C" {
    void init_mytkinter( Tcl_Interp* );
    void init_slicer(void );
  }

  #include "vtkTclUtil.h"

  #endif // Check for Slicer3 Python

#else

// Slicer4 includes
#include "qSlicerApplication.h"
#include "qSlicerPythonManager.h"
#include <QString>
#include <QVariant>

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
  Tcl_Interp* Startup(int argc, char *argv[], ostream *err = 0);
  int SourceTclFile(const char *tclFile);
  const char* EvaluateTcl(const char* command);
  const char* GetTclNameFromPointer(vtkObject *obj);
  void RegisterObjectWithTcl(vtkObject *obj, const char* name);
  const char* GetApplicationTclName();

  void InitializePythonViaTcl(Tcl_Interp* interp, int argc, char **argv);
  void EvaluatePython(const char* command);

  const char* GetTemporaryDirectory();
  const char* GetBinDirectory();
  const char* GetRepositoryRevision();
  void SetApplicationBinDir(const char*);
  vtkHTTPHandler* GetHTTPHandler(vtkMRMLScene* mrmlScene);
  void PromptBeforeExitOff();
  void DestroySlicerApplication();

  void AddDataIOToScene(vtkMRMLScene* mrmlScene, vtkSlicerApplicationLogic *appLogic, vtkDataIOManagerLogic *dataIOManagerLogic);
  void RemoveDataIOFromScene(vtkMRMLScene* mrmlScene, vtkDataIOManagerLogic *dataIOManagerLogic);

  std::string randomStrGen(int length);
//ETX

private:
  vtkSlicerCommonInterface();
  ~vtkSlicerCommonInterface();
  vtkSlicerCommonInterface(const vtkSlicerCommonInterface&);
  void operator=(const vtkSlicerCommonInterface&);

  //BTX
  std::string StringHolder;
  //ETX

};

#endif
