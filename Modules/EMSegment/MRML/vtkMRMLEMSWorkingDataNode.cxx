#include "vtkMRMLEMSWorkingDataNode.h"
#include <sstream>
#include "vtkMRMLScene.h"
#include "vtkMRMLEMSTargetNode.h"
#include "vtkMRMLEMSAtlasNode.h"

// for some reason it was otherwise not wrapping it in tcl
// maybe take it out later 

//-----------------------------------------------------------------------------
vtkMRMLEMSWorkingDataNode* 
vtkMRMLEMSWorkingDataNode::
New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSWorkingDataNode");
  if(ret)
    {
    return (vtkMRMLEMSWorkingDataNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSWorkingDataNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* 
vtkMRMLEMSWorkingDataNode::
CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSWorkingDataNode");
  if(ret)
    {
    return (vtkMRMLEMSWorkingDataNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSWorkingDataNode;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSWorkingDataNode::vtkMRMLEMSWorkingDataNode()
{
  this->InputTargetNodeID                = NULL;
  this->AlignedTargetNodeID              = NULL;
  this->InputAtlasNodeID                 = NULL;
  this->AlignedAtlasNodeID               = NULL;
  this->InputSubParcellationNodeID       = NULL;
  this->AlignedSubParcellationNodeID     = NULL;

  this->InputTargetNodeIsValid           = 0;
  this->AlignedTargetNodeIsValid         = 0;  
  this->InputAtlasNodeIsValid            = 0;  
  this->AlignedAtlasNodeIsValid          = 0;  
}

//-----------------------------------------------------------------------------
vtkMRMLEMSWorkingDataNode::~vtkMRMLEMSWorkingDataNode()
{
  this->SetInputTargetNodeID(NULL);
  this->SetAlignedTargetNodeID(NULL);
  this->SetInputAtlasNodeID(NULL);
  this->SetAlignedAtlasNodeID(NULL);
  this->SetInputAtlasNodeID(NULL);
  this->SetAlignedAtlasNodeID(NULL);
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSWorkingDataNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  of << indent << " InputTargetNodeID=\"" 
     << (this->InputTargetNodeID ? this->InputTargetNodeID : "NULL")
     << "\" ";
  of << indent << " InputTargetNodeIsValid=\""   << this->InputTargetNodeIsValid << "\" ";

  of << indent << "AlignedTargetNodeID=\"" 
     << (this->AlignedTargetNodeID ? this->AlignedTargetNodeID : "NULL")
     << "\" ";
  of << indent << " AlignedTargetNodeIsValid=\"" << this->AlignedTargetNodeIsValid  << "\" ";

  of << indent << "InputAtlasNodeID=\"" 
     << (this->InputAtlasNodeID ? this->InputAtlasNodeID : "NULL")
     << "\" ";
  of << indent << " InputAtlasNodeIsValid=\""    << this->InputAtlasNodeIsValid  << "\" ";

  of << indent << "AlignedAtlasNodeID=\"" 
     << (this->AlignedAtlasNodeID ? this->AlignedAtlasNodeID : "NULL")
     << "\" ";
  of << indent << " AlignedAtlasNodeIsValid=\"" <<  this->AlignedAtlasNodeIsValid  << "\" ";

  of << indent << "InputSubParcellationNodeID=\"" 
     << (this->InputSubParcellationNodeID ? this->InputSubParcellationNodeID : "NULL")
     << "\" ";

  of << indent << "AlignedSubParcellationNodeID=\"" 
     << (this->AlignedSubParcellationNodeID ? this->AlignedSubParcellationNodeID : "NULL")
     << "\" ";

}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSWorkingDataNode::
UpdateReferenceID(const char* oldID, const char* newID)
{
  if (this->InputTargetNodeID && 
      !strcmp(oldID, this->InputTargetNodeID))
    {
    this->SetInputTargetNodeID(newID);
    }
  if (this->AlignedTargetNodeID && 
      !strcmp(oldID, this->AlignedTargetNodeID))
    {
    this->SetAlignedTargetNodeID(newID);
    }
  if (this->InputAtlasNodeID && 
      !strcmp(oldID, this->InputAtlasNodeID))
    {
    this->SetInputAtlasNodeID(newID);
    }
  if (this->AlignedAtlasNodeID && 
      !strcmp(oldID, this->AlignedAtlasNodeID))
    {
    this->SetAlignedAtlasNodeID(newID);
    }
  if (this->InputSubParcellationNodeID && 
      !strcmp(oldID, this->InputSubParcellationNodeID))
    {
    this->SetInputSubParcellationNodeID(newID);
    }
  if (this->AlignedSubParcellationNodeID && 
      !strcmp(oldID, this->AlignedSubParcellationNodeID))
    {
    this->SetAlignedSubParcellationNodeID(newID);
    }
}

//-----------------------------------------------------------------------------
void 
vtkMRMLEMSWorkingDataNode::
UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->InputTargetNodeID != NULL && 
      this->Scene->GetNodeByID(this->InputTargetNodeID) == NULL)
    {
    this->SetInputTargetNodeID(NULL);
    }
  if (this->AlignedTargetNodeID != NULL && 
      this->Scene->GetNodeByID(this->AlignedTargetNodeID) == NULL)
    {
    this->SetAlignedTargetNodeID(NULL);
    }
  if (this->InputAtlasNodeID != NULL && 
      this->Scene->GetNodeByID(this->InputAtlasNodeID) == NULL)
    {
    this->SetInputAtlasNodeID(NULL);
    }
  if (this->AlignedAtlasNodeID != NULL && 
      this->Scene->GetNodeByID(this->AlignedAtlasNodeID) == NULL)
    {
    this->SetAlignedAtlasNodeID(NULL);
    }
  if (this->InputSubParcellationNodeID != NULL && 
      this->Scene->GetNodeByID(this->InputSubParcellationNodeID) == NULL)
    {
    this->SetInputSubParcellationNodeID(NULL);
    }
  if (this->AlignedSubParcellationNodeID != NULL && 
      this->Scene->GetNodeByID(this->AlignedSubParcellationNodeID) == NULL)
    {
    this->SetAlignedSubParcellationNodeID(NULL);
    }
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSWorkingDataNode::ReadXMLAttributes(const char** attrs)
{
  Superclass::ReadXMLAttributes(attrs);

  //
  // we assume an even number of elements
  //
  const char* key;
  const char* val;
  while (*attrs != NULL)
    {
    key = *attrs++;
    val = *attrs++;
    
    if (!strcmp(key, "InputTargetNodeID"))
      {
      this->SetInputTargetNodeID(val);
      }
    else if (!strcmp(key, "InputTargetNodeIsValid"))
      {
    vtksys_stl::stringstream ss;
    ss << val;
    ss >> this->InputTargetNodeIsValid;
      }
    else if (!strcmp(key, "AlignedTargetNodeID"))
      {
      this->SetAlignedTargetNodeID(val);
      }
    else if (!strcmp(key, "AlignedTargetNodeIsValid"))
      {
    vtksys_stl::stringstream ss;
    ss << val;
    ss >> this->AlignedTargetNodeIsValid;
      }
    else if (!strcmp(key, "InputAtlasNodeIsValid"))
      {
    vtksys_stl::stringstream ss;
    ss << val;
    ss >> this->InputAtlasNodeIsValid;
      }
    else if (!strcmp(key, "AlignedAtlasNodeIsValid"))
      {
    vtksys_stl::stringstream ss;
    ss << val;
    ss >> this->AlignedAtlasNodeIsValid;
      }
    else if (!strcmp(key, "InputAtlasNodeID"))
      {
      this->SetInputAtlasNodeID(val);
      }
    else if (!strcmp(key, "AlignedAtlasNodeID"))
      {
      this->SetAlignedAtlasNodeID(val);
      }
    else if (!strcmp(key, "InputSubParcellationNodeID"))
      {
      this->SetInputSubParcellationNodeID(val);
      }
    else if (!strcmp(key, "AlignedSubParcellationNodeID"))
      {
      this->SetAlignedSubParcellationNodeID(val);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSWorkingDataNode::Copy(vtkMRMLNode *rhs)
{
  Superclass::Copy(rhs);
  vtkMRMLEMSWorkingDataNode* node = (vtkMRMLEMSWorkingDataNode*) rhs;

  this->SetInputTargetNodeID(node->InputTargetNodeID);
  this->SetAlignedTargetNodeID(node->AlignedTargetNodeID);
  this->SetInputAtlasNodeID(node->InputAtlasNodeID);
  this->SetAlignedAtlasNodeID(node->AlignedAtlasNodeID);
  this->SetInputSubParcellationNodeID(node->InputSubParcellationNodeID);
  this->SetAlignedSubParcellationNodeID(node->AlignedSubParcellationNodeID);
  this->InputTargetNodeIsValid           = node->InputTargetNodeIsValid;
  this->AlignedTargetNodeIsValid         = node->AlignedTargetNodeIsValid;  
  this->InputAtlasNodeIsValid            = node->InputAtlasNodeIsValid;  
  this->AlignedAtlasNodeIsValid          = node->AlignedAtlasNodeIsValid;  
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSWorkingDataNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  
  os << indent << "InputTargetNodeID: " <<
    (this->InputTargetNodeID ? this->InputTargetNodeID : "(none)") 
     << "\n";
  os << indent << "AlignedTargetNodeID: " <<
    (this->AlignedTargetNodeID ? this->AlignedTargetNodeID : "(none)") 
     << "\n";

  os << indent << "InputAtlasNodeID: " <<
    (this->InputAtlasNodeID ? this->InputAtlasNodeID : "(none)") 
     << "\n";

  os << indent << "AlignedAtlasNodeID: " <<
    (this->AlignedAtlasNodeID ? this->AlignedAtlasNodeID : "(none)") 
     << "\n";

  os << indent << "InputSubParcellationNodeID: " <<
    (this->InputSubParcellationNodeID ? this->InputSubParcellationNodeID : "(none)") 
     << "\n";

  os << indent << "AlignedSubParcellationNodeID: " <<
    (this->AlignedSubParcellationNodeID ? this->AlignedSubParcellationNodeID : "(none)") 
     << "\n";

  os << indent << "InputTargetNodeIsValid: " << this->InputTargetNodeIsValid << "\n";
  os << indent << "AlignedTargetNodeIsValid: " << this->AlignedTargetNodeIsValid << "\n";
  os << indent << "InputAtlasNodeIsValid: " << this->InputAtlasNodeIsValid  << "\n";
  os << indent << "AlignedAtlasNodeIsValid: " << this->AlignedAtlasNodeIsValid << "\n";

}

//-----------------------------------------------------------------------------
vtkMRMLEMSTargetNode*
vtkMRMLEMSWorkingDataNode::
GetInputTargetNode()
{
  vtkMRMLEMSTargetNode* node = NULL;
  if (this->GetScene() && this->InputTargetNodeID)
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->InputTargetNodeID);
    node = vtkMRMLEMSTargetNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTargetNode*
vtkMRMLEMSWorkingDataNode::
GetAlignedTargetNode()
{
  vtkMRMLEMSTargetNode* node = NULL;
  if (this->GetScene() && this->AlignedTargetNodeID )
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->AlignedTargetNodeID);
    node = vtkMRMLEMSTargetNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSAtlasNode*
vtkMRMLEMSWorkingDataNode::
GetInputAtlasNode()
{
  vtkMRMLEMSAtlasNode* node = NULL;
  if (this->GetScene() && this->InputAtlasNodeID )
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->InputAtlasNodeID);
    node = vtkMRMLEMSAtlasNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSAtlasNode*
vtkMRMLEMSWorkingDataNode::
GetAlignedAtlasNode()
{
  vtkMRMLEMSAtlasNode* node = NULL;
  if (this->GetScene() && this->AlignedAtlasNodeID )
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->AlignedAtlasNodeID);
    node = vtkMRMLEMSAtlasNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSVolumeCollectionNode*
vtkMRMLEMSWorkingDataNode::
GetInputSubParcellationNode()
{
  vtkMRMLEMSVolumeCollectionNode* node = NULL;
  if (this->GetScene() && this->InputSubParcellationNodeID )
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->InputSubParcellationNodeID);
    node = vtkMRMLEMSVolumeCollectionNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSVolumeCollectionNode*
vtkMRMLEMSWorkingDataNode::
GetAlignedSubParcellationNode()
{
  vtkMRMLEMSVolumeCollectionNode* node = NULL;
  if (this->GetScene() && this->AlignedSubParcellationNodeID )
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->AlignedSubParcellationNodeID);
    node = vtkMRMLEMSVolumeCollectionNode::SafeDownCast(snode);
    }
  return node;
}


