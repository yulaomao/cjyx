/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDMMLSliceIntersectionRepresentation2D.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or https://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

/**
 * @class   vtkDMMLSliceIntersectionRepresentation2D
 * @brief   represent intersections of other slice views in the current slice view
 *
 * @sa
 * vtkSliceIntersectionWidget vtkWidgetRepresentation vtkAbstractWidget
*/

#ifndef vtkDMMLSliceIntersectionRepresentation2D_h
#define vtkDMMLSliceIntersectionRepresentation2D_h

#include "vtkDMMLDisplayableManagerExport.h" // For export macro
#include "vtkDMMLAbstractWidgetRepresentation.h"

#include "vtkDMMLSliceNode.h"

class vtkDMMLApplicationLogic;
class vtkDMMLModelDisplayNode;
class vtkDMMLSliceLogic;

class vtkProperty2D;
class vtkActor2D;
class vtkPolyDataMapper2D;
class vtkPolyData;
class vtkPoints;
class vtkCellArray;
class vtkTextProperty;
class vtkLeaderActor2D;
class vtkTextMapper;
class vtkTransform;
class vtkActor2D;

class SliceIntersectionDisplayPipeline;


class VTK_DMML_DISPLAYABLEMANAGER_EXPORT vtkDMMLSliceIntersectionRepresentation2D : public vtkDMMLAbstractWidgetRepresentation
{
public:
  /**
   * Instantiate this class.
   */
  static vtkDMMLSliceIntersectionRepresentation2D *New();

  //@{
  /**
   * Standard methods for instances of this class.
   */
  vtkTypeMacro(vtkDMMLSliceIntersectionRepresentation2D, vtkDMMLAbstractWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  //@}

  void SetSliceNode(vtkDMMLSliceNode* sliceNode);
  vtkDMMLSliceNode* GetSliceNode();

  void AddIntersectingSliceLogic(vtkDMMLSliceLogic* sliceLogic);
  void RemoveIntersectingSliceNode(vtkDMMLSliceNode* sliceNode);
  void UpdateIntersectingSliceNodes();
  void RemoveAllIntersectingSliceNodes();

  //@{
  /**
   * Methods to make this class behave as a vtkProp.
   */
  void GetActors2D(vtkPropCollection *) override;
  void ReleaseGraphicsResources(vtkWindow *) override;
  int RenderOverlay(vtkViewport *viewport) override;
  //@}

  void SetDMMLApplicationLogic(vtkDMMLApplicationLogic*);
  vtkGetObjectMacro(DMMLApplicationLogic, vtkDMMLApplicationLogic);

  double* GetSliceIntersectionPoint();

  void TransformIntersectingSlices(vtkMatrix4x4* rotatedSliceToSliceTransformMatrix);

protected:
  vtkDMMLSliceIntersectionRepresentation2D();
  ~vtkDMMLSliceIntersectionRepresentation2D() override;

  SliceIntersectionDisplayPipeline* GetDisplayPipelineFromSliceLogic(vtkDMMLSliceLogic* sliceLogic);

  static void SliceNodeModifiedCallback(vtkObject* caller, unsigned long eid, void* clientData, void* callData);
  void SliceNodeModified(vtkDMMLSliceNode* sliceNode);
  void SliceModelDisplayNodeModified(vtkDMMLModelDisplayNode* sliceNode);

  void UpdateSliceIntersectionDisplay(SliceIntersectionDisplayPipeline *pipeline);

  double GetSliceRotationAngleRad(int eventPos[2]);

  // The internal transformation matrix
  vtkTransform *CurrentTransform;
  vtkTransform *TotalTransform;
  double Origin[4]; //the current origin in world coordinates
  double DisplayOrigin[3]; //the current origin in display coordinates
  double CurrentTranslation[3]; //translation this movement
  double StartWorldPosition[4]; //Start event position converted to world

  // Support picking
  double LastEventPosition[2];

  // Slice intersection point in XY
  double SliceIntersectionPoint[4];

  vtkDMMLApplicationLogic* DMMLApplicationLogic;

  class vtkInternal;
  vtkInternal * Internal;

private:
  vtkDMMLSliceIntersectionRepresentation2D(const vtkDMMLSliceIntersectionRepresentation2D&) = delete;
  void operator=(const vtkDMMLSliceIntersectionRepresentation2D&) = delete;
};

#endif
