/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

 Module:    $RCSfile: vtkMRMLAnnotationSliceViewDisplayableManager.h.h,v $
 Date:      $Date: 2010/07/26 04:48:05 $
 Version:   $Revision: 1.1 $

 =========================================================================auto=*/

#ifndef __vtkMRMLAnnotationSliceViewDisplayableManager_h
#define __vtkMRMLAnnotationSliceViewDisplayableManager_h

// AnnotationModule includes
#include "qSlicerAnnotationModuleExport.h"
#include "vtkMRMLAnnotationClickCounter.h"

// MRMLDisplayableManager includes
#include <vtkMRMLAbstractSliceViewDisplayableManager.h>
#include "MRMLDisplayableManager/vtkMRMLAnnotationDisplayableManagerHelper.h"

class vtkMRMLAnnotationNode;
class vtkSlicerViewerWidget;
class vtkMRMLAnnotationDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkAbstractWidget;

class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationSliceViewDisplayableManager :
    public vtkMRMLAbstractSliceViewDisplayableManager
{
public:

  static vtkMRMLAnnotationSliceViewDisplayableManager *New();
  vtkTypeRevisionMacro(vtkMRMLAnnotationSliceViewDisplayableManager, vtkMRMLAbstractSliceViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData);

  // the following functions must be public to be accessible by the callback
  /// Propagate properties of MRML node to widget.
  virtual void PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget);
  /// Propagate properties of widget to MRML node.
  virtual void PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node);

protected:

  vtkMRMLAnnotationSliceViewDisplayableManager();
  virtual ~vtkMRMLAnnotationSliceViewDisplayableManager();


  virtual void Create();

  /// Called after the corresponding MRML event is triggered, from AbstractDisplayableManager
  /// \sa ProcessMRMLEvents
  virtual void OnMRMLSceneAboutToBeClosedEvent();
  virtual void OnMRMLSceneClosedEvent();
  virtual void OnMRMLSceneAboutToBeImportedEvent();
  virtual void OnMRMLSceneImportedEvent();
  virtual void OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemovedEvent(vtkMRMLNode* node);

  /// Observe all associated nodes.
  void SetAndObserveNodes();

  /// Preset functions for certain events.
  void OnMRMLAnnotationNodeModifiedEvent(vtkMRMLNode* node);
  void OnMRMLAnnotationNodeTransformModifiedEvent(vtkMRMLNode* node);
  void OnMRMLAnnotationNodeLockModifiedEvent(vtkMRMLNode* node);

  /// Get the widget of a node.
  vtkAbstractWidget * GetWidget(vtkMRMLAnnotationNode * node);

  // Get the coordinates of a click in the RenderWindow
  void OnClickInSliceViewGetCoordinates();
  /// Callback for click in RenderWindow
  virtual void OnClickInSliceViewWindow(double x, double y);
  /// Counter for clicks in 3D Render Window
  vtkMRMLAnnotationClickCounter* m_ClickCounter;

  /// Place a seed for widgets
  virtual void PlaceSeed(double x, double y);
  /// Return the placed seeds
  vtkHandleWidget * GetSeed(int index);

  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLAnnotationNode* node);
  /// Gets called when widget was created
  virtual void OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node);

  /// Check if it is the right displayManager
  virtual bool IsCorrectDisplayableManager();

  /// Convert display to world coordinates
  virtual double * GetDisplayToWorldCoordinates(double x, double y);

  /// Convert display to world coordinates
  virtual double * GetWorldToDisplayCoordinates(double r, double a, double s);
  virtual double * GetWorldToDisplayCoordinates(double * worldPoints);

  /// Focus of this displayableManager
  const char* m_Focus;

  /// Disable processing when updating is in progress.
  int m_Updating;

  virtual void OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller);

  /// Could be overloaded if DisplayableManager subclass
  virtual void OnMRMLSliceNodeModifiedEvent();

private:

  vtkMRMLAnnotationSliceViewDisplayableManager(const vtkMRMLAnnotationSliceViewDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLAnnotationSliceViewDisplayableManager&); /// Not Implemented

  virtual void OnInteractorStyleEvent(int eventid);

  vtkMRMLAnnotationDisplayableManagerHelper * Helper;

  int m_DisableInteractorStyleEventsProcessing;

};

#endif

