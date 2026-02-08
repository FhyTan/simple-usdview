#include "MainWindow.h"

#include <qboxlayout.h>
#include <qlist.h>
#include <qsplitter.h>

MainWindow::MainWindow() : QMainWindow() {
    m_outliner = new Outliner(this);
    m_stageViewWidget = new StageViewWidget(this);

    m_splitter = new QSplitter(this);
    m_splitter->addWidget(m_outliner);
    m_splitter->addWidget(m_stageViewWidget);
    m_splitter->setSizes(QList<int>{300, 800});

    setCentralWidget(m_splitter);
}

MainWindow::~MainWindow() = default;