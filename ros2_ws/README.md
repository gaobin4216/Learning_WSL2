# ROS2 工作空间说明文档

## 概述

本工作空间包含五个ROS2功能包，分别演示了ROS2的核心通信模式和参数系统：

| 包名 | 通信模式 | 特点 |
|------|---------|------|
| `my_pub_sub` | 话题（Topic） | 异步发布/订阅，一对多 |
| `my_custom_msg` | 自定义消息 | 定义自己的.msg类型 |
| `my_service` | 服务（Service） | 同步请求/响应，一对一 |
| `my_action` | 动作（Action） | 异步任务，有反馈和取消 |
| `my_parameter` | 参数（Parameter） | 动态配置节点参数 |

---

## 1. my_pub_sub - 发布/订阅模式

### 功能说明

演示ROS2最基本的通信方式：话题（Topic）。一个节点发布消息，另一个节点订阅消息，实现一对多的异步通信。

### 代码结构

```
my_pub_sub/
├── CMakeLists.txt
├── package.xml
├── src/
│   ├── talker.cpp        # 发布者节点
│   └── listener.cpp      # 订阅者节点
└── launch/
    └── talk_and_listen.launch.py
```

### 实现逻辑

#### Talker（发布者）

```cpp
class Talker : public rclcpp::Node
{
public:
    Talker() : Node("talker"), count_(0)
    {
        // 1. 创建发布者，话题名"chatter"，队列大小10
        publisher_ = this->create_publisher<std_msgs::msg::String>("chatter", 10);

        // 2. 创建定时器，每500ms触发一次
        timer_ = this->create_wall_timer(500ms, std::bind(&Talker::timer_callback, this));
    }

private:
    void timer_callback()
    {
        // 3. 创建消息并填充数据
        auto message = std_msgs::msg::String();
        message.data = "Hello, ROS2! Count: " + std::to_string(count_++);

        // 4. 发布消息
        publisher_->publish(message);
    }
};
```

#### Listener（订阅者）

```cpp
class Listener : public rclcpp::Node
{
public:
    Listener() : Node("listener")
    {
        // 1. 创建订阅者，话题名"chatter"，绑定回调函数
        subscription_ = this->create_subscription<std_msgs::msg::String>(
            "chatter", 10,
            std::bind(&Listener::topic_callback, this, std::placeholders::_1));
    }

private:
    // 2. 收到消息时自动调用
    void topic_callback(const std_msgs::msg::String::SharedPtr msg)
    {
        RCLCPP_INFO(this->get_logger(), "Received: '%s'", msg->data.c_str());
    }
};
```

### 运行逻辑

```
┌─────────┐                    ┌──────────┐
│  Talker  │  "chatter"话题    │ Listener │
│ (发布者) │ ────────────────→ │ (订阅者) │
└─────────┘   String消息       └──────────┘
     │                              │
     │  每500ms发布一次              │  收到即打印
     │  "Hello, ROS2! Count: X"    │
```

### 运行方法

```bash
# 方法1：使用launch文件（推荐）
ros2 launch my_pub_sub talk_and_listen.launch.py

# 方法2：分别运行（需要两个终端）
ros2 run my_pub_sub talker
ros2 run my_pub_sub listener
```

### 运行结果

```
[talker]: Publishing: 'Hello, ROS2! Count: 0'
[listener]: Received: 'Hello, ROS2! Count: 0'
[talker]: Publishing: 'Hello, ROS2! Count: 1'
[listener]: Received: 'Hello, ROS2! Count: 1'
...
```

---

## 2. my_custom_msg - 自定义消息类型

### 功能说明

演示如何定义和使用自定义的.msg消息类型。创建一个Person消息，包含姓名、年龄、身高字段。

### 代码结构

```
my_custom_msg/
├── CMakeLists.txt
├── package.xml
├── msg/
│   └── Person.msg                # 自定义消息定义
├── src/
│   ├── person_publisher.cpp      # 发布者
│   └── person_subscriber.cpp     # 订阅者
└── launch/
    └── person_demo.launch.py
```

### 消息定义

```msg
# Person.msg
string name      # 姓名
int32 age        # 年龄
float64 height   # 身高
```

### 消息生成流程

```
Person.msg  →  rosidl生成器  →  person.hpp  →  C++代码使用
(手写)         (编译时自动生成)    (自动)
```

生成的头文件位置：
- `build/my_custom_msg/rosidl_generator_cpp/my_custom_msg/msg/person.hpp`
- `install/my_custom_msg/include/my_custom_msg/my_custom_msg/msg/person.hpp`

### 实现逻辑

#### PersonPublisher（发布者）

```cpp
#include "my_custom_msg/msg/person.hpp"  // 包含自动生成的头文件

class PersonPublisher : public rclcpp::Node
{
public:
    PersonPublisher() : Node("person_publisher")
    {
        // 1. 创建发布者，使用自定义消息类型
        publisher_ = this->create_publisher<my_custom_msg::msg::Person>("person_topic", 10);
        timer_ = this->create_wall_timer(1000ms, std::bind(&PersonPublisher::timer_callback, this));
    }

private:
    void timer_callback()
    {
        // 2. 创建自定义消息对象
        auto message = my_custom_msg::msg::Person();

        // 3. 填充字段
        message.name = "Alice";
        message.age = 25;
        message.height = 1.65;

        // 4. 发布
        publisher_->publish(message);
    }
};
```

#### PersonSubscriber（订阅者）

```cpp
class PersonSubscriber : public rclcpp::Node
{
public:
    PersonSubscriber() : Node("person_subscriber")
    {
        // 1. 订阅自定义消息类型的话题
        subscription_ = this->create_subscription<my_custom_msg::msg::Person>(
            "person_topic", 10,
            std::bind(&PersonSubscriber::topic_callback, this, std::placeholders::_1));
    }

private:
    void topic_callback(const my_custom_msg::msg::Person::SharedPtr msg)
    {
        // 2. 访问自定义消息的字段
        RCLCPP_INFO(this->get_logger(), "name='%s', age=%d, height=%.2f",
                    msg->name.c_str(), msg->age, msg->height);
    }
};
```

### 命名规则

| 位置 | 命名风格 | 示例 |
|------|---------|------|
| .msg文件名 | 大写驼峰 | `Person.msg` |
| C++头文件 | 小写下划线 | `person.hpp` |
| C++类名 | 大写驼峰 | `Person` |

### 类型对照表

| .msg类型 | C++类型 | 示例 |
|---------|---------|------|
| `string` | `std::string` | `msg.name = "Alice"` |
| `int32` | `int32_t` | `msg.age = 25` |
| `float64` | `double` | `msg.height = 1.65` |

### 运行方法

```bash
ros2 launch my_custom_msg person_demo.launch.py
```

### 运行结果

```
[person_publisher]: Publishing: name='Alice', age=25, height=1.65
[person_subscriber]: Received: name='Alice', age=25, height=1.65
[person_publisher]: Publishing: name='Alice', age=26, height=1.65
[person_subscriber]: Received: name='Alice', age=26, height=1.65
...
```

---

## 3. my_service - 服务模式

### 功能说明

演示ROS2的服务（Service）通信模式。客户端发送请求，服务端处理后返回响应，实现同步的请求-响应通信。

### 代码结构

```
my_service/
├── CMakeLists.txt
├── package.xml
├── src/
│   ├── server.cpp    # 服务端
│   └── client.cpp    # 客户端
└── launch/
    └── service_demo.launch.py
```

### 服务定义（使用ROS2内置）

```srv
# AddTwoInts.srv（example_interfaces提供）
int64 a          # 请求字段：第一个数
int64 b          # 请求字段：第二个数
---              # 分隔符
int64 sum        # 响应字段：两数之和
```

### 实现逻辑

#### Server（服务端）

```cpp
using AddTwoInts = example_interfaces::srv::AddTwoInts;

class AddTwoIntsServer : public rclcpp::Node
{
public:
    AddTwoIntsServer() : Node("add_two_ints_server")
    {
        // 1. 创建服务，绑定回调函数
        service_ = this->create_service<AddTwoInts>(
            "add_two_ints",
            std::bind(&AddTwoIntsServer::handle_add, this, _1, _2));
        //                                       this  请求  响应
    }

private:
    // 2. 回调函数：处理请求，填充响应
    void handle_add(
        const std::shared_ptr<AddTwoInts::Request> request,
        std::shared_ptr<AddTwoInts::Response> response)
    {
        // 3. 读取请求，计算结果
        response->sum = request->a + request->b;
    }

    rclcpp::Service<AddTwoInts>::SharedPtr service_;
};
```

#### Client（客户端）

```cpp
int main(int argc, char *argv[])
{
    auto node = rclcpp::Node::make_shared("add_two_ints_client");

    // 1. 创建客户端
    auto client = node->create_client<AddTwoInts>("add_two_ints");

    // 2. 创建请求
    auto request = std::make_shared<AddTwoInts::Request>();
    request->a = atoll(argv[1]);  // 命令行参数
    request->b = atoll(argv[2]);

    // 3. 等待服务上线
    while (!client->wait_for_service(1s)) {
        RCLCPP_INFO(node->get_logger(), "Service not available, waiting...");
    }

    // 4. 发送请求并等待响应
    auto result = client->async_send_request(request);

    // 5. 阻塞等待结果
    if (rclcpp::spin_until_future_complete(node, result) ==
        rclcpp::FutureReturnCode::SUCCESS)
    {
        // 6. 获取响应
        RCLCPP_INFO(node->get_logger(), "Result: %ld", result.get()->sum);
    }
}
```

### 运行逻辑

```
Client                                    Server
  │                                         │
  │  1. create_client("add_two_ints")       │  1. create_service("add_two_ints")
  │                                         │
  │  2. wait_for_service()                  │
  │  ─────────────────────────────→         │
  │                                         │  2. 服务就绪
  │                                         │
  │  3. async_send_request({a:3, b:5})      │
  │  ═══════════════════════════════════→   │
  │                                         │  3. handle_add()被调用
  │                                         │     response->sum = 3 + 5 = 8
  │                                         │
  │  4. 收到响应 {sum:8}                    │
  │  ←══════════════════════════════════    │  4. 自动返回响应
  │                                         │
  │  5. result.get()->sum = 8               │
```

### 运行方法

```bash
# 方法1：使用launch文件启动服务端
ros2 launch my_service service_demo.launch.py

# 方法2：客户端调用（另一个终端）
ros2 run my_service client 3 5
```

### 运行结果

```
[server]: Service server ready
[server]: Incoming request: a=3, b=5
[server]: Sending back response: 8
[client]: Result: 3 + 5 = 8
```

---

## 4. my_action - 动作模式

### 功能说明

演示ROS2的动作（Action）通信模式。动作是服务的增强版，支持长时间任务执行、进度反馈和任务取消。使用斐波那契数列作为示例。

### 代码结构

```
my_action/
├── CMakeLists.txt
├── package.xml
├── src/
│   ├── server.cpp    # 动作服务端
│   └── client.cpp    # 动作客户端
└── launch/
    └── action_demo.launch.py
```

### 动作定义（使用ROS2内置）

```msg
# Fibonacci.action（example_interfaces提供）
# 请求（Goal）
int32 order        # 要计算的斐波那契数列长度
---
# 结果（Result）
int32[] sequence   # 最终的斐波那契数列
---
# 反馈（Feedback）
int32[] sequence   # 当前已计算的部分数列
```

### 实现逻辑

#### Server（动作服务端）

```cpp
using Fibonacci = example_interfaces::action::Fibonacci;
using GoalHandleFibonacci = rclcpp_action::ServerGoalHandle<Fibonacci>;

class FibonacciServer : public rclcpp::Node
{
public:
    FibonacciServer() : Node("fibonacci_server")
    {
        // 1. 创建动作服务器
        action_server_ = rclcpp_action::create_server<Fibonacci>(
            this, "fibonacci",
            std::bind(&FibonacciServer::handle_goal, this, _1, _2),
            std::bind(&FibonacciServer::handle_cancel, this, _1),
            std::bind(&FibonacciServer::handle_accepted, this, _1));
    }

private:
    // 2. 处理目标请求（是否接受）
    rclcpp_action::GoalResponse handle_goal(
        const rclcpp_action::GoalUUID & uuid,
        std::shared_ptr<const Fibonacci::Goal> goal)
    {
        if (goal->order > 50) {
            return rclcpp_action::GoalResponse::REJECT;  // 拒绝
        }
        return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;  // 接受并执行
    }

    // 3. 处理取消请求
    rclcpp_action::CancelResponse handle_cancel(
        const std::shared_ptr<GoalHandleFibonacci> goal_handle)
    {
        return rclcpp_action::CancelResponse::ACCEPT;  // 接受取消
    }

    // 4. 接受目标后，在新线程中执行
    void handle_accepted(const std::shared_ptr<GoalHandleFibonacci> goal_handle)
    {
        std::thread{std::bind(&FibonacciServer::execute, this, _1), goal_handle}.detach();
    }

    // 5. 执行任务
    void execute(const std::shared_ptr<GoalHandleFibonacci> goal_handle)
    {
        const auto goal = goal_handle->get_goal();
        auto feedback = std::make_shared<Fibonacci::Feedback>();
        auto result = std::make_shared<Fibonacci::Result>();

        // 计算斐波那契数列
        for (int i = 1; (i < goal->order) && rclcpp::ok(); ++i) {
            // 检查是否被取消
            if (goal_handle->is_canceling()) {
                goal_handle->canceled(result);
                return;
            }

            // 计算下一个数
            sequence.push_back(sequence[i] + sequence[i - 1]);

            // 发布反馈
            goal_handle->publish_feedback(feedback);
        }

        // 任务完成
        goal_handle->succeed(result);
    }
};
```

#### Client（动作客户端）

```cpp
class FibonacciClient : public rclcpp::Node
{
public:
    FibonacciClient() : Node("fibonacci_client")
    {
        // 1. 创建动作客户端
        client_ptr_ = rclcpp_action::create_client<Fibonacci>(this, "fibonacci");
    }

    void send_goal(int order)
    {
        // 2. 等待动作服务器
        if (!this->client_ptr_->wait_for_action_server(5s)) {
            return;
        }

        // 3. 创建目标
        auto goal_msg = Fibonacci::Goal();
        goal_msg.order = order;

        // 4. 设置回调函数
        auto send_goal_options = rclcpp_action::Client<Fibonacci>::SendGoalOptions();
        send_goal_options.goal_response_callback =
            std::bind(&FibonacciClient::goal_response_callback, this, _1);
        send_goal_options.feedback_callback =
            std::bind(&FibonacciClient::feedback_callback, this, _1, _2);
        send_goal_options.result_callback =
            std::bind(&FibonacciClient::result_callback, this, _1);

        // 5. 异步发送目标
        this->client_ptr_->async_send_goal(goal_msg, send_goal_options);
    }

private:
    // 目标响应回调
    void goal_response_callback(GoalHandleFibonacci::SharedPtr goal_handle)
    {
        if (!goal_handle) {
            RCLCPP_ERROR(this->get_logger(), "Goal was rejected");
        } else {
            RCLCPP_INFO(this->get_logger(), "Goal accepted");
        }
    }

    // 反馈回调
    void feedback_callback(
        GoalHandleFibonacci::SharedPtr,
        const std::shared_ptr<const Fibonacci::Feedback> feedback)
    {
        // 打印当前进度
        for (auto number : feedback->sequence) {
            std::cout << number << " ";
        }
        std::cout << std::endl;
    }

    // 结果回调
    void result_callback(const GoalHandleFibonacci::WrappedResult & result)
    {
        // 打印最终结果
        for (auto number : result.result->sequence) {
            std::cout << number << " ";
        }
    }
};
```

### 运行逻辑

```
Client                                        Server
  │                                             │
  │  1. create_client("fibonacci")              │  1. create_server("fibonacci")
  │                                             │
  │  2. wait_for_action_server()                │
  │  ─────────────────────────────→             │
  │                                             │
  │  3. async_send_goal({order:10})             │
  │  ═══════════════════════════════════════→   │
  │                                             │  2. handle_goal() → ACCEPT
  │                                             │  3. handle_accepted()
  │  4. goal_response_callback()                │  4. execute() 开始执行
  │     "Goal accepted"                         │
  │                                             │
  │  5. feedback_callback()                     │  5. publish_feedback()
  │     "0 1 1"                                 │     发送进度
  │  ←══════════════════════════════════════    │
  │                                             │
  │  6. feedback_callback()                     │  6. 继续计算
  │     "0 1 1 2"                               │
  │  ←══════════════════════════════════════    │
  │                                             │
  │     ... (多次反馈) ...                       │
  │                                             │
  │  7. result_callback()                       │  7. goal_handle->succeed()
  │     "0 1 1 2 3 5 8 13 21 34"               │
  │  ←══════════════════════════════════════    │
```

### 运行方法

```bash
# 方法1：使用launch文件启动动作服务器
ros2 launch my_action action_demo.launch.py

# 方法2：客户端发送目标（另一个终端）
ros2 run my_action client 10
```

### 运行结果

```
[server]: Action server ready
[client]: Sending goal
[server]: Executing goal
[server]: Publishing feedback
[client]: 0 1
[server]: Publishing feedback
[client]: 0 1 1
[server]: Publishing feedback
[client]: 0 1 1 2
...
[server]: Goal succeeded
[client]: Result: 0 1 1 2 3 5 8 13 21 34
```

---

## 四种通信模式对比

| 特性 | Topic | Custom Msg | Service | Action |
|------|-------|------------|---------|--------|
| 通信方式 | 异步 | 异步 | 同步 | 异步 |
| 关系 | 一对多 | 一对多 | 一对一 | 一对一 |
| 方向 | 单向 | 单向 | 双向 | 双向 |
| 反馈 | 无 | 无 | 无 | 有 |
| 取消 | 不支持 | 不支持 | 不支持 | 支持 |
| 适用场景 | 传感器数据、状态 | 自定义数据 | 查询、控制 | 长时间任务 |

---

## 工作空间管理

### 构建

```bash
cd ~/Learning_WSL2/ros2_ws
colcon build
```

### 构建单个包

```bash
colcon build --packages-select my_pub_sub
```

### 激活环境

```bash
source ~/Learning_WSL2/ros2_ws/install/setup.bash
```

### 查看话题

```bash
ros2 topic list
ros2 topic echo /chatter
```

### 查看服务

```bash
ros2 service list
ros2 service call /add_two_ints example_interfaces/srv/AddTwoInts "{a: 3, b: 5}"
```

### 查看动作

```bash
ros2 action list
ros2 action info /fibonacci
```

---

## 依赖关系

```
my_pub_sub
├── rclcpp
└── std_msgs

my_custom_msg
├── rclcpp
├── rosidl_default_generators
└── std_msgs

my_service
├── rclcpp
└── example_interfaces

my_action
├── rclcpp
├── rclcpp_action
└── example_interfaces

my_parameter
└── rclcpp
```

---

## 5. my_parameter - 参数系统

### 功能说明

演示ROS2的参数（Parameter）系统。参数允许在运行时动态配置节点的行为，无需修改代码。

### 代码结构

```
my_parameter/
├── CMakeLists.txt
├── package.xml
├── src/
│   └── parameter_node.cpp    # 参数节点
└── launch/
    └── parameter_demo.launch.py
```

### 实现逻辑

#### ParameterNode（参数节点）

```cpp
class ParameterNode : public rclcpp::Node
{
public:
    ParameterNode() : Node("parameter_node")
    {
        // 1. 声明参数（带默认值）
        this->declare_parameter("robot_name", "default_robot");
        this->declare_parameter("speed", 1.0);
        this->declare_parameter("max_retries", 3);
        this->declare_parameter("debug_mode", false);

        // 2. 获取参数值
        std::string robot_name = this->get_parameter("robot_name").as_string();
        double speed = this->get_parameter("speed").as_double();
        int max_retries = this->get_parameter("max_retries").as_int();
        bool debug_mode = this->get_parameter("debug_mode").as_bool();

        // 3. 注册参数变更回调
        param_callback_handle_ = this->add_on_set_parameters_callback(
            std::bind(&ParameterNode::parameter_callback, this, _1));
    }

private:
    // 参数变更回调函数
    rcl_interfaces::msg::SetParametersResult parameter_callback(
        const std::vector<rclcpp::Parameter> & parameters)
    {
        for (const auto & param : parameters) {
            RCLCPP_INFO(this->get_logger(), "Parameter '%s' changed to: %s",
                        param.get_name().c_str(), param.value_to_string().c_str());
        }
        auto result = rcl_interfaces::msg::SetParametersResult();
        result.successful = true;
        return result;
    }
};
```

### 参数类型对照

| 参数方法 | C++类型 | 示例 |
|---------|---------|------|
| `as_string()` | `std::string` | `param.as_string()` |
| `as_int()` | `int64_t` | `param.as_int()` |
| `as_double()` | `double` | `param.as_double()` |
| `as_bool()` | `bool` | `param.as_bool()` |
| `as_string_array()` | `std::vector<std::string>` | `param.as_string_array()` |
| `as_byte_array()` | `std::vector<uint8_t>` | `param.as_byte_array()` |

### 运行方法

```bash
# 方法1：使用默认参数
ros2 run my_parameter parameter_node

# 方法2：命令行传参
ros2 run my_parameter parameter_node --ros-args \
  -p robot_name:=my_robot \
  -p speed:=3.14 \
  -p max_retries:=5 \
  -p debug_mode:=true

# 方法3：使用launch文件
ros2 launch my_parameter parameter_demo.launch.py

# 方法4：launch文件传参
ros2 launch my_parameter parameter_demo.launch.py robot_name:=test_robot speed:=2.0
```

### 运行时动态修改参数

```bash
# 查看参数
ros2 param list
ros2 param get /parameter_node robot_name

# 设置参数
ros2 param set /parameter_node robot_name "new_robot"
ros2 param set /parameter_node speed 5.0
```

### 运行结果

```
[parameter_node]: === Parameter Node Started ===
[parameter_node]: robot_name: test_robot
[parameter_node]: speed: 3.14
[parameter_node]: max_retries: 3
[parameter_node]: debug_mode: false
[parameter_node]: Current: name=test_robot, speed=3.14

# 动态修改后
[parameter_node]: Parameter 'robot_name' changed to: new_robot
[parameter_node]: Current: name=new_robot, speed=3.14
```

### 参数配置文件（YAML）

可以使用YAML文件批量配置参数：

```yaml
# config/params.yaml
parameter_node:
  ros__parameters:
    robot_name: "yaml_robot"
    speed: 4.0
    max_retries: 10
    debug_mode: true
```

在launch文件中加载：

```python
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource

Node(
    package='my_parameter',
    executable='parameter_node',
    parameters=['config/params.yaml']
)
```

---

## 五种模式对比

| 特性 | Topic | Custom Msg | Service | Action | Parameter |
|------|-------|------------|---------|--------|-----------|
| 通信方式 | 异步 | 异步 | 同步 | 异步 | 同步 |
| 关系 | 一对多 | 一对多 | 一对一 | 一对一 | 一对一 |
| 方向 | 单向 | 单向 | 双向 | 双向 | 双向 |
| 反馈 | 无 | 无 | 无 | 有 | 无 |
| 取消 | 不支持 | 不支持 | 不支持 | 支持 | 不支持 |
| 适用场景 | 传感器数据 | 自定义数据 | 查询控制 | 长任务 | 配置参数 |
