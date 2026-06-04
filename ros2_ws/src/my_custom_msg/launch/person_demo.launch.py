from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(
            package='my_custom_msg',
            executable='person_publisher',
            name='person_publisher',
            output='screen'
        ),
        Node(
            package='my_custom_msg',
            executable='person_subscriber',
            name='person_subscriber',
            output='screen'
        ),
    ])
