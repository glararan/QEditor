#ifndef HEIGHTMAPWIDGET_H
#define HEIGHTMAPWIDGET_H

#include <QDialog>

namespace Ui
{
    class HeightmapWidget;
}

class HeightmapWidget : public QDialog
{
    Q_OBJECT

public:
    explicit HeightmapWidget(QWidget* parent = 0);
    ~HeightmapWidget();

private:
    Ui::HeightmapWidget* ui;

private slots:
    void showImportBrowser();
    void showExportBrowser();
    void showSTLExportBrowser();

    void doImport();
    void doExport();
    void doSTLExport();

    void setScale(double scale);

signals:
    void importing(QString path, float scale);
    void exporting(QString path, float scale);
    void stlExporting(QString path, float surface, bool scaleHeight, bool low);

    void setScale(float scale);
};

#endif // HEIGHTMAPWIDGET_H
