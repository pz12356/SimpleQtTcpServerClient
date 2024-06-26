#ifndef DIALOG_H
#define DIALOG_H

#include <QAbstractSocket>
#include <QDialog>
#include <QHostAddress>
#include <QMessageBox>
#include <QString>
#include <QTcpSocket>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui {
class Dialog;
}
QT_END_NAMESPACE

class Dialog : public QDialog {
  Q_OBJECT

public:
  Dialog(QWidget *parent = nullptr);
  ~Dialog();

signals:
  void newMessage(QString);
private slots:
  void readSocket();
  void discardSocket();
  void displayError(QAbstractSocket::SocketError socketError);

  void displayMessage(const QString &str);
  void on_pushButton_sendMessage_clicked();

private:
  Ui::Dialog *ui;
  QTcpSocket *socket;

  QFile* file;
  QTextStream out;
};
#endif // DIALOG_H
