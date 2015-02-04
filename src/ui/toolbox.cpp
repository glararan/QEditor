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

#include "toolbox.h"

ToolBox::ToolBox()
: layout(new QVBoxLayout)
{
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}

void ToolBox::addItem(ToolItem* item)
{
    int count = layout->count();

    if (count > 1)
        layout->removeItem(layout->itemAt(count - 1));

    layout->addWidget(item);
    layout->addStretch();
}

void ToolBox::clear()
{
    while(layout->count() > 0)
        layout->removeItem(layout->itemAt(layout->count() - 1));
}

QSize ToolBox::sizeHint() const
{
    ensurePolished();

    return QSize(314, 0).boundedTo(QSize(640, 480));
}

ToolItem::ToolItem(const QString& t, QWidget* item) : item(item)
{
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    QString titleString = t;
    titleString.replace(0, 1, titleString.at(0).toUpper());

    title = new QLabel(titleString);
    title->setStyleSheet("QLabel { padding-left: 7px; border-radius: 4px;  border: 1px solid #AAA; background: #E3E3E3; }");
    title->setMargin(3);
    title->setMinimumWidth(item->width());

    layout->addWidget(title);
    layout->addWidget(item);

    setLayout(layout);

    item->setVisible(false);
}

QWidget* ToolItem::getWidget()
{
    return item;
}

void ToolItem::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event);

    item->setVisible(!item->isVisible());
}
