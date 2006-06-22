/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkGradientAnisotropicDiffusionFilterLogic.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkCommandLineModuleLogic.h"
#include "vtkCommandLineModule.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"

#include "itksys/Process.h"
#include "itksys/SystemTools.hxx"

#include <algorithm>
#include <set>

struct DigitsToCharacters
{
  char operator() (char in)
    {
      if (in >= 48 && in <= 57)
        {
        return in + 17;
        }

      return in;
    }
};


vtkCommandLineModuleLogic* vtkCommandLineModuleLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkCommandLineModuleLogic");
  if(ret)
    {
      return (vtkCommandLineModuleLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkCommandLineModuleLogic;
}


//----------------------------------------------------------------------------
vtkCommandLineModuleLogic::vtkCommandLineModuleLogic()
{
  this->CommandLineModuleNode = NULL;
}

//----------------------------------------------------------------------------
vtkCommandLineModuleLogic::~vtkCommandLineModuleLogic()
{
  this->SetCommandLineModuleNode(NULL);
}

//----------------------------------------------------------------------------
void vtkCommandLineModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

//----------------------------------------------------------------------------
std::string
vtkCommandLineModuleLogic
::ConstructTemporaryFileName(const std::string& name) const
{
  std::string fname = name;
  
  // To avoid confusing the Archetype readers, convert any
  // numbers in the filename to characters [0-9]->[A-J]
  std::transform(fname.begin(), fname.end(),
                 fname.begin(), DigitsToCharacters());
  
  fname = this->TemporaryDirectory + "/" + fname + ".nrrd";

  return fname;
}

void vtkCommandLineModuleLogic::Apply()
{
  // check if MRML node is present 
  if (this->CommandLineModuleNode == NULL)
    {
    vtkErrorMacro("No input CommandLineModuleNode found");
    return;
    }

  
  // map to keep track of MRML Ids and filenames
  typedef std::map<std::string, std::string> MRMLIDToFileNameMap;
  MRMLIDToFileNameMap nodesToReload;
  MRMLIDToFileNameMap nodesToWrite;

  // vector of files to delete
  std::set<std::string> filesToDelete;
  
  // iterators for parameter groups
  std::vector<ModuleParameterGroup>::const_iterator pgbeginit
    = this->CommandLineModuleNode->GetModuleDescription()
    .GetParameterGroups().begin();
  std::vector<ModuleParameterGroup>::const_iterator pgendit
    = this->CommandLineModuleNode->GetModuleDescription()
    .GetParameterGroups().end();
  std::vector<ModuleParameterGroup>::const_iterator pgit;

  
  // Make a pass over the parameters and estabilsh which parameters
  // have images that need to be written before execution or loaded
  // upon completion. 
  for (pgit = pgbeginit; pgit != pgendit; ++pgit)
    {
    // iterate over each parameter in this group
    std::vector<ModuleParameter>::const_iterator pbeginit
      = (*pgit).GetParameters().begin();
    std::vector<ModuleParameter>::const_iterator pendit
      = (*pgit).GetParameters().end();
    std::vector<ModuleParameter>::const_iterator pit;

    for (pit = pbeginit; pit != pendit; ++pit)
      {
      if ((*pit).GetTag() == "image")
        {
        // only keep track of images associated with real nodes
        if (!this->MRMLScene->GetNodeByID((*pit).GetDefault().c_str())
            || (*pit).GetDefault() == "None")
          {
          continue;
          }

        std::string fname
          = this->ConstructTemporaryFileName((*pit).GetDefault());

        filesToDelete.insert(fname);

        if ((*pit).GetChannel() == "input")
          {
          nodesToWrite[(*pit).GetDefault()] = fname;
          }
        else if ((*pit).GetChannel() == "output")
          {
          nodesToReload[(*pit).GetDefault()] = fname;
          }
        }
      }
    }
  

  // build the command line
  //
  //
  std::vector<std::string> commandLineAsString;

  // Command to execute
  commandLineAsString.push_back( this->CommandLineModuleNode
                                 ->GetModuleDescription().GetTarget() );


  // Run over all the parameters with flags
  for (pgit = pgbeginit; pgit != pgendit; ++pgit)
    {
    // iterate over each parameter in this group
    std::vector<ModuleParameter>::const_iterator pbeginit
      = (*pgit).GetParameters().begin();
    std::vector<ModuleParameter>::const_iterator pendit
      = (*pgit).GetParameters().end();
    std::vector<ModuleParameter>::const_iterator pit;

    for (pit = pbeginit; pit != pendit; ++pit)
      {
      if ((*pit).GetLongFlag() != "")
        {
        if ((*pit).GetTag() != "boolean" && (*pit).GetTag() != "image")
          {
          // simple parameter, write flag and value
          commandLineAsString.push_back(std::string("--")
                                        + (*pit).GetLongFlag());
          commandLineAsString.push_back((*pit).GetDefault());
          }
        if ((*pit).GetTag() == "boolean")
          {
          // booleans only have a flag (no value)
          if ((*pit).GetDefault() == "true")
            {
            commandLineAsString.push_back(std::string("--")
                                          + (*pit).GetLongFlag());
            }
          }
        if ((*pit).GetTag() == "image")
          {
          MRMLIDToFileNameMap::const_iterator id2fn;
          
          id2fn  = nodesToWrite.find( (*pit).GetDefault() );
          if ((*pit).GetChannel() == "input" && id2fn != nodesToWrite.end())
            {
            // Only put out the flag if the node is in out list
            commandLineAsString.push_back(std::string("--"
                                                      + (*pit).GetLongFlag()));
            commandLineAsString.push_back( (*id2fn).second );
            }

          id2fn  = nodesToReload.find( (*pit).GetDefault() );
          if ((*pit).GetChannel() == "output" && id2fn != nodesToReload.end())
            {
            commandLineAsString.push_back(std::string("--"
                                                      + (*pit).GetLongFlag()));
            commandLineAsString.push_back( (*id2fn).second );
            }
          }
        }
      else if ((*pit).GetFlag() != "")
        {
        if ((*pit).GetTag() != "boolean" && (*pit).GetTag() != "image")
          {
          // simple parameter, write flag and value
          commandLineAsString.push_back(std::string("-")+(*pit).GetFlag());
          commandLineAsString.push_back((*pit).GetDefault());
          }
        if ((*pit).GetTag() == "boolean")
          {
          // booleans only have a flag (no value)
          if ((*pit).GetDefault() == "true")
            {
            commandLineAsString.push_back(std::string("-")
                                          + (*pit).GetFlag());
            }
          }
        if ((*pit).GetTag() == "image")
          {
          MRMLIDToFileNameMap::const_iterator id2fn;
          
          id2fn  = nodesToWrite.find( (*pit).GetDefault() );
          if ((*pit).GetChannel() == "input"  &&  id2fn != nodesToWrite.end())
            {
            // Only put out the flag if the node is in our list
            commandLineAsString.push_back(std::string("-" + (*pit).GetFlag()));
            commandLineAsString.push_back( (*id2fn).second );
            }

          id2fn  = nodesToReload.find( (*pit).GetDefault() );
          if ((*pit).GetChannel() == "output" && id2fn != nodesToReload.end())
            {
            // Only put out the flag if the node is in our list
            commandLineAsString.push_back(std::string("-" + (*pit).GetFlag()));
            commandLineAsString.push_back( (*id2fn).second );
            }
          }
        }
      }
    }

  // now tack on any parameters that are based on indices
  //
  // build a list of indices to traverse in order
  std::map<int, ModuleParameter> indexmap;
  for (pgit = pgbeginit; pgit != pgendit; ++pgit)
    {
    // iterate over each parameter in this group
    std::vector<ModuleParameter>::const_iterator pbeginit
      = (*pgit).GetParameters().begin();
    std::vector<ModuleParameter>::const_iterator pendit
      = (*pgit).GetParameters().end();
    std::vector<ModuleParameter>::const_iterator pit;
  
    for (pit = pbeginit; pit != pendit; ++pit)
      {
      if ((*pit).GetIndex() != "")
        {
        indexmap[atoi((*pit).GetIndex().c_str())] = (*pit);
        }
      }
    }

  // walk the index parameters in order
  std::map<int, ModuleParameter>::const_iterator iit;
  for (iit = indexmap.begin(); iit != indexmap.end(); ++iit)
    {
    if ((*iit).second.GetTag() != "image")
      {
      commandLineAsString.push_back((*iit).second.GetDefault());
      }
    else
      {
      MRMLIDToFileNameMap::const_iterator id2fn;

      if ((*iit).second.GetChannel() == "input")
        {
        // Check to make sure the index parameter is set
        id2fn  = nodesToWrite.find( (*iit).second.GetDefault() );
        if (id2fn != nodesToWrite.end())
          {
          commandLineAsString.push_back( (*id2fn).second );
          }
        else
          {
          vtkErrorMacro("No input volume assigned to \""
                        << (*iit).second.GetLabel().c_str() << "\"");
          return;
          }
        }
      else if ((*iit).second.GetChannel() == "output")
        {
        // Check to make sure the index parameter is set
        id2fn  = nodesToReload.find( (*iit).second.GetDefault() );
        if (id2fn != nodesToReload.end())
          {
          commandLineAsString.push_back( (*id2fn).second );
          }
        else
          {
          vtkErrorMacro("No output volume assigned to \""
                        << (*iit).second.GetLabel().c_str() << "\"");
          return;
          }
        }
      }
    }


  // copy the command line arguments into an array of pointers to
  // chars
  char **command = new char*[commandLineAsString.size()+1];
  for (std::vector<std::string>::size_type i=0; i < commandLineAsString.size(); ++i)
    {
    command[i] = const_cast<char*>(commandLineAsString[i].c_str());
    }
  command[commandLineAsString.size()] = 0;

  // print the command line
  //
  for (std::vector<std::string>::size_type i=0; i < commandLineAsString.size(); ++i)
    {
    std::cout << command[i] << " ";
    }
  std::cout << std::endl;

  
  // write out the input volumes
  //
  //
  MRMLIDToFileNameMap::const_iterator id2fn;

  for (id2fn = nodesToWrite.begin();
       id2fn != nodesToWrite.end();
       ++id2fn)
    {
    vtkMRMLVolumeArchetypeStorageNode *out
      = vtkMRMLVolumeArchetypeStorageNode::New();
    out->SetFileArchetype( (*id2fn).second.c_str() );

    out->WriteData( this->MRMLScene->GetNodeByID( (*id2fn).first.c_str() ) );
    
    out->Delete();
    }
  

  // run the filter
  //
  //
  itksysProcess *process = itksysProcess_New();

  // setup the command
  itksysProcess_SetCommand(process, command);
  itksysProcess_SetOption(process,
                          itksysProcess_Option_Detach, 0);
  itksysProcess_SetOption(process,
                          itksysProcess_Option_HideWindow, 1);
  // itksysProcess_SetTimeout(process, 5.0); // 5 seconds

  // execute the command
  itksysProcess_Execute(process);

  // Wait for the command to finish
  char *tbuffer;
  int length;
  int pipe;
  std::string stdoutbuffer;
  std::string stderrbuffer;
  while ((pipe = itksysProcess_WaitForData(process ,&tbuffer,
                                           &length, 0)) != 0)
    {
    if (length != 0 && tbuffer != 0)
      {
      if (pipe == itksysProcess_Pipe_STDOUT)
        {
        stdoutbuffer = stdoutbuffer.append(tbuffer, length);
        }
      else if (pipe == itksysProcess_Pipe_STDERR)
        {
        stderrbuffer = stderrbuffer.append(tbuffer, length);
        }
      }
    }
  itksysProcess_WaitForExit(process, 0);
  
  // check the exit state / error state of the process
  int result = itksysProcess_GetState(process);
  if (result == itksysProcess_State_Exited)
    {
    // executable exited cleanly and must of done
    // "something" 
    if (itksysProcess_GetExitValue(process) == 0)
      {
      // executable exited without errors,
      std::cout << this->CommandLineModuleNode->GetModuleDescription().GetTitle()
                << " completed without errors" << std::endl;
      }
    else
      {
      std::cout << this->CommandLineModuleNode->GetModuleDescription().GetTitle()
                << " completed with errors" << std::endl;
      }
    }
  else if (result == itksysProcess_State_Expired)
    {
    std::cout << this->CommandLineModuleNode->GetModuleDescription().GetTitle()
              << " timed out" << std::endl;
    }
  else
    {
    std::cout << this->CommandLineModuleNode->GetModuleDescription().GetTitle()
              << " unknown termination. " << result << std::endl;
    }
  
  // clean up
  itksysProcess_Delete(process);
  
  
  // import the results
  //
  //
  for (id2fn = nodesToReload.begin();
       id2fn != nodesToReload.end();
       ++id2fn)
    {
    vtkMRMLVolumeArchetypeStorageNode *in
      = vtkMRMLVolumeArchetypeStorageNode::New();
    in->SetFileArchetype( (*id2fn).second.c_str() );

    in->ReadData( this->MRMLScene->GetNodeByID( (*id2fn).first.c_str() ) );
    
    in->Delete();
    }

  // clean up
  //
  //
  delete [] command;

  bool removed;
  std::set<std::string>::iterator fit;
  for (fit = filesToDelete.begin(); fit != filesToDelete.end(); ++fit)
    {
    removed = itksys::SystemTools::RemoveFile((*fit).c_str());
    if (!removed)
      {
      std::cout << "Unable to delete temporary file " << *fit << std::endl;
      }
    }
}
