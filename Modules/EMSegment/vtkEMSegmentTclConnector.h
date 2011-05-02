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

#endif

// the Tcl connector accesses Tcl in Slicer3 and Slicer4
class VTK_EMSEGMENT_EXPORT vtkEMSegmentTclConnector: public vtkObject
{
public:
  static vtkEMSegmentTclConnector *New();
  vtkTypeMacro(vtkEMSegmentTclConnector, vtkObject);

//BTX
  int SourceTclFile(const char *tclFile);

  const char* GetTempDirectory();

  const char* Script(const char* format);
  const char* Script(const char* format, int row);
  const char* Script(const char* format, int row, int column);

  const char* GetTclNameFromPointer(vtkObject *obj);

//ETX
private:
  vtkEMSegmentTclConnector();
  ~vtkEMSegmentTclConnector();
  vtkEMSegmentTclConnector(const vtkEMSegmentTclConnector&);
  void operator=(const vtkEMSegmentTclConnector&);

};

#endif
