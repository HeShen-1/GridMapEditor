#include "../include/grideditor.h"
#include <QPainter>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QRandomGenerator>
#include <QSet>
#include <QQueue>

GridEditor::GridEditor(QWidget *parent)
    : QWidget(parent), rows(0), cols(0), cellSize(20), currentState(Obstacle),
      startPos(-1, -1), endPos(-1, -1), currentStep(0), currentCarPos(-1, -1),
      isExecuting(false), codeExecutionMode(false)
{
    setMinimumSize(200, 200);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    loadImages();
    
    // 初始化执行定时器
    executionTimer = new QTimer(this);
    connect(executionTimer, &QTimer::timeout, this, &GridEditor::moveToNextPosition);
    executionTimer->setInterval(500); // 500ms间隔
}

void GridEditor::loadImages()
{
    // 加载并缩放图片
    if (!carImage.load(":/images/pic/car.png")) {
        qDebug() << "Failed to load car.png";
    } else {
        carImage = carImage.scaled(cellSize, cellSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    
    if (!flagImage.load(":/images/pic/flag.png")) {
        qDebug() << "Failed to load flag.png";
    } else {
        flagImage = flagImage.scaled(cellSize, cellSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
}

void GridEditor::createGrid(int newRows, int newCols)
{
    rows = newRows;
    cols = newCols;
    grid.resize(rows);
    for (int i = 0; i < rows; ++i) {
        grid[i].resize(cols);
        for (int j = 0; j < cols; ++j) {
            grid[i][j] = Empty;
        }
    }
    startPos = QPoint(-1, -1);
    endPos = QPoint(-1, -1);
    updateCellSize();
    updateGridOffset();
    update();
}

void GridEditor::clearGrid()
{
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            grid[i][j] = Empty;
        }
    }
    startPos = QPoint(-1, -1);
    endPos = QPoint(-1, -1);
    update();
}

void GridEditor::setCellState(const QPoint& pos, CellState state)
{
    if (!isValidGridPos(pos)) return;

    // 在代码执行模式下，限制修改操作
    if (codeExecutionMode) {
        // 只允许修改障碍物
        if (state != Obstacle && state != Empty) {
            return; // 不允许设置起点和终点
        }
        
        // 不允许修改现有的起点和终点
        if (grid[pos.y()][pos.x()] == Start || grid[pos.y()][pos.x()] == End) {
            return;
        }
    }

    bool hasChanged = false;
    CellState oldState = grid[pos.y()][pos.x()];

    // 如果要设置的位置已经有起点或终点，先清除它
    if (grid[pos.y()][pos.x()] == Start) {
        startPos = QPoint(-1, -1);
    }
    else if (grid[pos.y()][pos.x()] == End) {
        endPos = QPoint(-1, -1);
    }

    // 如果是设置起点
    if (state == Start) {
        // 如果已经有起点，先清除原来的起点
        if (startPos != QPoint(-1, -1)) {
            grid[startPos.y()][startPos.x()] = Empty;
        }
        startPos = pos;
        // 清除该位置的其他状态（如VisitedPath等）
        grid[pos.y()][pos.x()] = Start;
        hasChanged = true;
    }
    // 如果是设置终点
    else if (state == End) {
        // 如果已经有终点，先清除原来的终点
        if (endPos != QPoint(-1, -1)) {
            grid[endPos.y()][endPos.x()] = Empty;
        }
        endPos = pos;
        // 清除该位置的其他状态
        grid[pos.y()][pos.x()] = End;
        hasChanged = true;
    }
    // 如果是设置为空或障碍
    else {
        // 如果当前位置是起点或终点，需要清除对应的标记
        if (pos == startPos) {
            startPos = QPoint(-1, -1);
        }
        if (pos == endPos) {
            endPos = QPoint(-1, -1);
        }
        // 设置新的状态
        grid[pos.y()][pos.x()] = state;
        hasChanged = (oldState != state);
    }

    // 如果栅格发生了变化，发出信号
    if (hasChanged) {
        emit gridChanged();
    }

    update();
}

GridEditor::CellState GridEditor::getCellState(const QPoint& pos) const
{
    if (!isValidGridPos(pos)) return Empty;
    return grid[pos.y()][pos.x()];
}

void GridEditor::updateCellSize()
{
    if (rows <= 0 || cols <= 0) return;
    
    // 计算合适的单元格大小
    int maxCellWidth = width() / cols;
    int maxCellHeight = height() / rows;
    cellSize = qMin(maxCellWidth, maxCellHeight);
    
    // 确保单元格大小不小于最小值
    cellSize = qMax(cellSize, 20);

    // 重新加载并缩放图片
    loadImages();
}

void GridEditor::updateGridOffset()
{
    if (rows <= 0 || cols <= 0) return;
    
    // 计算栅格的总大小
    int gridWidth = cols * cellSize;
    int gridHeight = rows * cellSize;
    
    // 计算偏移量使栅格居中
    gridOffset.setX((width() - gridWidth) / 2);
    gridOffset.setY((height() - gridHeight) / 2);
}

void GridEditor::paintEvent(QPaintEvent * /* event */)
{
    if (rows <= 0 || cols <= 0) {
        // 如果还没有创建栅格，显示提示信息
        QPainter painter(this);
        painter.setPen(Qt::gray);
        painter.drawText(rect(), Qt::AlignCenter, tr("请创建栅格地图"));
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制栅格
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            QRect cell(gridOffset.x() + j * cellSize,
                      gridOffset.y() + i * cellSize,
                      cellSize, cellSize);
            
            // 先填充背景
            painter.fillRect(cell, Qt::white);
            
            // 检查当前位置是否是起点或终点
            QPoint currentPos(j, i);
            bool isStartPosition = (currentPos == startPos);
            bool isEndPosition = (currentPos == endPos);
            
            // 根据状态绘制单元格背景
            if (isStartPosition) {
                // 起点：如果小车不在起点，显示淡绿色背景
                if (!isExecuting || currentCarPos != startPos) {
                    painter.fillRect(cell, QColor(0, 255, 0, 50)); // 淡绿色起点背景
                }
            } else if (isEndPosition) {
                // 终点背景（可选）
                // painter.fillRect(cell, QColor(255, 0, 0, 50)); // 可以添加终点背景色
            } else {
                // 其他位置根据grid状态绘制
                switch (grid[i][j]) {
                    case Obstacle:
                        painter.fillRect(cell, Qt::black);
                        break;
                    case Path:
                        painter.fillRect(cell, QColor(0, 0, 255, 100)); // 半透明蓝色路径
                        break;
                    case VisitedPath:
                        painter.fillRect(cell, QColor(0, 255, 0, 150)); // 绿色走过的路径
                        break;
                    case Current:
                        painter.drawPixmap(cell, carImage);
                        break;
                    default:
                        break;
                }
            }
            
            // 绘制图标层（在背景之上）
            if (isStartPosition) {
                // 起点始终显示小车图标
                painter.drawPixmap(cell, carImage);
            } else if (isEndPosition) {
                // 终点始终显示旗帜
                painter.drawPixmap(cell, flagImage);
            }
            
            // 如果小车在执行中且在非起点终点的位置，绘制移动的小车
            if (isExecuting && currentPos == currentCarPos && !isStartPosition && !isEndPosition) {
                painter.drawPixmap(cell, carImage);
            }
            
            // 绘制网格线
            painter.setPen(Qt::gray);
            painter.drawRect(cell);
        }
    }
}

void GridEditor::mousePressEvent(QMouseEvent *event)
{
    QPoint gridPos = pixelToGrid(event->pos());
    if (!isValidGridPos(gridPos)) return;

    if (event->button() == Qt::RightButton) {
        handleRightClick(gridPos);
    } else if (event->button() == Qt::LeftButton) {
        setCellState(gridPos, currentState);
    }
}

void GridEditor::mouseMoveEvent(QMouseEvent *event)
{
    QPoint gridPos = pixelToGrid(event->pos());
    if (!isValidGridPos(gridPos)) return;

    if (event->buttons() & Qt::LeftButton) {
        // 只有在绘制障碍物时才允许拖动
        if (currentState == Obstacle) {
            setCellState(gridPos, currentState);
        }
    } else if (event->buttons() & Qt::RightButton) {
        // 允许拖动右键来清除
        handleRightClick(gridPos);
    }
}

void GridEditor::handleRightClick(const QPoint& pos)
{
    // 只有当点击的是障碍物时才清除
    if (grid[pos.y()][pos.x()] == Obstacle) {
        setCellState(pos, Empty);
    }
}

void GridEditor::resizeEvent(QResizeEvent * /* event */)
{
    updateCellSize();
    updateGridOffset();
}

QPoint GridEditor::pixelToGrid(const QPoint& pixel) const
{
    int x = (pixel.x() - gridOffset.x()) / cellSize;
    int y = (pixel.y() - gridOffset.y()) / cellSize;
    return QPoint(x, y);
}

bool GridEditor::isValidGridPos(const QPoint& pos) const
{
    return pos.x() >= 0 && pos.x() < cols && pos.y() >= 0 && pos.y() < rows;
}

bool GridEditor::saveToJson(const QString& filename) const
{
    QJsonObject json;
    
    // 保存网格基本信息
    json["rows"] = rows;
    json["cols"] = cols;
    
    // 保存网格数据
    QJsonArray gridData;
    for (int i = 0; i < rows; ++i) {
        QJsonArray rowData;
        for (int j = 0; j < cols; ++j) {
            rowData.append(static_cast<int>(grid[i][j]));
        }
        gridData.append(rowData);
    }
    json["grid"] = gridData;
    
    // 保存起点和终点位置
    if (startPos != QPoint(-1, -1)) {
        QJsonObject startPosObj;
        startPosObj["x"] = startPos.x();
        startPosObj["y"] = startPos.y();
        json["startPos"] = startPosObj;
    }
    
    if (endPos != QPoint(-1, -1)) {
        QJsonObject endPosObj;
        endPosObj["x"] = endPos.x();
        endPosObj["y"] = endPos.y();
        json["endPos"] = endPosObj;
    }
    
    // 写入文件
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QJsonDocument doc(json);
    file.write(doc.toJson());
    return true;
}

bool GridEditor::loadFromJson(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray data = file.readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    
    if (doc.isNull() || parseError.error != QJsonParseError::NoError) {
        // JSON解析失败，设置错误信息供MainWindow显示
        lastErrorMessage = tr("无法解析JSON文件: %1").arg(parseError.errorString());
        return false;
    }
    
    QJsonObject json = doc.object();
    
    // 验证必要的字段是否存在
    if (!json.contains("rows") || !json.contains("cols") || !json.contains("grid")) {
        lastErrorMessage = tr("JSON文件缺少必要的字段（rows、cols、grid）");
        return false;
    }
    
    // 读取网格基本信息
    int newRows = json["rows"].toInt();
    int newCols = json["cols"].toInt();
    
    if (newRows <= 0 || newCols <= 0) {
        lastErrorMessage = tr("网格尺寸无效（行数: %1, 列数: %2）").arg(newRows).arg(newCols);
        return false;
    }
    
    // 验证网格数据
    QJsonArray gridData = json["grid"].toArray();
    if (gridData.size() != newRows) {
        lastErrorMessage = tr("网格数据行数与声明不符");
        return false;
    }
    
    // 创建新网格
    createGrid(newRows, newCols);
    
    // 读取网格数据
    for (int i = 0; i < rows; ++i) {
        if (i >= gridData.size()) {
            lastErrorMessage = tr("网格数据不完整");
            return false;
        }
        
        QJsonArray rowData = gridData[i].toArray();
        if (rowData.size() != newCols) {
            lastErrorMessage = tr("第%1行数据列数与声明不符").arg(i + 1);
            return false;
        }
        
        for (int j = 0; j < cols; ++j) {
            int cellValue = rowData[j].toInt();
            if (cellValue < 0 || cellValue > static_cast<int>(VisitedPath)) {
                lastErrorMessage = tr("网格数据包含无效值: %1").arg(cellValue);
                return false;
            }
            grid[i][j] = static_cast<CellState>(cellValue);
        }
    }
    
    // 读取起点和终点位置
    startPos = QPoint(-1, -1);
    if (json.contains("startPos")) {
        QJsonObject startPosObj = json["startPos"].toObject();
        if (startPosObj.contains("x") && startPosObj.contains("y")) {
            int x = startPosObj["x"].toInt();
            int y = startPosObj["y"].toInt();
            if (x >= 0 && x < newCols && y >= 0 && y < newRows) {
                startPos = QPoint(x, y);
            }
        }
    }
    
    endPos = QPoint(-1, -1);
    if (json.contains("endPos")) {
        QJsonObject endPosObj = json["endPos"].toObject();
        if (endPosObj.contains("x") && endPosObj.contains("y")) {
            int x = endPosObj["x"].toInt();
            int y = endPosObj["y"].toInt();
            if (x >= 0 && x < newCols && y >= 0 && y < newRows) {
                endPos = QPoint(x, y);
            }
        }
    }
    
    lastErrorMessage.clear(); // 清除错误信息
    update();  // 重绘界面
    return true;
}

// 路径执行功能实现
void GridEditor::executePathfinding(const QList<QPoint>& path)
{
    if (path.isEmpty()) {
        emit executionError(tr("路径为空！"));
        return;
    }
    
    if (isExecuting) {
        stopExecution();
    }
    
    // 验证路径的有效性
    for (int i = 0; i < path.size(); ++i) {
        const QPoint& pos = path[i];
        if (!isValidGridPos(pos)) {
            emit executionError(tr("路径包含无效坐标: (%1, %2)").arg(pos.x()).arg(pos.y()));
            return;
        }
        
        // 检查路径点是否可通行（除了起点和终点）
        if (i > 0 && i < path.size() - 1) {
            CellState state = grid[pos.y()][pos.x()];
            if (state == Obstacle) {
                emit executionError(tr("路径经过障碍物: (%1, %2)").arg(pos.x()).arg(pos.y()));
                return;
            }
        }
        
        // 检查相邻步骤是否连续（只能移动到相邻格子）
        if (i > 0) {
            const QPoint& prevPos = path[i-1];
            int dx = abs(pos.x() - prevPos.x());
            int dy = abs(pos.y() - prevPos.y());
            if (dx + dy != 1) {
                emit executionError(tr("路径不连续: 从(%1, %2)到(%3, %4)")
                    .arg(prevPos.x()).arg(prevPos.y()).arg(pos.x()).arg(pos.y()));
                return;
            }
        }
    }
    
    // 验证起点和终点
    if (startPos == QPoint(-1, -1) || endPos == QPoint(-1, -1)) {
        emit executionError(tr("请先设置起点和终点！"));
        return;
    }
    
    if (path.first() != startPos) {
        emit executionError(tr("路径起点与地图起点不匹配！"));
        return;
    }
    
    if (path.last() != endPos) {
        emit executionError(tr("路径终点与地图终点不匹配！"));
        return;
    }
    
    // 清除之前的路径显示
    clearPath();
    
    // 设置路径显示
    for (int i = 1; i < path.size() - 1; ++i) {
        const QPoint& pos = path[i];
        if (grid[pos.y()][pos.x()] == Empty) {
            grid[pos.y()][pos.x()] = Path;
        }
    }
    
    // 开始执行
    currentPath = path;
    currentStep = 0;
    currentCarPos = startPos;
    isExecuting = true;
    
    update();
    executionTimer->start();
}

void GridEditor::clearPath()
{
    bool pathWasCleared = false;
    
    // 清除路径显示，但保护起点和终点
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            QPoint pos(j, i);
            // 如果是起点或终点，跳过
            if (pos == startPos || pos == endPos) {
                continue;
            }
            
            if (grid[i][j] == Path || grid[i][j] == Current || grid[i][j] == VisitedPath) {
                grid[i][j] = Empty;
                pathWasCleared = true;
            }
        }
    }
    
    // 确保起点和终点状态正确
    if (startPos != QPoint(-1, -1)) {
        grid[startPos.y()][startPos.x()] = Start;
    }
    if (endPos != QPoint(-1, -1)) {
        grid[endPos.y()][endPos.x()] = End;
    }
    
    // 如果清除了路径，发出信号
    if (pathWasCleared) {
        emit pathCleared();
    }
    
    update();
}

void GridEditor::clearPathSilently()
{
    // 清除路径显示，但保护起点和终点，不发出信号
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            QPoint pos(j, i);
            // 如果是起点或终点，跳过
            if (pos == startPos || pos == endPos) {
                continue;
            }
            
            if (grid[i][j] == Path || grid[i][j] == Current || grid[i][j] == VisitedPath) {
                grid[i][j] = Empty;
            }
        }
    }
    
    // 确保起点和终点状态正确
    if (startPos != QPoint(-1, -1)) {
        grid[startPos.y()][startPos.x()] = Start;
    }
    if (endPos != QPoint(-1, -1)) {
        grid[endPos.y()][endPos.x()] = End;
    }
    
    update();
}

void GridEditor::stopExecution()
{
    if (!isExecuting) return;
    
    executionTimer->stop();
    isExecuting = false;
    
    // 小车回到起点
    currentCarPos = startPos;
    
    clearPath();
}

void GridEditor::stopExecutionSilently()
{
    if (!isExecuting) return;
    
    executionTimer->stop();
    isExecuting = false;
    
    // 小车回到起点
    currentCarPos = startPos;
    
    // 静默清除路径，不发出pathCleared信号
    clearPathSilently();
}

void GridEditor::moveToNextPosition()
{
    if (!isExecuting || currentStep >= currentPath.size()) {
        stopExecutionSilently(); // 正常完成时静默停止
        emit executionFinished();
        return;
    }
    
    // 如果不是第一步，将上一个位置标记为绿色走过的路径
    if (currentStep > 0) {
        QPoint prevPos = currentPath[currentStep - 1];
        // 只有非起点和终点的位置才标记为绿色
        if (prevPos != startPos && prevPos != endPos) {
            grid[prevPos.y()][prevPos.x()] = VisitedPath;
        }
        // 确保起点和终点的grid状态正确
        if (startPos != QPoint(-1, -1)) {
            grid[startPos.y()][startPos.x()] = Start;
        }
        if (endPos != QPoint(-1, -1)) {
            grid[endPos.y()][endPos.x()] = End;
        }
    }
    
    currentCarPos = currentPath[currentStep];
    currentStep++;
    
    update();
    
    // 如果到达终点
    if (currentStep >= currentPath.size()) {
        QTimer::singleShot(1000, this, [this]() {
            // 到达终点后，返回起点
            currentCarPos = startPos;
            update();
            
            // 再等1秒后结束执行
            QTimer::singleShot(1000, this, [this]() {
                stopExecutionSilently(); // 正常完成时静默停止
                emit executionFinished();
            });
        });
    }
}

QVector<QVector<int>> GridEditor::getGridData() const
{
    QVector<QVector<int>> data(rows);
    for (int i = 0; i < rows; ++i) {
        data[i].resize(cols);
        for (int j = 0; j < cols; ++j) {
            // 转换为算法所需的格式：0-可通行，1-障碍
            if (grid[i][j] == Obstacle) {
                data[i][j] = 1;
            } else {
                data[i][j] = 0;
            }
        }
    }
    return data;
}

bool GridEditor::hasValidStartAndEnd() const
{
    return startPos != QPoint(-1, -1) && endPos != QPoint(-1, -1);
}

void GridEditor::setCodeExecutionMode(bool enabled)
{
    codeExecutionMode = enabled;
    
    // 如果退出执行模式
    if (!enabled) {
        // 如果小车正在移动，立即停止
        if (isExecuting) {
            stopExecution();
        } else {
            // 如果没有移动，只清除路径显示
            clearPath();
        }
    }
}

bool GridEditor::hasPath() const
{
    // 检查网格中是否有路径相关的状态
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            CellState state = grid[i][j];
            if (state == Path || state == Current || state == VisitedPath) {
                return true;
            }
        }
    }
    return false;
}

void GridEditor::generateRandomObstacles(double density, int connectivityType, int pathCount, bool useSeed, int seed)
{
    if (rows <= 0 || cols <= 0) {
        return;
    }
    
    if (startPos == QPoint(-1, -1) || endPos == QPoint(-1, -1)) {
        return;
    }
    
    // 设置随机种子
    QRandomGenerator* generator;
    if (useSeed) {
        generator = new QRandomGenerator(seed);
    } else {
        generator = QRandomGenerator::global();
    }
    
    // 清除现有障碍物（保留起点和终点）
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            QPoint pos(j, i);
            if (pos != startPos && pos != endPos) {
                grid[i][j] = Empty;
            }
        }
    }
    
    // 计算要生成的障碍物数量
    int totalCells = rows * cols;
    int availableCells = totalCells - 2; // 减去起点和终点
    int targetObstacles = static_cast<int>(availableCells * density);
    
    // 根据连通性类型生成障碍物
    switch (connectivityType) {
        case 0: // 无可通行通路
            generateObstaclesWithNoPath(generator, targetObstacles);
            break;
        case 1: // 一条可通行通路
            generateObstaclesWithOnePath(generator, targetObstacles);
            break;
        case 2: // 多条可通行通路
            generateObstaclesWithMultiplePaths(generator, targetObstacles, pathCount);
            break;
    }
    
    // 如果使用了自定义种子，需要删除生成器
    if (useSeed) {
        delete generator;
    }
    
    emit gridChanged();
    update();
}

void GridEditor::generateObstaclesWithNoPath(QRandomGenerator* generator, int targetObstacles)
{
    // 生成随机障碍物，直到起点和终点不连通
    QVector<QPoint> availablePositions;
    
    // 收集所有可用位置（除了起点和终点）
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            QPoint pos(j, i);
            if (pos != startPos && pos != endPos) {
                availablePositions.append(pos);
            }
        }
    }
    
    // 随机打乱位置
    for (int i = availablePositions.size() - 1; i > 0; --i) {
        int j = generator->bounded(i + 1);
        availablePositions.swapItemsAt(i, j);
    }
    
    // 逐步添加障碍物，直到达到目标数量或者起点终点不连通
    int addedObstacles = 0;
    for (const QPoint& pos : availablePositions) {
        if (addedObstacles >= targetObstacles) break;
        
        // 临时设置障碍物
        grid[pos.y()][pos.x()] = Obstacle;
        
        // 检查连通性
        if (!isPathExists(startPos, endPos)) {
            // 已经不连通了，目标达成
            addedObstacles++;
            break;
        }
        
        addedObstacles++;
    }
}

void GridEditor::generateObstaclesWithOnePath(QRandomGenerator* generator, int targetObstacles)
{
    // 首先使用BFS找到一条从起点到终点的路径
    QList<QPoint> path = findPathBFS(startPos, endPos);
    if (path.isEmpty()) {
        return; // 如果找不到路径，直接返回
    }
    
    // 将路径上的点标记为受保护的
    QList<QPoint> protectedCells = path;
    
    // 收集所有可用位置（除了路径上的点）
    QVector<QPoint> availablePositions;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            QPoint pos(j, i);
            if (!protectedCells.contains(pos)) {
                availablePositions.append(pos);
            }
        }
    }
    
    // 随机打乱位置
    for (int i = availablePositions.size() - 1; i > 0; --i) {
        int j = generator->bounded(i + 1);
        availablePositions.swapItemsAt(i, j);
    }
    
    // 逐步添加障碍物，确保只有一条路径存在
    int addedObstacles = 0;
    for (const QPoint& pos : availablePositions) {
        if (addedObstacles >= targetObstacles) break;
        
        // 临时设置障碍物
        grid[pos.y()][pos.x()] = Obstacle;
        
        // 检查是否仍然只有一条路径
        if (countPaths(startPos, endPos) == 1) {
            addedObstacles++;
        } else {
            // 如果路径数量不是1，撤销这个障碍物
            grid[pos.y()][pos.x()] = Empty;
        }
    }
}

void GridEditor::generateObstaclesWithMultiplePaths(QRandomGenerator* generator, int targetObstacles, int pathCount)
{
    // 收集所有可用位置（除了起点和终点）
    QVector<QPoint> availablePositions;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            QPoint pos(j, i);
            if (pos != startPos && pos != endPos) {
                availablePositions.append(pos);
            }
        }
    }
    
    // 随机打乱位置
    for (int i = availablePositions.size() - 1; i > 0; --i) {
        int j = generator->bounded(i + 1);
        availablePositions.swapItemsAt(i, j);
    }
    
    // 逐步添加障碍物，确保至少有指定数量的路径
    int addedObstacles = 0;
    for (const QPoint& pos : availablePositions) {
        if (addedObstacles >= targetObstacles) break;
        
        // 临时设置障碍物
        grid[pos.y()][pos.x()] = Obstacle;
        
        // 检查路径数量
        int currentPaths = countPaths(startPos, endPos);
        if (currentPaths >= pathCount) {
            addedObstacles++;
        } else {
            // 如果路径数量不足，撤销这个障碍物
            grid[pos.y()][pos.x()] = Empty;
        }
    }
}

bool GridEditor::isPathExists(const QPoint& start, const QPoint& end)
{
    if (start == QPoint(-1, -1) || end == QPoint(-1, -1)) {
        return false;
    }
    
    QVector<QVector<bool>> visited(rows, QVector<bool>(cols, false));
    QQueue<QPoint> queue;
    
    queue.enqueue(start);
    visited[start.y()][start.x()] = true;
    
    // BFS搜索
    while (!queue.isEmpty()) {
        QPoint current = queue.dequeue();
        
        if (current == end) {
            return true;
        }
        
        // 检查四个方向
        QList<QPoint> directions = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
        for (const QPoint& dir : directions) {
            QPoint next(current.x() + dir.x(), current.y() + dir.y());
            
            if (isValidGridPos(next) && !visited[next.y()][next.x()]) {
                // 如果是空格子、起点或终点，就可以通过
                CellState state = grid[next.y()][next.x()];
                if (state == Empty || next == start || next == end) {
                    visited[next.y()][next.x()] = true;
                    queue.enqueue(next);
                }
            }
        }
    }
    
    return false;
}

QList<QPoint> GridEditor::findPathBFS(const QPoint& start, const QPoint& end)
{
    QList<QPoint> path;
    if (start == QPoint(-1, -1) || end == QPoint(-1, -1)) {
        return path;
    }
    
    QVector<QVector<bool>> visited(rows, QVector<bool>(cols, false));
    QQueue<QPoint> queue;
    QVector<QVector<QPoint>> parent(rows, QVector<QPoint>(cols, QPoint(-1, -1))); // 用于回溯路径
    
    queue.enqueue(start);
    visited[start.y()][start.x()] = true;
    
    while (!queue.isEmpty()) {
        QPoint current = queue.dequeue();
        
        if (current == end) {
            // 回溯构建路径
            QPoint pathPoint = end;
            while (pathPoint != QPoint(-1, -1)) {
                path.prepend(pathPoint);
                pathPoint = parent[pathPoint.y()][pathPoint.x()];
            }
            return path;
        }
        
        // 检查四个方向
        QList<QPoint> directions = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
        for (const QPoint& dir : directions) {
            QPoint next(current.x() + dir.x(), current.y() + dir.y());
            
            if (isValidGridPos(next) && !visited[next.y()][next.x()]) {
                CellState state = grid[next.y()][next.x()];
                if (state == Empty || next == start || next == end) {
                    visited[next.y()][next.x()] = true;
                    queue.enqueue(next);
                    parent[next.y()][next.x()] = current;
                }
            }
        }
    }
    
    return path; // 返回空路径
}

int GridEditor::countPaths(const QPoint& start, const QPoint& end)
{
    // 简化版本：如果存在路径返回1，否则返回0
    // 对于复杂的多路径计数，需要更复杂的算法
    return isPathExists(start, end) ? 1 : 0;
} 