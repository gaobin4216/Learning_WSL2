#include <chrono>
#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "my_custom_msg/msg/person.hpp"

using namespace std::chrono_literals;

class PersonPublisher : public rclcpp::Node
{
public:
    PersonPublisher() : Node("person_publisher"), count_(0)
    {
        publisher_ = this->create_publisher<my_custom_msg::msg::Person>("person_topic", 10);
        timer_ = this->create_wall_timer(1000ms, std::bind(&PersonPublisher::timer_callback, this));
        RCLCPP_INFO(this->get_logger(), "Person Publisher started");
    }

private:
    void timer_callback()
    {
        auto message = my_custom_msg::msg::Person();
        message.name = "Alice";
        message.age = 25 + count_++;
        message.height = 1.65;
        RCLCPP_INFO(this->get_logger(), "Publishing: name='%s', age=%d, height=%.2f",
                    message.name.c_str(), message.age, message.height);
        publisher_->publish(message);
    }

    rclcpp::Publisher<my_custom_msg::msg::Person>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    size_t count_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<PersonPublisher>());
    rclcpp::shutdown();
    return 0;
}
