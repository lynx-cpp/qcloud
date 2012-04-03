#include "iplugin.h"

class GeneralNetworkPlugin: public QObject, public QCloud::IPlugin
{
    Q_OBJECT
    Q_INTERFACES (QCloud::IPlugin)
public:
    virtual QString name();
    virtual QString displayName();
    virtual QObject* create();
    virtual PluginCategory category();
};