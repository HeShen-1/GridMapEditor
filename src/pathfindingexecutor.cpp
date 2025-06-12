#include "../include/pathfindingexecutor.h"
#include <QDebug>
#include <QQueue>
#include <QStack>
#include <QSet>
#include <QPair>
#include <QRegularExpression>
#include <queue>
#include <algorithm>

PathfindingExecutor::PathfindingExecutor(QObject *parent)
    : QObject(parent)
{
}

void PathfindingExecutor::executeCode(const QString& code, 
                                      const QVector<QVector<int>>& grid,
                                      const QPoint& start,
                                      const QPoint& end)
{
    if (grid.isEmpty() || grid[0].isEmpty()) {
        emit executionError(tr("网格数据为空！"));
        return;
    }
    
    if (start.x() < 0 || start.y() < 0 || end.x() < 0 || end.y() < 0) {
        emit executionError(tr("起点或终点坐标无效！"));
        return;
    }
    
    if (!isValid(start.x(), start.y(), grid)) {
        emit executionError(tr("起点位置不可通行！"));
        return;
    }
    
    if (!isValid(end.x(), end.y(), grid)) {
        emit executionError(tr("终点位置不可通行！"));
        return;
    }

    // 检测算法类型
    AlgorithmType algorithm = detectAlgorithm(code);
    if (algorithm == Unknown) {
        emit executionError(tr("无法识别的算法类型！请确保代码包含正确的算法实现。"));
        return;
    }

    // 执行对应的算法
    QList<QPoint> path;
    try {
        switch (algorithm) {
            case AStar:
                path = executeAStar(grid, start, end);
                break;
            case Dijkstra:
                path = executeDijkstra(grid, start, end);
                break;
            case BFS:
                path = executeBFS(grid, start, end);
                break;
            case DFS:
                path = executeDFS(grid, start, end);
                break;
            case DStar:
                path = executeDStar(grid, start, end);
                break;
            default:
                emit executionError(tr("不支持的算法类型！"));
                return;
        }
        
        if (path.isEmpty()) {
            emit noPathFound(tr("未找到从起点到终点的路径！"));
        } else {
            emit pathFound(path);
        }
    } catch (...) {
        emit executionError(tr("算法执行过程中发生未知错误！"));
    }
}

PathfindingExecutor::AlgorithmType PathfindingExecutor::detectAlgorithm(const QString& code)
{
    QString lowerCode = code.toLower();
    
    // 检测A*算法
    if (lowerCode.contains("astar") || 
        (lowerCode.contains("heuristic") && lowerCode.contains("priority")) ||
        lowerCode.contains("a*")) {
        return AStar;
    }
    
    // 检测Dijkstra算法
    if (lowerCode.contains("dijkstra") || 
        (lowerCode.contains("distance") && lowerCode.contains("priority"))) {
        return Dijkstra;
    }
    
    // 检测BFS算法
    if (lowerCode.contains("bfs") || 
        lowerCode.contains("breadth") ||
        (lowerCode.contains("queue") && !lowerCode.contains("priority"))) {
        return BFS;
    }
    
    // 检测DFS算法
    if (lowerCode.contains("dfs") || 
        lowerCode.contains("depth") ||
        lowerCode.contains("recursive") ||
        lowerCode.contains("stack")) {
        return DFS;
    }
    
    // 检测D*算法
    if (lowerCode.contains("dstar") || 
        lowerCode.contains("d*") ||
        lowerCode.contains("backpointer") ||
        (lowerCode.contains("insert") && lowerCode.contains("processstate"))) {
        return DStar;
    }
    
    return Unknown;
}

PathfindingExecutor::Language PathfindingExecutor::detectLanguage(const QString& code)
{
    QString lowerCode = code.toLower();
    
    if (lowerCode.contains("#include") || lowerCode.contains("std::")) {
        return CPlusPlus;
    }
    
    if (lowerCode.contains("import java") || lowerCode.contains("public class")) {
        return Java;
    }
    
    if (lowerCode.contains("import ") || lowerCode.contains("def ") || lowerCode.contains("class ")) {
        return Python;
    }
    
    return UnknownLanguage;
}

QList<QPoint> PathfindingExecutor::executeAStar(const QVector<QVector<int>>& grid,
                                                const QPoint& start,
                                                const QPoint& end)
{
    int rows = grid.size();
    int cols = grid[0].size();
    
    QVector<QVector<Node>> nodeMap(rows, QVector<Node>(cols));
    QVector<QVector<bool>> closedList(rows, QVector<bool>(cols, false));
    QList<Node*> openList;
    
    // 初始化起始节点
    nodeMap[start.y()][start.x()].pos = start;
    nodeMap[start.y()][start.x()].g = 0;
    nodeMap[start.y()][start.x()].h = heuristic(start, end);
    nodeMap[start.y()][start.x()].f = nodeMap[start.y()][start.x()].h;
    
    openList.append(&nodeMap[start.y()][start.x()]);
    
    QVector<QPoint> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    
    while (!openList.isEmpty()) {
        // 找到f值最小的节点
        Node* current = openList[0];
        int currentIndex = 0;
        for (int i = 1; i < openList.size(); ++i) {
            if (openList[i]->f < current->f) {
                current = openList[i];
                currentIndex = i;
            }
        }
        
        openList.removeAt(currentIndex);
        closedList[current->pos.y()][current->pos.x()] = true;
        
        if (current->pos == end) {
            return reconstructPath(nodeMap, start, end);
        }
        
        for (const QPoint& dir : directions) {
            QPoint neighbor = current->pos + dir;
            
            if (!isValid(neighbor.x(), neighbor.y(), grid) || 
                closedList[neighbor.y()][neighbor.x()]) {
                continue;
            }
            
            int tentativeG = current->g + 1;
            
            if (nodeMap[neighbor.y()][neighbor.x()].pos == QPoint(-1, -1)) {
                nodeMap[neighbor.y()][neighbor.x()].pos = neighbor;
                nodeMap[neighbor.y()][neighbor.x()].g = tentativeG;
                nodeMap[neighbor.y()][neighbor.x()].h = heuristic(neighbor, end);
                nodeMap[neighbor.y()][neighbor.x()].f = tentativeG + nodeMap[neighbor.y()][neighbor.x()].h;
                nodeMap[neighbor.y()][neighbor.x()].parent = current->pos;
                openList.append(&nodeMap[neighbor.y()][neighbor.x()]);
            } else if (tentativeG < nodeMap[neighbor.y()][neighbor.x()].g) {
                nodeMap[neighbor.y()][neighbor.x()].g = tentativeG;
                nodeMap[neighbor.y()][neighbor.x()].f = tentativeG + nodeMap[neighbor.y()][neighbor.x()].h;
                nodeMap[neighbor.y()][neighbor.x()].parent = current->pos;
                
                if (!openList.contains(&nodeMap[neighbor.y()][neighbor.x()])) {
                    openList.append(&nodeMap[neighbor.y()][neighbor.x()]);
                }
            }
        }
    }
    
    return QList<QPoint>(); // 未找到路径
}

QList<QPoint> PathfindingExecutor::executeDijkstra(const QVector<QVector<int>>& grid,
                                                   const QPoint& start,
                                                   const QPoint& end)
{
    int rows = grid.size();
    int cols = grid[0].size();
    
    QVector<QVector<int>> dist(rows, QVector<int>(cols, INT_MAX));
    QVector<QVector<QPoint>> cameFrom(rows, QVector<QPoint>(cols, QPoint(-1, -1)));
    QList<QPoint> queue;
    
    dist[start.y()][start.x()] = 0;
    queue.append(start);
    
    QVector<QPoint> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    
    while (!queue.isEmpty()) {
        // 找到距离最小的点
        QPoint current = queue[0];
        int currentIndex = 0;
        for (int i = 1; i < queue.size(); ++i) {
            if (dist[queue[i].y()][queue[i].x()] < dist[current.y()][current.x()]) {
                current = queue[i];
                currentIndex = i;
            }
        }
        
        queue.removeAt(currentIndex);
        
        if (current == end) {
            return reconstructPath(cameFrom, start, end);
        }
        
        for (const QPoint& dir : directions) {
            QPoint neighbor = current + dir;
            
            if (!isValid(neighbor.x(), neighbor.y(), grid)) {
                continue;
            }
            
            int newDist = dist[current.y()][current.x()] + 1;
            
            if (newDist < dist[neighbor.y()][neighbor.x()]) {
                dist[neighbor.y()][neighbor.x()] = newDist;
                cameFrom[neighbor.y()][neighbor.x()] = current;
                
                if (!queue.contains(neighbor)) {
                    queue.append(neighbor);
                }
            }
        }
    }
    
    return QList<QPoint>(); // 未找到路径
}

QList<QPoint> PathfindingExecutor::executeBFS(const QVector<QVector<int>>& grid,
                                              const QPoint& start,
                                              const QPoint& end)
{
    int rows = grid.size();
    int cols = grid[0].size();
    
    QQueue<QPoint> queue;
    QVector<QVector<bool>> visited(rows, QVector<bool>(cols, false));
    QVector<QVector<QPoint>> cameFrom(rows, QVector<QPoint>(cols, QPoint(-1, -1)));
    
    queue.enqueue(start);
    visited[start.y()][start.x()] = true;
    
    QVector<QPoint> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    
    while (!queue.isEmpty()) {
        QPoint current = queue.dequeue();
        
        if (current == end) {
            return reconstructPath(cameFrom, start, end);
        }
        
        for (const QPoint& dir : directions) {
            QPoint neighbor = current + dir;
            
            if (isValid(neighbor.x(), neighbor.y(), grid) && 
                !visited[neighbor.y()][neighbor.x()]) {
                
                visited[neighbor.y()][neighbor.x()] = true;
                cameFrom[neighbor.y()][neighbor.x()] = current;
                queue.enqueue(neighbor);
            }
        }
    }
    
    return QList<QPoint>(); // 未找到路径
}

QList<QPoint> PathfindingExecutor::executeDFS(const QVector<QVector<int>>& grid,
                                              const QPoint& start,
                                              const QPoint& end)
{
    int rows = grid.size();
    int cols = grid[0].size();
    
    QVector<QVector<bool>> visited(rows, QVector<bool>(cols, false));
    QList<QPoint> path;
    
    // 递归DFS函数
    std::function<bool(const QPoint&)> dfsRecursive = [&](const QPoint& current) -> bool {
        if (current == end) {
            path.append(current);
            return true;
        }
        
        if (!isValid(current.x(), current.y(), grid) || 
            visited[current.y()][current.x()]) {
            return false;
        }
        
        visited[current.y()][current.x()] = true;
        path.append(current);
        
        QVector<QPoint> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
        
        for (const QPoint& dir : directions) {
            QPoint neighbor = current + dir;
            if (dfsRecursive(neighbor)) {
                return true;
            }
        }
        
        // 回溯
        path.removeLast();
        return false;
    };
    
    if (dfsRecursive(start)) {
        return path;
    } else {
        return QList<QPoint>(); // 未找到路径
    }
}

bool PathfindingExecutor::isValid(int x, int y, const QVector<QVector<int>>& grid)
{
    return x >= 0 && x < grid[0].size() && y >= 0 && y < grid.size() && grid[y][x] == 0;
}

int PathfindingExecutor::heuristic(const QPoint& a, const QPoint& b)
{
    return abs(a.x() - b.x()) + abs(a.y() - b.y()); // 曼哈顿距离
}

QList<QPoint> PathfindingExecutor::reconstructPath(const QVector<QVector<QPoint>>& cameFrom,
                                                   const QPoint& start,
                                                   const QPoint& end)
{
    QList<QPoint> path;
    QPoint current = end;
    
    while (current != QPoint(-1, -1)) {
        path.prepend(current);
        if (current == start) {
            break;
        }
        current = cameFrom[current.y()][current.x()];
    }
    
    return path;
}

QList<QPoint> PathfindingExecutor::reconstructPath(const QVector<QVector<Node>>& nodeMap,
                                                   const QPoint& start,
                                                   const QPoint& end)
{
    QList<QPoint> path;
    QPoint current = end;
    
    while (current != QPoint(-1, -1)) {
        path.prepend(current);
        if (current == start) {
            break;
        }
        current = nodeMap[current.y()][current.x()].parent;
    }
    
    return path;
}

void PathfindingExecutor::executeCodeSilently(const QString& code, 
                                               const QVector<QVector<int>>& grid,
                                               const QPoint& start,
                                               const QPoint& end)
{
    if (grid.isEmpty() || grid[0].isEmpty()) {
        return; // 静默失败
    }
    
    if (start.x() < 0 || start.y() < 0 || end.x() < 0 || end.y() < 0) {
        return; // 静默失败
    }
    
    if (!isValid(start.x(), start.y(), grid)) {
        return; // 静默失败
    }
    
    if (!isValid(end.x(), end.y(), grid)) {
        return; // 静默失败
    }

    // 检测算法类型
    AlgorithmType algorithm = detectAlgorithm(code);
    if (algorithm == Unknown) {
        return; // 静默失败
    }

    // 执行对应的算法
    QList<QPoint> path;
    try {
        switch (algorithm) {
            case AStar:
                path = executeAStar(grid, start, end);
                break;
            case Dijkstra:
                path = executeDijkstra(grid, start, end);
                break;
            case BFS:
                path = executeBFS(grid, start, end);
                break;
            case DFS:
                path = executeDFS(grid, start, end);
                break;
            case DStar:
                path = executeDStar(grid, start, end);
                break;
            default:
                return; // 静默失败
        }
        
        if (!path.isEmpty()) {
            emit pathFound(path); // 只有成功时才发出信号
        }
    } catch (...) {
        // 静默失败，不发出错误信号
    }
}

QList<QPoint> PathfindingExecutor::executeDStar(const QVector<QVector<int>>& grid,
                                                const QPoint& start,
                                                const QPoint& end)
{
    int rows = grid.size();
    int cols = grid[0].size();
    
    // D*算法的简化实现（实际上使用A*算法作为基础）
    // 在静态环境中，D*算法退化为A*算法
    // 这里提供一个基本的实现用于演示
    
    struct DStarNode {
        QPoint pos;
        int g, h, k;
        QPoint parent;
        bool inOpenList;
        bool inClosedList;
        
        DStarNode() : pos(-1, -1), g(INT_MAX), h(0), k(INT_MAX), 
                     parent(-1, -1), inOpenList(false), inClosedList(false) {}
        DStarNode(const QPoint& p) : pos(p), g(INT_MAX), h(0), k(INT_MAX), 
                                    parent(-1, -1), inOpenList(false), inClosedList(false) {}
    };
    
    QVector<QVector<DStarNode>> nodeMap(rows, QVector<DStarNode>(cols));
    QList<DStarNode*> openList;
    
    // 初始化节点映射
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            nodeMap[i][j].pos = QPoint(j, i);
        }
    }
    
    // 初始化目标节点
    DStarNode* goal = &nodeMap[end.y()][end.x()];
    goal->g = 0;
    goal->h = 0;
    goal->k = 0;
    goal->inOpenList = true;
    openList.append(goal);
    
    QVector<QPoint> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    
    // 运行D*算法（简化版本）
    while (!openList.isEmpty()) {
        // 找到k值最小的节点
        DStarNode* current = openList[0];
        int minIndex = 0;
        for (int i = 1; i < openList.size(); ++i) {
            if (openList[i]->k < current->k) {
                current = openList[i];
                minIndex = i;
            }
        }
        
        openList.removeAt(minIndex);
        current->inOpenList = false;
        current->inClosedList = true;
        
        // 如果起点已经处理完成，退出
        if (current->pos == start) {
            break;
        }
        
        // 处理邻居节点
        for (const QPoint& dir : directions) {
            QPoint neighborPos = current->pos + dir;
            
            if (!isValid(neighborPos.x(), neighborPos.y(), grid)) {
                continue;
            }
            
            DStarNode* neighbor = &nodeMap[neighborPos.y()][neighborPos.x()];
            
            if (!neighbor->inClosedList) {
                int newG = current->g + 1;
                
                if (!neighbor->inOpenList) {
                    neighbor->g = newG;
                    neighbor->h = heuristic(neighborPos, start);
                    neighbor->k = neighbor->g + neighbor->h;
                    neighbor->parent = current->pos;
                    neighbor->inOpenList = true;
                    openList.append(neighbor);
                } else if (newG < neighbor->g) {
                    neighbor->g = newG;
                    neighbor->k = neighbor->g + neighbor->h;
                    neighbor->parent = current->pos;
                }
            }
        }
    }
    
    // 构建路径
    QList<QPoint> path;
    QPoint current = start;
    
    while (current != QPoint(-1, -1) && current != end) {
        path.append(current);
        if (current == end) break;
        
        DStarNode& node = nodeMap[current.y()][current.x()];
        current = node.parent;
        
        // 防止无限循环
        if (path.size() > rows * cols) {
            break;
        }
    }
    
    if (current == end) {
        path.append(end);
        return path;
    }
    
    return QList<QPoint>(); // 未找到路径
}

void PathfindingExecutor::executeCodeSilentlyWithCallback(const QString& code, 
                                                          const QVector<QVector<int>>& grid,
                                                          const QPoint& start,
                                                          const QPoint& end)
{
    if (grid.isEmpty() || grid[0].isEmpty()) {
        emit noPathFound(tr("网格数据为空！"));
        return;
    }
    
    if (start.x() < 0 || start.y() < 0 || end.x() < 0 || end.y() < 0) {
        emit noPathFound(tr("起点或终点坐标无效！"));
        return;
    }
    
    if (!isValid(start.x(), start.y(), grid)) {
        emit noPathFound(tr("起点位置不可通行！"));
        return;
    }
    
    if (!isValid(end.x(), end.y(), grid)) {
        emit noPathFound(tr("终点位置不可通行！"));
        return;
    }

    // 检测算法类型
    AlgorithmType algorithm = detectAlgorithm(code);
    if (algorithm == Unknown) {
        return; // 静默失败，不发出任何信号
    }

    // 执行对应的算法
    QList<QPoint> path;
    try {
        switch (algorithm) {
            case AStar:
                path = executeAStar(grid, start, end);
                break;
            case Dijkstra:
                path = executeDijkstra(grid, start, end);
                break;
            case BFS:
                path = executeBFS(grid, start, end);
                break;
            case DFS:
                path = executeDFS(grid, start, end);
                break;
            case DStar:
                path = executeDStar(grid, start, end);
                break;
            default:
                return; // 静默失败
        }
        
        if (path.isEmpty()) {
            emit noPathFound(tr("由于障碍物变化，无法找到可通行路径！"));
        } else {
            emit pathFound(path);
        }
    } catch (...) {
        // 静默失败，不发出错误信号
        emit noPathFound(tr("路径计算过程中发生错误！"));
    }
} 