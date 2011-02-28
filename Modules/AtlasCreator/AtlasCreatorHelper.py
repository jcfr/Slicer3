from Slicer import slicer
from time import strftime
from math import floor
import os
import glob
import shutil
import sys

class AtlasCreatorHelper(object):
    '''
        The helper class for the Atlas Creator
    '''



    '''=========================================================================================='''
    def __init__(self,parentClass):
        '''
            Initialize this class
            
            parentClass
                a pointer to the parentClass
                
            Returns
                n/a
        '''

        self.__parentClass = parentClass
        
        # deactivated by default
        self.__debugMode = 0
        
        self.__lookupIJKRAS = dict()



    '''=========================================================================================='''
    def EnableDebugMode(self):
        '''
            Enables the debug Mode
            
            Returns
                n/a
        '''
        self.__debugMode = 1



    '''=========================================================================================='''
    def debug(self,message):
        ''' 
            Print statement to stdout including a timestamp and a debug label
            
            message
                value to print, will be casted to String
            
            Returns
                n/a
        '''        
        
        if self.__debugMode:

            print "[AtlasCreator " + strftime("%H:%M:%S") + "] DEBUG: " + str(message)

            # flush, to always show the output
            try:
                sys.stdout.flush()
            except:
                pass



    '''=========================================================================================='''
    def info(self,message):
        ''' 
            Print statement to stdout including a timestamp
            
            message
                value to print, will be casted to String
            
            Returns
                n/a
        '''         
        # activated by default
        infoMode = 1

        if infoMode:

            print "[AtlasCreator " + strftime("%H:%M:%S") + "] " + str(message)

            # flush, to always show the output
            try:
                sys.stdout.flush()
            except:
                pass



    '''=========================================================================================='''
    def DeleteFilesAndDirectory(self,filePathList,wipe=False):
        ''' 
            Delete all files of a filePathList and also the directory
            
            filePathList
                the list of file paths to delete
            wipe
                if TRUE, wipe it without asking questions!
            
            Returns
                TRUE or FALSE depending on success
        '''         
        
        # if we want to wipe something, just do it!
        if wipe:
            if os.path.isdir(os.path.dirname(filePathList[0])):
                shutil.rmtree(os.path.dirname(filePathList[0]), True, None)
                return True
            
        # now delete the content in the temporary directory
        for file in filePathList:
            if os.path.isfile(file):
                os.remove(file)
            else:
                return False
            
        # now delete the whole temporary directory
        if os.path.isdir(os.path.dirname(filePathList[0])):
            os.rmdir(os.path.dirname(filePathList[0]))
        else:
            return False     
        
        
        return True
    


    '''=========================================================================================='''
    def GetSlicerLaunchPrefix(self,useCMTK=False):
        '''
            Get the path to the 3D Slicer launcher ready to start a plugin. (OS independent)
            
            useCMTK
                If TRUE, add the CMTK directory instead of the Plugin Directory
            
            Returns
                the path to the 3D Slicer launcher configured to start a plugin as a String
                f.e. /home/user/Slicer3-build/Slicer3 --launch /home/user/Slicer3-build/lib/Slicer3/Plugins/
                If useCMTK is TRUE:
                f.e. /home/user/Slicer3-build/Slicer3 --launch /home/user/Slicer3user/@SVNREV@/CMTK4Slicer
                pay attention to the trailing /
        '''
        # read the Slicer environment
        slicerDir = str(slicer.Application.GetBinDir())+os.sep+".."+os.sep
        slicerLauncher = os.path.normpath(slicerDir+"Slicer3")
        self.info("Found 3D Slicer launcher at " + str(slicerLauncher))
        
        if not os.path.isfile(slicerLauncher):
            self.info("ERROR! Could not find Slicer3 launcher.. Aborting..")
            return None
        
        if useCMTK:
            executableDir = os.path.normpath(self.GetCMTKInstallationDirectory())
        else:
            executableDir = os.path.normpath(slicer.Application.GetPluginsDir())
        
        if not os.path.isdir(executableDir):
            self.info("ERROR! Could not find Executable directory.. Aborting..")
            self.info("ERROR! Path not found: " + str(executableDir))
            return None
            
            
        launchCommandPrefix = slicerLauncher + " --launch " + executableDir + os.sep
        self.debug("The launchCommandPrefix is " + str(launchCommandPrefix))
        
        return launchCommandPrefix



    '''=========================================================================================='''
    def GetSlicerTemporaryDirectory(self):
        '''
            Get the path to the 3D Slicer temporary directory
            
            Returns
                the path to the 3D Slicer temporary directory as a String
                pay attention to the trailing /
        '''
        return os.path.normpath(str(slicer.Application.GetTemporaryDirectory())) + os.sep
    
    
    
    '''=========================================================================================='''
    def BinarizeImageByLabel(self,imageData,labelValue):
        ''' 
            Binarize an image to 0 and 1, while just keeping the voxels with the labelValue
            
            imageData
                vtkImageData
            labelValue
                the value of the voxels to keep
            
            Returns
                vtkImageData after binarizing
        '''            

        threshold = slicer.vtkImageThreshold()
        threshold.SetInput(imageData)
        threshold.ThresholdBetween(labelValue,labelValue)
        threshold.ReplaceInOn()
        threshold.ReplaceOutOn()
        threshold.SetInValue(1)
        threshold.SetOutValue(0)
        threshold.Update()
        
        output = slicer.vtkImageData()
        output.DeepCopy(threshold.GetOutput())
        
        return output        
                
                
                
    '''=========================================================================================='''
    def DivideImage(self,imageData,dividend):
        ''' 
            Divide an image
            This includes automatic Re-Cast to Float
            
            imageData
                vtkImageData
            dividend
                the number to divide with, will be casted to float
            
            Returns
                vtkImageData after division
        '''            
        imageData.DeepCopy(self.ReCastImage(imageData, "Float"))
        div = slicer.vtkImageMathematics()
        div.SetInput(imageData)
        div.SetOperationToMultiplyByK()
        div.SetConstantK(1.0/float(dividend))
        div.Update()
        
        output = slicer.vtkImageData()
        output.DeepCopy(div.GetOutput())
        
        return output        
    
         

    '''=========================================================================================='''
    def AddImages(self,imageData1,imageData2):
        ''' 
            Adds two images
            
            imageData1
                vtkImageData
            imageData2
                vtkImageData
            
            Returns
                vtkImageData after addition
        '''            
        add = slicer.vtkImageMathematics()
        add.SetInput1(imageData1)
        add.SetInput2(imageData2)
        add.SetOperationToAdd()
        add.Update()
        
        output = slicer.vtkImageData()
        output.DeepCopy(add.GetOutput())
        
        return output        


        
    '''=========================================================================================='''
    def ReCastImage(self,imageData,castString):
        ''' 
            Re-Cast an image to a different type
            
            imageData
                vtkImageData
            castString
                New type as a String, possible values are
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
                
            Returns
                vtkImageData after Re-Cast
        '''        
        cast = slicer.vtkImageCast()
        cast.SetInput(imageData)
        cast.ClampOverflowOn()
        
        if (castString == "Char"):
            cast.SetOutputScalarTypeToChar()
        elif (castString == "Unsigned Char"):
            cast.SetOutputScalarTypeToUnsignedChar()
        elif (castString == "Double"):
            cast.SetOutputScalarTypeToUnsignedChar()
        elif (castString == "Float"):
            cast.SetOutputScalarTypeToFloat()
        elif (castString == "Int"):
            cast.SetOutputScalarTypeToInt()
        elif (castString == "Unsigned Int"):
            cast.SetOutputScalarTypeToUnsignedInt()
        elif (castString == "Long"):
            cast.SetOutputScalarTypeToLong()
        elif (castString == "Unsigned Long"):
            cast.SetOutputScalarTypeToUnsignedLong()
        elif (castString == "Short"):
            cast.SetOutputScalarTypeToShort()
        elif (castString == "Unsigned Short"):
            cast.SetOutputScalarTypeToUnsignedShort()
        else:
            cast.SetOutputScalarTypeToShort()
            
        cast.Update()
        
        output = slicer.vtkImageData()
        output.DeepCopy(cast.GetOutput())
        
        return output     



    '''=========================================================================================='''    
    def GetLabels(self,labelMap):
        '''
            Get the list of labels in a labelMap
            
            labelMap
                vtkImageData of a labelMap volume
            
            Returns
                the list of labels
        '''        
        accum = slicer.vtkImageAccumulate()
        accum.SetInput(labelMap)
        accum.UpdateWholeExtent()
        accum.Update()
        data = accum.GetOutput()
        numBins = accum.GetComponentExtent()[1]
        nonZeroLabels = []
        for i in range(1, numBins + 1):
            numVoxels = data.GetScalarComponentAsDouble(i,0,0,0)
            if (numVoxels>0):
                nonZeroLabels.append(i)
        
        self.debug("Found the following labels: " + str(nonZeroLabels))        
        
        return nonZeroLabels
    
    

    '''=========================================================================================='''    
    def GetBRAINSFitRegistrationCommand(self,templateFilePath,movingImageFilePath,outputTransformFilePath,outputImageFilePath,onlyAffineReg,multiThreading,backgroundValue):
        '''
            Get the command to Register an image to a template using BRAINSFit
            
            templateFilePath
                the file path to the template (target) image
            movingImageFilePath
                the file path to the moving image
            outputTransformFilePath
                the file path to the transformation output
            outputImageFilePath
                the file path to the aligned image output
            onlyAffineReg
                if true, just use affine registration and no BSpline
            multiThreading
                if true, use multi threading
            backgroundValue
                the backgroundValue of the moving image                
                
            Returns
                the command to Register an image
        '''        
        
        registrationCommand = "BRAINSFit"
        registrationCommand += " --fixedVolume "+os.path.normpath(templateFilePath)
        registrationCommand += " --movingVolume "+os.path.normpath(movingImageFilePath)
        registrationCommand += " --outputTransform "+os.path.normpath(outputTransformFilePath)
        registrationCommand += " --outputVolume "+os.path.normpath(outputImageFilePath)
        registrationCommand += " --maxBSplineDisplacement 10.0 --outputVolumePixelType short --interpolationMode Linear"
        #registrationCommand += " --maskProcessingMode  ROIAUTO --ROIAutoDilateSize 3.0 --maskInferiorCutOffFromCenter 65.0"
        #registrationCommand += " --useRigid --useScaleVersor3D --useScaleSkewVersor3D"
        registrationCommand += " --initializeTransformMode useCenterOfHeadAlign --useRigid --useScaleVersor3D --useScaleSkewVersor3D"
        registrationCommand += " --useAffine"

        if not onlyAffineReg:
            registrationCommand += " --useBSpline"
            
        if not multiThreading:
            registrationCommand += " --debugNumberOfThreads 1"
        else:
            registrationCommand += " --debugNumberOfThreads -1"

        registrationCommand += " --backgroundFillValue " + str(backgroundValue)
        
        registrationCommand += " --numberOfSamples 100000 --numberOfIterations 1500"
        registrationCommand += " --translationScale 1000.0 --reproportionScale 1.0 --skewScale 1.0 --splineGridSize 28,20,24 --fixedVolumeTimeIndex 0"
        registrationCommand += " --movingVolumeTimeIndex 0 --medianFilterSize 0,0,0 --numberOfHistogramBins 50 --numberOfMatchPoints 10 --useCachingOfBSplineWeightsMode ON"
        registrationCommand += " --useExplicitPDFDerivativesMode AUTO --relaxationFactor 0.5 --failureExitCode -1"
        registrationCommand += " --debugLevel 0 --costFunctionConvergenceFactor 1e+9 --projectedGradientTolerance 1e-5"


        return str(registrationCommand)


    
    '''=========================================================================================='''    
    def GetBRAINSFitResampleCommand(self,segmentationFilePath,templateFilePath,transformFilePath,outputSegmentationFilePath):
        '''
            Get the command to Resample a segmentation using BRAINSFit
            
            segmentationFilePath
                the file path to the segmentation
            templateFilePath
                the file path to the template used to define the output space
            transformFilePath
                the file path to the existing transformation          
            outputSegmentationFilePath
                the file path to the resampled segmentation output
                
            Returns
                the command to Resample a segmentation
        '''
        resampleCommand = "BRAINSResample"
        resampleCommand += " --inputVolume "+os.path.normpath(segmentationFilePath)
        resampleCommand += " --referenceVolume "+os.path.normpath(templateFilePath)
        resampleCommand += " --warpTransform "+os.path.normpath(transformFilePath)
        resampleCommand += " --outputVolume "+os.path.normpath(outputSegmentationFilePath)
        resampleCommand += " --defaultValue 8.0 --pixelType short --interpolationMode NearestNeighbor"

        return str(resampleCommand)



    '''=========================================================================================='''    
    def GetCMTKRegistrationCommand(self,templateFilePath,movingImageFilePath,outputTransformFilePath,outputImageFilePath,onlyAffineReg,multiThreading,backgroundValue):
        '''
            Get the command to Register an image to a template using CMTK
            
            templateFilePath
                the file path to the template (target) image
            movingImageFilePath
                the file path to the moving image
            outputTransformFilePath
                the file path to the transformation output
            outputImageFilePath
                the file path to the aligned image output
            onlyAffineReg
                if true, just use affine registration and no BSpline
            multiThreading
                if true, use multi threading
            backgroundValue
                the backgroundValue of the moving image
                
            Returns
                the command to Register an image
        '''        
    
        
        registrationCommand = "registration"
        registrationCommand += " --initxlate --exploration 8.0 --dofs 6 --dofs 9 --accuracy 0.5"
        registrationCommand += " -o " + os.path.normpath(outputTransformFilePath)
        registrationCommand += " --write-reformatted " + os.path.normpath(outputImageFilePath)
        registrationCommand += " " + os.path.normpath(templateFilePath)
        registrationCommand += " " + os.path.normpath(movingImageFilePath)

        return str(registrationCommand)


    
    '''=========================================================================================='''    
    def GetCMTKResampleCommand(self,segmentationFilePath,templateFilePath,transformFilePath,outputSegmentationFilePath):
        '''
            Get the command to Resample a segmentation using CMTK
            
            segmentationFilePath
                the file path to the segmentation
            templateFilePath
                the file path to the template used to define the output space
            transformFilePath
                the file path to the existing transformation          
            outputSegmentationFilePath
                the file path to the resampled segmentation output
                
            Returns
                the command to Resample a segmentation
        '''
        
        resampleCommand = "reformatx"
        resampleCommand += " -o " + os.path.normpath(outputSegmentationFilePath)
        resampleCommand += " --floating " + os.path.normpath(templateFilePath)
        resampleCommand += " " + os.path.normpath(segmentationFilePath)
        resampleCommand += " " + os.path.normpath(transformFilePath)
        
        return str(resampleCommand)



    '''=========================================================================================='''    
    def GetSaveDeformationFieldCommand(self,movingImageFilePath,transformFilePath,outputDeformationFieldFilePath):
        '''
            Get the command to Save a deformation field
            
            movingImageFilePath
                the file path to the moving image used as the reference image
            transformFilePath
                the file path to the existing BSpline transformation
            outputDeformationFieldFilePath
                the file path to the Deformation Field output
                
            Returns
                the command to save a deformation field
        '''        
    
        saveDefFieldCommand = "BSplineToDeformationField"
        saveDefFieldCommand += " --refImage "+os.path.normpath(movingImageFilePath)
        saveDefFieldCommand += " --tfm "+os.path.normpath(transformFilePath)
        saveDefFieldCommand += " --defImage "+os.path.normpath(outputDeformationFieldFilePath)

        return str(saveDefFieldCommand)



    '''=========================================================================================='''    
    def LoadVolume(self,filePath):
        '''
            Loads an image as a MRMLScalarVolumeNode and add it to the MRML Scene
            
            filePath
                the file path to an existing image in any 3D Slicer readable format
            
            Returns
                vtkMRMLScalarVolumeNode containing the loaded image and meta information
        '''
        # if we do not have a filePath, exit and return None
        if not filePath:
            return None
        
        filePath = os.path.normpath(filePath)
        
        # load the file using Slicer's mechanism
        volumeNode = slicer.VolumesGUI.GetLogic().AddArchetypeScalarVolume(filePath,"tmpAtlasCreatorNode")
        
        return volumeNode



    '''=========================================================================================='''    
    def SaveVolume(self,filePath,volumeNode):
        '''
            Saves a MRMLScalarVolumeNode
            
            filePath
                the file path to save a MRMLScalarVolumeNode in any 3D Slicer readable format
            
            volumeNode
                vtkMRMLScalarVolumeNode to save
            
            Returns
                TRUE or FALSE depending on success
        '''
        # if we do not have a filePath, exit and return None
        if not filePath:
            return False

        filePath = os.path.normpath(filePath)
        
        success = slicer.VolumesGUI.GetLogic().SaveArchetypeVolume(filePath,volumeNode)
        
        return success
    
    
    
    '''=========================================================================================='''   
    def ConvertDirectoryToList(self,directory):
        '''
            Convert a directory to a list of filePaths. While reading the directory,
            only real files are added to the list, sub-directories as well as links are ignored.
            
            directory
                a String containing the path to the directory
                
            Returns
                list of filePaths
        '''
        listOfFilePaths = []
        
        if not directory:
            return listOfFilePaths
        
        if not os.path.isdir(directory):
            return listOfFilePaths
        
        # loop through the directory
        for entry in glob.glob(os.path.join(directory, '*.*')):
            
            if os.path.isfile(entry) and not os.path.islink(entry):
                
                # this is a real file and not a link or subdir
                # add it to the list
                listOfFilePaths.append(os.path.normpath(entry))
                
        return listOfFilePaths
    
                
        
    '''=========================================================================================='''    
    def DisplayImageInSlicer(self,filePath,nodeName,):
        '''
            Displays an image in 3D Slicer and re-calculates Window/Level on the displayNode
            
            filePath
                the file path to an existing image in any 3D Slicer readable format
                
            nodeName
                the name of the new volume Node
            
            Returns
                The ID of the MRML node as a String or None on failure
        '''
        # if we do not have a filePath, exit and return None
        if not filePath:
            return None
        
        if not nodeName:
            nodeName = "AtlasCreatorNode"
        
        filePath = os.path.normpath(filePath)
        
        # load the file using Slicer's mechanism
        volumeNode = slicer.VolumesGUI.GetLogic().AddArchetypeScalarVolume(filePath,nodeName)

        # get the displayNode to re-calculate Window/Level
        displayNode = volumeNode.GetDisplayNode()
        if displayNode:
            newDisplayNode = displayNode.NewInstance()
            newDisplayNode.Copy(displayNode)
            slicer.MRMLScene.AddNodeNoNotify(newDisplayNode)
            volumeNode.SetAndObserveDisplayNodeID(newDisplayNode.GetID())
            newDisplayNode.AutoWindowLevelOff()
            newDisplayNode.AutoWindowLevelOn()
        
        return str(volumeNode.GetID())
    
    
    
    '''=========================================================================================='''
    def GuessBackgroundValue(self,filePath):
        '''
            Guesses the background value for a given image
            
            filePath
                the filepath to an existing image
            
            Returns
                The guess for a background value
        '''
        emlogic = slicer.vtkEMSegmentLogic()
        
        # load the image
        node = self.LoadVolume(filePath)
        
        # guess the bg value
        guess = emlogic.GuessRegistrationBackgroundLevel(node)
        
        # remove the node from the scene
        slicer.MRMLScene.RemoveNode(node)
        
        # and return the bg value
        return guess
    
    

    '''==========================================================================================''' 
    def GetCMTKInstallationDirectory(self):
        '''
            Return the installation directory of the CMTK extension.
            
            Returns
                The installation directory as String or None if CMTK was not found
        '''
        extensionsDirForCurrentRev = os.path.join(str(slicer.Application.GetExtensionsInstallPath()),str(slicer.Application.GetSvnRevision()))
        self.debug("Looking for CMTK in " + str(extensionsDirForCurrentRev))
        
        if not os.path.isdir(extensionsDirForCurrentRev):
            # extension directory does not exist
            return None
        
        cmtkDir = os.path.join(extensionsDirForCurrentRev,"CMTK4Slicer")
        if not os.path.isdir(cmtkDir):
            # CMTK directory does not exist
            return None
        
        if not os.path.isfile(os.path.join(cmtkDir,"registration")):
            # CMTK registration command not found
            return None

        # CMTK seems to be available
        return cmtkDir
    
