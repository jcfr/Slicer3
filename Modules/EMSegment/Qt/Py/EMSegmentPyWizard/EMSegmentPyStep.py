from __main__ import qt, ctk

class EMSegmentPyStep(ctk.ctkWorkflowWidgetStep) :
  
  def __init__(self, stepid):
    self.initialize(stepid)
    
    self.__mrmlManager = None
    self.__logic = None
    
  def setMRMLManager(self,mrmlManager):
    self.__mrmlManager = mrmlManager
    
  def setLogic(self,logic):
    self.__logic = logic
    
  def mrmlManager(self):
    return self.__mrmlManager

  def logic(self):
    return self.__logic
    
  def createUserInterface(self):
    layout = qt.QVBoxLayout(self)
    label = qt.QLabel("This is %s" % self.id())
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
    
    
