#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>
#include <map>
#include <vector>
#include "Client.hpp"

class Channel {
private:
    std::string _name;
    std::string _topic;
    std::string _password;
    int _userLimit;
    bool _inviteOnly;
    bool _topicRestricted;
    bool _hasPassword;
    bool _has_limit;

    std::set<Client*> _members;
    std::set<Client*> _operators;
    std::set<Client*> _invitedUsers;

public:

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                         Constructors / Destructor                         |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/
    Channel();
    Channel(const std::string& name);
    ~Channel();

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                               Getters / Setters                           |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/
    std::string getNamesList() const;
    const std::string& getName() const;
    const std::string& getTopic() const;
    void setTopic(const std::string& topic);

    const std::string& getPassword() const;
    void setPassword(const std::string& password);
    void removePassword();
    bool hasPassword() const;
    bool checkPassword(const std::string& password) const;

    int getUserLimit() const;
    void setUserLimit(int limit);
    void removeUserLimit();
    bool hasUserLimit() const;
    bool isFull() const;

    void setInviteOnly(bool value);
    bool isInviteOnly() const;

    void setTopicRestricted(bool value);
    bool isTopicRestricted() const;

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                            Members / Operators                            |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/
    void addMember(Client* client);
    void removeMember(Client* client);
    bool hasMember(Client* client) const;
    std::set<Client*>& getMembers();

    void addOperator(Client* client);
    void removeOperator(Client* client);
    bool isOperator(Client* client) const;

    void invite(Client* client);
    bool isInvited(Client* client) const;

    bool isEmpty() const;

/*
	.   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   .
	|                                Messaging                                  |
	'   -'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-',-'   '
*/
    void broadcast(const std::string& message, Client* exclude = NULL);
};

#endif
