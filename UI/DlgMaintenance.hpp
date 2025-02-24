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
    static bool execDlgMaintenance(QWidget* parent);

  private:
    DlgMaintenance(QWidget* parent);
    ~DlgMaintenance();
    Ui::DlgMaintenance* ui;
    bool                Fixed;

    void log(QString message);
};

#endif // DLGMAINTENANCE_HPP
