package require Itcl

#########################################################
#
if {0} { ;# comment

    This is function is executed by EMSegmenter

    # TODO :

}
#
#########################################################

#
# namespace procs
#

#
# Remember to source first GenericTask.tcl as it has all the variables/basic structure defined
#
namespace eval EMSegmenterPreProcessingTcl {

    #
    # Variables Specific to this Preprocessing
    #
    variable TextLabelSize 1
    variable CheckButtonSize 2
    variable VolumeMenuButtonSize 0
    variable TextEntrySize 3

    # Check Button
    variable atlasAlignedFlagID 0
    variable inhomogeneityCorrectionFlagID 1
    variable ac_imagesDirTextID 0
    variable ac_segmentationsDirTextID 1
    variable ac_outputDirTextID 2

    # Text Entry
    # not defined for this task

    #
    # OVERWRITE DEFAULT
    #

    # -------------------------------------
    # Define GUI
    # return 1 when error occurs
    # -------------------------------------
    proc ShowUserInterface { } {
        variable preGUI
        variable atlasAlignedFlagID
        variable inhomogeneityCorrectionFlagID
        variable ac_imagesDirTextID
        variable ac_segmentationsDirTextID
        variable ac_outputDirTextID
        variable LOGIC

        # Always has to be done initially so that variables are correctly defined
        if { [InitVariables] } {
            puts stderr "ERROR: MRI Human Brain: ShowUserInterface: Not all variables are correctly defined!"
            return 1
        }
        $LOGIC PrintText  "TCLMRI: Preprocessing MRI Human Brain - ShowUserInterface"

        $preGUI DefineTextLabel "This task only applies to non-skull stripped scans! \n\nShould the EMSegmenter " 0
        $preGUI DefineCheckButton "- register the atlas to the input scan ?" 0 $atlasAlignedFlagID
        $preGUI DefineCheckButton "- perform image inhomogeneity correction on input scan ?" 0 $inhomogeneityCorrectionFlagID
        $preGUI DefineTextEntry "Enter path to images"             "../../" $ac_imagesDirTextID
        $preGUI DefineTextEntry "Enter path to segmentations"      "../../" $ac_segmentationsDirTextID
        $preGUI DefineTextEntry "Enter path to output directory"   "../../" $ac_outputDirTextID

        # Define this at the end of the function so that values are set by corresponding MRML node
        $preGUI SetButtonsFromMRML
    }

    # -------------------------------------
    # Define Preprocessing Pipeline
    # return 1 when error occurs
    # -------------------------------------
    proc Run { } {
        variable preGUI
        variable workingDN
        variable alignedTargetNode
        variable inputAtlasNode
        variable mrmlManager
        variable LOGIC

        variable atlasAlignedFlagID
        variable inhomogeneityCorrectionFlagID
        variable ac_imagesDirTextID
        variable ac_segmentationsDirTextID
        variable ac_outputDirTextID

        $LOGIC PrintText "TCLMRI: =========================================="
        $LOGIC PrintText "TCLMRI: == Preprocress Data"
        $LOGIC PrintText "TCLMRI: =========================================="

        set atlasAlignedFlag             [ GetCheckButtonValueFromMRML $atlasAlignedFlagID ]
        set inhomogeneityCorrectionFlag  [ GetCheckButtonValueFromMRML $inhomogeneityCorrectionFlagID ]
        set imagesDir                    [ GetTextEntryValueFromMRML   $ac_imagesDirTextID ]
        set segmentationsDir             [ GetTextEntryValueFromMRML   $ac_segmentationsDirTextID ]
        set outputDir                    [ GetTextEntryValueFromMRML   $ac_outputDirTextID ]

        # ---------------------------------------
        # Step 1 : Initialize/Check Input
        if {[InitPreProcessing]} {
            return 1
        }

        set Slicer3_HOME $::env(Slicer3_HOME)
        set template     "$Slicer3_HOME/../Slicer3/Modules/AtlasCreator/TestData/originals/case62.nrrd"

        set list [AtlasCreator $template $Slicer3_HOME/$segmentationsDir $Slicer3_HOME/$imagesDir $outputDir "-1"]
        puts $list


        #  virtual void      SetTreeNodeSpatialPriorVolumeID(vtkIdType nodeID, vtkIdType volumeID);
        #  virtual vtkIdType GetTreeNodeSpatialPriorVolumeID(vtkIdType nodeID);
#        set MOD [$::slicer3::Application GetModuleGUIByName "EMSegmenter"]
#        set mrmlManager [$MOD GetMRMLManager]
#        set node_id 1004
#        set vol_id [$mrmlManager GetTreeNodeSpatialPriorVolumeID $node_id]
#        puts "$node_id $vol_id"
#        set vol_id 1010
#        $mrmlManager SetTreeNodeSpatialPriorVolumeID $node_id $vol_id
#        set vol_id [$mrmlManager GetTreeNodeSpatialPriorVolumeID $node_id]
#        puts "$node_id $vol_id"

        # Atlas creator returns a directory with priors
        # loop through this directory and add volume nodes to the scene
        # for each leaf in the tree set/replace the atlasnode by the ac atlas node
        # how to find the right atlas volume?

        # In MRML it looks like:

        # <EMSAtlas
        #  id="vtkMRMLEMSAtlasNode1"  name="vtkMRMLEMSAtlasNode1"  hideFromEditors="false"  selectable="true"  selected="false"  NodeIDs="
        #Key vtkMRMLEMSTreeNode4 NodeID vtkMRMLScalarVolumeNode2
        #Key vtkMRMLEMSTreeNode3 NodeID vtkMRMLScalarVolumeNode1
        #Key vtkMRMLEMSTreeNode6 NodeID vtkMRMLScalarVolumeNode3
        #Key vtkMRMLEMSTreeNode7 NodeID vtkMRMLScalarVolumeNode4
        #Key vtkMRMLEMSTreeNode8 NodeID vtkMRMLScalarVolumeNode6
        #Key atlas_registration_image0 NodeID vtkMRMLScalarVolumeNode7"   NumberOfTrainingSamples="82"  ></EMSAtlas>
        #
#        $inputAtlasNode GetIndexByKey vtkMRMLEMSTreeNode3

#        $inputAtlasNode GetNodeIDByKey  vtkMRMLEMSTreeNode4
#        vtkMRMLScalarVolumeNode2
#        $mrmlManager MRMLNodeIDToVTKNodeIDMap vtkMRMLScalarVolumeNode2  ???

        #        set atlasRegistrationVolumeIndex -1;
        #         $LOGIC PrintText [[$mrmlManager GetGlobalParametersNode] GetRegistrationAtlasVolumeKey]
        #         $LOGIC PrintText [[$mrmlManager GetGlobalParametersNode] GetRegistrationAtlasVolumeKey]
        #         $LOGIC PrintText [$inputAtlasNode GetIndexByKey $atlasRegistrationVolumeKey]

        # ----------------------------------------------------------------------------
        # We have to create this function so that we can run it in command line mode
        #

        $LOGIC PrintText "TCLMRI: ==> Preprocessing Setting: $atlasAlignedFlag $inhomogeneityCorrectionFlag"


        # -------------------------------------
        # Step 4: Perform Intensity Correction
        if { $inhomogeneityCorrectionFlag == 1 } {
            set subjectICCMaskNode -1
            set subjectIntensityCorrectedNodeList [PerformIntensityCorrection $subjectICCMaskNode]
            if { $subjectIntensityCorrectedNodeList == "" } {
                PrintError "Run: Intensity Correction failed !"
                return 1
            }
            if { [UpdateVolumeCollectionNode "$alignedTargetNode" "$subjectIntensityCorrectedNodeList"] } {
                return 1
            }
        } else {
            $LOGIC PrintText "TCLMRI: Skipping intensity correction"
        }

        # write results over to alignedTargetNode

        # -------------------------------------
        # Step 5: Atlas Alignment - you will also have to include the masks
        # Defines $workingDN GetAlignedAtlasNode
        if { [RegisterAtlas $atlasAlignedFlag] } {
            PrintError "Run: Atlas alignment failed !"
            return 1
        }


        # -------------------------------------
        # Step 6: Perform autosampling to define intensity distribution
        if { [ComputeIntensityDistributions] } {
            PrintError "Run: Could not automatically compute intensity distribution !"
            return 1
        }

        # -------------------------------------
        # Step 7: Check validity of Distributions
        set failedIDList [CheckAndCorrectTreeCovarianceMatrix]
        if { $failedIDList != "" } {
            set MSG "Log Covariance matrices for the following classes seemed incorrect:\n"
            foreach ID $failedIDList {
                set MSG "${MSG}[$mrmlManager GetTreeNodeName $ID]\n"
            }
            set MSG "${MSG}This can cause failure of the automatic segmentation. To address the issue, please visit the web site listed under Help"

            if { 0 } {
                # TODO
                $preGUI PopUpWarningWindow "$MSG"
            } else {
                $LOGIC PrintText "TCLMRI: WARNING: $MSG"
            }
        }

        return 0
    }

    #
    # TASK SPECIFIC FUNCTIONS
    #

}


namespace eval EMSegmenterSimpleTcl {
    # 0 = Do not create a check list for the simple user interface
    # simply remove
    # 1 = Create one - then also define ShowCheckList and
    #     ValidateCheckList where results of checklist are transfered to Preprocessing

    proc CreateCheckList { } {
        return 1
    }

    proc ShowCheckList { } {
        variable inputChannelGUI
        # Always has to be done initially so that variables are correctly defined
        if { [InitVariables] } {
            PrintError "ShowCheckList: Not all variables are correctly defined!"
            return 1
        }

        $inputChannelGUI DefineTextLabel "Please insure that input scans are not skull stripped" 0
        $inputChannelGUI DefineCheckButton "Perform image inhomogeneity correction on input scans ?" 0 $EMSegmenterPreProcessingTcl::inhomogeneityCorrectionFlagID
        $inputChannelGUI DefineTextEntry "Enter path to images"             "../../" $EMSegmenterPreProcessingTcl::ac_imagesDirTextID
        $inputChannelGUI DefineTextEntry "Enter path to segmentations"      "../../" $EMSegmenterPreProcessingTcl::ac_segmentationsDirTextID
        $inputChannelGUI DefineTextEntry "Enter path to output directory"   "../../" $EMSegmenterPreProcessingTcl::ac_outputDirTextID

        # Define this at the end of the function so that values are set by corresponding MRML node
        $inputChannelGUI SetButtonsFromMRML
        return 0

    }

    proc ValidateCheckList { } {
        return 0
    }

    proc PrintError { TEXT } {
        puts stderr "TCLMRI: ERROR:EMSegmenterSimpleTcl::${TEXT}"
    }
}
