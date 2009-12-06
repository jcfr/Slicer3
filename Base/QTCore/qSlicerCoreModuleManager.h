#ifndef __qSlicerCoreModuleManager_h
#define __qSlicerCoreModuleManager_h

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QObject>

#include "qSlicerBaseQTCoreWin32Header.h"

class qSlicerAbstractModule;
class qSlicerModuleFactory;

class qSlicerCoreModuleManagerPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerCoreModuleManager : public QObject
{
  Q_OBJECT
public:
  typedef qSlicerCoreModuleManager Self;
  qSlicerCoreModuleManager();
  virtual ~qSlicerCoreModuleManager(){}

  // Description:
  static qSlicerCoreModuleManager* instance();

  virtual void printAdditionalInfo();

  // Description:
  // Return a pointer to the current module factory
  qSlicerModuleFactory* factory();

  // Description:
  bool loadModule(const QString& moduleTitle);
  bool loadModuleByName(const QString& moduleName);

  // Description:
  bool unLoadModule(const QString& moduleTitle);
  bool unLoadModuleByName(const QString& moduleName);

  // Description:
  bool isLoaded(const QString& moduleTitle);

  // Description:
  qSlicerAbstractModule* getModule(const QString& moduleTitle);
  qSlicerAbstractModule* getModuleByName(const QString& moduleName);

  // Description:
  // Convenient method to get module title given its name
  QString moduleTitle(const QString& moduleName) const;

  // Description:
  // Convenient method to get module name given its title
  QString moduleName(const QString& moduleTitle) const;

signals:
  void moduleLoaded(qSlicerAbstractModule* module);
  void moduleAboutToBeUnloaded(qSlicerAbstractModule* module);

private:
  qSlicerCoreModuleManager(const qSlicerCoreModuleManager&);  // Not implemented.
  void operator=(const qSlicerCoreModuleManager&);  // Not implemented.

  QCTK_DECLARE_PRIVATE(qSlicerCoreModuleManager);
};

#endif