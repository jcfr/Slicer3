/*=auto=========================================================================

  Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLColorNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.0 $

=========================================================================auto=*/
// .NAME vtkMRMLColorNode - MRML node to represent color information.
// .SECTION Description
// Color nodes describe colour look up tables. The tables may be pre-generated by
// Slicer (the label map colours, some default ramps, a random one) or created by
// a user. More than one model or label volume or editor can access the prebuilt
// nodes.

#ifndef __vtkMRMLColorNode_h
#define __vtkMRMLColorNode_h

#include <string>
#include <vector>

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkLookupTable.h"

class VTK_MRML_EXPORT vtkMRMLColorNode : public vtkMRMLNode
{
public:
  static vtkMRMLColorNode *New();
  vtkTypeMacro(vtkMRMLColorNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);


  // Description:
  // Read in a text file holding colour table elements
  // id name r g b a
  // comments start with a hash mark
  virtual void ReadFile ();
  
  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);
  
  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "Color";};

  // Description:
  // 
  virtual void UpdateScene(vtkMRMLScene *scene);

  vtkGetObjectMacro(LookupTable, vtkLookupTable);
  vtkSetObjectMacro(LookupTable, vtkLookupTable);

  // Description:
  // Get/Set for Type
  void SetType(int type);
  vtkGetMacro(Type,int);
  void SetTypeToGrey();
  void SetTypeToIron();
  void SetTypeToRainbow();
  void SetTypeToOcean();
  void SetTypeToDesert();
  void SetTypeToInvGrey();
  void SetTypeToReverseRainbow();
  void SetTypeToFMRI();
  void SetTypeToFMRIPA();
  void SetTypeToLabels();
  void SetTypeToRandom();
  void SetTypeToFile();

  void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  //BTX
  // Description:
  // The list of valid table types
 
  // Grey - greyscale ramp
  // Iron - neutral
  // Rainbow - red-orange-yellow-blue-indigo-violet
  // Ocean - bluish ramp
  // Desert - orange ramp
  // InvGrey - inverted greyscale ramp
  // ReverseRainbow - inverted Rainbow
  // FMRI - fMRI map
  // FMRIPA - fMRI Positive Activation map
  // Labels - the Slicer default editor labels
  // Random - 255 random colors
  // File - read in from file
  enum
    {
      Grey = 1,
      Iron = 2,
      Rainbow = 3,
      Ocean = 4,
      Desert = 5,
      InvGrey = 6,
      ReverseRainbow = 7,
      FMRI = 8,
      FMRIPA = 9,
      Labels = 10,
      Random = 11,
      File = 12,
    };
  //ETX

  // Description:
  // return a text string describing the colour look up table type
  const char * GetTypeAsString();

  //BTX
  // Description:
  // DisplayModifiedEvent is generated when display node parameters is changed
  // PolyDataModifiedEvent is generated when something else is changed
  // TypeModifiedEvent is generated when the type of the colour look up table changes
  enum
    {
      DisplayModifiedEvent = 20000,
      PolyDataModifiedEvent = 20001,
      TypeModifiedEvent = 20002,
    };
//ETX

  // Description:
  // Get the 0th based nth name of this colour
  const char *GetColorName(int ind);
  // Description:
  // Get the 0th based nth name of this colour, replacing the spaces with
  // subst
  const char *GetColorNameWithoutSpaces(int ind, const char *subst);
  
  // Description:
  // Add a color name to the vector
  void AddColorName(const char *name);
  // Description:
  // Set the 0th based nth name of this colour
  void SetColorName(int ind, const char *name);
  // Description:
  // Set the 0th based nth name of this colour, replacing the subst character
  // with spaces
  void SetColorNameWithSpaces(int ind, const char *name, const char *subst);
  
  // Description:
  // Name of the file name from which to read color information
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  
protected:
  vtkMRMLColorNode();
  ~vtkMRMLColorNode();
  vtkMRMLColorNode(const vtkMRMLColorNode&);
  void operator=(const vtkMRMLColorNode&);

  // Description:
  // Set values in the names vector from the colour rgba entries in the colour
  // table
  void SetNamesFromColors();
  
  // Description:
  // Which type of look up table does this node hold? 
  // Valid values are in the enumerated list
  int Type;

  // Description: 
  // The look up table, constructed according to the Type
  vtkLookupTable *LookupTable;

  //BTX
  // Description:
  // A vector of names for the color table elements
  std::vector<std::string> Names;
  //ETX

  // Description:
  // A file name to read text attributes from
  char *FileName;
};

#endif
