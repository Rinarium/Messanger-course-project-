#include "database.h"

DataBase::DataBase()
{
    database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName("chat.sqlite");
    database.open();
    qDebug() << QDir::currentPath() << endl;

    query = new QSqlQuery(database);
    command = "UPDATE Users SET Online = 0";
    query->exec(command);
    command = "UPDATE Users SET IP = 0";
    query->exec(command);
}

DataBase::~DataBase()
{
    database.close();
}

void DataBase::setIP(QString IP, int ID)
{
    command = "UPDATE Users SET IP = \"%1\" "
              "WHERE ID = %2";
    query->exec(command.arg(IP).arg(ID));
}

void DataBase::makeOffline(int ID)
{
    command = "UPDATE Users SET Online = 0 "
              "WHERE ID = %1;";
    query->exec(command.arg(ID));
}

void DataBase::makeOnline(int ID)
{
    command = "UPDATE Users SET Online = 1 "
              "WHERE ID = %1;";
    query->exec(command.arg(ID));
}

QList<int> DataBase::getContactList(int ID)
{

    command = "SELECT Dialog FROM Contacts "
              "WHERE ID = %1 "
              "AND Dialog != \"FALSE\";";

    query->exec(command.arg(ID));
    query->first();

    QList<int> contactList;

    do
    {
        contactList << query->value(0).toInt();

    } while(query->next());

    return contactList;
}

void DataBase::updateReceived(QString idConv, QString idcommander)
{
    command = "UPDATE Contacts SET Received = 0 "
              "WHERE Conversation = %1 "
              "AND ID != %2;";

    query->exec(command.arg(idConv).arg(idcommander));
}

QList<int> DataBase::getMemberList(QString idConv)
{
    command = "SELECT ID FROM Contacts "
              "WHERE Conversation = %1 "
              "AND Left = 0";

    query->exec(command.arg(idConv));
    query->first();

    QList<int> memberList;

    do
    {
        memberList << query->value(0).toInt();

    } while(query->next());

    return memberList;
}

QString DataBase::checkNewDialog(QString idConv)
{
    command = "SELECT ID, Dialog FROM Contacts "
              "WHERE Conversation = %1;";

    query->exec(command.arg(idConv));
    query->first();

    if (query->value(1).toString() == "FALSE") return NULL;

    if (query->next() == false)
    {
        query->first();

        QString idInter = query->value(0).toString();
        QString idUser = query->value(1).toString();

        command = "INSERT INTO Contacts (ID, Conversation, Dialog, Received, Left) "
                  "VALUES (%1, %2, %3, 0, 0)";
        query->exec(command.arg(idUser).arg(idConv).arg(idInter));

        command = "SELECT Nickname FROM Users "
                  "WHERE ID = %1";
        query->exec(command.arg(idInter));
        query->first();

        return query->value(0).toString();
    }
    else return NULL;
}

int DataBase::checkPasswotd(QString login, QString password)
{
    command = "SELECT * FROM Users "
              "WHERE Login = \"%2\"";

    query->exec(command.arg(login));
    query->first();

    if (password.compare(query->value(2).toString(), Qt::CaseSensitive) != 0) return 0;

    if (query->value(4).toBool() == true) return 1;

    return 2;
}

int DataBase::getID(QString login)
{
    command = "SELECT ID FROM Users "
              "WHERE Login = \"%2\"";

    query->exec(command.arg(login));
    query->first();

    return query->value(0).toInt();
}

QString DataBase::formConversations(QString id)
{
    QSqlQuery queryID;

    QString findID = "SELECT Conversation, Dialog, Received "
                     "FROM Contacts "
                     "WHERE ID = %1 AND Left = 0;";
    QString conversationName = "SELECT %1 FROM %2 "
                               "WHERE ID=%3;";
    QString findName, conversationList = "";
    QString adminID;

    findName = "SELECT Nickname FROM Users "
               "WHERE ID=%1;";
    query->exec(findName.arg(id));
    query->first();

    QString ownInfo = id + "," + query->value(0).toString();

    findID = findID.arg(id);
    queryID.exec(findID);

    if (queryID.first() == false) return ownInfo;

    do
    {
        qDebug() << queryID.value(0).toInt() << queryID.value(1).toString() << endl;

        if (queryID.value(1).toString() != "FALSE")
        {
            findName = conversationName.arg("Nickname")
                                       .arg("Users")
                                       .arg(queryID.value(1).toString());
            adminID = "-1";
        }
        else
        {
            findName = conversationName.arg("Name")
                                       .arg("Conversations")
                                       .arg(queryID.value(0).toString());
            adminID = "0";
        }

        query->exec(findName);
        query->first();

        conversationList = queryID.value(0).toString() + ',' + query->value(0).toString()
                + ',' + adminID + ',' + queryID.value(2).toString() + "|" + conversationList;
    } while(queryID.next());

    int pos = conversationList.lastIndexOf('|');
    conversationList = conversationList.left(pos);

    conversationList = ownInfo + "|" + conversationList;

    qDebug() << conversationList << endl;
    return conversationList;
}

QString DataBase::formMembers(QString id)
{
    QSqlQuery queryID;

    QString findID = "SELECT ID, Left FROM Contacts "
                     "WHERE Conversation=%1;";
    QString userName = "SELECT Nickname, Online FROM Users "
                       "WHERE ID=%1;";
    QString findName, conversationList = "";

    findID = findID.arg(id);
    queryID.exec(findID);

    if (queryID.first() == false) return conversationList;

    do
    {
        qDebug() << queryID.value(0).toInt() << endl;

        findName = userName.arg(queryID.value(0).toString());

        query->exec(findName);
        query->first();

        conversationList = queryID.value(0).toString() + "," + query->value(0).toString() + ","
                           + query->value(1).toString() + ',' + queryID.value(1).toString() + "|" + conversationList;
    } while(queryID.next());

    int pos = conversationList.lastIndexOf('|');
    conversationList = conversationList.left(pos);

    QString admin = "SELECT Admin FROM Conversations "
                    "WHERE ID=%1;";

    findName = admin.arg(id);

    query->exec(findName);
    query->first();

    conversationList = query->value(0).toString() + "||" + conversationList;

    qDebug() << conversationList << endl;
    return conversationList;
}

int DataBase::sigin(QString login, QString password)
{
    command = "SELECT * FROM Users "
              "WHERE Login = \"%1\"";
    query->exec(command.arg(login));

    if (query->first()) return 0;

    command = "INSERT INTO Users (Login, Password) "
              "VALUES ('%1', '%2')";

    query->exec(command.arg(login).arg(password));
    return 1;
}

int DataBase::changePassword(QString login, QString password)
{
    command = "SELECT * FROM Users "
              "WHERE Login = \"%1\"";
    query->exec(command.arg(login));

    if (!query->first()) return 0;

    command = "UPDATE Users SET Password = \"%2\" "
             "WHERE Login = \"%1\";";

    query->exec(command.arg(login).arg(password));
    return 1;
}

void DataBase::setReceived(QString idConv, int idPerson)
{
    command = "UPDATE Contacts SET Received = 1 "
              "WHERE Conversation = %1 "
              "AND ID = %2;";
    query->exec(command.arg(idConv).arg(idPerson));
}

int DataBase::changeNickname(QString name, int ID)
{
    QString command = "SELECT ID FROM Users "
                   "WHERE Nickname = \"%1\";";
    query->exec((command.arg(name)));

    if (query->first() == false)
    {
        command = "UPDATE Users SET Nickname = \"%1\" "
               "WHERE ID = %2;";
        query->exec(command.arg(name).arg(ID));
        return 1;
    }

    return 0;
}

QSet<int> DataBase::getRelationList(int ID)
{
    command = "SELECT Conversation FROM Contacts "
           "WHERE ID = %1;";
    query->exec(command.arg(ID));
    query->first();

    QSqlQuery queryID;
    QSet<int> idSet;

    do
    {
        command = "SELECT ID FROM Contacts "
               "WHERE Conversation = %1 "
               "AND ID != %2;";
        queryID.exec(command.arg(query->value(0).toString()).arg(QString::number(ID)));
        queryID.first();
        do
        {
            idSet << queryID.value(0).toInt();
        } while(queryID.next());

    } while(query->next());

    return idSet;
}

QString DataBase::createConversation(QString name, int idPerson, int idCreator)
{
     command = "INSERT INTO Conversations (Name, Dialog, Admin) "
               "VALUES (\"%1\", \"%2\", %3)";

    if (idPerson < 0)
    {
        query->exec(command.arg(name).arg("FALSE").arg(idCreator));
    }
    else
    {
        query->exec(command.arg("").arg("TRUE").arg(QString::number(-1)));
    }

    query->exec("SELECT ID FROM Conversations");
    query->last();
    QString packet = query->value(0).toString();

    command = "INSERT INTO Contacts (ID, Conversation, Dialog, Received, Left) "
              "VALUES (%1, %2, \"%3\", 1, 0)";

    if (idPerson < 0)
    {
        query->exec(command.arg(idCreator).arg(query->value(0).toString()).arg("FALSE"));
        packet = packet + '|' + name;
    }
    else
    {
        query->exec(command.arg(idCreator).arg(query->value(0).toString()).arg(idPerson));
        query->exec("SELECT Nickname FROM Users WHERE ID = " + QString::number(idPerson));
        query->first();
        packet = packet + '|' + query->value(0).toString();
    }

    return packet;
}

QString DataBase::getLastConversation()
{
    query->exec("SELECT ID FROM Conversations");
    query->last();
    return query->value(0).toString();
}

QString DataBase::checkRestore(int idConv, int idPerson)
{
    QString command = "SELECT Conversation FROM Contacts "
                      "WHERE ID = %1 AND Dialog = %2";
    query->exec(command.arg(idPerson).arg(idConv));

    if (query->first() == false) return "";
    return query->value(0).toString();
}

void DataBase::returnToConv(int idConv, int idPerson)
{
    command = "UPDATE Contacts SET Left = 0 "
              "WHERE ID = %1 AND Dialog = %2";
    query->exec(command.arg(idPerson).arg(idConv));
}

QString DataBase::getNickname(int id)
{
    query->exec("SELECT Nickname FROM Users WHERE ID = " + QString::number(id));
    query->first();
    return query->value(0).toString();
}

QString DataBase::getID(QString idPersonChat, int idInterloc)
{
    command = "SELECT Dialog FROM Contacts "
              "WHERE Conversation = %1 "
              "AND ID = %2";
    query->exec(command.arg(idPersonChat).arg(idInterloc));
    query->first();
    return query->value(0).toString();
}

QString DataBase::addToChat(QString idPerson, QString idConv)
{
   command = "SELECT Left FROM Contacts "
             "WHERE Conversation = %1 "
             "AND ID = %2";
   query->exec(command.arg(idConv).arg(idPerson));
   if (query->first() != true)
   {
       command = "INSERT INTO Contacts (ID, Conversation, Dialog, Received, Left) "
                 "VALUES (%1, %2, \"FALSE\", 0, 0)";
       query->exec(command.arg(idPerson).arg(idConv));
   }
   else
   {
       command = "UPDATE Contacts SET Left = 0 "
                 "WHERE Conversation = %1 "
                 "AND ID = %2";
       query->exec(command.arg(idConv).arg(idPerson));
   }

    command = "UPDATE Contacts SET Receveid = 0 "
              "WHERE Conversation = %1;";

    query->exec(command.arg(idConv));

    command = "SELECT Name FROM Conversations "
              "WHERE ID = %1";

    query->exec(command.arg(idConv));
    query->first();
    return query->value(0).toString();
}

QString DataBase::getFullUserList(){
    command = "SELECT ID, Nickname FROM Users";
    query->exec(command);
    query->first();

    QString packet = "";

    do
    {
        packet = query->value(0).toString() + ',' + query->value(1).toString() + '|' + packet;
    } while(query->next());

    int pos = packet.lastIndexOf('|');
    packet = packet.left(pos);
    return packet;
}

QList<int> DataBase::getWholeConvMember(QString idConv)
{
    command = "SELECT ID FROM Contacts "
              "WHERE Conversation = %1";

    query->exec(command.arg(idConv));
    query->first();

    QList<int> memberList;

    do
    {
        memberList << query->value(0).toInt();

    } while(query->next());

    return memberList;
}

void DataBase::deleteConversation(QString idConv)
{
    command = "DELETE FROM Contacts "
              "WHERE Conversation = %1;";
    query->exec(command.arg(idConv));

    command = "DELETE FROM Conversations "
              "WHERE ID = %1;";
    query->exec(command.arg(idConv));
}

int DataBase::getInterlocID(int idConv, int idPerson)
{
    command = "SELECT Dialog FROM Contacts "
              "WHERE ID = %1 AND Conversation = %2";
    query->exec(command.arg(idPerson).arg(idConv));
    query->first();
    return query->value(0).toInt();
}

QString DataBase::kickFromChat(QString idPerson, QString idConv)
{
    QString command = "UPDATE Contacts SET Left = 1 "
                      "WHERE ID = %1 "
                      "AND Conversation = %2;";
    query->exec(command.arg(idPerson).arg(idConv));

    command = "UPDATE Contacts SET Received = 0 "
              "WHERE Conversation = %1;";
    query->exec(command.arg(idConv));

    command = "SELECT ID FROM Contacts "
              "WHERE Conversation = %1 "
              "AND Left = 0;";

    query->exec(command.arg(idConv));

    if (query->first() == false) return "-1";
    else return query->value(0).toString();
}

void DataBase::changeAdmin(QString idConv, QString candidate)
{
    QString send = "UPDATE Conversations SET Admin = %1 "
                   "WHERE ID = %2;";
    query->exec(send.arg(candidate).arg(idConv));
}

QString DataBase::getIP(int idPerson)
{
    command = "SELECT IP FROM Users "
              "WHERE ID = %1 AND Online = 1";
    query->exec(command.arg(idPerson));

    if (query->first() == true) return query->value(0).toString();
    else return NULL;
}
