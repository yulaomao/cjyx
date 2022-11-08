/*=========================================================================

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/

#ifndef __vtkDMMLParser_h
#define __vtkDMMLParser_h

// DMML includes
#include "vtkDMML.h"
class vtkDMMLNode;
class vtkDMMLScene;

// VTK includes
#include "vtkXMLParser.h"
class vtkCollection;

// STD includes
#include <stack>

/// \brief Parse XML scene file.
class VTK_DMML_EXPORT vtkDMMLParser : public vtkXMLParser
{
public:
  static vtkDMMLParser *New();
  vtkTypeMacro(vtkDMMLParser,vtkXMLParser);

  vtkDMMLScene* GetDMMLScene() {return this->DMMLScene;};
  void SetDMMLScene(vtkDMMLScene* scene) {this->DMMLScene = scene;};

  vtkCollection* GetNodeCollection() {return this->NodeCollection;};
  void SetNodeCollection(vtkCollection* scene) {this->NodeCollection = scene;};

protected:
  vtkDMMLParser() = default;;
  ~vtkDMMLParser() override  = default;
  vtkDMMLParser(const vtkDMMLParser&);
  void operator=(const vtkDMMLParser&);

  void StartElement(const char* name, const char** atts) override;
  void EndElement (const char *name) override;

private:
  vtkDMMLScene* DMMLScene{nullptr};
  vtkCollection* NodeCollection{nullptr};
  std::stack< vtkDMMLNode *> NodeStack;
};

#endif
