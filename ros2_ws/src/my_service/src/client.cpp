#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/srv/add_two_ints.hpp"

#include <chrono>
#include <cstdlib>
#include <memory>

using namespace std::chrono_literals;
using AddTwoInts = example_interfaces::srv::AddTwoInts;

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);

    if (argc != 3) {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Usage: client X Y");
        return 1;
    }

    auto node = rclcpp::Node::make_shared("add_two_ints_client");
    auto client = node->create_client<AddTwoInts>("add_two_ints");

    auto request = std::make_shared<AddTwoInts::Request>();
    request->a = atoll(argv[1]);
    request->b = atoll(argv[2]);

    while (!client->wait_for_service(1s)) {
        if (!rclcpp::ok()) {
            RCLCPP_ERROR(node->get_logger(), "Interrupted while waiting");
            return 1;
        }
        RCLCPP_INFO(node->get_logger(), "Service not available, waiting...");
    }

    auto result = client->async_send_request(request);

    if (rclcpp::spin_until_future_complete(node, result) ==
        rclcpp::FutureReturnCode::SUCCESS)
    {
        RCLCPP_INFO(node->get_logger(), "Result: %ld + %ld = %ld",
                    request->a, request->b, result.get()->sum);
    } else {
        RCLCPP_ERROR(node->get_logger(), "Failed to call service");
    }

    rclcpp::shutdown();
    return 0;
}
