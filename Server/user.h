#ifndef USER_H
#define USER_H

#include <QtNetwork>

class User
{
    friend bool operator == (const User &first, const User &second);
public:
    User();
    void setId(int id);
    void setSocket(QTcpSocket* socket);
    int getId();
    QTcpSocket* getSocket();

private:
    int id;
    QTcpSocket* socket;
};

#endif // USER_H
