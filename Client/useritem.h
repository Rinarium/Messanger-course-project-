#ifndef USERITEM_H
#define USERITEM_H

#include <QWidget>
#include <QListWidgetItem>

struct User
{
    int id;
    QString nickname;

    friend bool operator == (const User &first, const User &second);
};

class UserItem : public QListWidgetItem
{
public:
    explicit UserItem(QListWidget *parent = 0);
    ~UserItem();
    User info;
   // int id;
    int adminID;
    void copyInfo(QListWidgetItem *item);
};


#endif // USERITEM_H
