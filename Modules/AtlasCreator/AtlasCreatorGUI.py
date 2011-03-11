from SlicerScriptedModule import ScriptedModuleGUI
from Slicer import slicer
from operator import itemgetter, attrgetter
import glob
import os

from AtlasCreatorHelper import AtlasCreatorHelper
from AtlasCreatorLogic import AtlasCreatorLogic
from AtlasCreatorConfiguration import AtlasCreatorConfiguration
from AtlasCreatorSkipRegistrationConfiguration import AtlasCreatorSkipRegistrationConfiguration
from AtlasCreatorGridConfiguration import AtlasCreatorGridConfiguration

vtkKWPushButton_InvokedEvent = 10000

vtkKWFileBrowserDialog_FileNameChangedEvent = 15000

vtkKWRadioButton_SelectedStateChangedEvent = 10000

vtkKWCheckButton_SelectedStateChangedEvent = 10000

vtkKWEntry_EntryValueChangedEvent = 10000

vtkMRMLScene_NodeAddedEvent = 66000

vtkMRMLScene_CloseEvent = 66003

vtkMRMLAtlasCreatorNode_LaunchComputationEvent = 31337

vtkSlicerNodeSelectorWidget_NodeSelectedEvent = 11000

class AtlasCreatorGUI(ScriptedModuleGUI):

    def __init__(self):

        ScriptedModuleGUI.__init__(self)

        self.SetCategory("Registration")
        self.SetGUIName("AtlasCreator")

        self._moduleFrame = slicer.vtkSlicerModuleCollapsibleFrame()
        self._topFrame = slicer.vtkSlicerModuleCollapsibleFrame()

        self._origDirButton = slicer.vtkKWLoadSaveButtonWithLabel()
        self._segDirButton = slicer.vtkKWLoadSaveButtonWithLabel()
        self._outDirButton = slicer.vtkKWLoadSaveButtonWithLabel()
        
        self._transformsFrame = slicer.vtkKWFrameWithLabel()
        self._transformsDirCheckBox = slicer.vtkKWCheckButtonWithLabel()
        self._transformsDirButton = slicer.vtkKWLoadSaveButtonWithLabel()
        self._transformsTemplateButton = slicer.vtkKWLoadSaveButtonWithLabel()

        self._secondFrame = slicer.vtkSlicerModuleCollapsibleFrame()

        self._defaultCaseFrame = slicer.vtkKWFrameWithLabel()

        self._dynamicRadio = slicer.vtkKWRadioButton()

        self._meanSpinBox = slicer.vtkKWSpinBoxWithLabel()

        self._fixedRadio = slicer.vtkKWRadioButton()

        self._defaultCaseEntry = slicer.vtkKWComboBoxWithLabel()

        self._regTypeRadios = slicer.vtkKWRadioButtonSetWithLabel()

        self._saveTransformsCheckBox = slicer.vtkKWCheckButtonWithLabel()

        self._deleteAlignedImagesCheckBox = slicer.vtkKWCheckButtonWithLabel()

        self._deleteAlignedSegmentationsCheckBox = slicer.vtkKWCheckButtonWithLabel()

        self._thirdFrame = slicer.vtkSlicerModuleCollapsibleFrame()

        self._labelsEntry = slicer.vtkKWEntryWithLabel()

        self._normalizeAtlasCheckBox = slicer.vtkKWCheckButtonWithLabel()

        self._outputCastCombo = slicer.vtkKWComboBoxWithLabel()
        
        self._fourthFrame = slicer.vtkSlicerModuleCollapsibleFrame()

        self._clusterCheckBox = slicer.vtkKWCheckButtonWithLabel()
        self._schedulerEntry = slicer.vtkKWEntryWithLabel()
        self._waitEntry = slicer.vtkKWEntryWithLabel()

        self._fifthFrame = slicer.vtkSlicerModuleCollapsibleFrame()
        
        self._normalizeValueEntry = slicer.vtkKWEntryWithLabel()

        self._generateButton = slicer.vtkKWPushButton()

        self._helper = AtlasCreatorHelper(self)

        self._logic = AtlasCreatorLogic(self)

        self._associatedMRMLNode = None

        self._updating = 0
        
        


    def Destructor(self):



        self._helper = None
        self._logic = None

    def RemoveMRMLNodeObservers(self):
        pass
    
    def RemoveLogicObservers(self):
        pass

    def AddGUIObservers(self):
        self._generateButtonTag = self.AddObserverByNumber(self._generateButton,vtkKWPushButton_InvokedEvent)
        self._origDirButtonTag = self.AddObserverByNumber(self._origDirButton.GetWidget().GetLoadSaveDialog(),vtkKWFileBrowserDialog_FileNameChangedEvent)
        self._segDirButtonTag = self.AddObserverByNumber(self._segDirButton.GetWidget().GetLoadSaveDialog(),vtkKWFileBrowserDialog_FileNameChangedEvent)
        self._fixedRadioTag = self.AddObserverByNumber(self._fixedRadio,vtkKWRadioButton_SelectedStateChangedEvent)
        self._dynamicRadioTag = self.AddObserverByNumber(self._dynamicRadio,vtkKWRadioButton_SelectedStateChangedEvent)
        self._normalizeAtlasCheckBoxTag = self.AddObserverByNumber(self._normalizeAtlasCheckBox.GetWidget(),vtkKWCheckButton_SelectedStateChangedEvent)
        self._defaultCaseEntryTag = self.AddObserverByNumber(self._defaultCaseEntry.GetWidget(),vtkKWEntry_EntryValueChangedEvent)
        self._transformsTemplateButtonTag = self.AddObserverByNumber(self._transformsTemplateButton.GetWidget().GetLoadSaveDialog(),vtkKWFileBrowserDialog_FileNameChangedEvent)
        self._mrmlNodeAddedTag = self.AddMRMLObserverByNumber(slicer.MRMLScene,vtkMRMLScene_NodeAddedEvent)


    def RemoveGUIObservers(self):
        pass

    def ProcessGUIEvents(self,caller,event):
        if not self._updating:

            if caller == self._generateButton and event == vtkKWPushButton_InvokedEvent:
                self.GenerateAtlas()
            elif caller == self._origDirButton.GetWidget().GetLoadSaveDialog() and event == vtkKWFileBrowserDialog_FileNameChangedEvent:
                self.UpdateCaseCombobox()
            elif caller == self._segDirButton.GetWidget().GetLoadSaveDialog() and event == vtkKWFileBrowserDialog_FileNameChangedEvent:
                self.UpdateCaseCombobox()
            elif caller == self._fixedRadio and event == vtkKWRadioButton_SelectedStateChangedEvent:
                self.ToggleMeanAndDefaultCase1()
            elif caller == self._dynamicRadio and event == vtkKWRadioButton_SelectedStateChangedEvent:
                self.ToggleMeanAndDefaultCase2()
            elif caller == self._normalizeAtlasCheckBox.GetWidget() and event == vtkKWCheckButton_SelectedStateChangedEvent:
                self.ToggleNormalize()
            elif caller == self._defaultCaseEntry.GetWidget() and event == vtkKWEntry_EntryValueChangedEvent:
                self.GetHelper().debug("defaultCaseEntry changed")
                self.UpdateLabelList()
            elif caller == self._transformsTemplateButton.GetWidget().GetLoadSaveDialog() and event == vtkKWFileBrowserDialog_FileNameChangedEvent:
                self.UpdateLabelList()
            
                
                
    def ToggleNormalize(self):
        normalize = self._normalizeAtlasCheckBox.GetWidget().GetSelectedState()
        
        if normalize:
            self._outputCastCombo.SetEnabled(0)
            self._outputCastCombo.GetWidget().SetValue("Double")
        else:
            self._outputCastCombo.SetEnabled(1)
                
    def ToggleMeanAndDefaultCase1(self):
        useDefCase = self._fixedRadio.GetSelectedState()
        useMean = self._dynamicRadio.GetSelectedState()

        if useDefCase:
            self._dynamicRadio.SetSelectedState(0)
            #self._fixedRadio.SetSelectedState(1)
            self._defaultCaseEntry.SetEnabled(1)
            self._meanSpinBox.SetEnabled(0)
        else:
            self._dynamicRadio.SetSelectedState(1)
            #self._fixedRadio.SetSelectedState(0)
            self._defaultCaseEntry.SetEnabled(0)
            self._meanSpinBox.SetEnabled(1)

    def ToggleMeanAndDefaultCase2(self):
        useMean = self._dynamicRadio.GetSelectedState()

        if useMean:
            #self._dynamicRadio.SetSelectedState(1)
            self._fixedRadio.SetSelectedState(0)
            self._defaultCaseEntry.SetEnabled(0)
            self._meanSpinBox.SetEnabled(1)
        else:
            #self._dynamicRadio.SetSelectedState(0)
            self._fixedRadio.SetSelectedState(1)
            self._defaultCaseEntry.SetEnabled(1)
            self._meanSpinBox.SetEnabled(0)

    def UpdateCaseCombobox(self):
        if self._origDirButton.GetWidget().GetFileName() and self._segDirButton.GetWidget().GetFileName():
            # originals and segmentations dir were configured, now we parse for potential image data
            nrrdFiles = glob.glob(os.path.join(self._origDirButton.GetWidget().GetFileName(), '*.nrrd'))
            nhdrFiles = glob.glob(os.path.join(self._origDirButton.GetWidget().GetFileName(), '*.nhdr'))
            hdrFiles = glob.glob(os.path.join(self._origDirButton.GetWidget().GetFileName(), '*.hdr'))          
            mhdFiles = glob.glob(os.path.join(self._origDirButton.GetWidget().GetFileName(), '*.mhd'))          
            mhaFiles = glob.glob(os.path.join(self._origDirButton.GetWidget().GetFileName(), '*.mha'))
        
            listOfFiles = [
                          ('.nrrd',len(nrrdFiles)),
                          ('.nhdr',len(nhdrFiles)),
                          ('.hdr',len(hdrFiles)),
                          ('.mhd',len(mhdFiles)),
                          ('.mha',len(mhaFiles)),                               
                          ]

            self._helper.debug("Found "+str(sorted(listOfFiles, key=itemgetter(1), reverse=True)[0][1])+" files of type "+str(sorted(listOfFiles, key=itemgetter(1), reverse=True)[0][0]))

            self._updating = 1
            for nrrdFile in nrrdFiles:
                caseFile = os.path.basename(nrrdFile)
                if os.path.isfile(os.path.join(self._segDirButton.GetWidget().GetFileName(),caseFile)):
                    # file exists in originals and segmentations directory, so we can add it to our list of cases
                    self._defaultCaseEntry.GetWidget().AddValue(caseFile)
                    self._defaultCaseEntry.GetWidget().SetValue(caseFile)
                    
            self._updating = 0
            
            # get the labels from last selected defaultCaseEntry value
            self.UpdateLabelList()
                    
                    
    def UpdateLabelList(self):
        if not self._updating:
            if self._transformsDirCheckBox.GetWidget().GetSelectedState():
                # use existing transforms mode
                listOfSegmentations = self.GetHelper().ConvertDirectoryToList(self._segDirButton.GetWidget().GetFileName())
                
                if len(listOfSegmentations) > 1:
                    defaultCaseSegmentationFilePath = listOfSegmentations[0]
            else:
                caseFile = self._defaultCaseEntry.GetWidget().GetValue()
                defaultCaseSegmentationFilePath = self._segDirButton.GetWidget().GetFileName() + os.sep + caseFile
                
            labelList = self.ReadLabelsFromImage(defaultCaseSegmentationFilePath)
            
            labelListAsString = ""
            for label in labelList:
                labelListAsString += str(label) + " "
                
            self._labelsEntry.GetWidget().SetValue(labelListAsString)
                               

    def GenerateAtlas(self):
        ''' call the logic '''

        # sanity checks
        if not self._origDirButton.GetWidget().GetFileName() and not self._transformsDirCheckBox.GetWidget().GetSelectedState():
            # we only need the original images if we do not skip the registration
            self.GetHelper().info("ERROR! The Original Images Directory was not set.. Aborting..")
            return False
        
        if not self._segDirButton.GetWidget().GetFileName():
            self.GetHelper().info("ERROR! The Segmentation Directory was not set.. Aborting..")
            return False                
            
        if not self._outDirButton.GetWidget().GetFileName():
            self.GetHelper().info("ERROR! The Output Directory was not set.. Aborting..")
            return False            

        # create a configuration container
        configuration = AtlasCreatorConfiguration()

        # if we have a special scenario, create a specific container
        if self._transformsDirCheckBox.GetWidget().GetSelectedState():
            # use the existing deformation field mode and skip the registration
            configuration = AtlasCreatorSkipRegistrationConfiguration()
            
            # now set the directory to the transforms
            configuration.SetTransformDirectory(os.path.normpath(self._transformsDirButton.GetWidget().GetFileName())+os.sep)
        
            if not self._transformsTemplateButton.GetWidget().GetFileName():
                # we need an existing template here
                self.GetHelper().info("ERROR! There was no template set.. Aborting..")
                return False
            
            configuration.SetExistingTemplate(self._transformsTemplateButton.GetWidget().GetFileName())
        
        elif self._clusterCheckBox.GetWidget().GetSelectedState():
            # cluster mode
            configuration = AtlasCreatorGridConfiguration()
            
            #now set the scheduler command
            schedCommand = self._schedulerEntry.GetWidget().GetValue()
            configuration.SetSchedulerCommand(schedCommand)
        
        # set the list of original images    
        configuration.SetOriginalImagesFilePathList(self.GetHelper().ConvertDirectoryToList(self._origDirButton.GetWidget().GetFileName()))
        
        # set the list of segmentations
        configuration.SetSegmentationsFilePathList(self.GetHelper().ConvertDirectoryToList(self._segDirButton.GetWidget().GetFileName()))
        
        # set the output directory        
        configuration.SetOutputDirectory(os.path.normpath(self._outDirButton.GetWidget().GetFileName())+os.sep)
        
        # set the template type
        if self._fixedRadio.GetSelectedState():
            configuration.SetTemplateType("fixed")
        else:
            configuration.SetTemplateType("dynamic")
            
        # set the mean iterations
        configuration.SetDynamicTemplateIterations(self._meanSpinBox.GetWidget().GetValue())    
        
        # set the default case
        defCaseFileName = self._defaultCaseEntry.GetWidget().GetValue()
        if not self._transformsDirCheckBox.GetWidget().GetSelectedState():     
            defCaseFilePath = os.path.join(self._origDirButton.GetWidget().GetFileName(),defCaseFileName)
            # ... normalize it and set it
            configuration.SetFixedTemplateDefaultCaseFilePath(os.path.normpath(defCaseFilePath))
            
        labels = self._labelsEntry.GetWidget().GetValue()
        configuration.SetLabelsList(labels)
            
        # set the registration type
        if self._regTypeRadios.GetWidget().GetWidget(0).GetSelectedState():
            # this means, affine registration is selected
            configuration.SetRegistrationType("Affine")
        else:
            # this means, non-rigid registration is selected
            configuration.SetRegistrationType("Non-Rigid")
            
        # set save deformation fields and transforms
        configuration.SetSaveTransforms(self._saveTransformsCheckBox.GetWidget().GetSelectedState())

        # set delete aligned images
        configuration.SetDeleteAlignedImages(self._deleteAlignedImagesCheckBox.GetWidget().GetSelectedState())

        # set delete aligned segmentations
        configuration.SetDeleteAlignedSegmentations(self._deleteAlignedSegmentationsCheckBox.GetWidget().GetSelectedState())

        # set normalize atlases
        configuration.SetNormalizeAtlases(self._normalizeAtlasCheckBox.GetWidget().GetSelectedState())
        
        # set output cast
        configuration.SetOutputCast(self._outputCastCombo.GetWidget().GetValue())
        
        # now start the calculation
        result = self._logic.Start(configuration)
        
        if result:

            # now loading the template
            templateID = self.GetHelper().DisplayImageInSlicer(os.path.normpath(self._outDirButton.GetWidget().GetFileName()+"/template.nrrd"),'Template')

            for currentLabel in labels: 
                # loading atlas for label i
                self.GetHelper().DisplayImageInSlicer(os.path.normpath(self._outDirButton.GetWidget().GetFileName()+"/atlas"+str(currentLabel)+".nrrd"),'AtlasForLabel'+str(currentLabel))
            
            # now loading the atlas for all labels
            newVolNodeID = self.GetHelper().DisplayImageInSlicer(os.path.normpath(self._outDirButton.GetWidget().GetFileName()+"/atlas.nrrd"),'AtlasAllLabels')

            selectionNode = slicer.ApplicationLogic.GetSelectionNode()
            selectionNode.SetReferenceActiveVolumeID(newVolNodeID)
            slicer.ApplicationLogic.PropagateVolumeSelection()


                  
    def CreateOutVolumeNodes(self):

        if not self._outVolumeSelector.GetSelected():

            self._outVolumeSelector.SetSelectedNew("vtkMRMLScalarVolumeNode")
            self._outVolumeSelector.ProcessNewNodeCommand("vtkMRMLScalarVolumeNode", "AtlasOutput")


    def UpdateMRML(self):

        if not self._updating:

            self._updating = 1

            self._updating = 0


    def UpdateGUI(self):

        if not self._updating:

            self._updating = 1

            self._updating = 0



    def ProcessMRMLEvents(self,callerID,event,callDataID = None):
        
        
        if self._associatedMRMLNode:
            if callerID == self._associatedMRMLNode.GetID() and event == vtkMRMLAtlasCreatorNode_LaunchComputationEvent:
                # the observed node was launched!
                self.GetHelper().info("LAUNCH!")
                

        # observe MRMLScene events
        if callerID == "MRMLScene" and event == vtkMRMLScene_NodeAddedEvent and callDataID:
            
            callDataAsMRMLNode = slicer.MRMLScene.GetNodeByID(callDataID)
            
            if isinstance(callDataAsMRMLNode, slicer.vtkMRMLAtlasCreatorNode):
                self.GetHelper().info("A new vtkMRMLAtlasCreatorNode was added: " + str(callDataID))
                self._associatedMRMLNode = callDataAsMRMLNode
                self.AddMRMLObserverByNumber(self._associatedMRMLNode,vtkMRMLAtlasCreatorNode_LaunchComputationEvent)



    def BuildGUI(self):

        self.GetUIPanel().AddPage("AtlasCreator","AtlasCreator","")
        self._atlascreatorPage = self.GetUIPanel().GetPageWidget("AtlasCreator")
        helpText = """**Atlas Creator v0.24**
        
More Information available at <a>http://www.slicer.org/slicerWiki/index.php/Modules:AtlasCreator</a>

**Input/Output**
Specify the I/O here. The original images and the segmentations must have the same filenames (f.e. case01.nrrd, case02.nrrd...) in both directories to get correctly linked together. 

Original Images: Directory to Original Images
Segmentations: Directory to Segmentations
Output directory: Directory to save transforms and atlases

If transforms already exist from a former registration, it is possible to skip the registration and use these transforms.
Optional Input:
- Skip Registration and Use Existing: True, if registration should be skipped
- Transforms directory: Path to existing transforms
- Existing Template: Path to an existing template
    
**Registration/Resampling**
The template can be either a fixed Volume or can be dynamically generated.

Registration Template:
- Dynamic: For dynamic generation, the number of iterations for calculating the Mean image have to be specified.
- Fixed: A filename to be selected as the default case. Possible files are present in the original images and segmentations directory.
    
Registration Type: Affine or Non-Rigid (BSpline) registration
    
Save transforms: Enable the saving of transforms and the default case. Then, the atlases can be generated again later without the Original Images but only with Segmentations.
    
**Atlas Generation**
Labels: The labels to include in the Atlas Generation. They are read automatically of the default case if possible.
Normalize Atlases to 0..1: Map the Atlas values to 0..1. This includes a cast to Float.
Output Case for Atlases: Change the Output Cast of the Atlases.
    
**Cluster Configuration**
The time-consuming task of Registration can be run on a cluster. Then, the Atlas Creator waits until all Registrations are complete.

Use Cluster: Activate the usage of the Scheduler Command for registration.
Scheduler Command: Executable to run before the commands for registering images. Could be qsub-run or similar to stage a cluster job.
"""
        aboutText = "This module was developed by Daniel Haehn and Kilian Pohl, University of Pennsylvania. The research was funded by an ARRA supplement to NIH NCRR (P41 RR13218)."
        self._helpAboutFrame = self.BuildHelpAndAboutFrame(self._atlascreatorPage,helpText,aboutText)


        self._moduleFrame.SetParent(self._atlascreatorPage)
        self._moduleFrame.Create()
        self._moduleFrame.SetLabelText("AtlasCreator")
        self._moduleFrame.ExpandFrame()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s" % (self._moduleFrame.GetWidgetName(),self._atlascreatorPage.GetWidgetName()))

        self._topFrame.SetParent(self._moduleFrame.GetFrame())
        self._topFrame.Create()
        self._topFrame.SetLabelText("Input/Output")
        self._topFrame.ExpandFrame()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._topFrame.GetWidgetName())

        self._origDirButton.SetParent(self._topFrame.GetFrame())
        self._origDirButton.Create()
        self._origDirButton.GetWidget().SetText("Click to pick a directory")
        self._origDirButton.SetLabelText("Original images:")
        self._origDirButton.GetWidget().GetLoadSaveDialog().ChooseDirectoryOn()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._origDirButton.GetWidgetName())

        self._origDirButton.GetWidget().GetLoadSaveDialog().Create()

        self._segDirButton.SetParent(self._topFrame.GetFrame())
        self._segDirButton.Create()
        self._segDirButton.GetWidget().SetText("Click to pick a directory")
        self._segDirButton.SetLabelText("Segmentations:")
        self._segDirButton.GetWidget().GetLoadSaveDialog().ChooseDirectoryOn()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._segDirButton.GetWidgetName())

        self._segDirButton.GetWidget().GetLoadSaveDialog().Create()

        self._outDirButton.SetParent(self._topFrame.GetFrame())
        self._outDirButton.Create()
        self._outDirButton.GetWidget().SetText("Click to pick a directory")
        self._outDirButton.SetLabelText("Output directory:")
        self._outDirButton.GetWidget().GetLoadSaveDialog().ChooseDirectoryOn()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._outDirButton.GetWidgetName())

        self._transformsFrame.SetParent(self._topFrame.GetFrame())
        self._transformsFrame.Create()
        self._transformsFrame.SetLabelText("Optional Input")
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._transformsFrame.GetWidgetName())

        self._transformsDirCheckBox.SetParent(self._transformsFrame.GetFrame())
        self._transformsDirCheckBox.Create()
        self._transformsDirCheckBox.SetLabelText("Skip Registration and use Existing:")
        self._transformsDirCheckBox.GetWidget().SetSelectedState(0)
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._transformsDirCheckBox.GetWidgetName())

        self._transformsDirButton.SetParent(self._transformsFrame.GetFrame())
        self._transformsDirButton.Create()
        self._transformsDirButton.GetWidget().SetText("Click to pick a directory")
        self._transformsDirButton.SetLabelText("Transforms directory:")
        self._transformsDirButton.GetWidget().GetLoadSaveDialog().ChooseDirectoryOn()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._transformsDirButton.GetWidgetName())

        self._transformsTemplateButton.SetParent(self._transformsFrame.GetFrame())
        self._transformsTemplateButton.Create()
        self._transformsTemplateButton.GetWidget().SetText("Click to pick a template")
        self._transformsTemplateButton.SetLabelText("Existing Template:")
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._transformsTemplateButton.GetWidgetName())

        self._secondFrame.SetParent(self._moduleFrame.GetFrame())
        self._secondFrame.Create()
        self._secondFrame.SetLabelText("Registration/Resampling")
        self._secondFrame.ExpandFrame()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._secondFrame.GetWidgetName())

        self._defaultCaseFrame.SetParent(self._secondFrame.GetFrame())
        self._defaultCaseFrame.Create()
        self._defaultCaseFrame.SetLabelText("Registration Template:")
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._defaultCaseFrame.GetWidgetName())

        self._dynamicRadio.SetParent(self._defaultCaseFrame.GetFrame())
        self._dynamicRadio.Create()
        self._dynamicRadio.SetText("Dynamic")
        self._dynamicRadio.SetBalloonHelpString("Align to mean of Original Images")
        slicer.TkCall("pack %s -side left -anchor nw -fill x -padx 2 -pady 1" % self._dynamicRadio.GetWidgetName())

        self._meanSpinBox.SetParent(self._defaultCaseFrame.GetFrame())
        self._meanSpinBox.Create()
        self._meanSpinBox.GetWidget().SetRange(1,10)
        self._meanSpinBox.GetWidget().SetIncrement(1)
        self._meanSpinBox.GetWidget().SetValue(5)
        self._meanSpinBox.SetLabelText("Alignment Iterations:")        
        self._meanSpinBox.SetBalloonHelpString("The number of iterations for aligning to the mean of the Original Images.")
        slicer.TkCall("pack %s -side top -anchor ne -fill x -padx 2 -pady 2" % self._meanSpinBox.GetWidgetName())

        self._fixedRadio.SetParent(self._defaultCaseFrame.GetFrame())
        self._fixedRadio.Create()
        self._fixedRadio.SetText("Fixed")
        self._fixedRadio.SetBalloonHelpString("Align to Default Case")
        slicer.TkCall("pack %s -side left -anchor nw -fill x -padx 2 -pady 1" % self._fixedRadio.GetWidgetName())

        self._defaultCaseEntry.SetParent(self._defaultCaseFrame.GetFrame())
        self._defaultCaseEntry.Create()
        self._defaultCaseEntry.GetWidget().ReadOnlyOn()
        self._defaultCaseEntry.SetLabelText("Default case:")
        self._defaultCaseEntry.SetBalloonHelpString("The filename of the default case used for registration.")
        slicer.TkCall("pack %s -side top -anchor ne -fill x -padx 2 -pady 2" % self._defaultCaseEntry.GetWidgetName())

        self._dynamicRadio.SetSelectedState(0)
        self._fixedRadio.SetSelectedState(1)
        self._defaultCaseEntry.SetEnabled(1)
        self._meanSpinBox.SetEnabled(0)

        self._regTypeRadios.SetParent(self._secondFrame.GetFrame())
        self._regTypeRadios.Create()
        self._regTypeRadios.SetLabelText("Registration Type:")

        affineRadio = self._regTypeRadios.GetWidget().AddWidget(0)
        affineRadio.SetText("Affine")

        nonRigidRadio = self._regTypeRadios.GetWidget().AddWidget(1)
        nonRigidRadio.SetText("Non-Rigid")
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._regTypeRadios.GetWidgetName())

        self._regTypeRadios.GetWidget().GetWidget(0).SetSelectedState(1)

        self._saveTransformsCheckBox.SetParent(self._secondFrame.GetFrame())
        self._saveTransformsCheckBox.Create()
        self._saveTransformsCheckBox.SetLabelText("Save Transforms:")
        self._saveTransformsCheckBox.GetWidget().SetSelectedState(1)
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._saveTransformsCheckBox.GetWidgetName())

        self._deleteAlignedImagesCheckBox.SetParent(self._secondFrame.GetFrame())
        self._deleteAlignedImagesCheckBox.Create()
        self._deleteAlignedImagesCheckBox.SetLabelText("Delete Aligned Images:")
        self._deleteAlignedImagesCheckBox.GetWidget().SetSelectedState(1)
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._deleteAlignedImagesCheckBox.GetWidgetName())

        self._deleteAlignedSegmentationsCheckBox.SetParent(self._secondFrame.GetFrame())
        self._deleteAlignedSegmentationsCheckBox.Create()
        self._deleteAlignedSegmentationsCheckBox.SetLabelText("Delete Aligned Segmentations:")
        self._deleteAlignedSegmentationsCheckBox.GetWidget().SetSelectedState(1)
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._deleteAlignedSegmentationsCheckBox.GetWidgetName())

        self._thirdFrame.SetParent(self._moduleFrame.GetFrame())
        self._thirdFrame.Create()
        self._thirdFrame.SetLabelText("Atlas Generation")
        self._thirdFrame.ExpandFrame()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._thirdFrame.GetWidgetName())

        self._labelsEntry.SetParent(self._thirdFrame.GetFrame())
        self._labelsEntry.Create()
        self._labelsEntry.SetLabelText("Labels:")
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._labelsEntry.GetWidgetName())

        self._normalizeAtlasCheckBox.SetParent(self._thirdFrame.GetFrame())
        self._normalizeAtlasCheckBox.Create()
        self._normalizeAtlasCheckBox.SetLabelText("Normalize Atlases:")
        self._normalizeAtlasCheckBox.GetWidget().SetSelectedState(0)
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._normalizeAtlasCheckBox.GetWidgetName())

        self._outputCastCombo.SetParent(self._thirdFrame.GetFrame())
        self._outputCastCombo.Create()
        self._outputCastCombo.GetWidget().ReadOnlyOn()
        self._outputCastCombo.SetLabelText("Output cast for Atlases:")
        self._outputCastCombo.SetBalloonHelpString("The output cast for the atlases.")
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._outputCastCombo.GetWidgetName())

        self._outputCastCombo.GetWidget().AddValue('Char')              
        self._outputCastCombo.GetWidget().AddValue('Unsigned Char')
        self._outputCastCombo.GetWidget().AddValue('Double')
        self._outputCastCombo.GetWidget().AddValue('Float')
        self._outputCastCombo.GetWidget().AddValue('Int')
        self._outputCastCombo.GetWidget().AddValue('Unsigned Int')                          
        self._outputCastCombo.GetWidget().AddValue('Long')
        self._outputCastCombo.GetWidget().AddValue('Unsigned Long') 
        self._outputCastCombo.GetWidget().AddValue('Short')
        self._outputCastCombo.GetWidget().AddValue('Unsigned Short')
        self._outputCastCombo.GetWidget().SetValue('Short')

        self._fourthFrame.SetParent(self._moduleFrame.GetFrame())
        self._fourthFrame.Create()
        self._fourthFrame.SetLabelText("Cluster Configuration")
        self._fourthFrame.CollapseFrame()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._fourthFrame.GetWidgetName())

        self._clusterCheckBox.SetParent(self._fourthFrame.GetFrame())
        self._clusterCheckBox.Create()
        self._clusterCheckBox.SetLabelText("Use Cluster:")
        self._clusterCheckBox.GetWidget().SetSelectedState(0)
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._clusterCheckBox.GetWidgetName())

        self._schedulerEntry.SetParent(self._fourthFrame.GetFrame())
        self._schedulerEntry.Create()
        self._schedulerEntry.SetLabelText("Scheduler Command:")
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._schedulerEntry.GetWidgetName())
        
        self._waitEntry.SetParent(self._fourthFrame.GetFrame())
        self._waitEntry.Create()
        self._waitEntry.SetLabelText("Wait interval to check if jobs completed [s]:")
        self._waitEntry.GetWidget().SetValue("60")
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._waitEntry.GetWidgetName())
        
        self._fifthFrame.SetParent(self._moduleFrame.GetFrame())
        self._fifthFrame.Create()
        self._fifthFrame.SetLabelText("Advanced")
        self._fifthFrame.CollapseFrame()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._fifthFrame.GetWidgetName())        
        
        self._generateButton.SetParent(self._moduleFrame.GetFrame())
        self._generateButton.Create()
        self._generateButton.SetEnabled(1)
        self._generateButton.SetText("Generate Atlas!")
        self._generateButton.SetBalloonHelpString("Click to generate the atlas")
        slicer.TkCall("pack %s -side top -anchor e -padx 2 -pady 2" % self._generateButton.GetWidgetName())


    def TearDownGUI(self):
        if self.GetUIPanel().GetUserInterfaceManager():
            self.GetUIPanel().RemovePage("AtlasCreator")

    def GetHelper(self):
        return self._helper

    def GetMyLogic(self):
        return self._logic

    def ReadLabelsFromImage(self,path):
        
        node = self.GetHelper().LoadVolume(os.path.normpath(path))
        labels = self.GetHelper().GetLabels(node.GetImageData())
        slicer.MRMLScene.RemoveNode(node)
        
        return labels
    
