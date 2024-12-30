#include "../Event/EventOpenIndex.hpp"
#include "../Global.hpp"
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
    switch (event->type()) {
        // First event received, only once, to open the index
        case (EVENT_OPEN_INDEX): {
            EventOpenIndex* Event = static_cast<EventOpenIndex*>(event);
            open(Event->forceIndexCheck());
            return true;
        }

        // Unknown event: defer it to QObject
        default:
            return QObject::event(event);
    }
}

//------------------------------------------------------------------------------------
//
//                   Index opening, depending on the index version
//
//------------------------------------------------------------------------------------

void Index::open(bool ForceIndexCheck)
{
    emit openingStarting();

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
                            openIndexV1(Stream, ForceIndexCheck);
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
            openIndexV0(Count, Stream, ForceIndexCheck);
        }
    }
    // Throw an error if the file exists and couldn't be opened
    else if (QFileInfo::exists(TBI_FILENAME)) {
        emit cantOpenIndex();
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

//  openDBv0
//
// Open a DB in the legacy format
//
void Index::openIndexV0(qint32 count, QDataStream& stream, bool ForceIndexCheck)
{
    // Emit index version + TB count
    emit openingHeader(0, count);

    for (int i = 0; i < count; i++) {
        // Read a TB
        TechnicalBulletin* TB = new TechnicalBulletin;
        stream >> TB;

        // Check stream status
        if (stream.status() != QDataStream::Ok) {
            emit unableToReadFileContent();
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

//  openDBv1
//
// Open a DB version 1
void Index::openIndexV1(QDataStream& stream, bool ForceIndexCheck)
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
            emit unableToReadFileContent();
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
