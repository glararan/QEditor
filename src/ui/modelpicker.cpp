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

#include "modelpicker.h"
#include "ui_modelpicker.h"

#include "mathhelper.h"

#include <QScrollArea>

ModelPicker::ModelPicker(QWidget* parent)
: QWidget(parent)
, ui(new Ui::ModelPicker)
, favouriteList(new FavouriteList(this))
, modelList(new ModelList(this))
/*, modelIconSize(QSize(78, 78))
, modelIconMargin(QSize(0, 0))
, columns((width() - 50) / (modelIconSize.width() + modelIconMargin.width()))
, box(new ToolBox())*/
{
    ui->setupUi(this);

    setMinimumHeight(200);

    //ui->verticalLayout->insertWidget(0, box);
    ui->horizontalLayout_2->addWidget(modelList);
    ui->gridLayout->addWidget(favouriteList, 3, 0, 1, 2);

    connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(deleteModel()));
    connect(ui->clearButton,  SIGNAL(clicked()), this, SLOT(clearModels()));

    connect(ui->searchLine, SIGNAL(textChanged(QString)), this, SLOT(search(QString)));

    connect(favouriteList, SIGNAL(currentRowChanged(int)), this, SLOT(modelSelected(int)));

    connect(modelList, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(modelSelected(QTreeWidgetItem*, QTreeWidgetItem*)));

    hide();
}

ModelPicker::~ModelPicker()
{
    clear();

    delete ui;

    //delete box;
    delete modelList;
    delete favouriteList;
}

void ModelPicker::loadPicker(ModelManager* modelManager)
{
    manager = modelManager;

    modelList->setModelManager(modelManager);
    modelList->setHeaderHidden(true);

    clear();

    QVector<QString> categories = manager->getCategories();

    for(int i = 0; i < categories.size(); ++i)
    {
        QVector<QString> names = manager->getNames(categories.at(i));

        QTreeWidgetItem* parent = new QTreeWidgetItem();
        parent->setText(0, categories.at(i));

        modelList->addTopLevelItem(parent);

        for(int j = 0; j < names.size(); ++j)
        {
            QTreeWidgetItem* child = new QTreeWidgetItem();
            child->setText(0, QFileInfo(names.at(j)).fileName());

            modelList->topLevelItem(i)->addChild(child);
        }

        /*TextureWell* well = new TextureWell(this, 0, columns, modelIconSize, modelIconMargin);

        connect(well, SIGNAL(selected(int, int)), this, SLOT(modelSelected(int, int)));

        QVector<QString> names = manager->getNames(categories.at(i));

        for(int j = 0; j < names.size(); ++j)
        {
            ModelData* model = manager->getModelData(manager->getIndex(names.at(j)));

            well->insertItem(model->image, names.at(j));
        }

        QScrollArea* scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true);
        scrollArea->setWidget(well);

        well->resize(columns * (modelIconSize.width() + 5) + 50, 50 + names.size() * (modelIconSize.height() + 5) / columns);

        items.push_back(well);
        scrolls.push_back(scrollArea);

        box->addItem(new ToolItem(categories.at(i), scrollArea));*/
    }

    manager->setCurrentModel(-1);
}

void ModelPicker::resizeEvent(QResizeEvent* event)
{
    /*int tColumns = (width() - 50) / (modelIconSize.width() + modelIconMargin.width());

    if(columns != tColumns)
    {
        columns = tColumns;

        QVector<QString> categories = manager->getCategories();

        for(int i = 0; i < categories.size(); ++i)
        {
            int count = manager->getNames(categories.at(i)).size();

            TextureWell* modelWell = items.at(i);

            modelWell->setRows(ceil(MathHelper::toDouble(count) / MathHelper::toDouble(columns)));

            if(count < columns)
                modelWell->setCols(count);
            else
                modelWell->setCols(columns);

            modelWell->resize(columns * (modelIconSize.width() + 5) + 50, 50 + count * (modelIconSize.height() + 5) / columns);
        }
    }*/

    QWidget::resizeEvent(event);
}

void ModelPicker::clear()
{
    /*for(int i = 0; i < items.size(); ++i)
        delete items.at(i);

    for(int i = 0; i < scrolls.size(); ++i)
        delete scrolls.at(i);

    items.clear();
    scrolls.clear();
    box->clear();*/
    modelList->clear();
    currentModelLocation.clear();
}

void ModelPicker::modelSelected(QTreeWidgetItem* current, QTreeWidgetItem* prev)
{
    Q_UNUSED(prev);

    if(!current->parent())
        return;

    QVector<QString> categories = manager->getCategories();

    for(int i = 0; i < categories.size(); ++i)
    {
        if(current->parent()->text(0) != categories.at(i))
            continue;

        QVector<QString> names = manager->getNames(categories.at(i));

        for(int j = 0; j < names.size(); ++j)
        {
            if(QFileInfo(names.at(j)).fileName() == current->text(0))
            {
                manager->setCurrentModel(manager->getIndex(names.at(j)));

                break;
            }
        }
    }
}

/*void ModelPicker::modelSelected(int row, int column)
{
    TextureWell* sender = static_cast<TextureWell*>(this->sender());

    if(sender)
    {
        currentModelLocation = sender->getToolTip(row, column);

        manager->setCurrentModel(manager->getIndex(currentModelLocation));
    }
}*/

void ModelPicker::modelSelected(int row)
{
    QString modelPath = favouriteList->item(row)->text();

    manager->setCurrentModel(manager->getIndex(modelPath));

    emit modelIsSelected();
}

/*void ModelPicker::modelviewSelected(int, int)
{
    TextureWell* sender = static_cast<TextureWell*>(this->sender());
}*/

void ModelPicker::search(QString filter)
{
    if(filter == QString())
    {
        for(int i = 0; i < modelList->topLevelItemCount(); ++i)
            showHideChildrens(modelList->topLevelItem(i));

        modelList->collapseAll();
    }
    else
    {
        for(int i = 0; i < modelList->topLevelItemCount(); ++i)
        {
            showHideChildrens(modelList->topLevelItem(i), false);
            showSpecificChildrens(modelList->topLevelItem(i), filter);
        }

        modelList->expandAll();
    }
}

void ModelPicker::showHideChildrens(QTreeWidgetItem* item, bool show)
{
    item->setHidden(!show);

    for(int i = 0; i < item->childCount(); ++i)
        showHideChildrens(item->child(i), show);
}

void ModelPicker::showSpecificChildrens(QTreeWidgetItem* item, QString filter)
{
    if(parent() && item->childCount() == 0 && item->text(0).contains(filter, Qt::CaseInsensitive))
    {
        showSpecificParents(item);

        item->setHidden(false);
    }

    for(int i = 0; i < item->childCount(); ++i)
        showSpecificChildrens(item->child(i), filter);
}

void ModelPicker::showSpecificParents(QTreeWidgetItem* item)
{
    if(item->parent())
    {
        showSpecificParents(item->parent());

        item->parent()->setHidden(false);
    }
}

void ModelPicker::deleteModel()
{
    if(favouriteList->currentRow() < 0)
        return;

    if(favouriteList->count() < 2)
        favouriteList->clear();
    else
        qDeleteAll(favouriteList->selectedItems());
}

void ModelPicker::clearModels()
{
    favouriteList->clear();
}

void ModelPicker::setPreviousModel()
{
    if(favouriteList->count() == 0)
        return;

    // selection event is called after set row
    if(favouriteList->currentRow() < 0)
        favouriteList->setCurrentRow(favouriteList->count() - 1);
    else
    {
        if(favouriteList->currentRow() == 0)
            favouriteList->setCurrentRow(favouriteList->count() - 1);
        else
            favouriteList->setCurrentRow(favouriteList->currentRow() - 1);
    }
}

void ModelPicker::setNextModel()
{
    if(favouriteList->count() == 0)
        return;

    // selection event is called after set row
    if(favouriteList->currentRow() < 0)
        favouriteList->setCurrentRow(0);
    else
    {
        if(favouriteList->currentRow() == favouriteList->count() - 1)
            favouriteList->setCurrentRow(0);
        else
            favouriteList->setCurrentRow(favouriteList->currentRow() + 1);
    }
}

FavouriteList::FavouriteList(QWidget* parent) : QListWidget(parent)
{
    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);
}

FavouriteList::~FavouriteList()
{
}

void FavouriteList::dragEnterEvent(QDragEnterEvent* event)
{
    if(event->mimeData()->hasFormat("text/plain"))
        event->acceptProposedAction();
}

void FavouriteList::dragMoveEvent(QDragMoveEvent* event)
{
    event->acceptProposedAction();
}

void FavouriteList::dropEvent(QDropEvent* event)
{
    if(event->mimeData()->hasText())
    {
        const QMimeData* data = event->mimeData();

        if(data->text() != QString())
            addItem(data->text());

        if(event->source() == this)
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
            event->acceptProposedAction();
    }
    else
        event->ignore();
}

ModelList::ModelList(QWidget* parent) : QTreeWidget(parent)
{
    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);
}

ModelList::~ModelList()
{
}

void ModelList::mousePressEvent(QMouseEvent* event)
{
    QTreeWidget::mousePressEvent(event);

    if(currentItem() && currentItem()->childCount() == 0 && currentItem()->text(0) != QString() && currentItem()->parent() && manager)
    {
        QString filePath = QString();

        QVector<QString> categories = manager->getCategories();

        for(int i = 0; i < categories.size(); ++i)
        {
            if(currentItem()->parent()->text(0) != categories.at(i))
                continue;

            QVector<QString> names = manager->getNames(categories.at(i));

            for(int j = 0; j < names.size(); ++j)
            {
                if(QFileInfo(names.at(j)).fileName() == currentItem()->text(0))
                {
                    filePath = names.at(j);

                    break;
                }
            }
        }

        QMimeData* mimeData = new QMimeData();
        mimeData->setText(filePath);

        QDrag* drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->setHotSpot(event->pos());
        drag->exec(Qt::CopyAction);
    }
}

void ModelList::mouseReleaseEvent(QMouseEvent* event)
{
    QTreeWidget::mouseReleaseEvent(event);
}

void ModelList::dragEnterEvent(QDragEnterEvent* event)
{
    if(event->source() == this)
        event->ignore();
}
