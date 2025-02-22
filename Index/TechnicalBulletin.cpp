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

#include "TechnicalBulletin.hpp"
#include <QByteArrayView>

//  TechnicalBulletin
//
// Create a TB using data provided by UI
//
TechnicalBulletin::TechnicalBulletin(QString        number,
                                     QString        title,
                                     QString        category,
                                     QString        rk,
                                     QString        techpub,
                                     QString        comment,
                                     QDate          releasedate,
                                     QString        registeredby,
                                     QString        replaces,
                                     QString        replacedby,
                                     QList<QString> keywords)
    : Number(number)
    , Title(title)
    , Category(category)
    , RK(rk)
    , TechPub(techpub)
    , Comment(comment)
    , ReleaseDate(releasedate)
    , RegisteredBy(registeredby)
    , Replaces(replaces)
    , ReplacedBy(replacedby)
    , Keywords(keywords)
{
}

//  updateData
//
// Update an existing TB
//
void TechnicalBulletin::updateData(QString        number,
                                   QString        title,
                                   QString        category,
                                   QString        rk,
                                   QString        techpub,
                                   QString        comment,
                                   QDate          releasedate,
                                   QString        registeredby,
                                   QString        replaces,
                                   QString        replacedby,
                                   QList<QString> keywords)
{
    this->Number       = number;
    this->Title        = title;
    this->Category     = category;
    this->RK           = rk;
    this->TechPub      = techpub;
    this->Comment      = comment;
    this->ReleaseDate  = releasedate;
    this->RegisteredBy = registeredby;
    this->Replaces     = replaces;
    this->ReplacedBy   = replacedby;
    this->Keywords     = keywords;
}

void TechnicalBulletin::fixTrimming()
{
    this->Number       = this->Number.trimmed();
    this->Title        = this->Title.trimmed();
    this->Category     = this->Category.trimmed();
    this->RK           = this->RK.trimmed();
    this->TechPub      = this->TechPub.trimmed();
    this->Comment      = this->Comment.trimmed();
    this->RegisteredBy = this->RegisteredBy.trimmed();
    this->Replaces     = this->Replaces.trimmed();
    this->ReplacedBy   = this->ReplacedBy.trimmed();
}

void TechnicalBulletin::fixDate()
{
    this->ReleaseDate = QDate::currentDate();
}

void TechnicalBulletin::fixTechpub()
{
    QList<QString> List(this->TechPub.split(',', Qt::SkipEmptyParts));
    for (int i = 0; i < List.size(); i++) {
        List[i] = List.at(i).trimmed();
    }
    this->TechPub = List.join(',');
}

//  >>
//
// Unserialize a TB
//
QDataStream& operator>>(QDataStream& stream, TechnicalBulletin* tb)
{
    QByteArray     Number;
    QByteArray     Title;
    QByteArray     Category;
    QByteArray     RK;
    QByteArray     TechPub;
    QByteArray     Comment;
    QDate          ReleaseDate;
    QByteArray     RegisteredBy;
    QByteArray     Replaces;
    QByteArray     ReplacedBy;
    QList<QString> Keywords;

    stream >> Number >> Title >> Category >> RK >> TechPub >> Comment >> ReleaseDate >> RegisteredBy >> Replaces >> ReplacedBy >> Keywords;
    tb->updateData(QString::fromUtf8(Number),
                   QString::fromUtf8(Title),
                   QString::fromUtf8(Category),
                   QString::fromUtf8(RK),
                   QString::fromUtf8(TechPub),
                   QString::fromUtf8(Comment),
                   ReleaseDate,
                   QString::fromUtf8(RegisteredBy),
                   QString::fromUtf8(Replaces),
                   QString::fromUtf8(ReplacedBy),
                   Keywords);
    return stream;
}

//  <<
//
// Serialize a TB
//
QDataStream& operator<<(QDataStream& stream, const TechnicalBulletin& tb)
{
    stream << tb.number().toUtf8() << tb.title().toUtf8() << tb.category().toUtf8() << tb.rk().toUtf8() << tb.techpub().toUtf8() << tb.comment().toUtf8() << tb.releaseDate()
           << tb.registeredBy().toUtf8() << tb.replaces().toUtf8() << tb.replacedBy().toUtf8() << tb.keywords();
    return stream;
}
