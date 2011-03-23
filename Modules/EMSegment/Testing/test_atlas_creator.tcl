#./Slicer3 --no-splash -p ../Slicer3/Modules/EMSegment/Testing/test_atlas_creator.tcl
set mrmlScene $::slicer3::MRMLScene

set Slicer3_HOME $::env(Slicer3_HOME)
puts $Slicer3_HOME

set targetFileName $Slicer3_HOME/../Slicer3/Modules/EMSegment/Tasks/MRI-Human-Brain/atlas_skulneck.nrrd
$mrmlScene SetURL $Slicer3_HOME/../Slicer3/Modules/EMSegment/Tasks/MRI-Human-Brain.mrml

set outputDir        "/tmp/AC/"
set template         "$Slicer3_HOME/../Slicer3/Modules/AtlasCreator/TestData/originals/case62.nrrd"
set segmentationsDir "$Slicer3_HOME/../Slicer3/Modules/AtlasCreator/TestData/segmentations/"
set imagesDir        "$Slicer3_HOME/../Slicer3/Modules/AtlasCreator/TestData/originals/"

puts "Start"
# Load In volume 
set MODVol [$::slicer3::Application GetModuleGUIByName "Volumes"]
set MODLogic [$MODVol GetLogic]
set targetNode [$MODLogic AddArchetypeVolume $targetFileName "TargetImage" 0 ]

source $Slicer3_HOME/../Slicer3/Modules/EMSegment/Tasks/GenericTask.tcl
EMSegmenterPreProcessingTcl::AtlasCreator $template $segmentationsDir $imagesDir $outputDir "-1"

puts "End"
