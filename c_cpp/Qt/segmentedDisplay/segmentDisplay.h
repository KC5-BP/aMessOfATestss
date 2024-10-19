#ifndef SEGMENTDISPLAY_H
#define SEGMENTDISPLAY_H

#include <QColor>
#include <QList>
#include <QPainterPath>
#include <QWidget>

class SegmentsDisplay: public QWidget {

public:
    SegmentsDisplay(QWidget *pQParent = nullptr);
    virtual ~SegmentsDisplay() = default;

    SegmentsDisplay(const SegmentsDisplay&) = delete;
    SegmentsDisplay& operator=(const SegmentsDisplay&) = delete;

    void setSegmentColor(int idx, QColor color);
    void setPainterScaleX(qreal _scaleX);
    void setPainterScaleY(qreal _scaleY);

protected:
    virtual void paintEvent(QPaintEvent *pQEvent) override;

private:
    QList<QColor> segsColor;
    QList<QPainterPath> paths;
    qreal scaleX, scaleY;
};

#endif // SEGMENTDISPLAY_H
