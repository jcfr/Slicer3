from __main__ import qt, ctk
import PythonQt

from EMSegmentStep import *

class EMSegmentDummyStep(EMSegmentStep) :
  
  def __init__(self, stepid):
    self.initialize(stepid)
    self.setName(self.id())
    self.setDescription('YYY')
    
  def createUserInterface(self):
    '''
    '''
    self.__parent = super(EMSegmentPyDummyStep, self)
    self.__layout = self.__parent.createUserInterface()
    
    
    label = qt.QLabel("PLACEHOLDER: " + self.id())
    self.__layout.addWidget(label)
    
