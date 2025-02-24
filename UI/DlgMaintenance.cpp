#include "DlgMaintenance.hpp"
#include "Event/EventCheckIndex.hpp"
#include "Event/EventFixIndex.hpp"
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
    connect(ui->ButtonsCheck, &QPushButton::clicked, [this]() {
        log(tr("Starting Index analysis"));
        QCoreApplication::postEvent(Index::instance(), new EventCheckIndex);
    });
    connect(ui->ButtonFix, &QPushButton::clicked, [this]() {
        log(tr("Fixing index"));
        QCoreApplication::postEvent(Index::instance(), new EventFixIndex);
    });
    connect(ui->ButtonClose, &QPushButton::clicked, [this]() { reject(); });

    // Index messages
    connect(Index::instance(), &Index::checkingDone, this, [this](int count) {
        log(tr("Checking done, total of %1 bad entries found\n").arg(count));
        ui->ButtonFix->setEnabled(count != 0);
    });
    connect(Index::instance(), &Index::fixingDone, this, [this](int count) {
        log(tr("Fixing done, total of %1 bad entries fixed\n").arg(count));
        ui->ButtonFix->setEnabled(false);
    });

    connect(Index::instance(), &Index::checkingTrimming, this, [this]() { log(tr("Checking trimming...")); });
    connect(Index::instance(), &Index::trimmingCheckDone, this, [this](int count) { log(tr("Trimming checked, %1 bad entries found").arg(count)); });

    connect(Index::instance(), &Index::checkingDate, this, [this]() { log(tr("Checking dates...")); });
    connect(Index::instance(), &Index::dateCheckDone, this, [this](int count) { log(tr("Dates checked, %1 bad entries found").arg(count)); });

    connect(Index::instance(), &Index::checkingTechpub, this, [this]() { log(tr("Checking tech pubs...")); });
    connect(Index::instance(), &Index::techpubCheckDone, this, [this](int count) { log(tr("Tech pubs checked, %1 bad entries found").arg(count)); });
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
