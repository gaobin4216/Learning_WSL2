from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(
            package='my_action',
            executable='server',
            name='fibonacci_server',
            output='screen'
        ),
    ])
