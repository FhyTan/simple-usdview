#pragma once

#include <qtmetamacros.h>
#include <qwidget.h>

#include <QMainWindow>
#include <QSplitter>

#include "Outliner.h"
#include "StageViewWidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    MainWindow();
    ~MainWindow() override;

   private:
    Outliner* m_outliner;
    StageViewWidget* m_stageViewWidget;
    QSplitter* m_splitter;
};