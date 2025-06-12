#include "../include/mainwindow.h"
#include "../include/codehighlighter.h"
#include "../include/gridcreatedialog.h"
#include "../include/examplecodedialog.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QStyle>
#include <QPalette>
#include <QInputDialog>
#include <QToolBar>
#include <QButtonGroup>
#include <QToolButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentAlgorithmName("自定义算法"), hasValidPathBeforeChange(false)
{
    // 创建中心部件
    splitter = new QSplitter(Qt::Horizontal);
    setCentralWidget(splitter);

    // 创建左侧面板
    leftPanel = new QWidget(this);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(5, 5, 5, 5);
    leftLayout->setSpacing(5);
    
    // 创建算法标题
    QLabel *titleLabel = new QLabel(tr("算法名称:"), this);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 12px;");
    
    algorithmTitle = new QLineEdit(currentAlgorithmName, this);
    algorithmTitle->setPlaceholderText(tr("请输入算法名称"));
    algorithmTitle->setMaxLength(50);
    connect(algorithmTitle, &QLineEdit::textChanged, this, &MainWindow::onAlgorithmTitleChanged);
    
    // 创建代码编辑器
    codeEditor = new CodeEditor(this);
    
    // 布局左侧面板
    leftLayout->addWidget(titleLabel);
    leftLayout->addWidget(algorithmTitle);
    leftLayout->addWidget(codeEditor);
    
    // 创建栅格编辑器
    gridEditor = new GridEditor(this);
    
    // 创建代码执行器
    executor = new PathfindingExecutor(this);

    // 添加到分割器
    splitter->addWidget(leftPanel);
    splitter->addWidget(gridEditor);

    // 设置初始大小
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);

    // 创建工具栏
    createToolBar();

    // 创建菜单和动作
    createActions();
    createMenus();
    createThemeMenu();
    
    // 连接信号和槽
    connect(executor, &PathfindingExecutor::pathFound, gridEditor, &GridEditor::executePathfinding);
    connect(executor, &PathfindingExecutor::executionError, this, [this](const QString& message) {
        QMessageBox::critical(this, tr("执行错误"), message);
        // 执行出错时退出代码执行模式
        gridEditor->setCodeExecutionMode(false);
        runCodeAction->setEnabled(true);
        stopExecutionAction->setEnabled(false);
    });
    connect(executor, &PathfindingExecutor::noPathFound, this, [this](const QString& message) {
        QMessageBox::warning(this, tr("路径规划结果"), message);
        // 清除当前显示的路径
        gridEditor->clearPath();
        // 无路径时退出代码执行模式
        gridEditor->setCodeExecutionMode(false);
        runCodeAction->setEnabled(true);
        stopExecutionAction->setEnabled(false);
    });
    connect(gridEditor, &GridEditor::executionError, this, [this](const QString& message) {
        QMessageBox::warning(this, tr("路径执行错误"), message);
    });
    connect(gridEditor, &GridEditor::executionFinished, this, [this]() {
        // 路径执行完成时退出代码执行模式
        gridEditor->setCodeExecutionMode(false);
        runCodeAction->setEnabled(true);
        stopExecutionAction->setEnabled(false);
    });
    connect(gridEditor, &GridEditor::pathCleared, this, [this]() {
        // 路径被清除，可能需要提示用户
        if (gridEditor->isInExecutionMode()) {
            // 在执行模式下路径被清除，说明被用户操作中断了
            QMessageBox::warning(this, tr("路径中断"), 
                tr("由于障碍物变化，当前路径已中断。\n请调整地图布局后重新运行。"));
        }
    });
    
    // 连接栅格变化信号，用于实时路径更新
    connect(gridEditor, &GridEditor::gridChanged, this, [this]() {
        // 只有在代码执行模式下才进行实时更新
        if (gridEditor->isInExecutionMode() && !codeEditor->toPlainText().trimmed().isEmpty()) {
            updatePathInRealTime();
        }
    });
    
    // 为实时更新添加专门的连接，处理路径状态变化
    connect(executor, &PathfindingExecutor::noPathFound, this, [this](const QString& message) {
        // 检查是否是实时更新触发的无路径情况
        if (gridEditor->isInExecutionMode() && 
            message.contains(tr("由于障碍物变化，无法找到可通行路径！"))) {
            // 立即停止小车移动
            gridEditor->stopExecution();
            // 静默清除路径
            gridEditor->clearPathSilently();
            
            // 只有当之前有路径而现在没有路径时，才显示警告
            if (hasValidPathBeforeChange) {
                QMessageBox::warning(this, tr("路径中断"), 
                    tr("由于障碍物变化，当前路径已中断。\n请调整地图布局后重新运行。"));
            }
            // 退出执行模式，暂停程序
            gridEditor->setCodeExecutionMode(false);
            runCodeAction->setEnabled(true);
            stopExecutionAction->setEnabled(false);
        }
    }, Qt::QueuedConnection);

    // 设置窗口标题和大小
    setWindowTitle(tr("Grid Map Editor"));
    resize(1200, 800);

    // 应用默认主题
    applyTheme("system");
}

MainWindow::~MainWindow()
{
}

void MainWindow::createActions()
{
    // 文件菜单动作
    newAction = new QAction(tr("新建窗口"), this);
    newAction->setShortcuts(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::newWindow);

    openAction = new QAction(tr("打开文件"), this);
    openAction->setShortcuts(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);

    saveAction = new QAction(tr("保存文件"), this);
    saveAction->setShortcuts(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);

    exitAction = new QAction(tr("退出"), this);
    exitAction->setShortcuts(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    // 栅格菜单动作
    newGridAction = new QAction(tr("创建栅格地图"), this);
    connect(newGridAction, &QAction::triggered, this, &MainWindow::createNewGrid);

    clearGridAction = new QAction(tr("清空栅格地图"), this);
    connect(clearGridAction, &QAction::triggered, this, &MainWindow::clearCurrentGrid);

    // 新增：保存和读取地图动作
    saveGridAction = new QAction(tr("保存地图"), this);
    connect(saveGridAction, &QAction::triggered, this, &MainWindow::saveGridMap);

    loadGridAction = new QAction(tr("读取地图"), this);
    connect(loadGridAction, &QAction::triggered, this, &MainWindow::loadGridMap);
    
    // 随机生成障碍物动作
    randomObstacleAction = new QAction(tr("随机生成障碍物"), this);
    connect(randomObstacleAction, &QAction::triggered, this, &MainWindow::generateRandomObstacles);

    // 示例代码动作
    exampleCodeAction = new QAction(tr("示例代码"), this);
    connect(exampleCodeAction, &QAction::triggered, this, &MainWindow::showExampleCode);
    
    // 运行代码动作
    runCodeAction = new QAction(tr("运行代码"), this);
    runCodeAction->setShortcut(QKeySequence("F5"));
    connect(runCodeAction, &QAction::triggered, this, &MainWindow::runCode);
    
    // 停止执行动作
    stopExecutionAction = new QAction(tr("停止执行"), this);
    stopExecutionAction->setEnabled(false);
    connect(stopExecutionAction, &QAction::triggered, this, &MainWindow::stopExecution);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("文件"));
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    viewMenu = menuBar()->addMenu(tr("视图"));
    themeMenu = viewMenu->addMenu(tr("主题"));

    gridMenu = menuBar()->addMenu(tr("栅格地图"));
    gridMenu->addAction(newGridAction);
    gridMenu->addAction(clearGridAction);
    gridMenu->addSeparator();
    gridMenu->addAction(saveGridAction);    // 新增：保存地图菜单项
    gridMenu->addAction(loadGridAction);    // 新增：读取地图菜单项
    gridMenu->addSeparator();
    gridMenu->addAction(randomObstacleAction);  // 新增：随机生成障碍物菜单项

    exampleMenu = menuBar()->addMenu(tr("示例代码"));
    exampleMenu->addAction(exampleCodeAction);
    
    QMenu *runMenu = menuBar()->addMenu(tr("运行"));
    runMenu->addAction(runCodeAction);
    runMenu->addAction(stopExecutionAction);
}

void MainWindow::createThemeMenu()
{
    themeGroup = new QActionGroup(this);

    QAction *systemThemeAction = themeMenu->addAction(tr("跟随系统"));
    systemThemeAction->setCheckable(true);
    systemThemeAction->setData("system");
    themeGroup->addAction(systemThemeAction);

    QAction *lightThemeAction = themeMenu->addAction(tr("浅色"));
    lightThemeAction->setCheckable(true);
    lightThemeAction->setData("light");
    themeGroup->addAction(lightThemeAction);

    QAction *darkThemeAction = themeMenu->addAction(tr("深色"));
    darkThemeAction->setCheckable(true);
    darkThemeAction->setData("dark");
    themeGroup->addAction(darkThemeAction);

    systemThemeAction->setChecked(true);
    connect(themeGroup, &QActionGroup::triggered, this, &MainWindow::setTheme);
}

void MainWindow::setTheme(QAction *action)
{
    QString theme = action->data().toString();
    applyTheme(theme);
}

void MainWindow::applyTheme(const QString &theme)
{
    QPalette palette;
    if (theme == "system") {
        // 跟随系统主题
        QApplication::setStyle(QApplication::style()->name());
        palette = QApplication::style()->standardPalette();
    } else if (theme == "dark") {
        // 深色主题
        palette.setColor(QPalette::Window, QColor(45, 45, 45));
        palette.setColor(QPalette::WindowText, Qt::white);
        palette.setColor(QPalette::Base, QColor(30, 30, 30));
        palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        palette.setColor(QPalette::Text, Qt::white);
        palette.setColor(QPalette::Button, QColor(53, 53, 53));
        palette.setColor(QPalette::ButtonText, Qt::white);
        palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        palette.setColor(QPalette::HighlightedText, Qt::black);
    } else if (theme == "light") {
        // 浅色主题
        palette.setColor(QPalette::Window, QColor(240, 240, 240));
        palette.setColor(QPalette::WindowText, Qt::black);
        palette.setColor(QPalette::Base, Qt::white);
        palette.setColor(QPalette::AlternateBase, QColor(245, 245, 245));
        palette.setColor(QPalette::Text, Qt::black);
        palette.setColor(QPalette::Button, QColor(240, 240, 240));
        palette.setColor(QPalette::ButtonText, Qt::black);
        palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        palette.setColor(QPalette::HighlightedText, Qt::white);
    }

    QApplication::setPalette(palette);
    codeEditor->setTheme(theme);
}

void MainWindow::newWindow()
{
    // 创建新的主窗口实例
    MainWindow *newWindow = new MainWindow();
    // 显示新窗口
    newWindow->show();
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("打开文件"), "",
        tr("源代码文件 (*.cpp *.h *.py *.java);;所有文件 (*)"));

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream in(&file);
            codeEditor->setPlainText(in.readAll());
            file.close();
            
            // 设置为自定义算法
            currentAlgorithmName = "自定义算法";
            algorithmTitle->setText(currentAlgorithmName);
        }
    }
}

void MainWindow::saveFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("保存文件"), "",
        tr("源代码文件 (*.cpp *.h *.py *.java);;所有文件 (*)"));

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QFile::WriteOnly | QFile::Text)) {
            QTextStream out(&file);
            out << codeEditor->toPlainText();
            file.close();
        }
    }
}

void MainWindow::createNewGrid()
{
    GridCreateDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        gridEditor->createGrid(dialog.getRows(), dialog.getCols());
    }
}

void MainWindow::clearCurrentGrid()
{
    gridEditor->clearGrid();
}

void MainWindow::createToolBar()
{
    QToolBar *toolBar = addToolBar(tr("编辑工具"));
    toolBar->setMovable(false);

    // 创建隐藏/显示代码区域的按钮
    toggleCodeButton = new QPushButton(tr("隐藏代码区"), this);
    toggleCodeButton->setFixedSize(100, 30);
    toggleCodeButton->setStyleSheet("QPushButton { font-size: 11px; }");
    connect(toggleCodeButton, &QPushButton::clicked, this, &MainWindow::toggleCodeEditor);
    toolBar->addWidget(toggleCodeButton);
    
    // 添加分隔符
    toolBar->addSeparator();

    // 创建按钮组
    QButtonGroup *buttonGroup = new QButtonGroup(this);

    // 创建工具按钮
    auto createToolButton = [&](const QString &text, GridEditor::CellState state) {
        QToolButton *button = new QToolButton(this);
        button->setText(text);
        button->setCheckable(true);
        button->setFixedSize(60, 30);  // 设置固定大小
        buttonGroup->addButton(button);
        toolBar->addWidget(button);
        connect(button, &QToolButton::clicked, [this, state]() {
            gridEditor->setCurrentState(state);
        });
        return button;
    };

    // 添加各种状态的按钮
    auto obstacleButton = createToolButton(tr("障碍"), GridEditor::Obstacle);
    auto startButton = createToolButton(tr("起点"), GridEditor::Start);
    auto endButton = createToolButton(tr("终点"), GridEditor::End);

    // 设置默认选中的按钮
    obstacleButton->setChecked(true);
    
    // 添加分隔符
    toolBar->addSeparator();
}

void MainWindow::saveGridMap()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("保存地图"), "",
        tr("JSON文件 (*.json);;所有文件 (*)"));

    if (!fileName.isEmpty()) {
        if (!fileName.endsWith(".json", Qt::CaseInsensitive)) {
            fileName += ".json";
        }
        
        if (!gridEditor->saveToJson(fileName)) {
            QMessageBox::warning(this, tr("保存失败"),
                tr("无法保存地图文件。\n请确保您有写入权限并重试。"));
        }
    }
}

void MainWindow::loadGridMap()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("读取地图"), "",
        tr("JSON文件 (*.json);;所有文件 (*)"));

    if (!fileName.isEmpty()) {
        if (!gridEditor->loadFromJson(fileName)) {
            QString errorMsg = gridEditor->getLastErrorMessage();
            if (errorMsg.isEmpty()) {
                errorMsg = tr("无法读取地图文件。\n请确保文件格式正确并重试。");
            }
            QMessageBox::warning(this, tr("读取失败"), errorMsg);
        }
    }
}

void MainWindow::showExampleCode()
{
    ExampleCodeDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString code = dialog.getSelectedCode();
        if (!code.isEmpty()) {
            codeEditor->setPlainText(code);
            
            // 根据选择的算法设置标题
            QString algorithm = dialog.getSelectedAlgorithm();
            QString language = dialog.getSelectedLanguage();
            QString title = QString("%1 (%2)").arg(algorithm).arg(language);
            currentAlgorithmName = title;
            algorithmTitle->setText(title);
        }
    }
}

void MainWindow::runCode()
{
    // 检查是否有代码
    QString code = codeEditor->toPlainText().trimmed();
    if (code.isEmpty()) {
        QMessageBox::warning(this, tr("运行错误"), tr("代码编辑器为空！请输入或选择示例代码。"));
        return;
    }
    
    // 检查栅格地图是否存在
    if (!gridEditor->hasValidStartAndEnd()) {
        QMessageBox::warning(this, tr("运行错误"), tr("请先创建栅格地图并设置起点和终点！"));
        return;
    }
    
    // 停止之前的执行
    gridEditor->stopExecution();
    
    // 进入代码执行模式
    gridEditor->setCodeExecutionMode(true);
    
    // 获取栅格数据
    QVector<QVector<int>> gridData = gridEditor->getGridData();
    QPoint start = gridEditor->getStartPos();
    QPoint end = gridEditor->getEndPos();
    
    // 更新按钮状态
    runCodeAction->setEnabled(false);
    stopExecutionAction->setEnabled(true);
    
    // 执行代码
    executor->executeCode(code, gridData, start, end);
}

void MainWindow::stopExecution()
{
    gridEditor->stopExecution();
    gridEditor->setCodeExecutionMode(false);
    runCodeAction->setEnabled(true);
    stopExecutionAction->setEnabled(false);
}

void MainWindow::onAlgorithmTitleChanged()
{
    currentAlgorithmName = algorithmTitle->text().trimmed();
    if (currentAlgorithmName.isEmpty()) {
        currentAlgorithmName = "自定义算法";
        algorithmTitle->setPlaceholderText(tr("请输入算法名称"));
    }
}

void MainWindow::updatePathInRealTime()
{
    // 检查是否有有效的起点和终点
    if (!gridEditor->hasValidStartAndEnd()) {
        return;
    }
    
    // 获取当前代码
    QString code = codeEditor->toPlainText().trimmed();
    if (code.isEmpty()) {
        return;
    }
    
    // 记录修改前是否有路径（通过检查当前是否有路径显示）
    hasValidPathBeforeChange = gridEditor->hasPath();
    
    // 获取最新的栅格数据
    QVector<QVector<int>> gridData = gridEditor->getGridData();
    QPoint start = gridEditor->getStartPos();
    QPoint end = gridEditor->getEndPos();
    
    // 静默执行路径计算（不显示错误对话框）
    executor->executeCodeSilentlyWithCallback(code, gridData, start, end);
}

void MainWindow::toggleCodeEditor()
{
    bool isVisible = leftPanel->isVisible();
    
    if (isVisible) {
        // 隐藏整个左侧代码区域
        leftPanel->hide();
        toggleCodeButton->setText(tr("显示代码区"));
        // 调整分割器比例，让栅格编辑器占据更多空间
        splitter->setSizes({0, 1});
    } else {
        // 显示整个左侧代码区域
        leftPanel->show();
        toggleCodeButton->setText(tr("隐藏代码区"));
        // 恢复分割器的默认比例
        splitter->setSizes({1, 1});
    }
}

void MainWindow::generateRandomObstacles()
{
    // 检查是否有栅格地图
    if (!gridEditor->hasValidStartAndEnd()) {
        QMessageBox::warning(this, tr("生成失败"), 
            tr("请先创建栅格地图并设置起点和终点！"));
        return;
    }
    
    // 显示随机障碍生成对话框
    RandomObstacleDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        double density = dialog.getObstacleDensity();
        RandomObstacleDialog::ConnectivityType connectivityType = dialog.getConnectivityType();
        int pathCount = dialog.getPathCount();
        bool useSeed = dialog.isUseSeed();
        int seed = dialog.getSeed();
        
        // 调用GridEditor的随机生成方法
        gridEditor->generateRandomObstacles(density, static_cast<int>(connectivityType), pathCount, useSeed, seed);
        
        // 提示生成完成
        QString message;
        switch (connectivityType) {
            case RandomObstacleDialog::NoPath:
                message = tr("已生成随机障碍物（无可通行通路）");
                break;
            case RandomObstacleDialog::OnePath:
                message = tr("已生成随机障碍物（一条可通行通路）");
                break;
            case RandomObstacleDialog::MultiplePaths:
                message = tr("已生成随机障碍物（%1条可通行通路）").arg(pathCount);
                break;
        }
        
        if (useSeed) {
            message += tr("，使用种子：%1").arg(seed);
        }
        
        QMessageBox::information(this, tr("生成完成"), message);
    }
} 