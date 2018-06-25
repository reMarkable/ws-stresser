#include "thread.h"
#include <QNetworkRequest>
#include <QWebSocket>
#include <QAbstractEventDispatcher>
#include <QTimer>
#include <QProcess>
#include <QDir>
#include <iostream>

Thread::Thread()
{
}

void Thread::run()
{
    while (!isInterruptionRequested()) {
        QUrl url;
        url.setScheme("wss");
        url.setHost(m_hostname);
        url.setPort(443);
        url.setPath(m_path);

        QEventLoop waitLoop;
        QList<QWebSocket*> sockets;
        QNetworkRequest request(url);
        request.setRawHeader("Authorization", generateFuzz("Bearer " + m_tokens[qrand() % m_tokens.count()]));
        for (int socket = 0; socket<m_connectionCount; socket++) {
            QThread::msleep(qrand() % 200);
            QWebSocket *ws = new QWebSocket;
            sockets.append(ws);

            connect(ws, &QWebSocket::connected, &waitLoop, [=, &waitLoop](){
//                generateFuzzAndSend(ws);
                std::cout << "+" << std::flush;

                QTimer *timer = new QTimer(ws);
                timer->setInterval(m_connectionTime);
                timer->setSingleShot(true);

                connect(timer, &QTimer::timeout, &waitLoop, [=](){
                    std::cout << "=" << std::flush;
                    ws->close();
                });

                timer->start();
            });

            connect(ws, static_cast<void(QWebSocket::*)(QAbstractSocket::SocketError)>(&QWebSocket::error),
                [](QAbstractSocket::SocketError){ std::cout << "!" << std::flush;});



            connect(ws, &QWebSocket::disconnected, &waitLoop, [=, &sockets, &waitLoop](){
                sockets.removeAll(ws);
                ws->deleteLater();
                std::cout << "-" << std::flush;
                if (!sockets.count()) {
                    waitLoop.quit();
                    std::cout << "@" << std::flush;
                }
            });

            ws->open(request);
            std::cout << "." << std::flush;
        }

        waitLoop.exec();

        QThread::sleep(1);
    }
}

QByteArray Thread::generateFuzz(const QByteArray &input)
{
    QProcess radamsaProcess;
    radamsaProcess.setProgram("radamsa");
    radamsaProcess.start();
    radamsaProcess.waitForStarted();
    if (radamsaProcess.state() != QProcess::Running) {
        return QByteArray();
    }
    radamsaProcess.write(input);
    radamsaProcess.waitForBytesWritten();
    radamsaProcess.closeWriteChannel();
    radamsaProcess.setStandardInputFile(QProcess::nullDevice());
    radamsaProcess.waitForFinished();
    return radamsaProcess.readAll();
}

static inline QByteArray getFileContents(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }

    return file.readAll().trimmed();
}

void Thread::sendFuzz(QWebSocket *socket)
{
    QDir dir("datacases");
    if (!dir.exists()) {
        return;
    }

    const QFileInfoList files = dir.entryInfoList(QDir::Files);
    if (files.empty()) {
        return;
    }

    const QString filePath = files.at(qrand() % files.size()).absoluteFilePath();

    const QByteArray fileContents = getFileContents(filePath);
    if (fileContents.isEmpty() && QFileInfo(filePath).size() != 0) {
        return;
    }

    const QByteArray fuzzedContents = generateFuzz(fileContents);

    socket->sendBinaryMessage(fuzzedContents);
}
