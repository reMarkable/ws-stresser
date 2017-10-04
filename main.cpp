#include <QCoreApplication>
#include <QDebug>
#include <QSettings>
#include <QFile>

#include "thread.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QSettings config("config.ini", QSettings::IniFormat);

    const QString hostname = config.value("hostname").toString();
    if (hostname.isEmpty()) {
        qWarning() << "No hostname in" << config.fileName();
        return 1;
    }

    const QString path = config.value("path").toString();
    if (path.isEmpty()) {
        qWarning() << "No path in" << config.fileName();
        return 1;
    }

    QList<QByteArray> tokens;
    QFile tokensFile("tokens.txt");
    if (!tokensFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to open" << tokensFile.fileName();
        return 1;
    }

    while (!tokensFile.atEnd()) {
        tokens.append(tokensFile.readLine().trimmed());
    }

    if (tokens.isEmpty()) {
        qWarning() << "No tokens in" << tokensFile.fileName();
        return 1;
    }

    int connectionTime = config.value("connectiontime").toInt();
    if (!connectionTime) {
        connectionTime = 60 * 20 * 1000;
        qWarning() << "No connection time in config.ini, defaulting to" << connectionTime;
    }

    int connectionCount = config.value("connectioncount").toInt();
    if (!connectionCount) {
        connectionCount = 10;
        qWarning() << "No connection count in config.ini, defaulting to" << connectionCount;
    }

    int threadCount = config.value("threadcount").toInt();
    if (!threadCount) {
        threadCount = 10;
        qWarning() << "No connection count in config.ini, defaulting to" << connectionCount;
    }
    QList<Thread*> threads;

    for (int i=0; i<threadCount; i++) {
        Thread *thread = new Thread;
        thread->setHostname(hostname);
        thread->setPath(path);
        thread->setTokens(tokens);
        thread->setConnectionTime(connectionTime);
        thread->setConnectionCount(connectionCount);
        threads.append(thread);
        thread->start();
    }

    qDebug() << "Started" << threadCount << "threads, each with" << connectionCount << "connections, total" << (threadCount * connectionCount) << "connections";
    qDebug() << "Press ctrl+c to quit";
    qDebug() << ". connecting, + connected, = disconnecting, - disconnected, ! error, @ loop finished and restarting";


    a.exec();

    return 0;
}
