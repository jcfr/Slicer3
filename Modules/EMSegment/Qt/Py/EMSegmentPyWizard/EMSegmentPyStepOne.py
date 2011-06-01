from __main__ import qt, ctk

from EMSegmentPyStep import *

class EMSegmentPyStepOne(EMSegmentPyStep) :
  
  def __init__(self, stepid):
    self.initialize(stepid)
    
  def createUserInterface(self):
    layout = qt.QVBoxLayout(self)
    label = qt.QLabel("EMPYSTEPONE: This is %s" % self.id())
    layout.addWidget(label)
    
  
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
    
    
