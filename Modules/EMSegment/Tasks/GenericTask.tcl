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
# This is the default processing pipeline - which does not do anything
#

namespace eval EMSegmenterPreProcessingTcl {

    #
    # Variables
    #

    ## Slicer
    variable GUI
    variable LOGIC
    variable SCENE

    ## EM GUI/MRML
    variable preGUI
    variable mrmlManager
    variable workingDN

    ## Input/Output
    variable inputAtlasNode
    # Variables used for segmentation
    # Input/Output target specific scans - by default this is defined by the input scans which are aligned with each other
    variable alignedTargetNode
    # spatial priors aligned to target node
    variable outputAtlasNode

    variable inputSubParcellationNode
    variable outputSubParcellationNode

    ## Task Specific GUI variables
    variable TextLabelSize 1
    variable CheckButtonSize 0
    variable VolumeMenuButtonSize 0
    variable TextEntrySize 0

    #
    # General Utility Functions
    #
    proc DeleteNode { NODE } {
        variable SCENE
        $SCENE RemoveNode $NODE
        # Note:
        #Do not need to do it as the destructor does it automatically
        #set displayNode [$NODE GetDisplayNode]
        #[$NODE GetDisplayNode]
        # if {$displayNode} { $SCENE RemoveNode $displayNode }
    }


    #------------------------------------------------------
    # returns filename when no error occurs
    proc CreateTemporaryFileNameForNode { Node } {
        variable GUI
        variable LOGIC

        # dry-run, XXXXXX cannot be in the middle of the name on some platforms (fc11)
        set CMD "mktemp -u \"[$GUI GetTemporaryDirectory]/XXXXXX\""
        set basefilename [ eval exec $CMD ]

        set filename ""
        set NAME ""

        if { [$Node GetClassName] == "vtkMRMLScalarVolumeNode" } {
            set NAME "_[$Node GetID].nrrd"
        } elseif { [$Node GetClassName] == "vtkMRMLScene"  } {
            set NAME "_[file tail [$Node GetURL]]"
        } else {
            #TODO,FIXME: need a elseif here
            # Transform node - check also for bspline
            set NAME "_[$Node GetID].mat"
        }

        if { $NAME != "" } {
            set filename $basefilename$NAME
            $LOGIC PrintText "TCL: Create file: $filename"
            set CMD "touch \"$filename\""
            eval exec $CMD
        } else {
            PrintError "Could not create file: $basefilename$NAME"
        }

        return $filename
    }


    #
    proc CreateFileName { type } {
        variable GUI
        variable LOGIC

        # dry-run, XXXXXX cannot be in the middle of the name on some platforms (fc11)
        set CMD "mktemp -u \"[$GUI GetTemporaryDirectory]/XXXXXX\""
        set basefilename [ eval exec $CMD ]

        set filename ""
        set NAME ""

        if { $type == "Volume" } {
            set NAME .nrrd
        } elseif { $type == "LinearTransform"  } {
            set NAME .mat
        } elseif { $type == "BSplineTransform"  } {
            set NAME .mat
        } elseif { $type == "Text"  } {
            set NAME .txt
        }
        
        if { $NAME != ""} {
            set filename $basefilename$NAME
            $LOGIC PrintText "TCL: Create file: $filename"
            set CMD "touch \"$filename\""
            eval exec $CMD
        } else {
            PrintError "Could not create file: $basefilename$NAME"
        }
        
        return $filename
    }

    #
    proc CreateDirName { type } {
        variable GUI
        variable LOGIC

        # dry-run, XXXXXX cannot be in the middle of the name on some platforms (fc11)
        set CMD "mktemp -u -d \"[$GUI GetTemporaryDirectory]/XXXXXX\""
        set basefilename [ eval exec $CMD ]

        set dirname ""
        set NAME ""

        if { $type == "xform" } {
            set NAME .xform
        } else {
            PrintError "CreateDirName: Unknown type"
        }

        if { $NAME != ""} {
            set dirname $basefilename$NAME
            $LOGIC PrintText "TCL: Create directory: $dirname"
            set CMD "mkdir \"$dirname\""
            eval exec $CMD
        } else {
            PrintError "Could not create file: $basefilename$NAME"
        }

        return $dirname
    }


    # TODO: ChangeName of this function, its doing more than only creating a volume node
    # vtkMRMLVolumeNode *volumeNode, const char *name)
    proc CreateVolumeNode { volumeNode name } {
        variable SCENE
        if {$volumeNode == ""} { return "" }
        # clone the display node
        set clonedDisplayNode [vtkMRMLScalarVolumeDisplayNode New]
        $clonedDisplayNode CopyWithScene [$volumeNode GetDisplayNode]
        $SCENE AddNode $clonedDisplayNode
        set dispID [$clonedDisplayNode GetID]
        $clonedDisplayNode Delete

        set clonedVolumeNode [vtkMRMLScalarVolumeNode New]
        $clonedVolumeNode CopyWithScene $volumeNode
        # MRML interprets "" as a ID -> can cause issues when trying to do a UpdateScene
        # $clonedVolumeNode SetAndObserveStorageNodeID ""
        $clonedVolumeNode SetName "$name"
        $clonedVolumeNode SetAndObserveDisplayNodeID $dispID

        if {0} {
            # copy over the volume's data
            $clonedVolumeData [vtkImageData New]
            $clonedVolumeData DeepCopy [volumeNode GetImageData]
            $clonedVolumeNode SetAndObserveImageData $clonedVolumeData
            $clonedVolumeNode SetModifiedSinceRead 1
            $clonedVolumeData Delete
        } else {
            $clonedVolumeNode SetAndObserveImageData ""
        }

        # add the cloned volume to the scene
        $SCENE AddNode $clonedVolumeNode
        set volID [$clonedVolumeNode GetID]
        $clonedVolumeNode Delete
        # Have to do it this way bc unlike in c++ the link to $clonedVolumeNode gets deleted
        return [$SCENE GetNodeByID $volID]
    }

    proc PrintError { TEXT } {
        variable LOGIC
        $LOGIC PrintText "TCL: ERROR: EMSegmenterPreProcessingTcl::${TEXT}"
    }


    # update volumeCollectionNode with new volumes - and delete the old ones
    proc UpdateVolumeCollectionNode { volumeCollectionNode newVolumeCollectionNode } {
        variable LOGIC

        set inputNum [$volumeCollectionNode GetNumberOfVolumes]
        $LOGIC PrintText "TCL: $inputNum targetNodes detected"
        
        # replace each volume
        for { set i 0 } {$i < $inputNum } { incr i } {
            set newVolumeNode [lindex $newVolumeCollectionNode $i]
            if {$newVolumeNode == "" } {
                PrintError "Run: Processed target node is incomplete !"
                return 1
            }
            set oldVolumeNode [$volumeCollectionNode GetNthVolumeNode $i]

            # Set up the new ones
            $volumeCollectionNode SetNthVolumeNodeID $i [$newVolumeNode GetID]

            # Remove old volumes associated with volumeCollectionNode
            # if you delete right away then volumeCollectionNode is decrease
            DeleteNode $oldVolumeNode
        }
        return 0
    }

    # ----------------------------------------------------------------------------
    # We have to create this function so that we can run it in command line mode
    #
    proc GetCheckButtonValueFromMRML { ID } {
        return [GetEntryValueFromMRML "C" $ID]
    }

    proc GetVolumeMenuButtonValueFromMRML { ID } {
        variable mrmlManager
        set MRMLID [GetEntryValueFromMRML "V" $ID]
        if { ("$MRMLID" != "") && ("$MRMLID" != "NULL") } {
            return [$mrmlManager MapMRMLNodeIDToVTKNodeID $MRMLID]
        }
        return 0
    }

    proc GetTextEntryValueFromMRML { ID } {
        return [GetEntryValueFromMRML "E" $ID]
    }


    proc GetEntryValueFromMRML { Type ID } {
        variable mrmlManager
        set TEXT [string range [string map { "|" "\} \{" } "[[$mrmlManager GetGlobalParametersNode] GetTaskPreProcessingSetting]"] 1 end]
        set TEXT "${TEXT}\}"
        set index 0
        foreach ARG $TEXT {
            if {"[string index $ARG 0]" == "$Type" } {
                if { $index == $ID } {
                    return "[string range $ARG 1 end]"
                }
                incr index
            }
        }
        return ""
    }


    #
    # Preprocessing Functions
    #
    proc InitVariables { {initLOGIC ""} {initManager ""} {initPreGUI "" } } {
        variable GUI
        variable preGUI
        variable LOGIC
        variable SCENE
        variable mrmlManager
        variable workingDN
        variable alignedTargetNode
        variable inputAtlasNode
        variable outputAtlasNode
        variable inputSubParcellationNode
        variable outputSubParcellationNode
        variable preferredRegistrationPackage
        variable selectedRegistrationPackage
        variable CMTKFOLDER
        
        set GUI $::slicer3::Application
        if { $GUI == "" } {
            puts stderr "ERROR: GenericTask: InitVariables: GUI not defined"
            return 1
        }

        if { $initLOGIC == "" } {
            set MOD [$GUI GetModuleGUIByName "EMSegmenter"]
            if {$MOD == ""} {
                puts stderr "ERROR: GenericTask: InitVariables: EMSegmenter not defined"
                return 1
            }
            set LOGIC [$MOD GetLogic]
            if { $LOGIC == "" } {
                puts stderr "ERROR: GenericTask: InitVariables: LOGIC not defined"
                return 1
            }
        } else {
            set LOGIC $initLOGIC
        }

        # Do not move it before bc LOGIC is not defined until here 

        $LOGIC PrintText "TCL: =========================================="
        $LOGIC PrintText "TCL: == Init Variables"
        $LOGIC PrintText "TCL: =========================================="
        
        
        if { $initManager == "" } {
            set MOD [$::slicer3::Application GetModuleGUIByName "EMSegmenter"]
            if {$MOD == ""} {
                PrintError "InitVariables: EMSegmenter not defined"
                return 1
            }

            set mrmlManager [$MOD GetMRMLManager]
            if { $mrmlManager == "" } {
                PrintError "InitVariables: mrmManager not defined"
                return 1
            }
        } else {
            set mrmlManager $initManager

        }

        set SCENE [$mrmlManager GetMRMLScene]
        if { $SCENE == "" } {
            PrintError "InitVariables: SCENE not defined"
            return 1
        }

        set workingDN [$mrmlManager GetWorkingDataNode]
        if { $workingDN == "" } {
            $LOGIC PrintText "TCL: EMSegmenterPreProcessingTcl::InitVariables: WorkingData not defined"
            return 1
        }

        if {$initPreGUI == "" } {
            set MOD [$::slicer3::Application GetModuleGUIByName "EMSegmenter"]
            if {$MOD == ""} {
                PrintError "InitVariables: EMSegmenter not defined"
                return 1
            }

            set preGUI [$MOD GetPreProcessingStep]
            if { $preGUI == "" } {
                PrintError "InitVariables: PreProcessingStep not defined"
                return 1
            }
        } else {
            set preGUI $initPreGUI
        }
        
        if { [$mrmlManager GetRegistrationPackageType] == [$mrmlManager GetPackageTypeFromString CMTK] } {
            set preferredRegistrationPackage CMTK
            $LOGIC PrintText "TCL: User selected CMTK"
        } elseif { [$mrmlManager GetRegistrationPackageType] == [$mrmlManager GetPackageTypeFromString BRAINS] } {
            set preferredRegistrationPackage BRAINS
            $LOGIC PrintText "TCL: User selected BRAINS"
        } else {
            PrintError "InitVariables: RegistrationPackage [$mrmlManager GetRegistrationPackageType] not defined"
            return 1
        }
        
        set selectedRegistrationPackage ""
        switch -exact "$preferredRegistrationPackage" {
            "CMTK" {
                set CMTKFOLDER [Get_CMTK_Installation_Path]
                if { $CMTKFOLDER != "" } {
                        $LOGIC PrintText "TCL: Found CMTK in $CMTKFOLDER"
                        set selectedRegistrationPackage "CMTK"
                } else {
                    $LOGIC PrintText "TCL: WARNING: Couldn't find CMTK, switch back to BRAINSTools"
                    set selectedRegistrationPackage "BRAINS"
                }
            }
            "BRAINS" {
                set selectedRegistrationPackage "BRAINS"
            }
            default {
                PrintError "registration package not known"
                set selectedRegistrationPackage "BRAINS"
                return 1
            }
        }

        # All other Variables are defined when running the pipeline as they are the volumes
        # Define alignedTargetNode when initializing pipeline
        set alignedTargetNode ""
        set inputAtlasNode ""
        set outputAtlasNode ""
        set inputSubParcellationNode ""
        set outputSubParcellationNode ""

        return 0
    }



    #------------------------------------------------------
    # return 0 when no error occurs
    proc ShowUserInterface { } {
        variable preGUI
        variable LOGIC

        if { [InitVariables] } {
            puts stderr "ERROR: GernicTask.tcl: ShowUserInterface: Not all variables are correctly defined!"
            return 1
        }

        $LOGIC PrintText "TCL: Preprocessing GenericTask"

        # -------------------------------------
        # Define Interface Parameters
        # -------------------------------------
        $preGUI DefineTextLabel "No preprocessing defined for this task!" 0
    }

    # ----------------------------------------------------------------
    # Make Sure that input volumes all have the same resolution
    # from StartPreprocessingTargetToTargetRegistration
    # ----------------------------------------------------------------
    proc RegisterInputImages { inputTargetNode fixedTargetImageIndex } {
        variable workingDN
        variable mrmlManager
        variable LOGIC
        variable SCENE

        $LOGIC PrintText "TCL: =========================================="
        $LOGIC PrintText "TCL: == Register Input Images --"
        $LOGIC PrintText "TCL: =========================================="
        # ----------------------------------------------------------------
        # set up rigid registration
        set alignedTargetNode [$workingDN GetAlignedTargetNode]
        if { $alignedTargetNode == "" } {
            # input scan does not have to be aligned
            set alignedTargetNode [$mrmlManager CloneTargetNode $inputTargetNode "Aligned"]
            $workingDN SetReferenceAlignedTargetNodeID [$alignedTargetNode GetID]
        } else {
            $mrmlManager SynchronizeTargetNode $inputTargetNode $alignedTargetNode "Aligned"
        }

        for { set i 0 } { $i < [$alignedTargetNode GetNumberOfVolumes] } {incr i} {
            set intputVolumeNode($i) [$inputTargetNode GetNthVolumeNode $i]
            if { $intputVolumeNode($i) == "" } {
                PrintError "RegisterInputImages: the ${i}th input node is not defined!"
                return 1
            }

            set intputVolumeData($i) [$intputVolumeNode($i) GetImageData]
            if { $intputVolumeData($i) == "" } {
                PrintError "RegisterInputImages: the ${i}the input node has no image data defined !"
                return 1
            }

            set outputVolumeNode($i) [$alignedTargetNode GetNthVolumeNode $i]
            if { $outputVolumeNode($i) == "" } {
                PrintError "RegisterInputImages: the ${i}th aligned input node is not defined!"
                return 1
            }

            set outputVolumeData($i) [$outputVolumeNode($i) GetImageData]
            if { $outputVolumeData($i) == "" } {
                PrintError "RegisterInputImages: the ${i}the output node has no image data defined !"
                return 1
            }
        }
        set fixedVolumeNode $outputVolumeNode($fixedTargetImageIndex)
        set fixedImageData $outputVolumeData($fixedTargetImageIndex)


        # ----------------------------------------------------------------
        # perfom "rigid registration" or "resample and cast only"
        if {[$mrmlManager GetEnableTargetToTargetRegistration] } {

            $LOGIC PrintText "TCL: ===> Register Target To Target "

            for { set i 0 } {$i < [$alignedTargetNode GetNumberOfVolumes] } { incr i } {
                if { $i == $fixedTargetImageIndex } {
                    continue;
                }

                set movingVolumeNode $intputVolumeNode($i)
                set outVolumeNode $outputVolumeNode($i)
                set backgroundLevel [$LOGIC GuessRegistrationBackgroundLevel $movingVolumeNode]

                # Using BRAINS suite, TODO: is affine=off here?
                set transformNode [BRAINSRegistration $fixedVolumeNode $movingVolumeNode $outVolumeNode $backgroundLevel "Rigid" 0]
                if { $transformNode == "" } {
                    PrintError "Transform node is null"
                    return 1
                }

                $LOGIC PrintText "TCL: === Just for debugging $transformNode [$transformNode GetName] [$transformNode GetID]"
                set outputNode [vtkMRMLScalarVolumeDisplayNode New]
                $outputNode SetName "blub1"
                $SCENE AddNode $outputNode
                set outputNodeID [$outputNode GetID]
                $outputNode Delete

                if { [Resample $movingVolumeNode $fixedVolumeNode $transformNode "NotUsedForBSpline" "BSplineTransform" Linear  $backgroundLevel [$SCENE GetNodeByID $outputNodeID]] } {
                    return 1
                }
                ## $SCENE RemoveNode $transformNode
            }
        } else {

            $LOGIC PrintText "TCL: ===> Skipping Registration of Target To Target "

            for { set i 0 } {$i < [$alignedTargetNode GetNumberOfVolumes] } { incr i } {
                if { $i == $fixedTargetImageIndex } {
                    continue;
                }

                set movingVolumeNode $intputVolumeNode($i)
                set outVolumeNode $outputVolumeNode($i)

                # Just creates output with same dimension as fixed volume
                $LOGIC StartPreprocessingResampleAndCastToTarget $movingVolumeNode $fixedVolumeNode $outVolumeNode
            }
        }

        # Clean up
        $workingDN SetAlignedTargetNodeIsValid 1

        return 0
    }

     # Create Voronoi diagram with correct scalar type from aligned subparcellation 
     proc GeneratedVoronoi { input } {
 
            set output [vtkImageData New]
            $output DeepCopy $input 

            set voronoi [vtkImageLabelPropagation New]
            $voronoi SetInput $output 
            $voronoi Update 

            set voronoiCast [vtkImageCast New]
            $voronoiCast SetInput [$voronoi GetPropagatedMap] 
            $voronoiCast SetOutputScalarType  [$output GetScalarType]
            $voronoiCast Update

            $input DeepCopy [$voronoiCast GetOutput]
            $voronoiCast Delete
            $voronoi Delete
            $output Delete
     }

    #------------------------------------------------------
    # from StartPreprocessingTargetToTargetRegistration
    #------------------------------------------------------
    proc SkipAtlasRegistration { } {
        variable workingDN
        variable mrmlManager
        variable LOGIC
        variable alignedTargetNode
        variable inputAtlasNode
        variable outputAtlasNode
        variable inputSubParcellationNode
        variable outputSubParcellationNode
        $LOGIC PrintText "TCL: =========================================="
        $LOGIC PrintText "TCL: == Skip Atlas Registration"
        $LOGIC PrintText "TCL: =========================================="

        # This function makes sure that the "output atlas" is identically to the "input atlas".
        # Each volume of the "output atlas" will then be resampled to the resolution of the "fixed target volume"
        # The "output atlas will be having the same ScalarType as the "fixed target volume'". There is no additionally cast necessary.

        set fixedTargetChannel 0

        # ----------------------------------------------------------------
        #  makes sure that the "output atlas" is identically to the "input atlas"
        # ----------------------------------------------------------------
        if { $outputAtlasNode == "" } {
            $LOGIC PrintText "TCL: Atlas was empty"
            #  $LOGIC PrintText "set outputAtlasNode \[$mrmlManager CloneAtlasNode $inputAtlasNode \"AlignedAtlas\"\] "
            set outputAtlasNode [$mrmlManager CloneAtlasNode $inputAtlasNode "Aligned"]
            $workingDN SetReferenceAlignedAtlasNodeID [$outputAtlasNode GetID]
        } else {
            $LOGIC PrintText "TCL: Atlas was just synchronized"
            $mrmlManager SynchronizeAtlasNode $inputAtlasNode $outputAtlasNode AlignedAtlas
        }

        if { $outputSubParcellationNode == "" } {
            $LOGIC PrintText "TCL: SubParcellation was empty"
            #  $LOGIC PrintText "set outputSubParcellationNode \[$mrmlManager CloneSubParcellationNode $inputSubParcellationNode \"AlignedSubParcellation\"\] "
            set outputSubParcellationNode [$mrmlManager CloneSubParcellationNode $inputSubParcellationNode "Aligned"]
            $workingDN SetReferenceAlignedSubParcellationNodeID [$outputSubParcellationNode GetID]
        } else {
            $LOGIC PrintText "TCL: SubParcellation was just synchronized"
            $mrmlManager SynchronizeSubParcellationNode $inputSubParcellationNode $outputSubParcellationNode AlignedSubParcellation
        }


        # ----------------------------------------------------------------
        # set the fixed target volume
        # ----------------------------------------------------------------
        set fixedTargetVolumeNode [$alignedTargetNode GetNthVolumeNode $fixedTargetChannel]
        if { [$fixedTargetVolumeNode GetImageData] == "" } {
            PrintError "SkipAtlasRegistration: Fixed image is null, skipping resampling"
            return 1;
        }


        # ----------------------------------------------------------------
        # Make Sure that atlas volumes all have the same resolution as input
        # ----------------------------------------------------------------
        for { set i 0 } {$i < [$outputAtlasNode GetNumberOfVolumes] } { incr i } {
            set movingVolumeNode [$inputAtlasNode GetNthVolumeNode $i]
            set outputVolumeNode [$outputAtlasNode GetNthVolumeNode $i]
            $LOGIC StartPreprocessingResampleAndCastToTarget $movingVolumeNode $fixedTargetVolumeNode $outputVolumeNode
        }

        for { set i 0 } {$i < [$outputSubParcellationNode GetNumberOfVolumes] } { incr i } {
            set movingVolumeNode [$inputSubParcellationNode GetNthVolumeNode $i]
            set outputVolumeNode [$outputSubParcellationNode GetNthVolumeNode $i]
            $LOGIC StartPreprocessingResampleAndCastToTarget $movingVolumeNode $fixedTargetVolumeNode $outputVolumeNode
            GeneratedVoronoi [$outputVolumeNode GetImageData]
        }


        $LOGIC PrintText "TCL: EMSEG: Atlas-to-target resampling complete."
        $workingDN SetAlignedAtlasNodeIsValid 1
        return 0
    }



    # -----------------------------------------------------------
    # sets up all variables
    # Define the three volume relates Input nodes to the pipeline
    # - alignedTargetNode
    # - inputAtlasNode
    # - outputAtasNode
    # -----------------------------------------------------------
    proc InitPreProcessing { } {
        variable mrmlManager
        variable LOGIC
        variable workingDN
        variable alignedTargetNode
        variable inputAtlasNode
        variable outputAtlasNode
        variable inputSubParcellationNode
        variable outputSubParcellationNode
        $LOGIC PrintText "TCL: =========================================="
        $LOGIC PrintText "TCL: == InitPreprocessing"
        $LOGIC PrintText "TCL: =========================================="

        # TODO: Check for
        # - environment variables  and
        # - command line executables
        #set PLUGINS_DIR "[$::slicer3::Application GetPluginsDir]"
        #if { $PLUGINS_DIR == "" } {
        #    PrintError "InitPreProcessing: Environmet variable not set corretly"
        #    return 1
        #}

        # -----------------------------------------------------------
        # Check and set valid variables
        if { [$mrmlManager GetGlobalParametersNode] == 0 } {
            PrintError "InitPreProcessing: Global parameters node is null, aborting!"
            return 1
        }

        $LOGIC StartPreprocessingInitializeInputData


        # -----------------------------------------------------------
        # Define Target Node
        # this should be the first step for any preprocessing
        # from StartPreprocessingTargetToTargetRegistration
        # -----------------------------------------------------------

        set inputTargetNode [$workingDN GetInputTargetNode]
        if {$inputTargetNode == "" } {
            PrintError "InitPreProcessing: InputTargetNode not defined"
            return 1
        }

        set inputTargetPositiveCollectionNode [RemoveNegativeValues $inputTargetNode]
        if { $inputTargetPositiveCollectionNode == "" } {
            PrintError "Run: RemoveNegativeValues failed !"
            return 1
        }

        if { [UpdateVolumeCollectionNode $inputTargetNode "$inputTargetPositiveCollectionNode"] } {
            PrintError "UpdateVolumeCollectionNode failed !"
            return 1
        }


        if {[RegisterInputImages $inputTargetNode 0] } {
            PrintError "InitPreProcessing: Target-to-Target failed!"
            return 1
        }

        set alignedTargetNode [$workingDN GetAlignedTargetNode]
        if {$alignedTargetNode == "" } {
            PrintError "InitPreProcessing: cannot retrieve Aligned Target Node !"
            return 1
        }

        # -----------------------------------------------------------
        # Define Atlas
        # -----------------------------------------------------------
        set inputAtlasNode [$mrmlManager GetAtlasInputNode]
        if {$inputAtlasNode == "" } {
            PrintError "InitPreProcessing: InputAtlas not defined"
            return 1
        }

        set outputAtlasNode [$workingDN GetAlignedAtlasNode]

        set inputSubParcellationNode [$mrmlManager GetSubParcellationInputNode]
        if {$inputSubParcellationNode == "" } {
            PrintError "InitPreProcessing: InputSubParcellation not defined"
            return 1
        }

        set outputSubParcellationNode [$workingDN GetAlignedSubParcellationNode]



        return 0
    }



    proc calcDFVolumeNode { inputVolumeNode referenceVolumeNode transformationNode }   {
        variable LOGIC

        $LOGIC PrintText "TCL: Create deformation field"

        # create a deformation field

        if { $transformationNode == "" } {
            PrintError "calcDFVolumeNode: transformation node not correctly defined"
            return 1
        }
        set tmpTransformFileName [WriteDataToTemporaryDir $transformationNode Transform]
        if { $tmpTransformFileName == "" } { return 1 }

        set tmpInputVolumeFileName [WriteImageDataToTemporaryDir $inputVolumeNode ]
        if { $tmpInputVolumeFileName == "" } { return 1 }

        set tmpReferenceVolumeFileName [WriteImageDataToTemporaryDir  $referenceVolumeNode ]
        if { $tmpReferenceVolumeFileName == "" } { return 1 }

        set DFNode [CreateVolumeNode $referenceVolumeNode "deformVolume"]
        set outputVolume [vtkImageData New]

        $DFNode SetAndObserveImageData $outputVolume
        $outputVolume Delete

        set deformationFieldFilename [ CreateTemporaryFileNameForNode $DFNode ]

        set PLUGINS_DIR "[$::slicer3::Application GetPluginsDir]"
        set CMDdeform "${PLUGINS_DIR}/BSplineToDeformationField"
        set CMDdeform "$CMDdeform --refImage \"$tmpReferenceVolumeFileName\"" 
        set CMDdeform "$CMDdeform --tfm \"$tmpTransformFileName\""
        set CMDdeform "$CMDdeform --defImage \"$deformationFieldFilename\""

        $LOGIC PrintText "TCL: Executing $CMDdeform"
        catch { eval exec $CMDdeform } errmsg
        $LOGIC PrintText "TCL: $errmsg"

        # store for later use, TODO: unable to read vector:
        # ERROR: In /projects/sandbox/Slicer3/trunk/Slicer3/Libs/vtkITK/vtkITKArchetypeImageSeriesScalarReader.cxx, line 169
        # vtkITKArchetypeImageSeriesScalarReader (0x8dbf5c0): UpdateFromFile: Unsupported number of components (only 1 allowed): 3
        ###        ReadDataFromDisk $DFNode $deformationFieldFilename Volume

        $LOGIC PrintText "TCL: Create deformation field...END"
        ###        return $DFNode
        return $deformationFieldFilename
    }


    # returns transformation when no error occurs
    # now call commandline directly
    #
    proc BRAINSResampleCLI { inputVolumeNode referenceVolumeNode outVolumeNode transformationNode backgroundLevel interpolationType BRAINSBSpline } {
        variable SCENE
        variable LOGIC

        $LOGIC PrintText "TCL: =========================================="
        $LOGIC PrintText "TCL: == Resample Image CLI"
        $LOGIC PrintText "TCL: =========================================="

        set PLUGINS_DIR "[$::slicer3::Application GetPluginsDir]"
        set CMD "${PLUGINS_DIR}/BRAINSResample"

        set tmpInputVolumeFileName [WriteImageDataToTemporaryDir $inputVolumeNode ]
        if { $tmpInputVolumeFileName == "" } { return 1 }
        set RemoveFiles "$tmpInputVolumeFileName"
        set CMD "$CMD --inputVolume \"$tmpInputVolumeFileName\""

        set tmpReferenceVolumeFileName [WriteImageDataToTemporaryDir $referenceVolumeNode ]
        if { $tmpReferenceVolumeFileName == "" } { return 1 }
        set RemoveFiles "$RemoveFiles \"$tmpReferenceVolumeFileName\""
        set CMD "$CMD --referenceVolume \"$tmpReferenceVolumeFileName\""

        if { $transformationNode == "" } {
            PrintError "BRAINSResampleCLI: transformation node not correctly defined"
            return 1
        }

        if { $BRAINSBSpline } {
            # use a BSpline transformation
            set tmpTransformFileName [WriteDataToTemporaryDir $transformationNode Transform]
            if { $tmpTransformFileName == "" } { return 1 }
            set RemoveFiles "$RemoveFiles \"$tmpTransformFileName\""
            set CMD "$CMD --warpTransform \"$tmpTransformFileName\""
        } else {
            #            set DFVolumeFileName [WriteImageDataToTemporaryDir $transformationNode]
            #            if { $DFVolumeFileName == "" } { return 1 }
            #            set RemoveFiles "$RemoveFiles \"$DFVolumeFileName\""
            #            set CMD "$CMD --deformationVolume \"$DFVolumeFileName\""

            # it is a filename
            set CMD "$CMD --deformationVolume \"$transformationNode\""

        }

        if { $outVolumeNode == "" } {
            PrintError "BRAINSResampleCLI: output volume node not correctly defined"
            return 1
        }
        set outVolumeFileName [CreateTemporaryFileNameForNode $outVolumeNode]
        if { $outVolumeFileName == "" } { return 1 }
        set CMD "$CMD --outputVolume \"$outVolumeFileName\""

        set CMD "$CMD --defaultValue \"$backgroundLevel\""

        set CMD "$CMD --pixelType"
        set referenceVolume [$referenceVolumeNode GetImageData]
        set scalarType [$referenceVolume GetScalarTypeAsString]
        switch -exact "$scalarType" {
            "bit" { set CMD "$CMD binary" }
            "unsigned char" { set CMD "$CMD uchar" }
            "unsigned short" { set CMD "$CMD ushort" }
            "unsigned int" { set CMD "$CMD uint" }
            "short" -
            "int" -
            "float" { set CMD "$CMD $scalarType" }
            default {
                PrintError "BRAINSResampleCLI: cannot resample a volume of type $scalarType"
                return 1
            }
        }

        # Linear
        set CMD "$CMD --interpolationMode $interpolationType"

        $LOGIC PrintText "TCL: Executing $CMD"
        catch { eval exec $CMD } errmsg
        $LOGIC PrintText "TCL: $errmsg"


        # Write results back to scene
        # This does not work $::slicer3::ApplicationLogic RequestReadData [$outVolumeNode GetID] $outVolumeFileName 0 1
        ReadDataFromDisk $outVolumeNode $outVolumeFileName Volume
        file delete -force \"$outVolumeFileName\"

        return 0
    }


    # This function registers the atlas to the input (both are non-skull stripped)
    # The transformation will then be applied to the known atlas ICC mask
    # The transformed mask will then be applied to the input 
    #
    # The function returns a node with stripped input volumes
    proc BRAINSSkullStripper { inputNode atlasNode } {
        variable SCENE
        variable LOGIC
        $LOGIC PrintText "TCL: =========================================="
        $LOGIC PrintText "TCL: == BRAINSSkullStripper"

        set PLUGINS_DIR "[$::slicer3::Application GetPluginsDir]"
        
        # initialize
        set inputNode_SkullStripped ""
        set atlasNode_SkullStripped ""

        # Run the algorithm on each volume
        for { set i 0 } {$i < [$inputNode GetNumberOfVolumes] } { incr i } {

            set inputVolumeNode [$inputNode GetNthVolumeNode $i]
            set inputVolumeData [$inputVolumeNode GetImageData]
            if { $inputVolumeData == "" } {
                PrintError "BRAINSSkullStripper: the ${i}th volume node has no input data defined!"
                foreach VolumeNode $inputNode_SkullStripped {
                    DeleteNode $VolumeNode
                }
                return ""
            }

            set tmpInputFileName [WriteDataToTemporaryDir $inputVolumeNode Volume]
            set RemoveFiles "\"$tmpInputFileName\""
            if { $tmpInputFileName == "" } {
                return ""
            }       

            #Threshold --threshold 0 --lower 0 --upper 255 --outsidevalue 1 --thresholdtype Above InputImage OutputImage
            #TODO: get atlas template file for this input
            set non_skull_stripped_atlas  /home/domibel/Desktop/data/spl_50_cases_with_manual_segmentation/ForSBIA/case2/spgr/case2norm.nrrd
            set atlas_mask /home/domibel/Desktop/data/spl_50_cases_with_manual_segmentation/ForSBIA/case2/spgr-strip/case2norm_mask.nrrd


            set atlas_mask_deformed [CreateFileName "Volume"]
            if { $atlas_mask_deformed == "" } {
                PrintError "it is empty"
            }


            set linearTransformFileName [CreateFileName "LinearTransform"]
            if { \"$linearTransformFileName\" == "" } {
                PrintError "it is empty"
            }

            set oArgument [CreateFileName "Volume"]
            if { \"$oArgument\" == "" } {
                PrintError "it is empty"
            }

            set deformationfield [CreateFileName "Volume"]
            if { \"$deformationfield\" == "" } {
                PrintError "it is empty"
            }


            set fixedVolumeFileName \"$tmpInputFileName\"
            set movingVolumeFileName \"$non_skull_stripped_atlas\"

            set outputVolumeFileName [CreateFileName "Volume"]
            if { $outputVolumeFileName == "" } {
                PrintError "it is empty"
            }

            # LINEAR REGISTRATION
            set CMD "${PLUGINS_DIR}/BRAINSFit"
            set CMD "$CMD --fixedVolume \"$fixedVolumeFileName\""
            set CMD "$CMD --movingVolume \"$movingVolumeFileName\""
            set CMD "$CMD --outputVolume \"$outputVolumeFileName\""
            set CMD "$CMD --outputTransform \"$linearTransformFileName\""
            set CMD "$CMD --initializeTransformMode useMomentsAlign --transformType Rigid,Affine"

            $LOGIC PrintText "TCL: Executing $CMD"
            catch { eval exec $CMD } errmsg
            $LOGIC PrintText "TCL: $errmsg"


            # NON-LINEAR REGISTRATION
            set CMD "${PLUGINS_DIR}/BRAINSDemonWarp"
            set CMD "$CMD -m \"$movingVolumeFileName\""
            set CMD "$CMD -f \"$fixedVolumeFileName\""
            set CMD "$CMD --initializeWithTransform \"$linearTransformFileName\""
            set CMD "$CMD -o $oArgument -O \"$deformationfield\""
            #set CMD "$CMD -i 1000,500,250,125,60 -n 5 -e --numberOfMatchPoints 16"
            # fast - for debugging
            set CMD "$CMD -i 1,5,2,1,1 -n 5 -e --numberOfMatchPoints 16"

            $LOGIC PrintText "TCL: Executing $CMD"
            catch { eval exec $CMD } errmsg
            $LOGIC PrintText "TCL: $errmsg"


            # WARP(=Resample) mask
            set CMD "${PLUGINS_DIR}/BRAINSResample"
            set CMD "$CMD --inputVolume \"$atlas_mask\""
            set CMD "$CMD --referenceVolume \"$fixedVolumeFileName\""
            set CMD "$CMD --deformationVolume \"$deformationfield\""
            set CMD "$CMD --outputVolume \"$atlas_mask_deformed\""
            #        set CMD "$CMD --defaultValue $backgroundLevel"
            set CMD "$CMD --pixelType"


            set referenceVolume [$inputVolumeNode GetImageData]
            set scalarType [$referenceVolume GetScalarTypeAsString]
            switch -exact "$scalarType" {
                "bit" { set CMD "$CMD binary" }
                "unsigned char" { set CMD "$CMD uchar" }
                "unsigned short" { set CMD "$CMD ushort" }
                "unsigned int" { set CMD "$CMD uint" }
                "short" -
                "int" -
                "float" { set CMD "$CMD $scalarType" }
                default {
                    PrintError "BRAINSSkullStripper: cannot resample a volume of type $scalarType"
                    return 1
                }
            }
            # Linear
            #        set CMD "$CMD --interpolationMode $interpolationType"

            $LOGIC PrintText "TCL: Executing $CMD"
            catch { eval exec $CMD } errmsg
            $LOGIC PrintText "TCL: $errmsg"




            # MASK input volume
            set CMD "${PLUGINS_DIR}/Mask"
            set CMD "$CMD --label 1 --replace 0 \"$tmpInputFileName\" \"$atlas_mask_deformed\" \"$outputVolumeFileName\""

            $LOGIC PrintText "TCL: Executing $CMD"
            catch { eval exec $CMD } errmsg
            $LOGIC PrintText "TCL: $errmsg"


            # create a new node for our output-list
            set outputVolumeNode [CreateVolumeNode $inputVolumeNode "[$inputVolumeNode GetName]_stripped"]
            set outputVolumeData [vtkImageData New]
            $outputVolumeNode SetAndObserveImageData $outputVolumeData
            $outputVolumeData Delete

            # Read results back
            ReadDataFromDisk $outputVolumeNode $outputVolumeFileName Volume
            file delete -force $outputVolumeFileName

            # still in for loop, create a list of Volumes
            set inputNode_SkullStripped "${inputNode_SkullStripped} $outputVolumeNode "
            $LOGIC PrintText "TCL: List of volume nodes: $inputNode_SkullStripped"


            ###
            set atlasVolumeNode [$atlasNode GetNthVolumeNode $i]
            set atlasVolumeData [$atlasVolumeNode GetImageData]
            if { $atlasVolumeData == "" } {
                #PrintError "BRAINSSkullStripper: the ${i}th volume node has no input data defined!"
                foreach VolumeNode $atlasNode_SkullStripped {
                    DeleteNode $VolumeNode
                }
                return ""
            }
            set tmpAtlasFileName [WriteDataToTemporaryDir $atlasVolumeNode Volume]
            set RemoveFiles "\"$tmpAtlasFileName\""
            if { $tmpAtlasFileName == "" } {
                return ""
            }       

            set outputAtlasVolumeFileName [CreateFileName "Volume"]
            if { $outputAtlasVolumeFileName == "" } {
                PrintError "it is empty"
            }
            #

            # WARP(=Resample) mask
            set CMD "${PLUGINS_DIR}/BRAINSResample"
            set CMD "$CMD --inputVolume \"$tmpAtlasFileName\""
            set CMD "$CMD --referenceVolume \"$fixedVolumeFileName\""
            set CMD "$CMD --deformationVolume \"$deformationfield\""
            set CMD "$CMD --outputVolume \"$outputAtlasVolumeFileName\""
            #        set CMD "$CMD --defaultValue \"$backgroundLevel\""
            set CMD "$CMD --pixelType"


            set referenceVolume [$inputVolumeNode GetImageData]
            set scalarType [$referenceVolume GetScalarTypeAsString]
            switch -exact "$scalarType" {
                "bit" { set CMD "$CMD binary" }
                "unsigned char" { set CMD "$CMD uchar" }
                "unsigned short" { set CMD "$CMD ushort" }
                "unsigned int" { set CMD "$CMD uint" }
                "short" -
                "int" -
                "float" { set CMD "$CMD $scalarType" }
                default {
                    PrintError "BRAINSSkullStripper: cannot resample a volume of type $scalarType"
                    return 1
                }
            }
            # Linear
            #        set CMD "$CMD --interpolationMode $interpolationType"

            $LOGIC PrintText "TCL: Executing $CMD"
            catch { eval exec $CMD } errmsg
            $LOGIC PrintText "TCL: $errmsg"



            #


            # create a new node for our output-list
            set atlasoutputVolumeNode [CreateVolumeNode $atlasVolumeNode "[$atlasVolumeNode GetName]_stripped"]
            set atlasoutputVolumeData [vtkImageData New]
            $atlasoutputVolumeNode SetAndObserveImageData $atlasoutputVolumeData
            $atlasoutputVolumeData Delete

            # Read results back
            ReadDataFromDisk $atlasoutputVolumeNode $outputAtlasVolumeFileName Volume
            file delete -force $outputAtlasVolumeFileName

            set atlasNode_SkullStripped "${atlasNode_SkullStripped} $atlasoutputVolumeNode "
            


        }
        return "$inputNode_SkullStripped"
    }



    proc CMTKResampleCLI { inputVolumeNode referenceVolumeNode outVolumeNode transformDirName backgroundLevel } {
        variable SCENE
        variable LOGIC
        variable CMTKFOLDER
        
        $LOGIC PrintText "TCL: =========================================="
        $LOGIC PrintText "TCL: == Resample Image CLI : CMTKResampleCLI "
        $LOGIC PrintText "TCL: =========================================="

        set CMD "$CMTKFOLDER/reformatx"

        set CMD "$CMD --pad-out $backgroundLevel"

        set outVolumeFileName [CreateTemporaryFileNameForNode $outVolumeNode]
        if { $outVolumeFileName == "" } { return 1 }
        set CMD "$CMD -o \"$outVolumeFileName\""

        set inputVolumeFileName [WriteDataToTemporaryDir $inputVolumeNode Volume]
        set RemoveFiles "$inputVolumeFileName"
        if { $inputVolumeFileName == "" } {
            return 1
        }
        set CMD "$CMD --floating \"$inputVolumeFileName\""

        # set the right scalar type
        set referenceVolume [$referenceVolumeNode GetImageData]
        set scalarType [$referenceVolume GetScalarTypeAsString]
        switch -exact "$scalarType" {
            "char" { set CMD "$CMD --char" }
            "unsigned char" { set CMD "$CMD --byte" }
            "short" { set CMD "$CMD --short" }
            "unsigned short" { set CMD "$CMD --ushort" }
            "int" { set CMD "$CMD --int" }
            "float" { set CMD "$CMD --float" }
            "double" { set CMD "$CMD --double" }
            default {
                PrintError "CMTKResampleCLI: cannot resample a volume of type $scalarType"
                return 1
            }
        }

        set referenceVolumeFileName [WriteDataToTemporaryDir $referenceVolumeNode Volume]
        set RemoveFiles "$RemoveFiles $referenceVolumeFileName"
        if { $referenceVolumeFileName == "" } { return 1 }
        set CMD "$CMD \"$referenceVolumeFileName\""

        set CMD "$CMD \"$transformDirName\""

        $LOGIC PrintText "TCL: Executing $CMD"
        catch { eval exec $CMD } errmsg
        $LOGIC PrintText "TCL: $errmsg"


        # Write results back to scene
        # This does not work $::slicer3::ApplicationLogic RequestReadData [$outVolumeNode GetID] $outVolumeFileName 0 1
        ReadDataFromDisk $outVolumeNode $outVolumeFileName Volume
        file delete -force $outVolumeFileName

        return 0
    }


    proc WaitForDataToBeRead { } {
        variable LOGIC
        $LOGIC PrintText "TCL: Size of ReadDataQueue: $::slicer3::ApplicationLogic GetReadDataQueueSize [$::slicer3::ApplicationLogic GetReadDataQueueSize]"
        set i 20
        while { [$::slicer3::ApplicationLogic GetReadDataQueueSize] && $i} {
            $LOGIC PrintText "Waiting for data to be read... [$::slicer3::ApplicationLogic GetReadDataQueueSize]"
            incr i -1
            update
            after 1000
        }
        if { $i <= 0 } {
            $LOGIC PrintText "Error: timeout waiting for data to be read"
        }
    }

    proc DeleteCommandLine {clmNode } {
        variable LOGIC
        # Wait for jobs to finish
        set waiting 1
        set needToWait { "Idle" "Scheduled" "Running" }

        while {$waiting} {
            $LOGIC PrintText "TCL: Waiting for task..."
            set waiting 0
            set status [$clmNode GetStatusString]
            $LOGIC PrintText "[$clmNode GetName] $status"
            if { [lsearch $needToWait $status] != -1 } {
                set waiting 1
                after 250
            }
        }

        WaitForDataToBeRead
        $clmNode Delete
    }

    proc Run { } {
        variable LOGIC
        $LOGIC PrintText "TCL: =========================================="
        $LOGIC PrintText "TCL: == Preprocess Data"
        $LOGIC PrintText "TCL: =========================================="
        if {[InitPreProcessing]} { return 1}
        # Simply sets the given atlas (inputAtlasNode) to the output atlas (outputAtlasNode)
        SkipAtlasRegistration
        # Remove Transformations
        variable LOGIC
        return 0
    }

    # This function will be called by the wizard and expects a path or "" in the case CMTK is not installed
    proc Get_CMTK_Installation_Path { } {
        variable LOGIC
        
        set CMTKFOLDER ""
        # search for directories , sorted with the highest svn first
        set dirs [lsort -decreasing [glob -directory [$::slicer3::Application GetExtensionsInstallPath] -type d * ] ]
        foreach dir $dirs {
            set filename $dir\/CMTK4Slicer/registration
            if { [file exists $filename] } {
                set CMTKFOLDER  $dir\/CMTK4Slicer
                $LOGIC PrintText "TCL: Found CMTK in $dir\/CMTK4Slicer"
                break
            }
        }
        
        return $CMTKFOLDER
    }

    proc WriteDataToTemporaryDir { Node Type } {
        variable SCENE

        set tmpName [CreateTemporaryFileNameForNode $Node]
        if { $tmpName == "" } { return "" }

        if { "$Type" == "Volume" } {
            set out [vtkMRMLVolumeArchetypeStorageNode New]
        } elseif { "$Type" == "Transform" } {
            set out [vtkMRMLTransformStorageNode New]
        } else {
            PrintError "WriteDataToTemporaryDir: Unkown type $Type"
            return 0
        }

        $out SetScene $SCENE
        $out SetFileName $tmpName
        set FLAG [$out WriteData $Node]
        $out Delete
        if  { $FLAG == 0 } {
            PrintError "WriteDataToTemporaryDir: could not write file $tmpName"
            return ""
        }

        return "$tmpName"
    }

    proc WriteImageDataToTemporaryDir { Node } {
        if { $Node == "" || [$Node GetImageData] == "" } {
            PrintError "WriteImageDataToTemporaryDir: volume node to be warped is not correctly defined"
            return ""
        }
        return  [WriteDataToTemporaryDir $Node Volume]
    }

    proc ReadDataFromDisk { Node FileName Type } {
        variable SCENE
        variable LOGIC

        $LOGIC PrintText "TCL: ReadDataFromDisk: Try to read $FileName"
        if { [file exists $FileName] == 0 } {
            PrintError "ReadDataFromDisk: $FileName does not exist"
            return 0
        }

        # Load a scalar or vector volume node
        # Need to maintain the original coordinate frame established by
        # the images sent to the execution model
        if { "$Type" == "Volume" } {
            set dataReader [vtkMRMLVolumeArchetypeStorageNode New]
            $dataReader SetCenterImage 0
        } elseif { "$Type" == "Transform" } {
            set dataReader [vtkMRMLTransformStorageNode New]
        } else {
            PrintError "ReadDataFromDisk: Unkown type $Type"
            return 0
        }

        $dataReader SetScene $SCENE
        $dataReader SetFileName "$FileName"
        set FLAG [$dataReader ReadData $Node]
        $dataReader Delete

        if { $FLAG == 0 } {
            PrintError "ReadDataFromDisk : could not read file $FileName"
            return 0
        }
        return 1
    }

    # returns a transformation Node
    proc BRAINSRegistration { fixedVolumeNode movingVolumeNode outVolumeNode backgroundLevel deformableType affineType } {
        variable SCENE
        variable LOGIC
        variable mrmlManager
        
        set RegistrationType "Rigid ScaleVersor3D ScaleSkewVersor3D Affine"
        
        if { $deformableType != 0 } {
            set RegistrationType "${RegistrationType} BSpline"
        }
        
        
        $LOGIC PrintText "TCL: =========================================="
        $LOGIC PrintText "TCL: == Image Alignment CommandLine: $RegistrationType "
        $LOGIC PrintText "TCL: =========================================="

        set PLUGINS_DIR "[$::slicer3::Application GetPluginsDir]"
        set CMD "\"${PLUGINS_DIR}/BRAINSFit\""

        if { $fixedVolumeNode == "" || [$fixedVolumeNode GetImageData] == "" } {
            PrintError "AlignInputImages: fixed volume node not correctly defined"
            return ""
        }

        set tmpFileName [WriteDataToTemporaryDir $fixedVolumeNode Volume]
        set RemoveFiles "\"$tmpFileName\""

        if { $tmpFileName == "" } {
            return ""
        }
        set CMD "$CMD --fixedVolume \"$tmpFileName\""

        if { $movingVolumeNode == "" || [$movingVolumeNode GetImageData] == "" } {
            PrintError "AlignInputImages: moving volume node not correctly defined"
            return ""
        }

        set tmpFileName [WriteDataToTemporaryDir $movingVolumeNode Volume]
        set RemoveFiles "$RemoveFiles $tmpFileName"

        if { $tmpFileName == "" } { return 1 }
        set CMD "$CMD --movingVolume \"$tmpFileName\""

        #  still define this
        if { $outVolumeNode == "" } {
            PrintError "AlignInputImages: output volume node not correctly defined"
            return ""
        }
        set outVolumeFileName [CreateTemporaryFileNameForNode $outVolumeNode]

        if { $outVolumeFileName == "" } {
            return ""
        }
        set CMD "$CMD --outputVolume \"$outVolumeFileName\""

        set RemoveFiles "$RemoveFiles \"$outVolumeFileName\""

        # Do no worry about fileExtensions=".mat" type="linear" reference="movingVolume"
        # these are set in vtkCommandLineModuleLogic.cxx automatically
        if { [lsearch $RegistrationType "BSpline"] > -1 } {
            set transformNode [vtkMRMLBSplineTransformNode New]
            $transformNode SetName "EMSegmentBSplineTransform"
            $SCENE AddNode $transformNode
            set transID [$transformNode GetID]
            set outTransformFileName [CreateTemporaryFileNameForNode $transformNode]
            $transformNode Delete
            set CMD "$CMD --bsplineTransform \"$outTransformFileName\" --maxBSplineDisplacement 10.0"
        } else {
            set transformNode [vtkMRMLLinearTransformNode New]
            $transformNode SetName "EMSegmentLinearTransform"
            $SCENE AddNode $transformNode
            set transID [$transformNode GetID]
            set outTransformFileName [CreateTemporaryFileNameForNode $transformNode]

            $transformNode Delete
            set CMD "$CMD --outputTransform \"$outTransformFileName\""
        }
        set RemoveFiles "$RemoveFiles \"$outTransformFileName\""

        # -- still define this End

        # Write Parameters
        set fixedVolume [$fixedVolumeNode GetImageData]
        set scalarType [$fixedVolume GetScalarTypeAsString]
        switch -exact "$scalarType" {
            "bit" { set CMD "$CMD --outputVolumePixelType binary" }
            "unsigned char" { set CMD "$CMD --outputVolumePixelType uchar" }
            "unsigned short" { set CMD "$CMD --outputVolumePixelType ushort" }
            "unsigned int" { set CMD "$CMD --outputVolumePixelType uint" }
            "short" -
            "int" -
            "float" { set CMD "$CMD --outputVolumePixelType $scalarType" }
            default {
                PrintError "BRAINSRegistration: cannot resample a volume of type $scalarType"
                return ""
            }
        }

        # Filter options - just set it here to make sure that if default values are changed this still works as it supposed to
        set CMD "$CMD --backgroundFillValue $backgroundLevel"
        set CMD "$CMD --interpolationMode Linear"
        set CMD "$CMD --maskProcessingMode  ROIAUTO --ROIAutoDilateSize 3.0 --maskInferiorCutOffFromCenter 65.0 --initializeTransformMode useCenterOfHeadAlign"

        # might be still wrong
        foreach TYPE $RegistrationType {
            set CMD "$CMD --use${TYPE}"
        }

        
        if { $affineType == [$mrmlManager GetRegistrationTypeFromString RegistrationFast] } {
            set CMD "$CMD --numberOfSamples 100000  --splineGridSize 7,5,12 --projectedGradientTolerance  1e-4"
        } elseif { $affineType == [$mrmlManager GetRegistrationTypeFromString RegistrationTest] } {
            set CMD "$CMD --numberOfSamples 1000 --splineGridSize 7,5,12 --projectedGradientTolerance 1e-1"
        } elseif { $affineType == [$mrmlManager GetRegistrationTypeFromString RegistrationSlow] } {
            set CMD "$CMD --numberOfSamples 100000 --splineGridSize 28,20,24 --projectedGradientTolerance 1e-5"
        } else {
            PrintError "BRAINSRegistration: Unknown affineType: $affineType"
            return ""
        }
        

        set CMD "$CMD --numberOfIterations 1500 --minimumStepLength 0.005 --translationScale 1000.0 --reproportionScale 1.0 --skewScale 1.0  --fixedVolumeTimeIndex 0 --movingVolumeTimeIndex 0 --medianFilterSize 0,0,0 --numberOfHistogramBins 50 --numberOfMatchPoints 10 --useCachingOfBSplineWeightsMode ON --useExplicitPDFDerivativesMode AUTO --relaxationFactor 0.5 --maximumStepLength 0.2 --failureExitCode -1 --debugNumberOfThreads -1 --debugLevel 0 --costFunctionConvergenceFactor 1e+9 --costMetric MMI"

        $LOGIC PrintText "TCL: Executing $CMD"
        catch { eval exec $CMD } errmsg
        $LOGIC PrintText "TCL: $errmsg"


        # Read results back to scene
        # $::slicer3::ApplicationLogic RequestReadData [$outVolumeNode GetID] $outVolumeFileName 0 1
        # Cannot do it that way bc vtkSlicerApplicationLogic needs a cachemanager,
        # which is defined through vtkSlicerCacheAndDataIOManagerGUI.cxx
        # instead:

        # Test:
        # ReadDataFromDisk $outVolumeNode /home/pohl/Slicer3pohl/463_vtkMRMLScalarVolumeNode17.nrrd Volume
        if { [ReadDataFromDisk $outVolumeNode $outVolumeFileName Volume] == 0 } {
            set nodeID [$SCENE GetNodeByID $transID]
            if { $nodeID != "" } {
                $SCENE RemoveNode $nodeID
            }
        }

        # Test:
        # ReadDataFromDisk [$SCENE GetNodeByID $transID] /home/pohl/Slicer3pohl/EMSegmentLinearTransform.mat Transform
        if { [ReadDataFromDisk [$SCENE GetNodeByID $transID] $outTransformFileName Transform] == 0 } {
            set nodeID [$SCENE GetNodeByID $transID]
            if { $nodeID != "" } {
                $SCENE RemoveNode $nodeID
            }
        }

        # Test: 
        # $LOGIC PrintText "==> [[$SCENE GetNodeByID $transID] Print]"

        foreach NAME $RemoveFiles {
            file delete -force $NAME
        }

        # Remove Transformation from image
        $movingVolumeNode SetAndObserveTransformNodeID ""
        $SCENE Edited

        # return ID or ""
        return [$SCENE GetNodeByID $transID]
    }

    proc CMTKRegistration { fixedVolumeNode movingVolumeNode outVolumeNode backgroundLevel deformableType affineType} {
        variable SCENE
        variable LOGIC
        variable CMTKFOLDER
        variable mrmlManager
        
        $LOGIC PrintText "TCL: =========================================="
        $LOGIC PrintText "TCL: == Image Alignment CommandLine: $deformableType "
        $LOGIC PrintText "TCL: =========================================="

        ## check arguments

        if { $fixedVolumeNode == "" || [$fixedVolumeNode GetImageData] == "" } {
            PrintError "CMTKRegistration: fixed volume node not correctly defined"
            return ""
        }

        if { $movingVolumeNode == "" || [$movingVolumeNode GetImageData] == "" } {
            PrintError "CMTKRegistration: moving volume node not correctly defined"
            return ""
        }

        if { $outVolumeNode == "" } {
            PrintError "CMTKRegistration: output volume node not correctly defined"
            return ""
        }

        set fixedVolumeFileName [WriteDataToTemporaryDir $fixedVolumeNode Volume]
        if { $fixedVolumeFileName == "" } {
            # remove files
            return ""
        }
        set RemoveFiles "$fixedVolumeFileName"


        set movingVolumeFileName [WriteDataToTemporaryDir $movingVolumeNode Volume]
        if { $movingVolumeFileName == "" } {
            #remove files
            return ""
        }
        set RemoveFiles "$RemoveFiles $movingVolumeFileName"


        set outVolumeFileName [CreateTemporaryFileNameForNode $outVolumeNode]
        if { $outVolumeFileName == "" } {
            #remove files
            return ""
        }
        set RemoveFiles "$RemoveFiles $outVolumeFileName"

        ## CMTK specific arguments

        set CMD "$CMTKFOLDER/registration"

        if { $affineType == [$mrmlManager GetRegistrationTypeFromString RegistrationTest] } {
            set CMD "$CMD --dofs 0"
        } elseif { $affineType == [$mrmlManager GetRegistrationTypeFromString RegistrationFast] } {
            set CMD "$CMD --accuracy 0.5 --initxlate --exploration 8.0 --dofs 6 --dofs 9"
        } elseif { $affineType == [$mrmlManager GetRegistrationTypeFromString RegistrationSlow] } {
            set CMD "$CMD --accuracy 0.1 --initxlate --exploration 8.0 --dofs 6 --dofs 9"
        } else {
            PrintError "CMTKRegistration: Unknown affineType: $affineType"
            return ""
        }


        # affine
        set outLinearTransformDirName [CreateDirName "xform"]

        set outTransformDirName $outLinearTransformDirName

        set CMD "$CMD -o \"$outLinearTransformDirName\""
        set CMD "$CMD --write-reformatted \"$outVolumeFileName\""
        set CMD "$CMD \"$fixedVolumeFileName\""
        set CMD "$CMD \"$movingVolumeFileName\""


        ## execute affine registration

        $LOGIC PrintText "TCL: Executing $CMD"
        catch { eval exec $CMD } errmsg
        $LOGIC PrintText "TCL: $errmsg"

        if { $deformableType != 0 } {

            set CMD "$CMTKFOLDER/warp"
            
            # BSpline
            set outNonLinearTransformDirName [CreateDirName "xform"]
            set outTransformDirName $outNonLinearTransformDirName

            set CMD "$CMD --verbose"
            set CMD "$CMD --fast"
            if { $deformableType == [$mrmlManager GetRegistrationTypeFromString RegistrationTest] } {
                set CMD "$CMD --delta-f-threshold 1"
            } elseif { $deformableType == [$mrmlManager GetRegistrationTypeFromString RegistrationFast] } {
                set CMD "$CMD --grid-spacing 40 --refine 1"
                set CMD "$CMD --energy-weight 5e-2"
                set CMD "$CMD --accuracy 1 --coarsest 6"
            } elseif { $deformableType == [$mrmlManager GetRegistrationTypeFromString RegistrationSlow] } {
                set CMD "$CMD --grid-spacing 40 --refine 4"
                set CMD "$CMD --energy-weight 5e-2"
                set CMD "$CMD --exploration 16 --accuracy 0.1 --coarsest 6"
            } else {
                PrintError "CMTKRegistration: Unknown deformableType: $deformableType"
                return ""
            }
            
            set CMD "$CMD --initial \"$outLinearTransformDirName\""
            set CMD "$CMD -o \"$outNonLinearTransformDirName\""
            set CMD "$CMD --write-reformatted \"$outVolumeFileName\""
            set CMD "$CMD \"$fixedVolumeFileName\""
            set CMD "$CMD \"$movingVolumeFileName\""

            ## execute bspline registration

            $LOGIC PrintText "TCL: Executing $CMD"
            catch { eval exec $CMD } errmsg
            $LOGIC PrintText "TCL: $errmsg"
        }

        ## Read results back to scene
        if { [ReadDataFromDisk $outVolumeNode $outVolumeFileName Volume] == 0 } {
            if { [file exists $outVolumeDirName] == 0 } {
                set outTransformDirName ""
            }
        }

        if { [file exists $outTransformDirName] == 0 } {
            set outTransformDirName ""
        }

        # Test: 
        # $LOGIC PrintText "==> [[$SCENE GetNodeByID $transID] Print]"

        foreach NAME $RemoveFiles {
            file delete -force $NAME
        }

        # Remove Transformation from image
        $movingVolumeNode SetAndObserveTransformNodeID ""
        $SCENE Edited

        # return transformation directory name or ""
        puts "outTransformDirName: $outTransformDirName"
        return $outTransformDirName
    }

    proc CheckAndCorrectClassCovarianceMatrix {parentNodeID } {
        variable mrmlManager
        variable LOGIC
        set n [$mrmlManager GetTreeNodeNumberOfChildren $parentNodeID ]
        set failedList ""
        for {set i 0 } { $i < $n  } { incr i } {
            set id [ $mrmlManager GetTreeNodeChildNodeID $parentNodeID $i ] 
            if { [ $mrmlManager GetTreeNodeIsLeaf $id ] } {
                if { [$mrmlManager IsTreeNodeDistributionLogCovarianceWithCorrectionInvertableAndSemiDefinite $id ] == 0 } {
                    # set the off diagonal to zeo 
                    $LOGIC PrintText "TCL:CheckAndCorrectClassCovarianceMatrix: Set off diagonal of the LogCovariance of [ $mrmlManager GetTreeNodeName $id] to zero - otherwise matrix not convertable and semidefinite"
                    $mrmlManager SetTreeNodeDistributionLogCovarianceOffDiagonal $id  0
                    # if it still fails then add to list 
                    if { [$mrmlManager IsTreeNodeDistributionLogCovarianceWithCorrectionInvertableAndSemiDefinite $id ] == 0 } {
                        set failedList "${failedList}$id "
                    }
                }
            } else {
                set failedList "${failedList}[CheckAndCorrectClassCovarianceMatrix $id]"
            }
        }
        return "$failedList"
    }
    

    proc CheckAndCorrectTreeCovarianceMatrix { } {
        variable mrmlManager
        set rootID [$mrmlManager GetTreeRootNodeID]
        return "[CheckAndCorrectClassCovarianceMatrix $rootID]"
    }

    proc Progress {args} {
        variable LOGIC
        $LOGIC PrintTextNoNewLine "."
    }

    proc wget { url  fileName } {
        package require http
        variable LOGIC
        $LOGIC  PrintTextNoNewLine "Loading $url "
        if { [ catch { set r [http::geturl $url -binary 1 -progress ::EMSegmenterPreProcessingTcl::Progress ] } errmsg ] }  {
            $LOGIC  PrintText " " 
            PrintError "Could not download $url: $errmsg"
            return 1
        }

        set fo [open $fileName w]
        fconfigure $fo -translation binary
        puts -nonewline $fo [http::data $r]
        close $fo
        $LOGIC PrintText "\nSaving to $fileName\n"
        ::http::cleanup $r

        return 0
    }

    # returns 1 if error occured
    proc DownloadFileIfNeededAndSetURIToNULL { node origMRMLFileName forceFlag } {
        variable GUI
        variable LOGIC
        variable SCENE

        if { [$node GetClassName] != "vtkMRMLVolumeArchetypeStorageNode" } {
            PrintError "DownloadFileIfNeededAndSetURIToNULL: Wrong node type" 
            return 1 
        } 
        
        # ONLY WORKS FOR AB
        set URI "[$node GetURI ]"
        $node SetURI ""

        if {$forceFlag == 0 } {
            set oldFileName [$node GetFileName]
            if { "$oldFileName" != "" } {
                # Turn it into absolute file if it is not already
                if { "[ file pathtype oldFileName ]" != "absolute" } {
                    set oldFileName "[file dirname $origMRMLFileName ]$oldFileName" 
                }

                if { [file exists $oldFileName ] && [file isfile $oldFileName ] } {
                    # Must set it again bc path of scene might have changed so set it to absolute first 
                    $node SetFileName $oldFileName
                    return 0
                }
            }
        }
        
        if {$URI == ""} {
            PrintError "DownloadFileIfNeededAndSetURIToNULL: File does not exist and URI is NULL" 
            return 1
        }

        # Need to download file to temp directory
        # dry-run, XXXXXX cannot be in the middle of the name on some platforms (fc11)
        set CMD "mktemp -u \"[$GUI GetTemporaryDirectory]/XXXXXX\""
        catch { set basefilename [ eval exec $CMD ] } errmsg
        set NAME "_[file tail $URI]"
        set filename $basefilename$NAME
        $LOGIC PrintText "DEBUG: Created file: $filename"


        if { [wget $URI $fileName] } {
            return 1
        }


        $node SetFileName $fileName
        return 0 
    }

    proc ReplaceInSceneURINameWithFileName { mrmlFileName } {
        variable GUI
        variable LOGIC
        variable SCENE

        # Important so that it will write out all the nodes we are interested 
        set mrmlScene [vtkMRMLScene New]
        set num [$SCENE GetNumberOfRegisteredNodeClasses]
        for { set i 0 } { $i < $num } { incr i } {
            set node [$SCENE GetNthRegisteredNodeClass $i]
            if { ($node != "" ) } {
                set name [$node GetClassName ]
                $mrmlScene RegisterNodeClass $node
            }
        }
        
        set parser [vtkMRMLParser New]
        $parser SetMRMLScene $mrmlScene
        $parser SetFileName $mrmlFileName
        
        if { [$parser Parse] } {
            set errorFlag 0
        } else {
            set errorFlag 1
        }
        $parser Delete

        if {$errorFlag == 0 } { 
            # Download all the files if needed 
            set TYPE "vtkMRMLVolumeArchetypeStorageNode"
            set n [$mrmlScene GetNumberOfNodesByClass $TYPE]

            for { set i 0 } { $i < $n } { incr i } {
                if { [DownloadFileIfNeededAndSetURIToNULL [$mrmlScene GetNthNodeByClass $i $TYPE] $mrmlFileName 0 ] } {
                    set errorFlag 1
                }
            }

            if { $errorFlag == 0 } {
                # Set the new path of mrmlScene - by first setting scene to old path so that the function afterwards cen extract the file name  
                $mrmlScene SetURL $mrmlFileName
                set tmpFileName [CreateTemporaryFileNameForNode  $mrmlScene]
                $mrmlScene SetURL $tmpFileName
                $mrmlScene SetRootDirectory [file dirname $tmpFileName ] 
                $mrmlScene Commit $tmpFileName
            }
        } 
        $mrmlScene Delete

        if { $errorFlag } { 
            return "" 
        }        

        $LOGIC PrintText "TCL: Wrote modified $mrmlFileName to $tmpFileName"

        return $tmpFileName
    }

    # -------------------------------------
    # Generate ICC Mask for input image
    # if succesfull returns ICC Mask Node
    # otherwise returns nothing
    # -------------------------------------
    proc GenerateICCMask { inputAtlasVolumeNode inputAtlasICCMaskNode subjectVolumeNode } {
        variable LOGIC
        $LOGIC PrintText "TCL: =========================================="
        $LOGIC PrintText "TCL: == Generate ICC MASK (not yet implemented)"
        $LOGIC PrintText "TCL: =========================================="
        set EXE_DIR "$::env(Slicer3_HOME)/bin"
        set PLUGINS_DIR "[$::slicer3::Application GetPluginsDir]"

        # set CMD "$PLUGINS_DIR/DemonsRegistration --fixed_image $Scan2Image --moving_image $Scan1Image --output_image $Scan1ToScan2Image --output_field $Scan1ToScan2Deformation --num_levels 3 --num_iterations 20,20,20 --def_field_sigma 1 --use_histogram_matching --verbose"

        set CMD "$PLUGINS_DIR/DemonsRegistration --fixed_image $Scan2Image --moving_image $Scan1Image --output_image $Scan1ToScan2Image --output_field $Scan1ToScan2Deformation --num_levels 3 --num_iterations 20,20,20 --def_field_sigma 1 --use_histogram_matching --verbose"

        return 1
    }

    proc RemoveNegativeValues { targetNode } {
        variable LOGIC
        variable SCENE
        $LOGIC PrintText "TCL: =========================================="
        $LOGIC PrintText "TCL: == Remove Negative Values "
        $LOGIC PrintText "TCL: =========================================="

        # initialize
        set result ""

        # Run the algorithm on each subject image
        for { set i 0 } {$i < [$targetNode GetNumberOfVolumes] } { incr i } {
            # Define input
            set inputNode [$targetNode GetNthVolumeNode $i]
            if { $inputNode == "" } {
                PrintError "RemoveNegativeValues: the ${i}th target node is not defined!"
                foreach NODE $result { DeleteNode $NODE }
                return ""
            }

            set inputVolume [$inputNode GetImageData]
            if { $inputVolume == "" } {
                PrintError "RemoveNegativeValues: the ${i}th target node has no input data defined!"
                foreach NODE $result { DeleteNode $NODE }
                return ""
            }

            $LOGIC PrintText "TCL: Start thresholding target image - start"

            # Define output
            set outputNode [CreateVolumeNode $inputNode "[$inputNode GetName]_pos"]
            set outputVolume [vtkImageData New]
            $outputNode SetAndObserveImageData $outputVolume
            $outputVolume Delete

            # Thresholding
            set thresh [vtkImageThreshold New]
            $thresh SetInput $inputVolume

            # replace negative values
            $thresh ThresholdByLower 0
            $thresh SetReplaceIn 1
            $thresh SetInValue 0

            # keep positive values
            $thresh SetOutValue 0
            $thresh SetReplaceOut 0

            $thresh Update
            set outputVolume [$outputNode GetImageData]
            $outputVolume DeepCopy [$thresh GetOutput]
            $thresh Delete
            
            $LOGIC PrintText "TCL: Start thresholding target image - stop"

            # still in for loop, create a list of outputNodes
            set result "${result}$outputNode "
        }
        return $result
    }

    # -------------------------------------
    # Perform intensity correction
    # if succesfull returns a list of intensity corrected subject volume nodes
    # otherwise returns nothing
    #     ./Slicer3 --launch N4ITKBiasFieldCorrection --inputimage ../Slicer3/Testing/Data/Input/MRMeningioma0.nrrd --maskimage /projects/birn/fedorov/Meningioma_anonymized/Cases/Case02/Case02_Scan1ICC.nrrd corrected_image.nrrd recovered_bias_field.nrrd
    # -------------------------------------
    proc PerformIntensityCorrection { subjectICCMaskNode } {
        variable LOGIC
        variable alignedTargetNode
        $LOGIC PrintText "TCL: =========================================="
        $LOGIC PrintText "TCL: == Intensity Correction "
        $LOGIC PrintText "TCL: =========================================="

        return [N4ITKBiasFieldCorrectionCLI $alignedTargetNode $subjectICCMaskNode]
    }

    # subjectICCMaskNode will be ignored
    proc N4ITKBiasFieldCorrectionCLI { inputNode subjectICCMaskNode } {
        variable SCENE
        variable LOGIC
        $LOGIC PrintText "TCL: =========================================="
        $LOGIC PrintText "TCL: ==     N4ITKBiasFieldCorrectionCLI      =="
        $LOGIC PrintText "TCL: =========================================="

        set PLUGINS_DIR "[$::slicer3::Application GetPluginsDir]"
        set CMD "${PLUGINS_DIR}/N4ITKBiasFieldCorrection"

        # initialize
        set correctedSubjectVolumeNodeList ""

        # Run the algorithm on each volume
        for { set i 0 } { $i < [$inputNode GetNumberOfVolumes] } { incr i } {

            set inputVolumeNode [$inputNode GetNthVolumeNode $i]
            set inputVolumeData [$inputVolumeNode GetImageData]
            if { $inputVolumeData == "" } {
                PrintError "N4ITKBiasFieldCorrectionCLI: the ${i}th volume node has not input data defined!"
                foreach VolumeNode $correctedSubjectVolumeNodeList {
                    DeleteNode $VolumeNode
                }
                return ""
            }

            set tmpFileName [WriteDataToTemporaryDir $inputVolumeNode Volume]
            set RemoveFiles "\"$tmpFileName\""
            if { $tmpFileName == "" } {
                return 1
            }
            set CMD "$CMD --inputimage \"$tmpFileName\""

            # set tmpFileName [WriteDataToTemporaryDir $subjectICCMaskNode Volume ]
            # set RemoveFiles "$RemoveFiles \"$tmpFileName\""
            # if { $tmpFileName == "" } { 
            # return 1
            #     }
            # set CMD "$CMD --maskimag \"$tmpFileName\""

            # create a new node for our output-list
            set outputVolumeNode [CreateVolumeNode $inputVolumeNode "[$inputVolumeNode GetName]_N4corrected"]
            set outputVolumeData [vtkImageData New]
            $outputVolumeNode SetAndObserveImageData $outputVolumeData
            $outputVolumeData Delete

            set outputVolumeFileName [ CreateTemporaryFileNameForNode $outputVolumeNode ]
            $LOGIC PrintText "$outputVolumeFileName"
            if { $outputVolumeFileName == "" } {
                return 1
            }
            set CMD "$CMD --outputimage \"$outputVolumeFileName\""
            set RemoveFiles "$RemoveFiles \"$outputVolumeFileName\""

            # for test purposes(reduces execution time)
            # set CMD "$CMD --iterations \"3,2,1\""

            # set outbiasVolumeFileName [ CreateTemporaryFileNameForNode $outbiasVolumeFileName ]
            # if { $outbiasVolumeFileName == "" } {
            #     return 1
            # }
            # set CMD "$CMD --outputbiasfield \"$outbiasVolumeFileName\""

            # execute algorithm
            $LOGIC PrintText "TCL: Executing $CMD"
            catch { eval exec $CMD } errmsg
            $LOGIC PrintText "TCL: $errmsg"

            # Read results back, we have to read 2 results

            ReadDataFromDisk $outputVolumeNode $outputVolumeFileName Volume
            file delete -force $outputVolumeFileName

            # ReadDataFromDisk $outbiasVolumeNode $outbiasVolumeFileName Volume  
            # file delete -force $outbiasVolumeFileName

            # still in for loop, create a list of Volumes
            set correctedSubjectVolumeNodeList "${correctedSubjectVolumeNodeList}$outputVolumeNode "
            $LOGIC PrintText "TCL: List of volume nodes: $correctedSubjectVolumeNodeList"
        }
        return "$correctedSubjectVolumeNodeList"
    }




    # -------------------------------------
    # Compute intensity distribution through auto sampling
    # if succesfull returns 0
    # otherwise returns 1
    # -------------------------------------
    proc ComputeIntensityDistributions { } {
        variable LOGIC
        variable GUI
        variable mrmlManager
        $LOGIC PrintText "TCL: =========================================="
        $LOGIC PrintText "TCL: == Update Intensity Distribution "
        $LOGIC PrintText "TCL: =========================================="

        # return [$mrmlManager ComputeIntensityDistributionsFromSpatialPrior [$LOGIC GetModuleShareDirectory] [$preGUI GetApplication]]
        if { [$LOGIC ComputeIntensityDistributionsFromSpatialPrior $GUI] } {
            return 1
        }
        return 0
    }

    # -------------------------------------
    # Register Atlas to Subject
    # This function is changing/updating some nodes
    #
    # if succesfull returns 0
    # otherwise returns 1
    # -------------------------------------
    proc RegisterAtlas { alignFlag } {
        variable workingDN
        variable mrmlManager
        variable LOGIC
        variable alignedTargetNode
        variable inputAtlasNode
        variable outputAtlasNode
        variable inputSubParcellationNode
        variable outputSubParcellationNode
        variable selectedRegistrationPackage

        $LOGIC PrintText "TCL: =========================================="
        $LOGIC PrintText "TCL: == Register Atlas"
        $LOGIC PrintText "TCL: =========================================="


        set affineFlag [expr ([$mrmlManager GetRegistrationAffineType] != [$mrmlManager GetRegistrationTypeFromString RegistrationOff])]
        set bSplineFlag [expr ([$mrmlManager GetRegistrationDeformableType] != [$mrmlManager GetRegistrationTypeFromString RegistrationOff])]

        if {($alignFlag == 0) || (( $affineFlag == 0 ) && ( $bSplineFlag == 0 )) } {
            return [SkipAtlasRegistration]
        }

        $LOGIC PrintText "TCL: == ($affineFlag / $bSplineFlag) "
        $LOGIC PrintText "TCL: =========================================="


        # ----------------------------------------------------------------
        # Setup
        # ----------------------------------------------------------------
        if { $outputAtlasNode == "" } {
            $LOGIC PrintText "TCL: Aligned Atlas was empty"
            #  $LOGIC PrintText "TCL: set outputAtlasNode \[ $mrmlManager CloneAtlasNode $inputAtlasNode \"AlignedAtlas\"\] "
            set outputAtlasNode [ $mrmlManager CloneAtlasNode $inputAtlasNode "Aligned"]
            $workingDN SetReferenceAlignedAtlasNodeID [$outputAtlasNode GetID]
        } else {
            $LOGIC PrintText "TCL: Atlas was just synchronized"
            $mrmlManager SynchronizeAtlasNode $inputAtlasNode $outputAtlasNode "Aligned"
        }

        if { $outputSubParcellationNode == "" } {
            $LOGIC PrintText "TCL: Aligned SubParcellation was empty"
            #  $LOGIC PrintText "TCL: set outputSubParcellationNode \[ $mrmlManager CloneSubParcellationNode $inputSubParcellationNode \"AlignedSubParcellation\"\] "
            set outputSubParcellationNode [ $mrmlManager CloneSubParcellationNode $inputSubParcellationNode "Aligned"]
            $workingDN SetReferenceAlignedSubParcellationNodeID [$outputSubParcellationNode GetID]
        } else {
            $LOGIC PrintText "TCL: SubParcellation was just synchronized"
            $mrmlManager SynchronizeSubParcellationNode $inputSubParcellationNode $outputSubParcellationNode "Aligned"
        }

        # TODO, don't activate it per default
        set stripped 1

        if { $stripped == 0 } {
            set alignedTargetNode_SkullStripped [SkullStripperCLI $alignedTargetNode]
            UpdateVolumeCollectionNode "$alignedTargetNode" "$alignedTargetNode_SkullStripped"
        }


        set fixedTargetChannel 0
        set fixedTargetVolumeNode [$alignedTargetNode GetNthVolumeNode $fixedTargetChannel]
        
        if { [$fixedTargetVolumeNode GetImageData] == "" } {
            PrintError "RegisterAtlas: Fixed image is null, skipping registration"
            return 1
        }

        set atlasRegistrationVolumeIndex -1;
        if {[[$mrmlManager GetGlobalParametersNode] GetRegistrationAtlasVolumeKey] != "" } {
            set atlasRegistrationVolumeKey [[$mrmlManager GetGlobalParametersNode] GetRegistrationAtlasVolumeKey]
            set atlasRegistrationVolumeIndex [$inputAtlasNode GetIndexByKey $atlasRegistrationVolumeKey]
        }

        if {$atlasRegistrationVolumeIndex < 0 } {
            PrintError "RegisterAtlas: Attempt to register atlas image but no atlas image selected!"
            return 1
        }

        set movingAtlasVolumeNode [$inputAtlasNode GetNthVolumeNode $atlasRegistrationVolumeIndex]
        set movingAtlasImageData [$movingAtlasVolumeNode GetImageData]

        set outputAtlasVolumeNode [$outputAtlasNode GetNthVolumeNode $atlasRegistrationVolumeIndex]
        set outAtlasImageData [$outputAtlasVolumeNode GetImageData]

        if { $movingAtlasImageData == "" } {
            PrintError "RegisterAtlas: Moving image is null, skipping"
            return 1
        }

        if {$outAtlasImageData == "" } {
            PrintError "RegisterAtlas: Registration output is null, skipping"
            return 1
        }

        set affineType [ $mrmlManager GetRegistrationAffineType ]
        set deformableType [ $mrmlManager GetRegistrationDeformableType ]
        set interpolationType [ $mrmlManager GetRegistrationInterpolationType ]

        if { 0 } {
            set fixedRASToMovingRASTransformAffine [ vtkTransform New]
            set fixedRASToMovingRASTransformDeformable ""
        }

        $LOGIC PrintText "TCL: ========== Info ========="
        $LOGIC PrintText "TCL: = Fixed:   [$fixedTargetVolumeNode GetName] "
        $LOGIC PrintText "TCL: = Moving:  [$movingAtlasVolumeNode GetName] "
        $LOGIC PrintText "TCL: = Affine:  $affineType"
        $LOGIC PrintText "TCL: = BSpline: $deformableType"
        $LOGIC PrintText "TCL: = Interp:  $interpolationType"
        $LOGIC PrintText "TCL: ========================="

        # ----------------------------------------------------------------
        # registration
        # ----------------------------------------------------------------

        
        set backgroundLevel [$LOGIC GuessRegistrationBackgroundLevel $movingAtlasVolumeNode]
        set transformDirName "" 
        set transformNode ""
        set transformNodeType ""
        
        
        
        switch -exact "$selectedRegistrationPackage" {
            "CMTK" {
                set transformDirName [CMTKRegistration $fixedTargetVolumeNode $movingAtlasVolumeNode $outputAtlasVolumeNode $backgroundLevel $deformableType $affineType]
                if { $transformDirName == "" } {
                    PrintError "RegisterAtlas: Transform node is null"
                    return 1
                }
                set transformNodeType "CMTKTransform"

                $LOGIC PrintText "TCL: Resampling atlas in CMTKRegistration ..."
                if { [Resample $movingAtlasVolumeNode $fixedTargetVolumeNode $transformNode $transformDirName $transformNodeType Linear $backgroundLevel $outputAtlasVolumeNode] } {
                    PrintError "RegisterAtlas: Could not resample(reformatx) atlas volume"
                    return 1
                }
            }
            "BRAINS" {
                set BSplineNode [BRAINSRegistration $fixedTargetVolumeNode $movingAtlasVolumeNode $outputAtlasVolumeNode $backgroundLevel $deformableType $affineType]
                if { $BSplineNode == "" } {
                    PrintError "RegisterAtlas: BSpline Transform node is null"
                    return 1
                }
                $LOGIC PrintText "TCL: RegisterAtlas: calcDFVolumeNode START"
                set transformNode [calcDFVolumeNode $movingAtlasVolumeNode $fixedTargetVolumeNode $BSplineNode]
                $LOGIC PrintText "TCL: RegisterAtlas: calcDFVolumeNode DONE, succesfull? FIXME, TODO"
                if { $transformNode == "" } {
                    PrintError "RegisterAtlas: Deformation Field Transform node is null"
                    return 1
                }
                set transformNodeType "DeformVolumeTransform"  
            }
            default {
                PrintError "registration package not known"
                return 1
            }
        }
        

        # ----------------------------------------------------------------
        # resample
        # ----------------------------------------------------------------

        # Spatial prior
        for { set i 0 } { $i < [$outputAtlasNode GetNumberOfVolumes] } { incr i } {
            if { $i == $atlasRegistrationVolumeIndex} { continue }
            $LOGIC PrintText "TCL: Resampling atlas image $i ..."
            set movingVolumeNode [$inputAtlasNode GetNthVolumeNode $i]
            set outputVolumeNode [$outputAtlasNode GetNthVolumeNode $i]
            set backgroundLevel [$LOGIC GuessRegistrationBackgroundLevel $movingVolumeNode]
            $LOGIC PrintText "TCL: Guessed background level: $backgroundLevel"

            if { [Resample $movingVolumeNode $fixedTargetVolumeNode $transformNode $transformDirName $transformNodeType Linear $backgroundLevel $outputVolumeNode] } {
                return 1
            }
        }
        
        # Sub parcelation
        for { set i 0 } { $i < [$outputSubParcellationNode GetNumberOfVolumes] } { incr i } {
            $LOGIC PrintText "TCL: Resampling subparcallation map  $i ..."
            set movingVolumeNode [$inputSubParcellationNode GetNthVolumeNode $i]
            set outputVolumeNode [$outputSubParcellationNode GetNthVolumeNode $i]
            if { [Resample $movingVolumeNode  $fixedTargetVolumeNode  $transformNode "$transformDirName" $transformNodeType NearestNeighbor 0 $outputVolumeNode] } {
                return 1
            }

            # Create Voronoi diagram with correct scalar type from aligned subparcellation 
            GeneratedVoronoi [$outputVolumeNode GetImageData]
        }

        $LOGIC PrintText "TCL: Atlas-to-target registration complete."
        $workingDN SetAlignedAtlasNodeIsValid 1
        return 0
    }

    
    # output: outputVolumeNode
    # no side effects
    proc Resample { inputVolumeNode referenceVolumeNode transformNode transformDirName transformType interpolationType backgroundLevel outputVolumeNode } {
        variable LOGIC
        if {[$inputVolumeNode GetImageData] == ""} {
            PrintError "Resample: Input image is null, skipping: $inputVolumeNode"
            return 1
        }
        if {[$outputVolumeNode GetImageData] == ""} {
            PrintError "Resample: Registration output is null, skipping: $outputVolumeNode"
            return 1
        }

        $LOGIC PrintText "TCL: Resampling image ..."

        switch $transformType {
            "CMTKTransform" {
                $LOGIC PrintText "TCL: with CMTKResampleCLI..."
                if { [CMTKResampleCLI $inputVolumeNode $referenceVolumeNode $outputVolumeNode $transformDirName $backgroundLevel] } {
                    return 1
                }
            }
            "BSplineTransform" {
                $LOGIC PrintText "TCL: with BRAINSResample using the BSpline"
                set BRAINSBSpline 1
                if { [BRAINSResampleCLI $inputVolumeNode $referenceVolumeNode $outputVolumeNode $transformNode $backgroundLevel $interpolationType $BRAINSBSpline] } {
                    return 1
                }
            }
            "DeformVolumeTransform" {
                $LOGIC PrintText "TCL: with BRAINSResample using the Deformation Field"
                set BRAINSBSpline 0
                if { [BRAINSResampleCLI $inputVolumeNode $referenceVolumeNode $outputVolumeNode $transformNode $backgroundLevel $interpolationType $BRAINSBSpline] } {
                    return 1
                }
            }
            default {
                PrintError "Unknown transformType $transformType"
                return 1
            }
        }
        return 0 
    }



    # Print error messages
    proc PrintError { TEXT } {
        variable LOGIC
        $LOGIC PrintText "TCL: ERROR: EMSegmenterPreProcessingTcl::${TEXT}"
    }


}

namespace eval EMSegmenterSimpleTcl {

    variable inputChannelGUI
    variable mrmlManager

    proc InitVariables { {GUI ""} } {
        variable inputChannelGUI
        variable mrmlManager
        if {$GUI == "" } {
            set GUI [$::slicer3::Application GetModuleGUIByName EMSegmenter]
        }
        if { $GUI == "" } {
            PrintError "InitVariables: GUI not defined"
            return 1
        }
        set mrmlManager [$GUI GetMRMLManager]
        if { $mrmlManager == "" } {
            PrintError "InitVariables: mrmManager not defined"
            return 1
        }
        set inputChannelGUI [$GUI GetInputChannelStep]
        if { $inputChannelGUI == "" } {
            PrintError "InitVariables: InputChannelStep not defined"
            return 1
        }
        return 0
    }

    proc PrintError { TEXT } {
        puts stderr "TCL: ERROR:EMSegmenterSimpleTcl::${TEXT}"
    }

    # 0 = Do not create a check list for the simple user interface
    # simply remove
    # 1 = Create one - then also define ShowCheckList and
    #     ValidateCheckList where results of checklist are transfered to Preprocessing
    proc CreateCheckList { } { return 0 }
    proc ShowCheckList { } { return 0 }
    proc ValidateCheckList { } { return 0 }
}
