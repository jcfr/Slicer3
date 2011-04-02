#ifndef __vtkEMSegmentDynamicFrame_h
#define __vtkEMSegmentDynamicFrame_h

#include "vtkEMSegment.h"
#include <vtksys/stl/vector>
#include "vtkKWFrameWithLabel.h"
#include "vtkEMSegmentMRMLManager.h"

class vtkKWCheckButtonWithLabel;
class vtkKWFrameWithLabel;
class vtkKWLabelWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWEntryWithLabel;
class vtkKWWidget ;
class vtkSlicerApplication;


// should be a vtkKW widget but for simplicity did it this way 
class VTK_EMSEGMENT_EXPORT vtkEMSegmentDynamicFrame  : public vtkKWFrameWithLabel
{
public:
  static vtkEMSegmentDynamicFrame *New();
  vtkTypeRevisionMacro(vtkEMSegmentDynamicFrame,vtkKWFrameWithLabel);

  void DefineCheckButton(const char *label, int initState, vtkIdType ID);
  int GetCheckButtonValue(vtkIdType ID); 

  void DefineTextLabel(const char *label, vtkIdType ID);

  void DefineVolumeMenuButton(const char *label, vtkIdType initVolID, vtkIdType buttonID);
  vtkIdType GetVolumeMenuButtonValue(vtkIdType ID); 
  void  VolumeMenuButtonCallback(vtkIdType buttonID, vtkIdType volID);

  void DefineTextEntry(const char *label, const char *initText, vtkIdType entryID);
  const char* GetTextEntryValue(vtkIdType ID); 

  void SetButtonsFromMRML();
 
  // Is only in here bc it is called from tcl files 
  void PopUpWarningWindow(const char *);

  // Description:
  vtkSetObjectMacro(MRMLManager, vtkEMSegmentMRMLManager);

  // void Create( vtkKWWidget *parent, vtkSlicerApplication* initSlicerApp, vtkEMSegmentMRMLManager *initMRMLManager);

  void SaveSettingToMRML();

  void CreateEntryLists();

protected:
  vtkEMSegmentDynamicFrame();
  ~vtkEMSegmentDynamicFrame();

private:
  vtkEMSegmentDynamicFrame(const vtkEMSegmentDynamicFrame&);
  void operator=(const vtkEMSegmentDynamicFrame&);

 //BTX
  std::vector<vtkKWMenuButtonWithLabel*> volumeMenuButton;
  std::vector<vtkIdType> volumeMenuButtonID;
  std::vector<vtkKWCheckButtonWithLabel*> checkButton;
  std::vector<vtkKWLabelWithLabel*> textLabel;
  std::vector<vtkKWEntryWithLabel*> textEntry;
  vtkSlicerApplication* slicerApp; 
  vtkEMSegmentMRMLManager *MRMLManager;
  //ETX
};

#endif
