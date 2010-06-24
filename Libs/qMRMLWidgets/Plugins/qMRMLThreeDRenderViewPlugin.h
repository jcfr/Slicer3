#ifndef __qMRMLThreeDRenderViewPlugin_h
#define __qMRMLThreeDRenderViewPlugin_h

#include "qMRMLWidgetsAbstractPlugin.h"

class QMRML_WIDGETS_PLUGIN_EXPORT qMRMLThreeDRenderViewPlugin :
  public QObject,
  public qMRMLWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qMRMLThreeDRenderViewPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString  domXml() const;
//   QIcon    icon() const;
  QString  includeFile() const;
  bool     isContainer() const;
  QString  name() const;
  
};

#endif