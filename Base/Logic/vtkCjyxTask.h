#ifndef __vtkCjyxTask_h
#define __vtkCjyxTask_h

#include "vtkObject.h"
#include "vtkSmartPointer.h"
#include "vtkDMMLAbstractLogic.h"
#include "vtkCjyxBaseLogic.h"

class VTK_CJYX_BASE_LOGIC_EXPORT vtkCjyxTask : public vtkObject
{
public:
  static vtkCjyxTask *New();
  vtkTypeMacro(vtkCjyxTask,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  typedef vtkDMMLAbstractLogic::TaskFunctionPointer TaskFunctionPointer;

  ///
  /// Set the function and object to call for the task.
  void SetTaskFunction(vtkDMMLAbstractLogic*, TaskFunctionPointer, void *clientdata);

  ///
  /// Execute the task.
  virtual void Execute();

  ///
  /// The type of task - this can be used, for example, to decide
  /// how many concurrent threads should be allowed
  enum
    {
    Undefined = 0,
    Processing,
    Networking
    };

  vtkSetClampMacro (Type, int, vtkCjyxTask::Undefined, vtkCjyxTask::Networking);
  vtkGetMacro (Type, int);
  void SetTypeToProcessing() {this->SetType(vtkCjyxTask::Processing);};
  void SetTypeToNetworking() {this->SetType(vtkCjyxTask::Networking);};

  const char* GetTypeAsString( ) {
    switch (this->Type)
      {
      case vtkCjyxTask::Undefined: return "Undefined";
      case vtkCjyxTask::Processing: return "Processing";
      case vtkCjyxTask::Networking: return "Networking";
      }
    return "Unknown";
  }

protected:
  vtkCjyxTask();
  ~vtkCjyxTask() override;
  vtkCjyxTask(const vtkCjyxTask&);
  void operator=(const vtkCjyxTask&);

private:
  vtkSmartPointer<vtkDMMLAbstractLogic> TaskObject;
  vtkDMMLAbstractLogic::TaskFunctionPointer TaskFunction;
  void *TaskClientData;

  int Type;

};
#endif


