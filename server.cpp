/*
 * Copyright © 2008-2014 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "RobotConnection.h"
#include <string>


int main(int argc, char* argv[])
{
    // Husk at åbne for firewall
    RobotConnection robot(NULL, 502);
    robot.start_server();

    std::string x;
    while (true) {
        std::cin >> x;
        if (x == "exit") {
            break;
        }

        if (x == "off") {
            robot.write_bool(500, false);
            robot.read_bool(0);
            robot.write_register(0, 1234);
            robot.read_register(0);


        }

        if (x == "on") {
            robot.write_bool(0, true);
        }
    }

    return 0;
}