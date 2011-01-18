#!/bin/sh
# the next line restarts using tclsh \
    exec tclsh "$0" "$@"



# little utility script for deleting old copies of slicer extension zip files
# NOTE: this runs on unix/posix only (requires find command)
#
proc Usage {} {
  puts "cleanextdir \[options\] <dir>"
  puts "-h, --help (this message)"
  puts "-v, --verbose print updates while processing"
  puts "<dir> is extension dir created by Slicer's extend.tcl script"
  puts ""
  puts "This script deletes all be the latest version of an extension build."
  puts "The <dir> is searched recursively for extenisons."
}

set ::DIR ""
set ::VERBOSE 0

set strippedargs ""
set argc [llength $argv]
for {set i 0} {$i < $argc} {incr i} {
    set a [lindex $argv $i]
    switch -glob -- $a {
        "-v" -
        "--verbose" {
            set ::VERBOSE 1
        }
        "--help" -
        "-h" {
            Usage
            exit 1
        }
        "-*" {
            Usage "unknown option $a\n"
            exit 1
        }
        default {
            lappend strippedargs $a
        }
    }
}

set argv $strippedargs
set argc [llength $argv]

if {$argc != 1 } {
  Usage
  exit 1
}

set ::DIR $strippedargs

set dirs [exec find $::DIR -type d]

foreach dir $dirs {
  if { $::VERBOSE } {
    puts "checking $dir"
  }
  set files [glob -nocomplain $dir/*.zip]
  foreach f $files {
    if { ![file exists $f] } {
      # file was deleted in earlier iteration
      continue
    }

    # extract the date, finds all matching files in dir
    set namelist [split [file tail $f] "-."]
    if { [llength $namelist] < 6 } {
      # file was not a slicer extension file, skip it
      continue
    }
    set module [lindex $namelist 0]
    set svnno [lindex $namelist 1]
    set year [lindex $namelist 2]
    set month [lindex $namelist 3]
    set day [lindex $namelist 4]
    set os [lindex $namelist 5]
    if { $os == "win32" } {
      set osarch "win32"
    } else {
      set osarch "$os-[lindex $namelist 6]"
    }


    set matchfiles [lsort [glob $dir/$module-$svnno-*-$osarch*]]

    if { [llength $matchfiles] > 1 } {
      foreach ff [lrange $matchfiles 0 end-1] {
        if { $::VERBOSE } {
          puts "deleting $ff"
        }
        file delete $ff
      }
    }
    if { $::VERBOSE } {
      puts "keeping [lindex $matchfiles end]"
    }
  }
}
