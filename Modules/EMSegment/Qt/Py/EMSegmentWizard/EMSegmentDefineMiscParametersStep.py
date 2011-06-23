from __main__ import qt, ctk
import PythonQt

from EMSegmentStep import *
from Helper import *

class EMSegmentDefineMiscParametersStep( EMSegmentStep ) :

  def __init__( self, stepid ):
    self.initialize( stepid )
    self.setName( '9. Define Miscellaneous Parameters' )
    self.setDescription( 'Define miscellaneous parameters for performing segmentation' )

    self.__parent = super( EMSegmentDefineMiscParametersStep, self )

  def createUserInterface( self ):
    '''
    '''
    self.__layout = self.__parent.createUserInterface()

    # the registration parameters
    voiGroupBox = qt.QGroupBox()
    voiGroupBox.setTitle( 'Define VOI' )
    self.__layout.addWidget( voiGroupBox )

    voiGroupBoxLayout = qt.QFormLayout( voiGroupBox )

    roiWidget = slicer.qMRMLROIWidget()
    voiGroupBoxLayout.addWidget( roiWidget )


  def onEntry( self, comingFrom, transitionType ):
    '''
    '''
    self.__parent.onEntry( comingFrom, transitionType )




  def validate( self, desiredBranchId ):
    '''
    '''
    self.__parent.validate( desiredBranchId )

    self.__parent.validationSucceeded( desiredBranchId )
