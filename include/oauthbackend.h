#ifndef QCLOUD_OAUTHBACKEND_H
#define QCLOUD_OAUTHBACKEND_H

#include <QtCore/QtPlugin>
#include <QtCore/QUrl>
#include <QtOAuth/QtOAuth>
#include <QtCloud/IBackend>

#include "qcloud_global.h"

namespace QCloud
{

class OAuthWidget;
class QCLOUD_EXPORT OAuthBackend : public IBackend
{
    Q_OBJECT
public:
    explicit OAuthBackend (QObject* parent = 0);
    virtual ~OAuthBackend();
    virtual void setNetworkAccessManager (QNetworkAccessManager* manager);
    virtual bool prepare();

    virtual bool requestToken();
    virtual bool authorize (QWidget* widget = 0) = 0;
    virtual void startAuth (QCloud::OAuthWidget* oauthWidget) = 0;
    virtual bool accessToken();
    virtual int error() const;

    QString appKey() const;
    void setAppKey (const QString& appkey);
    QString appSecret() const;
    void setAppSecret (const QString& appsecret);
    QString oauthToken() const;
    QString oauthTokenSecret() const;
    QString requestTokenUrl() const;
    uint timeout() const;
protected:
    QString m_appKey;
    QString m_appSecret;
    QString m_oauthToken;
    QString m_oauthTokenSecret;
    QString m_requestTokenUrl;
    QString m_authorizeUrl;
    QString m_accessTokenUrl;
    QOAuth::Interface* m_oauth;
};

}

#endif // IBACKEND_H