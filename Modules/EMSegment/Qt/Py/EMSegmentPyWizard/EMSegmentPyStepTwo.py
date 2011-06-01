from __main__ import qt, ctk

from EMSegmentPyStep import *

class EMSegmentPyStepTwo(EMSegmentPyStep) :
  
  def __init__(self, stepid):
    self.initialize(stepid)
    
  def createUserInterface(self):
    layout = qt.QVBoxLayout(self)
    label = qt.QLabel("EMPYSTEPTWO: This is %s" % self.id())
    layout.addWidget(label)
    #uiloader = qt.QUiLoader()
    #file = qt.QFile("/Users/daniel/SLICER/QT_TRUNK/Slicer4/QTModules/EMSegment/Qt/Resources/UI/qSlicerEMSegmentDefineAnatomicalTreeStep.ui")
    #file.open(qt.QFile.ReadOnly)
    #steptwo_widget = uiloader.load(file)
    #file.close()
    #layout.addWidget(steptwo_widget)
    
  
  def onEntry(self, comingFrom, transitionType):
    comingFromId = "None"
    if comingFrom: comingFromId = comingFrom.id()
    print "-> onEntry - current [%s] - comingFrom [%s]" % (self.id(), comingFromId)
    super(EMSegmentPyStep, self).onEntry(comingFrom, transitionType)
    
  def onExit(self, goingTo, transitionType):
    goingToId = "None"
    if goingTo: goingToId = goingTo.id()
    print "-> onExit - current [%s] - goingTo [%s]" % (self.id(), goingToId)
    super(EMSegmentPyStep, self).onExit(goingTo, transitionType)
    
  def validate(self, desiredBranchId):
    validationSuceeded = True
    print "-> validate %s" % self.id()    
    super(EMSegmentPyStep, self).validate(validationSuceeded, desiredBranchId)
    
    
