#pragma once

#include <pxr/usd/usd/common.h>
#include <pxr/usd/usd/stage.h>
#include <qcontainerfwd.h>
#include <qhash.h>
#include <qtreewidget.h>
#include <qwidget.h>

#include <QTreeWidget>
#include <QTreeWidgetItem>

class Outliner : public QTreeWidget {
    Q_OBJECT

   public:
    Outliner(QWidget *parent = nullptr);
    ~Outliner() override;

   Q_SIGNALS:
    void primSelected(const std::optional<pxr::UsdPrim> &prim);

   public Q_SLOTS:
    void onStageOpened(const pxr::UsdStagePtr &stage);
    void onPrimSelected(const std::optional<pxr::UsdPrim> &prim);
    void onItemClicked(QTreeWidgetItem *item, int column);

   private:
    void buildStageTree();

    pxr::UsdStagePtr m_stage;
    QHash<QString, QTreeWidgetItem *> m_pathToItemHash;
};