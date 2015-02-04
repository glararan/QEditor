/*This file is part of QEditor.

QEditor is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

QEditor is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with QEditor.  If not, see <http://www.gnu.org/licenses/>.*/

#include "tableview.h"

#include "mathhelper.h"

#include <QPainter>
#include <QPaintEvent>
#include <QStyleOptionFrame>
#include <QMenu>
#include <QMimeData>
#include <QApplication>
#include <QToolTip>
#include <QDrag>

TexturesArray::TexturesArray(int rows, int cols, QWidget* parent)
: QWidget(parent)
, nrows(rows)
, ncols(cols)
{
    d = 0;

    setFocusPolicy(Qt::StrongFocus);
    setAcceptDrops(true);

    cellw     = textureSize.width();
    cellh     = textureSize.height();
    curCol    = 0;
    curRow    = 0;
    selCol    = -1;
    selRow    = -1;
    itemCount = 0;

    toolTipArray.resize(rows * cols);
}

TexturesArray::~TexturesArray()
{
}

void TexturesArray::paintEvent(QPaintEvent* e)
{
    QRect r = e->rect();

    int cx = r.x();
    int cy = r.y();
    int ch = r.height();
    int cw = r.width();

    int colfirst = columnAt(cx);
    int collast  = columnAt(cx + cw);
    int rowfirst = rowAt(cy);
    int rowlast  = rowAt(cy + ch);

    if(isRightToLeft())
    {
        int t = colfirst;

        colfirst = collast;
        collast  = t;
    }

    QPainter painter(this);
    QPainter* p = &painter;

    QRect rect(0, 0, cellWidth(), cellHeight());

    if(collast < 0 || collast >= ncols)
        collast = ncols - 1;

    if(rowlast < 0 || rowlast >= nrows)
        rowlast = nrows - 1;

    // Go through the rows
    for(int r = rowfirst; r <= rowlast; ++r)
    {
        // get row position and height
        int rowp = rowY(r);

        // Go through the columns in the row r
        // if we know from where to where, go through [colfirst, collast],
        // else go through all of them
        for(int c = colfirst; c <= collast; ++c)
        {
            int margin = textureMargin.width() * c;

            // get position and width of column c
            int colp = columnX(c) + margin;

            // Translate painter and draw the cell
            rect.translate(colp, rowp);

            paintCell(p, r, c, rect);

            rect.translate(-colp, -rowp);
        }
    }
}

QSize TexturesArray::sizeHint() const
{
    ensurePolished();

    return gridSize().boundedTo(QSize(640, 480));
}

void TexturesArray::paintCell(QPainter* p, int row, int col, const QRect& rect)
{
    int index = col + row * numCols();

    if(index > itemCount)
        return;

    int b = 2; //margin

    const QPalette& g = palette();

    QStyleOptionFrame opt;

    int dfw = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

    opt.lineWidth    = dfw;
    opt.midLineWidth = 1;
    opt.rect         = rect.adjusted(b, b, -b, -b);
    opt.palette      = g;
    opt.state        = QStyle::State_Enabled | QStyle::State_Sunken;

    style()->drawPrimitive(QStyle::PE_Frame, &opt, p, this);

    b += dfw;

    if((row == curRow) && (col == curCol))
    {
        if (hasFocus())
        {
            QStyleOptionFocusRect opt;
            opt.palette = g;
            opt.rect    = rect;
            opt.state   = QStyle::State_None | QStyle::State_KeyboardFocusChange;

            style()->drawPrimitive(QStyle::PE_FrameFocusRect, &opt, p, this);
        }
    }

    paintCellContents(p, row, col, opt.rect.adjusted(dfw, dfw, -dfw, -dfw));
}

/*!
  Reimplement this function to change the contents of the well array.
 */
void TexturesArray::paintCellContents(QPainter* p, int row, int col, const QRect &r)
{
    if (d)
        p->fillRect(r, d->brush[row * numCols() + col]);
    else
    {
        p->fillRect(r, Qt::white);
        p->setPen(Qt::black);
        p->drawLine(r.topLeft(), r.bottomRight());
        p->drawLine(r.topRight(), r.bottomLeft());
    }
}

void TexturesArray::mousePressEvent(QMouseEvent* e)
{
    // The current cell marker is set to the cell the mouse is pressed in
    QPoint pos = e->pos();

    pos.setX(pos.x() - floor(MathHelper::toDouble(pos.x()) / MathHelper::toDouble(textureSize.width())) * textureMargin.width());
    pos.setY(pos.y() - floor(MathHelper::toDouble(pos.y()) / MathHelper::toDouble(textureSize.height())) * textureMargin.height());

    setCurrent(rowAt(pos.y()), columnAt(pos.x()));

    if(getToolTip(curRow, curCol) != QString())
    {
        QMimeData* mimeData = new QMimeData();
        mimeData->setText(getToolTip(curRow, curCol));

        QDrag* drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->setHotSpot(e->pos());
        drag->exec(Qt::CopyAction);
    }
}

void TexturesArray::mouseReleaseEvent(QMouseEvent* /* event */)
{
    // The current cell marker is set to the cell the mouse is clicked in
    setSelected(curRow, curCol);
}

void TexturesArray::dragEnterEvent(QDragEnterEvent* event)
{
    if(event->source() == this)
    {
        // The current cell marker is set to the cell the mouse is clicked in
        setSelected(curRow, curCol);

        event->ignore();
    }
}

bool TexturesArray::event(QEvent* e)
{
    if(e->type() == QEvent::ToolTip)
    {
        QHelpEvent* help = static_cast<QHelpEvent*>(e);

        QPoint pos = help->pos();
        pos.setX(pos.x() - floor(MathHelper::toDouble(pos.x()) / MathHelper::toDouble(textureSize.width())) * textureMargin.width());
        pos.setY(pos.y() - floor(MathHelper::toDouble(pos.y()) / MathHelper::toDouble(textureSize.height())) * textureMargin.height());

        int cellIndex = columnAt(pos.x());
        int rowIndex  = rowAt(pos.y());

        if((cellIndex > -1 && rowIndex > -1) && ncols * rowIndex + cellIndex >= 0 && ncols * rowIndex + cellIndex < toolTipArray.count())
        {
            if(toolTipArray[ncols * rowIndex + cellIndex] != "")
            {
                QToolTip::showText(help->globalPos(), toolTipArray[ncols * rowIndex + cellIndex]);

                return true;
            }
        }

        QToolTip::hideText();

        e->ignore();

        return true;
    }

    return QWidget::event(e);
}

/*
  Sets the cell currently having the focus. This is not necessarily
  the same as the currently selected cell.
*/

void TexturesArray::setCurrent(int row, int col)
{
    if((curRow == row) && (curCol == col))
        return;

    if(row < 0 || col < 0)
        row = col = -1;

    int oldRow = curRow;
    int oldCol = curCol;

    curRow = row;
    curCol = col;

    updateCell(oldRow, oldCol);
    updateCell(curRow, curCol);
}

/*
  Sets the currently selected cell to \a row, \a column. If \a row or
  \a column are less than zero, the current cell is unselected.

  Does not set the position of the focus indicator.
*/
void TexturesArray::setSelected(int row, int col)
{
    if((numRows() <= row) || (numCols() <= col))
        return;

    int oldRow = selRow;
    int oldCol = selCol;

    if(row < 0 || col < 0)
        row = col = -1;

    selCol = col;
    selRow = row;

    updateCell(oldRow, oldCol);
    updateCell(selRow, selCol);

    if(row >= 0)
        itemSelected(row, col);

#ifndef QT_NO_MENU
    if(isVisible() && qobject_cast<QMenu*>(parentWidget()))
        parentWidget()->close();
#endif
}

void TexturesArray::focusInEvent(QFocusEvent*)
{
    updateCell(curRow, curCol);
}

void TexturesArray::setCellBrush(int row, int col, const QBrush &b)
{
    if (!d)
    {
        d = new TexturesData;

        int i = numRows()*numCols();

        d->brush = new QBrush[i];
    }

    if(row >= 0 && row < numRows() && col >= 0 && col < numCols())
        d->brush[row * numCols() + col] = b;
}

/*
  Returns the brush set for the cell at \a row, \a column. If no brush is
  set, Qt::NoBrush is returned.
*/

QBrush TexturesArray::cellBrush(int row, int col)
{
    if(d && row >= 0 && row < numRows() && col >= 0 && col < numCols())
        return d->brush[row * numCols() + col];

    return Qt::NoBrush;
}

void TexturesArray::setTextureSize(const QSize size)
{
    textureSize = size;

    cellw = textureSize.width();
    cellh = textureSize.height();
}

QRect TexturesArray::cellGeometry(int row, int column)
{
    QRect r;

    if(row >= 0 && row < nrows && column >= 0 && column < ncols)
        r.setRect(columnX(column) + textureMargin.width() * column, rowY(row) + textureMargin.height() * row, cellw, cellh);

    return r;
}

void TexturesArray::updateToolTip(int row, int column, QString toolTip)
{
    toolTipArray[ncols * row + column] = toolTip;
}

QString TexturesArray::getToolTip(int row, int column)
{
    if(toolTipArray.count() <= ncols * row + column)
        return QString();

    return toolTipArray[ncols * row + column];
}

void TexturesArray::itemSelected(int row, int col)
{
    emit selected(row,col);
}

/*!\reimp
*/

void TexturesArray::focusOutEvent(QFocusEvent*)
{
    updateCell(curRow, curCol);
}

/*\reimp
*/
void TexturesArray::keyPressEvent(QKeyEvent* e)
{
    switch(e->key()) // Look at the key code
    {
    case Qt::Key_Left:                                // If 'left arrow'-key,
        if(curCol > 0)                                // and cr't not in leftmost col
            setCurrent(curRow, curCol - 1);           // set cr't to next left column
        break;

    case Qt::Key_Right:                               // Correspondingly...
        if(curCol < numCols()-1)
            setCurrent(curRow, curCol + 1);
        break;

    case Qt::Key_Up:
        if(curRow > 0)
            setCurrent(curRow - 1, curCol);
        break;

    case Qt::Key_Down:
        if(curRow < numRows()-1)
            setCurrent(curRow + 1, curCol);
        break;
#if 0
        // bad idea that shouldn't have been implemented; very counterintuitive
    case Qt::Key_Return:
    case Qt::Key_Enter:
        /*
              ignore the key, so that the dialog get it, but still select
              the current row/col
            */
        e->ignore();
        // fallthrough intended
#endif
    case Qt::Key_Space:
        setSelected(curRow, curCol);
        break;

    default:                                // If not an interesting key,
        e->ignore();                        // we don't accept the event
        return;
    }
}

///////////////////////////////////
/// TextureWell
///////////////////////////////////
TextureWell::TextureWell(QWidget* parent, int row, int columns, QSize textureIconSize, QSize textureIconMargin)
: TexturesArray(row, columns, parent)
, mousePressed(false)
, oldCurrent(-1, -1)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));

    setTextureSize(textureIconSize);
    setTextureMargin(textureIconMargin);

    values.resize(row * columns);

    setItemCount(0);
    setDefaultImages();
}

void TextureWell::clear()
{
    values.clear();
    values.resize(numCols() * numRows());

    setItemCount(0);
    setDefaultImages();

    TexturesArray::clear();
}

void TextureWell::insertItem(QImage image, QString toolTip, bool makeSpace)
{
    if(hasSpace())
    {
        int index = firstFreeIndex();

        values[index].first         = image.scaled(TextureSize());
        values[index].second.isFree = false;

        insertToolTip(index, toolTip);

        setItemCount(index);
    }
    else if(makeSpace)
    {
        setRows(numRows() + 1);
        insertItem(image, toolTip, false);
    }
}

void TextureWell::insertItems(QVector<QPair<QImage, QString> > vals, bool makeSpace)
{
    for(int i = 0; i < vals.count(); ++i)
        insertItem(vals.at(i).first, vals.at(i).second, makeSpace);
}

void TextureWell::updateItem(int row, int column, QImage image, QString toolTip)
{
    if(row >= numRows() || column >= numCols())
        return;

    values[(row * numCols()) + column].first         = image.scaled(TextureSize());
    values[(row * numCols()) + column].second.isFree = false;

    updateToolTip(row, column, toolTip);
}

QImage TextureWell::getImage(int row, int column)
{
    return values[(row * numCols()) + column].first;
}

void TextureWell::setRows(const int count)
{
    values.resize(numCols() * count);

    setDefaultImages();

    TexturesArray::setRows(count);
}

void TextureWell::setCols(const int count)
{
    values.resize(numRows() * count);

    setDefaultImages();

    TexturesArray::setCols(count);
}

void TextureWell::paintCellContents(QPainter* p, int row, int col, const QRect& r)
{
    int i = col + row * numCols();

    if(i >= values.count())
        return;

    p->fillRect(r, QBrush(QPixmap::fromImage(values.at(i).first.scaled(TextureSize()))));
}

void TextureWell::mousePressEvent(QMouseEvent* e)
{
    oldCurrent = QPoint(selectedRow(), selectedColumn());

    TexturesArray::mousePressEvent(e);

    mousePressed = true;

    pressPos = e->pos();


}

void TextureWell::mouseReleaseEvent(QMouseEvent* e)
{
    if(!mousePressed)
        return;

    TexturesArray::mouseReleaseEvent(e);

    mousePressed = false;
}

void TextureWell::itemSelected(int row, int col)
{
    if(!values[(row * numCols()) + col].second.isFree)
        emit selected(row, col);
}

int TextureWell::firstFreeIndex()
{
    for(int i = 0; i < values.size(); ++i)
    {
        if(values.at(i).second.isFree)
            return i;
    }

    return -1;
}

bool TextureWell::hasSpace()
{
    for(int i = 0; i < values.size(); ++i)
    {
        if(values.at(i).second.isFree)
            return true;
    }

    return false;
}

void TextureWell::setDefaultImages()
{
    QImage defaultimage(1, 1, QImage::Format_ARGB32);

    defaultimage.fill(Qt::transparent);

    for(int i = 0; i < values.size(); ++i)
    {
        if(values.at(i).first.isNull())
            values[i].first = defaultimage;
    }
}
