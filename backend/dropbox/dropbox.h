#ifndef DROPBOX_H
#define DROPBOX_H

#include <QFile>
#include "oauthbackend.h"

class DropboxRequest;

namespace QCloud
{
class App;
class Request;
}

class Dropbox : public QCloud::OAuthBackend
{
    friend class DropboxRequest;
    Q_OBJECT
public:
    explicit Dropbox (QObject* parent = 0);
    virtual ~Dropbox();
    virtual void setApp (QCloud::App* app);
    virtual bool authorize (QWidget* parent = 0);
    virtual QCloud::Request* uploadFile (const QString& localFileName, const QString& remoteFilePath);
    virtual QCloud::Request* downloadFile (const QString& remoteFilePath,const QString& localFileName);
    virtual QCloud::Request* copyFile (const QString& fromPath,const QString& toPath);
    virtual QCloud::Request* moveFile (const QString& fromPath,const QString& toPath);
    virtual QCloud::Request* createFolder (const QString& path);
    virtual QCloud::Request* deleteFile (const QString& path);
    virtual QCloud::Request* getInfo (const QString& path);
    virtual void startAuth (QCloud::OAuthWidget* widget);
    virtual void loadAccountInfo (const QString& key, QSettings& settings, QCloud::ISecureStore* securestore);
    virtual void saveAccountInfo (const QString& key, QSettings& settings, QCloud::ISecureStore* securestore);
    virtual QString userName();

protected:
    QByteArray m_uid;
    bool m_globalAccess;
    QString m_userName;
};

#endif
