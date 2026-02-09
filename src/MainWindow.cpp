#include "MainWindow.h"

#include <qboxlayout.h>
#include <qlist.h>
#include <qnamespace.h>
#include <qsplitter.h>

#include "Outliner.h"
#include "StageViewWidget.h"

MainWindow::MainWindow() : QMainWindow() {
    m_outliner = new Outliner(this);
    m_stageViewWidget = new StageViewWidget(this);

    m_splitter = new QSplitter(this);
    m_splitter->addWidget(m_outliner);
    m_splitter->addWidget(m_stageViewWidget);
    m_splitter->setSizes(QList<int>{300, 800});

    setCentralWidget(m_splitter);

    connect(m_stageViewWidget, &StageViewWidget::stageOpened, m_outliner,
            &Outliner::onStageOpened);
    connect(m_stageViewWidget, &StageViewWidget::primSelected, m_outliner,
            &Outliner::onPrimSelected);
    connect(m_outliner, &Outliner::primSelected, m_stageViewWidget,
            &StageViewWidget::onPrimSelected);
}

MainWindow::~MainWindow() = default;