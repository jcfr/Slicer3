from __main__ import qt, ctk
import PythonQt

from EMSegmentStep import *
from Helper import *

class EMSegmentEditRegistrationParametersStep( EMSegmentStep ) :

  def __init__( self, stepid ):
    self.initialize( stepid )
    self.setName( '5. Edit Registration Parameters' )
    self.setDescription( 'Specify atlas-to-input scans registration parameters.' )

    self.__parent = super( EMSegmentEditRegistrationParametersStep, self )

  def createUserInterface( self ):
    '''
    '''
    self.__layout = self.__parent.createUserInterface()

    # the registration parameters
    registrationParametersGroupBox = qt.QGroupBox()
    registrationParametersGroupBox.setTitle( 'Atlas-to-Input Registration Parameters' )
    self.__layout.addWidget( registrationParametersGroupBox )

    registrationParametersGroupBoxLayout = qt.QFormLayout( registrationParametersGroupBox )

    # TODO for all input channels add a qMRMLNodeComboBox

    # Affine Registration comboBox
    affineRegistrationComboBox = qt.QComboBox()
    affineRegistrationComboBox.addItems( Helper.GetRegistrationTypes() )
    registrationParametersGroupBoxLayout.addRow( 'Affine Registration:', affineRegistrationComboBox )

    # Deformable Registration comboBox
    deformableRegistrationComboBox = qt.QComboBox()
    deformableRegistrationComboBox.addItems( Helper.GetRegistrationTypes() )
    registrationParametersGroupBoxLayout.addRow( 'Deformable Registration:', deformableRegistrationComboBox )

    # Interpolation
    interpolationComboBox = qt.QComboBox()
    interpolationComboBox.addItems( Helper.GetInterpolationTypes() )
    registrationParametersGroupBoxLayout.addRow( 'Interpolation:', interpolationComboBox )

    # Package
    packageComboBox = qt.QComboBox()
    packageComboBox.addItems( Helper.GetPackages() )
    registrationParametersGroupBoxLayout.addRow( 'Package:', packageComboBox )


  def onEntry( self, comingFrom, transitionType ):
    '''
    '''
    self.__parent.onEntry( comingFrom, transitionType )




  def validate( self, desiredBranchId ):
    '''
    '''
    self.__parent.validate( desiredBranchId )

    self.__parent.validationSucceeded( desiredBranchId )
