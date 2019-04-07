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

#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QWidget>
#include <QFrame>
#include <QDragLeaveEvent>

struct TexturesData
{
    QBrush* brush;
};

class TexturesArray : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(int selectedColumn READ selectedColumn)
    Q_PROPERTY(int selectedRow READ selectedRow)

public:
    TexturesArray(int rows, int cols, QWidget* parent = 0);
    ~TexturesArray();

    QString cellContent(int row, int col) const;

    int selectedColumn() const { return selCol; }
    int selectedRow() const    { return selRow; }

    virtual void setCurrent(int row, int col);
    virtual void setSelected(int row, int col);

    QSize sizeHint() const;

    virtual void setCellBrush(int row, int col, const QBrush&);

    QBrush cellBrush(int row, int col);

    inline int cellWidth() const  { return cellw; }
    inline int cellHeight() const { return cellh; }
    inline int rowAt(int y) const { return y / cellh; }

    inline int columnAt(int x) const
    {
        if(isRightToLeft())
            return (ncols - (x / cellw) - 1);

        return (x / cellw);
    }

    inline int rowY(int row) const { return cellh * row; }

    inline int columnX(int column) const
    {
        if(isRightToLeft())
            return cellw * (ncols - column - 1);

        return cellw * column;
    }

    inline int numRows() const  { return nrows; }
    inline int numCols() const  { return ncols; }
    inline int numItems() const { return itemCount; }

    void setRows(const int count) { nrows = count; if(toolTipArray.count() < nrows * ncols) toolTipArray.resize(nrows * ncols); }
    void setCols(const int count) { ncols = count; if(toolTipArray.count() < nrows * ncols) toolTipArray.resize(nrows * ncols); }

    void setItemCount(const int count) { itemCount = count; }

    void clear()
    {
        toolTipArray.clear();
        toolTipArray.resize(nrows * ncols);
    }

    void insertToolTip(int index, QString toolTip)
    {
        toolTipArray[index] = toolTip;
    }

    inline QSize TextureSize() const   { return textureSize; }
    inline QSize TextureMargin() const { return textureMargin; }

    void setTextureSize(const QSize size);

    void setTextureMargin(const QSize margins) { textureMargin = margins; }

    void clearToolTip() { toolTipArray.clear(); }

    inline QRect cellRect() const { return QRect(0, 0, cellw, cellh); }
    inline QSize gridSize() const { return QSize(ncols * cellw, nrows * cellh); }

    QRect cellGeometry(int row, int column);
    void updateToolTip(int row, int column, QString toolTip);
    QString getToolTip(int row, int column);

    inline void updateCell(int row, int column) { update(cellGeometry(row, column)); }

signals:
    void selected(int row, int col);

protected:
    virtual void itemSelected(int row, int col);
    virtual void paintCell(QPainter*, int row, int col, const QRect&);
    virtual void paintCellContents(QPainter*, int row, int col, const QRect&);

    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void dragEnterEvent(QDragEnterEvent*);
    void keyPressEvent(QKeyEvent*);
    void focusInEvent(QFocusEvent*);
    void focusOutEvent(QFocusEvent*);
    void paintEvent(QPaintEvent*);

    bool event(QEvent*);

private:
    Q_DISABLE_COPY(TexturesArray)

    int nrows;
    int ncols;
    int cellw;
    int cellh;
    int curRow;
    int curCol;
    int selRow;
    int selCol;
    int itemCount;

    QSize textureSize;
    QSize textureMargin;

    QVector<QString> toolTipArray;

    TexturesData* d;
};

struct Slot
{
    Slot(bool free = true)
    {
        isFree = free;
    }

    bool isFree;
};

class TextureWell : public TexturesArray
{
public:
    TextureWell(QWidget* parent, int row, int columns, QSize textureIconSize, QSize textureIconMargin);

    void clear();

    void insertItem(QImage image, QString toolTip, bool makeSpace = true);
    void insertItems(QVector<QPair<QImage, QString>> vals, bool makeSpace = true);

    void updateItem(int row, int column, QImage image, QString toolTip);

    QImage getImage(int row, int column);

    void setRows(const int count);
    void setCols(const int count);

protected:
    void paintCellContents(QPainter*, int row, int col, const QRect&);

    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);

    void itemSelected(int row, int col);

private:
    int firstFreeIndex();

    bool hasSpace();

    void setDefaultImages();

    QVector<QPair<QImage, Slot> > values;

    bool mousePressed;

    QPoint pressPos;
    QPoint oldCurrent;
};

#endif // TABLEVIEW_H
