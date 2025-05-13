#include "Channel.hpp"
#include <algorithm>
#include <sys/socket.h>
#include <cstring>

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                         Constructors / Destructor                         |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

Channel::Channel() {}

Channel::Channel(const std::string& name) : _name(name), _topic(""), _password(""),
	_userLimit(0), _inviteOnly(false), _topicRestricted(false), _hasPassword(false),
	_has_limit(false) {}

Channel::~Channel() {}

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                               Getters / Setters                           |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

std::string Channel::getNamesList() const {
	std::string list;
	for (std::set<Client*>::const_iterator it = _members.begin(); it != _members.end(); ++it) {
		if (isOperator(*it))
			list += "@" + (*it)->getNickname() + " ";
		else
			list += (*it)->getNickname() + " ";
	}
	if (!list.empty())
		list.erase(list.end() - 1);
	return list;
}

const std::string& Channel::getName() const { return _name; }

const std::string& Channel::getTopic() const { return _topic; }

void Channel::setTopic(const std::string& topic) { _topic = topic; }

const std::string& Channel::getPassword() const { return _password; }

void Channel::setPassword(const std::string& password) { _password = password; _hasPassword = true; }

void Channel::removePassword() { _password.clear(); _hasPassword = false; }

bool Channel::hasPassword() const { return _hasPassword; }

bool Channel::checkPassword(const std::string& password) const {
	return (_hasPassword && password == _password);
}

int Channel::getUserLimit() const { return _userLimit; }

void Channel::setUserLimit(int limit) { _userLimit = limit; _has_limit = true; }

void Channel::removeUserLimit() { _userLimit = 0; _has_limit = false; }

bool Channel::hasUserLimit() const { return _has_limit; }

bool Channel::isFull() const {
	return (_has_limit && (int)_members.size() >= _userLimit);
}

void Channel::setInviteOnly(bool value) { _inviteOnly = value; }

bool Channel::isInviteOnly() const { return _inviteOnly; }

void Channel::setTopicRestricted(bool value) { _topicRestricted = value; }

bool Channel::isTopicRestricted() const { return _topicRestricted; }

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                            Members / Operators                            |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

void Channel::addMember(Client* client) {
	_members.insert(client);
}

void Channel::removeMember(Client* client) {
	_members.erase(client);
	_operators.erase(client);
	_invitedUsers.erase(client);
}

bool Channel::hasMember(Client* client) const {
	return _members.find(client) != _members.end();
}

std::set<Client*>& Channel::getMembers() {
	return _members;
}

void Channel::addOperator(Client* client) {
	_operators.insert(client);
}

void Channel::removeOperator(Client* client) {
	_operators.erase(client);
}

bool Channel::isOperator(Client* client) const {
	return _operators.find(client) != _operators.end();
}

void Channel::invite(Client* client) {
	_invitedUsers.insert(client);
}

bool Channel::isInvited(Client* client) const {
	return _invitedUsers.find(client) != _invitedUsers.end();
}

bool Channel::isEmpty() const {
	return _members.empty();
}

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                                Messaging                                  |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/

void Channel::broadcast(const std::string& message, Client* exclude) {
	std::set<Client*>::iterator it = _members.begin();
	for (; it != _members.end(); ++it) {
		if (*it != exclude) {
			send((*it)->getFd(), message.c_str(), message.size(), 0);
		}
	}
}
