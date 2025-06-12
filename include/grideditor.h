#ifndef GRIDEDITOR_H
#define GRIDEDITOR_H

#include <QWidget>
#include <QVector>
#include <QPoint>
#include <QPixmap>
#include <QString>
#include <QTimer>
#include <QList>

class GridEditor : public QWidget
{
    Q_OBJECT

public:
    enum CellState {
        Empty = 0,      // 空白（可通行）
        Obstacle = 1,   // 障碍
        Start = 2,      // 起点
        End = 3,        // 终点
        Path = 4,       // 路径（蓝色）
        Current = 5,    // 当前位置（小车）
        VisitedPath = 6 // 小车走过的路径（绿色）
    };

    explicit GridEditor(QWidget *parent = nullptr);

    void createGrid(int rows, int cols);
    void clearGrid();
    void setCellState(const QPoint& pos, CellState state);
    CellState getCellState(const QPoint& pos) const;
    void setCurrentState(CellState state) { currentState = state; }

    // 新增：保存和读取地图的方法声明
    bool saveToJson(const QString& filename) const;
    bool loadFromJson(const QString& filename);
    
    // 路径执行功能
    void executePathfinding(const QList<QPoint>& path);
    void clearPath();
    void clearPathSilently(); // 静默清除路径，不发出信号
    void stopExecution();
    void stopExecutionSilently(); // 静默停止执行，用于正常完成的情况
    QPoint getStartPos() const { return startPos; }
    QPoint getEndPos() const { return endPos; }
    QVector<QVector<int>> getGridData() const;
    bool hasValidStartAndEnd() const;
    
    // 随机障碍生成
    void generateRandomObstacles(double density, int connectivityType, int pathCount, bool useSeed, int seed);
    
    // 执行状态管理
    void setCodeExecutionMode(bool enabled);
    bool isInExecutionMode() const { return codeExecutionMode; }
    bool hasPath() const; // 检查是否有路径显示
    bool isCarMoving() const { return isExecuting; } // 检查小车是否正在移动
    QString getLastErrorMessage() const { return lastErrorMessage; } // 获取最后的错误信息

signals:
    void executionFinished();
    void executionError(const QString& message);
    void gridChanged(); // 栅格发生变化时发出的信号
    void pathCleared(); // 路径被清除时发出的信号

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void moveToNextPosition();

private:
    QVector<QVector<CellState>> grid;  // 存储栅格状态
    int rows;                          // 行数
    int cols;                          // 列数
    int cellSize;                      // 单元格大小
    QPoint gridOffset;                 // 栅格偏移量（用于居中）
    CellState currentState;            // 当前选择的状态
    QPoint startPos;                   // 起点位置
    QPoint endPos;                     // 终点位置
    QPixmap carImage;                  // 起点图片
    QPixmap flagImage;                 // 终点图片
    
    // 路径执行相关
    QList<QPoint> currentPath;         // 当前执行的路径
    int currentStep;                   // 当前步骤
    QPoint currentCarPos;              // 小车当前位置
    QTimer* executionTimer;            // 执行定时器
    bool isExecuting;                  // 是否正在执行
    bool codeExecutionMode;            // 是否处于代码执行模式
    QString lastErrorMessage;           // 存储最后的错误信息

    void updateCellSize();             // 更新单元格大小
    void updateGridOffset();           // 更新栅格偏移量
    QPoint pixelToGrid(const QPoint& pixel) const;  // 像素坐标转换为栅格坐标
    bool isValidGridPos(const QPoint& pos) const;   // 检查栅格坐标是否有效
    void loadImages();                 // 加载图片资源
    void handleRightClick(const QPoint& pos);       // 处理右键点击
    
    // 随机障碍生成的辅助方法
    void generateObstaclesWithNoPath(class QRandomGenerator* generator, int targetObstacles);
    void generateObstaclesWithOnePath(class QRandomGenerator* generator, int targetObstacles);
    void generateObstaclesWithMultiplePaths(class QRandomGenerator* generator, int targetObstacles, int pathCount);
    bool isPathExists(const QPoint& start, const QPoint& end);
    QList<QPoint> findPathBFS(const QPoint& start, const QPoint& end);
    int countPaths(const QPoint& start, const QPoint& end);
};

#endif // GRIDEDITOR_H 