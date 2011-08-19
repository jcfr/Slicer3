if {[catch {package require Itcl}]} { return }
package ifneeded Editor 3.0 [list 
  source [file join $dir Editor.tcl]
  source [file join $dir EditorNode.tcl]
  source [file join $dir EditorLogic.tcl]
  source [file join $dir EditorGUI.tcl]
  source [file join $dir Box.tcl]
  source [file join $dir EditBox.tcl]
  source [file join $dir EditColor.tcl]
  source [file join $dir HelperBox.tcl]
  source [file join $dir ColorBox.tcl]
  source [file join $dir EffectSWidget.tcl]
  source [file join $dir Labeler.tcl]
  source [file join $dir PaintEffect.tcl]
  source [file join $dir DrawEffect.tcl]
  source [file join $dir SaveIslandEffect.tcl]
  source [file join $dir ChangeIslandEffect.tcl]
  source [file join $dir ChangeLabelEffect.tcl]
  source [file join $dir ThresholdEffect.tcl]
  source [file join $dir RemoveIslandsEffect.tcl]
  source [file join $dir LevelTracingEffect.tcl]
  source [file join $dir WandEffect.tcl]
  source [file join $dir ImplicitRectangleEffect.tcl]
  source [file join $dir MakeModelEffect.tcl]
  source [file join $dir ErodeLabelEffect.tcl]
  source [file join $dir DilateLabelEffect.tcl]
  source [file join $dir IdentifyIslandsEffect.tcl]
  source [file join $dir GrowCutSegmentEffect.tcl] 
  source [file join $dir ModelDrawEffect.tcl] 
]
