# CMakeList.txt : CMake project for modbus server2, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("modbus server2")

find_package(libmodbus CONFIG REQUIRED)
    

# Add source to this project's executable.
add_executable (CMakeTarget "server.cpp"  "RobotConnection.h" "RobotConnection.cpp")

target_link_libraries(CMakeTarget PRIVATE modbus)
