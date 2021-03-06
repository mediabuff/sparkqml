#include <QtCore>
#include <QCoreApplication>
#include <QFAppDispatcher>
#include <QQmlContext>
#include <QDir>
#include <QtShell>
#include <QDesktopServices>
#include "qmlengine.h"
#include "appview.h"

AppView::AppView(QObject *parent) : QObject(parent)
{
}

void AppView::start()
{
    Q_ASSERT(!m_mockupFolder.isEmpty());

    m_mockupActor.setFolder(m_mockupFolder);
    m_mockupActor.init();
    m_mockupActor.createProject();

    m_engine.addImportPath("qrc:///");

    if (!m_defaultImportPathFile.isEmpty() &&
        QFile::exists(m_defaultImportPathFile)) {
        qDebug().noquote() << "Default qmlimport.path: " << m_defaultImportPathFile;
        QStringList importPathList = QmlEngine::readImportPathFile(m_defaultImportPathFile);
        foreach (QString path , importPathList) {
            m_engine.addImportPath(path);
        }
    }
    m_engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    QmlEngine* qmlEngine = qobject_cast<QmlEngine*>(QFAppDispatcher::singletonObject(&m_engine,"Spark.sys",1,0,"Engine"));
    qmlEngine->setDefaultImportPathFile(m_defaultImportPathFile);
    qmlEngine->setProImportPathList(QStringList() << m_mockupFolder);

    QObject* contentItem = m_engine.rootObjects().first()->property("contentItem").value<QObject*>();
    Q_ASSERT(contentItem);

    QObject* provider = contentItem->findChild<QObject*>("provider");
    Q_ASSERT(provider);

    connect(provider, SIGNAL(openMockupProject()), this, SLOT(openMockupProject()));

    QObject* actions = contentItem->findChild<QObject*>("actions");
    Q_ASSERT(actions);

    QMetaObject::invokeMethod(actions,"startApp");

    if (!m_source.isEmpty()) {
        loadSource();
    }
}

QString AppView::source() const
{
    return m_source;
}

void AppView::setSource(const QString &source)
{
    m_source = source;
}

void AppView::loadSource()
{    
    QObject* contentItem = m_engine.rootObjects().first()->property("contentItem").value<QObject*>();
    QObject* actions = contentItem->findChild<QObject*>("actions");

    QMetaObject::invokeMethod(actions,"load",Q_ARG(QVariant, m_source));
}

QQmlApplicationEngine *AppView::engine()
{
    return &m_engine;
}

void AppView::openMockupProject()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_mockupFolder + "/sparkmockup.qmlproject"));
}

QString AppView::defaultImportPathFile() const
{
    return m_defaultImportPathFile;
}

void AppView::setDefaultImportPathFile(const QString &importPathFile)
{
    m_defaultImportPathFile = importPathFile;
}

QString AppView::mockupFolder() const
{
    return m_mockupFolder;
}

void AppView::setMockupFolder(const QString &mockupFolder)
{
    m_mockupFolder = mockupFolder;
}
