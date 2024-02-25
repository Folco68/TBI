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

#include "DlgHelp.hpp"
#include "BeforeRelease.hpp"
#include "ui_DlgHelp.h"
#include <QFile>
#include <QPushButton>
#include <QString>
#include <QTextStream>

DlgHelp::DlgHelp(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DlgHelp)
{
    ui->setupUi(this);
    ui->Tabs->setCurrentIndex(0);
    connect(ui->ButtonClose, &QPushButton::clicked, this, [this]() { close(); });

    // Open About text
    // In case of failure, there is a place holder in the UI
    QFile FileAbout(":/DlgHelp/Docs/About.txt");
    if (FileAbout.open(QIODeviceBase::ReadOnly)) {
        QTextStream StreamAbout(&FileAbout);
        QString     StringAbout(StreamAbout.readAll());

        // Perform some replacements before displaying the text
        StringAbout.replace(REPLACE_APPLICATION_VERSION_STR, APPLICATION_VERSION_STR);
        StringAbout.replace(REPLACE_POSITION_STR, POSITION_STR);
        StringAbout.replace(REPLACE_COPYRIGHT_STR, COPYRIGHT_STR);
        StringAbout.replace(REPLACE_QT_VERSION_STR, QT_VERSION_STR);

        // Set up the text
        ui->TextEditAbout->setPlainText(StringAbout);
    }

    // Open Help text
    // In case of failure, there is a place holder in the UI
    QFile FileHelp(":/DlgHelp/Docs/Help.txt");
    if (FileHelp.open(QIODeviceBase::ReadOnly)) {
        QTextStream StreamHelp(&FileHelp);
        QString     StringHelp(StreamHelp.readAll());
        ui->TextEditHelp->setPlainText(StringHelp);
    }

    // Open Changelog text
    // In case of failure, there is a place holder in the UI
    QFile FileChangelog(":/DlgHelp/Docs/Changelog.txt");
    if (FileChangelog.open(QIODeviceBase::ReadOnly)) {
        QTextStream StreamChangelog(&FileChangelog);
        QString     StringChangelog(StreamChangelog.readAll());
        ui->TextEditChangelog->setPlainText(StringChangelog);
    }

    // Open ToDo text
    // In case of failure, there is a place holder in the UI
    QFile FileToDo(":/DlgHelp/Docs/TODO.txt");
    if (FileToDo.open(QIODeviceBase::ReadOnly)) {
        QTextStream StreamToDo(&FileToDo);
        QString     StringToDo(StreamToDo.readAll());
        ui->TextEditToDo->setPlainText(StringToDo);
    }
}

DlgHelp::~DlgHelp()
{
    delete ui;
}

void DlgHelp::showDlgHelp()
{
    DlgHelp* Dlg = new DlgHelp;
    Dlg->exec();
    delete Dlg;
}
