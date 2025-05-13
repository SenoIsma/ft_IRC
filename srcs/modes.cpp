#include "Server.hpp"
#include "lib.hpp"

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                                  MODE t                                   |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

void Server::mode_enable_t(Client& client, Channel* chan, const std::string&) {
	chan->setTopicRestricted(true);
	chan->broadcast(":" + client.getNickname() + " MODE " + chan->getName() + " +t\r\n");
}

void Server::mode_disable_t(Client& client, Channel* chan, const std::string&) {
	chan->setTopicRestricted(false);
	chan->broadcast(":" + client.getNickname() + " MODE " + chan->getName() + " -t\r\n");
}

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                                  MODE i                                   |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

void Server::mode_enable_i(Client& client, Channel* chan, const std::string&) {
	chan->setInviteOnly(true);
	chan->broadcast(":" + client.getNickname() + " MODE " + chan->getName() + " +i\r\n");
}

void Server::mode_disable_i(Client& client, Channel* chan, const std::string&) {
	chan->setInviteOnly(false);
	chan->broadcast(":" + client.getNickname() + " MODE " + chan->getName() + " -i\r\n");
}

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                                  MODE k                                   |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

void Server::mode_enable_k(Client& client, Channel* chan, const std::string& param) {
	if (param.empty()) return;
	chan->setPassword(param);
	chan->broadcast(":" + client.getNickname() + " MODE " + chan->getName() + " +k " + param + "\r\n");
}

void Server::mode_disable_k(Client& client, Channel* chan, const std::string&) {
	chan->removePassword();
	chan->broadcast(":" + client.getNickname() + " MODE " + chan->getName() + " -k\r\n");
}

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                                  MODE l                                   |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

void Server::mode_enable_l(Client& client, Channel* chan, const std::string& param) {
	if (param.empty()) return;
	int limit;
	std::istringstream(param) >> limit;
	if (limit <= 0) return;

	chan->setUserLimit(limit);

	std::stringstream ss;
	ss << limit;
	chan->broadcast(":" + client.getNickname() + " MODE " + chan->getName() + " +l " + ss.str() + "\r\n");
}

void Server::mode_disable_l(Client& client, Channel* chan, const std::string&) {
	chan->removeUserLimit();
	chan->broadcast(":" + client.getNickname() + " MODE " + chan->getName() + " -l\r\n");
}

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                                  MODE o                                   |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

void Server::mode_enable_o(Client& client, Channel* chan, const std::string& param) {
	Client* target = getClientByNick(param);
	if (!target || !chan->hasMember(target)) return;
	chan->addOperator(target);
	chan->broadcast(":" + client.getNickname() + " MODE " + chan->getName() + " +o " + param + "\r\n");
}

void Server::mode_disable_o(Client& client, Channel* chan, const std::string& param) {
	Client* target = getClientByNick(param);
	if (!target || !chan->hasMember(target)) return;
	chan->removeOperator(target);
	chan->broadcast(":" + client.getNickname() + " MODE " + chan->getName() + " -o " + param + "\r\n");
}

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                                Utilitaires                                |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

Client* Server::getClientByNick(const std::string& nickname) {
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if (it->second.getNickname() == nickname)
			return &it->second;
	}
	return NULL;
}
