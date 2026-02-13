#include "Outliner.h"

#include <pxr/base/tf/type.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/imageable.h>
#include <qabstractitemview.h>
#include <qdebug.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qtreewidget.h>
#include <qvariant.h>

#include <iostream>
#include <optional>
#include <string>

Outliner::Outliner(QWidget* parent) : QTreeWidget(parent) {
    setColumnCount(1);
    setHeaderHidden(true);
    setSelectionMode(SingleSelection);
    setTextElideMode(Qt::ElideNone);
    setIndentation(10);

    connect(this, &Outliner::itemClicked, this, &Outliner::onItemClicked);
}
Outliner::~Outliner() = default;

void Outliner::onStageOpened(const pxr::UsdStagePtr& stage) {
    // std::cout << "Outliner::onStageOpened" << std::endl;
    m_stage = stage;
    buildStageTree();
}

void Outliner::onPrimSelected(const std::optional<pxr::UsdPrim>& prim) {
    // std::cout << "Outliner::onPrimSelected" << std::endl;
    if (prim) {
        auto pathStr = prim->GetPath().GetString().c_str();
        auto item = m_pathToItemHash.value(pathStr, nullptr);

        if (item) {
            scrollToItem(item);
            setCurrentItem(item);
        }
    } else {
        clearSelection();
    }
}

void Outliner::onItemClicked(QTreeWidgetItem* item, int column) {
    auto prim = item->data(column, Qt::UserRole).value<pxr::UsdPrim>();
    Q_EMIT primSelected(std::make_optional(prim));
    // std::cout << "Outliner::onItemClicked: " << prim.GetPath() << std::endl;
}

void Outliner::buildStageTree() {
    QSignalBlocker blocker{this};

    clear();
    m_pathToItemHash.clear();

    // Construct usd prim tree
    auto item = new QTreeWidgetItem(this);
    int currentDepth = 0;
    for (pxr::UsdPrim prim : m_stage->Traverse()) {
        if (!prim.IsA<pxr::UsdGeomImageable>()) {
            continue;
        }

        auto path = prim.GetPath();
        auto name = prim.GetName().GetText();
        int depth = path.GetPathElementCount();

        // std::cout << "path:" << prim.GetPath() << " depth:" << depth
        //           << " name:" << name << std::endl;

        while (currentDepth >= depth) {
            item = item->parent();
            currentDepth -= 1;
        }
        // qDebug() << "current:" << item->text(0) << " depth:" << currentDepth;

        item = new QTreeWidgetItem(item);
        item->setText(0, name);
        item->setData(0, Qt::UserRole, QVariant::fromValue(prim));
        currentDepth += 1;
        m_pathToItemHash[path.GetString().c_str()] = item;
    }

    // Remove top untitled item
    auto topUntitledItem = topLevelItem(0);
    addTopLevelItems(topUntitledItem->takeChildren());
    delete topUntitledItem;

    expandAll();
    resizeColumnToContents(0);
}