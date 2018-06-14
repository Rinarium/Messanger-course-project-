#ifndef SERVER_H
#define SERVER_H

#include <QtWidgets>
#include <QtNetwork>
#include "user.h"
#include "servercommand.h"
#include "database.h"

using namespace ServerCommand;

class Server : public QWidget {

    Q_OBJECT

    public:
        Server();
        ~Server();
        void sendMessage(const QString &message);

    private slots:
        void newConnection();
        void dataReceived();
        void logoutUser();

    private:
        QLabel *serverState;
        QPushButton *quitButton;
        QTcpServer *server;
        User *currClient;
        QList<User> users;
        quint16 messageLength;
        quint16 operation;
        DataBase* database;
        QString formMembers(QString id);
        void login(QString buffer);
        void sigin(QString buffer);
        void changePassword(QString buffer);
        void sendPacket(int operation, int status, QString buffer);
        void sendConversation(QString id, int type);
        void changeNickname(QString name);
        void createConversation(QString name);
        void addToChat(QString buffer);
        void sendFullUserList();
        void kickFromChat(QString message);
        bool checkRestoreChat(int id);
        void checkIP();
        void sendIP(QString ids);
        void endCall(QString id);
        void writeCallInfo(QString idConv, int operation);
};

#endif // SERVER_H
