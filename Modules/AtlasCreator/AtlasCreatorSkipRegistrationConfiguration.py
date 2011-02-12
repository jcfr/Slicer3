from Slicer import slicer
from AtlasCreatorConfiguration import AtlasCreatorConfiguration

class AtlasCreatorSkipRegistrationConfiguration(AtlasCreatorConfiguration):
    '''
        This class reflects a parameter configuration for using the Atlas Creator
        with existing Transforms
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
        self.__transformsDirectory = ""
        
        self.__existingTemplate = ""
        
        
        
    '''=========================================================================================='''
    def GetConfigurationAsString(self):
        '''
            Returns
                the current configuration as a String
        '''
        output = AtlasCreatorConfiguration.GetConfigurationAsString(self)
        output += "Existing Transforms Directory: " + str(self.__transformsDirectory) + "\n"
        
        return output
    
    
        
    '''=========================================================================================='''
    def GetTransformDirectory(self):
        '''
            Returns
                a list of filePaths pointing to Transforms
        '''
        return self.__transformsDirectory



    '''=========================================================================================='''
    def SetTransformDirectory(self,value):
        '''
            Sets the directory for the existing transforms
            
            value
                a filePath to a directory containing existing transforms
                
            Returns
                n/a
        '''
        if value:
            self.__transformsDirectory = value



    '''=========================================================================================='''
    def GetExistingTemplate(self):
        '''
            Returns
                the filePath to an existing template
        '''
        return self.__existingTemplate



    '''=========================================================================================='''
    def SetExistingTemplate(self,value):
        '''
            Sets the filePath to an existing template
            
            value
                a filePath to an existing template
                
            Returns
                n/a
        '''
        if value:
            self.__existingTemplate = value


