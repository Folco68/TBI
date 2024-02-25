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

#include "DownloadMenu.hpp"
#include "Settings.hpp"
#include <QDesktopServices>
#include <QUrl>

DownloadMenu::DownloadMenu(QWidget* parent)
    : QMenu(parent)
{
}

DownloadMenu::~DownloadMenu()
{
    deleteActions();
}

void DownloadMenu::deleteActions()
{
    while (!this->ActionList.isEmpty()) {
        delete this->ActionList.takeLast();
    }
}

void DownloadMenu::setItems(QString DocsString, QString TBnumber, QWidget* WidgetToFocus)
{
    deleteActions();

    // Always add the TB (pfd)
    QAction* PdfAction = addAction("Technical Bulletin");
    this->ActionList.append(PdfAction);
    connect(PdfAction, &QAction::triggered, this, [TBnumber, WidgetToFocus]() {
        QDesktopServices::openUrl(QString(Settings::instance()->baseURLTechnicalBulletinPDF()).arg(TBnumber));
        if (WidgetToFocus != nullptr) {
            WidgetToFocus->setFocus();
        }
    });

    // Always add the TB CTI (pfd)
    QAction* CTIAction = addAction("Customer Technical Information");
    this->ActionList.append(CTIAction);
    TBnumber += CTI_SUFFIX;
    connect(CTIAction, &QAction::triggered, this, [TBnumber, WidgetToFocus]() {
        QDesktopServices::openUrl(QString(Settings::instance()->baseURLTechnicalBulletinPDF()).arg(TBnumber));
        if (WidgetToFocus != nullptr) {
            WidgetToFocus->setFocus();
        }
    });

    // Split the documents names
    QStringList DocList = DocsString.split(QChar(','), Qt::SkipEmptyParts, Qt::CaseInsensitive);

    if (!DocList.isEmpty()) {
        // Add all the docs, creating their own lambda function to start the download when the menu item is triggered
        for (int i = 0; i < DocList.count(); i++) {
            QAction* DocAction = addAction(DocList.at(i).trimmed());
            this->ActionList.append(DocAction);
            connect(DocAction, &QAction::triggered, this, [DocAction, WidgetToFocus]() {
                QString FullName = DocAction->text();
                QString Name     = FullName.section(QChar('-'), 1);
                QDesktopServices::openUrl(QString(Settings::instance()->baseURLTechnicalPublications()).arg(Name));

                if (WidgetToFocus != nullptr) {
                    WidgetToFocus->setFocus();
                }
            });
        }
    }
}
