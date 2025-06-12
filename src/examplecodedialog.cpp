#include "../include/examplecodedialog.h"

ExampleCodeDialog::ExampleCodeDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("选择示例代码"));
    setModal(true);
    resize(800, 600);
    
    setupUI();
    loadExampleCodes();
    
    // 设置默认选择
    algorithmComboBox->setCurrentIndex(0);
    languageComboBox->setCurrentIndex(0);
    onAlgorithmChanged();
}

QString ExampleCodeDialog::getSelectedCode() const
{
    return selectedCode;
}

void ExampleCodeDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 选择区域
    QHBoxLayout *selectionLayout = new QHBoxLayout();
    
    QLabel *algorithmLabel = new QLabel(tr("选择算法:"));
    algorithmComboBox = new QComboBox();
    algorithmComboBox->addItem(tr("A* 算法"));
    algorithmComboBox->addItem(tr("Dijkstra 算法"));
    algorithmComboBox->addItem(tr("BFS 广度优先搜索"));
    algorithmComboBox->addItem(tr("DFS 深度优先搜索"));
    algorithmComboBox->addItem(tr("D* 算法"));
    
    QLabel *languageLabel = new QLabel(tr("选择语言:"));
    languageComboBox = new QComboBox();
    languageComboBox->addItem("C++");
    languageComboBox->addItem("Java");
    languageComboBox->addItem("Python");
    
    selectionLayout->addWidget(algorithmLabel);
    selectionLayout->addWidget(algorithmComboBox);
    selectionLayout->addWidget(languageLabel);
    selectionLayout->addWidget(languageComboBox);
    selectionLayout->addStretch();
    
    // 代码预览区域
    QLabel *previewLabel = new QLabel(tr("代码预览:"));
    codePreview = new QTextEdit();
    codePreview->setReadOnly(true);
    codePreview->setFont(QFont("Consolas", 10));
    
    // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    selectButton = new QPushButton(tr("选择此代码"));
    cancelButton = new QPushButton(tr("取消"));
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(selectButton);
    buttonLayout->addWidget(cancelButton);
    
    // 添加到主布局
    mainLayout->addLayout(selectionLayout);
    mainLayout->addWidget(previewLabel);
    mainLayout->addWidget(codePreview);
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号
    connect(algorithmComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ExampleCodeDialog::onAlgorithmChanged);
    connect(languageComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ExampleCodeDialog::onLanguageChanged);
    connect(selectButton, &QPushButton::clicked, this, &ExampleCodeDialog::onCodeSelected);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void ExampleCodeDialog::loadExampleCodes()
{
    // A* 算法 - C++
    exampleCodes["A* 算法-C++"] = R"(#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <algorithm>

struct Node {
    int x, y;
    int g, h, f;
    Node* parent;
    
    Node(int x, int y) : x(x), y(y), g(0), h(0), f(0), parent(nullptr) {}
    
    bool operator>(const Node& other) const {
        return f > other.f;
    }
};

class AStar {
private:
    std::vector<std::vector<int>> grid;
    int rows, cols;
    
    int heuristic(int x1, int y1, int x2, int y2) {
        return std::abs(x1 - x2) + std::abs(y1 - y2); // 曼哈顿距离
    }
    
    bool isValid(int x, int y) {
        return x >= 0 && x < rows && y >= 0 && y < cols && grid[x][y] == 0;
    }
    
public:
    AStar(std::vector<std::vector<int>>& grid) : grid(grid) {
        rows = grid.size();
        cols = grid[0].size();
    }
    
    std::vector<std::pair<int, int>> findPath(int startX, int startY, int endX, int endY) {
        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openList;
        std::vector<std::vector<bool>> closedList(rows, std::vector<bool>(cols, false));
        std::vector<std::vector<Node*>> nodeMap(rows, std::vector<Node*>(cols, nullptr));
        
        Node* startNode = new Node(startX, startY);
        startNode->h = heuristic(startX, startY, endX, endY);
        startNode->f = startNode->g + startNode->h;
        
        openList.push(*startNode);
        nodeMap[startX][startY] = startNode;
        
        int dx[] = {-1, 1, 0, 0};
        int dy[] = {0, 0, -1, 1};
        
        while (!openList.empty()) {
            Node current = openList.top();
            openList.pop();
            
            if (current.x == endX && current.y == endY) {
                // 构建路径
                std::vector<std::pair<int, int>> path;
                Node* node = nodeMap[endX][endY];
                while (node != nullptr) {
                    path.push_back({node->x, node->y});
                    node = node->parent;
                }
                std::reverse(path.begin(), path.end());
                return path;
            }
            
            closedList[current.x][current.y] = true;
            
            for (int i = 0; i < 4; i++) {
                int newX = current.x + dx[i];
                int newY = current.y + dy[i];
                
                if (isValid(newX, newY) && !closedList[newX][newY]) {
                    int newG = nodeMap[current.x][current.y]->g + 1;
                    
                    if (nodeMap[newX][newY] == nullptr) {
                        nodeMap[newX][newY] = new Node(newX, newY);
                        nodeMap[newX][newY]->g = newG;
                        nodeMap[newX][newY]->h = heuristic(newX, newY, endX, endY);
                        nodeMap[newX][newY]->f = nodeMap[newX][newY]->g + nodeMap[newX][newY]->h;
                        nodeMap[newX][newY]->parent = nodeMap[current.x][current.y];
                        openList.push(*nodeMap[newX][newY]);
                    } else if (newG < nodeMap[newX][newY]->g) {
                        nodeMap[newX][newY]->g = newG;
                        nodeMap[newX][newY]->f = nodeMap[newX][newY]->g + nodeMap[newX][newY]->h;
                        nodeMap[newX][newY]->parent = nodeMap[current.x][current.y];
                    }
                }
            }
        }
        
        return {}; // 未找到路径
    }
};)";

    // A* 算法 - Java
    exampleCodes["A* 算法-Java"] = R"(import java.util.*;

class Node implements Comparable<Node> {
    int x, y;
    int g, h, f;
    Node parent;
    
    public Node(int x, int y) {
        this.x = x;
        this.y = y;
        this.g = 0;
        this.h = 0;
        this.f = 0;
        this.parent = null;
    }
    
    @Override
    public int compareTo(Node other) {
        return Integer.compare(this.f, other.f);
    }
}

public class AStar {
    private int[][] grid;
    private int rows, cols;
    
    public AStar(int[][] grid) {
        this.grid = grid;
        this.rows = grid.length;
        this.cols = grid[0].length;
    }
    
    private int heuristic(int x1, int y1, int x2, int y2) {
        return Math.abs(x1 - x2) + Math.abs(y1 - y2); // 曼哈顿距离
    }
    
    private boolean isValid(int x, int y) {
        return x >= 0 && x < rows && y >= 0 && y < cols && grid[x][y] == 0;
    }
    
    public List<int[]> findPath(int startX, int startY, int endX, int endY) {
        PriorityQueue<Node> openList = new PriorityQueue<>();
        boolean[][] closedList = new boolean[rows][cols];
        Node[][] nodeMap = new Node[rows][cols];
        
        Node startNode = new Node(startX, startY);
        startNode.h = heuristic(startX, startY, endX, endY);
        startNode.f = startNode.g + startNode.h;
        
        openList.offer(startNode);
        nodeMap[startX][startY] = startNode;
        
        int[] dx = {-1, 1, 0, 0};
        int[] dy = {0, 0, -1, 1};
        
        while (!openList.isEmpty()) {
            Node current = openList.poll();
            
            if (current.x == endX && current.y == endY) {
                // 构建路径
                List<int[]> path = new ArrayList<>();
                Node node = nodeMap[endX][endY];
                while (node != null) {
                    path.add(new int[]{node.x, node.y});
                    node = node.parent;
                }
                Collections.reverse(path);
                return path;
            }
            
            closedList[current.x][current.y] = true;
            
            for (int i = 0; i < 4; i++) {
                int newX = current.x + dx[i];
                int newY = current.y + dy[i];
                
                if (isValid(newX, newY) && !closedList[newX][newY]) {
                    int newG = nodeMap[current.x][current.y].g + 1;
                    
                    if (nodeMap[newX][newY] == null) {
                        nodeMap[newX][newY] = new Node(newX, newY);
                        nodeMap[newX][newY].g = newG;
                        nodeMap[newX][newY].h = heuristic(newX, newY, endX, endY);
                        nodeMap[newX][newY].f = nodeMap[newX][newY].g + nodeMap[newX][newY].h;
                        nodeMap[newX][newY].parent = nodeMap[current.x][current.y];
                        openList.offer(nodeMap[newX][newY]);
                    } else if (newG < nodeMap[newX][newY].g) {
                        nodeMap[newX][newY].g = newG;
                        nodeMap[newX][newY].f = nodeMap[newX][newY].g + nodeMap[newX][newY].h;
                        nodeMap[newX][newY].parent = nodeMap[current.x][current.y];
                    }
                }
            }
        }
        
        return new ArrayList<>(); // 未找到路径
    }
})";

    // A* 算法 - Python
    exampleCodes["A* 算法-Python"] = R"(import heapq
from typing import List, Tuple, Optional

class Node:
    def __init__(self, x: int, y: int):
        self.x = x
        self.y = y
        self.g = 0
        self.h = 0
        self.f = 0
        self.parent: Optional['Node'] = None
    
    def __lt__(self, other):
        return self.f < other.f

class AStar:
    def __init__(self, grid: List[List[int]]):
        self.grid = grid
        self.rows = len(grid)
        self.cols = len(grid[0])
    
    def heuristic(self, x1: int, y1: int, x2: int, y2: int) -> int:
        """曼哈顿距离"""
        return abs(x1 - x2) + abs(y1 - y2)
    
    def is_valid(self, x: int, y: int) -> bool:
        return 0 <= x < self.rows and 0 <= y < self.cols and self.grid[x][y] == 0
    
    def find_path(self, start_x: int, start_y: int, end_x: int, end_y: int) -> List[Tuple[int, int]]:
        open_list = []
        closed_list = [[False for _ in range(self.cols)] for _ in range(self.rows)]
        node_map = [[None for _ in range(self.cols)] for _ in range(self.rows)]
        
        start_node = Node(start_x, start_y)
        start_node.h = self.heuristic(start_x, start_y, end_x, end_y)
        start_node.f = start_node.g + start_node.h
        
        heapq.heappush(open_list, start_node)
        node_map[start_x][start_y] = start_node
        
        directions = [(-1, 0), (1, 0), (0, -1), (0, 1)]
        
        while open_list:
            current = heapq.heappop(open_list)
            
            if current.x == end_x and current.y == end_y:
                # 构建路径
                path = []
                node = node_map[end_x][end_y]
                while node is not None:
                    path.append((node.x, node.y))
                    node = node.parent
                path.reverse()
                return path
            
            closed_list[current.x][current.y] = True
            
            for dx, dy in directions:
                new_x, new_y = current.x + dx, current.y + dy
                
                if self.is_valid(new_x, new_y) and not closed_list[new_x][new_y]:
                    new_g = node_map[current.x][current.y].g + 1
                    
                    if node_map[new_x][new_y] is None:
                        node_map[new_x][new_y] = Node(new_x, new_y)
                        node_map[new_x][new_y].g = new_g
                        node_map[new_x][new_y].h = self.heuristic(new_x, new_y, end_x, end_y)
                        node_map[new_x][new_y].f = node_map[new_x][new_y].g + node_map[new_x][new_y].h
                        node_map[new_x][new_y].parent = node_map[current.x][current.y]
                        heapq.heappush(open_list, node_map[new_x][new_y])
                    elif new_g < node_map[new_x][new_y].g:
                        node_map[new_x][new_y].g = new_g
                        node_map[new_x][new_y].f = node_map[new_x][new_y].g + node_map[new_x][new_y].h
                        node_map[new_x][new_y].parent = node_map[current.x][current.y]
        
        return []  # 未找到路径)";

    // BFS 算法 - C++
    exampleCodes["BFS 广度优先搜索-C++"] = R"(#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>

struct Node {
    int x, y;
    Node* parent;
    
    Node(int x, int y) : x(x), y(y), parent(nullptr) {}
};

class BFS {
private:
    std::vector<std::vector<int>> grid;
    int rows, cols;
    
    bool isValid(int x, int y) {
        return x >= 0 && x < rows && y >= 0 && y < cols && grid[x][y] == 0;
    }
    
public:
    BFS(std::vector<std::vector<int>>& grid) : grid(grid) {
        rows = grid.size();
        cols = grid[0].size();
    }
    
    std::vector<std::pair<int, int>> findPath(int startX, int startY, int endX, int endY) {
        std::queue<Node*> queue;
        std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
        std::vector<std::vector<Node*>> nodeMap(rows, std::vector<Node*>(cols, nullptr));
        
        Node* startNode = new Node(startX, startY);
        queue.push(startNode);
        visited[startX][startY] = true;
        nodeMap[startX][startY] = startNode;
        
        int dx[] = {-1, 1, 0, 0};
        int dy[] = {0, 0, -1, 1};
        
        while (!queue.empty()) {
            Node* current = queue.front();
            queue.pop();
            
            if (current->x == endX && current->y == endY) {
                // 构建路径
                std::vector<std::pair<int, int>> path;
                Node* node = current;
                while (node != nullptr) {
                    path.push_back({node->x, node->y});
                    node = node->parent;
                }
                std::reverse(path.begin(), path.end());
                return path;
            }
            
            for (int i = 0; i < 4; i++) {
                int newX = current->x + dx[i];
                int newY = current->y + dy[i];
                
                if (isValid(newX, newY) && !visited[newX][newY]) {
                    Node* newNode = new Node(newX, newY);
                    newNode->parent = current;
                    queue.push(newNode);
                    visited[newX][newY] = true;
                    nodeMap[newX][newY] = newNode;
                }
            }
        }
        
        return {}; // 未找到路径
    }
};)";

    // DFS 算法 - Python
    exampleCodes["DFS 深度优先搜索-Python"] = R"(from typing import List, Tuple, Set

class DFS:
    def __init__(self, grid: List[List[int]]):
        self.grid = grid
        self.rows = len(grid)
        self.cols = len(grid[0])
    
    def is_valid(self, x: int, y: int) -> bool:
        return 0 <= x < self.rows and 0 <= y < self.cols and self.grid[x][y] == 0
    
    def find_path(self, start_x: int, start_y: int, end_x: int, end_y: int) -> List[Tuple[int, int]]:
        visited: Set[Tuple[int, int]] = set()
        path: List[Tuple[int, int]] = []
        
        def dfs(x: int, y: int) -> bool:
            if x == end_x and y == end_y:
                path.append((x, y))
                return True
            
            if (x, y) in visited or not self.is_valid(x, y):
                return False
            
            visited.add((x, y))
            path.append((x, y))
            
            # 尝试四个方向
            directions = [(-1, 0), (1, 0), (0, -1), (0, 1)]
            for dx, dy in directions:
                if dfs(x + dx, y + dy):
                    return True
            
            # 回溯
            path.pop()
            return False
        
        if dfs(start_x, start_y):
            return path
        else:
            return []  # 未找到路径)";

    // Dijkstra 算法 - Python
    exampleCodes["Dijkstra 算法-Python"] = R"(import heapq
from typing import List, Tuple, Dict

class Dijkstra:
    def __init__(self, grid: List[List[int]]):
        self.grid = grid
        self.rows = len(grid)
        self.cols = len(grid[0])
    
    def is_valid(self, x: int, y: int) -> bool:
        return 0 <= x < self.rows and 0 <= y < self.cols and self.grid[x][y] == 0
    
    def find_path(self, start_x: int, start_y: int, end_x: int, end_y: int) -> List[Tuple[int, int]]:
        distances = {}
        previous = {}
        pq = [(0, start_x, start_y)]
        distances[(start_x, start_y)] = 0
        
        directions = [(-1, 0), (1, 0), (0, -1), (0, 1)]
        
        while pq:
            current_dist, x, y = heapq.heappop(pq)
            
            if x == end_x and y == end_y:
                # 构建路径
                path = []
                current = (x, y)
                while current in previous:
                    path.append(current)
                    current = previous[current]
                path.append((start_x, start_y))
                path.reverse()
                return path
            
            if current_dist > distances.get((x, y), float('inf')):
                continue
            
            for dx, dy in directions:
                new_x, new_y = x + dx, y + dy
                
                if self.is_valid(new_x, new_y):
                    new_dist = current_dist + 1
                    
                    if new_dist < distances.get((new_x, new_y), float('inf')):
                        distances[(new_x, new_y)] = new_dist
                        previous[(new_x, new_y)] = (x, y)
                        heapq.heappush(pq, (new_dist, new_x, new_y))
        
        return []  # 未找到路径)";

    // BFS 算法 - Java
    exampleCodes["BFS 广度优先搜索-Java"] = R"(import java.util.*;

public class BFS {
    private int[][] grid;
    private int rows, cols;
    
    static class Node {
        int x, y;
        Node parent;
        
        Node(int x, int y) {
            this.x = x;
            this.y = y;
            this.parent = null;
        }
    }
    
    public BFS(int[][] grid) {
        this.grid = grid;
        this.rows = grid.length;
        this.cols = grid[0].length;
    }
    
    private boolean isValid(int x, int y) {
        return x >= 0 && x < rows && y >= 0 && y < cols && grid[x][y] == 0;
    }
    
    public List<int[]> findPath(int startX, int startY, int endX, int endY) {
        Queue<Node> queue = new LinkedList<>();
        boolean[][] visited = new boolean[rows][cols];
        Node[][] nodeMap = new Node[rows][cols];
        
        Node startNode = new Node(startX, startY);
        queue.offer(startNode);
        visited[startX][startY] = true;
        nodeMap[startX][startY] = startNode;
        
        int[] dx = {-1, 1, 0, 0};
        int[] dy = {0, 0, -1, 1};
        
        while (!queue.isEmpty()) {
            Node current = queue.poll();
            
            if (current.x == endX && current.y == endY) {
                // 构建路径
                List<int[]> path = new ArrayList<>();
                Node node = current;
                while (node != null) {
                    path.add(new int[]{node.x, node.y});
                    node = node.parent;
                }
                Collections.reverse(path);
                return path;
            }
            
            for (int i = 0; i < 4; i++) {
                int newX = current.x + dx[i];
                int newY = current.y + dy[i];
                
                if (isValid(newX, newY) && !visited[newX][newY]) {
                    Node newNode = new Node(newX, newY);
                    newNode.parent = current;
                    queue.offer(newNode);
                    visited[newX][newY] = true;
                    nodeMap[newX][newY] = newNode;
                }
            }
        }
        
        return new ArrayList<>(); // 未找到路径
    }
})";

    // Dijkstra 算法 - Java
    exampleCodes["Dijkstra 算法-Java"] = R"(import java.util.*;

public class Dijkstra {
    private int[][] grid;
    private int rows, cols;
    private static final int INF = Integer.MAX_VALUE;
    
    static class Node implements Comparable<Node> {
        int x, y;
        int dist;
        Node parent;
        
        Node(int x, int y, int dist) {
            this.x = x;
            this.y = y;
            this.dist = dist;
            this.parent = null;
        }
        
        @Override
        public int compareTo(Node other) {
            return Integer.compare(this.dist, other.dist);
        }
    }
    
    public Dijkstra(int[][] grid) {
        this.grid = grid;
        this.rows = grid.length;
        this.cols = grid[0].length;
    }
    
    private boolean isValid(int x, int y) {
        return x >= 0 && x < rows && y >= 0 && y < cols && grid[x][y] == 0;
    }
    
    public List<int[]> findPath(int startX, int startY, int endX, int endY) {
        int[][] dist = new int[rows][cols];
        Node[][] nodeMap = new Node[rows][cols];
        PriorityQueue<Node> pq = new PriorityQueue<>();
        
        // 初始化距离
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                dist[i][j] = INF;
            }
        }
        
        dist[startX][startY] = 0;
        nodeMap[startX][startY] = new Node(startX, startY, 0);
        pq.offer(nodeMap[startX][startY]);
        
        int[] dx = {-1, 1, 0, 0};
        int[] dy = {0, 0, -1, 1};
        
        while (!pq.isEmpty()) {
            Node current = pq.poll();
            
            if (current.x == endX && current.y == endY) {
                // 构建路径
                List<int[]> path = new ArrayList<>();
                Node node = nodeMap[endX][endY];
                while (node != null) {
                    path.add(new int[]{node.x, node.y});
                    node = node.parent;
                }
                Collections.reverse(path);
                return path;
            }
            
            if (current.dist > dist[current.x][current.y]) {
                continue;
            }
            
            for (int i = 0; i < 4; i++) {
                int newX = current.x + dx[i];
                int newY = current.y + dy[i];
                
                if (isValid(newX, newY)) {
                    int newDist = dist[current.x][current.y] + 1;
                    
                    if (newDist < dist[newX][newY]) {
                        dist[newX][newY] = newDist;
                        if (nodeMap[newX][newY] == null) {
                            nodeMap[newX][newY] = new Node(newX, newY, newDist);
                        } else {
                            nodeMap[newX][newY].dist = newDist;
                        }
                        nodeMap[newX][newY].parent = nodeMap[current.x][current.y];
                        pq.offer(nodeMap[newX][newY]);
                    }
                }
            }
        }
        
        return new ArrayList<>(); // 未找到路径
    }
})";

    // DFS 算法 - C++
    exampleCodes["DFS 深度优先搜索-C++"] = R"(#include <iostream>
#include <vector>
#include <algorithm>

class DFS {
private:
    std::vector<std::vector<int>> grid;
    int rows, cols;
    
    bool isValid(int x, int y) {
        return x >= 0 && x < rows && y >= 0 && y < cols && grid[x][y] == 0;
    }
    
    bool dfsRecursive(int x, int y, int endX, int endY, 
                     std::vector<std::vector<bool>>& visited,
                     std::vector<std::pair<int, int>>& path) {
        if (x == endX && y == endY) {
            path.push_back({x, y});
            return true;
        }
        
        if (visited[x][y] || !isValid(x, y)) {
            return false;
        }
        
        visited[x][y] = true;
        path.push_back({x, y});
        
        // 尝试四个方向
        int dx[] = {-1, 1, 0, 0};
        int dy[] = {0, 0, -1, 1};
        
        for (int i = 0; i < 4; i++) {
            int newX = x + dx[i];
            int newY = y + dy[i];
            
            if (dfsRecursive(newX, newY, endX, endY, visited, path)) {
                return true;
            }
        }
        
        // 回溯
        path.pop_back();
        return false;
    }
    
public:
    DFS(std::vector<std::vector<int>>& grid) : grid(grid) {
        rows = grid.size();
        cols = grid[0].size();
    }
    
    std::vector<std::pair<int, int>> findPath(int startX, int startY, int endX, int endY) {
        std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
        std::vector<std::pair<int, int>> path;
        
        if (dfsRecursive(startX, startY, endX, endY, visited, path)) {
            return path;
        } else {
            return {}; // 未找到路径
        }
    }
};)";

    // DFS 算法 - Java
    exampleCodes["DFS 深度优先搜索-Java"] = R"(import java.util.*;

public class DFS {
    private int[][] grid;
    private int rows, cols;
    
    public DFS(int[][] grid) {
        this.grid = grid;
        this.rows = grid.length;
        this.cols = grid[0].length;
    }
    
    private boolean isValid(int x, int y) {
        return x >= 0 && x < rows && y >= 0 && y < cols && grid[x][y] == 0;
    }
    
    private boolean dfsRecursive(int x, int y, int endX, int endY,
                                boolean[][] visited, List<int[]> path) {
        if (x == endX && y == endY) {
            path.add(new int[]{x, y});
            return true;
        }
        
        if (visited[x][y] || !isValid(x, y)) {
            return false;
        }
        
        visited[x][y] = true;
        path.add(new int[]{x, y});
        
        // 尝试四个方向
        int[] dx = {-1, 1, 0, 0};
        int[] dy = {0, 0, -1, 1};
        
        for (int i = 0; i < 4; i++) {
            int newX = x + dx[i];
            int newY = y + dy[i];
            
            if (dfsRecursive(newX, newY, endX, endY, visited, path)) {
                return true;
            }
        }
        
        // 回溯
        path.remove(path.size() - 1);
        return false;
    }
    
    public List<int[]> findPath(int startX, int startY, int endX, int endY) {
        boolean[][] visited = new boolean[rows][cols];
        List<int[]> path = new ArrayList<>();
        
        if (dfsRecursive(startX, startY, endX, endY, visited, path)) {
            return path;
        } else {
            return new ArrayList<>(); // 未找到路径
        }
    }
})";

    // BFS 算法 - Python
    exampleCodes["BFS 广度优先搜索-Python"] = R"(from collections import deque
from typing import List, Tuple, Optional

class Node:
    def __init__(self, x: int, y: int):
        self.x = x
        self.y = y
        self.parent: Optional['Node'] = None

class BFS:
    def __init__(self, grid: List[List[int]]):
        self.grid = grid
        self.rows = len(grid)
        self.cols = len(grid[0])
    
    def is_valid(self, x: int, y: int) -> bool:
        return 0 <= x < self.rows and 0 <= y < self.cols and self.grid[x][y] == 0
    
    def find_path(self, start_x: int, start_y: int, end_x: int, end_y: int) -> List[Tuple[int, int]]:
        queue = deque()
        visited = [[False for _ in range(self.cols)] for _ in range(self.rows)]
        node_map = [[None for _ in range(self.cols)] for _ in range(self.rows)]
        
        start_node = Node(start_x, start_y)
        queue.append(start_node)
        visited[start_x][start_y] = True
        node_map[start_x][start_y] = start_node
        
        directions = [(-1, 0), (1, 0), (0, -1), (0, 1)]
        
        while queue:
            current = queue.popleft()
            
            if current.x == end_x and current.y == end_y:
                # 构建路径
                path = []
                node = current
                while node is not None:
                    path.append((node.x, node.y))
                    node = node.parent
                path.reverse()
                return path
            
            for dx, dy in directions:
                new_x, new_y = current.x + dx, current.y + dy
                
                if self.is_valid(new_x, new_y) and not visited[new_x][new_y]:
                    new_node = Node(new_x, new_y)
                    new_node.parent = current
                    queue.append(new_node)
                    visited[new_x][new_y] = True
                    node_map[new_x][new_y] = new_node
        
        return []  # 未找到路径)";

    // 添加更多算法的其他语言版本...
    exampleCodes["Dijkstra 算法-C++"] = R"(#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <algorithm>

const int INF = std::numeric_limits<int>::max();

struct Node {
    int x, y;
    int dist;
    Node* parent;
    
    Node(int x, int y, int dist) : x(x), y(y), dist(dist), parent(nullptr) {}
    
    bool operator>(const Node& other) const {
        return dist > other.dist;
    }
};

class Dijkstra {
private:
    std::vector<std::vector<int>> grid;
    int rows, cols;
    
    bool isValid(int x, int y) {
        return x >= 0 && x < rows && y >= 0 && y < cols && grid[x][y] == 0;
    }
    
public:
    Dijkstra(std::vector<std::vector<int>>& grid) : grid(grid) {
        rows = grid.size();
        cols = grid[0].size();
    }
    
    std::vector<std::pair<int, int>> findPath(int startX, int startY, int endX, int endY) {
        std::vector<std::vector<int>> dist(rows, std::vector<int>(cols, INF));
        std::vector<std::vector<Node*>> nodeMap(rows, std::vector<Node*>(cols, nullptr));
        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
        
        dist[startX][startY] = 0;
        nodeMap[startX][startY] = new Node(startX, startY, 0);
        pq.push(*nodeMap[startX][startY]);
        
        int dx[] = {-1, 1, 0, 0};
        int dy[] = {0, 0, -1, 1};
        
        while (!pq.empty()) {
            Node current = pq.top();
            pq.pop();
            
            if (current.x == endX && current.y == endY) {
                // 构建路径
                std::vector<std::pair<int, int>> path;
                Node* node = nodeMap[endX][endY];
                while (node != nullptr) {
                    path.push_back({node->x, node->y});
                    node = node->parent;
                }
                std::reverse(path.begin(), path.end());
                return path;
            }
            
            if (current.dist > dist[current.x][current.y]) {
                continue;
            }
            
            for (int i = 0; i < 4; i++) {
                int newX = current.x + dx[i];
                int newY = current.y + dy[i];
                
                if (isValid(newX, newY)) {
                    int newDist = dist[current.x][current.y] + 1;
                    
                    if (newDist < dist[newX][newY]) {
                        dist[newX][newY] = newDist;
                        if (nodeMap[newX][newY] == nullptr) {
                            nodeMap[newX][newY] = new Node(newX, newY, newDist);
                        } else {
                            nodeMap[newX][newY]->dist = newDist;
                        }
                        nodeMap[newX][newY]->parent = nodeMap[current.x][current.y];
                        pq.push(*nodeMap[newX][newY]);
                    }
                }
            }
        }
        
        return {}; // 未找到路径
    }
};)";

    // D* 算法 - C++
    exampleCodes["D* 算法-C++"] = R"(#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <algorithm>
#include <climits>

class DStar {
private:
    struct Node {
        int x, y;
        int g, h, k;
        Node* backpointer;
        bool inOpenList;
        bool inClosedList;
        
        Node(int x, int y) : x(x), y(y), g(INT_MAX), h(0), k(INT_MAX), 
                            backpointer(nullptr), inOpenList(false), inClosedList(false) {}
    };
    
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<Node*>> nodeMap;
    std::priority_queue<std::pair<int, Node*>, std::vector<std::pair<int, Node*>>, 
                       std::greater<std::pair<int, Node*>>> openList;
    int rows, cols;
    int goalX, goalY;
    
    bool isValid(int x, int y) {
        return x >= 0 && x < rows && y >= 0 && y < cols && grid[x][y] == 0;
    }
    
    int heuristic(int x1, int y1, int x2, int y2) {
        return std::abs(x1 - x2) + std::abs(y1 - y2);
    }
    
    void insert(Node* node, int h_new) {
        if (!node->inOpenList && !node->inClosedList) {
            node->k = h_new;
            node->h = h_new;
            node->inOpenList = true;
            openList.push({node->k, node});
        } else if (node->inOpenList) {
            node->k = std::min(node->k, h_new);
        } else {
            node->k = std::min(node->h, h_new);
            node->inOpenList = true;
            node->inClosedList = false;
            openList.push({node->k, node});
        }
    }
    
    int processState() {
        if (openList.empty()) return -1;
        
        Node* node = openList.top().second;
        int k_old = openList.top().first;
        openList.pop();
        
        node->inOpenList = false;
        node->inClosedList = true;
        
        if (k_old < node->h) {
            // Raise state
            std::vector<std::pair<int, int>> neighbors = {{-1,0}, {1,0}, {0,-1}, {0,1}};
            for (auto& dir : neighbors) {
                int nx = node->x + dir.first;
                int ny = node->y + dir.second;
                if (isValid(nx, ny)) {
                    Node* neighbor = nodeMap[nx][ny];
                    if (neighbor->h <= k_old && node->h > neighbor->h + 1) {
                        node->backpointer = neighbor;
                        node->h = neighbor->h + 1;
                    }
                }
            }
        }
        
        if (k_old == node->h) {
            // Lower state
            std::vector<std::pair<int, int>> neighbors = {{-1,0}, {1,0}, {0,-1}, {0,1}};
            for (auto& dir : neighbors) {
                int nx = node->x + dir.first;
                int ny = node->y + dir.second;
                if (isValid(nx, ny)) {
                    Node* neighbor = nodeMap[nx][ny];
                    if (!neighbor->inClosedList || neighbor->h > node->h + 1) {
                        neighbor->backpointer = node;
                        insert(neighbor, node->h + 1);
                    }
                }
            }
        } else {
            // Recompute
            std::vector<std::pair<int, int>> neighbors = {{-1,0}, {1,0}, {0,-1}, {0,1}};
            for (auto& dir : neighbors) {
                int nx = node->x + dir.first;
                int ny = node->y + dir.second;
                if (isValid(nx, ny)) {
                    Node* neighbor = nodeMap[nx][ny];
                    if (neighbor->backpointer == node && neighbor->h != node->h + 1) {
                        insert(neighbor, INT_MAX);
                    } else if (neighbor->backpointer != node && neighbor->h > node->h + 1) {
                        insert(node, node->h);
                    }
                }
            }
        }
        
        return openList.empty() ? -1 : openList.top().first;
    }
    
public:
    DStar(std::vector<std::vector<int>>& grid) : grid(grid) {
        rows = grid.size();
        cols = grid[0].size();
        
        nodeMap.resize(rows);
        for (int i = 0; i < rows; i++) {
            nodeMap[i].resize(cols);
            for (int j = 0; j < cols; j++) {
                nodeMap[i][j] = new Node(j, i);
            }
        }
    }
    
    std::vector<std::pair<int, int>> findPath(int startX, int startY, int endX, int endY) {
        goalX = endX;
        goalY = endY;
        
        // 初始化目标节点
        Node* goal = nodeMap[endY][endX];
        goal->h = 0;
        goal->g = 0;
        insert(goal, 0);
        
        Node* start = nodeMap[startY][startX];
        
        // 运行D*算法
        while (true) {
            int k_min = processState();
            if (k_min == -1 || start->inClosedList) break;
        }
        
        // 构建路径
        std::vector<std::pair<int, int>> path;
        Node* current = start;
        
        while (current != nullptr && current != goal) {
            path.push_back({current->x, current->y});
            current = current->backpointer;
        }
        
        if (current == goal) {
            path.push_back({goal->x, goal->y});
            return path;
        }
        
        return {}; // 未找到路径
    }
};)";

    // D* 算法 - Java
    exampleCodes["D* 算法-Java"] = R"(import java.util.*;

public class DStar {
    static class Node {
        int x, y;
        int g, h, k;
        Node backpointer;
        boolean inOpenList;
        boolean inClosedList;
        
        Node(int x, int y) {
            this.x = x;
            this.y = y;
            this.g = Integer.MAX_VALUE;
            this.h = 0;
            this.k = Integer.MAX_VALUE;
            this.backpointer = null;
            this.inOpenList = false;
            this.inClosedList = false;
        }
    }
    
    private int[][] grid;
    private Node[][] nodeMap;
    private PriorityQueue<Node> openList;
    private int rows, cols;
    private int goalX, goalY;
    
    public DStar(int[][] grid) {
        this.grid = grid;
        this.rows = grid.length;
        this.cols = grid[0].length;
        
        this.nodeMap = new Node[rows][cols];
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                nodeMap[i][j] = new Node(j, i);
            }
        }
        
        this.openList = new PriorityQueue<>((a, b) -> Integer.compare(a.k, b.k));
    }
    
    private boolean isValid(int x, int y) {
        return x >= 0 && x < cols && y >= 0 && y < rows && grid[y][x] == 0;
    }
    
    private int heuristic(int x1, int y1, int x2, int y2) {
        return Math.abs(x1 - x2) + Math.abs(y1 - y2);
    }
    
    private void insert(Node node, int h_new) {
        if (!node.inOpenList && !node.inClosedList) {
            node.k = h_new;
            node.h = h_new;
            node.inOpenList = true;
            openList.offer(node);
        } else if (node.inOpenList) {
            node.k = Math.min(node.k, h_new);
        } else {
            node.k = Math.min(node.h, h_new);
            node.inOpenList = true;
            node.inClosedList = false;
            openList.offer(node);
        }
    }
    
    private int processState() {
        if (openList.isEmpty()) return -1;
        
        Node node = openList.poll();
        int k_old = node.k;
        
        node.inOpenList = false;
        node.inClosedList = true;
        
        if (k_old < node.h) {
            // Raise state
            int[] dx = {-1, 1, 0, 0};
            int[] dy = {0, 0, -1, 1};
            
            for (int i = 0; i < 4; i++) {
                int nx = node.x + dx[i];
                int ny = node.y + dy[i];
                if (isValid(nx, ny)) {
                    Node neighbor = nodeMap[ny][nx];
                    if (neighbor.h <= k_old && node.h > neighbor.h + 1) {
                        node.backpointer = neighbor;
                        node.h = neighbor.h + 1;
                    }
                }
            }
        }
        
        if (k_old == node.h) {
            // Lower state
            int[] dx = {-1, 1, 0, 0};
            int[] dy = {0, 0, -1, 1};
            
            for (int i = 0; i < 4; i++) {
                int nx = node.x + dx[i];
                int ny = node.y + dy[i];
                if (isValid(nx, ny)) {
                    Node neighbor = nodeMap[ny][nx];
                    if (!neighbor.inClosedList || neighbor.h > node.h + 1) {
                        neighbor.backpointer = node;
                        insert(neighbor, node.h + 1);
                    }
                }
            }
        }
        
        return openList.isEmpty() ? -1 : openList.peek().k;
    }
    
    public List<int[]> findPath(int startX, int startY, int endX, int endY) {
        goalX = endX;
        goalY = endY;
        
        // 初始化目标节点
        Node goal = nodeMap[endY][endX];
        goal.h = 0;
        goal.g = 0;
        insert(goal, 0);
        
        Node start = nodeMap[startY][startX];
        
        // 运行D*算法
        while (true) {
            int k_min = processState();
            if (k_min == -1 || start.inClosedList) break;
        }
        
        // 构建路径
        List<int[]> path = new ArrayList<>();
        Node current = start;
        
        while (current != null && current != goal) {
            path.add(new int[]{current.x, current.y});
            current = current.backpointer;
        }
        
        if (current == goal) {
            path.add(new int[]{goal.x, goal.y});
            return path;
        }
        
        return new ArrayList<>(); // 未找到路径
    }
})";

    // D* 算法 - Python
    exampleCodes["D* 算法-Python"] = R"(import heapq
from typing import List, Tuple, Optional

class Node:
    def __init__(self, x: int, y: int):
        self.x = x
        self.y = y
        self.g = float('inf')
        self.h = 0
        self.k = float('inf')
        self.backpointer: Optional['Node'] = None
        self.in_open_list = False
        self.in_closed_list = False

class DStar:
    def __init__(self, grid: List[List[int]]):
        self.grid = grid
        self.rows = len(grid)
        self.cols = len(grid[0])
        
        self.node_map = []
        for i in range(self.rows):
            row = []
            for j in range(self.cols):
                row.append(Node(j, i))
            self.node_map.append(row)
        
        self.open_list = []
        self.goal_x = 0
        self.goal_y = 0
    
    def is_valid(self, x: int, y: int) -> bool:
        return 0 <= x < self.cols and 0 <= y < self.rows and self.grid[y][x] == 0
    
    def heuristic(self, x1: int, y1: int, x2: int, y2: int) -> int:
        return abs(x1 - x2) + abs(y1 - y2)
    
    def insert(self, node: Node, h_new: int):
        if not node.in_open_list and not node.in_closed_list:
            node.k = h_new
            node.h = h_new
            node.in_open_list = True
            heapq.heappush(self.open_list, (node.k, id(node), node))
        elif node.in_open_list:
            node.k = min(node.k, h_new)
        else:
            node.k = min(node.h, h_new)
            node.in_open_list = True
            node.in_closed_list = False
            heapq.heappush(self.open_list, (node.k, id(node), node))
    
    def process_state(self) -> int:
        if not self.open_list:
            return -1
        
        k_old, _, node = heapq.heappop(self.open_list)
        
        node.in_open_list = False
        node.in_closed_list = True
        
        if k_old < node.h:
            # Raise state
            directions = [(-1, 0), (1, 0), (0, -1), (0, 1)]
            for dx, dy in directions:
                nx, ny = node.x + dx, node.y + dy
                if self.is_valid(nx, ny):
                    neighbor = self.node_map[ny][nx]
                    if neighbor.h <= k_old and node.h > neighbor.h + 1:
                        node.backpointer = neighbor
                        node.h = neighbor.h + 1
        
        if k_old == node.h:
            # Lower state
            directions = [(-1, 0), (1, 0), (0, -1), (0, 1)]
            for dx, dy in directions:
                nx, ny = node.x + dx, node.y + dy
                if self.is_valid(nx, ny):
                    neighbor = self.node_map[ny][nx]
                    if not neighbor.in_closed_list or neighbor.h > node.h + 1:
                        neighbor.backpointer = node
                        self.insert(neighbor, node.h + 1)
        
        return self.open_list[0][0] if self.open_list else -1
    
    def find_path(self, start_x: int, start_y: int, end_x: int, end_y: int) -> List[Tuple[int, int]]:
        self.goal_x = end_x
        self.goal_y = end_y
        
        # 初始化目标节点
        goal = self.node_map[end_y][end_x]
        goal.h = 0
        goal.g = 0
        self.insert(goal, 0)
        
        start = self.node_map[start_y][start_x]
        
        # 运行D*算法
        while True:
            k_min = self.process_state()
            if k_min == -1 or start.in_closed_list:
                break
        
        # 构建路径
        path = []
        current = start
        
        while current is not None and current != goal:
            path.append((current.x, current.y))
            current = current.backpointer
        
        if current == goal:
            path.append((goal.x, goal.y))
            return path
        
        return []  # 未找到路径)";
}

void ExampleCodeDialog::onAlgorithmChanged()
{
    onLanguageChanged();
}

void ExampleCodeDialog::onLanguageChanged()
{
    QString algorithm = algorithmComboBox->currentText();
    QString language = languageComboBox->currentText();
    QString key = algorithm + "-" + language;
    
    if (exampleCodes.contains(key)) {
        codePreview->setPlainText(exampleCodes[key]);
    } else {
        codePreview->setPlainText(tr("该算法的%1版本尚未提供").arg(language));
    }
}

void ExampleCodeDialog::onCodeSelected()
{
    selectedCode = codePreview->toPlainText();
    accept();
}

QString ExampleCodeDialog::getSelectedAlgorithm() const
{
    return algorithmComboBox->currentText();
}

QString ExampleCodeDialog::getSelectedLanguage() const
{
    return languageComboBox->currentText();
} 