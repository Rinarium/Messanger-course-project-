#ifndef DATABASE_H
#define DATABASE_H

#include "QtSql/QtSql"
#include "QList"

class DataBase
{
public:
    DataBase();
    ~DataBase();
    void setIP(QString IP, int ID);
    void makeOffline(int ID);
    void makeOnline(int ID);
    void updateReceived(QString idConv, QString idSender);
    void setReceived(QString idConv, int idPerson);
    void returnToConv(int idConv, int idPerson);
    void deleteConversation(QString idConv);
    void changeAdmin(QString idConv, QString candidate);
    int checkPasswotd(QString login, QString password);
    int changePassword(QString login, QString password);
    int changeNickname(QString name, int ID);
    int sigin(QString login, QString password);
    int getInterlocID(int idConv, int idPerson);
    int getID(QString login);
    QString getID(QString idPersonChat, int idInterloc);
    QString getIP(int idPerson);
    QString getNickname(int id);
    QString getLastConversation();
    QString createConversation(QString name, int idPerson, int idCreator);
    QString formConversations(QString id);
    QString formMembers(QString id);
    QString checkNewDialog(QString idConv);
    QString checkRestore(int idConv, int idPerson);
    QString addToChat(QString idPerson, QString idConv);
    QString kickFromChat(QString idPerson, QString idConv);
    QString getFullUserList();
    QList<int> getContactList(int ID);
    QList<int> getMemberList(QString idConv);
    QList<int> getWholeConvMember(QString idConv);
    QSet<int> getRelationList(int ID);

private:
    QSqlDatabase database;
    QSqlQuery* query;
    QString command;
};

#endif // DATABASE_H
