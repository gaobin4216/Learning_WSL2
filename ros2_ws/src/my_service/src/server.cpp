#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/srv/add_two_ints.hpp"

#include <memory>

using AddTwoInts = example_interfaces::srv::AddTwoInts;
using std::placeholders::_1;
using std::placeholders::_2;

class AddTwoIntsServer : public rclcpp::Node
{
public:
    AddTwoIntsServer() : Node("add_two_ints_server")
    {
        service_ = this->create_service<AddTwoInts>(
            "add_two_ints",
            std::bind(&AddTwoIntsServer::handle_add, this, _1, _2));
        RCLCPP_INFO(this->get_logger(), "Service server ready");
    }

private:
    void handle_add(
        const std::shared_ptr<AddTwoInts::Request> request,
        std::shared_ptr<AddTwoInts::Response> response)
    {
        response->sum = request->a + request->b;
        RCLCPP_INFO(this->get_logger(), "Incoming request: a=%ld, b=%ld",
                    request->a, request->b);
        RCLCPP_INFO(this->get_logger(), "Sending back response: %ld",
                    response->sum);
    }

    rclcpp::Service<AddTwoInts>::SharedPtr service_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<AddTwoIntsServer>());
    rclcpp::shutdown();
    return 0;
}
