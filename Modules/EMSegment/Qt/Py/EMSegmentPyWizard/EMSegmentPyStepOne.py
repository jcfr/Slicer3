from __main__ import qt, ctk

from EMSegmentPyStep import *

class EMSegmentPyStepOne(EMSegmentPyStep) :
  
  def __init__(self, stepid):
    self.initialize(stepid)
    
    
  def createUserInterface(self):
    layout = qt.QVBoxLayout(self)
    label = qt.QLabel("EMPYSTEPONE: This is %s" % self.id())
    layout.addWidget(label)
    
    self.ButtonBoxHint(self.ButtonBoxHidden)  
  
  def onEntry(self, comingFrom, transitionType):
    comingFromId = "None"
    if comingFrom: comingFromId = comingFrom.id()
    print "EM1 -> onEntry - current [%s] - comingFrom [%s]" % (self.id(), comingFromId)
    self.ButtonBoxHint(self.ButtonBoxHidden)  
    print self.ButtonBoxHint
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
    
    
