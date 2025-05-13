#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "lib.hpp"

class Client {
private:

    std::set<std::string> _joined_channels;

    int         _fd;
    std::string _buffer;
    
    bool _has_user;
    std::string _username;
    std::string _realname;

    bool _has_nick;
    std::string _nickname;

    bool _has_auth;
    std::string _password;


public:

    Client();
    Client(int fd);
    Client(const Client& other);
    Client& operator=(const Client& other);
    ~Client();

    int getFd() const;
    std::string& getBuffer();
    std::string getPassword() const;
    std::string getNickname() const;
    std::string getUsername() const;
    std::string getRealname() const;
    std::string getFullPrefix() const;

    bool hasAuth() const;
    bool hasNick() const;
    bool hasUser() const;

    void setAuth(bool status);
    void setHasUser(bool value);
    void setHasNick(bool value);

    void appendToBuffer(std::string const& data);
    void clearBuffer();

    void setPassword(std::string const& pass);
    void setNickname(std::string const& nick);
    void setUsername(std::string const& name);
    void setRealname(std::string const& name);

    void joinChannel(std::string const& name);
    void leaveChannel(std::string const& name);
    const std::set<std::string>& getJoinedChannels() const;


};

#endif
