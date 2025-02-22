#ifndef DLGMAINTENANCE_HPP
#define DLGMAINTENANCE_HPP

#include <QDialog>

namespace Ui {
class DlgMaintenance;
}

class DlgMaintenance : public QDialog
{
    Q_OBJECT

  public:
    static void execDlgMaintenance(QWidget* parent);

  private:
    DlgMaintenance(QWidget* parent);
    ~DlgMaintenance();
    Ui::DlgMaintenance* ui;

    void log(QString message);
    void checkIndex();
};

#endif // DLGMAINTENANCE_HPP
