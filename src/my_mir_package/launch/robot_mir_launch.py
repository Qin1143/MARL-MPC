import os
import pathlib
import launch
from launch_ros.actions import Node
from launch.actions import TimerAction
from launch import LaunchDescription
from ament_index_python.packages import get_package_share_directory
from webots_ros2_driver.webots_launcher import WebotsLauncher
from webots_ros2_driver.utils import controller_url_prefix


def generate_launch_description():
    package_dir = get_package_share_directory('my_mir_package')
    robot_description = pathlib.Path(os.path.join(package_dir, 'resource', 'my_mir_robot.urdf')).read_text()
    config = os.getcwd() + '/src/mpc_controller/config/mpc_controller.yaml'

    webots = WebotsLauncher(
        world=os.path.join(package_dir, 'worlds', 'my_world.wbt')
    )

    my_mir_driver = Node(
        package='webots_ros2_driver',
        executable='driver',
        output='screen',
        additional_env={'WEBOTS_CONTROLLER_URL': controller_url_prefix() + 'my_mir_robot'},
        parameters=[
            {'robot_description': robot_description},
        ]
    )

    control_node = Node(
        package='mpc_controller',
        executable='control_node',
        output='screen',
        parameters=[config]
    )

    rviz2_node = Node(
        package='rviz2',
        executable='rviz2',
        output='screen',
        arguments=['-d', os.path.join(package_dir, 'config', 'mir100.rviz')]
    )

    traj_pub = TimerAction(
        period=3.0,
        actions=[
            Node(
                package='traj_publish',
                executable='traj_pub',
                output='screen'
            )
        ]
    )

    return LaunchDescription([
        webots,
        my_mir_driver,
        control_node,
        rviz2_node,
        traj_pub,
        launch.actions.RegisterEventHandler(
            event_handler=launch.event_handlers.OnProcessExit(
                target_action=webots,
                on_exit=[launch.actions.EmitEvent(event=launch.events.Shutdown())],
            )
        )
    ])