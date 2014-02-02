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
