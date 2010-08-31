// AnnotationModule includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationTextThreeDViewDisplayableManager.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationTextNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLAnnotationThreeDViewDisplayableManager.h"

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkTextRepresentation.h>
#include <vtkTextWidget.h>
#include <vtkRenderer.h>
#include <vtkHandleRepresentation.h>
#include <vtkAbstractWidget.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

// std includes
#include <string>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationTextThreeDViewDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLAnnotationTextThreeDViewDisplayableManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
// vtkMRMLAnnotationTextThreeDViewDisplayableManager Callback
class vtkAnnotationTextWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationTextWidgetCallback *New()
  { return new vtkAnnotationTextWidgetCallback; }

  vtkAnnotationTextWidgetCallback(){}

  virtual void Execute (vtkObject *caller, unsigned long event, void*)
  {
    if (event == vtkCommand::EndInteractionEvent)
      {

      // sanity checks
      if (!this->m_DisplayableManager)
        {
        return;
        }
      if (!this->m_Node)
        {
        return;
        }
      if (!this->m_Widget)
        {
        return;
        }
      // sanity checks end

      // the interaction with the widget ended, now propagate the changes to MRML
      this->m_DisplayableManager->PropagateWidgetToMRML(this->m_Widget, this->m_Node);

      }
  }

  void SetWidget(vtkAbstractWidget *w)
  {
    this->m_Widget = w;
  }
  void SetNode(vtkMRMLAnnotationNode *n)
  {
    this->m_Node = n;
  }
  void SetDisplayableManager(vtkMRMLAnnotationThreeDViewDisplayableManager * dm)
  {
    this->m_DisplayableManager = dm;
  }

  vtkAbstractWidget * m_Widget;
  vtkMRMLAnnotationNode * m_Node;
  vtkMRMLAnnotationThreeDViewDisplayableManager * m_DisplayableManager;
};

//---------------------------------------------------------------------------
// vtkMRMLAnnotationTextThreeDViewDisplayableManager methods

//---------------------------------------------------------------------------
void vtkMRMLAnnotationTextThreeDViewDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
/// Create a new text widget.
vtkAbstractWidget * vtkMRMLAnnotationTextThreeDViewDisplayableManager::CreateWidget(vtkMRMLAnnotationNode* node)
{
  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return 0;
    }

  if (!node)
    {
    vtkErrorMacro("CreateWidget: Node not set!")
    return 0;
    }

  vtkMRMLAnnotationTextNode* textNode = vtkMRMLAnnotationTextNode::SafeDownCast(node);

  if (!textNode)
    {
    vtkErrorMacro("CreateWidget: Could not get text node!")
    return 0;
    }

  vtkTextWidget* textWidget = vtkTextWidget::New();
  VTK_CREATE(vtkTextRepresentation, textRep);

  textRep->SetMoving(1);

  if (textNode->GetTextLabel())
    {
    textRep->SetText(textNode->GetTextLabel());
    }
  else
    {
    textRep->SetText("New text");
    }

  textWidget->SetRepresentation(textRep);

  textWidget->SetInteractor(this->GetInteractor());

  // we get display coordinates
  // we need normalized viewport coordinates, so we transform
  VTK_CREATE(vtkHandleWidget, h1);
  h1 = this->m_HandleWidgetList[0];

  double* position1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetDisplayPosition();

  double x = position1[0];
  double y = position1[1];

  this->GetRenderer()->DisplayToNormalizedDisplay(x,y);
  this->GetRenderer()->NormalizedDisplayToViewport(x,y);
  this->GetRenderer()->ViewportToNormalizedViewport(x,y);

  // we set normalized viewport coordinates
  textRep->SetPosition(x,y);

  // add the callback
  vtkAnnotationTextWidgetCallback *myCallback = vtkAnnotationTextWidgetCallback::New();
  myCallback->SetNode(textNode);
  myCallback->SetWidget(textWidget);
  myCallback->SetDisplayableManager(this);
  textWidget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  myCallback->Delete();

  textWidget->On();

  return textWidget;

}

//---------------------------------------------------------------------------
/// Tear down the widget creation
void vtkMRMLAnnotationTextThreeDViewDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node)
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  this->m_Updating = 0;
  // propagate the widget to the MRML node
  this->PropagateWidgetToMRML(widget, node);
}

//---------------------------------------------------------------------------
/// Propagate properties of MRML node to widget.
void vtkMRMLAnnotationTextThreeDViewDisplayableManager::PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget)
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  if (!widget)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Widget was null!")
    return;
    }

  if (!node)
    {
    vtkErrorMacro("PropagateMRMLToWidget: MRML node was null!")
    return;
    }

  // cast to the specific widget
  vtkTextWidget * textWidget = vtkTextWidget::SafeDownCast(widget);

  if (!textWidget)
   {
   vtkErrorMacro("PropagateMRMLToWidget: Could not get text widget!")
   return;
   }

  // cast to the specific mrml node
  vtkMRMLAnnotationTextNode* textNode = vtkMRMLAnnotationTextNode::SafeDownCast(node);

  if (!textNode)
   {
   vtkErrorMacro("PropagateMRMLToWidget: Could not get text node!")
   return;
   }

  if (this->m_Updating)
    {
    vtkDebugMacro("PropagateMRMLToWidget: Updating in progress.. Exit now.")
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;

  // if this flag is true after the checks below, the widget will be set to modified
  bool hasChanged = false;

  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  vtkTextRepresentation * rep = vtkTextRepresentation::SafeDownCast(textWidget->GetRepresentation());

  double * position1 = rep->GetPosition();

  double x = position1[0];
  double y = position1[1];

  // we have normalized viewport coordinates but we need world coordinates
  this->GetRenderer()->NormalizedViewportToViewport(x,y);
  this->GetRenderer()->ViewportToNormalizedDisplay(x,y);
  this->GetRenderer()->NormalizedDisplayToDisplay(x,y);

  double* worldCoordinates = this->GetDisplayToWorldCoordinates(x,y);
  // now we have world coordinates :)

  // Check if the MRML node has position set at all
  if (!textNode->GetTextCoordinates())
    {
    textNode->SetTextCoordinates(worldCoordinates);
    hasChanged = true;
    }

  if (!textNode->GetText(0))
    {
    textNode->SetText(0,rep->GetText(),1,1);
    hasChanged = true;
    }

  if (!textNode->GetAnnotationTextDisplayNode())
    {
    textNode->CreateAnnotationTextDisplayNode();
    hasChanged = true;
    }

  //
  // Check if the position of the widget is different than the saved one in the mrml node
  // If yes, propagate the changes to widget
  //
  if (textNode->GetTextCoordinates()[0] != worldCoordinates[0] || textNode->GetTextCoordinates()[1] != worldCoordinates[1] || textNode->GetTextCoordinates()[2] != worldCoordinates[2])
    {
    // at least one coordinate has changed, so update the widget

    // now we have to transfer again from world coordinates to normalized viewport coordinates
    double * displayCoordinates = this->GetWorldToDisplayCoordinates(textNode->GetTextCoordinates()[0], textNode->GetTextCoordinates()[1], textNode->GetTextCoordinates()[2]);

    double u = displayCoordinates[0];
    double v = displayCoordinates[1];

    this->GetRenderer()->DisplayToNormalizedDisplay(u,v);
    this->GetRenderer()->NormalizedDisplayToViewport(u,v);
    this->GetRenderer()->ViewportToNormalizedViewport(u,v);
    // now we have transformed the world coordinates in the MRML node to normalized viewport coordinates and can really update the widget

    rep->SetPosition(u,v);
    hasChanged = true;
    }

  // Propagate the MRML text label to the widget
  if (strcmp(textNode->GetText(0).c_str(),rep->GetText()))
    {
    // text in the mrml has changed, propagate to widget representation
    rep->SetText(textNode->GetText(0).c_str());
    hasChanged = true;
    }

  // Propagate the MRML text scale to the widget
  if (textNode->GetTextScale() != rep->GetTextActor()->GetScaledTextProperty()->GetFontSize())
    {
    // text scale in the mrml node is different than the font size in the widget, so propagate
    rep->GetTextActor()->GetScaledTextProperty()->SetFontSize(textNode->GetTextScale());
    hasChanged = true;
    }

  // Propagate the MRML selected text color to the widget
  if (textNode->GetAnnotationTextDisplayNode()->GetSelectedColor()[0] != rep->GetTextActor()->GetScaledTextProperty()->GetColor()[0] || textNode->GetAnnotationTextDisplayNode()->GetSelectedColor()[1] != rep->GetTextActor()->GetScaledTextProperty()->GetColor()[1] || textNode->GetAnnotationTextDisplayNode()->GetSelectedColor()[2] != rep->GetTextActor()->GetScaledTextProperty()->GetColor()[2])
    {
    // mrml selected color property is different then the widget font color, so propagate to widget
    rep->GetTextActor()->GetScaledTextProperty()->SetColor(textNode->GetAnnotationTextDisplayNode()->GetSelectedColor());
    hasChanged = true;
    }

  // Propagate the MRML unselected text color to the widget
  if (textNode->GetAnnotationTextDisplayNode()->GetColor()[0] != rep->GetTextActor()->GetScaledTextProperty()->GetColor()[0] || textNode->GetAnnotationTextDisplayNode()->GetColor()[1] != rep->GetTextActor()->GetScaledTextProperty()->GetColor()[1] || textNode->GetAnnotationTextDisplayNode()->GetColor()[2] != rep->GetTextActor()->GetScaledTextProperty()->GetColor()[2])
    {
    // mrml selected color property is different then the widget font color, so propagate to widget
    rep->GetTextActor()->GetScaledTextProperty()->SetColor(textNode->GetAnnotationTextDisplayNode()->GetColor());
    hasChanged = true;
    }

  if (hasChanged)
    {
    // at least one value has changed, so set the widget to modified
    rep->NeedToRenderOn();
    textWidget->Modified();
    }

  // enable processing of modified events
  this->m_Updating = 0;

}

//---------------------------------------------------------------------------
/// Propagate properties of widget to MRML node.
void vtkMRMLAnnotationTextThreeDViewDisplayableManager::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node)
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  if (!widget)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Widget was null!")
    return;
    }

  if (!node)
    {
    vtkErrorMacro("PropagateWidgetToMRML: MRML node was null!")
    return;
    }

  // cast to the specific widget
   vtkTextWidget * textWidget = vtkTextWidget::SafeDownCast(widget);

   if (!textWidget)
     {
     vtkErrorMacro("PropagateWidgetToMRML: Could not get text widget!")
     return;
     }

   // cast to the specific mrml node
   vtkMRMLAnnotationTextNode* textNode = vtkMRMLAnnotationTextNode::SafeDownCast(node);

   if (!textNode)
     {
     vtkErrorMacro("PropagateWidgetToMRML: Could not get text node!")
     return;
     }

  if (this->m_Updating)
   {
   vtkDebugMacro("PropagateWidgetToMRML: Updating in progress.. Exit now.")
   return;
   }

  // disable processing of modified events
  this->m_Updating = 1;

  // if this flag is true after the checks below, the modified event gets fired
  bool hasChanged = false;

  // now get the widget properties (coordinates, measurement etc.) and save it to the mrml node
  vtkTextRepresentation * rep = vtkTextRepresentation::SafeDownCast(textWidget->GetRepresentation());

  double * position1 = rep->GetPosition();

  double x = position1[0];
  double y = position1[1];

  // we have normalized viewport coordinates but we need world coordinates
  this->GetRenderer()->NormalizedViewportToViewport(x,y);
  this->GetRenderer()->ViewportToNormalizedDisplay(x,y);
  this->GetRenderer()->NormalizedDisplayToDisplay(x,y);

  double* worldCoordinates = this->GetDisplayToWorldCoordinates(x,y);
  // now we have world coordinates :)

  // Check if the MRML node has position set at all
  if (!textNode->GetTextCoordinates())
    {
    textNode->SetTextCoordinates(worldCoordinates);
    hasChanged = true;
    }

  if (!textNode->GetText(0))
    {
    textNode->SetText(0,rep->GetText(),1,1);
    hasChanged = true;
    }

  if (!textNode->GetAnnotationTextDisplayNode())
    {
    textNode->CreateAnnotationTextDisplayNode();
    hasChanged = true;
    }

  //
  // Check if the position of the widget is different than the saved one in the mrml node
  // If yes, propagate the changes to the mrml node
  //
  if (textNode->GetTextCoordinates()[0] != worldCoordinates[0] || textNode->GetTextCoordinates()[1] != worldCoordinates[1] || textNode->GetTextCoordinates()[2] != worldCoordinates[2])
    {
    // at least one coordinate has changed, so update the mrml property
    textNode->SetTextCoordinates(worldCoordinates);
    hasChanged = true;
    }

  // Propagate the widget text to the MRML node
  if (strcmp(textNode->GetText(0).c_str(),rep->GetText()))
    {
    // widget text has changed, update mrml
    textNode->SetText(0,rep->GetText(),1,1);
    hasChanged = true;
    }

  // Propagate the widget font size to the MRML node
  if (textNode->GetTextScale() != rep->GetTextActor()->GetScaledTextProperty()->GetFontSize())
    {
    // widget font is different then the mrml textScale property, so propagate to mrml
    textNode->SetTextScale(rep->GetTextActor()->GetScaledTextProperty()->GetFontSize());
    hasChanged = true;
    }

  // Propagate the selected text color to the MRML node
  if (textNode->GetAnnotationTextDisplayNode()->GetSelectedColor()[0] != rep->GetTextActor()->GetScaledTextProperty()->GetColor()[0] || textNode->GetAnnotationTextDisplayNode()->GetSelectedColor()[1] != rep->GetTextActor()->GetScaledTextProperty()->GetColor()[1] || textNode->GetAnnotationTextDisplayNode()->GetSelectedColor()[2] != rep->GetTextActor()->GetScaledTextProperty()->GetColor()[2])
    {
    // widget font color is different then the mrml selected color property, so propagate to mrml
    textNode->GetAnnotationTextDisplayNode()->SetSelectedColor(rep->GetTextActor()->GetScaledTextProperty()->GetColor());
    hasChanged = true;
    }

  // Propagate the unselected text color to the MRML node
  if (textNode->GetAnnotationTextDisplayNode()->GetColor()[0] != rep->GetTextActor()->GetScaledTextProperty()->GetColor()[0] || textNode->GetAnnotationTextDisplayNode()->GetColor()[1] != rep->GetTextActor()->GetScaledTextProperty()->GetColor()[1] || textNode->GetAnnotationTextDisplayNode()->GetColor()[2] != rep->GetTextActor()->GetScaledTextProperty()->GetColor()[2])
    {
    // widget font color is different then the mrml selected color property, so propagate to mrml
    textNode->GetAnnotationTextDisplayNode()->SetColor(rep->GetTextActor()->GetScaledTextProperty()->GetColor());
    hasChanged = true;
    }

  if (hasChanged)
    {
    // at least one value has changed, so fire the modified event
    textNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, textNode);
    }

  // enable processing of modified events
  this->m_Updating = 0;

}

//---------------------------------------------------------------------------
/// Create a annotationMRMLnode
void vtkMRMLAnnotationTextThreeDViewDisplayableManager::OnClickInThreeDRenderWindow(double x, double y)
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  // place the seed where the user clicked
  this->PlaceSeed(x,y);

  if (this->m_ClickCounter->HasEnoughClicks(1))
    {

    // switch to updating state to avoid events mess
    this->m_Updating = 1;

    double* worldCoordinates = this->GetDisplayToWorldCoordinates(x,y);

    // create the MRML node
    vtkMRMLAnnotationTextNode *textNode = vtkMRMLAnnotationTextNode::New();
    textNode->SetTextCoordinates(worldCoordinates);
    textNode->SetTextLabel("New text");

    textNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("AnnotationText"));

    textNode->Initialize(this->GetMRMLScene());

    textNode->Delete();

    // reset updating state
    this->m_Updating = 0;

    }

  }