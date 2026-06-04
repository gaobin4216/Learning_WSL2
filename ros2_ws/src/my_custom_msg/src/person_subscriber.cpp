#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "my_custom_msg/msg/person.hpp"

class PersonSubscriber : public rclcpp::Node
{
public:
    PersonSubscriber() : Node("person_subscriber")
    {
        subscription_ = this->create_subscription<my_custom_msg::msg::Person>(
            "person_topic", 10,
            std::bind(&PersonSubscriber::topic_callback, this, std::placeholders::_1));
        RCLCPP_INFO(this->get_logger(), "Person Subscriber started");
    }

private:
    void topic_callback(const my_custom_msg::msg::Person::SharedPtr msg)
    {
        RCLCPP_INFO(this->get_logger(), "Received: name='%s', age=%d, height=%.2f",
                    msg->name.c_str(), msg->age, msg->height);
    }

    rclcpp::Subscription<my_custom_msg::msg::Person>::SharedPtr subscription_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<PersonSubscriber>());
    rclcpp::shutdown();
    return 0;
}
