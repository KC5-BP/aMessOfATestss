#include "dialog.h"

#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>

Dialog::Dialog(QWidget *parent) : QDialog(parent), i(0),
    redLbl(new QLabel("Red:")),
    greenLbl(new QLabel("Green:")),
    blueLbl(new QLabel("Blue:")) {

    redSpinBox   = new QSpinBox;
    redSpinBox->setRange(0, 255);
    redSpinBox->setValue(redSpinBox->minimum() +
                         (redSpinBox->maximum() - redSpinBox->minimum())/2);

    greenSpinBox = new QSpinBox;
    greenSpinBox->setRange(redSpinBox->minimum(), redSpinBox->maximum());
    greenSpinBox->setValue(redSpinBox->value());

    blueSpinBox  = new QSpinBox;
    blueSpinBox->setRange(redSpinBox->minimum(), redSpinBox->maximum());
    blueSpinBox->setValue(redSpinBox->value());

    btn = new QPushButton(QString("Change color of segment [%1]").arg(i));
    connect(btn, &QPushButton::clicked, this, [=](bool clicked) {
        display.setSegmentColor(i, QColor(redSpinBox->value(),
                                          greenSpinBox->value(),
                                          blueSpinBox->value() ));
        display.update();

        i = (i+1) % 7;
        btn->setText(QString("Change color of segment [%1]").arg(i));
    });

    minLbl = new QLabel("1");
    maxLbl = new QLabel("200");

    widthScaler = new QSlider;
    widthScaler->setTickInterval(10);
    widthScaler->setRange(minLbl->text().toInt(),
                          maxLbl->text().toInt());
    widthScaler->setValue(minLbl->text().toInt() +
                          (maxLbl->text().toInt() - minLbl->text().toInt())/2);
    widthScaler->setOrientation(Qt::Orientation::Horizontal);
    connect(widthScaler, &QSlider::valueChanged, this, [=](int value){
        display.setPainterScaleX(value/100.);
        display.update();
    });

    heightScaler = new QSlider;
    heightScaler->setTickInterval(widthScaler->tickInterval());
    heightScaler->setRange(widthScaler->minimum(), widthScaler->maximum());
    heightScaler->setValue(widthScaler->value());
    heightScaler->setOrientation(Qt::Orientation::Vertical);
    heightScaler->setInvertedAppearance(true);
    connect(heightScaler, &QSlider::valueChanged, this, [=](int value){
        display.setPainterScaleY(value/100.);
        display.update();
    });

    mainLayout = new QGridLayout(this);
    /* Line 0 */
    mainLayout->addWidget(redLbl, 0, 0);
    mainLayout->addWidget(redSpinBox, 0, 1);
    mainLayout->addWidget(greenLbl, 0, 2);
    mainLayout->addWidget(greenSpinBox, 0, 3);
    mainLayout->addWidget(blueLbl, 0, 4);
    mainLayout->addWidget(blueSpinBox, 0, 5);

    /* Line 1 */
    mainLayout->addWidget(btn, 1, 0, 1, 6);

    /* Line 2 */
    mainLayout->addWidget(&display, 2, 0, 1, 5);
    mainLayout->addWidget(heightScaler, 2, 5);

    /* Line 3 */
    mainLayout->addWidget(minLbl, 3, 0, 1, 1);
    mainLayout->addWidget(widthScaler, 3, 1, 1, 4);
    mainLayout->addWidget(maxLbl, 3, 5, 1, 1);

    resize(401, 753);   // Set by testing values approximately
}

Dialog::~Dialog() {}
