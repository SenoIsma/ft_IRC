#include "Server.hpp"
#include "lib.hpp"

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                         Constructors / Destructor                         |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

Server::Server(std::string port, std::string pass) : _socket_server(-1), _port(port), _pass(pass) {
	_init_commands();
	_init_mode_map();
	_create_socket();
}

Server::~Server() {
	std::cout << "[ DEBUG SERVER ] Destructor called" << std::endl;
	std::map<std::string, Channel*>::iterator it;
	for (it = _channels.begin(); it != _channels.end(); ++it) {
		delete it->second;
	}
}

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                              Socket & Epoll Init                           |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

void Server::_create_socket() {
	_socket_server = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_server == -1) {
		std::cerr << "Error creating socket" << std::endl;
		exit(1);
	}

	int opt = 1;
	setsockopt(_socket_server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(_port.c_str()));
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(_socket_server, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
		std::cerr << "Error binding socket" << std::endl;
		exit(1);
	}

	if (listen(_socket_server, 100) == -1) {
		std::cerr << "Error listening on socket" << std::endl;
		exit(1);
	}

	_epoll_fd = epoll_create1(0);
	if (_epoll_fd == -1) {
		perror("Failed to create Epoll Instance");
		close(_socket_server);
		exit(1);
	}

	epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = _socket_server;
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _socket_server, &ev) == -1) {
		perror("epoll_ctl: listen_socket");
		close(_socket_server);
		close(_epoll_fd);
		exit(1);
	}

	start_server();
}

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                                 Main Loop                                  |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

void Server::start_server() {
	const int MAX_EVENTS = 10;
	epoll_event events[MAX_EVENTS];

	while (true) {
		int nfds = epoll_wait(_epoll_fd, events, MAX_EVENTS, -1);
		if (nfds == -1) {
			std::cerr << "epoll_wait failed" << std::endl;
			break;
		}

		for (int i = 0; i < nfds; ++i) {
			int event_fd = events[i].data.fd;

			if (event_fd == _socket_server) {
				sockaddr_in client_addr;
				socklen_t addr_len = sizeof(client_addr);
				int client_fd = accept(_socket_server, (sockaddr*)&client_addr, &addr_len);

				if (client_fd < 0) {
					std::cerr << "Erreur accept" << std::endl;
					continue;
				}

				fcntl(client_fd, F_SETFL, O_NONBLOCK);
				_clients[client_fd] = Client(client_fd);
				std::cout << "Client " << client_fd << " connecté." << std::endl;

				epoll_event ev;
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = client_fd;
				epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
			} else {
				char buffer[1024];
				int bytes = recv(event_fd, buffer, sizeof(buffer) - 1, 0);

				if (bytes <= 0) {
					std::map<int, Client>::iterator it = _clients.find(event_fd);
					if (it != _clients.end())
						_handle_command(it->second, "QUIT :Connection lost\r\n");
					continue;
				} else {
					std::map<int, Client>::iterator it = _clients.find(event_fd);
					if (it == _clients.end())
						continue;
					Client& client = it->second;
					buffer[bytes] = '\0';
					client.appendToBuffer(buffer);

					if (client.getBuffer().size() > 1024) {
						std::cout << "[WARNING] Flood détecté. Déconnexion forcée." << std::endl;
						epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, event_fd, NULL);
						close(event_fd);
						_clients.erase(event_fd);
						continue;
					}

					std::string& buf = client.getBuffer();
					size_t pos;
					while ((pos = buf.find("\r\n")) != std::string::npos) {
						std::string line = buf.substr(0, pos);
						buf.erase(0, pos + 2);
						_handle_command(client, line);
						it = _clients.find(event_fd);
						if (it == _clients.end())
							break;
						client = it->second;
					}
				}
			}
		}
	}
}

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                                 Commandes                                 |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

void Server::_init_commands() {
    _command_map["PASS"] = &Server::handle_pass;
    _command_map["NICK"] = &Server::handle_nick;
    _command_map["USER"] = &Server::handle_user;
    _command_map["JOIN"] = &Server::handle_join;
    _command_map["PART"] = &Server::handle_part;
    _command_map["QUIT"] = &Server::handle_quit;
    _command_map["PRIVMSG"] = &Server::handle_privmsg;
    _command_map["TOPIC"] = &Server::handle_topic;
	_command_map["INVITE"] = &Server::handle_invite;
	_command_map["KICK"] = &Server::handle_kick;
	_command_map["NOTICE"] = &Server::handle_notice;
	_command_map["MODE"] = &Server::handle_mode;
	_command_map["PING"] = &Server::handle_ping;
	_command_map["CAP"] = &Server::handle_cap;
}

void Server::_handle_command(Client &client, const std::string &line) {
	if (client.hasNick() == true)
		std::cout << "[CMD] " << client.getNickname() << " : " << line << std::endl;
	else
    	std::cout << "[CMD] " << client.getFd() << " : " << line << std::endl;
    std::string cmd = line.substr(0, line.find(' '));
    std::map<std::string, CommandHandler>::iterator it = _command_map.find(cmd);

    if (it != _command_map.end())
        (this->*(it->second))(client, line);
    else
        std::cout << "[CMD] Commande inconnue" << std::endl;
}

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                                Utilitaires                                |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

int Server::get_socket() const { return _socket_server; }
int Server::get_port() const { return atoi(_port.c_str()); }
std::string Server::get_password() const { return _pass; }
