#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <map>
#include <set>
#include <vector>
#include <iostream>
#include <sys/epoll.h>

#include "lib.hpp"
#include "Client.hpp"
#include "Channel.hpp"

class Server {
private:
    int _socket_server;
    int _epoll_fd;
    std::string _port;
    std::string _pass;

    std::map<int, Client> _clients;
    std::map<std::string, Channel*> _channels;

    typedef void (Server::*CommandHandler)(Client&, const std::string&);
    std::map<std::string, CommandHandler> _command_map;
    typedef void (Server::*ModeHandler)(Client&, Channel*, const std::string&);
    std::map<char, std::map<char, ModeHandler> > _mode_map;

    void _init_mode_map();
    void _init_commands();
    void _create_socket();
    void _handle_command(Client &client, const std::string &line);
    void checkFullAuth(Client& client);

public:
    Server(std::string port, std::string pass);
    ~Server();

    void start_server();

    void handle_pass(Client& client, const std::string& line);
    void handle_nick(Client& client, const std::string& line);
    void handle_user(Client& client, const std::string& line);
    void handle_join(Client& client, const std::string& line);
    void handle_part(Client& client, const std::string& line);
    void handle_quit(Client& client, const std::string& line);
    void handle_privmsg(Client& client, const std::string& line);
    void handle_topic(Client& client, const std::string& line);
    void handle_invite(Client& client, const std::string& line);
    void handle_kick(Client& client, const std::string& line);
    void handle_notice(Client& client, const std::string& line);
    void handle_ping(Client& client, const std::string& line);
    void handle_cap(Client& client, const std::string& line);

    void handle_mode(Client& client, const std::string& line);
    // t
    void mode_enable_t(Client& client, Channel* chan, const std::string&);
    void mode_disable_t(Client& client, Channel* chan, const std::string&);
    // i
    void mode_enable_i(Client& client, Channel* chan, const std::string&);
    void mode_disable_i(Client& client, Channel* chan, const std::string&);
    // k
    void mode_enable_k(Client& client, Channel* chan, const std::string& param);
    void mode_disable_k(Client& client, Channel* chan, const std::string&);
    // l
    void mode_enable_l(Client& client, Channel* chan, const std::string& param);
    void mode_disable_l(Client& client, Channel* chan, const std::string&);
    // o
    void mode_enable_o(Client& client, Channel* chan, const std::string& param);
    void mode_disable_o(Client& client, Channel* chan, const std::string&);
    // utilitaire
    Client* getClientByNick(const std::string& nickname);


    void leaveAllChannels(Client& client);
    void disconnectClient(int fd);
    bool nicknameExists(const std::string& nickname) const;

    int get_socket() const;
    int get_port() const;
    std::string get_password() const;
};

#endif
