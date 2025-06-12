# Qt项目

这是一个使用Qt6.5.3和CMake构建的项目。

## 项目结构
```
GridMapEditor/
├── README.md                       # 项目说明文档
├── log.md                          # 开发日志
├── CMakeLists.txt                  # CMake构建配置文件
├── resources.qrc                   # Qt资源文件
├── app.rc                          # 应用程序资源配置
├── compile_commands.json           # 编译命令数据库
├── .gitignore                      # Git忽略文件配置
├── .vscode/                        # VS Code配置目录
│   └── c_cpp_properties.json       # C/C++扩展配置
├── src/                            # 源代码目录
│   ├── main.cpp                    # 程序入口文件
│   ├── mainwindow.cpp              # 主窗口实现
│   ├── grideditor.cpp              # 网格编辑器实现
│   ├── gridcreatedialog.cpp        # 网格创建对话框
│   ├── randomobstacledialog.cpp    # 随机障碍物对话框
│   ├── pathfindingexecutor.cpp     # 路径查找执行器
│   ├── examplecodedialog.cpp       # 示例代码对话框
│   ├── codeeditor.cpp              # 代码编辑器
│   └── codehighlighter.cpp         # 代码高亮器
├── include/                        # 头文件目录
│   ├── mainwindow.h                # 主窗口头文件
│   ├── grideditor.h                # 网格编辑器头文件
│   ├── gridcreatedialog.h          # 网格创建对话框头文件
│   ├── randomobstacledialog.h      # 随机障碍物对话框头文件
│   ├── pathfindingexecutor.h       # 路径查找执行器头文件
│   ├── examplecodedialog.h         # 示例代码对话框头文件
│   └── codehighlighter.h           # 代码高亮器头文件
├── map/                            # 地图文件目录
│   ├── new_map1.json               # 示例地图文件1
│   ├── new_map2.json               # 示例地图文件2
│   ├── new_map3.json               # 示例地图文件3
│   └── new_map4.json               # 示例地图文件4
├── pic/                            # 图片资源目录
│   ├── app_icon.ico                # 应用程序图标
│   ├── car.png                     # 小车图标
│   ├── car_y.png                   # 小车图标(黄色)
│   ├── flag.png                    # 旗帜图标
│   ├── flag_r.png                  # 旗帜图标(红色)
│   └── flag_y.png                  # 旗帜图标(黄色)
└── build/                          # 构建目录
    ├── GridMapEditor.exe           # 编译生成的可执行文件
    ├── CMakeFiles/                 # CMake生成的文件
    ├── GridMapEditor_autogen/      # Qt MOC自动生成的文件
    ├── platforms/                  # Qt平台插件
    ├── .qt/                        # Qt构建缓存
    ├── *.dll                       # 运行时依赖库
    └── 其他构建文件...
```

## 环境要求

- Qt 6.5.3
- CMake 3.16或更高版本
- MinGW-w64 编译器

## 环境配置

1. 安装Qt 6.5.3和MinGW64

2. 配置环境变量
   - 将Qt和MinGW的bin目录添加到系统PATH中：
     - .\Qt\6.5.3\mingw_64\bin
     - .\mingw64\bin

## 构建项目

1. 构建构建目录：
```bash
mkdir build
cd build
```

2. 配置CMake：
```bash
cmake ..
```

3. 构建项目：
```bash
cmake --build .
``` 

4. 运行程序：
```bash
./GridMapEditor.exe
```

# Q&A
1. 出现QT依赖报错
```
将.\Qt\6.5.3\mingw_64\bin目录下的Qt6Core.dll、Qt6Gui.dll、Qt6Widgets.dll依赖复制到build目录下
```