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
    variable CheckButtonSize 3
    variable VolumeMenuButtonSize 0
    variable TextEntrySize 0

    # Check Button
    variable atlasAlignedFlagID 0
    variable inhomogeneityCorrectionFlagID 1
    variable performSkullStrippingFlagID 2

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
        variable performSkullStrippingFlagID
        variable iccMaskSelectID
        variable LOGIC

        # Always has to be done initially so that variables are correctly defined
        if { [InitVariables] } {
            puts stderr "ERROR: MRI Human Brain: ShowUserInterface: Not all variables are correctly defined!"
            return 1
        }
        $LOGIC PrintText  "TCLMRI: Preprocessing MRI Human Brain - ShowUserInterface"

        $preGUI DefineTextLabel "This task only applies to skull stripped scans!\n\nShould the EMSegmenter " 0
        $preGUI DefineCheckButton " - register the atlas to the input scan ?" 0 $atlasAlignedFlagID
        # $preGUI DefineVolumeMenuButton "Define ICC mask of the atlas ?" 0 $iccMaskSelectID
        $preGUI DefineCheckButton " - perform image inhomogeneity correction on input scan ?" 0 $inhomogeneityCorrectionFlagID
        $preGUI DefineCheckButton " - perform skull stripping on input scan ?" 0 $performSkullStrippingFlagID

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
        variable performSkullStrippingFlagID
        variable iccMaskSelectID

        $LOGIC PrintText "TCLMRI: =========================================="
        $LOGIC PrintText "TCLMRI: == Preprocress Data"
        $LOGIC PrintText "TCLMRI: =========================================="

        set atlasAlignedFlag [ GetCheckButtonValueFromMRML $atlasAlignedFlagID ]
        set inhomogeneityCorrectionFlag [ GetCheckButtonValueFromMRML $inhomogeneityCorrectionFlagID ]
        set performSkullStrippingFlag [ GetCheckButtonValueFromMRML $performSkullStrippingFlagID ]

        # ---------------------------------------
        # Step 1 : Initialize/Check Input
        if {[InitPreProcessing]} {
             $LOGIC PrintText "TCLMRI: == Problem with init"
            return 1
        }
        # at this point the input volumes are aligned to the first volume



        # if not stripped
        if { $performSkullStrippingFlag } {


            $LOGIC PrintText "TCLMRI: =========================================="
            $LOGIC PrintText "TCLMRI: == Skull Stripping"
            $LOGIC PrintText "TCLMRI: =========================================="
            #TODO
            set maskNode -1
            set nonSkullStrippedAtlasVolumeNode -1
#            set tmpSCENE [$mrmlManager GetMRMLScene]
#            set nonSkullStrippedAtlasVolumeNode [$tmpSCENE GetNodesByName atlas_t1]
            set skullstrippedNodeList [BRAINSSkullStripper $alignedTargetNode $nonSkullStrippedAtlasVolumeNode $maskNode]

            if { $skullstrippedNodeList == "" } {
                PrintError "Run: BRAINSSkullStripper failed !"
                return 1
            }
            if { [UpdateVolumeCollectionNode "$alignedTargetNode" "$skullstrippedNodeList"] } {
                PrintError "Run: Update of BRAINSSkullStripper failed !"
                return 1
            }
        }

        # ----------------------------------------------------------------------------
        # We have to create this function so that we can run it in command line mode
        #
        set iccMaskVTKID 0
        # [GetVolumeMenuButtonValueFromMRML $iccMaskSelectID]

        $LOGIC PrintText "TCLMRI: ==> Preprocessing Setting: $atlasAlignedFlag $inhomogeneityCorrectionFlag $performSkullStrippingFlag"


        if { $iccMaskVTKID } {
            set inputAtlasICCMaskNode [$mrmlManager GetVolumeNode $iccMaskVTKID]
            if { $inputAtlasICCMaskNode == "" } {
                PrintError "Run: inputAtlasICCMaskNode is not defined"
                return 1
            }
        } else {
            set inputAtlasICCMaskNode ""
        }

        # -------------------------------------
        # Step 2: Generate ICC Mask Of input images
        if { $inputAtlasICCMaskNode != "" && 0} {
            set inputAtlasVolumeNode [$inputAtlas GetNthVolumeNode 0]
            set subjectVolumeNode [$alignedTargetNode GetNthVolumeNode 0]

            set subjectICCMaskNode [GenerateICCMask $inputAtlasVolumeNode $inputAtlasICCMaskNode $subjectVolumeNode]

            if { $subjectICCMaskNode == "" } {
                PrintError "Run: Generating ICC mask for Input failed!"
                return 1
            }
        } else {
            #  $LOGIC PrintText "TCLMRI: Skipping ICC Mask generation! - Not yet implemented"
            set subjectICCMaskNode ""
        }

        # -------------------------------------
        # Step 4: Perform Intensity Correction
        if { $inhomogeneityCorrectionFlag == 1 } {

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
            set MSG "Log Covariance matrices for the following classes seemed incorrect:\n "
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

        $inputChannelGUI DefineTextLabel "Please insure that input scans are skull stripped!\n\nShould the EMSegmenter " 0
        $inputChannelGUI DefineCheckButton " - perform image inhomogeneity correction on input scans ?" 0 $EMSegmenterPreProcessingTcl::inhomogeneityCorrectionFlagID
        $inputChannelGUI DefineCheckButton " - perform skull stripping on input scan ?" 0 $EMSegmenterPreProcessingTcl::performSkullStrippingFlagID

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

