/*
    Copyright 2011 Arne Jacobs

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "graphview.h"
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>
#include <QDebug>
#include <QItemSelectionRange>
#include <cmath>

GraphView::GraphView(QWidget *parent) :
    QAbstractItemView(parent),
    bars_(1),
    spacing_(10),
    horizontalScale_(0)
{
    QColor transparent = Qt::gray;
    transparent.setAlphaF(0.5f);
    selectionBrush_ = QBrush(transparent);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setViewportMargins(0, 0, 0, 0);
    // set scrollbar properties:
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // add a few pens:
    pens.append(QPen(Qt::gray));
    pens.append(QPen(Qt::red));
    pens.append(QPen(Qt::green));
    pens.append(QPen(Qt::blue));
    pens.append(QPen(Qt::yellow));
    pens.append(QPen(Qt::magenta));
    pens.append(QPen(Qt::cyan));
}

int GraphView::bars() const
{
    return bars_;
}

void GraphView::setBars(int value)
{
    Q_ASSERT(value > 0);
    bars_ = value;
    setDirtyRegion(viewport()->rect());
}

int GraphView::spacing() const
{
    return spacing_;
}

void GraphView::setSpacing(int value)
{
    Q_ASSERT(value >= 0);
    spacing_ = value;
    setDirtyRegion(viewport()->rect());
}

int GraphView::horizontalScale() const
{
    return horizontalScale_;
}

void GraphView::setHorizontalScale(int value)
{
    Q_ASSERT(value >= 0);
    // get the current leftmost index:
    QModelIndex index = indexAt(QPoint(0, 0));
    horizontalScale_ = value;
    updateGeometries();
    setDirtyRegion(viewport()->rect());
    // center on the previously centered index:
    if (index.isValid())
        scrollTo(index, QAbstractItemView::PositionAtTop);
}

const QBrush & GraphView::selectionBrush() const {
    return selectionBrush_;
}

void GraphView::setSelectionBrush(const QBrush &brush)
{
    selectionBrush_ = brush;
    // make the brush transparent:
    QColor transparent = selectionBrush_.color();
    transparent.setAlphaF(0.5f);
    selectionBrush_.setColor(transparent);
    setDirtyRegion(viewport()->rect());
}

void GraphView::setModel ( QAbstractItemModel * p_model )
{
    // disconnect the previous model:
    if (model()) {
        QObject::disconnect(model(), SIGNAL(columnsInserted(QModelIndex,int,int)), this, SLOT(columnsInserted(QModelIndex,int,int)));
        QObject::disconnect(model(), SIGNAL(columnsRemoved(QModelIndex,int,int)), this, SLOT(columnsRemoved(QModelIndex,int,int)));
        QObject::disconnect(model(), SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rowsInserted(QModelIndex,int,int)));
        QObject::disconnect(model(), SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(rowsRemoved(QModelIndex,int,int)));
    }
    // invoke the super class implementation:
    QAbstractItemView::setModel(p_model);
    // connect to the new model:
    if (model()) {
        QObject::connect(model(), SIGNAL(columnsInserted(QModelIndex,int,int)), this, SLOT(columnsInserted(QModelIndex,int,int)));
        QObject::connect(model(), SIGNAL(columnsRemoved(QModelIndex,int,int)), this, SLOT(columnsRemoved(QModelIndex,int,int)));
        QObject::connect(model(), SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rowsInserted(QModelIndex,int,int)));
        QObject::connect(model(), SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(rowsRemoved(QModelIndex,int,int)));
    }
}

QRect GraphView::visualRect(const QModelIndex &index) const
{
    QPoint offset(horizontalScrollBar()->value(), verticalScrollBar()->value());
    int heightWithoutSpacing = viewport()->height() - spacing() * (bars() - 1);
    int barHeight = heightWithoutSpacing / bars();
    int barTop = (barHeight + spacing()) * (index.column() % bars());
    int rowsPerPixel = 1 << horizontalScale();
    return QRect(QPoint(index.row() / rowsPerPixel, barTop) - offset, QSize(1, barHeight));
}

void GraphView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    QRect rect = visualRect(index);
    if (hint == QAbstractItemView::EnsureVisible) {
        // test if the item is already visible:
        bool visible = viewport()->rect().contains(rect);
        if (!visible) {
            // make it visible:
            if (rect.x() < 0) {
                // we use PositionAtTop as a metaphor for the nonexisting PositionAtLeft here:
                scrollTo(index, QAbstractItemView::PositionAtTop);
            } else {
                // we use PositionAtBottom as a metaphor for the nonexisting PositionAtRight here:
                scrollTo(index, QAbstractItemView::PositionAtBottom);
            }
        }
    } else if (hint == QAbstractItemView::PositionAtBottom) {
        // we use PositionAtBottom as a metaphor for the nonexisting PositionAtRight here:
        int scrollBarPosition = horizontalScrollBar()->value() - viewport()->width() + 1 + rect.x();
        horizontalScrollBar()->setValue(scrollBarPosition);
    } else if (hint == QAbstractItemView::PositionAtCenter) {
        int scrollBarPosition = horizontalScrollBar()->value() - viewport()->width() / 2 + rect.x();
        horizontalScrollBar()->setValue(scrollBarPosition);
    } else if (hint == QAbstractItemView::PositionAtTop) {
        // we use PositionAtTop as a metaphor for the nonexisting PositionAtLeft here:
        int scrollBarPosition = horizontalScrollBar()->value() + rect.x();
        horizontalScrollBar()->setValue(scrollBarPosition);
    }
}

QModelIndex GraphView::indexAt(const QPoint &point) const
{
    if (!model())
        return QModelIndex();
    QPoint offset(horizontalScrollBar()->value(), verticalScrollBar()->value());
    QPoint pointOffset = point + offset;
    int rowsPerPixel = 1 << horizontalScale();
    int row = pointOffset.x() * rowsPerPixel;
    if (row >= model()->rowCount())
        return QModelIndex();
    int heightWithoutSpacing = height() - spacing() * (bars() - 1);
    int barHeight = heightWithoutSpacing / bars();
    int col = pointOffset.y() / (barHeight + spacing());
    if (pointOffset.y() - col * (barHeight + spacing()) >= barHeight)
        return QModelIndex();
    return model()->index(row, col);
}

QModelIndex GraphView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers)
{
    // get the current index:
    QModelIndex index = selectionModel()->currentIndex();
    // change it based on the given curserAction:
    if ((cursorAction == QAbstractItemView::MoveLeft) || (cursorAction == QAbstractItemView::MovePrevious)) {
        return model()->index(qMax(0, index.row() - 1), index.column());
    } else if ((cursorAction == QAbstractItemView::MoveRight) || (cursorAction == QAbstractItemView::MoveNext)) {
        return model()->index(qMin(index.row() + 1, model()->rowCount() - 1), index.column());
    } else if (cursorAction == QAbstractItemView::MoveHome) {
        return model()->index(0, index.column());
    } else if (cursorAction == QAbstractItemView::MoveEnd) {
        return model()->index(model()->rowCount() - 1, index.column());
    } else if (cursorAction == QAbstractItemView::MovePageUp) {
        return model()->index(qMax(0, index.row() - viewport()->width() + 1), index.column());
    } else if (cursorAction == QAbstractItemView::MovePageDown) {
        return model()->index(qMin(index.row() + viewport()->width() - 1, model()->rowCount() - 1), index.column());
    } else
        return QModelIndex();
}

int GraphView::horizontalOffset() const
{
    return horizontalScrollBar()->value();
}

int GraphView::verticalOffset() const
{
    return verticalScrollBar()->value();
}

bool GraphView::isIndexHidden(const QModelIndex &) const
{
    // no items can be hidden in this view:
    return false;
}

void GraphView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    QItemSelection selection;
    // add all items that are within the given rectangle:
    QModelIndex topLeft = indexAt(rect.normalized().topLeft());
    QModelIndex bottomRight = indexAt(rect.normalized().bottomRight());
    QItemSelectionRange selectionRange(topLeft, bottomRight);
    selection.append(selectionRange);
    // also add all corresponding columns in other layers:
    for (int col = topLeft.column(); col <= bottomRight.column(); col++) {
        for (int colCorresponding = col + bars(); colCorresponding < model()->columnCount(); colCorresponding += bars()) {
            selection.append(QItemSelectionRange(model()->index(topLeft.row(), colCorresponding), model()->index(bottomRight.row(), colCorresponding)));
        }
    }
    selectionModel()->select(selection, command);
}

QRegion GraphView::visualRegionForSelection(const QItemSelection &selection) const
{
    QRegion region;
    for (int range = 0; range < selection.size(); range++) {
        int firstRow = selection[range].top();
        int lastRow = selection[range].bottom();
        for (int col = selection[range].left(); col <= selection[range].right(); col++) {
            region += QRect(visualRect(model()->index(firstRow, col)).topLeft(), visualRect(model()->index(lastRow, col)).bottomRight());
        }
    }
    return region;
}

void GraphView::paintEvent(QPaintEvent *p_event)
{
    if (model()) {
        // paint only the rectangle given by p_event->rect()
        // create a painter:
        QPainter painter(viewport());
        QRect rect = p_event->rect();
        // update the rectangle with the scrollbar values:
        QPoint offset(horizontalScrollBar()->value(), verticalScrollBar()->value());
        rect.translate(offset);
        // draw each sample in the given area:

        int rowsPerPixel = 1 << horizontalScale();
        // draw from the sample BEFORE the first visible sample (if such exists):
        int startRow = rect.x() * rowsPerPixel;
        if (startRow > 0)
            startRow -= rowsPerPixel;

        if (startRow < model()->rowCount()) {
            // draw to the sample AFTER the last visible sample (if such exists):
            int endRow = (rect.x() + rect.width()) * rowsPerPixel;
            if (endRow >= model()->rowCount())
                endRow = model()->rowCount() - 1;

            int heightWithoutSpacing = viewport()->height() - spacing() * (bars() - 1);
            int barHeight = heightWithoutSpacing / bars();
            for (int col = model()->columnCount() - 1; col >= 0; col--) {
                QPen penLight = pens[(col / bars()) % pens.size()];
                QPen pen = QPen(penLight.color().darker());

                int bar = col % bars();

                QPoint minPointFrom, maxPointFrom;
                for (int row = startRow; row <= endRow; row += rowsPerPixel) {
                    QPoint top, bottom;
//                    float mean, variance;
//                    getStatistics(row, col, mean, variance);
//                    float radius = sqrt(variance);
//                    top = QPoint(row / rowsPerPixel, qRound((1.0f - mean - radius) * 0.5f * (float)(barHeight - 1)) + (barHeight + spacing()) * bar);
//                    bottom = QPoint(row / rowsPerPixel, qRound((1.0f - mean + radius) * 0.5f * (float)(barHeight - 1)) + (barHeight + spacing()) * bar);
                    float min, max;
                    getBounds(row, col, min, max);
//                    if (min >= 0.0f) {
//                        top = QPoint(row / rowsPerPixel, qRound((1.0f - max) * 0.5f * (float)(barHeight - 1)) + (barHeight + spacing()) * bar);
//                        bottom = (rowsPerPixel == 0 ? top : QPoint(row / rowsPerPixel, qRound((1.0f - 0.0f) * 0.5f * (float)(barHeight - 1)) + (barHeight + spacing()) * bar));
//                    } else if (max <= 0.0f) {
//                        bottom = QPoint(row / rowsPerPixel, qRound((1.0f - min) * 0.5f * (float)(barHeight - 1)) + (barHeight + spacing()) * bar);
//                        top = (rowsPerPixel == 0 ? bottom : QPoint(row / rowsPerPixel, qRound((1.0f - 0.0f) * 0.5f * (float)(barHeight - 1)) + (barHeight + spacing()) * bar));
//                    } else {
                        top = QPoint(row / rowsPerPixel, qRound((1.0f - max) * 0.5f * (float)(barHeight - 1)) + (barHeight + spacing()) * bar);
                        bottom = QPoint(row / rowsPerPixel, qRound((1.0f - min) * 0.5f * (float)(barHeight - 1)) + (barHeight + spacing()) * bar);
//                    }
                    if (row == startRow) {
                        minPointFrom = bottom;
                        maxPointFrom = top;
                        painter.setPen(penLight);
                        painter.drawLine(minPointFrom - offset, maxPointFrom - offset);
                    } else {
                        QPoint minPointTo = bottom;
                        QPoint maxPointTo = top;
                        painter.setPen(penLight);
                        painter.drawLine(minPointTo - offset, maxPointTo - offset);
                        painter.setPen(pen);
                        painter.drawLine(minPointFrom - offset, minPointTo - offset);
                        painter.drawLine(maxPointFrom - offset, maxPointTo - offset);
                        minPointFrom = minPointTo;
                        maxPointFrom = maxPointTo;
                    }
                }
            }
        }
        // highlight the current selection:
        QRegion selectedRegion = visualRegionForSelection(selectionModel()->selection());
        for (int i = 0; i < selectedRegion.rectCount(); i++) {
            painter.fillRect(selectedRegion.rects()[i], selectionBrush());
        }
        // highlight the current item:
        if (selectionModel()->currentIndex().isValid()) {
            QPen penLight = pens[(selectionModel()->currentIndex().column() / bars()) % pens.size()];
            QRect rect = visualRect(selectionModel()->currentIndex());
            painter.fillRect(rect, penLight.color().darker());
        }
    }
}

void GraphView::updateGeometries()
{
    // update the scrollbar:
    if (model()) {
        int rowsPerPixel = 1 << horizontalScale();
        int widthOfAllRows = (model()->rowCount() + rowsPerPixel - 1) / rowsPerPixel;
        horizontalScrollBar()->setRange(0, widthOfAllRows - viewport()->width());
        horizontalScrollBar()->setPageStep(viewport()->width() - 1);
    }
    QAbstractItemView::updateGeometries();
}

void GraphView::getBounds(int row, int col, float &min, float&max)
{
    Q_ASSERT(model());
    int rowsPerPixel = 1 << horizontalScale();
    int firstRow = (row / rowsPerPixel) * rowsPerPixel;
    int lastRow = qMin(model()->rowCount(), firstRow + rowsPerPixel) - 1;
    min = max = model()->data(model()->index(firstRow, col)).toFloat();
    for (int row = firstRow + 1; row <= lastRow; row++) {
        float value = model()->data(model()->index(row, col)).toFloat();
        if (value < min)
            min = value;
        if (value > max)
            max = value;
    }
}

void GraphView::getStatistics(int row, int col, float &mean, float &variance)
{
    Q_ASSERT(model());
    int rowsPerPixel = 1 << horizontalScale();
    int firstRow = (row / rowsPerPixel) * rowsPerPixel;
    int lastRow = qMin(model()->rowCount(), firstRow + rowsPerPixel) - 1;
    float sum = 0.0f;
    int count = 0;
    for (int row = firstRow; row <= lastRow; row++) {
        float value = model()->data(model()->index(row, col)).toFloat();
        sum += value;
        count++;
    }
    mean = (count ? sum / (float)count : 0.0f);
    float sumOfSquaredDifferences = 0.0f;
    for (int row = firstRow; row <= lastRow; row++) {
        float value = model()->data(model()->index(row, col)).toFloat();
        float diff = value - mean;
        sumOfSquaredDifferences += diff * diff;
    }
    variance = (count ? sumOfSquaredDifferences / (float)count : 0.0f);
}

void GraphView::rowsInserted ( const QModelIndex &, int, int )
{
    updateGeometries();
    viewport()->update();
}

void GraphView::rowsRemoved ( const QModelIndex &, int, int )
{
    updateGeometries();
    viewport()->update();
}

void GraphView::columnsInserted ( const QModelIndex &, int, int )
{
    updateGeometries();
    viewport()->update();
}

void GraphView::columnsRemoved ( const QModelIndex &, int, int )
{
    updateGeometries();
    viewport()->update();
}

