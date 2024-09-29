from setuptools import setup

package_name = 'multi_mir_package'
data_files = []
data_files.append(('share/ament_index/resource_index/packages', ['resource/' + package_name]))
data_files.append(('share/' + package_name + '/launch', ['launch/multi_mir_launch.py']))
data_files.append(('share/' + package_name + '/worlds', ['worlds/multi_robot_world.wbt']))
data_files.append(('share/' + package_name + '/resource', ['resource/mir_robot_1.urdf']))
data_files.append(('share/' + package_name + '/resource', ['resource/mir_robot_2.urdf']))
data_files.append(('share/' + package_name + '/resource', ['resource/mir_robot_3.urdf']))
data_files.append(('share/' + package_name + '/resource', ['resource/mir_robot_4.urdf']))
data_files.append(('share/' + package_name, ['package.xml']))
data_files.append(('share/' + package_name + '/config', ['config/mir100.rviz']))

setup(
    name=package_name,
    version='0.0.0',
    packages=[package_name],
    data_files=data_files,
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='user',
    maintainer_email='user.name@mail.com',
    description='TODO: Package description',
    license='TODO: License declaration',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'mir_robot_1_driver = multi_mir_package.mir_robot_1_driver:main',
            'mir_robot_2_driver = multi_mir_package.mir_robot_2_driver:main',
            'mir_robot_3_driver = multi_mir_package.mir_robot_3_driver:main',
            'mir_robot_4_driver = multi_mir_package.mir_robot_4_driver:main'
        ],
    },
)