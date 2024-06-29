#ifndef DIALOG_H
#define DIALOG_H

#include <QMessageBox>
#include <QSet>
#include <QTcpServer>
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
  explicit Dialog(QWidget *parent = nullptr);
  ~Dialog() override;

signals:
  void newMessage(QString);

private slots:
  void newConnection();
  void appendToSocketList(QTcpSocket *socket);

  void readSocket();
  void discardSocket();

  void displayMessage(const QString &str) const;
  void sendMessage(QTcpSocket *socket);

  void on_pushButton_sendMessage_clicked();

  void refreshComboBox();

private:
  Ui::Dialog *ui;

  QTcpServer *m_server;
  QSet<QTcpSocket *> connection_set;

  QFile* file;
  QTextStream out;
};
#endif // DIALOG_H
