#include "thread.h"
#include <QNetworkRequest>
#include <QWebSocket>
#include <QAbstractEventDispatcher>
#include <QTimer>
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
        request.setRawHeader("Authorization", "Bearer " + m_tokens[qrand() % m_tokens.count()]);
        for (int socket = 0; socket<m_connectionCount; socket++) {
            QThread::msleep(qrand() % 200);
            QWebSocket *ws = new QWebSocket;
            sockets.append(ws);

            connect(ws, &QWebSocket::connected, &waitLoop, [=, &sockets, &waitLoop](){
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
