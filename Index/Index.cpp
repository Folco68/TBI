/***********************************************************************************************************************
 *                                                                                                                     *
 *                        TBI - Technical Bulletin Indexer - Save and index Technical Bulletins                        *
 *                                    allowing to use keywords to find them easily                                     *
 *                                 Copyright (C) 2020-2025 Martial Demolins AKA Folco                                  *
 *                                                                                                                     *
 *                        This program is free software: you can redistribute it and/or modify                         *
 *                        it under the terms of the GNU General Public License as published by                         *
 *                          the Free Software Foundation, either version 3 of the License, or                          *
 *                                         (at your option) any later version                                          *
 *                                                                                                                     *
 *                           This program is distributed in the hope that it will be useful                            *
 *                           but WITHOUT ANY WARRANTY; without even the implied warranty of                            *
 *                            MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                            *
 *                                     GNU General Public License for more details                                     *
 *                                                                                                                     *
 *                          You should have received a copy of the GNU General Public License                          *
 *                         along with this program.  If not, see <https://www.gnu.org/licenses                         *
 *                                                                                                                     *
 *                                  mail: martial <dot> demolins <at> gmail <dot> com                                  *
 *                                                                                                                     *
 **********************************************************************************************************************/

#include "Event/EventDeleteTB.hpp"
#include "Event/EventNewTB.hpp"
#include "Event/EventOpenIndex.hpp"
#include "Event/EventSave.hpp"
#include "Global.hpp"
#include "Index.hpp"
#include <QFile>
#include <QFileInfo>

//------------------------------------------------------------------------------------
//
//                                  Singleton stuff
//
//------------------------------------------------------------------------------------

Index* Index::index = nullptr;

Index* Index::instance()
{
    if (index == nullptr) {
        index = new Index;
    }
    return index;
}

void Index::release()
{
    if (index != nullptr) {
        index->deleteLater();
        index = nullptr;
    }
}

Index::Index()
    : OpeningSuccessful(true)
{}

Index::~Index()
{
    // Delete every element of the list and resize it to 0
    while (!Bulletins.isEmpty()) {
        delete Bulletins.takeLast();
    }
}

//------------------------------------------------------------------------------------
//
//                  Event handling. Events are sent by the UI thread
//                         with QCoreApplication::postEvent()
//
//------------------------------------------------------------------------------------

bool Index::event(QEvent* event)
{
    bool Return = false;

    if (event->type() == EVENT_OPEN_INDEX) {
        open(event);
        Return = true;
    }
    else if (event->type() == EVENT_SAVE) {
        save(event);
        Return = true;
    }
    else if (event->type() == EVENT_NEW_TB) {
        newTB(event);
        Return = true;
    }
    else if (event->type() == EVENT_DELETE_TB) {
        deleteTB(event);
        Return = true;
    }

    // Defer the event to the QObject if it was not handled by the switch/case
    return Return ? Return : QObject::event(event);
}

//------------------------------------------------------------------------------------
//
//                   Index opening, depending on the index version
//
//------------------------------------------------------------------------------------

void Index::open(QEvent* event)
{
    emit openingStarting();

    EventOpenIndex* Event(static_cast<EventOpenIndex*>(event));
    bool            ForceIndexCheck = Event->forceIndexCheck();

    // Early return if the file does not exist
    if (!QFileInfo::exists(TBI_FILENAME)) {
        emit noIndexFound();
        return;
    }

    // Opening
    QFile file(TBI_FILENAME);
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream Stream(&file);

        // The first version of .tbi files were not versionned. They contain the number of TB as an int, then the serialized TB themselves.
        // The next versions set this counter to 0, then stores a magic, then the index version.
        // This makes the old executables unable to open recent files,
        // and recent executables able to understand the old indexes.
        qint32 Count;
        Stream >> Count;

        // If count == 0, two cases:
        // - it's an empty old file. Reading the magic will lead to a stream reading error. Let's fail silently the opening.
        // - it's a versionned file. Read the magic then the version number to decide which opener must be used
        if (Count == 0) {
            // Try to read a magic
            QString Magic;
            Stream >> Magic;

            // If the stream failed to read data, it "should" be an empty unversionned file.
            // Ok, it could also be an USB stick pulled out when reading, but we cannot make the difference.
            // Let's assume it's an old empty file and let's do nothing.
            // So, we only consider streams with a successful magic reading.
            if (Stream.status() == QDataStream::Ok) {
                if (Magic == QString(TBI_MAGIC)) {
                    // If the magic is valid, read the version and open the file according to it
                    qint32 Version;
                    Stream >> Version;

                    switch (Version) {
                        case 1:
                            openIndexVersion1(Stream, ForceIndexCheck);
                            break;

                        default:
                            // Version of the future, unhandled by this binary...
                            emit indexTooRecent(Version);
                            this->OpeningSuccessful = false;
                    }
                }
                // Invalid magic
                else {
                    emit invalidMagic(Magic);
                    this->OpeningSuccessful = false;
                }
            }

            // Stream status control failed
            // But don't throw a message, most probably it means that it was an empty and unversionned file
            // else {
            //     QMessageBox::critical(this, WINDOW_TITLE, tr("Invalid file %1").arg(TBI_FILENAME));
            // }
        }

        // If count != 0, it's an old file, no doubt.
        else {
            openIndexVersion0(Count, Stream, ForceIndexCheck);
        }
    }
    // Throw an error if the file exists and couldn't be opened
    else if (QFileInfo::exists(TBI_FILENAME)) {
        emit failedToOpenIndex();
        this->OpeningSuccessful = false;
    }

    // Opening is complete now
    if (this->OpeningSuccessful) {
        emit openingSuccessful(this->Bulletins.count());
    }
    else {
        emit openingFailed(this->Bulletins.count());
    }
}

//  openIndexVersion0
//
// Open an index (legacy format)
//
void Index::openIndexVersion0(qint32 count, QDataStream& stream, bool ForceIndexCheck)
{
    // Emit index version + TB count
    emit openingHeader(0, count);

    for (int i = 0; i < count; i++) {
        // Read a TB
        TechnicalBulletin* TB = new TechnicalBulletin;
        stream >> TB;

        // Check stream status
        if (stream.status() != QDataStream::Ok) {
            emit failedToReadFileContent();
            delete TB;
            this->OpeningSuccessful = false;
            break;
        }

        // Stream is OK, add the TB to the UI
        this->Bulletins.append(TB);

        // Emit a progress message
        if (i && (i % 100 == 0)) {
            emit openingProgress(i);
        }
    }
}

//  openIndexVersion1
//
// Open an index version 1
void Index::openIndexVersion1(QDataStream& stream, bool ForceIndexCheck)
{
    // Read the number of TB
    qint32 Count;
    stream >> Count;

    // Emit index version + TB count
    emit openingHeader(1, Count);

    for (int i = 0; i < Count; i++) {
        // Read a TB
        TechnicalBulletin* TB = new TechnicalBulletin;
        stream >> TB;

        // Check stream status
        if (stream.status() != QDataStream::Ok) {
            emit failedToReadFileContent();
            delete TB;
            this->OpeningSuccessful = false;
            break;
        }

        // Stream is OK, add the TB to the UI
        this->Bulletins.append(TB);

        // Emit a progress message
        if (i && (i % 100 == 0)) {
            emit openingProgress(i);
        }
    }
}

QList<TechnicalBulletin*> Index::bulletins() const
{
    return this->Bulletins;
}

void Index::save(QEvent* event)
{
    EventSave* Event(static_cast<EventSave*>(event));
    bool       Backup = Event->backup();

    // On demand, create a backup by renaming the current index.
    // Remove current backup because File::rename() won't overwrite current file
    if (Backup) {
        QFile::remove(TBI_BACKUP_FILENAME);
        if (!QFile::rename(TBI_FILENAME, TBI_BACKUP_FILENAME)) {
            emit failedToCreateBackup();
            return;
        }
    }

    // Try to open the file
    QFile File(TBI_FILENAME);
    if (!File.open(QIODevice::WriteOnly)) {
        emit failedToOpenFileForSaving();
        return;
    }

    // Open a data stream and write into it
    QDataStream Stream(&File);

    // First, write 0 to support old DB
    Stream << (qint32) 0;

    // Then write magic + current version
    Stream << QString(TBI_MAGIC) << (qint32) CURRENT_TBI_VERSION;

    // Write TB count
    Stream << (qint32) (this->Bulletins.count());

    // Serialize TBs
    for (int i = 0; i < this->Bulletins.count(); i++) {
        Stream << *this->Bulletins.at(i);
        if (Stream.status() != QDataStream::Ok) {
            emit failedToWriteContent(i);
            return;
        }
    }

    emit savingSuccessful(this->Bulletins.count());
}

void Index::newTB(QEvent* event) {}

void Index::deleteTB(QEvent* event)
{
    EventDeleteTB*     Event(static_cast<EventDeleteTB*>(event));
    TechnicalBulletin* TB = Event->tb();

    // Consistency checks
    if (TB == nullptr) {
        emit failedToDeleteTB("NULLPTR received!!!", "");
        return;
    }

    if (!this->Bulletins.contains(TB)) {
        emit failedToDeleteTB(TB->number(), TB->title());
        return;
    }

    // Delete the TB and remove it from the list
    emit tbDeletionSuccessful(TB->number(), TB->title());
    delete TB;
    this->Bulletins.removeOne(TB);
}
