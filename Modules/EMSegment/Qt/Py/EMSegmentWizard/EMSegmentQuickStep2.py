from __main__ import qt, ctk, slicer
import PythonQt

from EMSegmentStep import *

class EMSegmentQuickStep2( EMSegmentStep ) :

  def __init__( self, stepid ):
    self.initialize( stepid )
    self.setName( '2. Define Structures' )
    self.setDescription( 'Define a hierarchy of structures to be segmented.' )

    self.__parent = super( EMSegmentQuickStep2, self )
    self.__layout = None
    self.__colorTableComboBox = None
    self.__anatomicalTree = None

    self.__numberOfStructures = 0

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
    self.__anatomicalTree.addDeleteSubclassesEnabled = True
    self.__anatomicalTree.toolTip = 'Please configure a hierarchy of structures for the input datasets.'
    self.__anatomicalTree.setSizePolicy( qt.QSizePolicy.MinimumExpanding, qt.QSizePolicy.MinimumExpanding )
    anatomicalTreeGroupBoxLayout.addWidget( self.__anatomicalTree )

  def onEntry( self, comingFrom, transitionType ):
    '''
    '''
    self.__parent.onEntry( comingFrom, transitionType )

    if self.__numberOfStructures == 0:

      self.__numberOfStructures = qt.QInputDialog.getInt( self, 'Number of Structures', 'Please specify how many different structures are in the input image(s).', 3, 1 )

      # no harm for division by zero since the input dialog prevents zero
      probability = 1.0 / self.__numberOfStructures

      for i in range( self.__numberOfStructures ):

        currentId = self.mrmlManager().AddTreeNode( self.mrmlManager().GetTreeRootNodeID() )
        if i == 0:
          self.mrmlManager().SetTreeNodeName( currentId, 'Background' )
          self.mrmlManager().SetTreeNodeIntensityLabel( currentId, 0 )
        elif i > 0:
          self.mrmlManager().SetTreeNodeName( currentId, 'Tissue' + str( i ) )
          self.mrmlManager().SetTreeNodeIntensityLabel( currentId, i )

        self.mrmlManager().SetTreeNodeClassProbability( currentId, probability )
        self.mrmlManager().SetTreeNodeSpatialPriorWeight( currentId, 0.0 )

    if self.__anatomicalTree:
      self.__anatomicalTree.setMRMLManager( self.mrmlManager() )
      self.__anatomicalTree.updateWidgetFromMRML()


  def validate( self, desiredBranchId ):
    '''
    '''
    self.__parent.validate( desiredBranchId )

    self.__parent.validationSucceeded( desiredBranchId )
