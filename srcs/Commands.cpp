#include "Server.hpp"
#include "lib.hpp"

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                             Authentification                              |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

void Server::handle_pass(Client& client, const std::string& line) {
	if (client.hasAuth()) return;
	if (line == "PASS")
		return;
	std::string pass = line.substr(5);
	client.setPassword(pass);
	if (pass == _pass) client.setAuth(true);
	checkFullAuth(client);
}

void Server::handle_nick(Client& client, const std::string& line) {
	if (client.hasNick()) return;
	std::string nickname = line.substr(5);
	if (nickname.empty()) return;
    if (nicknameExists(nickname)) {
        std::string err = ":localhost 433 * " + nickname + " :Nickname is already in use\r\n";
        send(client.getFd(), err.c_str(), err.size(), 0);
        return;
    }
	client.setNickname(nickname);
	client.setHasNick(true);
	checkFullAuth(client);
}

void Server::handle_user(Client& client, const std::string& line) {
	if (client.hasUser()) return;
	std::istringstream iss(line);
	std::string cmd, username, mode, unused, realname;
	iss >> cmd >> username >> mode >> unused;
	std::getline(iss, realname);
	if (!realname.empty() && realname[0] == ':')
		realname = realname.substr(1);
	client.setUsername(username);
	client.setRealname(realname);
	client.setHasUser(true);
	checkFullAuth(client);
}

void Server::checkFullAuth(Client& client) {
	if (!client.hasNick() || !client.hasUser() || !client.hasAuth())
		return;
	// Numéro de bienvenue 001 avec le prefix serveur et info user complète
	std::string rpl_001 = ":localhost 001 " + client.getNickname() + " :Welcome to the Internet Relay Network " + client.getFullPrefix() + "\r\n";
	// Message 002
	std::string rpl_002 = ":localhost 002 " + client.getNickname() + " :Your host is localhost, running version 1.0\r\n";
	// Message 003
	std::string rpl_003 = ":localhost 003 " + client.getNickname() + " :This server was created ...\r\n";
	// Message 004
	std::string rpl_004 = ":localhost 004 " + client.getNickname() + " localhost 1.0 o o\r\n";
	send(client.getFd(), rpl_001.c_str(), rpl_001.size(), 0);
	send(client.getFd(), rpl_002.c_str(), rpl_002.size(), 0);
	send(client.getFd(), rpl_003.c_str(), rpl_003.size(), 0);
	send(client.getFd(), rpl_004.c_str(), rpl_004.size(), 0);
}

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                                JOIN / PART                                |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

void Server::handle_join(Client& client, const std::string& line) {
	std::istringstream iss(line);
	std::string cmd, channel_name, password;
	iss >> cmd >> channel_name >> password;
	if (channel_name.empty() || channel_name[0] != '#') return;
	if (_channels.find(channel_name) == _channels.end()) {
		_channels[channel_name] = new Channel(channel_name);
		_channels[channel_name]->addOperator(&client);
	}
	Channel* chan = _channels[channel_name];
	// protection mode +i
	if (chan->isInviteOnly() && !chan->isInvited(&client)) {
		std::string err = ":localhost 473 " + client.getNickname() + " " + channel_name + " :Cannot join channel (+i)\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	// protection mode +k
	if (chan->hasPassword() && !chan->checkPassword(password)) {
		std::string err = ":localhost 475 " + client.getNickname() + " " + channel_name + " :Cannot join channel (+k)\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	// protection mode +l
	if (chan->isFull()) {
		std::string err = ":localhost 471 " + client.getNickname() + " " + channel_name + " :Cannot join channel (+l)\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}

	if (chan->hasMember(&client)) return;
	chan->addMember(&client);
	client.joinChannel(channel_name);
	std::string join_msg = ":" + client.getFullPrefix() + " JOIN :" + channel_name + "\r\n";
	chan->broadcast(join_msg, NULL);

	if (!chan->getTopic().empty()) {
		std::string topic = ":localhost 332 " + client.getNickname() + " " + channel_name + " :" + chan->getTopic() + "\r\n";
		send(client.getFd(), topic.c_str(), topic.size(), 0);
	}
	std::string names = chan->getNamesList();
	std::string namreply = ":localhost 353 " + client.getNickname() + " = " + channel_name + " :" + names + "\r\n";
	std::string end = ":localhost 366 " + client.getNickname() + " " + channel_name + " :End of /NAMES list\r\n";
	send(client.getFd(), namreply.c_str(), namreply.size(), 0);
	send(client.getFd(), end.c_str(), end.size(), 0);
}

void Server::handle_part(Client& client, const std::string& line) {
	std::istringstream iss(line);
	std::string cmd, channel_name, reason;
	iss >> cmd >> channel_name;
	std::getline(iss, reason);
	if (!reason.empty() && reason[1] == ':')
		reason = reason.substr(2);
	else
		reason = "";
	if (channel_name.empty()) return;
	std::map<std::string, Channel*>::iterator it = _channels.find(channel_name);
	if (it == _channels.end()) return;
	Channel* chan = it->second;
	if (!chan->hasMember(&client)) return;
	std::string part_msg = ":" + client.getFullPrefix() + " PART " + channel_name;
	if (!reason.empty())
		part_msg += " :" + reason;
	part_msg += "\r\n";
	chan->broadcast(part_msg);
	send(client.getFd(), part_msg.c_str(), part_msg.size(), 0);
	chan->removeMember(&client);
	client.leaveChannel(channel_name);
	if (chan->isEmpty()) {
		delete chan;
		_channels.erase(it);
	}
}

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                                PRIVMSG                                    |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

void Server::handle_privmsg(Client& sender, const std::string& line) {
	std::istringstream iss(line);
	std::string cmd, target;
	iss >> cmd >> target;
	std::string msg;
	std::getline(iss, msg);
	if (!msg.empty() && msg[0] == ' ') msg = msg.substr(1);
	if (!msg.empty() && msg[0] == ':') msg = msg.substr(1);
	if (target.empty()){
		std::string err = ":localhost 411 " + sender.getNickname() + " :No recipient given (PRIVMSG)\r\n";
		send(sender.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	if (msg.empty()){
		std::string err = ":localhost 412 " + sender.getNickname() + " :No text to send\r\n";
		send(sender.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	std::string full_msg = ":" + sender.getFullPrefix() + " PRIVMSG " + target + " :" + msg + "\r\n";
	if (target[0] == '#') {
		std::map<std::string, Channel*>::iterator it = _channels.find(target);
		if (it == _channels.end()) return;
		if (!it->second->hasMember(&sender)) {
			std::string err = ":localhost 404 " + sender.getNickname() + " " + target + " :Cannot send to channel\r\n";
			send(sender.getFd(), err.c_str(), err.size(), 0);
			return;
		}
		it->second->broadcast(full_msg, &sender);
	} else {
		std::map<int, Client>::iterator it;
		for (it = _clients.begin(); it != _clients.end(); ++it) {
			if (it->second.getNickname() == target) {
				send(it->second.getFd(), full_msg.c_str(), full_msg.size(), 0);
				return;
			}
		}
		std::string err = ":localhost 401 " + sender.getNickname() + " " + target + " :No such nick/channel\r\n";
		send(sender.getFd(), err.c_str(), err.size(), 0);
	}
}

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                                  QUIT                                     |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

void Server::handle_quit(Client& client, const std::string& line) {
	std::string reason = "Disconnected";
	size_t pos = line.find(":");
	if (pos != std::string::npos && pos + 1 < line.length())
		reason = line.substr(pos + 1);
	std::string nickname = client.getNickname();
	std::string prefix = client.getFullPrefix();
	std::string quit_msg = ":" + prefix + " QUIT :" + reason + "\r\n";
	std::set<std::string> joined = client.getJoinedChannels();
	std::set<std::string>::const_iterator it;
	for (it = joined.begin(); it != joined.end(); ++it) {
		Channel* chan = _channels[*it];
		chan->broadcast(quit_msg, &client);
		chan->removeMember(&client);
	}
	leaveAllChannels(client);
	std::cout << "[QUIT] " << nickname << " a quitté : " << reason << std::endl;
	disconnectClient(client.getFd());
}

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                                   TOPIC                                   |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

void Server::handle_topic(Client& client, const std::string& line) {
    std::istringstream iss(line);
    std::string cmd, channel_name;
    iss >> cmd >> channel_name;
    if (channel_name.empty()) return;
    std::map<std::string, Channel*>::iterator it = _channels.find(channel_name);
    if (it == _channels.end()) return;
    Channel* chan = it->second;
    if (!chan->hasMember(&client)) return;
    std::string rest;
    std::getline(iss, rest);
    if (!rest.empty() && rest[1] == ':') {
		// Si le salon est en +t et que le client n’est pas op
		if (chan->isTopicRestricted() && !chan->isOperator(&client)) {
			std::string err = ":localhost 482 " + client.getNickname() + " " + channel_name + " :You're not channel operator\r\n";
			send(client.getFd(), err.c_str(), err.size(), 0);
			return;
		}
        std::string new_topic = rest.substr(2);
        chan->setTopic(new_topic);

        std::string msg = ":" + client.getFullPrefix() + " TOPIC " + channel_name + " :" + new_topic + "\r\n";
        chan->broadcast(msg);
    } else {
        std::string topic = chan->getTopic();
		std::string msg;
        if (topic.empty())
			msg = ":localhost 331 " + client.getNickname() + " " + channel_name + " :" + "No topic is set" + "\r\n";
		else
       		msg = ":localhost 332 " + client.getNickname() + " " + channel_name + " :" + topic + "\r\n";
        send(client.getFd(), msg.c_str(), msg.size(), 0);
    }
}

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                                    MODE                                   |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

void Server::_init_mode_map() {
	_mode_map['+']['t'] = &Server::mode_enable_t;
	_mode_map['-']['t'] = &Server::mode_disable_t;
	_mode_map['+']['i'] = &Server::mode_enable_i;
	_mode_map['-']['i'] = &Server::mode_disable_i;
	_mode_map['+']['k'] = &Server::mode_enable_k;
	_mode_map['-']['k'] = &Server::mode_disable_k;
	_mode_map['+']['l'] = &Server::mode_enable_l;
	_mode_map['-']['l'] = &Server::mode_disable_l;
	_mode_map['+']['o'] = &Server::mode_enable_o;
	_mode_map['-']['o'] = &Server::mode_disable_o;
}

void Server::handle_mode(Client& client, const std::string& line) {
	std::istringstream iss(line);
	std::string cmd, channel_name, mode;
	iss >> cmd >> channel_name >> mode;
	std::string param;
	iss >> param;

	if (channel_name.empty())
		return;
	std::map<std::string, Channel*>::iterator it = _channels.find(channel_name);
	if (it == _channels.end())
		return;
	Channel* chan = it->second;
	if (mode.empty()) {
		std::string modes = "+";
		if (chan->isTopicRestricted()) modes += "t";
		if (chan->isInviteOnly()) modes += "i";
		if (chan->hasPassword()) modes += "k";
		if (chan->hasUserLimit()) modes += "l";
		if (modes == "+") return;
		std::string msg = ":localhost 324 " + client.getNickname() + " " + channel_name + " " + modes + "\r\n";
		send(client.getFd(), msg.c_str(), msg.size(), 0);
		return;
	}
	// check op pour modifier
	if (!chan->isOperator(&client)) {
		std::string err = ":localhost 482 " + client.getNickname() + " " + channel_name + " :You're not channel operator\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	char sign = mode[0];
	char flag = mode[1];
	// Vérifie si le mode nécessite un paramètre mais qu'il est manquant
	if ((flag == 'k' || flag == 'l' || flag == 'o') && sign == '+' && param.empty()) {
		std::string err = ":localhost 461 " + client.getNickname() + " MODE :Not enough parameters\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	if (_mode_map.count(sign) && _mode_map[sign].count(flag))
		(this->*(_mode_map[sign][flag]))(client, chan, param);
	else {
		std::string err = ":localhost 472 " + client.getNickname() + " " + std::string(1, flag) + " :is unknown mode\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
	}

}

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                                  INVITE                                   |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

void Server::handle_invite(Client& client, const std::string& line) {
	std::istringstream iss(line);
	std::string cmd, target_nick, channel_name;
	iss >> cmd >> target_nick >> channel_name;
	if (target_nick.empty() || channel_name.empty())
		return;
	std::map<std::string, Channel*>::iterator chan_it = _channels.find(channel_name);
	if (chan_it == _channels.end()){
		std::string err = ":localhost 401 " + client.getNickname() + " " + target_nick + " :No such nick/channel\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	Channel* chan = chan_it->second;
	if (!chan->hasMember(&client))
		return;
	if (!chan->isOperator(&client)){
		std::string err = ":localhost 482 " + client.getNickname() + " " + channel_name + " :You're not channel operator\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	Client* target = NULL;
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if (it->second.getNickname() == target_nick) {
			target = &it->second;
			break;
		}
	}
	if (!target){
		std::string err = ":localhost 401 " + client.getNickname() + " " + target_nick + " :No such nick/channel\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	if (chan->hasMember(target)){
		std::string err = ":localhost 443 " + client.getNickname() + " " + target_nick + " " + channel_name + " :is already on channel\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	chan->invite(target);
	std::string rpl = ":localhost 341 " + client.getNickname() + " " + channel_name + "\r\n";
	send(client.getFd(), rpl.c_str(), rpl.size(), 0);
	std::string notif = ":" + client.getFullPrefix() + " INVITE " + target_nick + " :" + channel_name + "\r\n";
	send(target->getFd(), notif.c_str(), notif.size(), 0);
}

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                                   KICK                                    |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

void Server::handle_kick(Client& client, const std::string& line) {
	std::istringstream iss(line);
	std::string cmd, channel_name, target_nick;
	iss >> cmd >> channel_name >> target_nick;
	if (channel_name.empty() || target_nick.empty())
		return;
	std::map<std::string, Channel*>::iterator it = _channels.find(channel_name);
	if (it == _channels.end()){
		std::string err = ":localhost 403 " + client.getNickname() + " " + channel_name + " :No such channel\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	Channel* chan = it->second;
	if (!chan->hasMember(&client)){
		std::string err = ":localhost 442 " + client.getNickname() + " " + channel_name + " :You're not on that channel\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	} 
	if (!chan->isOperator(&client)){
		std::string err = ":localhost 482 " + client.getNickname() + " " + channel_name + " :You're not channel operator\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	Client* target = NULL;
	for (std::map<int, Client>::iterator cit = _clients.begin(); cit != _clients.end(); ++cit) {
		if (cit->second.getNickname() == target_nick) {
			target = &cit->second;
			break;
		}
	}
	if (!target)
		return;
	if (!chan->hasMember(target)){
		std::string err = ":localhost 441 " + client.getNickname() + " " + target_nick + " " + channel_name + " :They aren't on that channel\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	std::string reason;
	std::getline(iss, reason);
	if (!reason.empty() && reason[1] == ':')
		reason = reason.substr(2);
	else
		reason = "Kicked";
	std::string msg = ":" + client.getFullPrefix() + " KICK " + channel_name + " " + target_nick + " :" + reason + "\r\n";
	chan->broadcast(msg);
	chan->removeMember(target);
	target->leaveChannel(channel_name);
	if (chan->isEmpty()){
		delete chan;
		_channels.erase(channel_name);
	}
}

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                                  NOTICE                                   |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

void Server::handle_notice(Client& client, const std::string& line) {
	std::istringstream iss(line);
	std::string cmd, target;
	iss >> cmd >> target;
	if (target.empty())
		return;
	std::string msg;
	std::getline(iss, msg);
	if (!msg.empty() && msg[1] == ':')
		msg = msg.substr(2);
	std::string full_msg = ":" + client.getFullPrefix() + " NOTICE " + target + " :" + msg + "\r\n";
	if (target[0] == '#') {
		std::map<std::string, Channel*>::iterator it = _channels.find(target);
		if (it == _channels.end()) return;
		Channel* chan = it->second;
		if (!chan->hasMember(&client)) return;
		chan->broadcast(full_msg, &client);
	} else {
		for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
			if (it->second.getNickname() == target) {
				send(it->second.getFd(), full_msg.c_str(), full_msg.size(), 0);
				return;
			}
		}
	}
}

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                            Utilitaires / Clean                            |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

void Server::leaveAllChannels(Client& client) {
	std::set<std::string> channels = client.getJoinedChannels();
	std::set<std::string>::iterator it;
	for (it = channels.begin(); it != channels.end(); ++it) {
		std::map<std::string, Channel*>::iterator chan_it = _channels.find(*it);
		if (chan_it != _channels.end()) {
			chan_it->second->removeMember(&client);
			if (chan_it->second->isEmpty()) {
				delete chan_it->second;
				_channels.erase(chan_it);
			}
		}
		client.leaveChannel(*it);
	}
}

void Server::disconnectClient(int fd) {
    std::map<int, Client>::iterator it = _clients.find(fd);
    if (it == _clients.end()) return;
    Client& client = it->second;
	client.setHasNick(false);
	client.setAuth(false);
	client.setHasUser(false);
	//leaveAllChannels(client);
	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);
	_clients.erase(fd);
	std::cout << "[SERVER] Client " << fd << " déconnecté." << std::endl;
}

bool Server::nicknameExists(const std::string& nickname) const {
    for (std::map<int, Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second.getNickname() == nickname)
            return true;
    }
    return false;
}






void Server::handle_ping(Client& client, const std::string& line) {
	size_t pos = line.find("PING");
	std::string token = "localhost";
	if (pos != std::string::npos && pos + 5 < line.length())
		token = line.substr(pos + 5);
	if (!token.empty() && token[0] == ':')
		token = token.substr(1);

	std::string pong = "PONG :" + token + "\r\n";
	send(client.getFd(), pong.c_str(), pong.size(), 0);
}

void Server::handle_cap(Client& client, const std::string& line) {
    if (line.find("LS") != std::string::npos) {
        std::string response = "CAP * LS :\r\n"; // vide = pas de capabilities supportées
        send(client.getFd(), response.c_str(), response.length(), 0);
    }
}