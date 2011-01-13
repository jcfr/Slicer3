#ifndef __vtkMRMLEMSWorkingDataNode_h
#define __vtkMRMLEMSWorkingDataNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkEMSegment.h"
#include "vtkMRMLEMSVolumeCollectionNode.h"

class vtkMRMLEMSTargetNode;
class vtkMRMLEMSAtlasNode;

class VTK_EMSEGMENT_EXPORT vtkMRMLEMSWorkingDataNode : 
  public vtkMRMLEMSVolumeCollectionNode
{
public:
  static vtkMRMLEMSWorkingDataNode *New();
  vtkTypeMacro(vtkMRMLEMSWorkingDataNode,vtkMRMLEMSVolumeCollectionNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes(const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "EMSWorkingData";}

  // Description:
  // Updates this node if it depends on other nodes
  // when the node is deleted in the scene
  virtual void UpdateReferences();

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  vtkGetStringMacro(InputTargetNodeID);
  //BTX
  vtkSetReferenceStringMacro(InputTargetNodeID);
  //ETX
  // For tcl Wrapping
  void SetReferenceInputTargetNodeID(const char *input)
  {
    this->SetInputTargetNodeID(input);
  }
  vtkMRMLEMSTargetNode* GetInputTargetNode();

  vtkGetStringMacro(AlignedTargetNodeID);
  //BTX
  vtkSetReferenceStringMacro(AlignedTargetNodeID);
  //ETX
  void SetReferenceAlignTargetNodeID(const char* name)
  {
    this->SetAlignedTargetNodeID(name);
  } 
  vtkMRMLEMSTargetNode* GetAlignedTargetNode();
  
  vtkGetStringMacro(InputAtlasNodeID);
  //BTX
  vtkSetReferenceStringMacro(InputAtlasNodeID);
  //ETX
  void SetReferenceInputAtlasNodeID(const char* name)
  {
    this->SetInputAtlasNodeID(name);
  } 
  vtkMRMLEMSAtlasNode* GetInputAtlasNode();

  vtkGetStringMacro(AlignedAtlasNodeID);
  //BTX
  vtkSetReferenceStringMacro(AlignedAtlasNodeID);
  //ETX
  void SetReferenceAlignedAtlasNodeID(const char* name)
  {
    this->SetAlignedAtlasNodeID(name);
  } 
  vtkMRMLEMSAtlasNode* GetAlignedAtlasNode();

  vtkGetStringMacro(InputSubParcellationNodeID);
  //BTX
  vtkSetReferenceStringMacro(InputSubParcellationNodeID);
  //ETX
  void SetReferenceInputSubParcellationNodeID(const char* name)
  {
    this->SetInputSubParcellationNodeID(name);
  } 
  vtkMRMLEMSVolumeCollectionNode* GetInputSubParcellationNode();

  vtkGetStringMacro(AlignedSubParcellationNodeID);
  //BTX
  vtkSetReferenceStringMacro(AlignedSubParcellationNodeID);
  //ETX
  void SetReferenceAlignedSubParcellationNodeID(const char* name)
  {
    this->SetAlignedSubParcellationNodeID(name);
  } 
  vtkMRMLEMSVolumeCollectionNode* GetAlignedSubParcellationNode();

  vtkGetMacro(InputTargetNodeIsValid, int);
  vtkSetMacro(InputTargetNodeIsValid, int);

  vtkGetMacro(AlignedTargetNodeIsValid, int);
  vtkSetMacro(AlignedTargetNodeIsValid, int);

  vtkGetMacro(InputAtlasNodeIsValid, int);
  vtkSetMacro(InputAtlasNodeIsValid, int);

  vtkGetMacro(AlignedAtlasNodeIsValid, int);
  vtkSetMacro(AlignedAtlasNodeIsValid, int);

protected:
  vtkMRMLEMSWorkingDataNode();
  ~vtkMRMLEMSWorkingDataNode();
  vtkMRMLEMSWorkingDataNode(const vtkMRMLEMSWorkingDataNode&);
  void operator=(const vtkMRMLEMSWorkingDataNode&);

  char*                InputTargetNodeID;
  char*                AlignedTargetNodeID;
  char*                InputAtlasNodeID;
  char*                AlignedAtlasNodeID;
  char*                InputSubParcellationNodeID;
  char*                AlignedSubParcellationNodeID;

  int                  InputTargetNodeIsValid;
  int                  AlignedTargetNodeIsValid;  
  int                  InputAtlasNodeIsValid;  
  int                  AlignedAtlasNodeIsValid;  
};

#endif
