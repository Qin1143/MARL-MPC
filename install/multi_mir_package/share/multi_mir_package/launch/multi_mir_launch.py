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
    robot_description_1 = pathlib.Path(os.path.join(package_dir, 'resource', 'mir_robot_1.urdf')).read_text()
    robot_description_2 = pathlib.Path(os.path.join(package_dir, 'resource', 'mir_robot_2.urdf')).read_text()
    robot_description_3 = pathlib.Path(os.path.join(package_dir, 'resource', 'mir_robot_3.urdf')).read_text()
    # config = os.getcwd() + '/src/mpc_controller/config/mpc_controller.yaml'
    robot_num_value = 3

    webots = WebotsLauncher(
        world=os.path.join(package_dir, 'worlds', 'multi_robot_world.wbt')
    )

    mir_robot_1_driver = Node(
        package='webots_ros2_driver',
        executable='driver',
        output='screen',
        additional_env={'WEBOTS_CONTROLLER_URL': controller_url_prefix() + 'mir_robot_1'},
        parameters=[
            {'robot_description': robot_description_1},
        ]
    )

    mir_robot_2_driver = Node(
        package='webots_ros2_driver',
        executable='driver',
        output='screen',
        additional_env={'WEBOTS_CONTROLLER_URL': controller_url_prefix() + 'mir_robot_2'},
        parameters=[
            {'robot_description': robot_description_2},
        ]
    )

    mir_robot_3_driver = Node(
        package='webots_ros2_driver',
        executable='driver',
        output='screen',
        additional_env={'WEBOTS_CONTROLLER_URL': controller_url_prefix() + 'mir_robot_3'},
        parameters=[
            {'robot_description': robot_description_3},
        ]
    )

    control_node = Node(
        package='multi_mpc_controller',
        executable='control_node',
        output='screen',
        parameters=[{'robot_num': robot_num_value}]
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
                package='multi_traj_publish',
                executable='traj_pub',
                output='screen',
                parameters=[{'robot_num': robot_num_value}]
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