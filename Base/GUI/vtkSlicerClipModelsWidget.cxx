#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerClipModelsWidget.h"
#include "vtkSlicerApplication.h"

#include "vtkKWWidget.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenu.h"
#include "vtkKWFrameWithLabel.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerClipModelsWidget );
vtkCxxRevisionMacro ( vtkSlicerClipModelsWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerClipModelsWidget::vtkSlicerClipModelsWidget ( ) 
{
  //---  
  // widgets comprising the SliceControllerWidget for now.
  this->RedSliceClipStateMenu = NULL;
  this->YellowSliceClipStateMenu = NULL;
  this->GreenSliceClipStateMenu = NULL;
  this->ClipTypeMenu = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerClipModelsWidget::~vtkSlicerClipModelsWidget ( )
{
  this->RemoveWidgetObservers();
  
  if ( this->RedSliceClipStateMenu ) 
    {
    this->RedSliceClipStateMenu->SetParent(NULL);
    this->RedSliceClipStateMenu->Delete ( );
    this->RedSliceClipStateMenu = NULL;
    }
  if ( this->YellowSliceClipStateMenu ) 
    {
    this->YellowSliceClipStateMenu->SetParent(NULL);
    this->YellowSliceClipStateMenu->Delete ( );
    this->YellowSliceClipStateMenu = NULL;
    }
  if ( this->GreenSliceClipStateMenu ) 
    {
    this->GreenSliceClipStateMenu->SetParent(NULL);
    this->GreenSliceClipStateMenu->Delete ( );
    this->GreenSliceClipStateMenu = NULL;
    }
    
  if ( this->ClipTypeMenu ) 
    {
    this->ClipTypeMenu->SetParent(NULL);
    this->ClipTypeMenu->Delete ( );
    this->ClipTypeMenu = NULL;
    }
  this->SetMRMLScene(NULL);
  this->SetClipModelsNode(NULL);
  
}

//----------------------------------------------------------------------------
void vtkSlicerClipModelsWidget::AddWidgetObservers ( )
{
  if ( this->RedSliceClipStateMenu == NULL ) 
    {
    vtkErrorMacro ("Can't add observers because CreateWidget hasn't been called");
    return;
    }
  
  this->YellowSliceClipStateMenu->GetWidget()->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand);
  this->RedSliceClipStateMenu->GetWidget()->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand);
  this->GreenSliceClipStateMenu->GetWidget()->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand);

  this->ClipTypeMenu->GetWidget()->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand);

}
  
//---------------------------------------------------------------------------
void vtkSlicerClipModelsWidget::RemoveWidgetObservers ( ) {

  if ( this->RedSliceClipStateMenu == NULL ) 
    {
    vtkErrorMacro ("Can't remove observers because CreateWidget hasn't been called");
    return;
    }
  
  this->YellowSliceClipStateMenu->GetWidget()->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand);
  this->RedSliceClipStateMenu->GetWidget()->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand);
  this->GreenSliceClipStateMenu->GetWidget()->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand);
  this->ClipTypeMenu->GetWidget()->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand);

}

//---------------------------------------------------------------------------
void vtkSlicerClipModelsWidget::CreateWidget ( ) 
{

  if ( !this->MRMLScene ) 
    {
    vtkErrorMacro ( << " MRML Scene must be set before creating widgets.");
    return;
    }
  
  // the widget is a frame with some widgets inside
  if (this->IsCreated ( ) ) 
    {
    vtkErrorMacro ( << this->GetClassName() << "already created.");
    return;
    
    }
  this->Superclass::CreateWidget ( );
  
  // ---
  //  FRAME            
  vtkKWFrameWithLabel *frame = vtkKWFrameWithLabel::New ( );
  frame->SetParent ( this->GetParent() );
  frame->Create ( );
  frame->SetLabelText ("Display");
  frame->CollapseFrame ( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 frame->GetWidgetName() );
  
  this->RedSliceClipStateMenu = vtkKWMenuButtonWithLabel::New();
  this->RedSliceClipStateMenu->SetParent(frame);
  this->RedSliceClipStateMenu->Create();
  this->RedSliceClipStateMenu->SetLabelWidth(18);
  this->RedSliceClipStateMenu->SetLabelText("Red Slice Clipping:");
  this->RedSliceClipStateMenu->GetWidget()->GetMenu()->AddRadioButton ( "Off");
  this->RedSliceClipStateMenu->GetWidget()->GetMenu()->AddRadioButton ( "Positive Space");
  this->RedSliceClipStateMenu->GetWidget()->GetMenu()->AddRadioButton ( "Negative Space");
  this->RedSliceClipStateMenu->GetWidget()->SetValue ( "Off" );
  this->Script(
    "pack %s -side top -anchor nw -expand n -fill x -padx 2 -pady 2", 
    this->RedSliceClipStateMenu->GetWidgetName());

  this->YellowSliceClipStateMenu = vtkKWMenuButtonWithLabel::New();
  this->YellowSliceClipStateMenu->SetParent(frame);
  this->YellowSliceClipStateMenu->Create();
  this->YellowSliceClipStateMenu->SetLabelWidth(18);
  this->YellowSliceClipStateMenu->SetLabelText("Yellow Slice Clipping:");
  this->YellowSliceClipStateMenu->GetWidget()->GetMenu()->AddRadioButton ( "Off");
  this->YellowSliceClipStateMenu->GetWidget()->GetMenu()->AddRadioButton ( "Positive Space");
  this->YellowSliceClipStateMenu->GetWidget()->GetMenu()->AddRadioButton ( "Negative Space");
  this->YellowSliceClipStateMenu->GetWidget()->SetValue ( "Off" );
  //this->YellowSliceClipStateMenu->GetWidget()->SetWidth ( 7 );
  this->Script(
    "pack %s -side top -anchor nw -expand n -fill x -padx 2 -pady 2", 
    this->YellowSliceClipStateMenu->GetWidgetName());

  this->GreenSliceClipStateMenu = vtkKWMenuButtonWithLabel::New();
  this->GreenSliceClipStateMenu->SetParent(frame);
  this->GreenSliceClipStateMenu->Create();
  this->GreenSliceClipStateMenu->SetLabelWidth(18);
  this->GreenSliceClipStateMenu->SetLabelText("Green Slice Clipping:");
  this->GreenSliceClipStateMenu->GetWidget()->GetMenu()->AddRadioButton ( "Off");
  this->GreenSliceClipStateMenu->GetWidget()->GetMenu()->AddRadioButton ( "Positive Space");
  this->GreenSliceClipStateMenu->GetWidget()->GetMenu()->AddRadioButton ( "Negative Space");
  this->GreenSliceClipStateMenu->GetWidget()->SetValue ( "Off" );
  this->Script(
    "pack %s -side top -anchor nw -expand n -fill x -padx 2 -pady 2", 
    this->GreenSliceClipStateMenu->GetWidgetName());

  this->ClipTypeMenu = vtkKWMenuButtonWithLabel::New();  
  this->ClipTypeMenu->SetParent(frame);
  this->ClipTypeMenu->Create();
  //this->ClipTypeMenu->SetLabelWidth(12);
  this->ClipTypeMenu->SetLabelText("Clip Type:");
  this->ClipTypeMenu->GetWidget()->GetMenu()->AddRadioButton ( "Intersection");
  this->ClipTypeMenu->GetWidget()->GetMenu()->AddRadioButton ( "Union");
  this->ClipTypeMenu->GetWidget()->SetValue ( "Intersection" );
  //this->ClipTypeMenu->GetWidget()->SetWidth ( 7 );
  this->Script(
    "pack %s -side top -anchor nw -expand n -fill x -padx 2 -pady 2", 
    this->ClipTypeMenu->GetWidgetName());

}

//----------------------------------------------------------------------------
void vtkSlicerClipModelsWidget::ProcessWidgetEvents ( vtkObject *caller,
                                               unsigned long event, 
                                               void *callData ) 
{ 
  // Update orientation if needed
  if ( vtkKWMenu::SafeDownCast(caller) == this->YellowSliceClipStateMenu->GetWidget()->GetMenu() )
    {
    vtkKWMenuButton *mb = this->YellowSliceClipStateMenu->GetWidget();
    if ( !strcmp (mb->GetValue(), "Off") )   
      {
      this->ClipModelsNode->SetYellowSliceClipState(vtkMRMLClipModelsNode::ClipOff);
      }
    else if ( !strcmp (mb->GetValue(), "Positive Space") )   
      {
      this->ClipModelsNode->SetYellowSliceClipState(vtkMRMLClipModelsNode::ClipPositiveSpace);
      }
    else if ( !strcmp (mb->GetValue(), "Negative Space") )   
      {
      this->ClipModelsNode->SetYellowSliceClipState(vtkMRMLClipModelsNode::ClipNegativeSpace);
      }
    }
  
  if ( vtkKWMenu::SafeDownCast(caller) == this->RedSliceClipStateMenu->GetWidget()->GetMenu() )
    {
    vtkKWMenuButton *mb = this->RedSliceClipStateMenu->GetWidget();
    if ( !strcmp (mb->GetValue(), "Off") )   
      {
      this->ClipModelsNode->SetRedSliceClipState(vtkMRMLClipModelsNode::ClipOff);
      }
    else if ( !strcmp (mb->GetValue(), "Positive Space") )   
      {
      this->ClipModelsNode->SetRedSliceClipState(vtkMRMLClipModelsNode::ClipPositiveSpace);
      }
    else if ( !strcmp (mb->GetValue(), "Negative Space") )   
      {
      this->ClipModelsNode->SetRedSliceClipState(vtkMRMLClipModelsNode::ClipNegativeSpace);
      }
    }
  
  if ( vtkKWMenu::SafeDownCast(caller) == this->GreenSliceClipStateMenu->GetWidget()->GetMenu() )
    {
    vtkKWMenuButton *mb = this->GreenSliceClipStateMenu->GetWidget();
    if ( !strcmp (mb->GetValue(), "Off") )   
      {
      this->ClipModelsNode->SetGreenSliceClipState(vtkMRMLClipModelsNode::ClipOff);
      }
    else if ( !strcmp (mb->GetValue(), "Positive Space") )   
      {
      this->ClipModelsNode->SetGreenSliceClipState(vtkMRMLClipModelsNode::ClipPositiveSpace);
      }
    else if ( !strcmp (mb->GetValue(), "Negative Space") )   
      {
      this->ClipModelsNode->SetGreenSliceClipState(vtkMRMLClipModelsNode::ClipNegativeSpace);
      }
    }
  
  
  if ( vtkKWMenu::SafeDownCast(caller) == this->ClipTypeMenu->GetWidget()->GetMenu() )
    {
    vtkKWMenuButton *mb = this->ClipTypeMenu->GetWidget();
    if ( !strcmp (mb->GetValue(), "Intersection") )   
      {
      this->ClipModelsNode->SetClipType(vtkMRMLClipModelsNode::ClipIntersection);
      }
    else if ( !strcmp (mb->GetValue(), "Union") )   
      {
      this->ClipModelsNode->SetClipType(vtkMRMLClipModelsNode::ClipUnion);
      }
    }
}



//----------------------------------------------------------------------------
void vtkSlicerClipModelsWidget::ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData ) 
{ 
  if ( this->ClipModelsNode == NULL)
    {
    return;
    }
  
  // 
  // Update the menu to match the node
  //
  vtkKWMenuButton *mb = this->YellowSliceClipStateMenu->GetWidget();
  switch (this->ClipModelsNode->GetYellowSliceClipState())
    {
    case vtkMRMLClipModelsNode::ClipOff:
      mb->SetValue("Off");
      break;
    case vtkMRMLClipModelsNode::ClipPositiveSpace:
      mb->SetValue("Positive Space");
      break;
    case vtkMRMLClipModelsNode::ClipNegativeSpace:
      mb->SetValue("Negative Space");
      break;
    default:
      break;
    }
  
  mb = this->RedSliceClipStateMenu->GetWidget();
  switch (this->ClipModelsNode->GetRedSliceClipState())
    {
    case vtkMRMLClipModelsNode::ClipOff:
      mb->SetValue("Off");
      break;
    case vtkMRMLClipModelsNode::ClipPositiveSpace:
      mb->SetValue("Positive Space");
      break;
    case vtkMRMLClipModelsNode::ClipNegativeSpace:
      mb->SetValue("Negative Space");
      break;
    default:
      break;
    }
  
  mb = this->GreenSliceClipStateMenu->GetWidget();
  switch (this->ClipModelsNode->GetGreenSliceClipState())
    {
    case vtkMRMLClipModelsNode::ClipOff:
      mb->SetValue("Off");
      break;
    case vtkMRMLClipModelsNode::ClipPositiveSpace:
      mb->SetValue("Positive Space");
      break;
    case vtkMRMLClipModelsNode::ClipNegativeSpace:
      mb->SetValue("Negative Space");
      break;
    default:
      break;
    }
  

  mb = this->ClipTypeMenu->GetWidget();
  switch (this->ClipModelsNode->GetClipType())
    {
    case vtkMRMLClipModelsNode::ClipIntersection:
      mb->SetValue("Intersection");
      break;
    case vtkMRMLClipModelsNode::ClipUnion:
      mb->SetValue("Union");
      break;
    default:
      break;
    }
  
}

//----------------------------------------------------------------------------
void vtkSlicerClipModelsWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  // widgets?
}

