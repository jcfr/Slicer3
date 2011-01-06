from SlicerScriptedModule import ScriptedModuleGUI
from Slicer import slicer
from operator import itemgetter, attrgetter
import glob
import os

from AtlasCreatorHelper import AtlasCreatorHelper
from AtlasCreatorLogic import AtlasCreatorLogic

vtkKWPushButton_InvokedEvent = 10000

vtkKWFileBrowserDialog_FileNameChangedEvent = 15000

vtkMRMLScene_CloseEvent = 66003

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

        self._secondFrame = slicer.vtkSlicerModuleCollapsibleFrame()

        self._defaultCaseEntry = slicer.vtkKWComboBoxWithLabel()

        self._regTypeRadios = slicer.vtkKWRadioButtonSetWithLabel()

        self._saveTransformCheckBox = slicer.vtkKWCheckButtonWithLabel()
        self._saveDeformationFieldCheckBox = slicer.vtkKWCheckButtonWithLabel()

        self._thirdFrame = slicer.vtkSlicerModuleCollapsibleFrame()
        self._labelEntry = slicer.vtkKWEntryWithLabel()
        self._generateButton = slicer.vtkKWPushButton()

        self._helper = AtlasCreatorHelper(self)

        self._logic = AtlasCreatorLogic(self)

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

            for nrrdFile in nrrdFiles:
                caseFile = os.path.basename(nrrdFile)
                if os.path.isfile(os.path.join(self._segDirButton.GetWidget().GetFileName(),caseFile)):
                    # file exists in originals and segmentations directory, so we can add it to our list of cases
                    self._defaultCaseEntry.GetWidget().AddValue(caseFile)
                    self._defaultCaseEntry.GetWidget().SetValue(caseFile)

    def GenerateAtlas(self):
        # call the logic
        result = self._logic.GenerateAtlas(self._origDirButton.GetWidget().GetFileName(),
                                                 self._segDirButton.GetWidget().GetFileName(),
                                                 self._outDirButton.GetWidget().GetFileName(),
                                                 self._defaultCaseEntry.GetWidget().GetValue(),
                                                 self._regTypeRadios.GetWidget().GetWidget(0).GetSelectedState(),
                                                 self._saveTransformCheckBox.GetWidget().GetSelectedState(),
                                                 self._saveDeformationFieldCheckBox.GetWidget().GetSelectedState(),
                                                 self._labelEntry.GetWidget().GetValue())

        if result:

            import os
            newVolNode = slicer.VolumesGUI.GetLogic().AddArchetypeScalarVolume(os.path.normpath(self._outDirButton.GetWidget().GetFileName()+"/atlas.nrrd"),'Atlas')

            displayNode = newVolNode.GetDisplayNode()
            if displayNode:
                newDisplayNode = displayNode.NewInstance()
                newDisplayNode.Copy(displayNode)
                slicer.MRMLScene.AddNodeNoNotify(newDisplayNode)
                newVolNode.SetAndObserveDisplayNodeID(newDisplayNode.GetID())
                newDisplayNode.AutoWindowLevelOff()
                newDisplayNode.AutoWindowLevelOn()


            selectionNode = slicer.ApplicationLogic.GetSelectionNode()
            selectionNode.SetReferenceActiveVolumeID(newVolNode.GetID())
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

    def ProcessMRMLEvents(self,caller,event):

        if caller == self.GetLogic().GetMRMLScene() and event == vtkMRMLScene_CloseEvent:
            self.OnSceneClose()

        elif caller == self.GetScriptedModuleNode():
            self.UpdateGUI()


    def BuildGUI(self):

        self.GetUIPanel().AddPage("AtlasCreator","AtlasCreator","")
        self._atlascreatorPage = self.GetUIPanel().GetPageWidget("AtlasCreator")
        helpText = "**A simple Atlas Creator**, developed by Daniel Haehn."
        aboutText = "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See http://www.slicer.org for details."
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
        self._segDirButton.SetLabelText("Manual segmentations:")
        self._segDirButton.GetWidget().GetLoadSaveDialog().ChooseDirectoryOn()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._segDirButton.GetWidgetName())

        self._segDirButton.GetWidget().GetLoadSaveDialog().Create()

        self._outDirButton.SetParent(self._topFrame.GetFrame())
        self._outDirButton.Create()
        self._outDirButton.GetWidget().SetText("Click to pick a directory")
        self._outDirButton.SetLabelText("Output directory:")
        self._outDirButton.GetWidget().GetLoadSaveDialog().ChooseDirectoryOn()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._outDirButton.GetWidgetName())

        self._secondFrame.SetParent(self._moduleFrame.GetFrame())
        self._secondFrame.Create()
        self._secondFrame.SetLabelText("Registration")
        self._secondFrame.ExpandFrame()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._secondFrame.GetWidgetName())

        self._defaultCaseEntry.SetParent(self._secondFrame.GetFrame())
        self._defaultCaseEntry.Create()
        self._defaultCaseEntry.GetWidget().ReadOnlyOn()
        self._defaultCaseEntry.SetLabelText("Default case:")
        self._defaultCaseEntry.SetBalloonHelpString("The filename of the default case used for registration.")
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._defaultCaseEntry.GetWidgetName())

        self._regTypeRadios.SetParent(self._secondFrame.GetFrame())
        self._regTypeRadios.Create()
        self._regTypeRadios.SetLabelText("Registration Type:")

        affineRadio = self._regTypeRadios.GetWidget().AddWidget(0)
        affineRadio.SetText("Affine")

        nonRigidRadio = self._regTypeRadios.GetWidget().AddWidget(1)
        nonRigidRadio.SetText("Non-Rigid")
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._regTypeRadios.GetWidgetName())

        self._regTypeRadios.GetWidget().GetWidget(0).SetSelectedState(1)

        self._saveTransformCheckBox.SetParent(self._secondFrame.GetFrame())
        self._saveTransformCheckBox.Create()
        self._saveTransformCheckBox.SetLabelText("Save Transforms:")
        self._saveTransformCheckBox.GetWidget().SetSelectedState(1)
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._saveTransformCheckBox.GetWidgetName())

        self._saveDeformationFieldCheckBox.SetParent(self._secondFrame.GetFrame())
        self._saveDeformationFieldCheckBox.Create()
        self._saveDeformationFieldCheckBox.SetLabelText("Save Deformation Fields:")
        self._saveDeformationFieldCheckBox.GetWidget().SetSelectedState(1)
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._saveDeformationFieldCheckBox.GetWidgetName())

        self._thirdFrame.SetParent(self._moduleFrame.GetFrame())
        self._thirdFrame.Create()
        self._thirdFrame.SetLabelText("Atlas Generation")
        self._thirdFrame.ExpandFrame()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._thirdFrame.GetWidgetName())

        self._labelEntry.SetParent(self._thirdFrame.GetFrame())
        self._labelEntry.Create()
        self._labelEntry.SetLabelText("Label Map Values:")
        self._labelEntry.GetWidget().SetValue("3 4 5 6 7 8 9 10 31 32")
        self._labelEntry.SetBalloonHelpString("The label map values of the manual segmentation.")
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._labelEntry.GetWidgetName())

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
