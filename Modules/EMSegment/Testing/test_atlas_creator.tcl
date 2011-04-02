#./Slicer3 --no-splash -p ../Slicer3/Modules/EMSegment/Testing/test_atlas_creator.tcl

set Slicer3_HOME $::env(Slicer3_HOME)
puts $Slicer3_HOME


set mrmlScene $::slicer3::MRMLScene
$mrmlScene SetURL $Slicer3_HOME/../Slicer3/Modules/EMSegment/Tasks/MRI-Human-Brain.mrml


puts "Start"
# Load In volume 
#set targetFileName "$Slicer3_HOME/../Slicer3/Modules/EMSegment/Tasks/MRI-Human-Brain/atlas_skulneck.nrrd"
set targetFileName "$Slicer3_HOME/../Slicer3/Modules/AtlasCreator/TestData/originals/case62.nrrd"

set MODVol [$::slicer3::Application GetModuleGUIByName "Volumes"]
set MODLogic [$MODVol GetLogic]

set MODEMS [$::slicer3::Application GetModuleGUIByName "EMSegmenter"]
set mrmlManager [$MODEMS GetMRMLManager]

set volumeNode [$MODLogic AddArchetypeVolume $targetFileName "TargetImage" 0]
puts "[$volumeNode GetID]"
$mrmlManager AddTargetSelectedVolumeByMRMLID [$volumeNode GetID]

set targetNode [$workingDN GetAlignedTargetNode]

source $Slicer3_HOME/../Slicer3/Modules/EMSegment/Tasks/GenericTask.tcl

set outputDir        "/tmp/AC/"
set segmentationsDir "$Slicer3_HOME/../Slicer3/Modules/AtlasCreator/TestData/segmentations/"
set imagesDir        "$Slicer3_HOME/../Slicer3/Modules/AtlasCreator/TestData/originals/"

EMSegmenterPreProcessingTcl::AtlasCreator $segmentationsDir $imagesDir $outputDir $targetNode

puts "End"
