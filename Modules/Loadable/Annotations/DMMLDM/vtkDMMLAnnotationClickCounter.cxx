
// Annotation DMMLDisplayableManager includes
#include "vtkDMMLAnnotationClickCounter.h"

// VTK includes
#include <vtkObjectFactory.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkDMMLAnnotationClickCounter);

//---------------------------------------------------------------------------
void vtkDMMLAnnotationClickCounter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
vtkDMMLAnnotationClickCounter::vtkDMMLAnnotationClickCounter()
{
  this->m_Clicks = 0;
}

//---------------------------------------------------------------------------
vtkDMMLAnnotationClickCounter::~vtkDMMLAnnotationClickCounter()
{
  // TODO Auto-generated destructor stub
}

//---------------------------------------------------------------------------
void vtkDMMLAnnotationClickCounter::Reset()
{
  this->m_Clicks = 0;
}

//---------------------------------------------------------------------------
int vtkDMMLAnnotationClickCounter::Click()
{
  return ++this->m_Clicks;
}

//---------------------------------------------------------------------------
bool vtkDMMLAnnotationClickCounter::HasEnoughClicks(int clicks)
{
  this->Click();

  if (this->m_Clicks==clicks)
    {
    this->Reset();
    return true;
    }

  return false;
}
