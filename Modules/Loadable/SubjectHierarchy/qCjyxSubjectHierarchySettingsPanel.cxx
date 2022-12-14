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

// Qt includes
#include <QSettings>

// QtGUI includes
#include "qCjyxApplication.h"
#include "qCjyxSubjectHierarchySettingsPanel.h"
#include "ui_qCjyxSubjectHierarchySettingsPanel.h"

// --------------------------------------------------------------------------
// qCjyxSubjectHierarchySettingsPanelPrivate

//-----------------------------------------------------------------------------
class qCjyxSubjectHierarchySettingsPanelPrivate: public Ui_qCjyxSubjectHierarchySettingsPanel
{
  Q_DECLARE_PUBLIC(qCjyxSubjectHierarchySettingsPanel);
protected:
  qCjyxSubjectHierarchySettingsPanel* const q_ptr;

public:
  qCjyxSubjectHierarchySettingsPanelPrivate(qCjyxSubjectHierarchySettingsPanel& object);
  void init();
};

// --------------------------------------------------------------------------
// qCjyxSubjectHierarchySettingsPanelPrivate methods

// --------------------------------------------------------------------------
qCjyxSubjectHierarchySettingsPanelPrivate
::qCjyxSubjectHierarchySettingsPanelPrivate(qCjyxSubjectHierarchySettingsPanel& object)
  :q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qCjyxSubjectHierarchySettingsPanelPrivate::init()
{
  Q_Q(qCjyxSubjectHierarchySettingsPanel);

  this->setupUi(q);

  // Default values
  this->AutoDeleteSubjectHierarchyChildrenEnabledCheckBox->setChecked(false);

  this->PatientIDTagCheckBox->setChecked(true);
  this->PatientBirthDateTagCheckBox->setChecked(false);
  this->StudyIDTagCheckBox->setChecked(false);
  this->StudyDateTagCheckBox->setChecked(true);

  // Register settings
  q->registerProperty("SubjectHierarchy/AutoDeleteSubjectHierarchyChildren", this->AutoDeleteSubjectHierarchyChildrenEnabledCheckBox,
                      "checked", SIGNAL(toggled(bool)),
                      "Enable/disable automatic subject hierarchy children deletion", ctkSettingsPanel::OptionNone);

  q->registerProperty("SubjectHierarchy/DisplayPatientIDInSubjectHierarchyItemName", this->PatientIDTagCheckBox,
                      "checked", SIGNAL(toggled(bool)),
                      "Include patient ID in the name of the patient item", ctkSettingsPanel::OptionNone);
  q->registerProperty("SubjectHierarchy/DisplayPatientBirthDateInSubjectHierarchyItemName", this->PatientBirthDateTagCheckBox,
                      "checked", SIGNAL(toggled(bool)),
                      "Include patient birth date in the name of the patient item", ctkSettingsPanel::OptionNone);
  q->registerProperty("SubjectHierarchy/DisplayStudyIDInSubjectHierarchyItemName", this->StudyIDTagCheckBox,
                      "checked", SIGNAL(toggled(bool)),
                      "Include study ID in the name of the study item", ctkSettingsPanel::OptionNone);
  q->registerProperty("SubjectHierarchy/DisplayStudyDateInSubjectHierarchyItemName", this->StudyDateTagCheckBox,
                      "checked", SIGNAL(toggled(bool)),
                      "Include study date in the name of the study item", ctkSettingsPanel::OptionNone);

  // Actions to propagate to the application when settings are changed
  QObject::connect(this->AutoDeleteSubjectHierarchyChildrenEnabledCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setAutoDeleteSubjectHierarchyChildrenEnabled(bool)));

  QObject::connect(this->PatientIDTagCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setDisplayPatientIDEnabled(bool)));
  QObject::connect(this->PatientBirthDateTagCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setDisplayPatientBirthDateEnabled(bool)));
  QObject::connect(this->StudyIDTagCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setDisplayStudyIDEnabled(bool)));
  QObject::connect(this->StudyDateTagCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setDisplayStudyDateEnabled(bool)));
}

// --------------------------------------------------------------------------
// qCjyxSubjectHierarchySettingsPanel methods

// --------------------------------------------------------------------------
qCjyxSubjectHierarchySettingsPanel::qCjyxSubjectHierarchySettingsPanel(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qCjyxSubjectHierarchySettingsPanelPrivate(*this))
{
  Q_D(qCjyxSubjectHierarchySettingsPanel);
  d->init();
}

// --------------------------------------------------------------------------
qCjyxSubjectHierarchySettingsPanel::~qCjyxSubjectHierarchySettingsPanel() = default;

// --------------------------------------------------------------------------
void qCjyxSubjectHierarchySettingsPanel::setAutoDeleteSubjectHierarchyChildrenEnabled(bool on)
{
  Q_UNUSED(on);
}

// --------------------------------------------------------------------------
void qCjyxSubjectHierarchySettingsPanel::setDisplayPatientIDEnabled(bool on)
{
  Q_UNUSED(on);
}

// --------------------------------------------------------------------------
void qCjyxSubjectHierarchySettingsPanel::setDisplayPatientBirthDateEnabled(bool on)
{
  Q_UNUSED(on);
}

// --------------------------------------------------------------------------
void qCjyxSubjectHierarchySettingsPanel::setDisplayStudyIDEnabled(bool on)
{
  Q_UNUSED(on);
}

// --------------------------------------------------------------------------
void qCjyxSubjectHierarchySettingsPanel::setDisplayStudyDateEnabled(bool on)
{
  Q_UNUSED(on);
}
