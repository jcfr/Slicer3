'''
    Launcher for the Atlas Creator
'''
import sys
import os
import getopt



'''=========================================================================================='''
def info(message):
    '''
        Print message to stdout and flush
    '''
    print str(message)
    import sys
    # flush, to always show the output
    sys.stdout.flush()



'''=========================================================================================='''
def is_int(s):
    try:
        int(s)
        return True
    except ValueError:
        return False
    
    
    
'''=========================================================================================='''
def usage():
    '''
        Print the help information.
    '''
    info("Usage:")
    info("")
    info("-h, --help")
    info("        Show this information.")
    info("")
    info("-i, --images DIR")
    info("        Directory containing original images.")
    info("")
    info("-s, --segmentations DIR")
    info("        Directory containing segmentations.")
    info("")
    info("-o, --output DIR")
    info("        Output directory.")
    info("")
    info("--cmtk")
    info("        Use the CMTK toolkit for registration and resampling, instead of BRAINSFit.")
    info("        The CMTK4Slicer extensions have to be installed in order to use CMTK.")
    info("")
    info("--skipRegistration")
    info("        Skip the registration and use existing transforms.")
    info("")
    info("        The following arguments have to be specified if the registration is skipped:")
    info("")
    info("        --transforms DIR")
    info("                Directory containing existing transforms.")
    info("")
    info("        --existingTemplate FILEPATH")
    info("                Filepath to an existing template used for resampling only.")
    info("")
    info("--dynamic")
    info("        Use a dynamic template for registration based on means of images.")
    info("")
    info("        The following arguments have to be specified if dynamic registration is chosen:")
    info("")    
    info("        -m, --meanIterations INT")
    info("                Number of iterations to compute and register against a mean image.")
    info("")
    info("--fixed")
    info("        Use a fixed template for registration.")
    info("")
    info("        The following arguments have to be specified if fixed registration is chosen:")    
    info("")    
    info("        --template FILEPATH")
    info("                Filepath to an image used as a template for fixed registration.")
    info("")
    info("-n, --non-rigid")
    info("        Use Non-Rigid registration additionally.")
    info("")
    info("-w, --writeTransforms")
    info("        Write transforms to output directory.")
    info("")
    info("--keepAligned")
    info("        Keep the aligned images and segmentations.")
    info("")    
    info("-l, --labels STRING")
    info("        List of labels to include for the atlases, f.e. \"3 4 5 6 8 10\".")
    info("")
    info("--normalize")
    info("        Normalize Atlases to 0..1.")
    info("        If activated, the output cast will be set to Double.")
    info("")
    info("--outputCast INT")
    info("        Output cast for the atlases. Possible values:")
    info("        0: Char")
    info("        1: Unsigned Char")
    info("        2: Double")
    info("        3: Float")
    info("        4: Int")
    info("        5: Unsigned Int")
    info("        6: Long")
    info("        7: Unsigned Long")
    info("        8: Short")
    info("        9: Unsigned Short")        
    info("        DEFAULT: 8")
    info("")
    info("-c, --cluster")
    info("        Use the cluster mode.")
    info("")
    info("        The following arguments have to be specified if cluster mode is chosen:")
    info("")
    info("        --schedulerCommand EXECUTABLE")
    info("                The executable to use as a scheduler in cluster mode, f.e. \"qsub\".")
    info("")
    info("--slicer FILEPATH")
    info("        Filepath to the 3D Slicer launcher including arguments, f.e. \"/usr/bin/Slicer3 --tmp_dir /var/tmp\".")
    info("        DEFAULT: Find the 3D Slicer launcher automatically.")
    info("")
    info("-d, --debug")
    info("        Enable debug information.")
    info("")
    info("--dryrun")
    info("        Output executable commands instead of running the registration or resampling.")
    info("")
    info("--examples")
    info("        Show usage examples.")
    info("")
    info("")
    info("Developed by Daniel Haehn and Kilian Pohl, University of Pennsylvania. The research was funded by an ARRA supplement to NIH NCRR (P41 RR13218).")
    info("")
    info("Thanks to everyone!")
    info("")



'''=========================================================================================='''
def examples():

    info("Examples:")
    info("-----------------------------------------------------------------------------------------------")
    info("1. Run fixed registration with the testdata:")
    info("")
    info('        python atlascreator.py -i TestData/originals/ -s TestData/segmentations/ -o /tmp/acout --fixed --template TestData/originals/case62.nrrd -w -l "3 4 5 6 7 8 9" --normalize')
    info("")
    info("-----------------------------------------------------------------------------------------------")
    info("2. Run fixed registration with the testdata and use CMTK instead of BRAINSFit:")
    info("")
    info('        python atlascreator.py -i TestData/originals/ -s TestData/segmentations/ -o /tmp/acout --fixed --template TestData/originals/case62.nrrd -w -l "3 4 5 6 7 8 9" --normalize --cmtk')
    info("")    
    info("-----------------------------------------------------------------------------------------------")
    info("3. Run dynamic registration with the testdata:")
    info("")
    info('        python atlascreator.py -i TestData/originals/ -s TestData/segmentations/ -o /tmp/acout --dynamic --meanIterations 5 -w -l "3 4 5 6 7 8 9" --normalize')
    info("")
    info("-----------------------------------------------------------------------------------------------")
    info("4. Run dynamic registration with the testdata on a cluster (scheduler command \"qsub\"):")
    info("")
    info('        python atlascreator.py -i TestData/originals/ -s TestData/segmentations/ -o /tmp/acout --dynamic --meanIterations 5 -w -l "3 4 5 6 7 8 9" --normalize --cluster --schedulerCommand \"qsub -b y\"')
    info("")
    info("-----------------------------------------------------------------------------------------------")
    info("5. Use existing registrations and just re-sample")
    info("")
    info('        python atlascreator.py --skipRegistration --transforms /tmp/acout --existingTemplate TestData/segmentations/case62.nrrd -s TestData/segmentations/ -o /tmp/acout -l "3 4 5 6 7 8 9" --normalize --outputCast 3')
    info("")



'''=========================================================================================='''
def main(argv):                          
    '''
        Starting point, parse the command line args.
    '''
    
    info("AtlasCreator for 3D Slicer")
    info("Version v0.24")
    info("")
    
    if len(argv) == 0:
        usage()  
        info("Error: No arguments specified.")
        info("")                      
        sys.exit(2)
    
    try:
        opts, args = getopt.getopt(argv, "hdi:s:o:m:nwl:c", ["help",
                                                        "images=",
                                                        "segmentations=",
                                                        "output=",
                                                        "cmtk",
                                                        "skipRegistration",
                                                        "transforms=",
                                                        "existingTemplate=",
                                                        "dynamic",
                                                        "meanIterations=",
                                                        "fixed",
                                                        "template=",                                                        
                                                        "non-rigid",
                                                        "writeTransforms",
                                                        "keepAligned",
                                                        "labels=",
                                                        "normalize",
                                                        "outputCast=",
                                                        "cluster",
                                                        "schedulerCommand=",
                                                        "slicer=",
                                                        "debug",
                                                        "dryrun",
                                                        "examples"])
        
    except getopt.GetoptError, err:
        usage()                    
        info("Error: " + str(err))
        info("")
        sys.exit(2)
    
    # set some default switches
    skipRegistration = False
    slicerLauncherFilePath = None
    imagesDir = None
    segmentationsDir = None
    outputDir = None
    
    useCMTK = False
    
    transformsDir = None
    existingTemplate = None
    
    dynamic = False
    meanIterations = 0
    
    fixed = False
    template = None
    
    nonRigid = False
    
    writeTransforms = False
    
    keepAligned = False
    
    labels = None
    
    normalize = False
    
    outputCast = 8
    
    cluster = False
    schedulerCommand = None
    
    debug = False
    
    dryrun = False
    
    for opt, arg in opts:                
        if opt in ("-h", "--help"):
            usage()     
            sys.exit()         
        elif opt in ("-i", "--images"):
            imagesDir = arg
        elif opt in ("-s", "--segmentations"):
            segmentationsDir = arg
        elif opt in ("-o", "--output"):
            outputDir = arg
        elif opt in ("--cmtk"):
            useCMTK = True
        elif opt in ("--skipRegistration"):
            skipRegistration = True
        elif opt in ("--transforms"):
            transformsDir = arg
        elif opt in ("--existingTemplate"):
            existingTemplate = arg
        elif opt in ("--dynamic"):
            dynamic = True
        elif opt in ("-m", "--meanIterations"):
            meanIterations = arg
        elif opt in ("--fixed"):
            fixed = True
        elif opt in ("--template"):
            template = arg
        elif opt in ("-n", "--non-rigid"):
            nonRigid = True
        elif opt in ("-w", "--writeTransforms"):
            writeTransforms = True
        elif opt in ("--keepAligned"):
            keepAligned = True
        elif opt in ("-l", "--labels"):
            labels = arg.strip().split(" ")
        elif opt in ("--normalize"):
            normalize = True
        elif opt in ("--outputCast"):
            outputCast = arg
        elif opt in ("-c", "--cluster"):
            cluster = True
        elif opt in ("--schedulerCommand"):
            schedulerCommand = arg
        elif opt in ("--slicer"):
            slicerLauncherFilePath = arg
        elif opt in ("-d", "--debug"):
            debug = True
        elif opt in ("--dryrun"):
            debug = True
            dryrun = True      
        elif opt in ("--examples"):
            examples()
            sys.exit()
        else:
            assert False, "unhandled option"

    # now check if we have all we need
    errorOccured = False
    
    
    # we need 3D Slicer, find the launcher
    if not slicerLauncherFilePath:
        # try to find the slicer launcher automatically
        slicerLauncherFilePath = sys.path[0] + os.sep + "../../../../Slicer3"
        slicerLauncherFilePath = os.path.abspath(slicerLauncherFilePath)
        if os.path.isfile(slicerLauncherFilePath):
            # running in bin directory
            info("Found 3D Slicer launcher at " + str(slicerLauncherFilePath))
        else:
            slicerLauncherFilePath = sys.path[0] + os.sep + "../../../Slicer3-build/Slicer3"
            slicerLauncherFilePath = os.path.abspath(slicerLauncherFilePath)
            if os.path.isfile(slicerLauncherFilePath):
                # running in src directory
                info("Found 3D Slicer3 launcher at " + str(slicerLauncherFilePath))
            else:
                info("Error: Could not find the 3D Slicer launcher!")
                info("Error: Try to specify the location with --slicer /path/Slicer3")
                errorOccured = True
    else:
        info("3D Slicer manually specified: " + str(slicerLauncherFilePath))
        
        
    # check if we have a valid images dir
    if not (imagesDir and os.path.isdir(imagesDir)) and not skipRegistration:
        # no valid imagesDir and registration is not skipped
        # we have to abort
        info("Error: Could not find the images!")
        info("Error: Location of --images is invalid: " + str(imagesDir))
        errorOccured = True
    elif imagesDir:
        imagesDir = os.path.abspath(imagesDir) + os.sep
        
    if not (segmentationsDir and os.path.isdir(segmentationsDir)):
        # no valid segmentationsDir
        # we have to abort
        info("Error: Could not find the segmentations!")
        info("Error: Location of --segmentations is invalid: " + str(segmentationsDir))
        errorOccured = True
    elif segmentationsDir:
        segmentationsDir = os.path.abspath(segmentationsDir) + os.sep
    
    if outputDir and os.path.isdir(outputDir):
        # outputDir already exists
        # we create a new unique one
        outputDir = os.path.abspath(outputDir)
        info("Warning: The output directory ("+str(outputDir)+") already exists..")
        
        # the directory already exists,
        # we want to add an index to the new one
        count = 2
        newOutputDir = os.path.abspath(str(outputDir)+str(count))
        
        while (os.path.isdir(newOutputDir)):
            count = count + 1
            newOutputDir = os.path.abspath(str(outputDir)+str(count))
        
        info("Warning: Using new output directory instead: " + str(newOutputDir))
        os.makedirs(newOutputDir)
        
        outputDir = newOutputDir + os.sep
        
    elif outputDir and not os.path.isfile(outputDir):
        # outputDir did not exist and is not a file
        # create it
        outputDir = os.path.abspath(outputDir) + os.sep
        os.makedirs(outputDir)
        info("Created output directory: " + str(outputDir))
    else:
        info("Error: Location of --output is invalid or not a directory: " + str(outputDir))
        errorOccured = True   
        
    # check if we have everything if skipRegistration is enabled
    if skipRegistration and transformsDir and existingTemplate:
        # check if transformDir and existingTemplate are not valid
        if not os.path.isdir(transformsDir):
            # transformDir invalid
            info("Error: Could not find the directory of existing transforms!")
            info("Error: Location of --transforms is invalid: "+str(transformsDir))
            errorOccured = True
        else:
            transformsDir = os.path.abspath(transformsDir) + os.sep
        if not os.path.isfile(existingTemplate):
            # existingTemplate invalid 
            info("Error: Could not find the existing template!")
            info("Error: Location of --existingTemplate is invalid: "+str(existingTemplate))
            errorOccured = True
        else:
            existingTemplate = os.path.abspath(existingTemplate)
    elif skipRegistration:
        # we don't have everything, abort!
        info("Error: To skip the registration, --transforms and --existingTemplate are required!")
        info("Error: Location of --transforms is invalid: "+str(transformsDir))
        info("Error: Location of --existingTemplate is invalid: "+str(existingTemplate))
        errorOccured = True
        
    # check if either dynamic or fixed registration is configured
    if not dynamic and not fixed and not skipRegistration:
        info("Error: The registration type was not set. Either --fixed or --dynamic are required!")
        errorOccured = True
       
    # check if both, dynamic and fixed were configured
    if dynamic and fixed:
        info("Error: Either --fixed or --dynamic are required - not both!")
        errorOccured = True
        
    # check if we have everything if it is dynamic mode
    if dynamic and meanIterations:
        # check if meanIterations is valid
        if not is_int(meanIterations) or not (int(meanIterations) >= 1):
            info("Error: The value of --meanIterations has to be an INT greater than 1.")
            if not is_int(meanIterations):
                info("Error: Value of --meanIterations is invalid: NaN")
            else:
                info("Error: Value of --meanIterations is invalid: " + str(int(meanIterations)))
            errorOccured = True
    elif dynamic:
        # we don't have everything, abort!
        info("Error: For dynamic registration, --meanIterations is required!")
        info("Error: Value of --meanIterations is invalid: " + str(int(meanIterations)))
        errorOccured = True
        
    # check if we have everything if it is fixed mode
    if fixed and template:
        # check if template is valid
        if not os.path.isfile(template):
            # existingTemplate invalid 
            info("Error: Could not find the template!")
            info("Error: Location of --template is invalid: "+str(template))
            errorOccured = True       
        else:
            template = os.path.abspath(template)
    elif fixed:
        # we don't have everything, abort!
        info("Error: For fixed registration, --template is required!")
        info("Error: Location of --template is invalid: " + str(template))
        errorOccured = True
        
    # check if at least one label was specified
    if labels and type(labels).__name__=='list' and len(labels) >= 1:
        # convert to integer list
        realLabels = []
        
        for l in labels:
            if is_int(l):
                realLabels.append(int(l))
        
        if len(realLabels) < 1:
            # invalid label list
            info("Error: At least one label (INT) is required!")
            info("Error: Value of --labels: " + str(labels))
            errorOccured = True
        else:
            labels = list(set(realLabels))
    else:
        info("Error: At least one label (INT) is required!")
        info("Error: Value of --labels: " + str(labels))
        errorOccured = True
        
    # check if we have a valid outputCast
    if outputCast and is_int(outputCast):
        
        if not (int(outputCast) > 0 and int(outputCast) <= 9):
            info("Error: The specified output cast is invalid - only values from 0-9 accepted.")
            info("Error: Value of --outputCast: " + str(outputCast))
            errorOccured = True
        
        # valid outputCast integer, convert to string
        if int(outputCast) == 0:
            outputCast = "Char"
        elif int(outputCast) == 1:
            outputCast = "Unsigned Char"
        elif int(outputCast) == 2:
            outputCast = "Double"
        elif int(outputCast) == 3:
            outputCast = "Float"
        elif int(outputCast) == 4:
            outputCast = "Int"
        elif int(outputCast) == 5:
            outputCast = "Unsigned Int"
        elif int(outputCast) == 6:
            outputCast = "Long"
        elif int(outputCast) == 7:
            outputCast = "Unsigned Long"
        elif int(outputCast) == 8:
            outputCast = "Short"
        elif int(outputCast) == 9:
            outputCast = "Unsigned Short"
        
    elif outputCast:
        info("Error: The specified output cast is invalid - only values from 0-9 accepted.")
        info("Error: Value of --outputCast: " + str(outputCast))
        errorOccured = True
    
    # check if we have everything if cluster mode is activated
    if cluster and not schedulerCommand:
        info("Error: In cluster mode, a schedulerCommand is required.")
        info("Error: Value of --schedulerCommand: "+str(schedulerCommand))
            
    if errorOccured:
        info("")
        info("Try --help or --examples to understand the usage of the Atlas Creator.")
        info("")
        sys.exit(2)
        
    #no error occured, so give some feedback        
    if debug:
        info("Debug Mode is enabled!")
        
    if dryrun:
        info("Dry-Run is activated: Output executable commands instead of running the registration or resampling.")

    # lets create the --evalpython command!!
    evalpythonCommand = "from Slicer import slicer;import sys;import os;"
    evalpythonCommand += "pathToAtlasCreator = os.path.normpath(str(slicer.Application.GetPluginsDir())+'"
    evalpythonCommand += "/../Modules/AtlasCreator');"
    evalpythonCommand += "sys.path.append(pathToAtlasCreator);"
    evalpythonCommand += "from AtlasCreatorGUI import *;"
    evalpythonCommand += "gui = AtlasCreatorGUI();"

    if debug or dryrun:
        evalpythonCommand += "gui.GetHelper().EnableDebugMode();"

    evalpythonCommand += "logic = gui.GetMyLogic();"
    
    if dryrun:
        evalpythonCommand += "logic.EnableDryrunMode();"

    if cluster:
        # cluster Mode
        evalpythonCommand += "configuration = AtlasCreatorGridConfiguration();"
        evalpythonCommand += "configuration.SetSchedulerCommand('"+schedulerCommand+"');"
    elif skipRegistration:
        # skipRegistration Mode
        evalpythonCommand += "configuration = AtlasCreatorSkipRegistrationConfiguration();"
        evalpythonCommand += "configuration.SetTransformDirectory('"+transformsDir+"');"
        evalpythonCommand += "configuration.SetExistingTemplate('"+existingTemplate+"');"

    else:
        # normal Mode
        evalpythonCommand += "configuration = AtlasCreatorConfiguration();"
        
    # now the configuration options which are valid for all
    if imagesDir:
        evalpythonCommand += "configuration.SetOriginalImagesFilePathList(gui.GetHelper().ConvertDirectoryToList('"+imagesDir+"'));"
    
    if segmentationsDir:
        evalpythonCommand += "configuration.SetSegmentationsFilePathList(gui.GetHelper().ConvertDirectoryToList('"+segmentationsDir+"'));"
    
    if outputDir:
        evalpythonCommand += "configuration.SetOutputDirectory('"+outputDir+"');"
    
    if useCMTK:
        evalpythonCommand += "configuration.SetToolkit('CMTK');"
    
    if fixed:
        evalpythonCommand += "configuration.SetTemplateType('fixed');"
        evalpythonCommand += "configuration.SetFixedTemplateDefaultCaseFilePath('"+template+"');"
    else:
        evalpythonCommand += "configuration.SetTemplateType('dynamic');"
        evalpythonCommand += "configuration.SetDynamicTemplateIterations("+str(meanIterations)+");"
    
    evalpythonCommand += "configuration.SetLabelsList("+str(labels)+");"
            
    if nonRigid:
        evalpythonCommand += "configuration.SetRegistrationType('Non-Rigid');"
    else:
        evalpythonCommand += "configuration.SetRegistrationType('Affine');"
            
    if writeTransforms:
        evalpythonCommand += "configuration.SetSaveTransforms(1);"
    else:
        evalpythonCommand += "configuration.SetSaveTransforms(0);"

    if keepAligned:
        evalpythonCommand += "configuration.SetDeleteAlignedImages(0);"
        evalpythonCommand += "configuration.SetDeleteAlignedSegmentations(0);"
    else:
        evalpythonCommand += "configuration.SetDeleteAlignedImages(1);"
        evalpythonCommand += "configuration.SetDeleteAlignedSegmentations(1);"
        
    if normalize:
        evalpythonCommand += "configuration.SetNormalizeAtlases(1);"
    else:
        evalpythonCommand += "configuration.SetNormalizeAtlases(0);"

    evalpythonCommand += "configuration.SetOutputCast('"+outputCast+"');"
    
    # add the start command
    evalpythonCommand += "logic.Start(configuration);"
    
    # cleanup
    evalpythonCommand += "logic = None; gui = None;"
        
    command = slicerLauncherFilePath + ' --no_splash --evalpython "' + evalpythonCommand + '"'

    os.system(command)

    
'''=========================================================================================='''
if __name__ == "__main__":
    main(sys.argv[1:])

