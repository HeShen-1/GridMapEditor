#ifndef PATHFINDINGEXECUTOR_H
#define PATHFINDINGEXECUTOR_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QPoint>
#include <QList>

class PathfindingExecutor : public QObject
{
    Q_OBJECT

public:
    enum AlgorithmType {
        AStar,
        Dijkstra,
        BFS,
        DFS,
        DStar,
        Unknown
    };

    enum Language {
        CPlusPlus,
        Java,
        Python,
        UnknownLanguage
    };

    explicit PathfindingExecutor(QObject *parent = nullptr);

    // 执行寻路算法
    void executeCode(const QString& code, 
                     const QVector<QVector<int>>& grid,
                     const QPoint& start,
                     const QPoint& end);
    
    // 静默执行寻路算法（不发出错误信号）
    void executeCodeSilently(const QString& code, 
                              const QVector<QVector<int>>& grid,
                              const QPoint& start,
                              const QPoint& end);
    
    // 静默执行寻路算法，但会在无路径时清除显示
    void executeCodeSilentlyWithCallback(const QString& code, 
                                         const QVector<QVector<int>>& grid,
                                         const QPoint& start,
                                         const QPoint& end);

signals:
    void pathFound(const QList<QPoint>& path);
    void executionError(const QString& message);
    void noPathFound(const QString& message);

private:
    AlgorithmType detectAlgorithm(const QString& code);
    Language detectLanguage(const QString& code);
    
    // 内置算法实现
    QList<QPoint> executeAStar(const QVector<QVector<int>>& grid,
                               const QPoint& start,
                               const QPoint& end);
    QList<QPoint> executeDijkstra(const QVector<QVector<int>>& grid,
                                  const QPoint& start,
                                  const QPoint& end);
    QList<QPoint> executeBFS(const QVector<QVector<int>>& grid,
                             const QPoint& start,
                             const QPoint& end);
    QList<QPoint> executeDFS(const QVector<QVector<int>>& grid,
                             const QPoint& start,
                             const QPoint& end);
    QList<QPoint> executeDStar(const QVector<QVector<int>>& grid,
                               const QPoint& start,
                               const QPoint& end);

    // 辅助函数
    bool isValid(int x, int y, const QVector<QVector<int>>& grid);
    int heuristic(const QPoint& a, const QPoint& b);
    QList<QPoint> reconstructPath(const QVector<QVector<QPoint>>& cameFrom,
                                  const QPoint& start,
                                  const QPoint& end);

    // 内部数据结构
    struct Node {
        QPoint pos;
        int g, h, f;
        QPoint parent;
        
        Node() : pos(-1, -1), g(0), h(0), f(0), parent(-1, -1) {}
        Node(const QPoint& p) : pos(p), g(0), h(0), f(0), parent(-1, -1) {}
        
        bool operator<(const Node& other) const {
            return f > other.f; // 优先队列是最大堆，我们需要最小堆
        }
    };
    
    // Node结构体专用的reconstructPath函数
    QList<QPoint> reconstructPath(const QVector<QVector<Node>>& nodeMap,
                                  const QPoint& start,
                                  const QPoint& end);
};

#endif // PATHFINDINGEXECUTOR_H 