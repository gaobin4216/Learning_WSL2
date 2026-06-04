#include <chrono>
#include <memory>
#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

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

        // 3. 打印参数值
        RCLCPP_INFO(this->get_logger(), "=== Parameter Node Started ===");
        RCLCPP_INFO(this->get_logger(), "robot_name: %s", robot_name.c_str());
        RCLCPP_INFO(this->get_logger(), "speed: %.2f", speed);
        RCLCPP_INFO(this->get_logger(), "max_retries: %d", max_retries);
        RCLCPP_INFO(this->get_logger(), "debug_mode: %s", debug_mode ? "true" : "false");

        // 4. 注册参数变更回调
        param_callback_handle_ = this->add_on_set_parameters_callback(
            std::bind(&ParameterNode::parameter_callback, this, std::placeholders::_1));

        // 5. 定时器打印当前参数
        timer_ = this->create_wall_timer(2000ms, std::bind(&ParameterNode::timer_callback, this));
    }

private:
    // 参数变更回调函数
    rcl_interfaces::msg::SetParametersResult parameter_callback(
        const std::vector<rclcpp::Parameter> & parameters)
    {
        auto result = rcl_interfaces::msg::SetParametersResult();
        result.successful = true;

        for (const auto & param : parameters) {
            RCLCPP_INFO(this->get_logger(), "Parameter '%s' changed to: %s",
                        param.get_name().c_str(), param.value_to_string().c_str());
        }

        return result;
    }

    // 定时器回调
    void timer_callback()
    {
        std::string robot_name = this->get_parameter("robot_name").as_string();
        double speed = this->get_parameter("speed").as_double();

        RCLCPP_INFO(this->get_logger(), "Current: name=%s, speed=%.2f",
                    robot_name.c_str(), speed);
    }

    rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr param_callback_handle_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ParameterNode>());
    rclcpp::shutdown();
    return 0;
}
