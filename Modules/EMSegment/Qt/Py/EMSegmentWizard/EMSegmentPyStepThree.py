from __main__ import qt, ctk
import PythonQt

from EMSegmentPyStep import *

class EMSegmentPyStepThree(EMSegmentPyStep) :
  
  def __init__(self, stepid):
    self.initialize(stepid)
    self.setName('3. Define Anatomical Tree')
    self.setDescription('Define a hierarchy of structures.')
    
    self.__parent = super(EMSegmentPyStepThree, self)    
    
  def createUserInterface(self):
    '''
    '''
    self.__layout = self.__parent.createUserInterface()
    
    # the anatomical tree
    anatomicalTreeGroupBox = qt.QGroupBox()
    anatomicalTreeGroupBox.setTitle('Anatomical Tree')
    self.__layout.addWidget(anatomicalTreeGroupBox)
    
    anatomicalTreeGroupBoxLayout = qt.QFormLayout(anatomicalTreeGroupBox)
    
    self.__anatomicalTree = PythonQt.qSlicerEMSegmentModuleWidgets.qSlicerEMSegmentAnatomicalTreeWidget()
    anatomicalTreeGroupBoxLayout.addWidget(self.__anatomicalTree)


  def onEntry(self, comingFrom, transitionType):
    '''
    '''
    self.__parent.onEntry(comingFrom, transitionType)
    
    self.__anatomicalTree.setMRMLManager(self.mrmlManager())    
    self.__anatomicalTree.updateWidgetFromMRML()    
    
    
