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

#ifndef INDEX_HPP
#define INDEX_HPP

#include "TechnicalBulletin.hpp"
#include <QEvent>
#include <QList>
#include <QObject>
#include <QString>

class Index : public QObject
{
    Q_OBJECT
  public:
    static Index* instance();
    static void   release();

  signals:
    void threadID(Qt::HANDLE handle);

    // Opening on its normal way
    void openingStarting();
    void openingHeader(int version, qint32 count);
    void openingProgress(int count);
    void openingSuccessful(QList<TechnicalBulletin*> bulletins); // Mark end of opening
    void noIndexFound();                                         // Mark end of opening

    // Opening failing
    void indexTooRecent(qint32 version);
    void invalidMagic(QString magic);
    void failedToOpenIndex();
    void failedToReadFileContent();
    void openingFailed(QList<TechnicalBulletin*> bulletins); // Mark end of opening

    // Saving
    void failedToCreateBackup();
    void failedToOpenFileForSaving();
    void failedToWriteContent(int count);
    void savingSuccessful(int count);

    // New / Edit / Delete
    void bulletinCreated(TechnicalBulletin* tb);
    void tbAlreadyExists(QString number);
    void unrecognizedTBnumber(QString number);
    void olderTBfound(TechnicalBulletin* tb);
    void failedToDeleteTB(QString number, QString title);
    void tbDeletionSuccessful(QString number, QString title);

    // Maintenance
    void checkingDone(int count);
    void fixingDone(int count);

    void trimmingCheckStart();
    void trimmingCheckDone(int count);
    void trimmingFixed();

    void dateCheckStart();
    void dateCheckDone(int count);
    void dateFixed();

    void techpubCheckStart();
    void techpubCheckDone(int count);
    void techpubFixed();

  private:
    Index();
    ~Index();
    static Index* index;

    // Normal operations
    bool event(QEvent* event) override;
    void open(QEvent* event);
    void openIndexVersion0(qint32 count, QDataStream& stream, bool ForceIndexCheck);
    void openIndexVersion1(QDataStream& stream, bool ForceIndexCheck);
    void save(QEvent* event);
    void newTB(QEvent* event);
    void deleteTB(QEvent* event);
    void mergeTB(QEvent* event);

    QList<TechnicalBulletin*> Bulletins;
    bool                      OpeningSuccessful;

    // Validation / Sanitization / Checks / Whatever
    bool validateNumber(QString number) const;
    void checkIndex();
    void fixIndex();
    void clearIncorrect();
    int  incorrectCount() const;

    QList<TechnicalBulletin*> IncorrectTrimming;
    QList<TechnicalBulletin*> IncorrectDate;
    QList<TechnicalBulletin*> IncorrectTechpub;
};

#endif // INDEX_HPP
