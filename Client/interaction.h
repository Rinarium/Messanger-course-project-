#ifndef INTERACTION_H
#define INTERACTION_H

#include <QDialog>
#include <QMessageBox>

#define MODE_CONV 0
#define MODE_NAME 1

namespace Ui {
class Interaction;
}

class Interaction : public QDialog
{
    Q_OBJECT

public:
    explicit Interaction(QWidget *parent = 0);
    ~Interaction();
    void setNickname(QString nickname);
    void getStatus(int status);
    void chooseMode(int mode);

private slots:
    void on_okayButton_clicked();
    void on_cancelButton_clicked();
    void on_nameEdit_textChanged(const QString &arg1);

signals:
    void sendInfo(QString nickname);

private:
    int mode;
    Ui::Interaction *ui;
};

#endif // INTERACTION_H
