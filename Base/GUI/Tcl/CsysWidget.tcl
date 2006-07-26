
if { [info command vtkNew] == "" } {
  proc vtkNew {class {group "default"}} {
    if { ![info exists ::vtkObjects($group)] } {set ::vtkObjects($group) ""}
    set o [$class New]
    set ::vtkObjects($group) "$o $::vtkObjects($group)"
    return $o
  }
}

if { [info command vtkDelete] == "" } {
  proc vtkDelete {{group "default"}} {
    if { ![info exists ::vtkObjects($group)] } {return}
    foreach o $::vtkObjects($group) {
      puts "$o ([$o GetClassName]) refcount [$o GetReferenceCount]"
      $o Delete
    }
    set ::vtkObjects($group) ""
  }
}

if { [info command vtkExists] == "" } {
  proc vtkExists { o {group "default"}} {
    if { ![info exists ::vtkObjects($group)] } {return 0}
    if { [lsearch $::vtkObjects($group) $o] == -1 } {
      return 0
    } else {
      return 1
    }
  }
}

proc CsysWidgetRemove {} {

  set sliceGUIs [vtkSlicerSliceGUI ListInstances]
  foreach sliceGUI $sliceGUIs {
    CsysWidgetDelete $sliceGUI 
  }
}

proc CsysWidgetAdd {} {

  CsysWidgetRemove

  set sliceGUIs [vtkSlicerSliceGUI ListInstances]
  foreach sliceGUI $sliceGUIs {
    CsysWidgetCreate $sliceGUI
  }
}

proc CsysWidgetCreate { sliceGUI } {

  set renderWidget [[$sliceGUI GetSliceViewer] GetRenderWidget]
 
  set o(sphere) [vtkNew vtkSphereSource $sliceGUI]
  set o(mapper) [vtkNew vtkPolyDataMapper2D $sliceGUI]
  set o(actor) [vtkNew vtkActor2D $sliceGUI]
  $o(mapper) SetInput [$o(sphere) GetOutput]
  $o(actor) SetMapper $o(mapper)
  [$renderWidget GetRenderer] AddActor2D $o(actor)

  foreach l {1 2} {
    set o(leader,$l) [vtkNew vtkLeaderActor2D $sliceGUI]
    $o(leader,$l) SetArrowPlacementToBoth
    $o(leader,$l) SetMaximumArrowSize 7
    set o(leaderMapper,$l) [vtkNew vtkPolyDataMapper2D $sliceGUI]
    $o(leader,$l) SetMapper $o(leaderMapper,$l)
    [$renderWidget GetRenderer] AddActor2D $o(leader,$l)
  }
  
  CsysWidgetUpdate $sliceGUI [array get o]
  $sliceGUI AddObserver AnyEvent "CsysWidgetUpdate $sliceGUI \"[array get o]\""
}

proc CsysWidgetUpdate { sliceGUI objs } {
  array set o $objs

  set renderWidget [[$sliceGUI GetSliceViewer] GetRenderWidget]
  set interactor [$renderWidget GetRenderWindowInteractor]
  set size [[$renderWidget GetRenderWindow]  GetSize]
  foreach {w h} $size {}
  foreach d {w h} c {cx cy} { set $c [expr [set $d] / 2.0] }

  foreach {ex ey} [$interactor GetEventPosition] {}
  if { [expr abs($ex - $cx > 15)] &&  [expr abs($ex - $cx > 15)] } {
    [$sliceGUI GetGUICallbackCommand] SetAbortFlag 1
  }

  $o(sphere) SetRadius 5
  $o(sphere) SetCenter $cx $cy 0

  $o(leader,1) SetPosition  [expr ($cx - 15) / $w] [expr $cy / $h]
  $o(leader,1) SetPosition2 [expr ($cx + 15) / $w] [expr $cy / $h]
  $o(leader,2) SetPosition  [expr $cx / $w] [expr ($cy - 15) / $h]
  $o(leader,2) SetPosition2 [expr $cx / $w] [expr ($cy + 15) / $h]

  [$sliceGUI GetSliceViewer] RequestRender
}

proc CsysWidgetDelete { sliceGUI } {

  set renderWidget [[$sliceGUI GetSliceViewer] GetRenderWidget]
  set actors [[$renderWidget GetRenderer] GetActors2D]
  $actors InitTraversal
  while { 1 } {
    set actor [$actors GetNextActor2D]
    if { $actor == "" } { break }
    if { [vtkExists $actor $sliceGUI] } {
      [$renderWidget GetRenderer] RemoveActor2D $actor
    }
  }
  vtkDelete $sliceGUI
}
