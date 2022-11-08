/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#include "qDMMLSubjectHierarchyComboBoxPlugin.h"
#include "qDMMLSubjectHierarchyComboBox.h"

//-----------------------------------------------------------------------------
qDMMLSubjectHierarchyComboBoxPlugin::qDMMLSubjectHierarchyComboBoxPlugin(QObject* pluginParent)
  : QObject(pluginParent)
{
}

//-----------------------------------------------------------------------------
QWidget *qDMMLSubjectHierarchyComboBoxPlugin::createWidget(QWidget* parentWidget)
{
  qDMMLSubjectHierarchyComboBox* pluginWidget =
    new qDMMLSubjectHierarchyComboBox(parentWidget);
  return pluginWidget;
}

//-----------------------------------------------------------------------------
QString qDMMLSubjectHierarchyComboBoxPlugin::domXml() const
{
  return "<widget class=\"qDMMLSubjectHierarchyComboBox\" \
          name=\"SubjectHierarchyComboBox\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qDMMLSubjectHierarchyComboBoxPlugin::includeFile() const
{
  return "qDMMLSubjectHierarchyComboBox.h";
}

//-----------------------------------------------------------------------------
bool qDMMLSubjectHierarchyComboBoxPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qDMMLSubjectHierarchyComboBoxPlugin::name() const
{
  return "qDMMLSubjectHierarchyComboBox";
}
