from __main__ import qt, ctk
import PythonQt

from EMSegmentPyStep import *

class EMSegmentPyStepTwo(EMSegmentPyStep) :
  
  def __init__(self, stepid):
    self.initialize(stepid)
    self.setName('2. Define Input Channels')
    self.setDescription('Name the input channels and choose the set of scans for segmentation.')
    
    self.__parent = super(EMSegmentPyStepTwo, self)    
    
  def createUserInterface(self):
    '''
    '''
    self.__layout = self.__parent.createUserInterface()
    
    # the input channels
    inputChannelGroupBox = qt.QGroupBox()
    inputChannelGroupBox.setTitle('Input Channels')
    self.__layout.addWidget(inputChannelGroupBox)
    
    inputChannelGroupBoxLayout = qt.QFormLayout(inputChannelGroupBox)
    
    self.__inputChannelList = PythonQt.qSlicerEMSegmentModuleWidgets.qSlicerEMSegmentInputChannelListWidget()
    inputChannelGroupBoxLayout.addWidget(self.__inputChannelList)
    
    # add empty row
    self.__layout.addRow("", qt.QWidget())        
    
    # registration settings
    input2inputChannelRegistration = qt.QGroupBox()
    input2inputChannelRegistration.setTitle('Input-to-Input Channel Registration')
    self.__layout.addWidget(input2inputChannelRegistration)

    input2inputChannelRegistrationLayout = qt.QFormLayout(input2inputChannelRegistration)
    
    alignInputScansCheckBox = qt.QCheckBox()
    input2inputChannelRegistrationLayout.addRow('Align input scans:',alignInputScansCheckBox)
    
  def onEntry(self, comingFrom, transitionType):
    '''
    '''
    self.__parent.onEntry(comingFrom, transitionType)
    
    self.__inputChannelList.setMRMLManager(self.mrmlManager()) 
    self.__inputChannelList.updateWidgetFromMRML()
    
    if self.__inputChannelList.inputChannelCount() == 0:
      self.__inputChannelList.addInputChannel()
    
    
