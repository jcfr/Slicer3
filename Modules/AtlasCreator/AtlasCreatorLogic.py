from Slicer import slicer
import os
import glob
import time
import tempfile
import shutil

from AtlasCreatorHelper import AtlasCreatorHelper

class AtlasCreatorLogic(object):
    '''
        The logic class for the Atlas Creator
        
        In this class, all files are passed as lists of filepaths to enable possible parallelized
        computation in a grid environment 
    '''
    
    
    
    '''=========================================================================================='''
    def __init__(self, parentClass):
        '''
            Initialize this class
            
            parentClass
                a pointer to the parentClass
                
            Returns
                n/a
        '''
        
        self._parentClass = parentClass
        
        self.__dryRun = 0



    '''=========================================================================================='''
    def EnableDryrunMode(self):
        '''
            Enables the dryrun Mode
            
            Returns
                n/a
        '''
        self.__dryRun = 1



    '''=========================================================================================='''
    def Helper(self):
        '''
            Return the helper
            
            Returns
                the helper
        '''
        if not self._parentClass:
            # no parentClass, we have to create our own helper
            # this is not normal behavior
            return AtlasCreatorHelper(self)
        else:
            # return the helper of the parentClass
            return self._parentClass.GetHelper()



    '''=========================================================================================='''
    def Start(self, node):
        '''
            Entry point for the Atlas Creation
            
            node
                an AtlasCreator MRML Node (vtkMRMLAtlasCreatorNode) transporting a configuration
                
            Returns
                TRUE or FALSE
        '''
        
        # check if we have a valid vtkMRMLAtlasCreatorNode
        if not isinstance(node, slicer.vtkMRMLAtlasCreatorNode):
            self.Helper().info("ERROR: No valid vtkMRMLAtlasCreatorNode!")
            self.Helper().info("Aborting now!")
            return False
        
        if node.GetDebugMode():
            # activate debug mode
            self.Helper().EnableDebugMode()
            self.Helper().info("Debug Mode is enabled!")
            
        if node.GetDryrunMode():
            # activate dryrun mode
            self.EnableDryrunMode()
            self.Helper().info("Dry-Run is activated: Output executable commands instead of running the registration or resampling.")
        
        clusterMode = 0
        skipRegistrationMode = 0

        if (node.GetUseCluster()):
            
            # Cluster Mode
            self.Helper().info("--------------------------------------------------------------------------------")
            self.Helper().info("                   Starting Atlas Creator Cluster Mode                          ")
            clusterMode = 1
            
        elif (node.GetSkipRegistration()):
            
            # Skip Registration mode
            self.Helper().info("--------------------------------------------------------------------------------")
            self.Helper().info("               Starting Atlas Creator Skip Registration Mode                    ")
            skipRegistrationMode = 1
            
        else:
            
            # Standard Mode
            self.Helper().info("--------------------------------------------------------------------------------")
            self.Helper().info("                      Starting Atlas Creator Normal Mode                        ")
        
        
        # check if we want to use CMTK
        useCMTK = False
        if node.GetToolkit() == "CMTK":
            # check if CMTK is installed
            cmtkDir = self.Helper().GetCMTKInstallationDirectory()
            if cmtkDir:
                self.Helper().info("Found CMTK at: " + cmtkDir)
                useCMTK = True
            else:
                self.Helper().info("ERROR! CMTK extensions not found!")
                self.Helper().info("ERROR! Falling back to BRAINSFit...")
                self.Helper().info("ERROR! Please install CMTK4Slicer in order to use CMTK!")
                useCMTK = False
    
        #
        # convert the string based filePath lists to real lists
        #
        originalImagesFilePathList = str(node.GetOriginalImagesFilePathList())
        if originalImagesFilePathList!="":
            originalImagesFilePathList = originalImagesFilePathList.split()
        else:
            originalImagesFilePathList = []
            
        segmentationsFilePathList = str(node.GetSegmentationsFilePathList())
        if segmentationsFilePathList!="":
            segmentationsFilePathList = segmentationsFilePathList.split()
        else:
            segmentationsFilePathList = []   
            
        # if we need to register, check if all files in the configuration.GetOriginalImagesFilePathList() are
        # also present in  configuration.GetSegmentationsFilePathList() and vice-versa
        #
        if not skipRegistrationMode:         
            
            originalImagesFilePathList = self.Helper().GetValidFilePaths(originalImagesFilePathList,
                                                                         segmentationsFilePathList)
            segmentationsFilePathList = self.Helper().GetValidFilePaths(segmentationsFilePathList,
                                                                        originalImagesFilePathList)
            
            
        # convert the string based labels list to a real list is not necessary because python does it
        # automatically
        labelsList = node.GetLabelsList()
            
        # at this point, we have a valid configuration, let's print it
        self.Helper().info("Configuration for Atlas Creator:\n" + str(node))
        
        # create the output directories     
        if not os.path.isdir(node.GetOutputDirectory()):
            os.makedirs(node.GetOutputDirectory())
        
        transformDirectory = node.GetOutputDirectory() + "transforms" + os.sep
        os.makedirs(transformDirectory)
        registeredDirectory = node.GetOutputDirectory() + "registered" + os.sep
        os.makedirs(registeredDirectory)
        resampledDirectory = node.GetOutputDirectory() + "resampled" + os.sep
        os.makedirs(resampledDirectory)
        scriptsRegistrationDirectory = node.GetOutputDirectory() + "scriptsRegistration" + os.sep
        os.makedirs(scriptsRegistrationDirectory)
        notifyRegistrationDirectory = node.GetOutputDirectory() + "notifyRegistration" + os.sep
        os.makedirs(notifyRegistrationDirectory)
        scriptsResamplingDirectory = node.GetOutputDirectory() + "scriptsResampling" + os.sep
        os.makedirs(scriptsResamplingDirectory)
        notifyResamplingDirectory = node.GetOutputDirectory() + "notifyResampling" + os.sep
        os.makedirs(notifyResamplingDirectory)        
        scriptsCombineToAtlasDirectory = node.GetOutputDirectory() + "scriptsCombineToAtlas" + os.sep
        os.makedirs(scriptsCombineToAtlasDirectory)
        notifyCombineToAtlasDirectory = node.GetOutputDirectory() + "notifyCombineToAtlas" + os.sep
        os.makedirs(notifyCombineToAtlasDirectory) 
             
        # executable configuration
        multiThreading = True # enable multiThreading by default
        sleepValue = 5 # seconds to wait between each check on completed jobs
        schedulerCommand = "" # assume no scheduler by default
        
        if clusterMode:
            # if this is a cluster mode, add the schedulerCommand and disable multiThreading
            # also, raise the seconds to wait between each check on completed jobs to 60
            self.Helper().info("Found cluster configuration..")
            schedulerCommand = node.GetSchedulerCommand()
            self.Helper().debug("Scheduler Command: " + str(schedulerCommand))
            multiThreading = False
            sleepValue = 60             
             
             
        # check if we register or if we use existing transforms
        if not skipRegistrationMode:
            # we register to get fresh transforms!!
            
            #
            #
            # REGISTRATION STAGE
            #
            #
            self.Helper().info("Entering Registration Stage..")
                    
            # create a temporary meanImage which might get used
            meanImageFilePath = tempfile.mkstemp(".nrrd", "acTmpMeanImage", node.GetOutputDirectory())[1]                   
                    
            #
            # FIXED REGISTRATION
            #
            if node.GetTemplateType() == "fixed":
                # fixed registration.. only register once against the defaultCase
                
                defaultCase = node.GetFixedTemplateDefaultCaseFilePath()
                
                self.Helper().info("Fixed registration against " + str(defaultCase))
                
                alignedImages = self.Register(schedulerCommand,
                                              originalImagesFilePathList,
                                              defaultCase,
                                              transformDirectory,
                                              registeredDirectory,
                                              scriptsRegistrationDirectory,
                                              notifyRegistrationDirectory,
                                              node.GetRegistrationType(),
                                              multiThreading,
                                              useCMTK,
                                              sleepValue)
                
            #
            # DYNAMIC REGISTRATION
            #
            elif node.GetTemplateType() == "dynamic":
                # dynamic registration.. register against mean until number of iterations reached
                
                alignedImages = originalImagesFilePathList
                
                self.Helper().info("Dynamic registration with " + str(node.GetDynamicTemplateIterations()) + " iterations")
                    
                for i in range(0, node.GetDynamicTemplateIterations()):
                    
                    self.Helper().info("Starting iteration " + str(i + 1) + "...")
                    
                    # create directories for the current registration iteration
                    iterationString = "iteration" + str(i + 1)
                    uniqueRegisteredDirectory = registeredDirectory + iterationString + os.sep
                    os.mkdir(uniqueRegisteredDirectory)
                    uniqueScriptsDirectory = scriptsRegistrationDirectory + iterationString + os.sep
                    os.mkdir(uniqueScriptsDirectory)
                    uniqueNotifyDirectory = notifyRegistrationDirectory + iterationString + os.sep
                    os.mkdir(uniqueNotifyDirectory)
                    
                    # we generate the current meanImage
                    meanVolumeNode = slicer.vtkMRMLScalarVolumeNode()
                    if not self.__dryRun:
                        meanVolumeNode = self.GetMeanImage(alignedImages)
                        if node.GetDeleteAlignedImages():
                            if alignedImages != originalImagesFilePathList():
                                # do not delete the original images
                                self.Helper().DeleteFilesAndDirectory(alignedImages)
                            
                    if not self.__dryRun:
                        self.Helper().SaveVolume(meanImageFilePath, meanVolumeNode)
                    
                    # we register the original images against the meanImage
                    # we then set the alignedImages and start over..
                    alignedImages = self.Register(schedulerCommand,
                                                  originalImagesFilePathList(),
                                                  meanImageFilePath,
                                                  transformDirectory,
                                                  uniqueRegisteredDirectory,
                                                  uniqueScriptsDirectory,
                                                  uniqueNotifyDirectory,
                                                  node.GetRegistrationType(),
                                                  multiThreading,
                                                  useCMTK,
                                                  sleepValue) 
    
                # now we point the defaultCase to the meanImageFilePath
                defaultCase = meanImageFilePath
                
                self.Helper().info("End of Dynamic registration..")
            
            # now wipe the temporary registered content, if selected
            if node.GetDeleteAlignedImages():
                shutil.rmtree(registeredDirectory, True, None)
                
            # we will save the template
            # this will ensure that we can later 
            # use the transforms (if they exist) and the template to resample
            # at this point, the defaultCase is either the meanImage or the fixed defaultCase
            v = self.Helper().LoadVolume(defaultCase)
            pathToTemplate = node.GetOutputDirectory() + "template.nrrd"
            self.Helper().info("Saving template to " + str(pathToTemplate))
            self.Helper().SaveVolume(str(pathToTemplate), v)
            # now remove the node from the mrmlscene
            slicer.MRMLScene.RemoveNode(v)
            
            # update reference to defaultCase to new location, only if templateType is dynamic
            if node.GetTemplateType() == "dynamic":
                defaultCase = pathToTemplate
            
            # now delete the temporary mean template
            os.remove(meanImageFilePath)
                    
        else:
            # we are skipping the registration
            self.Helper().info("Skipping the registration and using the existing transforms..")
            
            transformDirectory = node.GetTransformDirectory()
            # we set the defaultCase to an existing one
            defaultCase = node.GetExistingTemplate()
            
            

        #
        #
        # RESAMPLING STAGE
        #
        #
        self.Helper().info("Entering Resampling Stage..")          
                
        self.Resample(schedulerCommand,
                      segmentationsFilePathList,
                      defaultCase,
                      transformDirectory,
                      resampledDirectory,
                      scriptsResamplingDirectory,
                      notifyResamplingDirectory,
                      useCMTK,
                      sleepValue)
        
        if not self.__dryRun:
            resampledSegmentationsFilePathList = self.Helper().ConvertDirectoryToList(resampledDirectory)
        else:
            resampledSegmentationsFilePathList = segmentationsFilePathList
        

        if node.GetPCAAnalysis():
            #
            #
            # PCA ANALYSIS STAGE
            #
            #
            self.Helper().info("Performing PCA Analysis..")
            
            self.PerformPCAAnalysis(labelsList,
                                    resampledSegmentationsFilePathList,
                                    node.GetOutputDirectory())
            
        else:
            #
            #
            # COMBINE TO ATLAS STAGE
            #
            #
            self.Helper().info("Entering Combine-To-Atlas Stage..")
                        
            self.CombineToAtlas(schedulerCommand,
                                resampledSegmentationsFilePathList,
                                labelsList,
                                node.GetOutputCast(),
                                node.GetNormalizeTo(),
                                node.GetOutputDirectory(),
                                scriptsCombineToAtlasDirectory,
                                notifyCombineToAtlasDirectory,
                                sleepValue)
            
        
        # cleanup!!
        # delete the scripts and notify directories
        if not self.__dryRun:
            shutil.rmtree(scriptsRegistrationDirectory, True, None)
            shutil.rmtree(notifyRegistrationDirectory, True, None)
            shutil.rmtree(scriptsResamplingDirectory, True, None)
            shutil.rmtree(notifyResamplingDirectory, True, None)    
            shutil.rmtree(scriptsCombineToAtlasDirectory, True, None)
            shutil.rmtree(notifyCombineToAtlasDirectory, True, None)        
            
        # now delete the resampled segmentations
        if node.GetDeleteAlignedSegmentations():
            shutil.rmtree(resampledDirectory, True, None)
        
        # delete the transforms, if we did not want to save them
        if not node.GetSaveTransforms() and not skipRegistrationMode:
            shutil.rmtree(transformDirectory, True, None)
            
        
        self.Helper().info("--------------------------------------------------------------------------------")        
        self.Helper().info("                             All Done, folks!                                   ")
        self.Helper().info("--------------------------------------------------------------------------------")
        
        return True



    '''=========================================================================================='''
    def GetMeanImage(self, filePathsList):
        '''
            Get the mean image of a set of images and create a vtkMRMLScalarVolumeNode
            
            filePathsList
                list of file paths to the images
                
            Returns
                vtkMRMLScalarVolumeNode with imageData after the mean image was computed
        '''
        newMeanImage = slicer.vtkImageData()
        
        outputV = slicer.vtkMRMLScalarVolumeNode()
        
        firstRun = 1
        
        # first, add up all images
        for filePath in filePathsList:
            
            # we load the image from filePath as a MRML node
            v = self.Helper().LoadVolume(filePath)
            
            image = slicer.vtkImageData()
            # to prevent overflows,
            # divide the images individually before adding them up
            # this automatically casts the image to double
            image.DeepCopy(self.Helper().DivideImage(v.GetImageData(), len(filePathsList)))
            
            if firstRun:
                newMeanImage.DeepCopy(image)
                firstRun = 0
            else:
                # now add'em all up
                newMeanImage.DeepCopy(self.Helper().AddImages(image, newMeanImage))
                
            # now copy the orientation of the node to our output node
            outputV.CopyOrientation(v)    
            
            # now we remove the mrml node
            slicer.MRMLScene.RemoveNode(v)
            
        
        # now save the mean image data to the node
        outputV.SetAndObserveImageData(newMeanImage)
                
        # now return the mean image volume node
        return outputV
    

    
    '''=========================================================================================='''
    def Register(self, schedulerCommand, filePathsList, templateFilePath, outputTransformsDirectory, outputAlignedDirectory, outputScriptsDirectory, outputNotifyDirectory, registrationType, multiThreading, useCMTK=False, sleepValue=5):
        '''
            Register a set of images, get a transformation and save it
            
            schedulerCommand
                the schedulerCommand if used else ""
            filePathsList
                list of existing filepaths to images
            templateFilePath
                file path to the template
            outputTransformsDirectory
                directory to save the generated transformation
            outputAlignedDirectory
                directory to save the aligned images
            outputScriptsDirectory
                directory to use for generated scripts
            outputNotifyDirectory
                directory to use for notification files
            registrationType
                type of registration as String, could be "affine" and "non-rigid"
                if the value is invalid, affine registration is assumed
            multiThreading
                if TRUE, use multiThreading
            useCMTK
                if TRUE, use CMTK instead of BRAINSFit
            sleepValue
                seconds to wait between each check on completed jobs
                
            Returns
                A list of filepaths to the aligned Images or None depending on success
        '''
        
        # sanity checks
        if len(filePathsList) == 0:
            self.Helper().info("Empty filePathsList for Register() command. Aborting..")
            return None
        
        if not templateFilePath:
            self.Helper().info("Empty templateFilePath for Register() command. Aborting..")
            return None
        
        if not outputTransformsDirectory:
            self.Helper().info("Empty outputTransformsDirectory for Register() command. Aborting..")
            return None
        
        if not outputAlignedDirectory:
            self.Helper().info("Empty outputAlignedDirectory for Register() command. Aborting..")
            return None        

        if not outputScriptsDirectory:
            self.Helper().info("Empty outputScriptsDirectory for Register() command. Aborting..")
            return None   

        if not outputNotifyDirectory:
            self.Helper().info("Empty outputNotifyDirectory for Register() command. Aborting..")
            return None   
        
        if registrationType == "Affine":
            onlyAffineReg = 1
        elif registrationType == "Non-Rigid":
            onlyAffineReg = 0
        else:
            # assume only affine registration if wrong value
            onlyAffineReg = 1
            
        outputAlignedImages = []
            
        # get the launch command for Slicer3
        launchCommandPrefix = self.Helper().GetSlicerLaunchPrefix(useCMTK)
        
        uniqueID = 0
        
        # loop through filePathsList and start registration command
        for movingImageFilePath in filePathsList:
        
            # do not register the same file
            if movingImageFilePath == templateFilePath:
                continue

            # increase the uniqueID
            uniqueID = uniqueID + 1
            
            # guess the background level of the current image
            backgroundGuess = self.Helper().GuessBackgroundValue(movingImageFilePath)
            self.Helper().debug("Guessing background value: " + str(backgroundGuess))
            
            if useCMTK:
                # guess the background level of the current image, only if CMTK is used
                backgroundGuessTemplate = self.Helper().GuessBackgroundValue(templateFilePath)
                self.Helper().debug("Guessing background value for template: " + str(backgroundGuessTemplate))            
                
            movingImageName = os.path.splitext(os.path.basename(movingImageFilePath))[0]
            
            # generate file path to save output transformation
            # by getting the filename of the case and appending it to the outputTransformsDirectory
            outputTransformFilePath = outputTransformsDirectory + str(movingImageName) + ".mat"
            
            # generate file path to save aligned output image
            # by getting the filename of the case and appending it to the outputTransformsDirectory
            outputAlignedImageFilePath = outputAlignedDirectory + str(movingImageName) + ".nrrd"
            
            # we will create a string containing the command(s) which then get written to a script
            command = ""
            
            if useCMTK:
                
                if not multiThreading:
                    # for CMTK, we have to set the variable to disable multiThreading
                    command += "CMTK_NUM_THREADS=1\n"
                
                command += str(launchCommandPrefix) + self.Helper().GetCMTKAffineRegistrationCommand(templateFilePath,
                                                                                                    movingImageFilePath,
                                                                                                    outputTransformFilePath,
                                                                                                    outputAlignedImageFilePath,
                                                                                                    backgroundGuess,
                                                                                                    backgroundGuessTemplate)
                
                if not onlyAffineReg:
                    # for non-rigid, we include the second CMTK command
                    command += "\n"
                    command += str(launchCommandPrefix) + self.Helper().GetCMTKNonRigidRegistrationCommand(templateFilePath,
                                                                                                           movingImageFilePath,
                                                                                                           outputTransformFilePath,
                                                                                                           outputAlignedImageFilePath,
                                                                                                           backgroundGuess,
                                                                                                           backgroundGuessTemplate)
                
                
            else:    
                command += str(launchCommandPrefix) + self.Helper().GetBRAINSFitRegistrationCommand(templateFilePath,
                                                                                                   movingImageFilePath,
                                                                                                   outputTransformFilePath,
                                                                                                   outputAlignedImageFilePath,
                                                                                                   onlyAffineReg,
                                                                                                   multiThreading,
                                                                                                   backgroundGuess)
            
            self.Helper().debug("Register command(s): " + str(command))
            
            # create notification ID
            notify = "echo \"done\" > " + str(outputNotifyDirectory) + str(uniqueID) + ".ac"
            
            # now generate a script containing the commands and the notify line
            script = self.Helper().CreateScript(command, notify)
            
            scriptFilePath = outputScriptsDirectory + "script" + str(uniqueID) + ".sh"
            
            with open(scriptFilePath, 'w') as f:
                f.write(script)
            
            # set executable permissions
            os.chmod(scriptFilePath, 0700)
            
            self.Helper().debug("Executing generated Registration Script: " + scriptFilePath)
            
            if self.__dryRun:
                self.Helper().info("DRYRUN - skipping execution..")    
            else:
                self.Helper().Execute(schedulerCommand + " " + scriptFilePath)
                
            outputAlignedImages.append(str(outputAlignedImageFilePath))
            
        if self.__dryRun:
            return outputAlignedImages
        
        # at this point:
        # either the registration was completed if the os.system call did not send it to the background
        # or the registration still runs in the background
        # latter is possible, if a cluster scheduler was specified
        # now, we will wait until all output images exist
        
        allOutputsExist = False
        
        while not allOutputsExist:
            # not all outputs exist yet
            # get number of existing files
            numberOfExistingFiles = len(glob.glob(outputNotifyDirectory + "*.ac"))
            self.Helper().info("Waiting for Registration to complete.. (" + str(numberOfExistingFiles) + "/" + str(uniqueID) + " done)")
            
            # wait some secs and then check again
            time.sleep(int(sleepValue))
            
            # we assume everything exists
            allOutputsExist = True

            # but now we really check if it is so            
            #for file in outputAlignedImages:
            for index in range(1, uniqueID + 1):
                
                file = outputNotifyDirectory + str(index) + ".ac"
                
                if not os.path.isfile(file):
                    self.Helper().debug("Registration Job with id " + str(index) + " not completed!")
                    # if only one file does not exist,
                    # we know we have to wait longer
                    allOutputsExist = False
                    break
        
        self.Helper().debug("All Registration outputs exist!")
                
        return outputAlignedImages


    
    '''=========================================================================================='''
    def Resample(self, schedulerCommand, filePathsList, templateFilePath, transformDirectory, outputSegmentationDirectory, outputScriptsDirectory, outputNotifyDirectory, useCMTK=False, sleepValue=5):
        '''
            Resample a set of segmentations using exising transforms
            
            schedulerCommand
                the schedulerCommand if used else ""
            filePathsList
                list of existing filepaths to segmentations
            templateFilePath
                file path to the template
            transformDirectory
                directory to existing transformations
                the transformation has to be named after the basename of the filepaths with a .mat extension      
                (can be a directory as well)
            outputSegmentationDirectory
                directory to save the resampled images
            outputScriptsDirectory
                directory to use for generated scripts
            outputNotifyDirectory
                directory to use for notification files                
            useCMTK
                if TRUE, use CMTK instead of BRAINSFit
            sleepValue
                seconds to wait between each check on completed jobs
                                
            Returns
                TRUE or FALSE depending on success
        '''
        
        # sanity checks
        if len(filePathsList) == 0:
            self.Helper().info("Empty filePathsList for Resample() command. Aborting..")
            return False
        
        if not templateFilePath:
            self.Helper().info("Empty templateFilePath for Resample() command. Aborting..")
            return False

        if not transformDirectory:
            self.Helper().info("Empty transformDirectory for Resample() command. Aborting..")
            return False
        
        if not outputSegmentationDirectory:
            self.Helper().info("Empty outputSegmentationDirectory for Resample() command. Aborting..")
            return False
        
        if not outputScriptsDirectory:
            self.Helper().info("Empty outputScriptsDirectory for Resample() command. Aborting..")
            return False   

        if not outputNotifyDirectory:
            self.Helper().info("Empty outputNotifyDirectory for Resample() command. Aborting..")
            return False           

        # get the launch command for Slicer3
        launchCommandPrefix = self.Helper().GetSlicerLaunchPrefix(useCMTK)

        uniqueID = 0

        # loop through filePathsList and start resample command
        for segmentationFilePath in filePathsList:

            # do not resample the same file
            if os.path.basename(segmentationFilePath) == os.path.basename(templateFilePath):
                continue

            # increase the uniqueID
            uniqueID = uniqueID + 1
            
            # guess the background level of the current image
            backgroundGuess = self.Helper().GuessBackgroundValue(segmentationFilePath)
            self.Helper().debug("Guessing background value: " + str(backgroundGuess))            

            segmentationName = os.path.splitext(os.path.basename(segmentationFilePath))[0]

            transformFilePath = transformDirectory + str(segmentationName) + ".mat"                
                
            # generate file path to output segmentation
            # by getting the filename of the case and appending it to the outputSegmentationDirectory
            outputSegmentationFilePath = outputSegmentationDirectory + str(segmentationName) + ".nrrd"

            # we will create a string containing the command(s) which then get written to a script
            command = ""

            if useCMTK:
                command += str(launchCommandPrefix) + self.Helper().GetCMTKResampleCommand(segmentationFilePath,
                                                                                           templateFilePath,
                                                                                           transformFilePath,
                                                                                           outputSegmentationFilePath,
                                                                                           backgroundGuess)                
            else:
                command += str(launchCommandPrefix) + self.Helper().GetBRAINSFitResampleCommand(segmentationFilePath,
                                                                                                templateFilePath,
                                                                                                transformFilePath,
                                                                                                outputSegmentationFilePath,
                                                                                                backgroundGuess)

            self.Helper().debug("Resample command(s): " + str(command))
            
            # create notification ID
            notify = "echo \"done\" > " + str(outputNotifyDirectory) + str(uniqueID) + ".ac"
            
            # now generate a script containing the commands and the notify line
            script = self.Helper().CreateScript(command, notify)
            
            scriptFilePath = outputScriptsDirectory + "script" + str(uniqueID) + ".sh"
            
            with open(scriptFilePath, 'w') as f:
                f.write(script)
            
            # set executable permissions
            os.chmod(scriptFilePath, 0700)    
            
            self.Helper().debug("Executing generated Resample Script: " + scriptFilePath)
            
            if self.__dryRun:
                self.Helper().info("DRYRUN - skipping execution..")
            else:
                self.Helper().Execute(schedulerCommand + " " + scriptFilePath)
                
        if self.__dryRun:
            return True                
                
        # at this point:
        # either the resampling was completed if the os.system call did not send it to the background
        # or the registration still runs in the background
        # latter is possible, if a cluster scheduler was specified
        # now, we will wait until all output images exist
        
        allOutputsExist = False
        
        while not allOutputsExist:
            # not all outputs exist yet
            # get number of existing files
            numberOfExistingFiles = len(glob.glob(outputNotifyDirectory + "*.ac"))
            self.Helper().info("Waiting for Resampling to complete.. (" + str(numberOfExistingFiles) + "/" + str(uniqueID) + " done)")
            
            # wait some secs and then check again
            time.sleep(int(sleepValue))
            
            # we assume everything exists
            allOutputsExist = True

            # but now we really check if it is so            
            #for file in outputAlignedImages:
            for index in range(1, uniqueID + 1):
                
                file = outputNotifyDirectory + str(index) + ".ac"
                
                if not os.path.isfile(file):
                    self.Helper().debug("Resampling Job with id " + str(index) + " not completed!")
                    # if only one file does not exist,
                    # we know we have to wait longer
                    allOutputsExist = False
                    break
        
        self.Helper().debug("All Resampling outputs exist!")

        return True


    
    '''=========================================================================================='''
    def SaveDeformationFields(self, launchCommandPrefix, filePathsList, transformDirectory, outputDeformationFieldsDirectory):
        '''
            Save Deformation Fields for a list of files using existing transforms
            This only works if the transforms are BSpline based
            
            launchCommandPrefix
                prefix for the actual registration command
            filePathsList
                list of existing filepaths to segmentations
            transformDirectory
                directory to existing transformations
                the transformation has to be named after the basename of the filepaths with a .mat extension 
            outputDeformationFieldsDirectory
                directory to save the Deformation Fields
                
            Returns
                TRUE or FALSE depending on success
        '''
        
        # sanity checks
        if len(filePathsList) == 0:
            self.Helper().info("Empty filePathsList for SaveDeformationFields() command. Aborting..")
            return False

        if not transformDirectory:
            self.Helper().info("Empty transformDirectory for SaveDeformationFields() command. Aborting..")
            return False
        
        if not outputDeformationFieldsDirectory:
            self.Helper().info("Empty outputDeformationFieldsDirectory for SaveDeformationFields() command. Aborting..")
            return False

        # loop through filePathsList, get deformation fields and save it
        for movingImageFilePath in filePathsList:

            # generate file path to existing transformation
            # by getting the filename of the case and appending it to the transformDirectory
            transformFilePath = transformDirectory + str(os.path.basename(movingImageFilePath)) + ".mat"
            
            # generate file path to output deformation field
            # by getting the filename of the case and appending it to the outputDeformationFieldsDirectory
            outputDeformationFieldFilePath = outputDeformationFieldsDirectory + str(os.path.basename(movingImageFilePath)) + ".nrrd"

            command = str(launchCommandPrefix) + self.Helper().GetSaveDeformationFieldCommand(movingImageFilePath,
                                                                                              transformFilePath,
                                                                                              outputDeformationFieldFilePath)

            self.Helper().debug("SaveDeformationFields command: " + str(command))
            
            if self.__dryRun:
                self.Helper().info("DRYRUN - skipping execution..")
            else:
                self.Helper().Execute(command)

        return True
    
    
    
    '''=========================================================================================='''
    def CombineToAtlas(self, schedulerCommand, filePathsList, labelsList, reCastString, normalizeTo, outputAtlasDirectory, outputScriptsDirectory, outputNotifyDirectory, sleepValue=5):
        '''
            Combine segmentations to an Atlas based on labels. For each label an Atlas gets created. Additionally,
            a combined Atlas is generated. All output is saved.
            
            schedulerCommand
                the schedulerCommand if used else ""            
            filePathsList
                list of existing filepaths to segmentations
            labelsList
                list of labels to analyze and build Atlases for
            reCastString
                re-Cast the Atlases to a certain type defined as a String
                "Char"
                "Unsigned Char"
                "Double"
                "Float"
                "Int"
                "Unsigned Int"
                "Long"
                "Unsigned Long"
                "Short"
                "Unsigned Short"
                other values will result in "Short"
            normalizeTo
                flag to enable the normalization of Atlas values
                -1: disable
                any other: upper value for normalization
            outputAtlasDirectory
                directory to save the Atlases
            outputScriptsDirectory
                directory to use for generated scripts
            outputNotifyDirectory
                directory to use for notification files                    
            sleepValue
                seconds to wait between each check on completed jobs
                                
            Returns
                TRUE or FALSE depending on success
        '''
        
        # sanity checks
        if len(filePathsList) == 0:
            self.Helper().info("Empty filePathsList for CombineToAtlas() command. Aborting..")
            return False
        
        if len(labelsList) == 0:
            self.Helper().info("Empty labelsList for CombineToAtlas() command. Aborting..")
            return False
            
        if not outputAtlasDirectory:
            self.Helper().info("Empty outputAtlasDirectory for CombineToAtlas() command. Aborting..")
            return False
        
        if not outputScriptsDirectory:
            self.Helper().info("Empty outputScriptsDirectory for CombineToAtlas() command. Aborting..")
            return False   

        if not outputNotifyDirectory:
            self.Helper().info("Empty outputNotifyDirectory for CombineToAtlas() command. Aborting..")
            return False                
        
        # get the launch command for Slicer3
        launchCommandPrefix = self.Helper().GetSlicerLaunchPrefix(False, True)        
        
        uniqueID = 0
        
        # loop through all labels
        for label in labelsList:
            
            # increase the uniqueID
            uniqueID = uniqueID + 1            
            command = ""
            
            # add some args to the Slicer launcher call
            command += str(launchCommandPrefix) + " --launch " + str(slicer.Application.GetBinDir()) +os.sep+ "ACCombiner";
            
            # now the args
            command += " " + str(label)
            
            command += " \""
            
            for file in filePathsList:
                command += str(file) + " "
            
            command += "\""
            command += " " + str(outputAtlasDirectory)
            command += " \"" + str(reCastString) + "\""
            command += " " + str(normalizeTo)
    
            self.Helper().debug("CombineToAtlas Command(s): " + str(command))
    
            # create notification ID
            notify = "echo \"done\" > " + str(outputNotifyDirectory) + str(uniqueID) + ".ac"
            
            # now generate a script containing the commands and the notify line
            script = self.Helper().CreateScript(command, notify)
            
            scriptFilePath = outputScriptsDirectory + "script" + str(uniqueID) + ".sh"
            
            with open(scriptFilePath, 'w') as f:
                f.write(script)
            
            # set executable permissions
            os.chmod(scriptFilePath, 0700)    
            
            self.Helper().debug("Executing generated CombineToAtlas Script: " + scriptFilePath)
            
            if self.__dryRun:
                self.Helper().info("DRYRUN - skipping execution..") 
            else:
                self.Helper().Execute(schedulerCommand + " " + scriptFilePath)

        if self.__dryRun:
            return True     
                
        # at this point:
        # either the combination was completed if the os.system call did not send it to the background
        # or the registration still runs in the background
        # latter is possible, if a cluster scheduler was specified
        # now, we will wait until all output images exist
        
        allOutputsExist = False
        
        while not allOutputsExist:
            # not all outputs exist yet
            # get number of existing files
            numberOfExistingFiles = len(glob.glob(outputNotifyDirectory + "*.ac"))
            self.Helper().info("Waiting for CombineToAtlas to complete.. (" + str(numberOfExistingFiles) + "/" + str(uniqueID) + " done)")
            
            # wait some secs and then check again
            time.sleep(int(sleepValue))
            
            # we assume everything exists
            allOutputsExist = True

            # but now we really check if it is so            
            #for file in outputAlignedImages:
            for index in range(1, uniqueID + 1):
                
                file = outputNotifyDirectory + str(index) + ".ac"
                
                if not os.path.isfile(file):
                    self.Helper().debug("CombineToAtlas Job with id " + str(index) + " not completed!")
                    # if only one file does not exist,
                    # we know we have to wait longer
                    allOutputsExist = False
                    break
        
        self.Helper().debug("All CombineToAtlas outputs exist!")

        return True



    def PerformPCAAnalysis(self,labelsList,filePathsList,outputDirectory):
        '''
            Performs PCA Analysis on top of the Atlas Generation.
            
            labelsList
                list of labels to perform PCA analysis for
            filePathsList
                list of existing filepaths to segmentations
            outputDirectory
                directory to write the PCA analysis, trailing slash is required
                
            Returns
                TRUE or FALSE depending on success
        '''
        if len(labelsList) == 0:
            self.Helper().info("Empty labelsList for PerformPCAAnalysis() command. Aborting..")
            return False
            
        if len(filePathsList) == 0:
            self.Helper().info("Empty filePathsList for PerformPCAAnalysis() command. Aborting..")
            return False
        
        if not outputDirectory:
            self.Helper().info("Empty outputDirectory for PerformPCAAnalysis() command. Aborting..")
            return False
        
        # create output dir for PCA distance maps
        # f.e. /tmp/acout/pcaDistanceMaps/
        pcaOutputDirectory = outputDirectory + "PCA" + os.sep
        os.mkdir(pcaOutputDirectory)
        
        count = 0
        
        for filePath in filePathsList:
        
            count = count+1
        
            # generate the caseName (without extension)
            # f.e. case1, case2...
            caseName = "case"+str(count)
        
            # create directory structure for this case
            # f.e. /tmp/acout/pcaDistanceMaps/case01/
            casePcaDistanceMapDirectory = os.path.join(pcaOutputDirectory,caseName) + os.sep
            os.mkdir(casePcaDistanceMapDirectory)
        
            for label in labelsList:
                
                # create input path, which is just the current aligned segmentation
                inputAtlasFilePath = filePath
                
                # create output path
                # f.e. /tmp/acout/pcaDistanceMaps/case01/X.nrrd, where X is the current label
                outputDistanceMapFilePath = os.path.join(casePcaDistanceMapDirectory,str(label)+".nrrd")
                
                # generate distance maps
                slicer.TkCall(self.Helper().GetPCADistanceCommand(inputAtlasFilePath,label,outputDistanceMapFilePath))
                
        # now after creating all the distance maps,
        # we can generate the PCA shape representations
        gcmd = self.Helper().GetPCAGenerateCommand(pcaOutputDirectory,labelsList)
        self.Helper().info(gcmd)
        slicer.TkCall(gcmd)
            
        return True
            

