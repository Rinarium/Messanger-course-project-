#ifndef MEMBERLIST_H
#define MEMBERLIST_H

#include <QDialog>
#include <QDebug>
#include <QListWidget>
#include <QKeyEvent>
#include "useritem.h"
#include "servercommand.h"

using namespace ServerCommand;

namespace Ui {
class MemberList;
}

class MemberList : public QDialog
{
    Q_OBJECT

public:
    explicit MemberList(QWidget *parent = 0);
    ~MemberList();
    void setList(QList<QListWidgetItem*>& list);
    void fillList();
    void showList();
    void setMode(int mode);
    int getMode();

private slots:
    void on_CancelButton_clicked();
    void on_nameEdit_textChanged(const QString &arg1);
    void on_chooseButton_clicked();
    //void closeEvent(QCloseEvent *event);

signals:
    void transferUser(QListWidgetItem*);
    //void dialogClosed();

private:
    Ui::MemberList *ui;
    QList<QListWidgetItem*> memlist;
    int mode;
};

#endif // MEMBERLIST_H
