#include "memberlist.h"
#include "ui_memberlist.h"

MemberList::MemberList(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MemberList)
{
    ui->setupUi(this);
    this->setFixedSize(230, 300);
    ui->nameEdit->setPlaceholderText("Напишите имя...");

    setWindowTitle(tr("Собеседники"));
}

MemberList::~MemberList()
{
    delete ui;
}

void MemberList::on_CancelButton_clicked()
{
    this->hide();
}

void MemberList::setList(QList<QListWidgetItem*> &list)
{
    UserItem *user, *temp;
    foreach (QListWidgetItem *item, list)
    {
        user = new UserItem();
        temp = (UserItem *) item;
        user->setText(temp->text());
        user->info.id = temp->info.id;
        user->info.nickname = temp->info.nickname;
        memlist << user;
    }
}

void MemberList::fillList()
{
     qDebug() << memlist.size() << endl;
    foreach (QListWidgetItem *item, memlist)
    {
        ui->userList->addItem(item);
    }
}

void MemberList::showList()
{
     UserItem* user;
    foreach (QListWidgetItem *item, memlist)
    {
        user = (UserItem*) item;
        qDebug() << user->adminID << " " << user->info.id << " " << user->text()  << endl;
    }
}

void MemberList::on_nameEdit_textChanged(const QString &text)
{
    QString enteredText = text.toLower();
    QListWidgetItem *item;
    QString name;

    for (int i = 0; i < ui->userList->count(); i++)
    {
        item = ui->userList->item(i);
        name = item->text().toLower();

        if (name.indexOf(enteredText) == 0)
        {
            if (item->isHidden()) item->setHidden(false);
        }
        else item->setHidden(true);
    }
}

void MemberList::on_chooseButton_clicked()
{
    if (ui->userList->selectedItems().size() != 1) return;
    emit transferUser(ui->userList->selectedItems().first());
}

void MemberList::setMode(int mode)
{
    this->mode = mode;
}

int MemberList::getMode()
{
    return this->mode;
}
