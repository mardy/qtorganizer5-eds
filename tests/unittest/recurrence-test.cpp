/*
 * Copyright 2013 Canonical Ltd.
 *
 * This file is part of qtorganizer5-eds.
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


#include <QObject>
#include <QtTest>
#include <QDebug>

#include <QtOrganizer>

#include "qorganizer-eds-engine.h"
#include "eds-base-test.h"


using namespace QtOrganizer;

class RecurrenceTest : public QObject, public EDSBaseTest
{
    Q_OBJECT
private:
    static const QString defaultCollectionName;

    QOrganizerEDSEngine *m_engine;
    QOrganizerCollection m_collection;

    QOrganizerItem createTestEvent()
    {
        static QString displayLabelValue = QStringLiteral("Recurrence event test");
        static QString descriptionValue = QStringLiteral("Recucurrence event description");

        QOrganizerEvent ev;
        ev.setCollectionId(m_collection.id());
        ev.setStartDateTime(QDateTime(QDate(2013, 12, 2), QTime(0,0,0), QTimeZone("America/Recife")));
        ev.setEndDateTime(QDateTime(QDate(2013, 12, 2), QTime(0,30,0), QTimeZone("America/Recife")));
        ev.setDisplayLabel(displayLabelValue);
        ev.setDescription(descriptionValue);

        QOrganizerRecurrenceRule rule;
        rule.setFrequency(QOrganizerRecurrenceRule::Weekly);
        rule.setDaysOfWeek(QSet<Qt::DayOfWeek>() << Qt::Monday);
        rule.setLimit(QDate(2013, 12, 31));
        ev.setRecurrenceRule(rule);

        QtOrganizer::QOrganizerManager::Error error;
        QMap<int, QtOrganizer::QOrganizerManager::Error> errorMap;
        QList<QOrganizerItem> items;
        items << ev;
        bool saveResult = m_engine->saveItems(&items,
                                              QList<QtOrganizer::QOrganizerItemDetail::DetailType>(),
                                              &errorMap,
                                              &error);
        if (!saveResult) {
            qWarning() << "Fail to save items";
        }

        Q_ASSERT(saveResult);
        Q_ASSERT(error == QtOrganizer::QOrganizerManager::NoError);

        // append new item to be removed after the test
        appendToRemove(items[0].id());

        return items[0];
    }

private Q_SLOTS:
    void init()
    {
        EDSBaseTest::init();

        m_engine = QOrganizerEDSEngine::createEDSEngine(QMap<QString, QString>());

        QtOrganizer::QOrganizerManager::Error error;
        m_collection = QOrganizerCollection();
        m_collection.setMetaData(QOrganizerCollection::KeyName, defaultCollectionName);

        bool saveResult = m_engine->saveCollection(&m_collection, &error);
        QVERIFY(saveResult);
        QCOMPARE(error, QtOrganizer::QOrganizerManager::NoError);
    }

    void cleanup()
    {
        delete m_engine;
        m_engine = 0;
        EDSBaseTest::cleanup();
    }

    void testCreateDailyEvent()
    {
        static QString displayLabelValue = QStringLiteral("Daily test");
        static QString descriptionValue = QStringLiteral("Daily description");

        QOrganizerEvent ev;
        ev.setCollectionId(m_collection.id());
        ev.setStartDateTime(QDateTime(QDate(2013, 12, 2), QTime(0,0,0)));
        ev.setEndDateTime(QDateTime(QDate(2013, 12, 2), QTime(0,30,0)));
        ev.setDisplayLabel(displayLabelValue);
        ev.setDescription(descriptionValue);

        QOrganizerRecurrenceRule rule;
        rule.setFrequency(QOrganizerRecurrenceRule::Daily);
        rule.setLimit(QDate(2013, 12, 31));
        ev.setRecurrenceRule(rule);

        QSignalSpy itemsAdded(m_engine, SIGNAL(itemsAdded(QList<QOrganizerItemId>)));
        QtOrganizer::QOrganizerManager::Error error;
        QMap<int, QtOrganizer::QOrganizerManager::Error> errorMap;
        QList<QOrganizerItem> items;
        items << ev;
        bool saveResult = m_engine->saveItems(&items,
                                              QList<QtOrganizer::QOrganizerItemDetail::DetailType>(),
                                              &errorMap,
                                              &error);

        QVERIFY(saveResult);
        QCOMPARE(error, QtOrganizer::QOrganizerManager::NoError);
        QTRY_COMPARE(itemsAdded.count(), 1);
    }

    void testCreateWeeklyEvent()
    {
        static QString displayLabelValue = QStringLiteral("Weekly test");
        static QString descriptionValue = QStringLiteral("Weekly description");

        QOrganizerEvent ev;
        ev.setCollectionId(m_collection.id());
        ev.setStartDateTime(QDateTime(QDate(2013, 12, 2), QTime(0,0,0)));
        ev.setEndDateTime(QDateTime(QDate(2013, 12, 2), QTime(0,30,0)));
        ev.setDisplayLabel(displayLabelValue);
        ev.setDescription(descriptionValue);

        QOrganizerRecurrenceRule rule;
        rule.setFrequency(QOrganizerRecurrenceRule::Weekly);
        rule.setDaysOfWeek(QSet<Qt::DayOfWeek>() << Qt::Monday);
        rule.setLimit(QDate(2013, 12, 31));
        ev.setRecurrenceRule(rule);

        QtOrganizer::QOrganizerManager::Error error;
        QMap<int, QtOrganizer::QOrganizerManager::Error> errorMap;
        QList<QOrganizerItem> items;
        items << ev;
        bool saveResult = m_engine->saveItems(&items,
                                              QList<QtOrganizer::QOrganizerItemDetail::DetailType>(),
                                              &errorMap,
                                              &error);
        QVERIFY(saveResult);
        QCOMPARE(error, QtOrganizer::QOrganizerManager::NoError);

        // append new item to be removed after the test
        QOrganizerItemId parentId = items[0].id();
        appendToRemove(parentId);

        QOrganizerItemSortOrder sort;
        QOrganizerItemFetchHint hint;
        QOrganizerItemFilter filter;
        items = m_engine->items(filter,
                                QDateTime(QDate(2013, 11, 30), QTime(0,0,0)),
                                QDateTime(QDate(2015, 1, 1), QTime(0,0,0)),
                                100,
                                sort,
                                hint,
                                &error);
        QCOMPARE(items.count(), 5);

        QList<QDateTime> expectedDates;
        expectedDates << QDateTime(QDate(2013, 12, 2), QTime(0,0,0))
                      << QDateTime(QDate(2013, 12, 9), QTime(0,0,0))
                      << QDateTime(QDate(2013, 12, 16), QTime(0,0,0))
                      << QDateTime(QDate(2013, 12, 23), QTime(0,0,0))
                      << QDateTime(QDate(2013, 12, 30), QTime(0,0,0));


        for(int i=0; i < 5; i++) {
            QOrganizerItemParent itemParent = items[i].detail(QOrganizerItemDetail::TypeParent);
            QOrganizerEventTime time = items[i].detail(QOrganizerItemDetail::TypeEventTime);
            QCOMPARE(itemParent.parentId(), parentId);
            QCOMPARE(time.startDateTime(), expectedDates[i]);
        }
    }

    void testCreateMonthlyEvent()
    {
        static QString displayLabelValue = QStringLiteral("Monthly test");
        static QString descriptionValue = QStringLiteral("Monthly description");
        static QDateTime eventStartDate = QDateTime(QDate(2013, 1, 1), QTime(0, 0, 0), Qt::UTC);
        static QDateTime eventEndDate = QDateTime(QDate(2013, 1, 1), QTime(0, 30, 0), Qt::UTC);

        QOrganizerEvent ev;
        ev.setCollectionId(m_collection.id());
        ev.setStartDateTime(eventStartDate);
        ev.setEndDateTime(eventEndDate);
        ev.setDisplayLabel(displayLabelValue);
        ev.setDescription(descriptionValue);

        QOrganizerRecurrenceRule rule;
        rule.setFrequency(QOrganizerRecurrenceRule::Monthly);
        rule.setDaysOfMonth(QSet<int>() << 1 << 5);
        rule.setLimit(QDate(2013, 12, 31));
        ev.setRecurrenceRule(rule);

        QtOrganizer::QOrganizerManager::Error error;
        QMap<int, QtOrganizer::QOrganizerManager::Error> errorMap;
        QList<QOrganizerItem> items;
        items << ev;
        bool saveResult = m_engine->saveItems(&items,
                                              QList<QtOrganizer::QOrganizerItemDetail::DetailType>(),
                                              &errorMap,
                                              &error);
        QVERIFY(saveResult);
        QCOMPARE(error, QtOrganizer::QOrganizerManager::NoError);

        // append new item to be removed after the test
        appendToRemove(items[0].id());

        QOrganizerItemSortOrder sort;
        QOrganizerItemFetchHint hint;
        QOrganizerItemFilter filter;
        items = m_engine->items(filter,
                                eventStartDate,
                                eventStartDate.addYears(1),
                                100,
                                sort,
                                hint,
                                &error);
        QCOMPARE(items.count(), 24);
        for(int i=0; i < 12; i++) {
            QOrganizerEventTime time = items[i*2].detail(QOrganizerItemDetail::TypeEventTime);
            QCOMPARE(time.startDateTime(), eventStartDate.addMonths(i));

            time = items[(i*2)+1].detail(QOrganizerItemDetail::TypeEventTime);
            QCOMPARE(time.startDateTime(), QDateTime(QDate(2013, i+1, 5), QTime(0,0,0), Qt::UTC));
        }
    }

    void testModifyReccurenceEvents()
    {
        createTestEvent().id();

        QtOrganizer::QOrganizerManager::Error error;
        QOrganizerItemSortOrder sort;
        QOrganizerItemFetchHint hint;
        QOrganizerItemFilter filter;
        QList<QOrganizerItem> items;

        items = m_engine->items(filter,
                      QDateTime(QDate(2013, 11, 30), QTime(0,0,0)),
                      QDateTime(QDate(2014, 1, 1), QTime(0,0,0)),
                      100,
                      sort,
                      hint,
                      &error);
        QCOMPARE(items.count(), 5);
        QMap<int, QtOrganizer::QOrganizerManager::Error> errorMap;

        // remove only one item
        bool removeResult = m_engine->removeItems(QList<QtOrganizer::QOrganizerItemId>() << items[3].id(), &errorMap, &error);
        QCOMPARE(removeResult, true);
        QCOMPARE(error, QOrganizerManager::NoError);
        QCOMPARE(errorMap.size(), 0);

        items = m_engine->items(filter,
                      QDateTime(QDate(2013, 11, 30), QTime(0,0,0)),
                      QDateTime(QDate(2014, 1, 1), QTime(0,0,0)),
                      100,
                      sort,
                      hint,
                      &error);
        QCOMPARE(items.count(), 4);

        // edit only one item
        QList<QOrganizerItem> updateItems;
        QOrganizerItem updateItem = items[2];
        QList<QtOrganizer::QOrganizerItemDetail::DetailType> mask;

        updateItem.setDisplayLabel("Updated item 2");
        updateItems << updateItem;

        bool saveResult = m_engine->saveItems(&updateItems, mask, &errorMap, &error);
        QCOMPARE(saveResult, true);
        QCOMPARE(errorMap.size(), 0);
        QCOMPARE(error, QOrganizerManager::NoError);

        items = m_engine->items(QList<QOrganizerItemId>() << updateItem.id(),
                                hint, &errorMap, &error);

        QCOMPARE(errorMap.size(), 0);
        QCOMPARE(error, QOrganizerManager::NoError);
        QCOMPARE(items.size(), 1);
        QCOMPARE(items[0].displayLabel(), QStringLiteral("Updated item 2"));
    }

    void testQueryRecurrenceForAParentItem()
    {
         QOrganizerItem recurrenceEvent = createTestEvent();
         QtOrganizer::QOrganizerManager::Error error;
         QOrganizerItemSortOrder sort;
         QOrganizerItemFetchHint hint;
         QOrganizerItemFilter filter;
         QList<QOrganizerItem> items = m_engine->items(filter,
                                                       QDateTime(),
                                                       QDateTime(),
                                                       100,
                                                       sort,
                                                       hint,
                                                       &error);

         // this should return only the parent event
         QCOMPARE(error, QOrganizerManager::NoError);
         QCOMPARE(items.count(), 1);
         QCOMPARE(items[0].id(), recurrenceEvent.id());

         QOrganizerEvent parentEvent = static_cast<QOrganizerEvent>(items[0]);
         QCOMPARE(parentEvent.recurrenceRules().size(), 1);

         // Check if the limit date was saved correct
         QOrganizerRecurrenceRule rrule = parentEvent.recurrenceRule();
         QCOMPARE(rrule.limitDate(), QDate(2013, 12, 31));

         // query recurrence events for the event
         items = m_engine->itemOccurrences(recurrenceEvent,
                                           QDateTime(QDate(2013, 11, 30), QTime(0,0,0)),
                                           QDateTime(QDate(2014, 1, 1), QTime(0,0,0)),
                                           100,
                                           hint,
                                           &error);

         // check if all recurrence was returned
         QCOMPARE(items.count(), 5);

         QList<QDateTime> expectedDates;
         expectedDates << QDateTime(QDate(2013, 12, 2), QTime(0,0,0), QTimeZone("America/Recife"))
                       << QDateTime(QDate(2013, 12, 9), QTime(0,0,0), QTimeZone("America/Recife"))
                       << QDateTime(QDate(2013, 12, 16), QTime(0,0,0), QTimeZone("America/Recife"))
                       << QDateTime(QDate(2013, 12, 23), QTime(0,0,0), QTimeZone("America/Recife"))
                       << QDateTime(QDate(2013, 12, 30), QTime(0,0,0), QTimeZone("America/Recife"));
         for(int i=0; i < 5; i++) {
             QCOMPARE(items[i].type(), QOrganizerItemType::TypeEventOccurrence);
             QOrganizerEventTime time = items[i].detail(QOrganizerItemDetail::TypeEventTime);
             QCOMPARE(time.startDateTime(), expectedDates[i]);
         }
    }

    void testCreateSunTueWedThuFriSatEvents()
    {
        static QString displayLabelValue = QStringLiteral("testCreateSunTueWedThuFriSatEvents test");
        static QString descriptionValue = QStringLiteral("testCreateSunTueWedThuFriSatEvents description");

        QOrganizerEvent ev;
        ev.setCollectionId(m_collection.id());
        ev.setStartDateTime(QDateTime(QDate(2014, 03, 1), QTime(0,0,0), QTimeZone("America/Recife")));
        ev.setEndDateTime(QDateTime(QDate(2014, 03, 1), QTime(0,30,0), QTimeZone("America/Recife")));
        ev.setDisplayLabel(displayLabelValue);
        ev.setDescription(descriptionValue);

        QOrganizerRecurrenceRule rule;
        rule.setFrequency(QOrganizerRecurrenceRule::Weekly);
        QSet<Qt::DayOfWeek> dasyOfWeek;
        dasyOfWeek << Qt::Sunday
                   << Qt::Tuesday
                   << Qt::Wednesday
                   << Qt::Thursday
                   << Qt::Friday
                   << Qt::Saturday;

        rule.setDaysOfWeek(dasyOfWeek);
        ev.setRecurrenceRule(rule);

        QtOrganizer::QOrganizerManager::Error error;
        QMap<int, QtOrganizer::QOrganizerManager::Error> errorMap;
        QList<QOrganizerItem> items;
        items << ev;
        bool saveResult = m_engine->saveItems(&items,
                                              QList<QtOrganizer::QOrganizerItemDetail::DetailType>(),
                                              &errorMap,
                                              &error);
        QVERIFY(saveResult);
        QCOMPARE(error, QtOrganizer::QOrganizerManager::NoError);

        // append new item to be removed after the test
        QOrganizerItemId parentId = items[0].id();
        appendToRemove(parentId);

        QOrganizerItemSortOrder sort;
        QOrganizerItemFetchHint hint;
        QOrganizerItemFilter filter;

        // check if the parent was saved correct
        items = m_engine->items(QList<QOrganizerItemId>() << parentId, hint, &errorMap, &error);
        QCOMPARE(items.size(), 1);
        QOrganizerEvent result = items[0];
        QCOMPARE(result.collectionId(), ev.collectionId());
        QCOMPARE(result.startDateTime(), ev.startDateTime());
        QCOMPARE(result.endDateTime(), ev.endDateTime());
        QCOMPARE(result.displayLabel(), ev.displayLabel());
        QCOMPARE(result.description(), ev.description());

        QOrganizerRecurrenceRule savedRule = result.recurrenceRule();
        QCOMPARE(savedRule.frequency(), rule.frequency());
        QCOMPARE(savedRule.daysOfWeek(), rule.daysOfWeek());

        items = m_engine->items(filter,
                                QDateTime(QDate(2014, 03, 1), QTime(0,0,0)),
                                QDateTime(QDate(2014, 03, 8), QTime(24,0,0)),
                                100,
                                sort,
                                hint,
                                &error);

        QList<QDateTime> expectedDates;
        expectedDates << QDateTime(QDate(2014, 03, 1), QTime(0,0,0), QTimeZone("America/Recife"))
                      << QDateTime(QDate(2014, 03, 2), QTime(0,0,0), QTimeZone("America/Recife"))
                      << QDateTime(QDate(2014, 03, 4), QTime(0,0,0), QTimeZone("America/Recife"))
                      << QDateTime(QDate(2014, 03, 5), QTime(0,0,0), QTimeZone("America/Recife"))
                      << QDateTime(QDate(2014, 03, 6), QTime(0,0,0), QTimeZone("America/Recife"))
                      << QDateTime(QDate(2014, 03, 7), QTime(0,0,0), QTimeZone("America/Recife"));

        QCOMPARE(items.count(), expectedDates.size());
        for(int i=0, iMax=expectedDates.size(); i < iMax; i++) {
            QOrganizerItemParent itemParent = items[i].detail(QOrganizerItemDetail::TypeParent);
            QOrganizerEventTime time = items[i].detail(QOrganizerItemDetail::TypeEventTime);
            QCOMPARE(itemParent.parentId(), parentId);
            QCOMPARE(time.startDateTime(), expectedDates[i]);
        }
    }

    void testModifyAllRecurrence()
    {
        static const QString newDisplayLabel("New Display label for all items");
        QOrganizerItem item = createTestEvent();

        // edit all items
        QMap<int, QtOrganizer::QOrganizerManager::Error> errorMap;
        QtOrganizer::QOrganizerManager::Error error;
        QList<QOrganizerItem> updateItems;
        QList<QtOrganizer::QOrganizerItemDetail::DetailType> mask;

        item.setDisplayLabel(newDisplayLabel);
        updateItems << item;

        bool saveResult = m_engine->saveItems(&updateItems, mask, &errorMap, &error);
        QCOMPARE(saveResult, true);
        QCOMPARE(errorMap.size(), 0);
        QCOMPARE(error, QOrganizerManager::NoError);

        QOrganizerItemSortOrder sort;
        QOrganizerItemFetchHint hint;
        QOrganizerItemFilter filter;
        QList<QOrganizerItem> items = m_engine->items(filter,
                                                      QDateTime(QDate(2013, 11, 30), QTime(0,0,0)),
                                                      QDateTime(QDate(2014, 1, 1), QTime(0,0,0)),
                                                      100,
                                                      sort,
                                                      hint,
                                                      &error);
        QCOMPARE(items.count(), 5);
        Q_FOREACH(const QOrganizerItem &i, items) {
            QCOMPARE(i.displayLabel(), newDisplayLabel);
        }
    }

    void testModifyPriorEvents()
    {
        static const QString newDisplayLabel("New Display label for prior items");
        static const QDateTime startInteval(QDateTime(QDate(2013, 12, 2), QTime(0,0,0), QTimeZone("America/Recife")));
        static const QDateTime endInteval(QDateTime(QDate(2014, 1, 1), QTime(0,0,0), QTimeZone("America/Recife")));

        QOrganizerItem item = createTestEvent();

        QtOrganizer::QOrganizerManager::Error error;
        QOrganizerItemFetchHint hint;
        QList<QOrganizerItem> items;

        items = m_engine->itemOccurrences(item,
                                          startInteval,
                                          endInteval,
                                          100,
                                          hint,
                                          &error);

        QCOMPARE(items.count(), 5);
        // edit only events before 16/12/2013
        QOrganizerEventOccurrence changeItem = static_cast<QOrganizerEventOccurrence>(items[2]);
        QDate changeItemDate(2013, 12, 16);
        QCOMPARE(changeItem.startDateTime().date(), changeItemDate);

        // edit only one item
        changeItem.setDisplayLabel(newDisplayLabel);
        changeItem.setDescription("New Event Description");

        QtOrganizer::QOrganizerItemSaveRequest req(m_engine);
        changeItem.setDisplayLabel(newDisplayLabel);
        req.setItem(changeItem);
        req.setProperty("update-mode", 1 << 1);

        m_engine->startRequest(&req);
        m_engine->waitForRequestFinished(&req, 0);
        QCOMPARE(req.error(), QtOrganizer::QOrganizerManager::NoError);

        QOrganizerItemSortOrder sort;
        QOrganizerItemFilter filter;

        items = m_engine->items(filter,
                                startInteval,
                                endInteval,
                                100,
                                sort,
                                hint,
                                &error);

        QCOMPARE(items.count(), 5);
//FIXME
#if 0
        Q_FOREACH(const QOrganizerItem &i, items) {
            QOrganizerEventOccurrence event = static_cast<QOrganizerEventOccurrence>(i);
            qDebug() << i.displayLabel() << event.startDateTime().time() << i.description();
            //if (event.startDateTime().date() <= changeItemDate) {
            //    QCOMPARE(i.displayLabel(), newDisplayLabel);
            //} else {
            //    QCOMPARE(i.displayLabel(), item.displayLabel());
            //}
        }
#endif
    }
};

const QString RecurrenceTest::defaultCollectionName = QStringLiteral("TEST_RECURRENCE_EVENT_COLLECTION");

QTEST_MAIN(RecurrenceTest)

#include "recurrence-test.moc"
