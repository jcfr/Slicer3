from Slicer import slicer

class AtlasCreatorConfiguration(object):
    '''
        This class reflects a parameter configuration for the Atlas Creator
    '''
    
    
    
    '''=========================================================================================='''
    def __init__(self):
        '''
            Initialize this class
            
            Returns
                n/a
        '''
        self.__originalImagesFilePathList = []
        self.__segmentationsFilePathList = []
        self.__outputDirectory = ""
        
        self.__templateType = "fixed"
        self.__dynamicTemplateIterations = 5
        self.__fixedTemplateDefaultCaseFilePath = ""
        
        self.__labelsList = []
        
        self.__toolkit = "BRAINSFit"
        
        self.__registrationType = "Affine"
        
        self.__saveTransforms = 1
        self.__deleteAlignedImages = 1
        self.__deleteAlignedSegmentations = 1
        self.__normalizeAtlases = 0

        self.__normalizeTo = 1
        
        self.__outputCast = "Short" 



    '''=========================================================================================='''
    def GetConfigurationAsString(self):
        '''
            Returns
                the current configuration as a String
        '''
        output = "Original Images: " + str(self.__originalImagesFilePathList) + "\n"
        output += "Segmentations: " + str(self.__segmentationsFilePathList) + "\n"
        output += "Output Directory: " + str(self.__outputDirectory) + "\n"
        output += "Template Type: " + str(self.__templateType) + "\n"
        output += "Dynamic Template Iterations: " + str(self.__dynamicTemplateIterations) + "\n"
        output += "Fixed Template Default Case: " + str(self.__fixedTemplateDefaultCaseFilePath) + "\n"
        output += "Labels: " + str(self.__labelsList) + "\n"
        output += "Toolkit: " + str(self.__toolkit) + "\n"
        output += "Registration Type: " + str(self.__registrationType) + "\n"
        output += "Save Transforms: " + str(self.__saveTransforms) + "\n"
        output += "Delete Aligned Images: " + str(self.__deleteAlignedImages) + "\n"
        output += "Delete Aligned Segmentations: " + str(self.__deleteAlignedSegmentations) + "\n"
        output += "Normalize Atlases: " + str(self.__normalizeAtlases) + "\n"
        output += "Normalize To: " + str(self.__normalizeTo) + "\n"
        output += "Output Cast: " + str(self.__outputCast) + "\n"
        
        return output
    
    
    
    '''=========================================================================================='''
    def GetOriginalImagesFilePathList(self):
        '''
            Returns
                the originalImagesFilePathList
        '''
        return self.__originalImagesFilePathList



    '''=========================================================================================='''
    def SetOriginalImagesFilePathList(self,value):
        '''
            Sets the list of filePaths for the original Images
            
            value
                a list of at least 1 filePaths pointing to existing original images
                
            Returns
                n/a
        '''
        if type(value).__name__=='list' and len(value) >= 1:
            # we have at least one filePath in the list
            self.__originalImagesFilePathList = value



    '''=========================================================================================='''
    def GetSegmentationsFilePathList(self):
        '''
            Returns
                the originalImagesFilePathList
        '''
        return self.__segmentationsFilePathList



    '''=========================================================================================='''
    def SetSegmentationsFilePathList(self,value):
        '''
            Sets the list of filePaths for the segmentations
            
            value
                a list of at least 1 filePaths pointing to existing segmentations
                
            Returns
                n/a
        '''
        if type(value).__name__=='list' and len(value) >= 1:
            # we have at least one filePath in the list
            self.__segmentationsFilePathList = value



    '''=========================================================================================='''
    def GetOutputDirectory(self):
        '''
            Returns
                the output directory
        '''
        return self.__outputDirectory



    '''=========================================================================================='''
    def SetOutputDirectory(self, value):
        '''
            Sets the output directory
            
            value
                a String holding a path to a directory with a required trailing slash
                
            Returns
                n/a
        '''
        if value:
        
            self.__outputDirectory = value

            
                
    '''=========================================================================================='''
    def GetTemplateType(self):
        '''
            Returns
                the template Type (fixed or dynamic)
        '''
        return self.__templateType



    '''=========================================================================================='''
    def SetTemplateType(self, value):
        '''
            Sets the template type
            
            value
                a String defining the template type, either "fixed" or "dynamic"
                
            Returns
                n/a
        '''
        if (value == "fixed" or value == "dynamic"):
        
            self.__templateType = str(value)

        else:
            # use fixed as default if argument is illegal
            self.__templateType = "fixed"
            
            
    
    '''=========================================================================================='''
    def GetDynamicTemplateIterations(self):
        '''
            Returns
                number of iterations for dynamic template creation
        '''
        return int(self.__dynamicTemplateIterations)



    '''=========================================================================================='''
    def SetDynamicTemplateIterations(self, value):
        '''
            Sets the number of iterations for dynamic template creation
            
            value
                number of iterations
                
            Returns
                n/a
        '''
        if (value <= 0):
            self.__dynamicTemplateIterations = 5
        else:
            self.__dynamicTemplateIterations = int(value)
        
        
        
    '''=========================================================================================='''
    def GetFixedTemplateDefaultCaseFilePath(self):
        '''
            Returns
                filePath to the default Case
        '''
        return self.__fixedTemplateDefaultCaseFilePath



    '''=========================================================================================='''
    def SetFixedTemplateDefaultCaseFilePath(self, value):
        '''
            Sets the filePath to the default Case
            
            value
                filePath to the default Case
                
            Returns
                n/a
        '''
        if not value:
            self.__fixedTemplateDefaultCaseFilePath = ""
        else:
            self.__fixedTemplateDefaultCaseFilePath = str(value)
        
        
    
    '''=========================================================================================='''
    def GetLabelsList(self):
        '''
            Returns
                the labels as a list
        '''
        return self.__labelsList



    '''=========================================================================================='''
    def SetLabelsList(self,value):
        '''
            Sets the list of labels used for Atlas Creation
            
            value
                a list of at least 1 label as integer
                
            Returns
                n/a
        '''
        if type(value).__name__=='list' and len(value) >= 1:
            # we have at least one label in the list
            self.__labelsList = value
            
            
            
    '''=========================================================================================='''
    def GetToolkit(self):
        '''
            Returns
                the toolkit to perform registration and re-sampling
        '''
        return self.__toolkit



    '''=========================================================================================='''
    def SetToolkit(self, value):
        '''
            Sets the toolkit to perform registration and re-sampling
            
            value
                the toolkit as String. Supported values are
                "BRAINSFit", "CMTK"
                
            Returns
                n/a
        '''
        if value == "BRAINSFit" or value == "CMTK":
            self.__toolkit = str(value)
        else:
            self.__toolkit = "BRAINSFit"            
            
            
                            
    '''=========================================================================================='''
    def GetRegistrationType(self):
        '''
            Returns
                the registration Type
        '''
        return self.__registrationType



    '''=========================================================================================='''
    def SetRegistrationType(self, value):
        '''
            Sets the registration Type
            
            value
                the registration Type as String. Supported values are
                "Affine", "Non-Rigid"
                
            Returns
                n/a
        '''
        if value == "Affine" or value == "Non-Rigid":
            self.__registrationType = str(value)
        else:
            self.__registrationType = "Affine"
        
        
                
    '''=========================================================================================='''
    def GetSaveTransforms(self):
        '''
            Returns
                the flag for saving Transforms
        '''
        return self.__saveTransforms



    '''=========================================================================================='''
    def SetSaveTransforms(self, value):
        '''
            Sets the flag for saving Transforms
            
            value
                the flag as 0 or 1
                
            Returns
                n/a
        '''
        if value != 0 and value != 1:
            self.__saveTransforms = 1
        else:
            self.__saveTransforms = int(value)
        
        
                
    '''=========================================================================================='''
    def GetNormalizeAtlases(self):
        '''
            Returns
                the flag for normalizing atlases
        '''
        return self.__normalizeAtlases



    '''=========================================================================================='''
    def SetNormalizeAtlases(self, value):
        '''
            Sets the flag for normalizing atlases
            
            value
                the flag as 0 or 1
                
            Returns
                n/a
        '''
        if value != 0 and value != 1:
            self.__normalizeAtlases = 0
        else:
            self.__normalizeAtlases = int(value)



    '''=========================================================================================='''
    def GetNormalizeTo(self):
        '''
            Returns
                the flag for normalizing atlases
        '''
        return self.__normalizeTo



    '''=========================================================================================='''
    def SetNormalizeTo(self, value):
        '''
            Sets normalizeTo value
            
            value
                the new value as an integer
                
            Returns
                n/a
        '''
        self.__normalizeTo = int(value)
        
        
                
    '''=========================================================================================='''
    def GetDeleteAlignedImages(self):
        '''
            Returns
                the flag for deleting the aligned images
        '''
        return self.__deleteAlignedImages



    '''=========================================================================================='''
    def SetDeleteAlignedImages(self, value):
        '''
            Sets the flag for deleting the aligned images
            
            value
                the flag as 0 or 1
                
            Returns
                n/a
        '''
        if value != 0 and value != 1:
            self.__deleteAlignedImages = 1
        else:
            self.__deleteAlignedImages = int(value)    
    
    
    
        '''=========================================================================================='''
    def GetDeleteAlignedSegmentations(self):
        '''
            Returns
                the flag for deleting the aligned segmentations
        '''
        return self.__deleteAlignedSegmentations



    '''=========================================================================================='''
    def SetDeleteAlignedSegmentations(self, value):
        '''
            Sets the flag for deleting the aligned segmentations
            
            value
                the flag as 0 or 1
                
            Returns
                n/a
        '''
        if value != 0 and value != 1:
            self.__deleteAlignedSegmentations = 1
        else:
            self.__deleteAlignedSegmentations = int(value)    
    
    
    
    '''=========================================================================================='''
    def GetOutputCast(self):
        '''
            Returns
                the output cast
        '''
        return self.__outputCast



    '''=========================================================================================='''
    def SetOutputCast(self, value):
        '''
            Sets the output cast
            
            value
                a String defining the output cast, possible values are
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
                n/a
        '''
        if (value == "Char" or value == "Unsigned Char"
                            or value == "Double"
                            or value == "Float"
                            or value == "Int"
                            or value == "Unsigned Int"
                            or value == "Long"
                            or value == "Unsigned Long"
                            or value == "Short"
                            or value == "Unsigned Short"):
            self.__outputCast = str(value)

        else:
            # use Short as default if argument is illegal
            self.__outputCast = "Short"

