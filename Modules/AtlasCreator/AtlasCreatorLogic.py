from Slicer import slicer
import os, glob

class AtlasCreatorLogic(object):

    def __init__(self,parentClass):

        self._parentClass = parentClass

    def GenerateAtlas(self,inputOriginalsPath,inputManualSegmentationsPath,outputPath,defCase,onlyAffineReg,saveTransforms,saveDeformationFields,labels):
        self._parentClass.GetHelper().debug("--------------------------------------------------------------------------------")
        self._parentClass.GetHelper().debug("--------------------------------------------------------------------------------")
        self._parentClass.GetHelper().debug("AtlasCreator: GenerateAtlas() called")
        self._parentClass.GetHelper().debug("Path to Original Images: " + str(inputOriginalsPath))
        self._parentClass.GetHelper().debug("Path to Manual Segmentations: " + str(inputManualSegmentationsPath))
        self._parentClass.GetHelper().debug("Output path: " + str(outputPath))
        self._parentClass.GetHelper().debug("Default case: " + str(defCase))
        self._parentClass.GetHelper().debug("Use only affine registration: " + str(onlyAffineReg))
        self._parentClass.GetHelper().debug("Save Transforms: " + str(saveTransforms))
        self._parentClass.GetHelper().debug("Save Deformation Fields: " + str(saveDeformationFields))
        self._parentClass.GetHelper().debug("Labels: " + str(labels))
        self._parentClass.GetHelper().debug("--------------------------------------------------------------------------------")

        if not inputOriginalsPath or not inputManualSegmentationsPath or not outputPath:
            self._parentClass.GetHelper().debug("Input paths for the original images and the manual segmentations are required.")
            return 0

        # find default case
        if defCase:
            defaultCase = os.path.join(inputOriginalsPath, defCase)
            defaultCaseSeg = os.path.join(inputManualSegmentationsPath, defCase)

        # get the extension of the default case, we will use to find all other cases
        extension = os.path.splitext(defCase)[1]

        # get the Slicer paths without environment variables
        slicerDir = os.path.normpath(str(slicer.Application.GetBinDir())+"/../")
        slicerLauncher = os.path.normpath(slicerDir+"/Slicer3")
        slicerBinDir = os.path.normpath(str(slicer.Application.GetBinDir()))+"/"
        slicerPluginsDir = os.path.normpath(slicerDir+"lib/Slicer3/Plugins")+"/"
        slicerTempDir = os.path.normpath(str(slicer.Application.GetTemporaryDirectory()))

        #
        # registration stage
        #
        self._parentClass.GetHelper().debug("*** (1) Registration Stage ***")

        for origFile in glob.glob(os.path.join(inputOriginalsPath, '*'+extension)):

            if origFile!=defaultCase:
                self._parentClass.GetHelper().debug("Registering "+origFile+" to default case..")

                caseFile = os.path.basename(origFile)
                caseName = caseFile.rstrip('.nrrd')
                if saveTransforms:
                    outputTransform = outputPath+"/"+caseName+".mat"
                else:
                    outputTransform = slicerTempDir+"/"+caseName+".mat"

                os.system(slicerLauncher+" --launch "+slicerBinDir+self.Register(defaultCase,origFile,outputTransform,onlyAffineReg))


        #
        # resample stage
        #
        self._parentClass.GetHelper().debug("*** (2) Resample Stage ***")

        for segmentfile in glob.glob(os.path.join(inputManualSegmentationsPath, '*'+extension)):

            if segmentfile!=defaultCaseSeg:
                self._parentClass.GetHelper().debug("Resampling "+segmentfile+" using the calculated transform..")

                caseFile = os.path.basename(segmentfile)
                caseName = caseFile.rstrip('.nrrd')
                origFile = inputOriginalsPath+"/"+caseFile
                if saveTransforms:
                    inputTransform = outputPath+"/"+caseName+".mat"
                else:
                    inputTransform = slicerTempDir+"/"+caseName+".mat"
                outputRegisteredSegmentation = slicerTempDir+"/"+caseName+".nrrd"

                os.system(slicerLauncher+" --launch "+slicerBinDir+self.Resample(segmentfile,origFile,inputTransform,outputRegisteredSegmentation))


        #
        # save deformationfield stage
        #
        self._parentClass.GetHelper().debug("*** (3) Save Deformationfields ***")

        if not saveDeformationFields or onlyAffineReg:
            self._parentClass.GetHelper().debug("Saving was disabled or only affine registration was selected.")
        else:
            for origFile in glob.glob(os.path.join(inputOriginalsPath, '*'+extension)):

                if origFile!=defaultCase:
                    self._parentClass.GetHelper().debug("Saving the deformation field for "+origFile+"..")

                    caseFile = os.path.basename(origFile)
                    caseName = caseFile.rstrip('.nrrd')
                    if saveTransforms:
                        inputTransform = outputPath+"/"+caseName+".mat"
                    else:
                        inputTransform = slicerTempDir+"/"+caseName+".mat"
                    outputDeformationField = outputPath+"/"+caseName+"DeformationField.nrrd"

                    os.system(slicerLauncher+" --launch "+slicerPluginsDir+self.SaveDeformationField(origFile,inputTransform,outputDeformationField))

        #
        # create atlas stage
        #
        self._parentClass.GetHelper().debug("*** (4) Create Atlas Stage ***")

        atlas = slicer.vtkImageData()
        imageDataBuffer = slicer.vtkImageData()
        firstAtlas = True

        for label in labels:
            
            # for each label, we create an atlas using all manual segmentations
            currentLabelAtlas = slicer.vtkImageData()
            firstrun = True        

            for segmentfile in glob.glob(os.path.join(inputManualSegmentationsPath, '*'+extension)):
                
                # read the manual segmentation
                reader = slicer.vtkNRRDReader()
                reader.SetFileName(segmentfile)
                reader.Update()

                currentSegmentation = slicer.vtkImageData()
                currentSegmentation.DeepCopy(reader.GetOutput())
                
                # threshold for current label
                threshold = slicer.vtkImageThreshold()
                threshold.SetInput(currentSegmentation)
                threshold.ThresholdBetween(label,label)
                threshold.ReplaceInOn()
                threshold.ReplaceOutOn()
                threshold.SetInValue(1)
                threshold.SetOutValue(0)
                threshold.Update()

                if firstrun:
                    # copy just the first segmentation
                    currentLabelAtlas.DeepCopy(threshold.GetOutput())
                    firstrun = False
                else:
                    # copy the combined segmentation
                    add = slicer.vtkImageMathematics()
                    add.SetInput1(threshold.GetOutput())
                    add.SetInput2(currentLabelAtlas)
                    add.SetOperationToAdd()
                    add.Update()
                    currentLabelAtlas.DeepCopy(add.GetOutput())
    
             #numberOfManualSegmentations = len(glob.glob(os.path.join(inputManualSegmentationsPath, '*.nrrd')))
    
#            # now we divide our label atlas by the number of manual segmentations
#            divide = slicer.vtkImageMathematics()
#            divide.SetInput(currentLabelAtlas)
#            divide.SetOperationToMultiplyByK()
#            divide.SetConstantK(1/numberOfManualSegmentations)
#            divide.Update()
#            
    
            # and combine it with the other label atlases
            if firstAtlas:
                atlas.DeepCopy(currentLabelAtlas)
                firstAtlas = False
            else:
                add = slicer.vtkImageMathematics()
                add.SetInput1(currentLabelAtlas)
                add.SetInput2(atlas)
                add.SetOperationToAdd()
                add.Update()    
                atlas.DeepCopy(add.GetOutput())

        writer = slicer.vtkNRRDWriter()
        writer.SetFileName(os.path.normpath(outputPath+"/atlas.nrrd"))
        writer.SetInput(atlas)
        writer.Update()
            
        #
        # done creating the atlas
        #
        self._parentClass.GetHelper().debug("Atlas was created and saved to "+os.path.normpath(outputPath+"/atlas.nrrd")+".")
        self._parentClass.GetHelper().debug("--------------------------------------------------------------------------------")
        self._parentClass.GetHelper().debug("--------------------------------------------------------------------------------")
        self._parentClass.GetHelper().debug("--------------------------------------------------------------------------------")
        
        return 1


    def Register(self,defaultCase,origFile,outputTransform,onlyAffineReg):
        registrationCommand = "BRAINSFit"
        registrationCommand += " --fixedVolume "+os.path.normpath(defaultCase)
        registrationCommand += " --movingVolume "+os.path.normpath(origFile)
        registrationCommand += " --outputTransform "+os.path.normpath(outputTransform)
        registrationCommand += " --maxBSplineDisplacement 10.0 --outputVolumePixelType short --backgroundFillValue 0.0 --interpolationMode Linear"
        #registrationCommand += " --maskProcessingMode  ROIAUTO --ROIAutoDilateSize 3.0 --maskInferiorCutOffFromCenter 65.0"
        registrationCommand += " --useRigid --useScaleVersor3D --useScaleSkewVersor3D"
        #registrationCommand += " --initializeTransformMode useCenterOfHeadAlign --useRigid --useScaleVersor3D --useScaleSkewVersor3D"
        registrationCommand += " --useAffine"

        if not onlyAffineReg:
            registrationCommand += " --useBSpline"

        registrationCommand += " --numberOfSamples 100000 --numberOfIterations 1500"
        registrationCommand += " --translationScale 1000.0 --reproportionScale 1.0 --skewScale 1.0 --splineGridSize 28,20,24 --fixedVolumeTimeIndex 0"
        registrationCommand += " --movingVolumeTimeIndex 0 --medianFilterSize 0,0,0 --numberOfHistogramBins 50 --numberOfMatchPoints 10 --useCachingOfBSplineWeightsMode ON"
        registrationCommand += " --useExplicitPDFDerivativesMode AUTO --relaxationFactor 0.5 --failureExitCode -1 --debugNumberOfThreads -1"
        registrationCommand += " --debugLevel 0 --costFunctionConvergenceFactor 1e+9 --projectedGradientTolerance 1e-5"


        return registrationCommand


    def Resample(self,segmentFile,origFile,inputTransform,outputRegisteredSegmentation):
        resampleCommand = "BRAINSResample"
        resampleCommand += " --inputVolume "+os.path.normpath(segmentFile)
        resampleCommand += " --referenceVolume "+os.path.normpath(origFile)
        resampleCommand += " --warpTransform "+os.path.normpath(inputTransform)
        resampleCommand += " --outputVolume "+os.path.normpath(outputRegisteredSegmentation)
        resampleCommand += " --defaultValue 8.0 --pixelType short --interpolationMode NearestNeighbor"

        return resampleCommand


    def SaveDeformationField(self,origFile,inputTransform,outputDeformationField):
    
        saveDefFieldCommand = "BSplineToDeformationField"
        saveDefFieldCommand += " --refImage "+os.path.normpath(origFile)
        saveDefFieldCommand += " --tfm "+os.path.normpath(inputTransform)
        saveDefFieldCommand += " --defImage "+os.path.normpath(outputDeformationField)

        return saveDefFieldCommand


