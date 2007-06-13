#ifndef __vtkEMSegmentGUI_h
#define __vtkEMSegmentGUI_h

#include "vtkSlicerModuleGUI.h"
#include "vtkEMSegment.h"

class vtkEMSegmentLogic;
class vtkEMSegmentMRMLManager;
class vtkMRMLEMSNode;
class vtkKWWizardWidget;
class vtkEMSegmentParametersSetStep;
class vtkEMSegmentIntensityImagesStep;
class vtkEMSegmentIntensityNormalizationStep;
class vtkEMSegmentAnatomicalStructureStep;
class vtkEMSegmentSpatialPriorsStep;
class vtkEMSegmentNodeParametersStep;
class vtkEMSegmentIntensityDistributionsStep;
class vtkEMSegmentRegistrationParametersStep;
class vtkEMSegmentRunSegmentationStep;

class VTK_EMSEGMENT_EXPORT vtkEMSegmentGUI : 
  public vtkSlicerModuleGUI
{
public:
  static vtkEMSegmentGUI *New();
  vtkTypeMacro(vtkEMSegmentGUI,vtkSlicerModuleGUI);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description: Get the categorization of the module.
  //   const char *GetCategory() const
  //     { return "Segmentation"; }

  // Description: 
  // Get/Set logic node
  vtkGetObjectMacro(Logic, vtkEMSegmentLogic);
  virtual void SetLogic(vtkEMSegmentLogic*);
  
  // Get/Set mrml manager node
  vtkGetObjectMacro(MRMLManager, vtkEMSegmentMRMLManager);
  virtual void SetMRMLManager(vtkEMSegmentMRMLManager*);

  // Description: 
  // Get/Set MRML node
  vtkGetObjectMacro(Node, vtkMRMLEMSNode);
  virtual void SetNode(vtkMRMLEMSNode*);

  // Description: 
  // Get wizard widget
  vtkGetObjectMacro(WizardWidget, vtkKWWizardWidget);
  vtkGetObjectMacro(AnatomicalStructureStep, 
                    vtkEMSegmentAnatomicalStructureStep);

  // Description:
  // Create widgets
  virtual void BuildGUI();

  // Description:
  // Delete Widgets
  virtual void TearDownGUI();

  // Description:
  // Add observers to GUI widgets
  virtual void AddGUIObservers();
  
  // Description:
  // Remove observers to GUI widgets
  virtual void RemoveGUIObservers();

  // Description:
  // Remove observers to MRML node
  virtual void RemoveMRMLNodeObservers();

  // Description:
  // Remove observers to Logic
  virtual void RemoveLogicObservers();
  
  // Description:
  // Pprocess events generated by Logic
  virtual void ProcessLogicEvents( vtkObject *caller, unsigned long event,
                                   void *callData);

  // Description:
  // Pprocess events generated by GUI widgets
  virtual void ProcessGUIEvents( vtkObject *caller, unsigned long event,
                                 void *callData);

  // Description:
  // Pprocess events generated by MRML
  virtual void ProcessMRMLEvents( vtkObject *caller, unsigned long event, 
                                  void *callData);
  // Description:
  // Describe behavior at module startup and exit.
  virtual void Enter(){};
  virtual void Exit(){};

  // Description: The name of the Module - this is used to 
  // construct the proc invocations
  vtkGetStringMacro(ModuleName);
  vtkSetStringMacro(ModuleName);

  // Description: set an observer by number (work around
  // limitation in kwwidgets tcl wrapping)
  unsigned long AddObserverByNumber(vtkObject *observee, unsigned long event);

protected:

private:
  vtkEMSegmentGUI();
  ~vtkEMSegmentGUI();
  vtkEMSegmentGUI(const vtkEMSegmentGUI&);
  void operator=(const vtkEMSegmentGUI&);

  // Description:
  // Updates GUI widgets based on parameters values in MRML node
  void UpdateGUI();

  // Description:
  // Updates parameters values in MRML node based on GUI widgets 
  void UpdateMRML();
  
  // Description:
  // Updates registration progress on the status bar of the main application. 
  virtual void UpdateRegistrationProgress();

  vtkEMSegmentLogic       *Logic;
  vtkEMSegmentMRMLManager *MRMLManager;
  vtkMRMLEMSNode          *Node;
  
  char *ModuleName;

  // Description:
  // The wizard widget and steps
  vtkKWWizardWidget                      *WizardWidget;
  vtkEMSegmentParametersSetStep          *ParametersSetStep;
  vtkEMSegmentAnatomicalStructureStep    *AnatomicalStructureStep;
  vtkEMSegmentSpatialPriorsStep          *SpatialPriorsStep;
  vtkEMSegmentIntensityImagesStep        *IntensityImagesStep;
  vtkEMSegmentIntensityNormalizationStep *NormalizationStep;
  vtkEMSegmentIntensityDistributionsStep *IntensityDistributionsStep;
  vtkEMSegmentNodeParametersStep         *NodeParametersStep;
  vtkEMSegmentRegistrationParametersStep *RegistrationParametersStep;
  vtkEMSegmentRunSegmentationStep        *RunSegmentationStep;

  // Description:
  // Populate the logic with testing data, load some volumes
  virtual void PopulateTestingData();
};

#endif
