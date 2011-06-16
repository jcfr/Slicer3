from __main__ import qt, ctk
import PythonQt

from EMSegmentStep import *

class EMSegmentDefineAnatomicalTreeStep( EMSegmentStep ) :

  def __init__( self, stepid ):
    self.initialize( stepid )
    self.setName( '3. Define Anatomical Tree' )
    self.setDescription( 'Define a hierarchy of structures.' )

    self.__parent = super( EMSegmentDefineAnatomicalTreeStep, self )

  def createUserInterface( self ):
    '''
    '''
    self.__layout = self.__parent.createUserInterface()

    # the anatomical tree
    anatomicalTreeGroupBox = qt.QGroupBox()
    anatomicalTreeGroupBox.setTitle( 'Anatomical Tree' )
    self.__layout.addWidget( anatomicalTreeGroupBox )

    anatomicalTreeGroupBoxLayout = qt.QFormLayout( anatomicalTreeGroupBox )

    self.__anatomicalTree = PythonQt.qSlicerEMSegmentModuleWidgets.qSlicerEMSegmentAnatomicalTreeWidget()
    self.__anatomicalTree.structureNameEditable = True
    self.__anatomicalTree.labelColumnVisible = True
    anatomicalTreeGroupBoxLayout.addWidget( self.__anatomicalTree )


  def onEntry( self, comingFrom, transitionType ):
    '''
    '''
    self.__parent.onEntry( comingFrom, transitionType )

    self.__anatomicalTree.setMRMLManager( self.mrmlManager() )
    self.__anatomicalTree.updateWidgetFromMRML()


  def validate( self, desiredBranchId ):
    '''
    '''
    self.__parent.validate( desiredBranchId )

    self.__parent.validationSucceeded( desiredBranchId )
