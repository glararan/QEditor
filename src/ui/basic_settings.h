#ifndef BASIC_SETTINGS_H
#define BASIC_SETTINGS_H

#include <QWizard>

namespace Ui
{
    class Basic_Settings;
}

class World;

class Basic_Settings : public QWizard
{
    Q_OBJECT

public:
    Basic_Settings(World* mWorld, QWidget* parent = 0);
    ~Basic_Settings();

protected:
    void accept();
    void reject();

private:
    Ui::Basic_Settings* ui;

    World* world;

private slots:
    void setDefaultHeightEnabled(int state);

    void openTerrainGenerationWidget();
    void openTerrainImportExportWidget();

    void cancelEvent();

public slots:

signals:
    void openTerrainGeneration();
    void openTerrainImportExport();
};

#endif // BASIC_SETTINGS_H
