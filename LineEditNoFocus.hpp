#ifndef LINEEDITNOFOCUS_HPP
#define LINEEDITNOFOCUS_HPP

#include <QFocusEvent>
#include <QLineEdit>

//  LineEditNoFocus
//
// This class prevent the QLineEdit widget to have its text selected when it get focused
// I don't know why the text is selected when compiling with MSVS, not with MinGW
//
class LineEditNoFocus : public QLineEdit
{
  public:
    LineEditNoFocus(QWidget* parent);
    ~LineEditNoFocus() override;

  protected:
    void focusInEvent(QFocusEvent* event) override;
};

#endif // LINEEDITNOFOCUS_HPP
