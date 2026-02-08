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
    Outliner(QWidget* parent = nullptr);
    ~Outliner() override;

    void setUsdStage(const pxr::UsdStagePtr stage);

   private:
    pxr::UsdStagePtr m_stage;
    QHash<QString, QTreeWidgetItem*> m_pathToItemHash;
};