#ifndef TELEPORTWIDGET_H
#define TELEPORTWIDGET_H

#include <QDialog>

namespace Ui
{
    class TeleportWidget;
}

class TeleportWidget : public QDialog
{
    Q_OBJECT
    
public:
    explicit TeleportWidget(QWidget* parent = 0);
    ~TeleportWidget();
    
private:
    Ui::TeleportWidget* ui;

    bool checkCoords(QString coords);

    void writeToBookmarks(QString name, QVector3D location);
    bool readFromBookmarks(QString search, QVector3D& location);

    void loadBookmarks();

public slots:
    void bookmarksAdd();
    void bookmarksGo();
    void bookmarksTeleport();

signals:
    void TeleportTo(QVector3D* location);
};

#endif // TELEPORTWIDGET_H