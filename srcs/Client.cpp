#include "Client.hpp"

/*--- Constructor & Destructor ---*/

Client::Client() : _fd(-1) {}

Client::Client(int fd) : _fd(fd), _buffer(""), _has_user(false), _username(""),
    _realname(""), _has_nick(false), _nickname(""), _has_auth(false), _password("") {}

Client::~Client(){}

Client::Client(const Client& other) : _fd(other._fd), _buffer(other._buffer),
	_has_user(other._has_user), _username(other._username), _realname(other._realname),
	_has_nick(other._has_nick), _nickname(other._nickname), _has_auth(other._has_auth),
	_password(other._password) {}

/*--- Functions Get ---*/

int Client::getFd() const{
    return (this->_fd);
}

std::string& Client::getBuffer() {
    return (this->_buffer);
}

std::string Client::getPassword() const {
    return _password;
}

std::string Client::getUsername() const {
    return _username;
}

std::string Client::getRealname() const {
    return _realname;
}

std::string Client::getNickname() const {
    return _nickname;
}

const std::set<std::string>& Client::getJoinedChannels() const {
    return _joined_channels;
}

std::string Client::getFullPrefix() const {
	return _nickname + "!" + _username + "@localhost";
}


/*--- Functions password ---*/

bool Client::hasAuth() const {
    return _has_auth;
}

void Client::setAuth(bool status) {
    _has_auth = status;
}

void Client::setPassword(std::string const& pass) {
    _password = pass;
}

/*--- Functions nickname ---*/

void Client::setNickname(std::string const& nick) {
    _nickname = nick;
}


bool Client::hasNick() const {
    return _has_nick;
}

void Client::setHasNick(bool value) {
    _has_nick = value;
}

/*--- Functions user ---*/

bool Client::hasUser() const {
    return _has_user;
}

void Client::setHasUser(bool value) {
    _has_user = value;
}

void Client::setUsername(std::string const& name) {
    _username = name;
}

void Client::setRealname(std::string const& name) {
    _realname = name;
}

/*--- Functions Buffer ---*/

void Client::appendToBuffer(std::string const& data) {
    _buffer += data;
}

void Client::clearBuffer() {
    _buffer.clear();
}

/*--- Functions Channel ---*/

void Client::joinChannel(std::string const& name) {
    _joined_channels.insert(name);
}

void Client::leaveChannel(std::string const& name) {
    _joined_channels.erase(name);
}

/*--- Operator ---*/

Client& Client::operator=(const Client& other) {
	if (this != &other) {
		_fd         = other._fd;
		_has_user   = other._has_user;
		_username   = other._username;
		_realname   = other._realname;
		_has_nick   = other._has_nick;
		_nickname   = other._nickname;
		_has_auth   = other._has_auth;
		_password   = other._password;
		_buffer     = other._buffer;
	}
	return *this;
}

