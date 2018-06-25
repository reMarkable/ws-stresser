#ifndef THREAD_H
#define THREAD_H

#include <QObject>
#include <QThread>

class QWebSocket;

class Thread : public QThread
{
    Q_OBJECT

public:
    Thread();

    void setHostname(const QString &hostname) { m_hostname = hostname; }
    void setPath(const QString &path) { m_path = path; }
    void setTokens(const QList<QByteArray> &tokens) { m_tokens = tokens; }
    void setConnectionTime(const int connectionTime) { m_connectionTime = connectionTime; }
    void setConnectionCount(const int count) { m_connectionCount = count; }

signals:
    void connected();
    void disconnected();

protected:
    virtual void run();

private:
    static QByteArray generateFuzz(const QByteArray &input);
    static void sendFuzz(QWebSocket *socket);

    QString m_hostname;
    QString m_path;
    QList<QByteArray> m_tokens;
    int m_connectionTime = 120000;
    int m_connectionCount = 100;
};

#endif // THREAD_H
