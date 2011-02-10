from Slicer import slicer
from AtlasCreatorConfiguration import AtlasCreatorConfiguration

class AtlasCreatorSkipRegistrationConfiguration(AtlasCreatorConfiguration):
    '''
        This class reflects a parameter configuration for using the Atlas Creator
        with existing Deformation Fields
    '''
    
    
    
    '''=========================================================================================='''
    def __init__(self):
        '''
            Initialize this class
            
            Returns
                n/a
        '''
        
        # access the superClass
        AtlasCreatorConfiguration.__init__(self)
        
        # also, initialize custom fields now
        self.__deformationFieldsFilePathList = []
        
        
        
    '''=========================================================================================='''
    def GetDeformationFieldsFilePathList(self):
        '''
            Returns
                a list of filePaths pointing to Deformation Fields
        '''
        return self.__originalImagesFilePathList



    '''=========================================================================================='''
    def SetDeformationFieldsFilePathList(self,value):
        '''
            Sets the list of filePaths for the Deformations Fields
            
            value
                a list of at least 1 filePaths pointing to existing original images
                
            Returns
                n/a
        '''
        if type(value).__name__=='list' and len(value) >= 1:
            # we have at least one filePath in the list
            self.__originalImagesFilePathList = value


