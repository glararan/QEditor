#include "toolbox.h"


ToolBox::ToolBox() :
    layout(new QVBoxLayout)
{
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}

void ToolBox::addItem(ToolItem *item)
{
    int count = layout->count();
    if (count > 1) {
        layout->removeItem(layout->itemAt(count - 1));
    }

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

    return QSize(314,0).boundedTo(QSize(640, 480));
}


ToolItem::ToolItem(const QString &t, QWidget *item) : item(item) {
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    QString titleString = t;
    titleString.replace(0,1,titleString.at(0).toUpper());
    title = new QLabel(titleString);
    title->setStyleSheet("QLabel { padding-left: 7px; border-radius: 4px;  border: 1px solid #AAA; background: #E3E3E3; }");
    title->setMargin(3);
    title->setMinimumWidth(item->width());

    layout->addWidget(title);
    layout->addWidget(item);

    setLayout(layout);

    item->setVisible(false);
}

QWidget *ToolItem::getWidget()
{
    return item;
}

void ToolItem::mousePressEvent(QMouseEvent *event) {
    item->setVisible(!item->isVisible());
}
