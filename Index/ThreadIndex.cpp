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

#include "ThreadIndex.hpp"
#include "../UI/MainWindow.hpp"
#include <QDataStream>
#include <QFile>
#include <QFileInfo>

ThreadIndex::ThreadIndex(MainWindow* MainWindowPtr, bool ForceIndexCheck)
    : MainWindowPtr(MainWindowPtr)
    , ForceIndexCheck(ForceIndexCheck)
    , Modified(false)
{
    // Connections
    connect(this->MainWindowPtr, &MainWindow::save, this, [this](bool backup) { save(backup); });
}

ThreadIndex::~ThreadIndex()
{
    // Destroy index data
}

void ThreadIndex::run()
{
    // Try to open the index if one exists
    if (QFileInfo::exists(TBI_FILENAME)) {
        QFile file(TBI_FILENAME);
        if (file.open(QIODevice::ReadOnly)) {
            QDataStream Stream(&file);

            // The first version of .tbi files was not versionned. They contain the number of TB as an int (32 bits), then the serialized TB themselves.
            // The next versions set this counter to 0, then store a magic, then the DB version.
            // This makes the old executables unable to open recent files.
            // And recent executables able to understand the old DB.
            qint32 Version;
            qint32 Count;
            Stream >> Count;

            // If count == 0, two cases:
            // - it's an empty old file. Reading the magic will lead to a stream reading error. Let's fail silently the opening because there is nothing to read.
            // - it's a versionned file. Read the magic then the version number to decide which opener must be used.
            if (Count == 0) {
                // Try to read a magic
                QString Magic;
                Stream >> Magic;

                // If the stream failed to read data, it "should" be an empty unversionned file.
                // Ok, it could also be an USB stick pulled out when reading, but we cannot make the difference.
                // Let's assume it is an old empty file and let's do nothing.
                // So, we only consider streams with a successful magic reading.
                if (Stream.status() == QDataStream::Ok) {
                    if (Magic == QString(TBI_MAGIC)) {
                        // If the magic is valid, read the version and the TB count, then open the file according to it
                        Stream >> Version >> Count;
                        emit openingIndex(Version, Count);

                        switch (Version) {

                            case 1:
                                if (readIndexV1(Count, Stream, ForceIndexCheck)) {
                                    emit indexOpenedSuccessfully(Count);
                                }
                                else {
                                    this->Modified = true; // File partially opened
                                    emit indexReadingFailed(this->Bulletins.count());
                                }
                                break;

                            default:
                                emit indexTooRecent(Version);
                        }
                    }

                    // Invalid magic
                    else {
                        emit invalidIndexIdentifier(Magic);
                    }
                }

                // Stream status control failed when reading the magic.
                // But don't throw a message, it just means that it was an empty and unversionned file.
                // Legacy code from MainWindow.cpp:
                // else {
                //     QMessageBox::critical(this, WINDOW_TITLE, tr("Invalid file %1").arg(TBI_FILENAME));
                // }
            }

            // If count != 0, it's an old file, no doubt.
            else {
                if (readIndexV0(Count, Stream, ForceIndexCheck)) {
                    emit indexOpenedSuccessfully(Count);
                }
                else {
                    this->Modified = true; // File partially opened
                    emit indexReadingFailed(this->Bulletins.count());
                }
            }
        }

        // QFile::open() failure
        else {
            emit failedToOpenIndex();
        }
    }

    // The file does not exist
    else {
        emit noIndexFound();
    }

    // Finally, run the event loop to handle the signals emitted by the GUI
    emit openingComplete();
    exec();
}

//  readIndexV0
//
// Open an index in the legacy format
//
bool ThreadIndex::readIndexV0(int count, QDataStream& stream, bool ForceIndexCheck)
{
    for (int i = 0; i < count; i++) {
        // Emit a message intended to a progress bar
        if (i % 100 == 0) {
            emit tbRead(i);
        }

        // Read a TB
        TechnicalBulletin* TB = new TechnicalBulletin;
        stream >> TB;

        // Check stream status
        if (stream.status() != QDataStream::Ok) {
            delete TB;
            return false;
        }

        // Add the bulletin to the list
        this->Bulletins << TB;
    }

    // Success
    return true;
}

//  readIndexV1
//
// Open an index version 1
bool ThreadIndex::readIndexV1(qint32 count, QDataStream& stream, bool ForceIndexCheck)
{
    for (int i = 0; i < count; i++) {
        // Emit a message intended for a progress bar
        if (i % 100 == 0) {
            emit tbRead(i);
        }

        // Read a TB
        TechnicalBulletin* TB = new TechnicalBulletin;
        stream >> TB;

        // Check stream status
        if (stream.status() != QDataStream::Ok) {
            delete TB;
            return false;
        }

        // Add the bulletin to the list
        this->Bulletins << TB;
    }

    // Success
    return true;
}

QList<TechnicalBulletin*> ThreadIndex::tbList() const
{
    return this->Bulletins;
}

void ThreadIndex::save(bool backup)
{
    // TODO: save

    this->Modified = false;
    emit saveComplete(SAVE_SUCCESSFUL);
}
