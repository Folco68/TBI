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
#include "Event/EventMergeTB.hpp"
#include "Event/EventNewTB.hpp"
#include "Event/EventOpenIndex.hpp"
#include "Event/EventSave.hpp"
#include "Global.hpp"
#include "Index.hpp"
#include <QCoreApplication>
#include <QDate>
#include <QFile>
#include <QFileInfo>
#include <QThread>

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
    for (int i = 0; i < this->Bulletins.size(); i++) {
        delete this->Bulletins.at(i);
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
    else if (event->type() == EVENT_MERGE_TB) {
        mergeTB(event);
        Return = true;
    }
    else if (event->type() == EVENT_CHECK_INDEX) {
        checkIndex();
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
    emit threadID(QThread::currentThreadId());
    emit openingStarting();

    EventOpenIndex* Event(static_cast<EventOpenIndex*>(event));
    bool            ForceIndexCheck = Event->forceIndexCheck();

    // Early return if the file does not exist
    if (!QFileInfo::exists(INDEX_FILENAME)) {
        emit noIndexFound();
        return;
    }

    // Opening
    QFile file(INDEX_FILENAME);
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
            //     QMessageBox::critical(this, WINDOW_TITLE, tr("Invalid file %1").arg(INDEX_FILENAME));
            // }
        }

        // If count != 0, it's an old file, no doubt.
        else {
            openIndexVersion0(Count, Stream, ForceIndexCheck);
        }
    }
    // Throw an error if the file exists and couldn't be opened
    else if (QFileInfo::exists(INDEX_FILENAME)) {
        emit failedToOpenIndex();
        this->OpeningSuccessful = false;
    }

    // Opening is complete now
    if (this->OpeningSuccessful) {
        emit openingSuccessful(this->Bulletins.size());
    }
    else {
        emit openingFailed(this->Bulletins.size());
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
        QFile::remove(INDEX_BACKUP_FILENAME);
        if (!QFile::rename(INDEX_FILENAME, INDEX_BACKUP_FILENAME)) {
            emit failedToCreateBackup();
            return;
        }
    }

    // Try to open the file
    QFile File(INDEX_FILENAME);
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
    Stream << (qint32) (this->Bulletins.size());

    // Serialize TBs
    for (int i = 0; i < this->Bulletins.size(); i++) {
        Stream << *this->Bulletins.at(i);
        if (Stream.status() != QDataStream::Ok) {
            emit failedToWriteContent(i);
            return;
        }
    }

    emit savingSuccessful(this->Bulletins.size());
}

void Index::newTB(QEvent* event)
{
    EventNewTB* Event(static_cast<EventNewTB*>(event));

    /*******************************************************************************************************************
     *                                                                                                                 *
     *                             Perform multiple checks before adding a TB to the index                             *
     *                                                                                                                 *
     ******************************************************************************************************************/

    QString Number = Event->number();

    // TB number must be valid to perform these tests
    if (validateNumber(Number)) {
        QString Radix(Number.chopped(1 + 2)); // sizeof('_') + sizeof(version)
        QString Version(Number.last(2));      // sizeof(version)

        for (int i = 0; i < this->Bulletins.size(); i++) {
            QString TmpNumber = this->Bulletins.at(i)->number();
            if (TmpNumber.chopped(1 + 2) == Radix) {
                QString TmpVersion(TmpNumber.right(2)); // sizeof(version)

                // Forbid TB with identical number
                if (Version == TmpVersion) {
                    emit tbAlreadyExists(Number);
                    return;
                }

                // If the TB replaces an old one, offer to upgrade it.
                // The TB can be added now, merge will be done later
                if (Version > TmpVersion) {
                    emit olderTBfound(this->Bulletins.at(i));
                    break;
                }
            }
        }
    }
    else {
        emit unrecognizedTBnumber(Number);
    }

    TechnicalBulletin* TB = new TechnicalBulletin(Event->number(),
                                                  Event->title(),
                                                  Event->category(),
                                                  Event->rk(),
                                                  Event->techPub(),
                                                  Event->comment(),
                                                  Event->releaseDate(),
                                                  Event->registeredBy(),
                                                  Event->replaces(),
                                                  Event->replacedBy(),
                                                  Event->keywords());
    this->Bulletins.append(TB);
    emit bulletinCreated(TB);
}

void Index::mergeTB(QEvent* event)
{
    EventMergeTB*      Event(static_cast<EventMergeTB*>(event));
    TechnicalBulletin* OldTB(Event->tb());
    TechnicalBulletin* NewTB(this->Bulletins.constLast());
    QList<QString>     NewKeywords = NewTB->keywords();

    if (Event->mergeKeywords()) {
        for (int i = 0; i < OldTB->keywords().size(); i++) {
            QString OldKeyword(OldTB->keywords().at(i));
            if (!NewKeywords.contains(OldTB->keywords().at(i))) {
                NewKeywords.append(OldKeyword);
            }
        }
        NewTB->setKeywords(NewKeywords);
    }

    // Delete the old TB
    EventDeleteTB DeleteEvent(OldTB);
    QCoreApplication::sendEvent(this, &DeleteEvent);
}

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

/***********************************************************************************************************************
 *                                                                                                                     *
 *                                    Validation / Sanitization / Checks / Whatever                                    *
 *                                                                                                                     *
 *                         The number of a TB looks like: [machines]_[year]_[rank]_[version],                          *
 *                                                       where:                                                        *
 *      [machines] is for one type of machine ('TPA3F'), for a category ('TPA3E3'), or a group of category ('FM')      *
 *                                 [year] 4 digits. The year in the Gregorian calendar                                 *
 *                              [rank] 2 digits. Reset at 01 at the beginning of the year                              *
 *                  [version] 2 digits. Starts at 01, increased by 1 at every release of the same TB                   *
 *                                                                                                                     *
 *              This is unofficial, so there are extra consistency checks to prevent a bad interpretation              *
 *                                                                                                                     *
 **********************************************************************************************************************/

bool Index::validateNumber(QString number) const
{
    // Split the 4 groups of the TB number
    QList<QString> SplittedNumber(number.split('_', Qt::KeepEmptyParts));

    // We need 4 groups of information
    if (SplittedNumber.size() != 4) {
        return false;
    }

    QString Year(SplittedNumber.at(1));
    QString Rank(SplittedNumber.at(2));
    QString Version(SplittedNumber.at(3));

    // Year must be 4 digits and <= current year
    if ((Year.size() != 4) || (Year.toInt() > QDate::currentDate().year())) {
        return false;
    }

    // Rank must be 2 digits
    if (Rank.size() != 2) {
        return false;
    }

    // Version must be 2 digits
    if (Version.size() != 2) {
        return false;
    }

    // All looks fine
    return true;
}

void Index::checkIndex()
{
    /*******************************************************************************************************************
     *                                                                                                                 *
     *                                                    Trimming                                                     *
     *                                                                                                                 *
     ******************************************************************************************************************/

    emit checkingTrimming();
    for (int i = 0; i < this->Bulletins.size(); i++) {
        TechnicalBulletin* TB(this->Bulletins.at(i));
        bool               Add(false);

        if (TB->number().trimmed() != TB->number()) {
            Add = true;
        }
        if (TB->title().trimmed() != TB->title()) {
            Add = true;
        }
        if (TB->category().trimmed() != TB->category()) {
            Add = true;
        }
        if (TB->rk().trimmed() != TB->rk()) {
            Add = true;
        }
        if (TB->techpub().trimmed() != TB->techpub()) {
            Add = true;
        }
        if (TB->comment().trimmed() != TB->comment()) {
            Add = true;
        }
        if (TB->registeredBy().trimmed() != TB->registeredBy()) {
            Add = true;
        }
        if (TB->replaces().trimmed() != TB->replaces()) {
            Add = true;
        }
        if (TB->replacedBy().trimmed() != TB->replacedBy()) {
            Add = true;
        }

        if (Add) {
            this->IncorrectTrimming.append(TB);
        }
    }
    emit trimmingCheckDone(this->IncorrectTrimming.size());

    /*******************************************************************************************************************
     *                                                                                                                 *
     *                                         Looking for dates in the future                                         *
     *                                                                                                                 *
     ******************************************************************************************************************/

    emit checkingDate();
    for (int i = 0; i < this->Bulletins.size(); i++) {
        QDate              CurrentDate(QDate::currentDate());
        TechnicalBulletin* TB(this->Bulletins.at(i));
        if (TB->releaseDate() > CurrentDate) {
            this->IncorrectDate.append(TB);
        }
    }
    emit dateCheckDone(this->IncorrectDate.size());

    /*******************************************************************************************************************
     *                                                                                                                 *
     *                                             Technical publications                                              *
     *                                                                                                                 *
     ******************************************************************************************************************/

    emit checkingTechpub();
    for (int i = 0; i < this->Bulletins.size(); i++) {
        TechnicalBulletin* TB(this->Bulletins.at(i));
        QList<QString>     List(TB->techpub().split(',', Qt::KeepEmptyParts));
        for (int j = 0; j < List.size(); j++) {
            List[j] = List.at(j).trimmed();
        }
        if (List.join(',') != TB->techpub()) {
            this->IncorrectTechpub.append(TB);
        }
    }
    emit techpubCheckDone(this->IncorrectTechpub.size());
}

void Index::fixIndex()
{
    /*******************************************************************************************************************
     *                                                                                                                 *
     *                                                    Trimming                                                     *
     *                                                                                                                 *
     ******************************************************************************************************************/

    for (int i = 0; i < this->IncorrectTrimming.size(); i++) {
        this->IncorrectTrimming.at(i)->fixTrimming();
    }
    emit trimmingFixed();

    /*******************************************************************************************************************
     *                                                                                                                 *
     *                                                      Date                                                       *
     *                                                                                                                 *
     ******************************************************************************************************************/

    for (int i = 0; i < this->IncorrectDate.size(); i++) {
        this->IncorrectDate.at(i)->fixDate();
    }
    emit dateFixed();

    /*******************************************************************************************************************
     *                                                                                                                 *
     *                                             Technical publications                                              *
     *                                                                                                                 *
     ******************************************************************************************************************/

    for (int i = 0; i < this->IncorrectTechpub.size(); i++) {
        this->IncorrectTechpub.at(i)->fixTechpub();
    }
    emit techpubFixed();

    /*******************************************************************************************************************
    *                                                                                                                  *
    *                                                      Clean                                                       *
    *                                                                                                                  *
    *******************************************************************************************************************/

    this->IncorrectTrimming.clear();
    this->IncorrectDate.clear();
    this->IncorrectTechpub.clear();
}
