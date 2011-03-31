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

        self._inputOutputFrame = slicer.vtkSlicerModuleCollapsibleFrame()
        self._parametersFrame = slicer.vtkSlicerModuleCollapsibleFrame()
        
        # I/O Frame
        self._origDirButton = slicer.vtkKWLoadSaveButtonWithLabel()
        self._segDirButton = slicer.vtkKWLoadSaveButtonWithLabel()
        self._outDirButton = slicer.vtkKWLoadSaveButtonWithLabel()                
        self._registrationTypeRadios = slicer.vtkKWRadioButtonSetWithLabel()
        # End I/O Frame
        
        # Parameters Frame
        self._toolkitCombo = slicer.vtkKWComboBoxWithLabel()
        self._deformationTypeRadios = slicer.vtkKWRadioButtonSetWithLabel()
        self._meanIterationsSpinBox = slicer.vtkKWSpinBoxWithLabel()
        self._defaultCaseCombo = slicer.vtkKWComboBoxWithLabel()
        # End Parameters Frame
        
        
        # Advanced Frame
        
        self._advancedFrame = slicer.vtkSlicerModuleCollapsibleFrame()
        self._clusterFrame = slicer.vtkSlicerModuleCollapsibleFrame()
        self._pcaFrame = slicer.vtkSlicerModuleCollapsibleFrame()
        self._transformsFrame = slicer.vtkSlicerModuleCollapsibleFrame()
        self._miscFrame = slicer.vtkSlicerModuleCollapsibleFrame()
        
        # Cluster Frame
        self._clusterCheckBox = slicer.vtkKWCheckButtonWithLabel()
        self._schedulerEntry = slicer.vtkKWEntryWithLabel()        
        # End Cluster Frame
        
        # PCA Frame
        self._pcaCheckBox = slicer.vtkKWCheckButtonWithLabel()
        self._maxEigenVectors = slicer.vtkKWSpinBoxWithLabel()
        self._pcaCombine = slicer.vtkKWCheckButtonWithLabel()
        # End PCA Frame
        
        # ExistingTransforms Frame
        self._transformsDirCheckBox = slicer.vtkKWCheckButtonWithLabel()
        self._transformsDirButton = slicer.vtkKWLoadSaveButtonWithLabel()
        self._transformsTemplateButton = slicer.vtkKWLoadSaveButtonWithLabel()
        # End ExistingTransforms Frame
        
        # Misc. Frame
        self._labelsEntry = slicer.vtkKWEntryWithLabel()
        self._normalizeAtlasCheckBox = slicer.vtkKWCheckButtonWithLabel()
        self._normalizeValueEntry = slicer.vtkKWEntryWithLabel()
        self._outputCastCombo = slicer.vtkKWComboBoxWithLabel()
        self._saveTransformsCheckBox = slicer.vtkKWCheckButtonWithLabel()
        self._deleteAlignedImagesCheckBox = slicer.vtkKWCheckButtonWithLabel()
        self._deleteAlignedSegmentationsCheckBox = slicer.vtkKWCheckButtonWithLabel()
        self._debugCheckBox = slicer.vtkKWCheckButtonWithLabel()
        self._dryrunCheckBox = slicer.vtkKWCheckButtonWithLabel() 
        # End Misc. Frame
        
        # End Advanced Frame
        
        
        # The Start Button
        self._generateButton = slicer.vtkKWPushButton()
        

        # Internal Classes and Flags
        self._helper = AtlasCreatorHelper(self)

        self._logic = AtlasCreatorLogic(self)

        self._associatedMRMLNode = None

        self._updating = 0
        
        


    def Destructor(self):

        self._helper = None
        self._logic = None
        self._associatedMRMLNode = None
            
        

    def RemoveMRMLNodeObservers(self):
        pass
    
    def RemoveLogicObservers(self):
        pass

    def AddGUIObservers(self):
        self._generateButtonTag = self.AddObserverByNumber(self._generateButton,vtkKWPushButton_InvokedEvent)
        self._origDirButtonTag = self.AddObserverByNumber(self._origDirButton.GetWidget().GetLoadSaveDialog(),vtkKWFileBrowserDialog_FileNameChangedEvent)
        self._segDirButtonTag = self.AddObserverByNumber(self._segDirButton.GetWidget().GetLoadSaveDialog(),vtkKWFileBrowserDialog_FileNameChangedEvent)
        self._pairFixedRadioTag = self.AddObserverByNumber(self._pairFixedRadio,vtkKWRadioButton_SelectedStateChangedEvent)
        self._pairOnlineRadioTag = self.AddObserverByNumber(self._pairOnlineRadio,vtkKWRadioButton_SelectedStateChangedEvent)
        # TODO Group Online Tag..
        self._normalizeAtlasCheckBoxTag = self.AddObserverByNumber(self._normalizeAtlasCheckBox.GetWidget(),vtkKWCheckButton_SelectedStateChangedEvent)
        self._defaultCaseEntryTag = self.AddObserverByNumber(self._defaultCaseCombo.GetWidget(),vtkKWEntry_EntryValueChangedEvent)
        self._transformsTemplateButtonTag = self.AddObserverByNumber(self._transformsTemplateButton.GetWidget().GetLoadSaveDialog(),vtkKWFileBrowserDialog_FileNameChangedEvent)
        self._mrmlNodeAddedTag = self.AddMRMLObserverByNumber(slicer.MRMLScene,vtkMRMLScene_NodeAddedEvent)


    def RemoveGUIObservers(self):
        pass

    def ProcessGUIEvents(self,caller,event):
        if not self._updating:

            if caller == self._generateButton and event == vtkKWPushButton_InvokedEvent:
                self.UpdateMRML()
                self.GenerateAtlas()
            elif caller == self._origDirButton.GetWidget().GetLoadSaveDialog() and event == vtkKWFileBrowserDialog_FileNameChangedEvent:                
                self.UpdateCaseCombobox()
                self.UpdateMRML()
            elif caller == self._segDirButton.GetWidget().GetLoadSaveDialog() and event == vtkKWFileBrowserDialog_FileNameChangedEvent:
                self.UpdateCaseCombobox()
                self.UpdateMRML()
            elif caller == self._pairFixedRadio and event == vtkKWRadioButton_SelectedStateChangedEvent:
                self.ToggleMeanAndDefaultCase1()
                self.UpdateMRML()
            elif caller == self._pairOnlineRadio and event == vtkKWRadioButton_SelectedStateChangedEvent:
                self.ToggleMeanAndDefaultCase2()
                self.UpdateMRML()
            elif caller == self._normalizeAtlasCheckBox.GetWidget() and event == vtkKWCheckButton_SelectedStateChangedEvent:
                self.ToggleNormalize()
                self.UpdateMRML()
            elif caller == self._defaultCaseCombo.GetWidget() and event == vtkKWEntry_EntryValueChangedEvent:
                self.GetHelper().debug("defaultCaseEntry changed")
                self.UpdateLabelList()
                self.UpdateMRML()
            elif caller == self._transformsTemplateButton.GetWidget().GetLoadSaveDialog() and event == vtkKWFileBrowserDialog_FileNameChangedEvent:
                self.UpdateLabelList()
                self.UpdateMRML()
                
                
    def ToggleNormalize(self):
        normalize = self._normalizeAtlasCheckBox.GetWidget().GetSelectedState()
        
        if normalize:
            self._outputCastCombo.SetEnabled(0)
            self._outputCastCombo.GetWidget().SetValue("Float")
            self._normalizeValueEntry.SetEnabled(1)
        else:
            self._outputCastCombo.SetEnabled(1)
            self._normalizeValueEntry.SetEnabled(0)
                
    def ToggleMeanAndDefaultCase1(self):
        useDefCase = self._pairFixedRadio.GetSelectedState()
        useMean = self._pairOnlineRadio.GetSelectedState()

        if useDefCase:
            self._pairOnlineRadio.SetSelectedState(0)
            #self._fixedRadio.SetSelectedState(1)
            self._defaultCaseCombo.SetEnabled(1)
            self._meanIterationsSpinBox.SetEnabled(0)
        else:
            self._pairOnlineRadio.SetSelectedState(1)
            #self._fixedRadio.SetSelectedState(0)
            self._defaultCaseCombo.SetEnabled(0)
            self._meanIterationsSpinBox.SetEnabled(1)

    def ToggleMeanAndDefaultCase2(self):
        useMean = self._pairOnlineRadio.GetSelectedState()

        if useMean:
            #self._dynamicRadio.SetSelectedState(1)
            self._pairFixedRadio.SetSelectedState(0)
            self._defaultCaseCombo.SetEnabled(0)
            self._meanIterationsSpinBox.SetEnabled(1)
        else:
            #self._dynamicRadio.SetSelectedState(0)
            self._pairFixedRadio.SetSelectedState(1)
            self._defaultCaseCombo.SetEnabled(1)
            self._meanIterationsSpinBox.SetEnabled(0)

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
                    self._defaultCaseCombo.GetWidget().AddValue(caseFile)
                    self._defaultCaseCombo.GetWidget().SetValue(caseFile)
                    
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
                caseFile = self._defaultCaseCombo.GetWidget().GetValue()
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
        configuration.SetDynamicTemplateIterations(self._meanIterationsSpinBox.GetWidget().GetValue())    
        
        # set the default case
        defCaseFileName = self._defaultCaseCombo.GetWidget().GetValue()
        if not self._transformsDirCheckBox.GetWidget().GetSelectedState():     
            defCaseFilePath = os.path.join(self._origDirButton.GetWidget().GetFileName(),defCaseFileName)
            # ... normalize it and set it
            configuration.SetFixedTemplateDefaultCaseFilePath(os.path.normpath(defCaseFilePath))
            
        labels = self._labelsEntry.GetWidget().GetValue()
        configuration.SetLabelsList(labels)
            
        # set the registration type
        if self._deformationTypeRadios.GetWidget().GetWidget(0).GetSelectedState():
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
        configuration.SetNormalizeTo(self._normalizeValueEntry.GetWidget().GetValue())
        
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
            #newVolNodeID = self.GetHelper().DisplayImageInSlicer(os.path.normpath(self._outDirButton.GetWidget().GetFileName()+"/atlas.nrrd"),'AtlasAllLabels')

            selectionNode = slicer.ApplicationLogic.GetSelectionNode()
            selectionNode.SetReferenceActiveVolumeID(templateID)
            slicer.ApplicationLogic.PropagateVolumeSelection()


                  
    def CreateOutVolumeNodes(self):

        if not self._outVolumeSelector.GetSelected():

            self._outVolumeSelector.SetSelectedNew("vtkMRMLScalarVolumeNode")
            self._outVolumeSelector.ProcessNewNodeCommand("vtkMRMLScalarVolumeNode", "AtlasOutput")


    def UpdateMRML(self):

        if not self._updating:

            self._updating = 1
            
            if not self._associatedMRMLNode:
                # if we do not have an associated MRML Node, we will create one right here!!
                self._associatedMRMLNode = slicer.vtkMRMLAtlasCreatorNode()
                self._associatedMRMLNode.InitializeByDefault()
            
            # TODO debug
            
            # TODO dryrun
            
            # cluster stuff
            if self._clusterCheckBox.GetWidget().GetSelectedState():
                self._associatedMRMLNode.SetUseCluster(1)
            else:
                self._associatedMRMLNode.SetUseCluster(0)
                
            schedCommand = self._schedulerEntry.GetWidget().GetValue()
            if schedCommand:
                self._associatedMRMLNode.SetSchedulerCommand(schedCommand)
                
            ###############################################################
            
            # input panel
            imagesDir = self._origDirButton.GetWidget().GetFileName()
            if imagesDir:
                self._associatedMRMLNode.SetOriginalImagesFilePathList(self.GetHelper().ConvertDirectoryToString(imagesDir))

            segmentationsDir = self._segDirButton.GetWidget().GetFileName()
            if segmentationsDir:
                self._associatedMRMLNode.SetSegmentationsFilePathList(self.GetHelper().ConvertDirectoryToString(segmentationsDir))
                
            outputDir = self._origDirButton.GetWidget().GetFileName()
            if outputDir:
                self._associatedMRMLNode.SetOutputDirectory(outputDir)
            
            # input panel, skip registration frame
            if self._transformsDirCheckBox.GetWidget().GetSelectedState():
                self._associatedMRMLNode.SetSkipRegistration(1)
            else:
                self._associatedMRMLNode.SetSkipRegistration(0)
                
            transformsDir = self._transformsDirButton.GetWidget().GetFileName()
            if transformsDir:
                self._associatedMRMLNode.SetTransformsDir(self.GetHelper().ConvertDirectoryToString(transformsDir))
                
            transformsTemplate = self._transformsTemplateButton.GetWidget().GetFileName()
            if transformsTemplate:
                self._associatedMRMLNode.SetExistingTemplate(os.path.normpath(transformsTemplate))
            
            ###############################################################
                    
            
                
            # TODO CMTK/BRAINSFit
                
            '''
            
               
                if debug or dryrun:
                    evalpythonCommand += "n.SetDebugMode(1);"
            
                if dryrun:
                    evalpythonCommand += "n.SetDryrunMode(1);"
            
                # set special settings if clusterMode or skipRegistrationMode is requested
                if cluster:
                    # cluster Mode
                    evalpythonCommand += "n.SetUseCluster(1);"
                    evalpythonCommand += "n.SetSchedulerCommand('" + schedulerCommand + "');"
                elif skipRegistration:
                    # skipRegistration Mode
                    evalpythonCommand += "n.SetSkipRegistration(1);"
                    evalpythonCommand += "n.SetTransformsDirectory('" + transformsDir + "');"
                    evalpythonCommand += "n.SetExistingTemplate('" + existingTemplate + "');"
                    
                # now the configuration options which are valid for all
                if imagesDir:
                    evalpythonCommand += "n.SetOriginalImagesFilePathList('" + ConvertDirectoryToString(imagesDir) + "');"
                
                if segmentationsDir:
                    evalpythonCommand += "n.SetSegmentationsFilePathList('" + ConvertDirectoryToString(segmentationsDir) + "');"
                
                if outputDir:
                    evalpythonCommand += "n.SetOutputDirectory('" + outputDir + "');"
                
                if useCMTK:
                    evalpythonCommand += "n.SetToolkit('CMTK');"
                else:
                    evalpythonCommand += "n.SetToolkit('BRAINSFit');"
                
                if fixed:
                    evalpythonCommand += "n.SetTemplateType('fixed');"
                    evalpythonCommand += "n.SetFixedTemplateDefaultCaseFilePath('" + template + "');"
                else:
                    evalpythonCommand += "n.SetTemplateType('dynamic');"
                    evalpythonCommand += "n.SetDynamicTemplateIterations(" + str(meanIterations) + ");"
                
                evalpythonCommand += "n.SetLabelsList('" + ConvertListToString(labels) + "');"
                        
                if nonRigid:
                    evalpythonCommand += "n.SetRegistrationType('Non-Rigid');"
                else:
                    evalpythonCommand += "n.SetRegistrationType('Affine');"
                        
                if writeTransforms:
                    evalpythonCommand += "n.SetSaveTransforms(1);"
                else:
                    evalpythonCommand += "n.SetSaveTransforms(0);"
            
                if keepAligned:
                    evalpythonCommand += "n.SetDeleteAlignedImages(0);"
                    evalpythonCommand += "n.SetDeleteAlignedSegmentations(0);"
                else:
                    evalpythonCommand += "n.SetDeleteAlignedImages(1);"
                    evalpythonCommand += "n.SetDeleteAlignedSegmentations(1);"
                    
                if normalize:
                    evalpythonCommand += "n.SetNormalizeAtlases(1);"
                    evalpythonCommand += "n.SetNormalizeTo(" + str(normalizeTo) + ");"
                else:
                    evalpythonCommand += "n.SetNormalizeAtlases(0);"
                    evalpythonCommand += "n.SetNormalizeTo(-1);"
            
                if pca:
                    evalpythonCommand += "n.SetPCAAnalysis(1);"
                    evalpythonCommand += "n.SetPCAMaxEigenVectors(" + str(pcaMaxEigenVectors) + ");"
                else:
                    evalpythonCommand += "n.SetPCAAnalysis(0);"
                    evalpythonCommand += "n.SetPCAMaxEigenVectors(10);"
                    
                if pcaCombine:
                    evalpythonCommand += "n.SetPCACombine(1);"
                else:
                    evalpythonCommand += "n.SetPCACombine(0);"
                    
                
            
                evalpythonCommand += "n.SetOutputCast('" + outputCast + "');"
                '''             

            self._updating = 0


    def UpdateGUI(self):

        if not self._updating:

            self._updating = 1

            self._updating = 0



    def ProcessMRMLEvents(self,callerID,event,callDataID = None):
        
        
        if self._associatedMRMLNode:
            # check if the callerID is still a valid Node in the scene
            if slicer.MRMLScene.GetNodeByID(callerID):
                # check if the callerID equals the associated MRML Node but only if the event is the launch event
                # we do not want to react to Modify events etc.
                if event == vtkMRMLAtlasCreatorNode_LaunchComputationEvent and callerID == self._associatedMRMLNode.GetID():
                    # the observed node was launched!
                    self.GetMyLogic().Start(self._associatedMRMLNode)
                    

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
        helpText = """**Atlas Creator v0.3**
        
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

        # starting I/O Frame
        self._inputOutputFrame.SetParent(self._moduleFrame.GetFrame())
        self._inputOutputFrame.Create()
        self._inputOutputFrame.SetLabelText("Input/Output")
        self._inputOutputFrame.ExpandFrame()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._inputOutputFrame.GetWidgetName())

        self._origDirButton.SetParent(self._inputOutputFrame.GetFrame())
        self._origDirButton.Create()
        self._origDirButton.GetWidget().SetText("Click to pick a directory")
        self._origDirButton.SetLabelWidth(20)
        self._origDirButton.SetLabelText("Original images:")
        self._origDirButton.GetWidget().GetLoadSaveDialog().ChooseDirectoryOn()
        self._origDirButton.SetBalloonHelpString("The folder of the original Images.")
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._origDirButton.GetWidgetName())

        self._origDirButton.GetWidget().GetLoadSaveDialog().Create()

        self._segDirButton.SetParent(self._inputOutputFrame.GetFrame())
        self._segDirButton.Create()
        self._segDirButton.GetWidget().SetText("Click to pick a directory")
        self._segDirButton.SetLabelWidth(20)
        self._segDirButton.SetLabelText("Segmentations:")
        self._segDirButton.GetWidget().GetLoadSaveDialog().ChooseDirectoryOn()
        self._segDirButton.SetBalloonHelpString("The folder of the Segmentations.")
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._segDirButton.GetWidgetName())

        self._segDirButton.GetWidget().GetLoadSaveDialog().Create()

        self._outDirButton.SetParent(self._inputOutputFrame.GetFrame())
        self._outDirButton.Create()
        self._outDirButton.SetLabelWidth(20)
        self._outDirButton.GetWidget().SetText("Click to pick a directory")
        self._outDirButton.SetLabelText("Output directory:")
        self._outDirButton.GetWidget().GetLoadSaveDialog().ChooseDirectoryOn()
        self._outDirButton.SetBalloonHelpString("The Output Folder. Note: a new folder based on the same name will be created.")
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._outDirButton.GetWidgetName())


        self._registrationTypeRadios.SetParent(self._inputOutputFrame.GetFrame())
        self._registrationTypeRadios.Create()
        self._registrationTypeRadios.SetLabelWidth(20)
        self._registrationTypeRadios.SetLabelText("Registration Type:")

        self._pairFixedRadio = self._registrationTypeRadios.GetWidget().AddWidget(0)
        self._pairFixedRadio.SetText("Pair Fixed")
        self._pairFixedRadio.SetBalloonHelpString("Pair the cases against a fixed template.")

        self._pairOnlineRadio = self._registrationTypeRadios.GetWidget().AddWidget(1)
        self._pairOnlineRadio.SetText("Pair Online")
        self._pairOnlineRadio.SetBalloonHelpString("Pair the cases against a dynamic template calculated as a mean Image.")
        
        self._groupOnlineRadio = self._registrationTypeRadios.GetWidget().AddWidget(2)
        self._groupOnlineRadio.SetText("Group Online")
        self._groupOnlineRadio.SetEnabled(0) # TODO disabled for now :)
        self._groupOnlineRadio.SetBalloonHelpString("Use un-biased registration: each case against each case. UNDER CONSTRUCTION!")
        
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._registrationTypeRadios.GetWidgetName())

        self._registrationTypeRadios.GetWidget().GetWidget(0).SetSelectedState(1) # by default, Pair Fixed
        self._defaultCaseCombo.SetEnabled(1)
        self._meanIterationsSpinBox.SetEnabled(0)        
        # ending I/O Frame
        
        # starting Parameter Frame
        self._parametersFrame.SetParent(self._moduleFrame.GetFrame())
        self._parametersFrame.Create()
        self._parametersFrame.SetLabelText("Parameters")
        self._parametersFrame.CollapseFrame()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._parametersFrame.GetWidgetName())

        self._toolkitCombo.SetParent(self._parametersFrame.GetFrame())
        self._toolkitCombo.Create()
        self._toolkitCombo.GetWidget().ReadOnlyOn()
        self._toolkitCombo.SetLabelText("Toolkit:")
        self._toolkitCombo.SetLabelWidth(20)
        self._toolkitCombo.SetBalloonHelpString("The toolkit to use for Registration.")
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._toolkitCombo.GetWidgetName())

        self._toolkitCombo.GetWidget().AddValue("BRAINSFit")
        self._toolkitCombo.GetWidget().AddValue("CMTK")
        self._toolkitCombo.GetWidget().SetValue("BRAINSFit")

        self._deformationTypeRadios.SetParent(self._parametersFrame.GetFrame())
        self._deformationTypeRadios.Create()
        self._deformationTypeRadios.SetLabelWidth(20)
        self._deformationTypeRadios.SetLabelText("Deformation:")

        affineRadio = self._deformationTypeRadios.GetWidget().AddWidget(0)
        affineRadio.SetText("Affine")
        affineRadio.SetBalloonHelpString("Use affine deformation.")

        nonRigidRadio = self._deformationTypeRadios.GetWidget().AddWidget(1)
        nonRigidRadio.SetText("Non-Rigid")
        nonRigidRadio.SetBalloonHelpString("Use non-rigid deformation.")
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._deformationTypeRadios.GetWidgetName())

        self._deformationTypeRadios.GetWidget().GetWidget(0).SetSelectedState(1)


        self._meanIterationsSpinBox.SetParent(self._parametersFrame.GetFrame())
        self._meanIterationsSpinBox.Create()
        self._meanIterationsSpinBox.GetWidget().SetRange(1,10)
        self._meanIterationsSpinBox.GetWidget().SetIncrement(1)
        self._meanIterationsSpinBox.GetWidget().SetValue(5)
        self._meanIterationsSpinBox.SetLabelWidth(20)
        self._meanIterationsSpinBox.SetLabelText("Alignment Iterations:")        
        self._meanIterationsSpinBox.SetBalloonHelpString("The number of iterations for aligning to the mean of the Original Images.")
        slicer.TkCall("pack %s -side top -anchor ne -fill x -padx 2 -pady 2" % self._meanIterationsSpinBox.GetWidgetName())

        self._defaultCaseCombo.SetParent(self._parametersFrame.GetFrame())
        self._defaultCaseCombo.Create()
        self._defaultCaseCombo.GetWidget().ReadOnlyOn()
        self._defaultCaseCombo.SetLabelText("Default case:")
        self._defaultCaseCombo.SetLabelWidth(20)
        self._defaultCaseCombo.SetBalloonHelpString("The filename of the default case used for registration.")
        slicer.TkCall("pack %s -side top -anchor ne -fill x -padx 2 -pady 2" % self._defaultCaseCombo.GetWidgetName())

        # ending Parameter Frame


        # starting Advanced Frame
        self._advancedFrame.SetParent(self._moduleFrame.GetFrame())
        self._advancedFrame.Create()
        self._advancedFrame.SetLabelText("Advanced")
        self._advancedFrame.CollapseFrame()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._advancedFrame.GetWidgetName())

        # starting Cluster Frame
        self._clusterFrame.SetParent(self._advancedFrame.GetFrame())
        self._clusterFrame.Create()
        self._clusterFrame.SetLabelText("Cluster Configuration")
        self._clusterFrame.CollapseFrame()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._clusterFrame.GetWidgetName())

        self._clusterCheckBox.SetParent(self._clusterFrame.GetFrame())
        self._clusterCheckBox.Create()
        self._clusterCheckBox.SetLabelText("Use Cluster:")
        self._clusterCheckBox.GetWidget().SetSelectedState(0)
        self._clusterCheckBox.SetLabelWidth(20)
        self._clusterCheckBox.SetBalloonHelpString("Use the cluster configuration to perform all computations.")
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._clusterCheckBox.GetWidgetName())

        self._schedulerEntry.SetParent(self._clusterFrame.GetFrame())
        self._schedulerEntry.Create()
        self._schedulerEntry.SetLabelText("Scheduler Command:")
        self._schedulerEntry.SetLabelWidth(20)
        self._schedulerEntry.SetBalloonHelpString("The scheduler command is used to execute computations in a cluster configuration.")
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._schedulerEntry.GetWidgetName())
        # ending Cluster Frame

        # starting PCA Frame
        self._pcaFrame.SetParent(self._advancedFrame.GetFrame())
        self._pcaFrame.Create()
        self._pcaFrame.SetLabelText("Principal Component Analysis")
        self._pcaFrame.CollapseFrame()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._pcaFrame.GetWidgetName())   

        self._pcaCheckBox.SetParent(self._pcaFrame.GetFrame())
        self._pcaCheckBox.Create()
        self._pcaCheckBox.SetLabelText("Activate PCA:")
        self._pcaCheckBox.GetWidget().SetSelectedState(0)
        self._pcaCheckBox.SetBalloonHelpString("Activate PCA Analysis on top of Atlas Creation.")
        self._pcaCheckBox.SetLabelWidth(20)
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._pcaCheckBox.GetWidgetName())

        self._maxEigenVectors.SetParent(self._pcaFrame.GetFrame())
        self._maxEigenVectors.Create()
        self._maxEigenVectors.GetWidget().SetRange(1,100)
        self._maxEigenVectors.GetWidget().SetIncrement(1)
        self._maxEigenVectors.GetWidget().SetValue(10)
        self._maxEigenVectors.SetLabelText("Max. EigenVectors:")
        self._maxEigenVectors.SetLabelWidth(20)
        self._maxEigenVectors.SetBalloonHelpString("Set the maximal number of EigenVectors to use for PCA.")
        slicer.TkCall("pack %s -side top -anchor ne -fill x -padx 2 -pady 2" % self._maxEigenVectors.GetWidgetName())

        self._pcaCombine.SetParent(self._pcaFrame.GetFrame())
        self._pcaCombine.Create()
        self._pcaCombine.SetLabelText("Combine PCAs:")
        self._pcaCombine.GetWidget().SetSelectedState(0)
        self._pcaCombine.SetLabelWidth(20)
        self._pcaCombine.SetBalloonHelpString("Generate only one PCA output for all labels.")        
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._pcaCombine.GetWidgetName())
        # ending PCA Frame

        # starting ExistingTransforms Frame
        self._transformsFrame.SetParent(self._advancedFrame.GetFrame())
        self._transformsFrame.Create()
        self._transformsFrame.SetLabelText("Use Existing Transforms")
        self._transformsFrame.CollapseFrame()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._transformsFrame.GetWidgetName())

        self._transformsDirCheckBox.SetParent(self._transformsFrame.GetFrame())
        self._transformsDirCheckBox.Create()
        self._transformsDirCheckBox.SetLabelText("Skip Registration:")
        self._transformsDirCheckBox.SetLabelWidth(20)
        self._transformsDirCheckBox.GetWidget().SetSelectedState(0)
        self._transformsDirCheckBox.SetBalloonHelpString("Use existing Transforms and skip Registration.")
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._transformsDirCheckBox.GetWidgetName())

        self._transformsDirButton.SetParent(self._transformsFrame.GetFrame())
        self._transformsDirButton.Create()
        self._transformsDirButton.GetWidget().SetText("Click to pick a directory")
        self._transformsDirButton.SetLabelText("Transforms directory:")
        self._transformsDirButton.SetLabelWidth(20)
        self._transformsDirButton.SetBalloonHelpString("Select the folder containing existing transforms.")
        self._transformsDirButton.GetWidget().GetLoadSaveDialog().ChooseDirectoryOn()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._transformsDirButton.GetWidgetName())

        self._transformsTemplateButton.SetParent(self._transformsFrame.GetFrame())
        self._transformsTemplateButton.Create()
        self._transformsTemplateButton.GetWidget().SetText("Click to pick a template")
        self._transformsTemplateButton.SetLabelText("Existing Template:")
        self._transformsTemplateButton.SetLabelWidth(20)
        self._transformsTemplateButton.SetBalloonHelpString("Pick an existing template to use as Resampling space.")
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._transformsTemplateButton.GetWidgetName())
        # ending ExistingTransforms Frame
        
        # starting Misc. Frame
        self._miscFrame.SetParent(self._advancedFrame.GetFrame())
        self._miscFrame.Create()
        self._miscFrame.SetLabelText("Misc.")
        self._miscFrame.CollapseFrame()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._miscFrame.GetWidgetName())        

        self._labelsEntry.SetParent(self._miscFrame.GetFrame())
        self._labelsEntry.Create()
        self._labelsEntry.SetLabelText("Labels:")
        self._labelsEntry.SetLabelWidth(20)
        self._labelsEntry.SetBalloonHelpString("Set the labels to generate Atlases for.")
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._labelsEntry.GetWidgetName())

        self._saveTransformsCheckBox.SetParent(self._miscFrame.GetFrame())
        self._saveTransformsCheckBox.Create()
        self._saveTransformsCheckBox.SetLabelText("Save Transforms:")
        self._saveTransformsCheckBox.GetWidget().SetSelectedState(1)
        self._saveTransformsCheckBox.SetLabelWidth(20)
        self._saveTransformsCheckBox.SetBalloonHelpString("Save all generated transforms to the Output Directory.")
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._saveTransformsCheckBox.GetWidgetName())
        
        self._normalizeAtlasCheckBox.SetParent(self._miscFrame.GetFrame())
        self._normalizeAtlasCheckBox.Create()
        self._normalizeAtlasCheckBox.SetLabelText("Normalize Atlases:")
        self._normalizeAtlasCheckBox.GetWidget().SetSelectedState(0)
        self._normalizeAtlasCheckBox.SetLabelWidth(20)
        self._normalizeAtlasCheckBox.SetBalloonHelpString("Normalize all Atlases to a given value. If enabled, the outputCast will be Float.")        
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._normalizeAtlasCheckBox.GetWidgetName())

        self._normalizeValueEntry.SetParent(self._miscFrame.GetFrame())
        self._normalizeValueEntry.Create()
        self._normalizeValueEntry.SetLabelText("Normalize to:")
        self._normalizeValueEntry.GetWidget().SetValue("1")
        self._normalizeValueEntry.SetLabelWidth(20)
        self._normalizeValueEntry.SetEnabled(0)
        self._normalizeValueEntry.SetBalloonHelpString("Set the value to normalize all Atlases to.")
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._normalizeValueEntry.GetWidgetName())

        self._outputCastCombo.SetParent(self._miscFrame.GetFrame())
        self._outputCastCombo.Create()
        self._outputCastCombo.GetWidget().ReadOnlyOn()
        self._outputCastCombo.SetLabelText("Output cast for Atlases:")
        self._outputCastCombo.SetLabelWidth(20)
        self._outputCastCombo.SetBalloonHelpString("The output cast for the Atlases.")
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

        self._deleteAlignedImagesCheckBox.SetParent(self._miscFrame.GetFrame())
        self._deleteAlignedImagesCheckBox.Create()
        self._deleteAlignedImagesCheckBox.SetLabelText("Delete Aligned Images:")
        self._deleteAlignedImagesCheckBox.GetWidget().SetSelectedState(1)
        self._deleteAlignedImagesCheckBox.SetLabelWidth(20)
        self._deleteAlignedImagesCheckBox.SetBalloonHelpString("If selected, all aligned images will be deleted after use.")        
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._deleteAlignedImagesCheckBox.GetWidgetName())

        self._deleteAlignedSegmentationsCheckBox.SetParent(self._miscFrame.GetFrame())
        self._deleteAlignedSegmentationsCheckBox.Create()
        self._deleteAlignedSegmentationsCheckBox.SetLabelText("Delete Aligned Segs.:")
        self._deleteAlignedSegmentationsCheckBox.GetWidget().SetSelectedState(1)
        self._deleteAlignedSegmentationsCheckBox.SetLabelWidth(20)
        self._deleteAlignedSegmentationsCheckBox.SetBalloonHelpString("If selected, all aligned segmentations will be deleted after use.")        
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._deleteAlignedSegmentationsCheckBox.GetWidgetName())        
        
        self._debugCheckBox.SetParent(self._miscFrame.GetFrame())
        self._debugCheckBox.Create()
        self._debugCheckBox.SetLabelText("Debug Output:")
        self._debugCheckBox.GetWidget().SetSelectedState(0)
        self._debugCheckBox.SetLabelWidth(20)
        self._debugCheckBox.SetBalloonHelpString("If selected, print debug output to the console.")
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._debugCheckBox.GetWidgetName())
        
        self._dryrunCheckBox.SetParent(self._miscFrame.GetFrame())
        self._dryrunCheckBox.Create()
        self._dryrunCheckBox.SetLabelText("Dry-Run (Simulation):")
        self._dryrunCheckBox.GetWidget().SetSelectedState(0)
        self._dryrunCheckBox.SetLabelWidth(20)
        self._dryrunCheckBox.SetBalloonHelpString("If selected, no computation is really executed - only commands are printed to the console.")        
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._dryrunCheckBox.GetWidgetName())
        # ending Misc. Frame
        
        # ending Advanced Frame


        self._generateButton.SetParent(self._moduleFrame.GetFrame())
        self._generateButton.Create()
        self._generateButton.SetEnabled(1)
        self._generateButton.SetText("Start!")
        self._generateButton.SetBalloonHelpString("Click to generate the atlases! After computation, they will be loaded into 3D Slicer.")
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
    
