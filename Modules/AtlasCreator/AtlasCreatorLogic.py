from Slicer import slicer
import os
import glob
import time

from AtlasCreatorConfiguration import AtlasCreatorConfiguration
from AtlasCreatorGridConfiguration import AtlasCreatorGridConfiguration
from AtlasCreatorSkipRegistrationConfiguration import AtlasCreatorSkipRegistrationConfiguration
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
    def Start(self, configuration):
        '''
            Entry point for the Atlas Creation
            
            configuration
                the configuration for the Atlas Creator task
                
            Returns
                TRUE or FALSE
        '''
                
        clusterMode = 0
        skipRegistrationMode = 0

        if (isinstance(configuration, AtlasCreatorGridConfiguration)):
            
            # Cluster Mode
            self.Helper().info("--------------------------------------------------------------------------------")
            self.Helper().info("                   Starting Atlas Creator Cluster Mode                          ")
            clusterMode = 1
            
        elif (isinstance(configuration, AtlasCreatorSkipRegistrationConfiguration)):
            
            # Skip Registration mode
            self.Helper().info("--------------------------------------------------------------------------------")
            self.Helper().info("               Starting Atlas Creator Skip Registration Mode                    ")
            skipRegistrationMode = 1
            
        elif (isinstance(configuration, AtlasCreatorConfiguration)):
            
            # Standard Mode
            self.Helper().info("--------------------------------------------------------------------------------")
            self.Helper().info("                      Starting Atlas Creator Normal Mode                        ")
            
        else:
            self.Helper().info("ERROR! Invalid Configuration.. Aborting..")
            return False
    
        # at this point, we have a valid configuration, let's print it
        self.Helper().info("Configuration for Atlas Creator:\n" + str(configuration.GetConfigurationAsString()))
        
        # check if we want to save the transforms in a directory other than the temporary directory
        if configuration.GetSaveTransforms():
            # save the transforms, so use the custom transformDirectory
            transformDirectory = configuration.GetOutputDirectory()
        else:
            # do not save the transforms, so use the temporary directory
            transformDirectory = self.Helper().GetSlicerTemporaryDirectory()        
                     
        # check if we register or if we use existing transforms
        if not skipRegistrationMode:
            # we register to get fresh transforms!!
            
            #
            #
            # REGISTRATION STAGE
            #
            #
            self.Helper().info("Entering Registration Stage..")
            
            # check if it is cluster mode, if yes change the launcher for the registration
            slicerLaunchPrefixForRegistration = self.Helper().GetSlicerLaunchPrefix()
            multiThreading = True
            
            if clusterMode:
                # if this is a cluster mode, add the schedulerCommand
                self.Helper().info("Found cluster configuration..")
                self.Helper().debug("Scheduler Command: " + str(configuration.GetSchedulerCommand()))
                slicerLaunchPrefixForRegistration = configuration.GetSchedulerCommand() + " " + str(slicerLaunchPrefixForRegistration)
                # deactivate multiThreading in a cluster environment
                multiThreading = False
                    
            #
            # FIXED REGISTRATION
            #
            if configuration.GetTemplateType() == "fixed":
                # fixed registration.. only register once against the defaultCase
                
                defaultCase = configuration.GetFixedTemplateDefaultCaseFilePath()
                
                self.Helper().info("Fixed registration against " + str(defaultCase))
                
                self.Register(slicerLaunchPrefixForRegistration,
                              configuration.GetOriginalImagesFilePathList(),
                              defaultCase,
                              transformDirectory,
                              configuration.GetRegistrationType(),
                              multiThreading)
                
            #
            # DYNAMIC REGISTRATION
            #
            elif configuration.GetTemplateType() == "dynamic":
                # dynamic registration.. register against mean until number of iterations reached
                
                alignedImages = configuration.GetOriginalImagesFilePathList()
                
                self.Helper().info("Dynamic registration with " + str(configuration.GetDynamicTemplateIterations()) + " iterations")
                    
                for i in range(0, configuration.GetDynamicTemplateIterations()):
                    
                    self.Helper().info("Starting iteration " + str(i + 1) + "...")
                    
                    # we generate the current meanImage
                    meanImage = slicer.vtkImageData()
                    if not self.__dryRun:
                        meanImage.DeepCopy(self.GetMeanImage(alignedImages))
                    meanImageFilePath = self.Helper().GetSlicerTemporaryDirectory() + "tmpMeanImage.nrrd"
                    if not self.__dryRun:
                        self.Helper().SaveImage(meanImageFilePath, meanImage)
                    
                    # we register the original images against the meanImage
                    # we then set the alignedImages and start over..
                    alignedImages = self.Register(slicerLaunchPrefixForRegistration,
                                                  configuration.GetOriginalImagesFilePathList(),
                                                  meanImageFilePath,
                                                  transformDirectory,
                                                  configuration.GetRegistrationType(),
                                                  multiThreading) 
    
                # now we point the defaultCase to the meanImageFilePath
                defaultCase = meanImageFilePath
                
                self.Helper().info("End of Dynamic registration..")
                
            # we will save the template
            # this will ensure that we can later 
            # use the transforms (if they exist) and the template to resample
            # at this point, the defaultCase is either the meanImage or the fixed defaultCase
            defaultCaseImageData = slicer.vtkImageData()
            defaultCaseImageData.DeepCopy(self.Helper().LoadImage(defaultCase))
            pathToTemplate = configuration.GetOutputDirectory() + "template.nrrd"
            self.Helper().info("Saving template to " + str(pathToTemplate))
            self.Helper().SaveImage(str(pathToTemplate), defaultCaseImageData)
                    
        else:
            # we are skipping the registration
            self.Helper().info("Skipping the registration and using the existing transforms..")
            
            transformDirectory = configuration.GetTransformDirectory()
            # we set the defaultCase to an existing one
            defaultCase = configuration.GetExistingTemplate()
            
            

        #
        #
        # RESAMPLING STAGE
        #
        #
        self.Helper().info("Entering Resampling Stage..")
                
        self.Resample(self.Helper().GetSlicerLaunchPrefix(),
                      configuration.GetSegmentationsFilePathList(),
                      defaultCase,
                      transformDirectory,
                      self.Helper().GetSlicerTemporaryDirectory())
            
        #
        #
        # COMBINE TO ATLAS STAGE
        #
        #
        self.Helper().info("Entering Combine-To-Atlas Stage..")
                    
        self.CombineToAtlas(configuration.GetSegmentationsFilePathList(),
                            configuration.GetLabelsList(),
                            configuration.GetOutputCast(),
                            configuration.GetNormalizeAtlases(),
                            configuration.GetOutputDirectory())
        
        
        self.Helper().info("--------------------------------------------------------------------------------")        
        self.Helper().info("                             All Done, folks!                                   ")
        self.Helper().info("--------------------------------------------------------------------------------")
        
        return True



    '''=========================================================================================='''
    def GetMeanImage(self, filePathsList):
        '''
            Get the mean image of a set of images
            
            filePathsList
                list of file paths to the images
                
            Returns
                vtkImageData after the mean image was computed
        '''
        newMeanImage = slicer.vtkImageData()
        
        firstRun = 1
        
        # first, add up all images
        for filePath in filePathsList:
            
            # we load the image from filePath
            image = slicer.vtkImageData()
            image.DeepCopy(self.Helper().LoadImage(filePath))
            
            if firstRun:
                newMeanImage.DeepCopy(image)
                firstRun = 0
            else:
                # now add'em all up
                newMeanImage.DeepCopy(self.Helper().AddImages(image, newMeanImage))
                
        # second, divide by the number of images added
        # this automatically casts to float
        newMeanImage.DeepCopy(self.Helper().DivideImage(newMeanImage, len(filePathsList)))
    
        # now return the mean image
        return newMeanImage
    

    
    '''=========================================================================================='''
    def Register(self, launchCommandPrefix, filePathsList, templateFilePath, outputDirectory, registrationType, multiThreading):
        '''
            Register a set of images, get a transformation and save it
            
            launchCommandPrefix
                prefix for the actual registration command
            filePathsList
                list of existing filepaths to images
            templateFilePath
                file path to the template
            outputDirectory
                directory to save the generated transformation
            registrationType
                type of registration as String, could be "affine" and "non-rigid"
                if the value is invalid, affine registration is assumed
            multiThreading
                if TRUE, use multiThreading
                
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
        
        if not outputDirectory:
            self.Helper().info("Empty outputDirectory for Register() command. Aborting..")
            return None
        
        if registrationType == "Affine":
            onlyAffineReg = 1
        elif registrationType == "Non-Rigid":
            onlyAffineReg = 0
        else:
            # assume only affine registration if wrong value
            onlyAffineReg = 1
            
        outputAlignedImages = []
            
            
        # create a unique temp directory in Slicer's temp directory
        uniqueTempDir = mktemp(self.Helper().GetSlicerTemporaryDirectory()) + os.sep
            
        # loop through filePathsList and start registration command
        for movingImageFilePath in filePathsList:
            
            # do not register the same file
            if movingImageFilePath == templateFilePath:
                continue
            
            movingImageName = os.path.splitext(os.path.basename(movingImageFilePath))[0]
            
            # generate file path to save output transformation
            # by getting the filename of the case and appending it to the outputDirectory
            outputTransformFilePath = outputDirectory + str(movingImageName) + ".mat"
            
            # generate file path to save aligned output image
            # by getting the filename of the case and appending it to the outputDirectory
            outputAlignedImageFilePath = uniqueTempDir + str(movingImageName) + ".nrrd"
            
            command = str(launchCommandPrefix) + self.Helper().GetRegistrationCommand(templateFilePath,
                                                                                      movingImageFilePath,
                                                                                      outputTransformFilePath,
                                                                                      outputAlignedImageFilePath,
                                                                                      onlyAffineReg,
                                                                                      multiThreading)
            
            self.Helper().debug("Register command: " + str(command))
            
            
            if self.__dryRun:
                self.Helper().info("DRYRUN - skipping execution..")            
            else:
                os.system(command)
                
            outputAlignedImages.append(str(outputAlignedImageFilePath))
            
        
        # at this point:
        # either the registration was completed if the os.system call did not send it to the background
        # or the registration still runs in the background
        # latter is possible, if a cluster scheduler was specified
        # now, we will wait until all output images exist
        
        allOutputsExist = False
        
        while not allOutputsExist:
            # not all outputs exist yet
            self.Helper().info("Waiting for Registration to complete..")
            
            # wait 5 secs and then check again
            time.sleep(5)
            
            # we assume everything exists
            allOutputsExist = True

            # but now we really check if it is so            
            for file in outputAlignedImages:
                
                if not os.path.isfile(file):
                    self.Helper().debug("Output does not exist: " + str(file))
                    # if only one file does not exist,
                    # we know we have to wait longer
                    allOutputsExist = False
                    break
                
        self.Helper().debug("All outputs exist!")        
                
        return outputAlignedImages


    
    '''=========================================================================================='''
    def Resample(self, launchCommandPrefix, filePathsList, templateFilePath, transformDirectory, outputSegmentationDirectory):
        '''
            Resample a set of segmentations using exising transforms
            
            launchCommandPrefix
                prefix for the actual registration command
            filePathsList
                list of existing filepaths to segmentations
            templateFilePath
                file path to the template
            transformDirectory
                directory to existing transformations
                the transformation has to be named after the basename of the filepaths with a .mat extension      
            outputSegmentationDirectory
                directory to save the resampled images
                
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

        # loop through filePathsList and start resample command
        for segmentationFilePath in filePathsList:

            # do not resample the same file
            if os.path.basename(segmentationFilePath) == os.path.basename(templateFilePath):
                continue

            segmentationName = os.path.splitext(os.path.basename(segmentationFilePath))[0]

            transformFilePath = transformDirectory + str(segmentationName) + ".mat"                
                
            # generate file path to output segmentation
            # by getting the filename of the case and appending it to the outputSegmentationDirectory
            outputSegmentationFilePath = outputSegmentationDirectory + str(segmentationName) + ".nrrd"

            command = str(launchCommandPrefix) + self.Helper().GetResampleCommand(segmentationFilePath,
                                                                                  templateFilePath,
                                                                                  transformFilePath,
                                                                                  outputSegmentationFilePath)

            self.Helper().debug("Resample command: " + str(command))
            
            if self.__dryRun:
                self.Helper().info("DRYRUN - skipping execution..")
            else:
                os.system(command)

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
                os.system(command)

        return True
    
    
    
    '''=========================================================================================='''
    def CombineToAtlas(self, filePathsList, labelsList, reCastString, useNormalization, outputAtlasDirectory):
        '''
            Combine segmentations to an Atlas based on labels. For each label an Atlas gets created. Additionally,
            a combined Atlas is generated. All output is saved.
            
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
            useNormalization
                flag to enable the normalization of Atlas values between 0 and 1
                0: disable
                1: enable
            outputAtlasDirectory
                directory to save the Atlases
                
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

        if useNormalization == 1:
            normalize = True
        else:
            normalize = False
            
        if not outputAtlasDirectory:
            self.Helper().info("Empty outputAtlasDirectory for CombineToAtlas() command. Aborting..")
            return False
        
        # the combined atlas imageData
        atlas = slicer.vtkImageData()
        
        firstAtlasRun = True

        # loop through all labels
        for label in labelsList:
            
            # for each label, we create an atlas using all manual segmentations
            currentLabelAtlas = slicer.vtkImageData()
            firstRun = True        

            # loop through all segmentations
            for segmentationFilePath in filePathsList:
                
                # read the manual segmentation
                currentSegmentation = slicer.vtkImageData()
                currentSegmentation.DeepCopy(self.Helper().LoadImage(segmentationFilePath))
                
                # binarize the current segmentation
                currentSegmentation.DeepCopy(self.Helper().BinarizeImageByLabel(currentSegmentation, label))

                if firstRun:
                    # binarize an image by a label to the currentLabelAtlas
                    currentLabelAtlas.DeepCopy(currentSegmentation)
                    firstRun = False
                else:
                    # combine a binarized image by the current label with the existing currentLabelAtlas
                    currentLabelAtlas.DeepCopy(self.Helper().AddImages(currentSegmentation, currentLabelAtlas))

            # and combine it with the other label atlases
            if firstAtlasRun:
                # the first atlas iteration, just copy the currentLabelAtlas
                atlas.DeepCopy(currentLabelAtlas)
                firstAtlasRun = False
            else:
                # all other runs, add the currentLabelAtlas to the existing atlas
                atlas.DeepCopy(self.Helper().AddImages(currentLabelAtlas, atlas))
                
            # now we copied the currentLabelAtlas to the atlas
            # so we can modify currentLabelAtlas before saving..
            
            # normalize the currentLabelAtlas, if requested
            if normalize:
                self.Helper().info("Normalizing atlas for label " + str(label) + " to 0..1")
                currentLabelAtlas.DeepCopy(self.Helper().DivideImage(currentLabelAtlas, len(filePathsList)))
            else:
                # re-Cast, only if not normalized
                self.Helper().info("Re-Casting atlas for label " + str(label) + " to " + str(reCastString))
                currentLabelAtlas.DeepCopy(self.Helper().ReCastImage(currentLabelAtlas, str(reCastString)))

            # now save the currentLabelAtlas
            self.Helper().SaveImage(str(outputAtlasDirectory) + "atlas" + str(label) + ".nrrd", currentLabelAtlas)
            
        # now we attack the combined atlas   
        
        # normalize the combined Atlas, if requested 
        if normalize:
            self.Helper().info("Normalizing combined atlas to 0..1")
            atlas.DeepCopy(self.Helper().DivideImage(atlas, len(filePathsList)))
        else:
            # re-Cast, only if not normalized
            self.Helper().info("Re-Casting atlas to " + str(reCastString))
            atlas.DeepCopy(self.Helper().ReCastImage(atlas, str(reCastString)))
            
        # now save the atlas
        self.Helper().SaveImage(str(outputAtlasDirectory) + "atlas.nrrd", atlas)
            
        return True



