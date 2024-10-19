#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>

#include "segmentDisplay.h"

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

private:
    QGridLayout *mainLayout = nullptr;
    QPushButton *btn = nullptr;
    SegmentsDisplay display;
    QSlider *widthScaler = nullptr, *heightScaler = nullptr;
    QLabel  *minLbl = nullptr, *maxLbl = nullptr;
    QLabel *redLbl   = nullptr,
           *greenLbl = nullptr,
           *blueLbl  = nullptr;
    QSpinBox *redSpinBox   = nullptr,
             *greenSpinBox = nullptr,
             *blueSpinBox  = nullptr;
    int i;
};
#endif // DIALOG_H
