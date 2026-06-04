from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(
            package='my_service',
            executable='server',
            name='add_two_ints_server',
            output='screen'
        ),
    ])
