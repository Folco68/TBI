#ifndef DLGSETTINGS_HPP
#define DLGSETTINGS_HPP

#include <QDialog>

namespace Ui {
class DlgSettings;
}

class DlgSettings : public QDialog
{
    Q_OBJECT

  public:
    static void showDlgSettings(QWidget* parent = nullptr);
    ~DlgSettings();

  private:
    Ui::DlgSettings *ui;

    explicit DlgSettings(QWidget* parent = nullptr);
};

#endif // DLGSETTINGS_HPP
