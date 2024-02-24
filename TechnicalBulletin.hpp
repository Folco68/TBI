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

#ifndef TECHNICALBULLETIN_HPP
#define TECHNICALBULLETIN_HPP

#include <QByteArray>
#include <QDataStream>
#include <QDate>
#include <QList>
#include <QString>

//  TechnicalBulletin
//
// Class containing the data of a Technical Bulletin
//
class TechnicalBulletin
{
  public:
    TechnicalBulletin() {}
    TechnicalBulletin(QByteArray data);
    TechnicalBulletin(QString        number,
                      QString        title,
                      QString        category,
                      QString        rk,
                      QString        techpub,
                      QString        comment,
                      QDate          releasedate,
                      QString        registeredby,
                      QString        replaces,
                      QString        replacedby,
                      QList<QString> keywords);
    void setData(QString        number,
                 QString        title,
                 QString        category,
                 QString        rk,
                 QString        techpub,
                 QString        comment,
                 QDate          releasedate,
                 QString        registeredby,
                 QString        replaces,
                 QString        replacedby,
                 QList<QString> keywords);

    QString        number() const { return this->Number; }
    QString        title() const { return this->Title; }
    QString        category() const { return this->Category; }
    QString        rk() const { return this->RK; }
    QString        techpub() const { return this->TechPub; }
    QString        comment() const { return this->Comment; }
    QDate          releaseDate() const { return this->ReleaseDate; }
    QString        registeredBy() const { return this->RegisteredBy; }
    QString        replaces() const { return this->Replaces; }
    QString        replacedBy() const { return this->ReplacedBy; }
    QList<QString> keywords() const { return this->Keywords; }

    QString keywordsString() const;

    void setKeywords(QList<QString> keywords) { this->Keywords = keywords; }

  private:
    QString        Number;
    QString        Title;
    QString        Category;
    QString        RK;
    QString        TechPub;
    QString        Comment;
    QDate          ReleaseDate;
    QString        RegisteredBy;
    QString        Replaces;
    QString        ReplacedBy;
    QList<QString> Keywords;
};

// Serialization
QDataStream& operator>>(QDataStream& stream, TechnicalBulletin* tb);
QDataStream& operator<<(QDataStream& stream, const TechnicalBulletin& tb);

// A TB pointer is saved in metadata of every table line, in column
// COLUMN_METADATA
Q_DECLARE_METATYPE(TechnicalBulletin*)

#endif // TECHNICALBULLETIN_HPP
