proc VolumeReader { i  filePrefix } {

    set Extension [file extension "$filePrefix"]

    vtkITKArchetypeImageSeriesScalarReader Volume($i,vol)
    set VOL Volume($i,vol)

   $VOL SetArchetype $filePrefix
   if {![$VOL CanReadFile $filePrefix]} {
  puts "ERROR:ITK_Generic_Reader: Cannot read $filePrefix"
  exit 1
   }
   puts "Reading $filePrefix"

   if {[catch "$VOL UpdateInformation" res]} {
        puts "Error:ITK_Generic_Reader: Cannot read information for file $filePrefix\n\n$res"
        exit 1 
   }

   $VOL SetOutputScalarTypeToNative
   # $VOL SetOutputScalarTypeTo$Volume($i,scalarType)
   
   # $VOL SetDesiredCoordinateOrientationToNative
   $VOL SetUseNativeOriginOff

   $VOL Update
}
proc VolumeMathWriter {DATA DIR  FILE  FORMAT MATRIX  } {
    if {[file isdirectory $DIR] == 0} {
  catch {file mkdir $DIR}
       puts "Make Directory $DIR"
    }

    set export_iwriter  [vtkITKImageWriter New] 
    $export_iwriter SetInput $DATA 
    set FileName $DIR/$FILE$FORMAT 
    $export_iwriter SetFileName $FileName
    $export_iwriter SetRasToIJKMatrix $MATRIX
    $export_iwriter SetUseCompression 1                  
    # Write volume data
    puts "Writing $FileName ..." 
    $export_iwriter Write
    $export_iwriter Delete

    # set index 1
    # set FILE "$DIR/PCAModellingStep"
    # while {[file exists ${FILE}${index}.log]} { incr index }
    # WriteASCIIFile ${FILE}${index}.log "PCA_Modelling $::argv_orig"    
}

