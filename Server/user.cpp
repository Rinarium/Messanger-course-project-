#include "user.h"

User::User()
{
    socket = nullptr;
}

void User::setId(int id)
{
    this->id = id;
}

void User::setSocket(QTcpSocket* socket)
{
    this->socket = socket;
}

int User::getId()
{
    return id;
}

QTcpSocket* User::getSocket()
{
    return socket;
}

bool operator == (const User &first, const User &second)
{
    if (first.id == second.id) return true;
    if (first.socket == second.socket) return true;
    return false;
}
