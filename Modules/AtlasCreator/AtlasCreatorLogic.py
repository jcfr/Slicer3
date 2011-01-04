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

        if not inputOriginalsPath or not inputManualSegmentationsPath:
            self._parentClass.GetHelper().debug("Input paths for the original images and the manual segmentations are required.")
            return 0

        # find default case
        if defCase:
            defaultCase = os.path.join(inputOriginalsPath, defCase)
            defaultCaseSeg = os.path.join(inputManualSegmentationsPath, defCase)
        else:
            # no default case was specified, choose the first one
            defaultCase = glob.glob(os.path.join(inputOriginalsPath, '*.nrrd'))[0];
            defaultCaseSeg = glob.glob(os.path.join(inputManualSegmentationsPath, '*.nrrd'))[0];

        # get the Slicer paths without environment variables
        slicerDir = str(slicer.Application.GetBinDir())+"/../"
        slicerPluginsDir = slicerDir+"lib/Slicer3/Plugins/"
        slicerTempDir = str(slicer.Application.GetTemporaryDirectory())

        #
        # registration stage
        #
        self._parentClass.GetHelper().debug("*** (1) Registration Stage ***")

        for origFile in glob.glob(os.path.join(inputOriginalsPath, '*.nrrd')):

            if origFile!=defaultCase:
                self._parentClass.GetHelper().debug("Registering "+origFile+" to default case..")

                caseFile = os.path.basename(origFile)
                caseName = caseFile.rstrip('.nrrd')
                if saveTransforms:
                    outputTransform = outputPath+"/"+caseName+".mat"
                else:
                    outputTransform = slicerTempDir+"/"+caseName+".mat"

                os.system(slicerDir+"Slicer3 --launch "+slicerPluginsDir+self.Register(defaultCase,origFile,outputTransform,onlyAffineReg))


        #
        # resample stage
        #
        self._parentClass.GetHelper().debug("*** (2) Resample Stage ***")

        for segmentfile in glob.glob(os.path.join(inputManualSegmentationsPath, '*.nrrd')):

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

                os.system(slicerDir+"Slicer3 --launch "+slicerPluginsDir+self.Resample(segmentfile,origFile,inputTransform,outputRegisteredSegmentation))


        #
        # save deformationfield stage
        #
        self._parentClass.GetHelper().debug("*** (3) Save Deformationfields ***")

        if not saveDeformationFields:
            self._parentClass.GetHelper().debug("Saving was disabled.")
        else:
            for origFile in glob.glob(os.path.join(inputOriginalsPath, '*.nrrd')):

                if origFile!=defaultCase:
                    self._parentClass.GetHelper().debug("Saving the deformation field for "+origFile+"..")

                    caseFile = os.path.basename(origFile)
                    caseName = caseFile.rstrip('.nrrd')
                    if saveTransforms:
                        inputTransform = outputPath+"/"+caseName+".mat"
                    else:
                        inputTransform = slicerTempDir+"/"+caseName+".mat"
                    outputDeformationField = inputOriginalsPath+"/"+caseName+"DeformationField.nrrd"

                    os.system(slicerDir+"Slicer3 --launch "+slicerPluginsDir+self.SaveDeformationField(origFile,inputTransform,outputDeformationField))

        #
        # create atlas stage
        #
        self._parentClass.GetHelper().debug("*** (4) Create Atlas Stage ***")

        atlas = slicer.vtkImageData()

        for label in labels:
            
            # for each label, we create an atlas using all manual segmentations
            currentLabelAtlas = slicer.vtkImageData()
        
            for segmentfile in glob.glob(os.path.join(inputManualSegmentationsPath, '*.nrrd')):
                
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
                threshold.ReplaceInOff()
                threshold.ReplaceOutOn()
                threshold.SetOutValue(0)
                threshold.Update()
        
                # divide by label value -> all values are 1 or 0
                shiftScale = slicer.vtkImageShiftScale()
                shiftScale.SetInput(threshold.GetOutput())
                shiftScale.SetShift(0)
                shiftScale.SetScale(1/label)
                shiftScale.Update()
        
                # combine with other segmentations
                add = slicer.vtkImageMathematics()
                add.SetInput1(shiftScale.GetOutput())
                add.SetInput2(currentLabelAtlas)
                add.SetOperationToAdd()
                add.Update()            
    
                # copy the combined segmentation
                currentLabelAtlas.DeepCopy(add)
    
            numberOfManualSegmentations = len(glob.glob(os.path.join(inputManualSegmentationsPath, '*.nrrd')))
    
            # now we divide our label atlas by the number of manual segmentations
            divide = slicer.vtkImageMathematics()
            divide.SetInput(currentLabelAtlas)
            divide.SetOperationToMultiplyByK()
            divide.SetConstantK(1/numberOfManualSegmentations)
            divide.Update()
                
            # and combine it with the other label atlases
            # 
            # the result is an image with probabilities between 0 and 1
            add = slicer.vtkImageMathematics()
            add.SetInput1(divide.GetOutput())
            add.SetInput2(atlas)
            add.SetOperationToAdd()
            add.Update()
            
            atlas.DeepCopy(add.GetOutput())
            
        #
        # done creating the atlas, now return it
        #
        self._parentClass.GetHelper().debug("Atlas was created.")
        self._parentClass.GetHelper().debug("--------------------------------------------------------------------------------")
        self._parentClass.GetHelper().debug("--------------------------------------------------------------------------------")
        self._parentClass.GetHelper().debug("--------------------------------------------------------------------------------")
        
        return atlas


    def Register(self,defaultCase,origFile,outputTransform,onlyAffineReg):
        registrationCommand = "BRAINSFit "
        registrationCommand += "--fixedVolume "+os.path.normpath(defaultCase)+" "
        registrationCommand += "--movingVolume "+os.path.normpath(origFile)+" "
        registrationCommand += "--outputTransform "+os.path.normpath(outputTransform)+" "
        registrationCommand += "--maxBSplineDisplacement 10.0 --outputVolumePixelType short --backgroundFillValue 0.0 --interpolationMode Linear "
        #registrationCommand += "--maskProcessingMode  ROIAUTO --ROIAutoDilateSize 3.0 --maskInferiorCutOffFromCenter 65.0 "
        registrationCommand += "--useRigid --useScaleVersor3D --useScaleSkewVersor3D "
        #registrationCommand += "--initializeTransformMode useCenterOfHeadAlign --useRigid --useScaleVersor3D --useScaleSkewVersor3D "
        registrationCommand += "--useAffine "

        if not onlyAffineReg:
            registrationCommand += "--useBSpline "

        registrationCommand += "--numberOfSamples 100000 --numberOfIterations 1500 "
        registrationCommand += "--translationScale 1000.0 --reproportionScale 1.0 --skewScale 1.0 --splineGridSize 28,20,24 --fixedVolumeTimeIndex 0 "
        registrationCommand += "--movingVolumeTimeIndex 0 --medianFilterSize 0,0,0 --numberOfHistogramBins 50 --numberOfMatchPoints 10 --useCachingOfBSplineWeightsMode ON "
        registrationCommand += "--useExplicitPDFDerivativesMode AUTO --relaxationFactor 0.5 --failureExitCode -1 --debugNumberOfThreads -1 "
        registrationCommand += "--debugLevel 0 --costFunctionConvergenceFactor 1e+9 --projectedGradientTolerance 1e-5"

        self._parentClass.GetHelper().debug(registrationCommand)

        return registrationCommand


    def Resample(self,segmentFile,origFile,inputTransform,outputRegisteredSegmentation):
        resampleCommand = "BRAINSResample "
        resampleCommand += "--inputVolume "+os.path.normpath(segmentFile)+" "
        resampleCommand += "--referenceVolume "+os.path.normpath(origFile)+" "
        resampleCommand += "--warpTransform "+os.path.normpath(inputTransform)+" "
        resampleCommand += "--outputVolume "+os.path.normpath(outputRegisteredSegmentation)+" "
        resampleCommand += "--defaultValue 8.0 --pixelType short --interpolationMode NearestNeighbor"

        return resampleCommand


    def SaveDeformationField(self,origFile,inputTransform,outputDeformationField):
    
        saveDefFieldCommand = "BSplineToDeformationField "
        saveDefFieldCommand += "--refImage "+os.path.normpath(origFile)+" "
        saveDefFieldCommand += "--tfm "+os.path.normpath(inputTransform)+" "
        saveDefFieldCommand += "--defImage "+os.path.normpath(outputDeformationField)

        return saveDefFieldCommand


