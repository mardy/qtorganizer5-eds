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

#include "qorganizer-eds-saverequestdata.h"
#include "qorganizer-eds-enginedata.h"

#include <QtOrganizer/QOrganizerManagerEngine>
#include <QtOrganizer/QOrganizerItemSaveRequest>

using namespace QtOrganizer;

SaveRequestData::SaveRequestData(QOrganizerEDSEngine *engine,
                                 QtOrganizer::QOrganizerAbstractRequest *req)
    : RequestData(engine, req)
{
    // map items by collection
    Q_FOREACH(QOrganizerItem i, request<QOrganizerItemSaveRequest>()->items()) {
        QString collectionId = i.collectionId().toString();
        QList<QOrganizerItem> li = m_items[collectionId];
        li << i;
        m_items.insert(collectionId, li);
    }
}

SaveRequestData::~SaveRequestData()
{
}

void SaveRequestData::finish(QtOrganizer::QOrganizerManager::Error error)
{
    QOrganizerManagerEngine::updateItemSaveRequest(request<QOrganizerItemSaveRequest>(),
                                                   m_result,
                                                   error,
                                                   m_erros,
                                                   QOrganizerAbstractRequest::FinishedState);
    Q_FOREACH(QOrganizerItem item, m_result) {
        m_changeSet.insertAddedItem(item.id());
    }
    emitChangeset(&m_changeSet);
}

void SaveRequestData::appendResults(QList<QOrganizerItem> result)
{
    m_result += result;
}

QString SaveRequestData::nextCollection()
{
    if (m_items.isEmpty()) {
        m_currentCollection = QString();
        m_currentItems.clear();
    } else {
        m_currentCollection = m_items.keys().first();
        m_currentItems = m_items.take(m_currentCollection);
    }
    m_workingItems.clear();
    return m_currentCollection;
}

QString SaveRequestData::currentCollection() const
{
    return m_currentCollection;
}

QList<QOrganizerItem> SaveRequestData::takeItemsToCreate()
{
    QList<QOrganizerItem> result;

    Q_FOREACH(const QOrganizerItem &i, m_currentItems) {
        if (i.id().isNull()) {
            result << i;
            m_currentItems.removeAll(i);
        }
    }
    return result;
}

QList<QOrganizerItem> SaveRequestData::takeItemsToUpdate()
{
    QList<QOrganizerItem> result;

    Q_FOREACH(const QOrganizerItem &i, m_currentItems) {
        if (!i.id().isNull()) {
            result << i;
            m_currentItems.removeAll(i);
        }
    }
    return result;
}

bool SaveRequestData::end() const
{
    return m_items.isEmpty();
}

void SaveRequestData::appendResult(const QOrganizerItem &item, QOrganizerManager::Error error)
{
    if (error != QOrganizerManager::NoError) {
        int index = request<QOrganizerItemSaveRequest>()->items().indexOf(item);
        if (index != -1) {
            m_erros.insert(index, error);
        }
    } else {
        m_result << item;
    }
}


void SaveRequestData::setWorkingItems(QList<QOrganizerItem> items)
{
    m_workingItems = items;
}

QList<QOrganizerItem> SaveRequestData::workingItems() const
{
    return m_workingItems;
}
