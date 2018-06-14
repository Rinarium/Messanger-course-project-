#include "interaction.h"
#include "ui_interaction.h"

Interaction::Interaction(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Interaction)
{
    ui->setupUi(this);
    ui->nameEdit->setMaxLength(10);

    setWindowTitle(tr("Форма"));
}

Interaction::~Interaction()
{
    delete ui;
}

void Interaction::on_okayButton_clicked()
{
    if (ui->nameEdit->text().isEmpty() == true)
    {
        QMessageBox::about(this, "Внимание", "Заполните поле!");
        return;
    }
    emit sendInfo(ui->nameEdit->text());
}

void Interaction::on_cancelButton_clicked()
{
    this->reject();
}

void Interaction::setNickname(QString nickname)
{
    ui->nameEdit->setText(nickname);
}

void Interaction::getStatus(int status)
{
    if (status == 1)
    {
        this->on_cancelButton_clicked();
    }
    else
    {
        QMessageBox::about(this, "Внимание", "Такой никнейм уже занят!");
    }
}

void Interaction::on_nameEdit_textChanged(const QString &arg1)
{

}

void Interaction::chooseMode(int mode)
{
    this->mode = mode;
    if (mode == MODE_NAME)
    {
        ui->nameLabel->setText("Никнейм");
        ui->okayButton->setText("Изменить");
    }
    else
    {
        ui->nameLabel->setText("Название");
        ui->okayButton->setText("Создать");
    }
}
