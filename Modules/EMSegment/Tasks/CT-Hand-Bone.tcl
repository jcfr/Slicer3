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
    variable TextEntrySize 0
    variable UseBRAINS 1

    # Check Button
    variable atlasAlignedFlagID 0
    variable inhomogeneityCorrectionFlagID 1

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
        variable iccMaskSelectID
        variable inhomogeneityCorrectionFlagID
        variable LOGIC

        # Always has to be done initially so that variables are correctly defined
        if { [InitVariables] } {
            PrintError "ERROR: MRI-HumanBrain: ShowUserInterface: Not all variables are correctly defined!"
            return 1
        }
        $LOGIC PrintText "TCLMRI: Preprocessing MRI Human Brain - ShowUserInterface"

        $preGUI DefineTextLabel "This task only applies to non-skull stripped scans! \n\nShould the EMSegmenter " 0
        $preGUI DefineCheckButton "- register the atlas to the input scan ?" 0 $atlasAlignedFlagID
        # $preGUI DefineCheckButton "Are the input scans skull stripped ?" 0 $skullStrippedFlagID
        # $preGUI DefineVolumeMenuButton "Define ICC mask of the atlas ?" 0 $iccMaskSelectID
        $preGUI DefineCheckButton "- perform image inhomogeneity correction on input scan ?" 0 $inhomogeneityCorrectionFlagID

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
        variable subjectNode
        variable inputAtlasNode
        variable mrmlManager
        variable LOGIC

        variable atlasAlignedFlagID
        variable iccMaskSelectID
        variable inhomogeneityCorrectionFlagID

        $LOGIC PrintText "TCLMRI: =========================================="
        $LOGIC PrintText "TCLMRI: == Preprocress Data"
        $LOGIC PrintText "TCLMRI: =========================================="
        # ---------------------------------------
        # Step 1 : Initialize/Check Input
        if {[InitPreProcessing]} {
            return 1
        }

        set inputTargetNode [$workingDN GetInputTargetNode]
        set alignedTargetNode [$workingDN GetAlignedTargetNode]
        set inputAtlasNode [$workingDN GetInputAtlasNode]
        set alignedAtlasNode [$workingDN GetAlignedAtlasNode]

        if { $inputTargetNode != "" } {
            $LOGIC PrintText "Detected [$inputTargetNode GetNumberOfVolumes] inputTargetNodeVolumes"
        }
        if { $alignedTargetNode != "" } {
            $LOGIC PrintText "Detected [$alignedTargetNode GetNumberOfVolumes] alignedTargetNodeVolumes"
        }
        if { $inputAtlasNode != "" } {
            $LOGIC PrintText "Detected [$inputAtlasNode GetNumberOfVolumes] inputAtlasNodeVolumes"
        } 
        if { $alignedAtlasNode != "" } {
             $LOGIC PrintText "Detected [$alignedAtlasNode GetNumberOfVolumes] alignedAtlasNodeVolumes"
        }

        if { $alignedAtlasNode == "" } {
            $LOGIC PrintText "TCL: Aligned Atlas was empty"
            set alignedAtlasNode [ $mrmlManager CloneAtlasNode $inputAtlasNode "Aligned"]
            $workingDN SetReferenceAlignedAtlasNodeID [$alignedAtlasNode GetID]
        } else {
            $LOGIC PrintText "TCL: Atlas was just synchronized"
            $mrmlManager SynchronizeAtlasNode $inputAtlasNode $alignedAtlasNode "Aligned"
        }


        # (flip), fiducial_threshold, blur, binarize, choose largest component of: atlas and subject
        # RegisterHandAtlas
        


        set inputTargetVolumeNode [$inputTargetNode GetNthVolumeNode 0]
        set inputTargetVolumeFileName [WriteDataToTemporaryDir $inputTargetVolumeNode Volume]

        set blurredInputTargetVolumeFileName [CreateTemporaryFileName $inputTargetVolumeNode]
        CTHandBonePipeline $inputTargetVolumeFileName $blurredInputTargetVolumeFileName
 


        set atlasRegistrationVolumeIndex -1;
        if {[[$mrmlManager GetGlobalParametersNode] GetRegistrationAtlasVolumeKey] != "" } {
            set atlasRegistrationVolumeKey [[$mrmlManager GetGlobalParametersNode] GetRegistrationAtlasVolumeKey]
            set atlasRegistrationVolumeIndex [$inputAtlasNode GetIndexByKey $atlasRegistrationVolumeKey]
        }

        if {$atlasRegistrationVolumeIndex < 0 } {
            PrintError "RegisterAtlas: Attempt to register atlas image but no atlas image selected!"
            return 1
        }

        set inputAtlasVolumeNode [$inputAtlasNode GetNthVolumeNode $atlasRegistrationVolumeIndex]
        set inputAtlasVolumeFileName [WriteDataToTemporaryDir $inputAtlasVolumeNode Volume]

        set blurredInputAtlasVolumeFileName [CreateTemporaryFileName $inputAtlasVolumeNode]
        CTHandBonePipeline $inputAtlasVolumeFileName $blurredInputAtlasVolumeFileName




#        set transformfile RegisterAtlasToSubject { $outputAtlasFileName $outputFileName }

        set PLUGINS_DIR "[$::slicer3::Application GetPluginsDir]"
        set CMD "${PLUGINS_DIR}/BRAINSFit"

        set fixedVolumeFileName $blurredInputTargetVolumeFileName
        set CMD "$CMD --fixedVolume $fixedVolumeFileName"

        set movingVolumeFileName $blurredInputAtlasVolumeFileName
        set CMD "$CMD --movingVolume $movingVolumeFileName"

        set outputVolumeFileName "/tmp/outputvolume_registered.nrrd"
        set CMD "$CMD --outputVolume $outputVolumeFileName"

        set linearTransform "/tmp/Subject_Transform.txt"
        set CMD "$CMD --outputTransform $linearTransform"

        set CMD "$CMD --initializeTransformMode useMomentsAlign --transformType Rigid,Affine"

        $LOGIC PrintText "TCL: Executing $CMD"
        catch { eval exec $CMD } errmsg
        $LOGIC PrintText "TCL: $errmsg"



        set CMD "${PLUGINS_DIR}/BRAINSDemonWarp"
        set CMD "$CMD -m $movingVolumeFileName -f $fixedVolumeFileName"
        set CMD "$CMD --initializeWithTransform $linearTransform"
        set oArgument "/tmp/Subject_demons.nii"
        set deformationfield "/tmp/AtoS_deformationfield.nii"
        set CMD "$CMD -o $oArgument -O $deformationfield"

        set CMD "$CMD -i 1000,500,250,125,60 -n 5 -e --numberOfMatchPoints 16"
        # fast - for debugging
        # set CMD "$CMD -i 1,5,2,1,1 -n 5 -e --numberOfMatchPoints 16"

        $LOGIC PrintText "TCL: Executing $CMD"
        catch { eval exec $CMD } errmsg
        $LOGIC PrintText "TCL: $errmsg"


        set fixedTargetChannel 0
        set fixedTargetVolumeNode [$subjectNode GetNthVolumeNode $fixedTargetChannel]
        set fixedTargetVolumeFileName [WriteImageDataToTemporaryDir $fixedTargetVolumeNode]

        for { set i 0 } { $i < [$alignedAtlasNode GetNumberOfVolumes] } { incr i } {
#            if { $i == $atlasRegistrationVolumeIndex} { continue }
            $LOGIC PrintText "TCL: Resampling atlas image $i ..."
            set inputAtlasVolumeNode [$inputAtlasNode GetNthVolumeNode $i]
            set outputAtlasVolumeNode [$alignedAtlasNode GetNthVolumeNode $i]
            set backgroundLevel [$LOGIC GuessRegistrationBackgroundLevel $inputAtlasVolumeNode]
            $LOGIC PrintText "TCL: Guessed background level: $backgroundLevel"

            set inputAtlasVolumeFileName [WriteImageDataToTemporaryDir $inputAtlasVolumeNode]
            set outputAtlasVolumeFileName [WriteImageDataToTemporaryDir $outputAtlasVolumeNode]

            set CMD "${PLUGINS_DIR}/BRAINSResample"
            set CMD "$CMD --inputVolume $inputAtlasVolumeFileName  --referenceVolume $fixedTargetVolumeFileName"
            set CMD "$CMD --outputVolume $outputAtlasVolumeFileName --deformationVolume $deformationfield"

            set CMD "$CMD --pixelType"
            set fixedTargetVolume [$fixedTargetVolumeNode GetImageData]
            set scalarType [$fixedTargetVolume GetScalarTypeAsString]
            switch -exact "$scalarType" {
                "bit" { set CMD "$CMD binary" }
                "unsigned char" { set CMD "$CMD uchar" }
                "unsigned short" { set CMD "$CMD ushort" }
                "unsigned int" { set CMD "$CMD uint" }
                "short" -
                "int" -
                "float" { set CMD "$CMD $scalarType" }
                default {
                    PrintError "BRAINSResample: cannot resample a volume of type $scalarType"
                    return 1
                }
            }


            $LOGIC PrintText "TCL: Executing $CMD"
            catch { eval exec $CMD } errmsg
            $LOGIC PrintText "TCL: $errmsg"

            ReadDataFromDisk $outputAtlasVolumeNode $outputAtlasVolumeFileName Volume
            file delete -force $outputAtlasVolumeFileName
        }

#       ComputeIntensityDistributions

        return 0

        #TODO, return value
        #        if { $ret == "-1" }
        #       {
        #          $LOGIC PrintText "blurring failed, but keep going"
        #       }

        #  int blur(const char* inputImage, const char* outputImage, const double gaussianVariance, const unsigned int maxKernelWidth);
        #  int binary_threshold(const char* inputDirectory, const char* outputThresholdImage, const unsigned short _lowerThreshold, const unsigned short _upperThreshold);
        #  int flip( const char* inputFile, const char* outputFlippedImageFile, int flipAxisX, int flipAxisY, int flipAxisZ );
        #  int fiducial_threshold(const char* inputImage, const char* outputImage, const char* fiducialfile, const char* logfile );
        #  int largest_component( const char* inputImage, const char* outputImage );


#        if { $alignedTargetNode == "" } {
#            # input scan does not have to be aligned
#            set alignedTargetNode [$mrmlManager CloneTargetNode $inputTargetNode "Aligned"]
#            $workingDN SetReferenceAlignedTargetNodeID [$alignedTargetNode GetID]
#        } else {
#            $mrmlManager SynchronizeTargetNode $inputTargetNode $alignedTargetNode "Aligned"
#        }

#        for { set i 0 } { $i < [$alignedTargetNode GetNumberOfVolumes] } {incr i} {
#             $LOGIC PrintText "read $i th alignedTargetNode"
#            set intputVolumeNode($i) [$inputTargetNode GetNthVolumeNode $i]
#            if { $intputVolumeNode($i) == "" } {
#                PrintError "RegisterInputImages: the ${i}th input node is not defined!"
#                return 1
#            }
#        }


        # ----------------------------------------------------------------------------
        # We have to create this function so that we can run it in command line mode
        #
        set atlasAlignedFlag [ GetCheckButtonValueFromMRML $atlasAlignedFlagID ]
        set skullStrippedFlag 0
        set iccMaskVTKID 0
        # [GetVolumeMenuButtonValueFromMRML $iccMaskSelectID]
        set inhomogeneityCorrectionFlag [GetCheckButtonValueFromMRML $inhomogeneityCorrectionFlagID]

        $LOGIC PrintText "TCLMRI: ==> Preprocessing Setting: $atlasAlignedFlag $inhomogeneityCorrectionFlag"

        if { ($atlasAlignedFlag == 0) && ($skullStrippedFlag == 1) } {
            PrintError "Run: We currently cannot align the atlas to skull stripped image"
            return 1
        }

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
            set subjectVolumeNode [$subjectNode GetNthVolumeNode 0]

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
            if { [UpdateSubjectNode "$subjectIntensityCorrectedNodeList"] } {
                return 1
            }
        } else {
            $LOGIC PrintText "TCLMRI: Skipping intensity correction"
        }

        # write results over to subjectNode

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
            $preGUI PopUpWarningWindow "$MSG"
        }
        return 0
    }

    #
    # TASK SPECIFIC FUNCTIONS
    #
    proc CTHandBonePipeline { inputFileName outputFileNameX } {

        set CTHandBoneHelper [vtkCTHandBoneClass New]

#        set outputFileName "/tmp/Target_Flip.nii"
#        set ret [$CTHandBoneHelper flip $inputFileName $outputFileName "1" "0" "0"]
#
#        set inputFileName $outputFileName
        set outputFileName "/tmp/Target_Flip_Threshold.nii"
        #TODO
        set fiducialfile "/projects/sandbox/Slicer3/trunk/Slicer3/Modules/EMSegment/Tasks/CT-Hand-Bone-Code/FiducialsSubject2.fcsv"
        #TODO
        set logfile "/tmp/logfile.txt"
        set ret [$CTHandBoneHelper fiducial_threshold $inputFileName $outputFileName $fiducialfile $logfile]

        set inputFileName $outputFileName
        set outputFileName "/tmp/Target_Flip_Threshold_Blur.nii"
        set ret [$CTHandBoneHelper blur $inputFileName $outputFileName "1.5" "5"]
         $LOGIC PrintText "binary..."
        set inputFileName $outputFileName
        set outputFileName "/tmp/Target_Flip_Threshold_Blur_Binary.nii"
        set ret [$CTHandBoneHelper binary_threshold $inputFileName $outputFileName "0" "30"]
        $LOGIC PrintText "laregest..."
        set inputFileName $outputFileName
#        set outputFileName "/tmp/Target_Flip_Threshold_Blur_Binary_Largest.nii"
        set ret [$CTHandBoneHelper largest_component $inputFileName $outputFileNameX]
        $LOGIC PrintText "atlas template..."

        $CTHandBoneHelper Delete

    }
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

        $inputChannelGUI DefineTextLabel "Is the subject right handed?" 0
        $inputChannelGUI DefineCheckButton "Perform image inhomogeneity correction on input scans ?" 0 $EMSegmenterPreProcessingTcl::inhomogeneityCorrectionFlagID

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
