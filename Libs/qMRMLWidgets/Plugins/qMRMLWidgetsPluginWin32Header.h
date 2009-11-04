/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: qMRMLWidgetsPluginConfigure.h,v $
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
// .NAME qMRMLWidgetsPluginWin32 - manage Windows system differences
// .SECTION Description
// The qMRMLWidgetsPluginWin32 captures some system differences between Unix
// and Windows operating systems. 

#ifndef __qMRMLWidgetsPluginWin32Header_h
#define __qMRMLWidgetsPluginWin32Header_h

#include <qMRMLWidgetsConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
 #if defined(qMRMLWidgetsPlugin_EXPORTS)
  #define QMRML_WIDGETS_PLUGIN_EXPORT __declspec( dllexport ) 
 #else
  #define QMRML_WIDGETS_PLUGIN_EXPORT __declspec( dllimport ) 
 #endif
#else
 #define QMRML_WIDGETS_PLUGIN_EXPORT
#endif

#endif