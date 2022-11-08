/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qCjyxAbstractModule.h"

// STD includes

class AModule: public qCjyxAbstractModule
{
public:
  QString title()const override { return "A Title";}
  qCjyxAbstractModuleRepresentation* createWidgetRepresentation() override
  {
    return nullptr;
  }

  vtkDMMLAbstractLogic* createLogic() override
  {
    return nullptr;
  }
protected:
  void setup () override {}
};

int qCjyxAbstractModuleTest1(int, char * [] )
{
  AModule module;

  return EXIT_SUCCESS;
}

