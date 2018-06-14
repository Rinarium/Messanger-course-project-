#include "useritem.h"

UserItem::UserItem(QListWidget *parent) :
    QListWidgetItem(parent, 0)
{

}

UserItem::~UserItem()
{

}

void UserItem::copyInfo(QListWidgetItem *item)
{
    UserItem* user = (UserItem*) item;
    this->info.id = user->info.id;
    this->info.nickname = user->info.nickname;
    this->setText(user->text());
}

bool operator == (const User &first, const User &second)
{
    if (first.id != second.id) return false;
    //if (first.nickname.compare(second.nickname, Qt::CaseSensitive) == 0) return false;
    return true;
}
