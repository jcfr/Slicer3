from Slicer import slicer
from AtlasCreatorConfiguration import AtlasCreatorConfiguration

class AtlasCreatorGridConfiguration(AtlasCreatorConfiguration):
    '''
        This class reflects a parameter configuration for parallelized computing using the Atlas Creator
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
        
        self.__schedulerCommand = ""
        
        
        
    '''=========================================================================================='''
    def GetSchedulerCommand(self):
        '''
            Returns
                the scheduler command
        '''        
        return self.__schedulerCommand
    
    
    
    '''=========================================================================================='''
    def SetSchedulerCommand(self,value):
        '''
            Sets the scheduler command for the cluster
            
            value
                the scheduler command as String
                
            Returns
                n/a
        '''
        if value:
            self.__schedulerCommand = str(value) 
            
            
            
            
