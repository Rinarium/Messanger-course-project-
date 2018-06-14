#include "authorization.h"
#include "ui_authorization.h"

Authorization::Authorization(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Authorization)
{
    ui->setupUi(this);

    ui->password_edit->setEchoMode(QLineEdit::Password);
    ui->repeat_edit->setEchoMode(QLineEdit::Password);

    setWindowTitle(tr("Авторизация"));
    QPixmap photo;
    photo.load("logo.gif", 0, Qt::AutoColor);
    photo = photo.scaled(110, 110, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    ui->logo->setPixmap(photo);

    ui->cancel_button->setHidden(true);
    ui->send_button->setHidden(true);
    ui->repeat_edit->setHidden(true);
    ui->repeat_lbl->setHidden(true);
    ui->info_lbl->setHidden(true);

    ui->login_button->setEnabled(false);
}

Authorization::~Authorization()
{
    delete ui;
}

void Authorization::on_login_button_clicked()
{
    client->setLoginInfo(ui->login_edit->text(), ui->password_edit->text());
    this->client->connectToServer("127.0.0.1", 1337);
    client->logIn();
}

void Authorization::setClient(Client &client)
{
    this->client = &client;
    connect(&client, SIGNAL(statusOperation(int,int)), this, SLOT(getSatus(int, int)));
    connect(&client, SIGNAL(backOffline(int)), this, SLOT(showAuthorization(int)));
}

void Authorization::on_signin_button_clicked()
{
    ui->cancel_button->setHidden(false);
    ui->send_button->setHidden(false);
    ui->repeat_edit->setHidden(false);
    ui->repeat_lbl->setHidden(false);
    ui->info_lbl->setHidden(false);
    ui->signin_button->setHidden(true);
    ui->login_button->setHidden(true);
    ui->forgot_button->setHidden(true);
    ui->status_lbl->setHidden(true);

    ui->login_edit->clear();
    ui->password_edit->clear();
    ui->repeat_edit->clear();
    ui->invite_lbl->setText("Заполните все поля для регистрации:");

    sigin = true;
}

void Authorization::on_forgot_button_clicked()
{
    ui->cancel_button->setHidden(false);
    ui->send_button->setHidden(false);
    ui->repeat_edit->setHidden(false);
    ui->repeat_lbl->setHidden(false);
    ui->info_lbl->setHidden(false);
    ui->signin_button->setHidden(true);
    ui->login_button->setHidden(true);
    ui->forgot_button->setHidden(true);
    ui->status_lbl->setHidden(true);

    ui->login_edit->clear();
    ui->password_edit->clear();
    ui->repeat_edit->clear();
    ui->invite_lbl->setText("Введите свой логин и новый пароль:");

    sigin = false;
}

void Authorization::on_cancel_button_clicked()
{
    ui->cancel_button->setHidden(true);
    ui->send_button->setHidden(true);
    ui->repeat_edit->setHidden(true);
    ui->repeat_lbl->setHidden(true);
    ui->info_lbl->setHidden(true);
    ui->signin_button->setHidden(false);
    ui->login_button->setHidden(false);
    ui->forgot_button->setHidden(false);
    ui->status_lbl->setHidden(false);

    ui->login_edit->clear();
    ui->password_edit->clear();
    ui->info_lbl->clear();
    ui->status_lbl->clear();
    ui->invite_lbl->setText("Для авторизации введите логин и пароль:");


}

void Authorization::on_send_button_clicked()
{
    int count = 0;
    ui->info_lbl->clear();

    if (ui->login_edit->text().isEmpty()) count++;
    if (ui->password_edit->text().isEmpty()) count++;
    if (ui->repeat_edit->text().isEmpty()) count++;
    if (count != 0)
    {
        ui->info_lbl->setText("Не все поля заполнены!");
        return;
    }

    if (ui->password_edit->text().compare(ui->repeat_edit->text()))
    {
        ui->info_lbl->setText("Введенные пароли не совпадают!");
        return;
    }

    client->setLoginInfo(ui->login_edit->text(), ui->password_edit->text());

    client->connectToServer("127.0.0.1"/*"172.23.170.209"*//*"192.168.1.21"*/, 1337);
    if (sigin == true) client->sigIn();
    else client->changePassword();
}

void Authorization::getSatus(int operation, int status)
{
    switch (operation) {
    case ID_SIGIN:
        if (status == 1) ui->info_lbl->setText("Вы успешно зарегистрированы.");
        else ui->info_lbl->setText("Такой логин уже занят!");
        break;
    case ID_AUTHO:
        if (status == 2)
        {
            ui->status_lbl->setText("Добро пожаловать в чат!");
            this->hide();
        }
        else if (status == 1) ui->status_lbl->setText("Этот пользователь уже онлайн!");
        else ui->status_lbl->setText("Неверный логин или пароль!");
        break;
    case ID_CHNPW:
        if (status == 1) ui->info_lbl->setText("Пароль успешно изменен.");
        else ui->info_lbl->setText("Некорректный логин!");
    case ID_CNNCT:
        ui->status_lbl->setText("Не удалось подключиться к серверу!");
        break;
    }
}

void Authorization::on_login_edit_textChanged(const QString &arg1)
{
    if (ui->login_edit->text().isEmpty() || ui->password_edit->text().isEmpty())
        ui->login_button->setEnabled(false);
    else ui->login_button->setEnabled(true);
}

void Authorization::on_password_edit_textChanged(const QString &arg1)
{
    if (ui->login_edit->text().isEmpty() || ui->password_edit->text().isEmpty())
        ui->login_button->setEnabled(false);
    else ui->login_button->setEnabled(true);
}

void Authorization::showAuthorization(int id)
{
    if (this->isVisible() == true) return;
    ui->login_edit->clear();
    ui->password_edit->clear();
    if (id == ID_LGOUT) ui->status_lbl->clear();
    else ui->status_lbl->setText("Потеряно соединение с сервером!\nПовторите попытку.");
    this->show();
    this->setFocus();
}
