from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(
            package='my_pub_sub',
            executable='talker',
            name='talker',
            output='screen'
        ),
        Node(
            package='my_pub_sub',
            executable='listener',
            name='listener',
            output='screen'
        ),
    ])
