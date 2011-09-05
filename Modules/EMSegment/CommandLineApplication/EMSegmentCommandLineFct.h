

#ifndef __EMSegmentCommandLineFct_h
#define __EMSegmentCommandLineFct_h

// STD includes
#include <string>
#include <vector>

class vtkEMSegmentLogic;
class vtkEMSegmentMRMLManager;
class vtkImageData;
class vtkMRMLScene;
class vtkMRMLVolumeNode;
class vtkSlicerApplicationLogic;
class vtkSlicerCommonInterface;

// -------------------------------------------------------------------------------------------
class ProgressReporter
{
 public:
  void ReportProgress(const std::string& message,
                      float totalProgress = 0.0f,
                      float stageProgress = 0.0f)
  {
    // unused variable
    (void)(message);
    (void)(totalProgress);
    (void)(stageProgress);
  }
};

// -------------------------------------------------------------------------------------------
// does not actually read an image from disk, this is intended for
// creating an image that you will later want to write to
vtkMRMLVolumeNode*
AddNewScalarArchetypeVolume(vtkMRMLScene* mrmlScene,
                            const char* filename,
                            int centerImage,
                            int labelMap,
                            const char* volname);

// -------------------------------------------------------------------------------------------
// This function checks to see if the image stored in standardFilename
// differs from resultData.  True is returned if the images differ,
// false is returned if they are identical.
bool ImageDiff(vtkImageData* resultData, std::string standardFilename);


// -------------------------------------------------------------------------------------------
int LoadMRMLScene(vtkEMSegmentMRMLManager* emMRMLManager, vtkMRMLScene* mrmlScene,  int verbose);

// -------------------------------------------------------------------------------------------
int DefineTargetVolume( int useDefaultTarget,  std::vector<std::string>  targetVolumeFileNames, vtkEMSegmentLogic* EMSLogic, vtkMRMLScene* mrmlScene,  vtkSlicerApplicationLogic* appLogic, int verbose, bool centered);

// -------------------------------------------------------------------------------------------
int LoadUserDefinedAtlas (std::vector<std::string> atlasVolumeFileNames,  vtkEMSegmentLogic* EMSLogic, vtkMRMLScene* mrmlScene, vtkSlicerApplicationLogic* appLogic, int verbose, bool centered);

// -------------------------------------------------------------------------------------------
int DefineEMSTemplate (int useDefaultParametersNode, std::string parametersMRMLNodeName, vtkEMSegmentMRMLManager* emMRMLManager, int verbose);

// -------------------------------------------------------------------------------------------
int DefineFinalOutput(  int useDefaultOutput, std::string resultVolumeFileName, vtkEMSegmentMRMLManager* emMRMLManager, vtkMRMLScene* mrmlScene, int verbose);

// -------------------------------------------------------------------------------------------
int RunPreprocessing(vtkEMSegmentLogic* EMSLogic, std::string EMSLogicTcl, std::string emMRMLManagerTcl, vtkSlicerCommonInterface* slicerCommon, vtkEMSegmentMRMLManager* emMRMLManager, int verbose);

// -------------------------------------------------------------------------------------------
bool WriteResultsToFile(int disableCompression, vtkEMSegmentMRMLManager* emMRMLManager, int verbose);


// -------------------------------------------------------------------------------------------
bool CompareResultsToStandard(std::string resultStandardVolumeFileName, int disableCompression, vtkEMSegmentMRMLManager* emMRMLManager, vtkMRMLScene* mrmlScene, int verbose);

// -------------------------------------------------------------------------------------------
void GenerateEmptyMRMLScene(const char* filename);

#endif

