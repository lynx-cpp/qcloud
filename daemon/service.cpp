#include "service.h"
#include "appmanager.h"
#include "factory.h"
#include "ibackend.h"
#include "daemon.h"
#include "accountmanager.h"
#include "account.h"
#include "request.h"
#include "entryinfo.h"
#include <QFileInfo>
#include <QTemporaryFile>

#include <QDebug>

Service::Service (Daemon* daemon) : Server (daemon)
    ,m_daemon(daemon)
{
    currentRequestId = 0;
    qDebug() << "Service : The SecureStore Pointer is " << m_daemon->secureStore();
    encrypter = new QCloud::Encrypter(m_daemon->secureStore());
}

Service::~Service()
{

}

void Service::addAccount (const QString& backendName, const QString& appName)
{
    QCloud::App* app = QCloud::AppManager::instance()->app(appName);
    if (!app)
        return;
    QCloud::IBackend* backend = QCloud::Factory::instance()->createBackend(backendName);
    if (backend) {
        backend->setApp(app);
        backend->setNetworkAccessManager(m_daemon->createNetwork());
        if (backend->authorize()) {
            m_daemon->accountManager()->addAccount(backend);
            notifyAccountUpdated();
        }
        else
            delete backend;
    }
}

void Service::deleteAccount(const QString& strid)
{
    QUuid uuid(strid);
    qDebug() << "Ask accountManager to delete uuid : " << uuid;
    if (m_daemon->accountManager()->deleteAccount(uuid)) {
        qDebug() << uuid << " deleted";
        notifyAccountUpdated();
    }
}

QCloud::InfoList Service::listApps()
{
    QCloud::InfoList list;
    QList<QCloud::App*> appList(QCloud::AppManager::instance()->appList());
    foreach(QCloud::App* app, appList)
    {
        list << *app;
    }
    return list;
}

QCloud::InfoList Service::listBackends()
{
    QCloud::InfoList list;
    QList<QCloud::IPlugin*> backenList(QCloud::Factory::instance()->backendList());
    foreach(QCloud::IPlugin* backend, backenList)
    {
        list << *backend;
    }
    return list;
}


int Service::sync (const QString& app_name)
{
    return 0;
}

int Service::uploadFile (const QString& uuid, const QString& file, uint type, const QString& dest)
{
    Account *account = m_daemon->accountManager()->findAccount(uuid);
    if (!account)
        return -1;
    UploadRequestHandler* requestHandler = new UploadRequestHandler(currentRequestId, this);
    
    QTemporaryFile* tmpFile = new QTemporaryFile();
    
    if (!tmpFile->open())
        return -1;
    tmpFile->close();
    QString fromFile = file;
    if (account->app()->fileEncrypted()){
        encrypter->encrypt(file,tmpFile->fileName());
        qDebug() << "Encryption Finished";
        fromFile = tmpFile->fileName();
    }
    requestHandler->setTmpFile(tmpFile);
    
    QCloud::Request* request = account->backend()->uploadFile(tmpFile->fileName(), type, dest);
    requestHandler->setRequest(request);
    connect(requestHandler, SIGNAL(uploadProgress(int,qint64, qint64)), SLOT(notifyUploadProgress(int,qint64, qint64)));
    return currentRequestId ++;
}


int Service::downloadFile (const QString& uuid, const QString& src, const QString& file, uint type)
{
    Account *account = m_daemon->accountManager()->findAccount(uuid);
    if (!account)
        return -1;
    DownloadRequestHandler* requestHandler = new DownloadRequestHandler(currentRequestId, this, m_daemon->secureStore());
    
    QTemporaryFile* tmpFile = new QTemporaryFile();
    if (!tmpFile->open()){
        qDebug() << "Cannot Create Temp File!";
        return -1;
    }
    tmpFile->close();
    QString dest = file;
    requestHandler->setTmpFile(NULL);
    if (account->app()->fileEncrypted()){
        dest = tmpFile->fileName();
        requestHandler->setTmpFile(tmpFile);
        requestHandler->setDestFile(file);
    }
    
    qDebug() << "Now Downloading " << src << " " << dest;
    QCloud::Request* request = account->backend()->downloadFile(src, dest, type);
    requestHandler->setRequest(request);
    connect(requestHandler, SIGNAL(downloadProgress(int,qint64, qint64)), SLOT(notifyDownloadProgress(int,qint64, qint64)));
    return currentRequestId ++;
}

QCloud::InfoList Service::listAccounts()
{
    QCloud::InfoList infoList;
    QList< Account* > list = m_daemon->accountManager()->listAccounts();
    foreach(Account* account, list) {
        QCloud::Info info;
        info.setName(account->uuid());
        info.setDescription(QString("%1 - %2").arg(account->backend()->info().description()).arg(account->app()->description()));
        info.setIconName(account->backend()->info().iconName());
        info.setDisplayName(account->userName());

        infoList << info;
        qDebug() << "UUID : " << account->uuid() << "userName : " << account->userName();
    }
    return infoList;
}

int Service::listFiles(const QString& uuid,const QString& directory)
{
    Account *account = m_daemon->accountManager()->findAccount(uuid);
    if (!account)
        return -1;
    ListFilesRequestHandler* requestHandler = new ListFilesRequestHandler(currentRequestId, this);
    qDebug() << account->backend()->userName();
    QCloud::Request* request = account->backend()->pathInfo(directory,&requestHandler->entryInfo,&requestHandler->entryInfoList);
    requestHandler->setRequest(request);
    return currentRequestId ++;
}

int Service::createFolder (const QString& uuid, const QString& directory)
{
    Account *account = m_daemon->accountManager()->findAccount(uuid);
    if (!account)
        return -1;
    GeneralRequestHandler* requestHandler = new GeneralRequestHandler(currentRequestId, this);
    qDebug() << account->backend()->userName();
    QCloud::Request* request = account->backend()->createFolder(directory);
    requestHandler->setRequest(request);
    return currentRequestId ++;
}

int Service::deleteFile (const QString& uuid, const QString& path)
{
    Account *account = m_daemon->accountManager()->findAccount(uuid);
    if (!account)
        return -1;
    GeneralRequestHandler* requestHandler = new GeneralRequestHandler(currentRequestId, this);
    qDebug() << account->backend()->userName();
    QCloud::Request* request = account->backend()->deleteFile(path);
    requestHandler->setRequest(request);
    return currentRequestId ++;
}

int Service::moveFile (const QString& uuid, const QString& src, const QString& dst)
{
    Account *account = m_daemon->accountManager()->findAccount(uuid);
    if (!account)
        return -1;
    GeneralRequestHandler* requestHandler = new GeneralRequestHandler(currentRequestId, this);
    qDebug() << account->backend()->userName();
    QCloud::Request* request = account->backend()->moveFile(src, dst);
    requestHandler->setRequest(request);
    return currentRequestId ++;
}

int Service::copyFile (const QString& uuid, const QString& src, const QString& dst)
{
    Account *account = m_daemon->accountManager()->findAccount(uuid);
    if (!account)
        return -1;
    GeneralRequestHandler* requestHandler = new GeneralRequestHandler(currentRequestId, this);
    qDebug() << account->backend()->userName();
    QCloud::Request* request = account->backend()->copyFile(src, dst);
    requestHandler->setRequest(request);
    return currentRequestId ++;
}

int Service::fetchInfo (const QString& uuid, const QString& path)
{
    Account *account = m_daemon->accountManager()->findAccount(uuid);
    if (!account)
        return -1;
    FileInfoRequestHandler* requestHandler = new FileInfoRequestHandler(currentRequestId, this);
    qDebug() << account->backend()->userName();
    QCloud::Request* request = account->backend()->pathInfo(path, &requestHandler->entryInfo);
    requestHandler->setRequest(request);
    return currentRequestId ++;
}

ListFilesRequestHandler::ListFilesRequestHandler(int id, QCloud::Server* server) : RequestHandler(server)
{
    m_id = id;
    m_server = server;
}


void ListFilesRequestHandler::requestFinished()
{
    qDebug() << "Sending finished signal..." << m_id << m_request->metaObject()->className();

    m_server->notifyDirectoryInfoTransformed(m_id, m_request->error(), entryInfoList);
    delete this;
}

ListFilesRequestHandler::~ListFilesRequestHandler()
{

}


FileInfoRequestHandler::FileInfoRequestHandler(int id, QCloud::Server* server) : RequestHandler(server)
{
    m_id = id;
    m_server = server;
}


void FileInfoRequestHandler::requestFinished()
{
    qDebug() << "Sending finished signal..." << m_id << m_request->metaObject()->className();

    m_server->notifyFileInfoTransformed(m_id, m_request->error(), entryInfo);
    delete this;
}

FileInfoRequestHandler::~FileInfoRequestHandler()
{

}


GeneralRequestHandler::GeneralRequestHandler(int id, QCloud::Server* server) : RequestHandler(server)
{
    m_id = id;
    m_server = server;
}
GeneralRequestHandler::~GeneralRequestHandler()
{

}


void GeneralRequestHandler::requestFinished()
{
    qDebug() << "Sending finished signal..." << m_id << m_request->metaObject()->className();

    m_server->notifyRequestFinished(m_id, m_request->error());
    delete this;
}

void UploadRequestHandler::requestFinished()
{
    delete m_tmpFile;
    GeneralRequestHandler::requestFinished();
}

void UploadRequestHandler::setTmpFile(QTemporaryFile* tmpFile)
{
    m_tmpFile = tmpFile;
}

UploadRequestHandler::UploadRequestHandler(int id, QCloud::Server* server): GeneralRequestHandler(id, server)
{
    m_id = id;
    m_server = server;
}

UploadRequestHandler::~UploadRequestHandler()
{

}

DownloadRequestHandler::DownloadRequestHandler(int id, QCloud::Server* server, QCloud::ISecureStore* secureStore): GeneralRequestHandler(id, server)
{
    m_id = id;
    m_server = server;
    encrypter = new QCloud::Encrypter(secureStore);
}

void DownloadRequestHandler::requestFinished()
{
    if (m_tmpFile!=NULL){
        qDebug() << "Tmp file Not Null , now decrypting...";
        encrypter->decrypt(m_tmpFile->fileName(),m_dest);
        delete m_tmpFile;
    }
    else
        qDebug() << "NULL tmp file, download finished.";
    GeneralRequestHandler::requestFinished();
}

void DownloadRequestHandler::setDestFile(const QString& dest)
{
    m_dest = dest;
}

void DownloadRequestHandler::setTmpFile(QTemporaryFile* tmpFile)
{
    m_tmpFile = tmpFile;
}

DownloadRequestHandler::~DownloadRequestHandler()
{

}

