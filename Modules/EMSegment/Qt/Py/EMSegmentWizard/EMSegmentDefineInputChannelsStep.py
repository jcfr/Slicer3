from __main__ import qt, ctk
import PythonQt

from EMSegmentStep import *
from Helper import *

class EMSegmentDefineInputChannelsStep( EMSegmentStep ) :

  def __init__( self, stepid ):
    self.initialize( stepid )
    self.setName( '2. Define Input Channels' )
    self.setDescription( 'Name the input channels and choose the set of scans for segmentation.' )

    self.__parent = super( EMSegmentDefineInputChannelsStep, self )


  def createUserInterface( self ):
    '''
    '''
    self.__layout = self.__parent.createUserInterface()

    # the input channels
    inputChannelGroupBox = qt.QGroupBox()
    inputChannelGroupBox.setTitle( 'Input Channels' )
    self.__layout.addWidget( inputChannelGroupBox )

    inputChannelGroupBoxLayout = qt.QFormLayout( inputChannelGroupBox )

    self.__inputChannelList = PythonQt.qSlicerEMSegmentModuleWidgets.qSlicerEMSegmentInputChannelListWidget()
    inputChannelGroupBoxLayout.addWidget( self.__inputChannelList )

    # add empty row
    self.__layout.addRow( "", qt.QWidget() )

    # registration settings
    input2inputChannelRegistration = qt.QGroupBox()
    input2inputChannelRegistration.setTitle( 'Input-to-Input Channel Registration' )
    self.__layout.addWidget( input2inputChannelRegistration )

    input2inputChannelRegistrationLayout = qt.QFormLayout( input2inputChannelRegistration )

    self.__alignInputScansCheckBox = qt.QCheckBox()
    input2inputChannelRegistrationLayout.addRow( 'Align input scans:', self.__alignInputScansCheckBox )
    self.__alignInputScansCheckBox.connect( "stateChanged(int)", self.onAlignInputScansChanged )

  def onAlignInputScansChanged( self ):
    '''
    Gets called whenever the 'Align input scans' checkbox is toggled
    '''
    if self.__alignInputScansCheckBox.isChecked():
      self.mrmlManager().SetEnableTargetToTargetRegistration( 1 )
    else:
      self.mrmlManager().SetEnableTargetToTargetRegistration( 0 )

  def onEntry( self, comingFrom, transitionType ):
    '''
    '''
    self.__parent.onEntry( comingFrom, transitionType )

    self.__inputChannelList.setMRMLManager( self.mrmlManager() )
    self.__inputChannelList.updateWidgetFromMRML()

    if self.__inputChannelList.inputChannelCount() == 0:
      self.__inputChannelList.addInputChannel()

  def validate( self, desiredBranchId ):
    '''
    '''
    self.__parent.validate( desiredBranchId )

    # we need at least one input channel
    if self.__inputChannelList.inputChannelCount() == 0:
      self.__parent.validationFailed( desiredBranchId, 'Input Channel Error', 'Please add at least one input channel!' )

    # we need an assigned volume for each channel
    for c in range( self.__inputChannelList.inputChannelCount() ):
      if not self.__inputChannelList.inputChannelVolume( c ):
        self.__parent.validationFailed( desiredBranchId, 'Input Channel Error', 'Please assign a volume to each input channel!' )

    # TODO number of input channels changed
    # this->GetNumberOfInputChannels() != mrmlManager->GetGlobalParametersNode()->GetNumberOfTargetInputChannels()

    # TODO check if all channels have different volumes assigned

    # TODO check on bad names for the channels



    self.__parent.validationSucceeded( desiredBranchId )


