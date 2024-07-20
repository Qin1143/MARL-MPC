import os
import pathlib
import launch
from launch_ros.actions import Node
from launch.actions import TimerAction
from launch import LaunchDescription
from ament_index_python.packages import get_package_share_directory
from webots_ros2_driver.webots_launcher import WebotsLauncher
# from webots_ros2_driver.webots_controller import WebotsController
from webots_ros2_driver.utils import controller_url_prefix


def generate_launch_description():
    package_dir = get_package_share_directory('multi_mir_package')
    robot_description = pathlib.Path(os.path.join(package_dir, 'resource', 'mir_robot_1.urdf')).read_text()
    config = os.getcwd() + '/src/mpc_controller/config/mpc_controller.yaml'

    webots = WebotsLauncher(
        world=os.path.join(package_dir, 'worlds', 'multi_robot_world.wbt')
    )

    mir_robot_1_driver = Node(
        package='webots_ros2_driver',
        executable='driver',
        output='screen',
        additional_env={'WEBOTS_CONTROLLER_URL': controller_url_prefix() + 'mir_robot_1'},
        parameters=[
            {'robot_description': robot_description},
        ]
    )

    mir_robot_2_driver = Node(
        package='webots_ros2_driver',
        executable='driver',
        output='screen',
        additional_env={'WEBOTS_CONTROLLER_URL': controller_url_prefix() + 'mir_robot_2'},
        parameters=[
            {'robot_description': robot_description},
        ]
    )

    mir_robot_3_driver = Node(
        package='webots_ros2_driver',
        executable='driver',
        output='screen',
        additional_env={'WEBOTS_CONTROLLER_URL': controller_url_prefix() + 'mir_robot_3'},
        parameters=[
            {'robot_description': robot_description},
        ]
    )

    # multi_mir_driver = WebotsController(
    #     robot_name='multi_robot_1',
    #     parameters=[{'robot_description' : robot_description}],
    #     respawn=True
    # )

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
        mir_robot_1_driver,
        mir_robot_2_driver,
        mir_robot_3_driver,
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