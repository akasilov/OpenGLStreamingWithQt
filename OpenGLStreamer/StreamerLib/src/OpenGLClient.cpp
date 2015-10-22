/**************************************************************************
**   Author: Oleksiy Kasilov, bbv Software Services AG
**   Date:   10/17/2015
**   Year:   2015
**************************************************************************/

#include "OpenGLClient.h"
#include <QDebug>
#include <QApplication>

#include <cassert>

QT_USE_NAMESPACE

OpenGLClient::OpenGLClient(const QUrl &url, bool debug, QObject *parent) :
    OpenGLProxy(debug, parent),
    mUrl(url),
    mDebug(debug),
    mOpenGLFunctionInvokers({CREATE_INVOKER(glClear),
                             CREATE_INVOKER_M(glUniformMatrix, 4, f),
                             CREATE_INVOKER(glUniform1i),
                             CREATE_INVOKER(glBindBuffer),
                             CREATE_INVOKER(glGetAttribLocation),
                             CREATE_INVOKER(glEnableVertexAttribArray),
                             CREATE_INVOKER(glVertexAttribPointer),
                             CREATE_INVOKER(glDrawElements),
                             CREATE_INVOKER(glGetUniformLocation),
                             CREATE_INVOKER(glBindTexture),
                             CREATE_INVOKER(glGetBufferSubData)
                            })
{
    if (mDebug)
        qDebug() << "WebSocket server:" << url;
    QObject::connect(&mWebSocket, &QWebSocket::connected, this, &OpenGLClient::onConnected);
    QObject::connect(&mWebSocket, &QWebSocket::disconnected, this, &OpenGLClient::onDisconnected);

    mWebSocket.open(QUrl(url));
}

void OpenGLClient::onConnected()
{
    if (mDebug)
        qDebug() << "WebSocket connected";
    QObject::connect(&mWebSocket, &QWebSocket::binaryMessageReceived,
            this, &OpenGLClient::onBinaryMessageReceived);

    //qApp->installEventFilter(this);
}

void OpenGLClient::onBinaryMessageReceived(const QByteArray &message)
{
    mMessageQueue.enqueue(message);
    if (mDebug)
        qDebug() << "Queue size:" << mMessageQueue.size();
}

void OpenGLClient::onDisconnected()
{
    //qApp->removeEventFilter(this);
}

/*bool OpenGLClient::eventFilter(QObject *obj, QEvent *ev)
{
    if (!isChild(obj, m_Obj))
        return false;

    QEvent *clonedEv = cloneEvent(ev);

    if (clonedEv)
    {
        int timeOffset;
        QDateTime curDt(QDateTime::currentDateTime());
        timeOffset = m_RecordingStartTime.daysTo(curDt) * 24 * 3600 * 1000 + m_RecordingStartTime.time().msecsTo(curDt.time());
        m_Recording.push_back(EventDelivery(timeOffset, obj, clonedEv));
    }

    return false;
}*/

void OpenGLClient::update()
{
    while (!mMessageQueue.isEmpty())
    {
        QByteArray message = mMessageQueue.takeFirst();

        QString funcName(message.data());

        if (mDebug)
            qDebug() << "OpenGL function" << funcName << "received";

        auto& invoker = mOpenGLFunctionInvokers[funcName];

        assert(invoker);

        if (invoker)
        {
            invoker->glCall(message);
        }
    }
}

bool OpenGLClient::updatedNeeded()
{
    return !mMessageQueue.isEmpty();
}

