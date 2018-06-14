#ifndef CLIENT_H
#define CLIENT_H

#include <QtGui>
#include <QDebug>
#include <QSound>
#include <QtNetwork>
#include <QMessageBox>
#include <QInputDialog>
#include "ui_client.h"
#include "useritem.h"
#include "interaction.h"
#include "memberlist.h"
#include "servercommand.h"
#include "voipclient.h"

using namespace ServerCommand;


class Client : public QMainWindow, private Ui::Client {

    Q_OBJECT

    public:
        Client();
        Client(const Client &client);
        ~Client();
        Client& operator =(const Client& client);
        void connectToServer(QString IP, int port);
        void setLoginInfo(QString login, QString password);
        void changePassword();
        void sigIn();
        void logIn();

    private slots:
        void on_sendButton_clicked();
        void on_message_returnPressed();
        void on_conversationsList_itemClicked(QListWidgetItem *item);
        void on_filterChat_textChanged(const QString &arg1);
        void on_filterMember_textChanged(const QString &arg1);
        void on_exit_triggered();
        void on_disableSound_triggered(bool checked);
        void on_about_triggered();
        void on_changeNick_triggered();
        void on_newConversation_triggered();
        void on_userList_itemClicked(QListWidgetItem *item);
        void dataReceived();
        void connectUser();
        void loggedOut();
        void logOut();
        void socketError(QAbstractSocket::SocketError error);
        void getNewNickname(QString nickanme);
        void getConvName(QString);
        void showContextMenu(const QPoint&);
        void getSelectedToCreate();
        void getUserFromSearch(QListWidgetItem* item);
        void createDialog(QListWidgetItem *item);
        void checkNickname();
        void kickFromChat();
        void on_addMemberButton_clicked();
        void on_findPeople_triggered();
        void on_fullConversation_triggered();
        void on_deleteButton_clicked();
        void on_userList_itemPressed(QListWidgetItem *item);
        void on_callButon_clicked();
        void on_endButton_clicked();
        void endCall();

        void on_changePorts_triggered();

signals:
        void statusOperation(int, int);
        void backOffline(int);

    private:
        quint16 messageLength;
        quint16 operation;
        quint16 status;
        QString login;
        QString password;
        QString nickname;
        QTcpSocket *socket;
        VoIPClient *call;
        int id;
        int idCaller;
        QList<User> conversations;
        QList<User> users;
        UserItem* currConversation;
        UserItem* selectedUser;
        Interaction* suboperation;
        MemberList *searchWindow;
        bool soundOff;
        void sendPacket(int operation, QString buffer);
        void fillList(QString packet);
        void fillMessageArea(QString packet);
        void addInfoMessage (int operation, QString id);
        void getMessage(QString packet);
        void changeFont(int idOperation, QListWidgetItem *item);
        void changeOnlineStatus(QString packet);
        void changeNickname(QString packet, int status);
        void addNewConversation(QString packet);
        void sendInfoToAdd(QListWidgetItem *item);
        void addToChat(QString packet);
        void chooseNewConversation(QString packet);
        void cleanConversation(QString packet);
        void startTalking(QString ip, int status);
        void incomingCall(QString ip);
        QList<QListWidgetItem*>& formListMember(ServerCommand::Mode, QString packet, QList<QListWidgetItem*>& list);
};

#endif // CLIENT_H
