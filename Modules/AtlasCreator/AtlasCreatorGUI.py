from SlicerScriptedModule import ScriptedModuleGUI
from Slicer import slicer

from AtlasCreatorHelper import AtlasCreatorHelper
from AtlasCreatorLogic import AtlasCreatorLogic

vtkKWPushButton_InvokedEvent = 10000

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

        self._defaultCaseEntry = slicer.vtkKWEntryWithLabel()

        self._regTypeRadios = slicer.vtkKWRadioButtonSetWithLabel()

        self._saveTransformCheckBox = slicer.vtkKWCheckButtonWithLabel()
        self._saveDeformationFieldCheckBox = slicer.vtkKWCheckButtonWithLabel()

        self._thirdFrame = slicer.vtkSlicerModuleCollapsibleFrame()
        self._labelEntry = slicer.vtkKWEntryWithLabel()
        self._outVolumeSelector = slicer.vtkSlicerNodeSelectorWidget()
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


    def RemoveGUIObservers(self):
        pass

    def ProcessGUIEvents(self,caller,event):
        if not self._updating:

            if caller == self._generateButton and event == vtkKWPushButton_InvokedEvent:
                self.GenerateAtlas()

          
    def GenerateAtlas(self):
        # call the logic
        tmpImageData = self._logic.GenerateAtlas(self._origDirButton.GetWidget().GetFileName(),self._segDirButton.GetWidget().GetFileName(),self._outDirButton.GetWidget().GetFileName(),self._defaultCaseEntry.GetWidget().GetValue(),self._regTypeRadios.GetWidget().GetWidget(0).GetSelectedState(), self._saveTransformCheckBox.GetWidget().GetSelectedState(),self._saveDeformationFieldCheckBox.GetWidget().GetSelectedState(),self._labelEntry.GetWidget().GetValue())

        if tmpImageData:

            atlas = slicer.vtkImageData()
            atlas.DeepCopy(tmpImageData)

            # create an output node, if not existent
            self.CreateOutVolumeNodes()

            # save our atlas to the node
            outVolume = self._outVolumeSelector.GetSelected()
            outVolume.SetAndObserveImageData(atlas)
            outVolume.SetModifiedSinceRead(1)

                  
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
        self._topFrame.SetLabelText("Input")
        self._topFrame.ExpandFrame()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._topFrame.GetWidgetName())

        self._origDirButton.SetParent(self._topFrame.GetFrame())
        self._origDirButton.Create()
        self._origDirButton.GetWidget().SetText("Click to pick a directory")
        self._origDirButton.SetLabelText("Original images:")
        self._origDirButton.GetWidget().GetLoadSaveDialog().ChooseDirectoryOn()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._origDirButton.GetWidgetName())

        self._segDirButton.SetParent(self._topFrame.GetFrame())
        self._segDirButton.Create()
        self._segDirButton.GetWidget().SetText("Click to pick a directory")
        self._segDirButton.SetLabelText("Manual segmentations:")
        self._segDirButton.GetWidget().GetLoadSaveDialog().ChooseDirectoryOn()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2" % self._segDirButton.GetWidgetName())

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
        self._defaultCaseEntry.SetLabelText("Default case:")
        self._defaultCaseEntry.GetWidget().SetValue("")
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

        self._outVolumeSelector.SetNodeClass("vtkMRMLScalarVolumeNode","","1","AtlasOutput")
        self._outVolumeSelector.SetNewNodeEnabled(1)
        self._outVolumeSelector.SetParent(self._thirdFrame.GetFrame())
        self._outVolumeSelector.Create()
        self._outVolumeSelector.SetMRMLScene(self.GetLogic().GetMRMLScene())
        self._outVolumeSelector.UpdateMenu()
        self._outVolumeSelector.SetBorderWidth(2)
        self._outVolumeSelector.SetLabelText("Atlas Output Volume: ")
        self._outVolumeSelector.SetBalloonHelpString("select an output volume from the current mrml scene.")
        slicer.TkCall("pack %s -side top -anchor e -padx 20 -pady 4" % self._outVolumeSelector.GetWidgetName())

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
