from __main__ import vtk, qt, ctk, slicer

from EMSegmentPyWizard import *

class EMSegmentPy:
  def __init__(self, parent):
    parent.title = "EMSegmentPy"
    parent.category = ""
    parent.contributor = "--"
    parent.helpText = """dsfdsf"""
    parent.acknowledgementText = """sdfsdfdsf"""
    self.parent = parent

class EMSegmentPyWidget:
  def __init__(self, parent=None):
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)      
    else:
      self.parent = parent
    self.layout = self.parent.layout()

    # this flag is 1 if there is an update in progress
    self.__updating = 1
    
    # the pointer to the logic and the mrmlManager
    self.__mrmlManager = None
    self.__logic = None

    if not parent:
      self.setup()

      # after setup, be ready for events
      self.__updating = 0      
      
      self.parent.show()
      
    # register default slots
    #self.parent.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onMRMLSceneChanged)      
    

  def logic(self):
    if not self.__logic:
        self.__logic = slicer.modulelogic.vtkEMSegmentLogic()
    
    return self.__logic

  def mrmlManager(self):
    if not self.__mrmlManager:
        self.__mrmlManager = slicer.modulelogic.vtkEMSegmentLogic()
    
    return self.__mrmlManager

    
  def setup(self):
      self.workflow = ctk.ctkWorkflow()
      
    
      workflowWidget = ctk.ctkWorkflowStackedWidget()
      workflowWidget.setWorkflow(self.workflow)
    
      steps = []
    
      steps.append(EMSegmentPyStepOne('step1'))
      steps.append(EMSegmentPyStepTwo('step2'))
      steps.append(EMSegmentPyStep('step3'))
    
      # Add transition associated to steps
      for i in range(len(steps) - 1):
        self.workflow.addTransition(steps[i], steps[i + 1])
 
    
      # Propagate the logic and the MRML Manager to the steps
      for s in steps:
          
          s.setLogic(self.logic())
          s.setMRMLManager(self.mrmlManager())
    
      self.workflow.start()
    
      workflowWidget.visible = True
      self.layout.addWidget(workflowWidget)    
      
      # compress the layout
      self.layout.addStretch(1)        
      
      
