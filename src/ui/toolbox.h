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

#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <QtWidgets>
#include <QtCore>

class ToolItem : public QWidget {
public:
    ToolItem(const QString &t, QWidget *item);
    QWidget *getWidget();

protected:
    void mousePressEvent(QMouseEvent *event);

private:
    QLabel *title;
    QWidget *item;
};

class ToolBox : public QWidget {
public:
    ToolBox();
    void addItem(ToolItem *item);
    void clear();
    QSize sizeHint() const;

private:
    QVBoxLayout *layout;
};

#endif // TOOLBOX_H
