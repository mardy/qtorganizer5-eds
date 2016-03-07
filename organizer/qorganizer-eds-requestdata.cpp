/*
 * Copyright 2013 Canonical Ltd.
 *
 * This file is part of ubuntu-pim-service.
 *
 * contact-service-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * contact-service-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "qorganizer-eds-requestdata.h"

#include <QtCore/QDebug>
#include <QtCore/QEventLoop>
#include <QtCore/QTimer>

#include <QtOrganizer/QOrganizerAbstractRequest>
#include <QtOrganizer/QOrganizerManagerEngine>

using namespace QtOrganizer;
int RequestData::m_instanceCount = 0;

RequestData::RequestData(QOrganizerEDSEngine *engine, QtOrganizer::QOrganizerAbstractRequest *req)
    : m_parent(engine),
      m_client(0),
      m_finished(false),
      m_req(req)
{
    QOrganizerManagerEngine::updateRequestState(req, QOrganizerAbstractRequest::ActiveState);
    m_cancellable = g_cancellable_new();
    m_parent->m_runningRequests.insert(req, this);
    m_instanceCount++;
}

RequestData::~RequestData()
{
    if (m_cancellable) {
        g_clear_object(&m_cancellable);
    }

    if (m_client) {
        g_clear_object(&m_client);
    }
    m_instanceCount--;
}

GCancellable* RequestData::cancellable() const
{
    return m_cancellable;
}

bool RequestData::isLive() const
{
    return (!m_req.isNull() &&
            (m_req->state() == QOrganizerAbstractRequest::ActiveState));
}

ECalClient *RequestData::client() const
{
    return E_CAL_CLIENT(m_client);
}

QOrganizerEDSEngine *RequestData::parent() const
{
    return m_parent;
}

void RequestData::cancel()
{
    if (m_cancellable) {
        g_cancellable_cancel(m_cancellable);
    }

    if (isLive()) {
        finish(QOrganizerManager::UnspecifiedError,
               QOrganizerAbstractRequest::CanceledState);
    }
}

void RequestData::wait(int msec)
{
    QMutexLocker locker(&m_waiting);
    QEventLoop *loop = new QEventLoop;
    QOrganizerAbstractRequest *req = m_req.data();
    QObject::connect(req, &QOrganizerAbstractRequest::stateChanged, [req, loop](QOrganizerAbstractRequest::State newState) {
        if (newState != QOrganizerAbstractRequest::ActiveState) {
            loop->quit();
        }
    });
    QTimer timeout;
    if (msec > 0) {
        timeout.setInterval(msec);
        timeout.setSingleShot(true);
        timeout.start();
    }
    loop->exec(QEventLoop::AllEvents|QEventLoop::WaitForMoreEvents);
    delete loop;
}

bool RequestData::isWaiting()
{
    bool result = true;
    if (m_waiting.tryLock()) {
        result = false;
        m_waiting.unlock();
    }
    return result;
}

int RequestData::instanceCount()
{
    return m_instanceCount;
}

void RequestData::deleteLater()
{
    if (isWaiting()) {
        // still running
        return;
    }
    if (!m_parent.isNull()) {
        m_parent->m_runningRequests.remove(m_req);
    }
    delete this;
}

void RequestData::finish(QOrganizerManager::Error error,
                         QOrganizerAbstractRequest::State state)
{
    Q_UNUSED(error);
    Q_UNUSED(state);
    m_finished = true;

    // When cancelling an operation the callback passed for the async function
    // will be called and the request data object will be destroyed there
    if (state != QOrganizerAbstractRequest::CanceledState) {
        deleteLater();
    }
}

void RequestData::setClient(EClient *client)
{
    if (m_client == client) {
        return;
    }
    if (m_client) {
        g_clear_object(&m_client);
    }
    if (client) {
        m_client = client;
        g_object_ref(m_client);
    }
}
