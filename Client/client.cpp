#include "client.h"
#include <QDebug>

Client::Client()
{
    setupUi(this);

    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
    connect(socket, SIGNAL(connected()), this, SLOT(connectUser()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(loggedOut()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
    connect(userList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    userList->setContextMenuPolicy(Qt::CustomContextMenu);

    setWindowTitle(tr("Мессенджер"));

    messagesList->setPlaceholderText("Выберите чат для начала общения");
    message->setPlaceholderText("Напишите сообщение...");
    filterChat->setPlaceholderText("Поиск по чатам...");
    filterMember->setPlaceholderText("Поиск по участнкиам...");
    sendButton->setEnabled(false);
    deleteButton->setEnabled(false);
    callButon->setEnabled(false);
    endButton->setVisible(false);
    addMemberButton->hide();

    selectedUser = new UserItem();

    call = new VoIPClient();
    call->setLocalPort(1338);
    call->setRemotePort(1339);
    connect(call, SIGNAL(endCall()), this, SLOT(endCall()));

    messageLength = (quint16) 0;
    operation = (quint16) -1;
    status = (quint16) -1;
    idCaller = -1;
    soundOff = false;

    suboperation = NULL;
    searchWindow = NULL;
}

Client::~Client()
{
    socket->disconnectFromHost();
    delete selectedUser;
    delete call;
}

void Client::on_sendButton_clicked()
{
    if (message->text().isEmpty() == true) return;
    QString packet = QString::number(currConversation->info.id)
            + ',' + QString::number(id) + "|||" + message->text().replace('|', '\\');
    sendPacket(ServerCommand::ID_SENDM, packet);

    message->clear();
    message->setFocus();
}

void Client::on_message_returnPressed()
{
    sendButton->click();
}

void Client::dataReceived()
{
    QDataStream in(socket);

    if(messageLength == 0) {
        if(socket->bytesAvailable() < (int) sizeof(quint16)) {
            return;
        }
        in >> messageLength;
    }

    if(operation == (quint16) -1) {
        if(socket->bytesAvailable() < (int) sizeof(quint16)) {
            return;
        }
        in >> operation;
    }

    if(status == (quint16) -1) {
        if(socket->bytesAvailable() < (int) sizeof(quint16)) {
            return;
        }
        in >> status;
    }

    if(socket->bytesAvailable() < messageLength) {
        return;
    }

    QString packet;
    in >> packet;

    qDebug() << messageLength << " " << operation << " " << status << " " << packet << endl;

    switch (operation)
    {
    case ID_SIGIN:
        emit statusOperation(ID_SIGIN, status);
        break;
    case ID_AUTHO:
        emit statusOperation(ID_AUTHO, status);
        if (status == 2)
        {
            this->show();
            this->setFocus();
        }
        fillList(packet);
        break;
    case ID_CHNPW:
        emit statusOperation(ID_CHNPW, status);
        break;
    case ID_GETCV:
        fillMessageArea(packet);
        break;
    case ID_SENDM:
        getMessage(packet);
        break;
    case ID_USRON:
        changeOnlineStatus(packet);
        break;
    case ID_CHGNM:
        suboperation->getStatus(status);
        changeNickname(packet, status);
        break;
    case ID_CRTCV:
        addNewConversation(packet);
        break;
    case ID_ADDMR:
        addToChat(packet);
        break;
    case ID_LFTCV:
        cleanConversation(packet);
        break;
    case ID_ALLUR:
        chooseNewConversation(packet);
        break;
    case ID_GETIP:
        startTalking(packet, status);
        break;
    case ID_INCCL:
        incomingCall(packet);
        break;
    case ID_ENDCL:
        endCall();
        break;
    default:
        break;
    }
    messageLength = (quint16) 0;
    operation = (quint16) -1;
    status = (quint16) -1;
}

void Client::connectUser()
{
    qDebug() << "Successfully connected!";
}

void Client:: logOut()
{
    operation = (quint16) ID_LGOUT;
    socket->disconnectFromHost();
}

void Client::loggedOut()
{
    this->hide();
    emit backOffline(operation);

    message->clear();
    messagesList->clear();
    userList->clear();
    conversationsList->clear();
    filterChat->clear();
    filterMember->clear();
    sendButton->setEnabled(false);
    deleteButton->setEnabled(false);
    callButon->setEnabled(false);

    operation = (quint16) -1;
    qDebug() << "Logged out.";
}

void Client::socketError(QAbstractSocket::SocketError error)
{
    switch(error) {
        case QAbstractSocket::HostNotFoundError:
            qDebug() << "Error: Could not find server.";
            break;
        case QAbstractSocket::ConnectionRefusedError:
            qDebug() << "Error: Connection refused.";
            emit statusOperation(ID_CNNCT, 0);
            break;
        case QAbstractSocket::RemoteHostClosedError:
            qDebug() << "Error: Connection closed.";
            break;
        default:
           qDebug() << "Error: " << socket->errorString();
    }
}


void Client::connectToServer(QString IP, int port) {
    qDebug() << "Connecting...";
    socket->abort();
    //Connection:
    socket->connectToHost(IP, port);
}

void Client::setLoginInfo(QString login, QString password)
{
    this->login = login;
    this->password = password;
}

void Client::sendPacket(int operation, QString buffer)
{
    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);

    out << (quint16) 0;
    out << (quint16) operation;
    out << buffer;
    out.device()->seek(0);
    out << (quint16) (packet.size() - 2*sizeof(quint16));
    socket->write(packet);
}

void Client::logIn()
{
    QString logInfo = login + '|' + password;
    this->sendPacket(ID_AUTHO, logInfo);
}

void Client::sigIn()
{
    QString logInfo = login + '|' + password;
    this->sendPacket(ID_SIGIN, logInfo);
}

void Client::changePassword()
{
    QString logInfo = login + '|' + password;
    this->sendPacket(ID_CHNPW, logInfo);
}

void Client::fillList(QString packet)
{
    if (packet.isEmpty() == true) return;

    QStringList separateUser = packet.split('|');
    QStringList separateInfo;
    User user;
    int checkRead;

    separateInfo = separateUser.value(0).split(',');
    id = separateInfo.value(0).toInt();
    nickname = separateInfo.value(1);

    if (nickname.isEmpty() == true) checkNickname();

    for (int i = 1; i < separateUser.size(); i++)
    {
        separateInfo = separateUser.value(i).split(',');
        user.id = separateInfo.value(0).toInt();
        user.nickname = separateInfo.value(1);
        checkRead = separateInfo.value(3).toInt();
        conversations << user;

        UserItem* item = new UserItem();
        conversationsList->addItem(item);
        item->info.id = user.id;
        item->info.nickname = user.nickname;
        item->adminID = separateInfo.value(2).toInt();
        item->setText(user.nickname);
        if (checkRead == 0) changeFont(ID_NEW, item);
        //conversationsList->setCurrentItem(item);
    }
}

void Client::on_conversationsList_itemClicked(QListWidgetItem *item)
{
    sendButton->setEnabled(true);
    deleteButton->setEnabled(true);

    changeFont(ID_STD, item);

    currConversation = (UserItem*) item;
    int id = currConversation->info.id;

    sendPacket(ID_GETCV, QString::number(id));
}

void Client::fillMessageArea(QString packet)
{
    QStringList separateInfo = packet.split("||");
    QStringList infoHeader = separateInfo.value(1).split('|');
    QStringList person;
    int adminID = separateInfo.value(0).toInt();
    currConversation->adminID = adminID;

    userList->clear();
    users.clear();
    callButon->show();
    if (adminID == id)
    {
        addMemberButton->show();
        filterMember->setMaximumWidth(150);
    }
    else
    {
        addMemberButton->hide();
        filterMember->setMaximumWidth(180);        
    }

    if (idCaller == currConversation->info.id)
    {
        callButon->setVisible(false);
        endButton->setVisible(true);
    }
    else
    {
        callButon->setVisible(true);
        endButton->setVisible(false);
    }

    for (int i = 0; i < infoHeader.size(); i++)
    {
        person = infoHeader.value(i).split(',');
        UserItem* item = new UserItem();
        item->info.id = person.value(0).toInt();
        item->info.nickname = person.value(1);
        users << item->info;
        if (person.value(3) == '1') continue;
        item->setText(person.value(1));
        if (item->info.id == id) item->setText(item->text() + " (you)");
        if (item->info.id == adminID)
        {
            QFont font;
            font.setWeight(80);
            item->setFont(font);
            item->setText(item->text() + " (admin)");
            callButon->hide();
        }
        if (person.value(2).toInt() == 1)
        {
            item->setForeground(Qt::darkGreen);
            callButon->setEnabled(true);
        }
        else
        {
            item->setForeground(Qt::darkRed);
            callButon->setEnabled(false);
        }
        userList->addItem(item);
    }

    userList->sortItems(Qt::AscendingOrder);
    messagesList->clear();

    QStringList messages = separateInfo.value(2).split("\n");
    QTextCursor cursor = messagesList->textCursor();
    QTextBlockFormat textBlockFormat = cursor.blockFormat();
    QString info;
    User user;
    int i = 1;

    textBlockFormat.setAlignment(Qt::AlignCenter);
    cursor.mergeBlockFormat(textBlockFormat);
    messagesList->setTextCursor(cursor);
    messagesList->setFontWeight(86);
    if (messages.value(0).at(0) == '_') messagesList->append("Начало беседы.");
    else messagesList->append("Выберите \"Вся переписка\"");
    messagesList->setFontWeight(50);
    messagesList->append("");

    for (i; i < messages.size(); i++)
    {
        separateInfo = messages.value(i).split('|');
        qDebug() << separateInfo << endl;        
        user.id = separateInfo.value(0).toInt();
        if (user.id < 0)
        {
            addInfoMessage(user.id, separateInfo.value(1));
            continue;
        }
        info = users.value(users.indexOf(user, 0)).nickname;
        info = "<b>" + info + "</b>" + " <em>" + separateInfo.value(1) + " " + separateInfo.value(2) + "</em>";
        if (user.id == id) textBlockFormat.setAlignment(Qt::AlignRight);
        else textBlockFormat.setAlignment(Qt::AlignLeft);

        cursor.mergeBlockFormat(textBlockFormat);
        messagesList->setTextCursor(cursor);

        messagesList->append(info);
        messagesList->append(separateInfo.value(3) + "\n");
    }

    if (adminID == -1 && userList->count() == 1)
    {
        UserItem* item = new UserItem();
        item->setText(currConversation->text());
        item->setForeground(Qt::darkGray);
        userList->addItem(item);
        User user;
        user.id = -1;
        user.nickname = currConversation->text();
        users << user;
        callButon->setEnabled(false);
    }
}

void Client::addInfoMessage(int operation, QString id)
{
    QTextCursor cursor = messagesList->textCursor();
    QTextBlockFormat textBlockFormat = cursor.blockFormat();
    textBlockFormat.setAlignment(Qt::AlignCenter);
    cursor.mergeBlockFormat(textBlockFormat);
    messagesList->setTextCursor(cursor);

    QString info;
    if (operation > -3)
    {
        User user;
        user.id = id.toInt();
        info = users.value(users.indexOf(user, 0)).nickname;
    }

    switch(operation)
    {
    case -1:
        info = "Пользователь " + info + " был добавлен в чат.";
        break;
    case -2:
        info = "Пользователь " + info + " больше не состоит в чате.";
        break;
    case -3:
        info = "Исходящий звонок в " + id;
        break;
    case -4:
        info = "Конец звонка в " + id;
        break;
    default:
        break;
    }

    info = "<em>" + info + "</em>";
    messagesList->append(info);
    messagesList->append("");

    cursor = messagesList->textCursor();
    textBlockFormat = cursor.blockFormat();
    textBlockFormat.setAlignment(Qt::AlignLeft);
    cursor.mergeBlockFormat(textBlockFormat);
    messagesList->setTextCursor(cursor);
}

void Client::on_filterChat_textChanged(const QString &text)
{
    QString enteredText = text.toLower();
    QListWidgetItem *item;
    QString name;

    for (int i = 0; i < conversationsList->count(); i++)
    {
        item = conversationsList->item(i);
        name = item->text().toLower();

        if (name.indexOf(enteredText) == 0)
        {
            if (item->isHidden()) item->setHidden(false);
        }
        else item->setHidden(true);
    }
}

void Client::on_filterMember_textChanged(const QString &text)
{
    QString enteredText = text.toLower();
    QListWidgetItem *item;
    QString name;

    for (int i = 0; i < userList->count(); i++)
    {
        item = userList->item(i);
        name = item->text().toLower();

        if (name.indexOf(enteredText) == 0)
        {
            if (item->isHidden()) item->setHidden(false);
        }
        else item->setHidden(true);
    }
}

void Client::getMessage(QString packet)
{
    QStringList separateInfo = packet.split("||");
    User conversation;
    conversation.id = separateInfo.value(0).toInt();
    if (conversation.id != currConversation->info.id)
    {
        int num = conversations.indexOf(conversation);
        conversation.nickname = conversations.value(num).nickname;
        QList<QListWidgetItem*> list = conversationsList->findItems(conversation.nickname, Qt::MatchExactly);
        if (list.isEmpty() == true)
        {
            QString packet = separateInfo.value(0) + '|' + separateInfo.value(2) + '|';
            conversation.nickname =  separateInfo.value(2);
            addNewConversation(packet);
            list = conversationsList->findItems(conversation.nickname, Qt::MatchExactly);
        }
        QListWidgetItem *item = list.first();
        changeFont(ID_NEW, item);
        if (soundOff == false) QSound::play("alarm_001.wav");
    }
    else
    {
        separateInfo = separateInfo.value(1).split('|');

        QTextCursor cursor = messagesList->textCursor();
        QTextBlockFormat textBlockFormat = cursor.blockFormat();
        cursor = messagesList->textCursor();
        textBlockFormat = cursor.blockFormat();
        if (id == separateInfo.value(0).toInt()) textBlockFormat.setAlignment(Qt::AlignRight);
        else textBlockFormat.setAlignment(Qt::AlignLeft);
        cursor.mergeBlockFormat(textBlockFormat);
        messagesList->setTextCursor(cursor);

        User user;
        QString info;
        user.id = separateInfo.value(0).toInt();
        if (users.indexOf(user, 0) == -1) user.id = -1;
        info = users.value(users.indexOf(user, 0)).nickname;
        info = "<b>" + info + "</b>" + " <em>" + separateInfo.value(1) + " " + separateInfo.value(2) + "</em>";
        messagesList->append(info);
        messagesList->append(separateInfo.value(3));
    }
}

void Client::changeFont(int idOperation, QListWidgetItem *item)
{
    QFont font;
    if (idOperation == ID_STD)
    {
        font.setWeight(50);
        item->setBackground(Qt::white);
    }
    else
    {
        font.setWeight(80);
        item->setBackground(Qt::cyan);
    }

    item->setFont(font);
}

void Client::changeOnlineStatus(QString idUser)
{
    User user;
    user.id = idUser.toInt();

    int num = users.indexOf(user);
    if (num == -1) return;
    if (userList->count() == 0) return;

    QListWidgetItem *item = userList->findItems(users[num].nickname, Qt::MatchStartsWith).first();

    if (item->textColor() == Qt::darkGray) return;

    if (item->textColor() == Qt::darkGreen)
    {
        item->setTextColor(Qt::darkRed);
    }
    else
    {
        item->setTextColor(Qt::darkGreen);
        callButon->setEnabled(true);
    }
}

void Client::on_exit_triggered()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Выход", "Вы уверены, что хотите выйти?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) logOut();
}

void Client::on_disableSound_triggered(bool checked)
{
    soundOff = checked;
}

void Client::on_about_triggered()
{
    QMessageBox::about(this, "О программе", "Данная программа просто супер мегакласс, как говорится, шик, блекс, бурлеск.\nПользуемся. девочки!");
}

void Client::on_changeNick_triggered()
{
    suboperation = new Interaction();
    suboperation->setModal(true);
    suboperation->chooseMode(MODE_NAME);
    connect(suboperation, SIGNAL(sendInfo(QString)), this, SLOT(getNewNickname(QString)));
    connect(suboperation, SIGNAL(rejected()), this, SLOT(checkNickname()));
    suboperation->setNickname(nickname);
    suboperation->setFixedSize(300, 100);
    suboperation->show();
}

void Client::getNewNickname(QString newNickanme)
{
    sendPacket(ID_CHGNM, newNickanme);
}

void Client::changeNickname(QString packet, int status)
{
    if (status == 0) return;
    disconnect(suboperation, SIGNAL(sendInfo(QString)), this, SLOT(getNewNickname(QString)));
    disconnect(suboperation, SIGNAL(rejected()), this, SLOT(checkNickname()));
    delete suboperation;
    suboperation = NULL;

    QStringList separateInfo = packet.split('|');
    int idNew = separateInfo.value(0).toInt();
    QString newNickname = separateInfo.value(1);

    if (idNew == id) nickname = newNickname;

    User userWithUpdate;
    userWithUpdate.id = idNew;

    int idUser = users.indexOf(userWithUpdate, 0);
    if (idUser < 0) return;

    UserItem* item;

    for (int i = 0; i < conversationsList->count(); i++)
    {
        item = (UserItem*) userList->item(i);
        if (item->info.id == idNew)
        {
            item->setText(newNickname);
            break;
        }
    }

    for (int i = 0; i < userList->count(); i++)
    {
        item = (UserItem*) userList->item(i);
        if (item->info.id == idNew)
        {
            if (item->info.id == id) newNickname += " (you)";
            if (item->info.id == currConversation->adminID) newNickname += " (admin)";
            item->setText(newNickname);
            break;
        }
    }
}

void Client::on_newConversation_triggered()
{
    suboperation = new Interaction();
    suboperation->setModal(true);
    suboperation->chooseMode(MODE_CONV);
    connect(suboperation, SIGNAL(sendInfo(QString)), this, SLOT(getConvName(QString)));
    suboperation->setFixedSize(300, 100);
    suboperation->show();
}

void Client::getConvName(QString name)
{
    QString packet = "-1|" + name;
    sendPacket(ID_CRTCV, packet);
    disconnect(suboperation, SIGNAL(sendInfo(QString)), this, SLOT(getNewNickname(QString)));
    delete suboperation;
}

void Client::addNewConversation(QString packet)
{
    QStringList separateInfo = packet.split('|');
    int idConv = separateInfo.value(0).toInt();

    QString convName;

    convName = separateInfo.value(1);

    UserItem* conversation = new UserItem();
    conversation->info.id = idConv;
    conversation->info.nickname = convName;
    conversation->setText(convName);
    conversationsList->addItem(conversation);
    conversationsList->sortItems(Qt::AscendingOrder);

    User user;
    user.id = idConv;
    user.nickname = convName;
    conversations << user;

    if (separateInfo.size() > 2) return;
    conversation->setSelected(true);
    on_conversationsList_itemClicked(conversation);
}

void Client::on_userList_itemClicked(QListWidgetItem *item)
{
    UserItem* user = (UserItem*) item;
    selectedUser->info.id = user->info.id;
    selectedUser->info.nickname = item->text();
}

void Client::on_userList_itemPressed(QListWidgetItem *item)
{
    on_userList_itemClicked(item);
}

void Client::showContextMenu(const QPoint &pos)
{
    if (userList->count() == 0) return;
    if (userList->currentRow() == -1) return;

    QPoint globalPos = userList->mapToGlobal(pos);

    QMenu menu;
    menu.addAction("Написать", this, SLOT(getSelectedToCreate()));
    if (id == currConversation->adminID) menu.addAction("Исключить",  this, SLOT(kickFromChat()));

    menu.exec(globalPos);
}

void Client::getSelectedToCreate()
{
    UserItem *user = (UserItem*) userList->selectedItems().first();
    if (user->info.id == id) return;
    createDialog(userList->selectedItems().first());
}

void Client::createDialog(QListWidgetItem *item)
{
    UserItem *user = (UserItem*) item;
    QList<QListWidgetItem*> list = conversationsList->findItems(user->text(), Qt::MatchExactly);

    if (list.empty() == true)
    {
        int idUser = user->info.id;
        QString packet = QString::number(idUser) + "|";
        sendPacket(ID_CRTCV, packet);
    }
    else
    {
        user =(UserItem*) list.first();
        user->setSelected(true);
        on_conversationsList_itemClicked(user);
    }
}

void Client::on_addMemberButton_clicked()
{
    QList<QListWidgetItem*> list;
    formListMember(MODE_OWN, NULL, list);
    searchWindow = new MemberList();
    searchWindow->setModal(true);
    connect(searchWindow, SIGNAL(transferUser(QListWidgetItem*)), this, SLOT(getUserFromSearch(QListWidgetItem*)));
    searchWindow->setMode(MODE_OWN);
    searchWindow->setList(list);
    searchWindow->fillList();
    searchWindow->show();
}

QList<QListWidgetItem*>& Client::formListMember(ServerCommand::Mode mode, QString packet, QList<QListWidgetItem*>& list)
{
     UserItem *user;

    if (mode == MODE_OWN)
    {
        for (int i = 0; i < conversationsList->count(); i++)
        {
            user = (UserItem*) conversationsList->item(i);
            if (userList->findItems(user->info.nickname, Qt::MatchExactly).isEmpty() == false) continue;
            if (userList->findItems(user->info.nickname + " (admin)", Qt::MatchExactly).isEmpty() == false) continue;
            if (user->adminID < 0) list << user;
        }
    }
    else
    {
        QStringList separateInfo = packet.split('|');
        QStringList separatePerson;
        int idUser;
        QString name;
        foreach (QString person, separateInfo)
        {
          separatePerson = person.split(',');
          idUser = separatePerson.value(0).toInt();
          name = separatePerson.value(1);

          if (idUser == id) continue;
          if (conversationsList->findItems(name, Qt::MatchExactly).isEmpty() == false) continue;

          user = new UserItem();
          user->info.id = idUser;
          user->info.nickname = name;
          user->setText(name);
          list << user;
        }
    }
    return list;
}

void Client::getUserFromSearch(QListWidgetItem *item)
{
    UserItem* user = new UserItem();
    user->copyInfo(item);
    selectedUser = user;

    if (searchWindow->getMode() == MODE_OWN) sendInfoToAdd(user);
    else createDialog(user);

    searchWindow->deleteLater();
}

void Client::sendInfoToAdd(QListWidgetItem *item)
{
    UserItem* user = (UserItem*) item;
    QString packet = QString::number(currConversation->info.id) + '|' + QString::number(user->info.id);
    sendPacket(ID_ADDMR, packet);
}

void Client::addToChat(QString packet)
{
    QStringList separateInfo = packet.split('|');
    int idConv = separateInfo.value(0).toInt();
    QString nameConv = separateInfo.value(1);

    if (conversationsList->findItems(nameConv, Qt::MatchExactly).empty() == true)
    {
        UserItem* conversation = new UserItem();
        conversation->info.id = idConv;
        conversation->info.nickname = nameConv;
        conversation->setText(nameConv);
        conversationsList->addItem(conversation);
    }
    else
    {
        if (currConversation->info.id == idConv) on_conversationsList_itemClicked(currConversation);
        else changeFont(ID_NEW, currConversation);
    }
}

void Client::on_findPeople_triggered()
{
    sendPacket(ID_ALLUR, NULL);
}

void Client::chooseNewConversation(QString packet)
{
    QList<QListWidgetItem*> list;
    formListMember(MODE_NEW, packet, list);
    searchWindow = new MemberList();
    searchWindow->setModal(true);
    connect(searchWindow, SIGNAL(transferUser(QListWidgetItem*)), this, SLOT(getUserFromSearch(QListWidgetItem*)));
    searchWindow->setMode(MODE_NEW);
    searchWindow->setList(list);
    searchWindow->fillList();
    searchWindow->show();
}

void Client::on_fullConversation_triggered()
{
    sendPacket(ID_GTFLL, QString::number(currConversation->info.id));
}

void Client::checkNickname()
{
    if (nickname.isEmpty() == true)
    {
        if (suboperation == NULL) on_changeNick_triggered();
        else suboperation->show();
    }
}

void Client::on_deleteButton_clicked()
{
    QString packet = QString::number(id) + '|' + QString::number(currConversation->info.id);
    if (currConversation->adminID == id) packet += "|0";
    sendPacket(ID_LFTCV, packet);
}

void Client::kickFromChat()
{
    QString packet = QString::number(selectedUser->info.id) + '|' + QString::number(currConversation->info.id);
    sendPacket(ID_LFTCV, packet);
}

void Client::cleanConversation(QString packet)
{
    QStringList separate = packet.split('|');
    int idPerson = separate.at(0).toInt();
    int idConv = separate.at(1).toInt();

    UserItem *conversation;
    bool check = false;
    for (int i = 0; i < conversationsList->count(); i++)
    {
        conversation = (UserItem*) conversationsList->item(i);
        if (conversation->info.id == idConv)
        {
            check = true;
            break;
        }
    }

    if (check == false) return;

    if (idPerson != id)
    {
        if (idConv == currConversation->info.id) on_conversationsList_itemClicked(currConversation);
        else changeFont(ID_NEW, conversation);
    }
    else
    {
       delete conversation;
       messagesList->clear();
       userList->clear();
       sendButton->setEnabled(false);
       deleteButton->setEnabled(false);
       callButon->setEnabled(false);
    }
}

void Client::on_callButon_clicked()
{
    UserItem* user;
    user = (UserItem*) userList->item(0);
    if (user->info.id == id) user = (UserItem*) userList->item(1);

    QString packet = QString::number(user->info.id) + '|' +QString::number(currConversation->info.id);
    sendPacket(ID_GETIP, packet);
}

void Client::startTalking(QString ip, int status)
{
    if (status == 0)
    {
        QMessageBox::about(this, "Информация", "Пользователь не онлайн!");
        return;
    }

    idCaller = currConversation->info.id;

    callButon->setVisible(false);
    endButton->setVisible(true);

    call->setRemoteHost(QHostAddress(ip));
    call->startCall();

    addInfoMessage(-3, QTime::currentTime().toString("HH:mm") + '\n');
}

void Client::on_endButton_clicked()
{
    call->quit();
    idCaller = -1;

    callButon->setVisible(true);
    endButton->setVisible(false);

    QString packet = QString::number(currConversation->info.id);
    sendPacket(ID_ENDCL, packet);

    addInfoMessage(-4, QTime::currentTime().toString("HH:mm") + '\n');
}

void Client::incomingCall(QString message)
{
    QStringList separate = message.split('|');
    QString ip = separate.at(0);
    QString idConv = separate.at(1);
    QString nickname = separate.at(2);

    if (soundOff == false) QSound::play("alarm_001.wav");

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Исходящий звонок от", "Вам звонит " + nickname + ".\nВы хотите ответить?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        if (idCaller == -1) idCaller = idConv.toInt();
        else
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Внимание!", "Вы хотите завершить текущий разговор?",
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes)
            {
                call->finishCall();
                sendPacket(ID_ENDCL, QString::number(idCaller));
                idCaller = idConv.toInt();
            }
            else
            {
                sendPacket(ID_ENDCL, idConv);
            }

        }

        if (idCaller == currConversation->info.id)
        {
            callButon->setVisible(false);
            endButton->setVisible(true);

            addInfoMessage(-3, QTime::currentTime().toString("HH:mm") + '\n');
        }

        call->setRemoteHost(QHostAddress(ip));
        call->startCall();
    }
    else
    {
        sendPacket(ID_ENDCL, idConv);
    }
}

void Client::endCall()
{
    call->quit();

    if (idCaller == currConversation->info.id)
    {
        callButon->setVisible(true);
        endButton->setVisible(false);

        on_conversationsList_itemClicked(currConversation);
    }

    idCaller = -1;
}

void Client::on_changePorts_triggered()
{
    bool buttonOk;
    QString localPort = QInputDialog::getText( 0,
                                         "Ваш порт",
                                         "Порт:",
                                         QLineEdit::Normal,
                                         "",
                                         &buttonOk
                                        );
    if (!buttonOk) return;
    call->setLocalPort(localPort.toInt());
    QString remotePort = QInputDialog::getText( 0,
                                         "Порт собеседника",
                                         "Порт:",
                                         QLineEdit::Normal,
                                         "",
                                         &buttonOk
                                        );
    if (!buttonOk) return;
    call->setRemotePort(remotePort.toInt());
}
