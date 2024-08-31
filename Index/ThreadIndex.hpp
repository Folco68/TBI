/*
 * TBI - Technical Bulletin Indexer - Save and index Technical Bulletins,
 * allowing to use keywords to find them easily
 * Copyright (C) 2020 Martial Demolins AKA Folco
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * mail: martial <dot> demolins <at> gmail <dot> com
 */

#ifndef THREADINDEX_HPP
#define THREADINDEX_HPP

#include "TechnicalBulletin.hpp"
#include <QList>
#include <QThread>

// Need a MainWindow ptr, but can't include MainWindow header
class MainWindow;

class ThreadIndex: public QThread
{
    Q_OBJECT

  public:
    ThreadIndex(MainWindow* MainWindowPtr, bool ForceIndexCheck);
    ~ThreadIndex();

    QList<TechnicalBulletin*> tbList() const;

  signals:
    // Normal opening
    void openingIndex(qint32 version, qint32 count);
    void tbRead(int count);
    void indexOpenedSuccessfully(qint32 count);
    void noIndexFound();

    // Problem while opening
    void failedToOpenIndex();
    void invalidIndexIdentifier(QString magic);
    void indexTooRecent(qint32 version);
    void indexReadingFailed(int count);

    // End of opening (with or withour error)
    void openingComplete();

    // Save
    void saveComplete(int result);

  private:
    MainWindow*               MainWindowPtr;
    bool                      ForceIndexCheck;
    bool                      Modified;
    QList<TechnicalBulletin*> Bulletins;

    void run() override;
    bool readIndexV0(int count, QDataStream& stream, bool ForceIndexCheck);
    bool readIndexV1(qint32 Count, QDataStream& stream, bool ForceIndexCheck);

    // Slots triggered by MainWindow
    void save(bool backup);
};

// Index filename
#define TBI_BACKUP_FILENAME "index.bak"
#define TBI_FILENAME        "index.tbi"

// Current TBI version
#define CURRENT_TBI_VERSION 1

// Magic string to identify a TBI DB
#define TBI_MAGIC "TBI_DB_BY_MARTIAL_DEMOLINS"

// Save option
#define BACKUP_ON_SAVE    true
#define NO_BACKUP_ON_SAVE false

// Save results
#define SAVE_SUCCESSFUL          0
#define BACKUP_FAILED            1
#define SAVE_FAILED              2
#define SAVE_COULD_NOT_OPEN_FILE 3


#endif // THREADINDEX_HPP
