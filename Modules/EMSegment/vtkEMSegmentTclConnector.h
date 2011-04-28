#ifndef __vtkEMSegmentTclConnector_h
#define __vtkEMSegmentTclConnector_h

#include "vtkEMSegment.h"
// needed to get the CMake variables
#include <vtkSlicerConfigure.h>

#ifdef Slicer3_USE_KWWIDGETS

// Slicer3 includes

#include  "vtkSlicerApplication.h"

#else

// Slicer4 includes

#include "qSlicerApplication.h"
// TODO

#endif

// common includes for Slicer3 and Slicer4
#include "vtkEMSegmentLogic.h"


// the Tcl connector accesses Tcl in Slicer3 and Slicer4
class VTK_EMSEGMENT_EXPORT vtkEMSegmentTclConnector :
  public vtkObject
{
public:
  static vtkEMSegmentTclConnector *New();
  vtkTypeMacro(vtkEMSegmentTclConnector,  vtkObject);

  virtual int SourceTclFile(const char *tclFile);







//BTX

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  vtkSetObjectMacro(SlicerApp, vtkSlicerApplication);

#else

  // Slicer4
  vtkSetObjectMacro(SlicerApp, qSlicerApplication);

#endif


  vtkSetObjectMacro(EMSLogic,  vtkEMSegmentLogic);
  vtkGetObjectMacro(EMSLogic,  vtkEMSegmentLogic);

//ETX

private:
  vtkEMSegmentTclConnector();
  ~vtkEMSegmentTclConnector();
  vtkEMSegmentTclConnector(const vtkEMSegmentTclConnector&);
  void operator=(const vtkEMSegmentTclConnector&);

#ifdef Slicer3_USE_KWWIDGETS
  // Slicer3
  vtkSlicerApplication* SlicerApp;
#else
  // Slicer4
  qSlicerApplication* SlicerApp;

#endif

  vtkEMSegmentLogic *EMSLogic;

};

#endif
