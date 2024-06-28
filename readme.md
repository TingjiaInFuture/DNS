
### 1. 程序总体结构

- **主程序 (main.c)**: 负责程序的初始化、加载配置文件、启动多线程服务器。
- **配置加载模块 (config.c)**: 负责加载和解析配置文件。
- **DNS 查询模块 (dns_query.c)**: 负责处理 DNS 查询，包括本地查询和中继查询。
- **缓存模块 (cache.c)**: 负责实现 DNS 查询结果的缓存，使用 LRU（最近最少使用）算法。
- **多线程管理模块 (thread_manager.c)**: 负责创建和管理处理客户端请求的线程。
- **日志模块 (log.c)**: 负责记录调试和运行日志。

### 2. 各子程序的功能

- **主程序 (main.c)**:
  - 初始化系统资源（如加载配置、初始化日志模块等）。
  - 启动多线程服务器，监听客户端的 DNS 请求。
  - 清理资源并退出。

- **配置加载模块 (config.c)**:
  - 读取并解析配置文件，获取 DNS 服务器地址、缓存大小等配置信息。

- **DNS 查询模块 (dns_query.c)**:
  - 处理来自客户端的 DNS 查询请求。
  - 在本地表中查找域名对应的 IP 地址。
  - 如果未找到，向上级 DNS 服务器发送查询请求。
  - 返回查询结果给客户端。

- **缓存模块 (cache.c)**:
  - 实现 LRU 缓存算法，存储最近的 DNS 查询结果。
  - 提供缓存查询、更新和删除的接口。

- **多线程管理模块 (thread_manager.c)**:
  - 创建线程池，管理线程的创建、销毁。
  - 分配客户端请求到不同的线程进行处理。

- **日志模块 (log.c)**:
  - 提供日志记录功能，包括调试信息和运行日志。

### 3. 多线程关系

- **主线程**: 负责初始化系统和启动监听线程。
- **监听线程**: 负责监听客户端请求，并将请求分配给工作线程。
- **工作线程**: 从线程池中获取，负责具体处理客户端的 DNS 请求，包括本地查询和中继查询。

### 4. 面向对象的设计思想

尽管 C 语言不是面向对象的语言，但可以通过模块化设计和抽象接口来模拟面向对象的编程思想：

- **抽象接口**: 每个模块提供一组操作接口，隐藏内部实现细节。例如，缓存模块提供 `cache_lookup`、`cache_insert` 等接口。
- **模块化设计**: 每个功能模块相对独立，模块之间通过接口进行交互，减少耦合度。
- **数据封装**: 使用 `struct` 来封装数据和相关操作。例如，DNS 查询模块可以定义 `struct DNSQuery`，封装查询信息和处理函数。

