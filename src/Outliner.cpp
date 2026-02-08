#include "Outliner.h"

#include <pxr/base/tf/type.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/imageable.h>
#include <qabstractitemview.h>
#include <qdebug.h>
#include <qnamespace.h>
#include <qtreewidget.h>
#include <qvariant.h>

#include <iostream>
#include <string>

Outliner::Outliner(QWidget* parent) : QTreeWidget(parent) {
    setColumnCount(1);
    setHeaderHidden(true);
    setSelectionMode(SingleSelection);
}
Outliner::~Outliner() = default;

void Outliner::setUsdStage(const pxr::UsdStagePtr stage) {
    clear();
    m_pathToItemHash.clear();

    // Construct usd prim tree
    auto item = new QTreeWidgetItem(this);
    int currentDepth = 0;
    for (pxr::UsdPrim prim : stage->Traverse()) {
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
        item->setData(0, Qt::UserRole, QVariant::fromValue(path));
        currentDepth += 1;
        m_pathToItemHash[path.GetString().c_str()] = item;
    }

    // Remove top untitled item
    auto topUntitledItem = topLevelItem(0);
    addTopLevelItems(topUntitledItem->takeChildren());
    delete topUntitledItem;

    expandAll();

    // auto test_item = m_pathToItemHash.value("/root/Geo/Plane", nullptr);
    // if (test_item) {
    //     qDebug() << "Success find item: " << test_item;
    //     scrollToItem(test_item);
    //     setCurrentItem(test_item);
    // } else {
    //     qDebug() << "Failed find item: ";
    // }
}