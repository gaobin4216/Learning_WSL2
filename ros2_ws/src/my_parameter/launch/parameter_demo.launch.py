from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration


def generate_launch_description():
    return LaunchDescription([
        # 声明启动参数
        DeclareLaunchArgument(
            'robot_name',
            default_value='my_robot',
            description='Name of the robot'
        ),
        DeclareLaunchArgument(
            'speed',
            default_value='2.5',
            description='Robot speed'
        ),

        # 启动节点，传递参数
        Node(
            package='my_parameter',
            executable='parameter_node',
            name='parameter_node',
            output='screen',
            parameters=[{
                'robot_name': LaunchConfiguration('robot_name'),
                'speed': LaunchConfiguration('speed'),
                'max_retries': 5,
                'debug_mode': True,
            }]
        ),
    ])
