#include <functional>
#include <future>
#include <memory>
#include <string>
#include <sstream>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "example_interfaces/action/fibonacci.hpp"

using Fibonacci = example_interfaces::action::Fibonacci;
using GoalHandleFibonacci = rclcpp_action::ClientGoalHandle<Fibonacci>;

using namespace std::placeholders;

class FibonacciClient : public rclcpp::Node
{
public:
    FibonacciClient() : Node("fibonacci_client")
    {
        this->client_ptr_ = rclcpp_action::create_client<Fibonacci>(
            this, "fibonacci");
    }

    void send_goal(int order)
    {
        using namespace std::chrono_literals;

        if (!this->client_ptr_->wait_for_action_server(5s)) {
            RCLCPP_ERROR(this->get_logger(), "Action server not available");
            return;
        }

        auto goal_msg = Fibonacci::Goal();
        goal_msg.order = order;

        RCLCPP_INFO(this->get_logger(), "Sending goal");

        auto send_goal_options = rclcpp_action::Client<Fibonacci>::SendGoalOptions();
        send_goal_options.goal_response_callback =
            std::bind(&FibonacciClient::goal_response_callback, this, _1);
        send_goal_options.feedback_callback =
            std::bind(&FibonacciClient::feedback_callback, this, _1, _2);
        send_goal_options.result_callback =
            std::bind(&FibonacciClient::result_callback, this, _1);

        this->client_ptr_->async_send_goal(goal_msg, send_goal_options);
    }

private:
    rclcpp_action::Client<Fibonacci>::SharedPtr client_ptr_;

    void goal_response_callback(GoalHandleFibonacci::SharedPtr goal_handle)
    {
        if (!goal_handle) {
            RCLCPP_ERROR(this->get_logger(), "Goal was rejected");
        } else {
            RCLCPP_INFO(this->get_logger(), "Goal accepted, waiting for result");
        }
    }

    void feedback_callback(
        GoalHandleFibonacci::SharedPtr,
        const std::shared_ptr<const Fibonacci::Feedback> feedback)
    {
        std::ostringstream ss;
        ss << "Next number in sequence received: ";
        for (auto number : feedback->sequence) {
            ss << number << " ";
        }
        RCLCPP_INFO(this->get_logger(), "%s", ss.str().c_str());
    }

    void result_callback(const GoalHandleFibonacci::WrappedResult & result)
    {
        switch (result.code) {
            case rclcpp_action::ResultCode::SUCCEEDED:
                break;
            case rclcpp_action::ResultCode::ABORTED:
                RCLCPP_ERROR(this->get_logger(), "Goal was aborted");
                return;
            case rclcpp_action::ResultCode::CANCELED:
                RCLCPP_ERROR(this->get_logger(), "Goal was canceled");
                return;
            default:
                RCLCPP_ERROR(this->get_logger(), "Unknown result code");
                return;
        }

        std::ostringstream ss;
        ss << "Result received: ";
        for (auto number : result.result->sequence) {
            ss << number << " ";
        }
        RCLCPP_INFO(this->get_logger(), "%s", ss.str().c_str());

        rclcpp::shutdown();
    }
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);

    if (argc != 2) {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Usage: client ORDER");
        return 1;
    }

    auto action_client = std::make_shared<FibonacciClient>();
    action_client->send_goal(atoi(argv[1]));

    rclcpp::spin(action_client);
    rclcpp::shutdown();
    return 0;
}
