#include "DlgMaintenance.hpp"
#include "Event/EventCheckIndex.hpp"
#include "Index/Index.hpp"
#include "ui_DlgMaintenance.h"
#include <QCoreApplication>
#include <QScrollBar>
#include <QString>

DlgMaintenance::DlgMaintenance(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DlgMaintenance)
{
    ui->setupUi(this);
    connect(ui->ButtonClose, &QPushButton::clicked, [this]() { reject(); });
    connect(ui->ButtonsCheck, &QPushButton::clicked, [this]() { checkIndex(); });

    // Index messages
    connect(Index::instance(), &Index::checkingTrimming, this, [this]() { log(tr("Checking trimming...")); });
    connect(Index::instance(), &Index::trimmingCheckDone, this, [this](int count) { log(tr("Trimming checked, found %1 bad entries").arg(count)); });

    connect(Index::instance(), &Index::checkingDate, this, [this]() { log(tr("Checking dates...")); });
    connect(Index::instance(), &Index::dateCheckDone, this, [this](int count) { log(tr("Dates checked, found %1 bad entries").arg(count)); });

    connect(Index::instance(), &Index::checkingTechpub, this, [this]() { log(tr("Checking tech pubs...")); });
    connect(Index::instance(), &Index::techpubCheckDone, this, [this](int count) { log(tr("Tech pubs checked, found %1 bad entries").arg(count)); });
}

void DlgMaintenance::execDlgMaintenance(QWidget* parent)
{
    DlgMaintenance* Dlg = new DlgMaintenance(parent);
    Dlg->exec();
    delete Dlg;
}

DlgMaintenance::~DlgMaintenance()
{
    delete ui;
}

void DlgMaintenance::log(QString message)
{
    QString Log(ui->TextEditLog->toPlainText());
    Log.append(tr("%1\n").arg(message));

    // Auto-scroll
    ui->TextEditLog->setPlainText(Log);
    ui->TextEditLog->verticalScrollBar()->setValue(ui->TextEditLog->verticalScrollBar()->maximum());
}

/***********************************************************************************************************************
 *                                                                                                                     *
 *                                                     Index check                                                     *
 *                                                                                                                     *
 **********************************************************************************************************************/

void DlgMaintenance::checkIndex()
{
    log(tr("Starting Index analysis"));
    QCoreApplication::postEvent(Index::instance(), new EventCheckIndex());
}

