#include "RobotConnection.h"

#define NB_CONNECTION    5
#define MAPPING_SIZE    500

int RobotConnection::server_socket = -1;
modbus_t* RobotConnection::ctx = NULL;
modbus_mapping_t* RobotConnection::mb_mapping = NULL;

void RobotConnection::close_sigint(int dummy)
{
    if (server_socket != -1) {
        shutdown(server_socket,1);
    }
    modbus_free(ctx);
    modbus_mapping_free(mb_mapping);

    exit(dummy);
}

RobotConnection::RobotConnection(const char* ip_addr, int port) : ip_addr(ip_addr), port(port), s(0), kill_thread(false), is_connected(false) {
    
}

bool RobotConnection::read_bool(const int address)
{
    if (!validate_address(address)) {
        std::cout << "ERROR: Address out of range" << std::endl;
        return false;
    }
    return mb_mapping->tab_bits[address];
}

void RobotConnection::write_bool(int address, bool value)
{
    if (!validate_address(address)) {
        std::cout << "ERROR: Address out of range" << std::endl;
        return;
    }

    mb_mapping->tab_input_bits[address] = value;
}

int RobotConnection::read_register(int address)
{
    if (!validate_address(address)) {
        std::cout << "ERROR: Address out of range" << std::endl;
        return false;
    }
    return mb_mapping->tab_registers[address];
}

void RobotConnection::write_register(int address, int value)
{
    if (!validate_address(address)) {
        std::cout << "ERROR: Address out of range" << std::endl;
        return;
    }

    mb_mapping->tab_input_registers[address] = value;
}

bool RobotConnection::IsConnected()
{
    return is_connected;
}

void RobotConnection::start_server() {
    if (!is_connected)
    {
        printf("booting server\n");
        th = std::thread(&RobotConnection::boot_server, this);
    }
}

void RobotConnection ::boot_server() {
    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
    int master_socket;
    int rc;
    fd_set refset;
    fd_set rdset;
    /* Maximum file descriptor number */
    int fdmax;

    ctx = modbus_new_tcp(ip_addr, port);
    //modbus_set_debug(ctx, TRUE);
    mb_mapping = modbus_mapping_new(MAPPING_SIZE, MAPPING_SIZE,
        MAPPING_SIZE, MAPPING_SIZE);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n",
            modbus_strerror(errno));
        modbus_free(ctx);
        return;
    }

    server_socket = modbus_tcp_listen(ctx, NB_CONNECTION);
    if (server_socket == -1) {
        fprintf(stderr, "Unable to listen TCP connection\n");
        modbus_free(ctx);
        return;
    }

    /* Clear the reference set of socket */
    FD_ZERO(&refset);
    /* Add the server socket */
    FD_SET(server_socket, &refset);

    /* Keep track of the max file descriptor */
    fdmax = server_socket;

    is_connected = true;

    for (;;) {
        if (kill_thread) {
            std::cout << "breaking thread" << std::endl;
            return;
        }
        rdset = refset;
        if (select(fdmax + 1, &rdset, NULL, NULL, NULL) == -1) {
            perror("Server select() failure.");
            close_sigint(1);
        }

        /* Run through the existing connections looking for data to be
         * read */
        for (master_socket = 0; master_socket <= fdmax; master_socket++) {

            if (!FD_ISSET(master_socket, &rdset)) {
                continue;
            }

            if (master_socket == server_socket) {
                /* A client is asking a new connection */
                socklen_t addrlen;
                struct sockaddr_in clientaddr;
                int newfd;

                /* Handle new connections */
                addrlen = sizeof(clientaddr);
                memset(&clientaddr, 0, sizeof(clientaddr));
                newfd = accept(server_socket, (struct sockaddr*)&clientaddr, &addrlen);
                if (newfd == -1) {
                    perror("Server accept() error");
                }
                else {
                    FD_SET(newfd, &refset);

                    if (newfd > fdmax) {
                        /* Keep track of the maximum */
                        fdmax = newfd;
                    }
                    printf("New connection from %s:%d on socket %d\n",
                        inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port, newfd);
                }
            }
            else {
                modbus_set_socket(ctx, master_socket);
                rc = modbus_receive(ctx, query);
                if (rc > 0) {
                    modbus_reply(ctx, query, rc, mb_mapping);
                }
                else if (rc == -1) {
                    /* This server in ended on connection closing or
                     * any errors. */
                    printf("Connection closed on socket %d\n", master_socket);
                    //close(master_socket);
                    shutdown(server_socket, 1);

                    ///* Remove from reference set */
                    FD_CLR(master_socket, &refset);

                    if (master_socket == fdmax) {
                        fdmax--;
                    }
                }
            }
        }
    }
}

bool RobotConnection::validate_address(const int address)
{
    if (address > MAPPING_SIZE)
        return false;
    if (address < 0)
        return false;
    return true;
}

RobotConnection::~RobotConnection()
{
    kill_thread = true;
    th.join();
    printf("Quit the loop: %s\n", modbus_strerror(errno));

    close_sigint(1);
    
}

