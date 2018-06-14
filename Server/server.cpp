#include "server.h"

Server::Server()
{
    serverState = new QLabel;
    quitButton = new QPushButton(tr("Выход"));
    connect(quitButton, SIGNAL(clicked()), qApp, SLOT(quit()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(serverState);
    layout->addWidget(quitButton);
    this->setLayout(layout);

    setWindowTitle(tr("Сервер"));

    server = new QTcpServer(this);
    if(!server->listen(QHostAddress::Any, 1337)) {
        serverState->setText(tr("Сервер не может запуститься: ") + server->errorString());
    } else {
        serverState->setText(tr("Сервер запущен. Порт: ") + QString::number(server->serverPort()));
        connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
    }
    messageLength = 0;
    currClient = nullptr;

    database = new DataBase();
}

Server::~Server()
{
    delete database;
}

void Server::newConnection()
{
    User* newUser = new User();
    newUser->setId(-1);
    newUser->setSocket(server->nextPendingConnection());
    users << *newUser;

    connect(newUser->getSocket(), SIGNAL(readyRead()), this, SLOT(dataReceived()));
    connect(newUser->getSocket(), SIGNAL(disconnected()), this, SLOT(logoutUser()));
}

void Server::checkIP()
{
    QString IP = currClient->getSocket()->peerAddress().toString().split(':').last();

    database->setIP(IP, currClient->getId());
}

void Server::dataReceived()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (socket == 0) return;

    QDataStream in(socket);
    if (messageLength == 0)
    {
        if(socket->bytesAvailable() < (int)sizeof(quint16)) return;
        in >> messageLength;
    }

    if (socket->bytesAvailable() < (int)sizeof(quint16)) return;

    in >> operation;

    if (socket->bytesAvailable() < messageLength) return;

    QString message;
    in >> message;

    User user;
    user.setSocket(socket);
    int numUser = users.indexOf(user);
    currClient = &users[numUser];

    qDebug() << messageLength << " " << operation << " " << message;
    messageLength = 0;

    switch(operation)
    {
    case ID_SIGIN:
        sigin(message);
        break;
    case ID_AUTHO:
        login(message);
        break;
    case ID_CHNPW:
        changePassword(message);
        break;
    case ID_GETCV:
        sendConversation(message, ID_GETCV);
        break;
    case ID_GTFLL:
        sendConversation(message, ID_GTFLL);
        break;
    case ID_SENDM:
        sendMessage(message);
        break;
    case ID_CHGNM:
        changeNickname(message);
        break;
    case ID_CRTCV:
        createConversation(message);
        break;
    case ID_ADDMR:
        addToChat(message);
        break;
    case ID_LFTCV:
        kickFromChat(message);
        break;
    case ID_ALLUR:
        sendFullUserList();
        break;
    case ID_GETIP:
        sendIP(message);
        break;
    case ID_ENDCL:
        endCall(message);
        break;
    default:
        break;
    }
}

void Server::logoutUser()
{
    User user;
    user.setSocket(qobject_cast<QTcpSocket *>(sender()));
    if(user.getSocket() == 0) {
        return;
    }

    int idUser = users.indexOf(user);
    idUser = users[idUser].getId();

    users.removeOne(user);
    user.getSocket()->deleteLater();

    database->makeOffline(idUser);

    QList<int> contactList = database->getContactList(idUser);

    User temp = *currClient;
    QString id = QString::number(idUser);

    foreach(int idContact, contactList)
    {
        user.setId(idContact);
        idUser = users.indexOf(user, 0);
        if (idUser < 0) continue;
        currClient = &users[idUser];

        sendPacket(ID_USRON, 0, id);
    }

    currClient = &temp;
}

void Server::sendMessage(const QString &mess)
{
    QStringList separate = mess.split("|||");
    QStringList header = separate.value(0).split(',');

    QString path = "Conversations/" + header.value(0) + ".txt";
    QFile file(path);
    file.open(QFile::ReadWrite | QFile::Text);
    QTextStream out(&file);
    if (!file.isOpen())
    {
        sendPacket(ID_SENDM, 0, NULL);
        return;
    }

    out.seek(file.size());
    QString info = header.value(1) + '|' + QDate::currentDate().toString("dd.MM.yy")
            + '|' + QTime::currentTime().toString("HH:mm") + '|' + separate.value(1) + '\n';
    out << info;
    file.close();

    database->updateReceived(header.value(0), header.value(1));
    QString addInfo = database->checkNewDialog(header.value(0));
    if (addInfo.isEmpty() == false) addInfo = "||" + addInfo;
    QList<int> memberList = database->getMemberList(header.value(0));

    QString packet = header.value(0) + "||" + info + addInfo;

    User user;
    int idUser;

    foreach(int idContact, memberList)
    {
        user.setId(idContact);
        idUser = users.indexOf(user, 0);
        if (idUser < 0) continue;
        currClient = &users[idUser];
        sendPacket(ID_SENDM, 1, packet);
    }
}

void Server::login(QString buffer)
{
    QStringList separate = buffer.split('|');
    QString login = separate.at(0);
    QString password = separate.at(1);

    QString message = "";
    int status = database->checkPasswotd(login, password);
    int ID;

    if (status == 2)
    {
        ID = database->getID(login);
        message = database->formConversations(QString::number(ID));
        currClient->setId(ID);
        checkIP();
    }

    qDebug() << "Mess: " << message << endl;
    sendPacket(ID_AUTHO, status, message);

    if (status < 2) return;

    database->makeOnline(ID);

    QList<int> contactList = database->getContactList(ID);

    User user;
    int idUser;
    QString id = QString::number(currClient->getId());

    foreach(int idContact, contactList)
    {
        user.setId(idContact);
        idUser = users.indexOf(user, 0);
        if (idUser < 0) continue;
        currClient = &users[idUser];

        sendPacket(ID_USRON, 1, id);
    }
}

void Server::sigin(QString buffer)
{
    QStringList separate = buffer.split('|');
    QString login = separate.at(0);
    QString password = separate.at(1);

    int status = database->sigin(login, password);

    sendPacket(ID_SIGIN, status, NULL);
}

void Server::changePassword(QString buffer)
{
    QStringList separate = buffer.split('|');
    QString login = separate.at(0);
    QString password = separate.at(1);

    int status = database->changePassword(login, password);

    sendPacket(ID_CHNPW, status, NULL);
}

void Server::sendPacket(int operation, int status, QString buffer)
{
    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);

    out << (quint16) 0;
    out << (quint16) operation;
    out << (quint16) status;
    out << buffer;
    out.device()->seek(0);
    out << (quint16) (packet.size() - 3*sizeof(quint16));
    currClient->getSocket()->write(packet);
}

void Server::sendConversation(QString id, int type)
{
    database->setReceived(id, currClient->getId());

    QString path = "Conversations/" + id + ".txt";
    QFile file(path);
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream in(&file);
    if (!file.isOpen())
    {
        sendPacket(ID_GETCV, 0, NULL);
        return;
    }

    if (file.size() > 2048 && type == ID_GETCV)
    {
        in.seek(file.size() - 2000);
        in.readLine();
    }

    QString messages = "";

    while (!in.atEnd())
    {
        messages = messages + in.readLine() + "\n";
    }

    file.close();

    int pos = messages.lastIndexOf('\n');
    messages = messages.left(pos);

    messages = database->formMembers(id) + "||" + messages;

    qDebug() << messages << endl;
    sendPacket(ID_GETCV, 1, messages);
}

void Server::changeNickname(QString name)
{
    int status = database->changeNickname(name, currClient->getId());

    if (status == 0)
    {
         sendPacket(ID_CHGNM, 0, NULL);
         return;
    }

    QSet<int> idSet;
    idSet << currClient->getId();

    idSet + database->getRelationList(currClient->getId());

    User user;
    int idUser;
    QString packet = QString::number(currClient->getId()) + '|' + name;

    foreach (int id, idSet) {
        user.setId(id);
        idUser = users.indexOf(user, 0);
        if (idUser < 0) continue;
        qDebug() << id << " l" << endl;
        currClient = &users[idUser];
        sendPacket(ID_CHGNM, 1, packet);
    }
}

void Server::createConversation(QString message)
{
    QStringList separate = message.split('|');
    int id = separate.at(0).toInt();
    QString name = separate.at(1);

    if (checkRestoreChat(id) == true) return;

    QString packet = database->createConversation(name, id, currClient->getId());

    QString idConv = database->getLastConversation();;

    QFile file("Conversations/" + idConv + ".txt");
    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);
    out << "_\n";
    file.close();

    sendPacket(ID_CRTCV, 1, packet);
}

bool Server::checkRestoreChat(int id)
{
    QString packet = database->checkRestore(id, currClient->getId());

    if (packet.isEmpty() == true) return false;

    QFile file("Conversations/" + packet + ".txt");
    file.open(QFile::ReadWrite | QFile::Text);
    QTextStream out(&file);
    out.seek(file.size());
    out << "-1|" + QString::number(currClient->getId()) + "\n";
    file.close();

    database->returnToConv(id, currClient->getId());

    packet = packet + '|' + database->getNickname(id);

    sendPacket(ID_CRTCV, 1, packet);
    return true;
}

void Server::addToChat(QString message)
{
    QStringList separate = message.split('|');
    QString idConv = separate.at(0);
    QString idPersonChat = separate.at(1);

    QString idPerson = database->getID(idPersonChat, currClient->getId());

    QString packet = idConv + '|' + database->addToChat(idPerson, idConv);

    QString path = "Conversations/" + idConv + ".txt";
    QFile file(path);
    file.open(QFile::ReadWrite | QFile::Text);
    QTextStream out(&file);

    out.seek(file.size());
    out << "-1|" + idPerson + '\n';

    file.close();

    QList<int> memberList = database->getMemberList(idConv);

    User user;
    int idUser;

    foreach(int idContact, memberList)
    {
        user.setId(idContact);
        idUser = users.indexOf(user, 0);
        if (idUser < 0) continue;
        currClient = &users[idUser];
        sendPacket(ID_ADDMR, 0, packet);
    }
}

void Server::sendFullUserList()
{
    QString packet = database->getFullUserList();
    sendPacket(ID_ALLUR, 1, packet);
}

void Server::kickFromChat(QString message)
{
    QStringList separate = message.split('|');
    QString idPerson = separate.at(0);
    QString idConv = separate.at(1);

    QString idCandidate = database->kickFromChat(idPerson, idConv);

    QString path = "Conversations/" + idConv + ".txt";
    QFile file(path);
    int status;

    if (idCandidate == "-1")
    {
        file.remove();
        status = 0;
    }
    else
    {
        file.open(QFile::ReadWrite | QFile::Text);
        QTextStream out(&file);

        out.seek(file.size());
        out << "-2|" + idPerson + '\n';

        file.close();
        if (separate.size() > 2) database->changeAdmin(idConv, idCandidate);
        status = 1;
    }

    QList<int> memberList = database->getWholeConvMember(idConv);

    User user;
    int idUser;

    foreach(int idContact, memberList)
    {
        user.setId(idContact);
        idUser = users.indexOf(user, 0);
        if (idUser < 0) continue;
        currClient = &users[idUser];
        sendPacket(ID_LFTCV, status, message);
    }

    if (status == 0) database->deleteConversation(idConv);
}

void Server::sendIP(QString ids)
{
    QStringList separate = ids.split('|');
    QString idPerson = separate.at(0);
    QString idConv = separate.at(1);

    QString IP = database->getIP(idPerson.toInt());
    QString nickname, packet;
    int status;

    if (IP.isEmpty() == false)
    {
        status = 1;
        packet = IP;
        nickname = database->getNickname(currClient->getId());
    }
    else
    {
        status = 0;
        packet = "";
    }

    sendPacket(ID_GETIP, status, packet);

    if (status == 0) return;

    IP = database->getIP(currClient->getId());

    packet = IP + '|' + idConv + '|' + nickname;

    User user;
    user.setId(idPerson.toInt());
    int idUser = users.indexOf(user, 0);
    currClient = &users[idUser];
    sendPacket(ID_INCCL, status, packet);

    writeCallInfo(idConv, 0);
}

void Server::endCall(QString idConv)
{
    int idPerson = database->getInterlocID(idConv.toInt(), currClient->getId());

    User user;
    user.setId(idPerson);
    int idUser = users.indexOf(user, 0);
    currClient = &users[idUser];
    sendPacket(ID_ENDCL, 1, NULL);

    writeCallInfo(idConv, 1);
}

void Server::writeCallInfo(QString idConv, int operation)
{
    QString path = "Conversations/" + idConv + ".txt";
    QFile file(path);
    file.open(QFile::ReadWrite | QFile::Text);
    QTextStream out(&file);

    out.seek(file.size());

    QString message;

    if (operation == 0) message = "-3";
    else message = "-4";
    message += "|" + QTime::currentTime().toString("HH:mm") + '\n';

    out << message;

    file.close();
}
