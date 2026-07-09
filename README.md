# Library Management System (LMS)

基于 C++17 的图书管理系统，使用 SQLite 数据库存储数据，Drogon 框架提供 REST API。

## 功能

| 功能 | 说明 |
|------|------|
| 📚 图书管理 | 添加、查询、搜索图书 |
| 📖 借阅管理 | 借书、还书（含逾期检测） |
| 💰 罚款管理 | 逾期罚款自动计算（¥0.5/天）、罚款支付 |
| ⏰ 逾期提醒 | 查询逾期记录，查看累计罚款 |
| 📋 借阅历史 | 按用户查询借阅记录 |

## 技术栈

- **后端语言**: C++17
- **前端框架**: Vue 3 + Vite
- **数据库**: SQLite3
- **Web 框架**: Drogon 1.9
- **构建工具**: CMake + vcpkg

## 环境要求

- [CMake](https://cmake.org/) ≥ 3.14
- [vcpkg](https://github.com/microsoft/vcpkg) 包管理器
- C++17 编译器（MSVC / GCC / Clang）

## 快速开始

### 1. 克隆项目

```bash
git clone https://github.com/xcjlms/lms.git
cd lms
```

### 2. 安装依赖（vcpkg）

```bash
# 安装 sqlite3 和 drogon
d:/work/vcpkg/vcpkg install sqlite3 drogon
```

### 3. 配置 & 编译

```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=D:/work/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

> 请将 `CMAKE_TOOLCHAIN_FILE` 路径改为你本机 vcpkg 的实际路径。
> 也可设置环境变量 `VCPKG_ROOT`，然后在 CMakeLists.txt 中自动读取。

### 4. 运行

```bash
./build/Debug/LibrarySystem.exe
```

服务器启动后监听 `http://0.0.0.0:8080`。

## 前端开发

### 环境要求

- [Node.js](https://nodejs.org/) ≥ 18

### 安装依赖

```bash
cd frontend
npm install
```

### 启动开发服务器（热更新）

```bash
cd frontend
npm run dev
```

浏览器打开 `http://localhost:3000`，修改代码后页面自动刷新。

Vite 已配置代理，前端发往 `/api/*` 的请求会自动转发到后端 `http://localhost:8080`。

> 注意：前端依赖后端 API，启动前端前请先启动后端服务器：
> ```bash
> ./build/Debug/LibrarySystem.exe
> ```

### 生产构建

```bash
cd frontend
npm run build
```

构建产物输出到 `frontend/dist/`，可部署到任意静态服务器。

## API 文档

### 图书接口

**查询图书列表**
```bash
curl http://localhost:8080/api/books
```

**搜索图书**
```bash
curl "http://localhost:8080/api/books?keyword=C++"
```

**添加图书**
```bash
curl -X POST http://localhost:8080/api/books \
  -H "Content-Type: application/json" \
  -d '{"title":"新书标题","author":"作者","isbn":"978-xxx","total_stock":3}'
```

### 借阅接口

**借书**
```bash
curl -X POST http://localhost:8080/api/borrow \
  -H "Content-Type: application/json" \
  -d '{"user_id":2,"book_id":1}'
```

**还书**
```bash
curl -X POST http://localhost:8080/api/return \
  -H "Content-Type: application/json" \
  -d '{"user_id":2,"book_id":1}'
```

**借阅历史**
```bash
curl "http://localhost:8080/api/records?user_id=2"
```

### 逾期接口

**查看逾期记录**
```bash
curl http://localhost:8080/api/overdue
```

### 罚款接口

**查询用户罚款**
```bash
curl "http://localhost:8080/api/fines?user_id=2"
```

**计算罚款**
```bash
curl -X POST http://localhost:8080/api/fines/calculate \
  -H "Content-Type: application/json" \
  -d '{"record_id":2}'
```

**支付罚款**
```bash
curl -X POST http://localhost:8080/api/fines/pay \
  -H "Content-Type: application/json" \
  -d '{"fine_id":1}'
```

## 测试数据

服务启动时自动创建以下测试数据：

| 用户 | 角色 |
|------|------|
| admin (ID=1) | 管理员 |
| alice (ID=2) | 读者 |
| bob (ID=3) | 读者 |

| 图书 | 库存 |
|------|------|
| C++ Primer Plus | 5 |
| Data Structures & Algorithm Analysis | 3 |
| Effective Modern C++ | 2 |

## 数据库

使用 SQLite 存储，文件为 `library.db`（程序启动时自动在运行目录创建）。

表结构：
- `users` — 用户账户
- `books` — 图书信息与库存
- `borrow_records` — 借阅记录
- `fine_records` — 罚款记录
- `feedback` — 读者反馈

## 项目结构

```
lms/
├── CMakeLists.txt                  # CMake 构建配置
├── vcpkg.json                      # vcpkg 依赖清单
├── main.cpp                        # 后端服务入口
├── LibrarySystem.h                 # 核心类声明
├── LibrarySystem.c                 # 核心业务逻辑
├── controllers/
│   ├── ApiController.h             # REST API 控制器
│   └── ApiController.cpp           # API 实现
├── frontend/                       # Vue 前端
│   ├── index.html                  # 入口 HTML
│   ├── vite.config.js              # Vite 配置（含 API 代理）
│   ├── package.json                # 前端依赖
│   └── src/
│       ├── main.js                 # Vue 启动入口
│       ├── App.vue                 # 根组件
│       └── components/
│           ├── Login.vue           # 登录页
│           ├── Home_admin.vue      # 管理员首页
│           ├── Home_reader.vue     # 读者首页
│           ├── Borrow.vue          # 借书
│           ├── Back.vue            # 还书
│           └── Fine.vue            # 罚款
├── README.md
```
