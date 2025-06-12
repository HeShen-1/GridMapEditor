#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QSplitter>
#include <QPlainTextEdit>
#include <QFileDialog>
#include <QActionGroup>
#include <QMessageBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include "grideditor.h"
#include "codehighlighter.h"
#include "examplecodedialog.h"
#include "pathfindingexecutor.h"
#include "randomobstacledialog.h"

class LineNumberArea;

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    void setTheme(const QString &theme);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    QWidget *lineNumberArea;
    CodeHighlighter *highlighter;
    void updateHighlighterTheme(const QString &theme);
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor), codeEditor(editor) {}
    QSize sizeHint() const override { return QSize(codeEditor->lineNumberAreaWidth(), 0); }

protected:
    void paintEvent(QPaintEvent *event) override { codeEditor->lineNumberAreaPaintEvent(event); }

private:
    CodeEditor *codeEditor;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openFile();
    void newWindow();
    void saveFile();
    void setTheme(QAction *action);
    void createNewGrid();
    void clearCurrentGrid();
    void saveGridMap();
    void loadGridMap();
    void showExampleCode();
    void runCode();
    void stopExecution();
    void onAlgorithmTitleChanged();
    void updatePathInRealTime();
    void toggleCodeEditor();
    void generateRandomObstacles();

private:
    void createMenus();
    void createActions();
    void createThemeMenu();
    void createToolBar();
    void applyTheme(const QString &theme);

private:
    // 界面组件
    QSplitter *splitter;
    QWidget *leftPanel;
    QLineEdit *algorithmTitle;
    CodeEditor *codeEditor;
    QPushButton *toggleCodeButton;
    GridEditor *gridEditor;
    
    // 执行相关
    PathfindingExecutor *executor;
    QString currentAlgorithmName;
    bool hasValidPathBeforeChange; // 记录修改前是否有有效路径

    // 菜单
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *themeMenu;
    QMenu *gridMenu;
    QMenu *exampleMenu;
    QMenu *helpMenu;

    // 动作
    QAction *newAction;
    QAction *openAction;
    QAction *saveAction;
    QAction *exitAction;
    QAction *newGridAction;
    QAction *clearGridAction;
    QAction *saveGridAction;
    QAction *loadGridAction;
    QAction *randomObstacleAction;
    QAction *exampleCodeAction;
    QAction *runCodeAction;
    QAction *stopExecutionAction;
    QActionGroup *themeGroup;
};

#endif // MAINWINDOW_H 