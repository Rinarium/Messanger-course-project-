#ifndef AUTHORIZATION_H
#define AUTHORIZATION_H

#include <QMainWindow>
#include "client.h"

namespace Ui {
class Authorization;
}

class Authorization : public QMainWindow
{
    Q_OBJECT

public:
    explicit Authorization(QWidget *parent = 0);
    ~Authorization();
    void setClient(Client &client);

private slots:
    void on_login_button_clicked();
    void on_signin_button_clicked();
    void on_cancel_button_clicked();
    void on_forgot_button_clicked();
    void on_send_button_clicked();
    void on_login_edit_textChanged(const QString &arg1);
    void on_password_edit_textChanged(const QString &arg1);
    void getSatus(int operation, int status);
    void showAuthorization(int);

private:
    Ui::Authorization *ui;
    Client *client;
    bool sigin;

};

#endif // AUTHORIZATION_H
