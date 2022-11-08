/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Cjyx

=========================================================================auto=*/

#ifndef __qCjyxMarkupsModuleWidgetsPythonQtDecorators_h
#define __qCjyxMarkupsModuleWidgetsPythonQtDecorators_h

// PythonQt includes
#include <PythonQt.h>

// Cjyx includes
#include "qDMMLMarkupsOptionsWidgetsFactory.h"

#include "qCjyxMarkupsModuleWidgetsExport.h"

// NOTE:
//
// For decorators it is assumed that the methods will never be called
// with the self argument as nullptr.  The self argument is the first argument
// for non-static methods.
//

class qCjyxMarkupsModuleWidgetsPythonQtDecorators : public QObject
{
  Q_OBJECT
public:

  qCjyxMarkupsModuleWidgetsPythonQtDecorators()
    {
    //PythonQt::self()->registerClass(&qDMMLMarkupsOptionsWidgetsFactory::staticMetaObject);
    // Note: Use registerCPPClassForPythonQt to register pure Cpp classes
    }

public slots:

  //----------------------------------------------------------------------------
  // qDMMLMarkupsOptionsWidgetsFactory

  //----------------------------------------------------------------------------
  // static methods

  //----------------------------------------------------------------------------
  qDMMLMarkupsOptionsWidgetsFactory* static_qDMMLMarkupsOptionsWidgetsFactory_instance()
    {
    return qDMMLMarkupsOptionsWidgetsFactory::instance();
    }

  //----------------------------------------------------------------------------
  // instance methods

  //----------------------------------------------------------------------------
  bool registerOptionsWidget(qDMMLMarkupsOptionsWidgetsFactory* factory,
                                       PythonQtPassOwnershipToCPP<qDMMLMarkupsAbstractOptionsWidget*> plugin)
    {
    return factory->registerOptionsWidget(plugin);
    }
};

//-----------------------------------------------------------------------------
void initqCjyxMarkupsModuleWidgetsPythonQtDecorators()
{
  PythonQt::self()->addDecorators(new qCjyxMarkupsModuleWidgetsPythonQtDecorators);
}

#endif
