#ifndef __vtkQueryAtlasUseSearchTermWidget_h
#define __vtkQueryAtlasUseSearchTermWidget_h


#include "vtkQueryAtlasWin32Header.h"
#include "vtkSlicerWidget.h"

class vtkKWPushButton;
class vtkKWMultiColumnListWithScrollbars;
class vtkQueryAtlasIcons;
class vtkKWFrame;


class VTK_QUERYATLAS_EXPORT vtkQueryAtlasUseSearchTermWidget : public vtkSlicerWidget
{
  
public:
  static vtkQueryAtlasUseSearchTermWidget* New();
  vtkTypeRevisionMacro(vtkQueryAtlasUseSearchTermWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get methods on class members ( no Set methods required. )
  vtkGetObjectMacro ( AddNewButton, vtkKWPushButton);
  vtkGetObjectMacro ( UseAllButton, vtkKWPushButton);
  vtkGetObjectMacro ( UseNoneButton, vtkKWPushButton);
  vtkGetObjectMacro ( DeselectAllButton, vtkKWPushButton);
  vtkGetObjectMacro ( ClearAllButton, vtkKWPushButton);
  vtkGetObjectMacro ( ClearSelectedButton, vtkKWPushButton);
  vtkGetObjectMacro ( MultiColumnList, vtkKWMultiColumnListWithScrollbars );
  vtkGetObjectMacro ( QueryAtlasIcons, vtkQueryAtlasIcons );
  vtkGetObjectMacro ( ContainerFrame, vtkKWFrame );
  vtkGetMacro (NumberOfColumns, int );
  
  // Description:
  // Method that gets all terms in the multicolumn list
  virtual void GetAllSearchTerms ( );
  // Description:
  // Method that gets all terms in the multicolumn list
  virtual void GetSearchTermsToUse ( );

  virtual void CheckAllSearchTerms ( );
  virtual void UncheckAllSearchTerms ( );
  virtual void DeselectAllSearchTerms ( );
  virtual void DeleteAllSearchTerms ( );
  virtual void AddNewSearchTerm ( const char *term);
  virtual void DeleteSelectedSearchTerms ( );
  virtual void GetNumberOfSearchTermsToUse( );
  virtual void GetNthSearchTermToUse ( );
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // removes observers on widgets in the class
  virtual void RemoveWidgetObservers ( );
  // Description:
  // adds observers on widgets in the class
  virtual void AddWidgetObservers ( );

  // Description:
  // add observers on color node
  virtual void AddMRMLObservers ( );

  // Description:
  // remove observers on color node
  virtual void RemoveMRMLObservers ( );

  
 protected:
  vtkQueryAtlasUseSearchTermWidget();
  virtual ~vtkQueryAtlasUseSearchTermWidget();

  vtkKWPushButton *AddNewButton;
  vtkKWPushButton *UseAllButton;
  vtkKWPushButton *UseNoneButton;
  vtkKWPushButton *DeselectAllButton;
  vtkKWPushButton *ClearAllButton;
  vtkKWPushButton *ClearSelectedButton;
  vtkKWMultiColumnListWithScrollbars *MultiColumnList;
  vtkQueryAtlasIcons *QueryAtlasIcons;
  vtkKWFrame *ContainerFrame;
  
  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Update the widget, used when the color node id changes
  void UpdateWidget();
  
  void UpdateMRML();

  // Description:
  // Called when the selected row changes, just update the label, called from UpdateWidget
  void UpdateSelectedColor();
  
  //BTX
  // Description:
  // The column orders in the list box
  enum
    {
      UseSelectionColumn = 0,
      SearchTermColumn = 1,
    };
  //ETX

  int NumberOfColumns;

  vtkQueryAtlasUseSearchTermWidget(const vtkQueryAtlasUseSearchTermWidget&); // Not implemented
  void operator=(const vtkQueryAtlasUseSearchTermWidget&); // Not Implemented
};

#endif

