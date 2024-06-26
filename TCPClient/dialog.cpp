#include "dialog.h"
#include "./ui_dialog.h"
#include <cstdlib>

#include <qlogging.h>

Dialog::Dialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::Dialog), file(new QFile) {
  ui->setupUi(this);

  file->setFileName("history.txt");
  out.setDevice(file);
  socket = new QTcpSocket(this);

  QString history;
  if (!file->open(QIODevice::ReadWrite | QIODevice::Text)) {
    history = "Unknonw error caused history couldn't be loaded\n";
  } else {
    QTextStream stream(file);
    history = stream.readAll();
    history.append("------------History-----------\n");
  }
  connect(this, &Dialog::newMessage, this, &Dialog::displayMessage);
  connect(socket, &QTcpSocket::readyRead, this, &Dialog::readSocket);
  connect(socket, &QTcpSocket::disconnected, this, &Dialog::discardSocket);
  connect(socket, &QAbstractSocket::errorOccurred, this, &Dialog::displayError);

  connect(socket, &QTcpSocket::connected, this, [&]() -> void {
    emit newMessage(history);
    qDebug() << "Local IP Address" << socket->localAddress().toString()
             << " local port: " << socket->localPort();
    qDebug() << "Peer IP Address" << socket->peerAddress().toString()
             << " peer port: " << socket->peerPort();
  });

  socket->connectToHost(QHostAddress::LocalHost, 8888);

  if (!socket->waitForConnected()) {
    QMessageBox::critical(
        this, "TCPClient",
        QString("The following error occured: %1.").arg(socket->errorString()));
    exit(EXIT_FAILURE);
  }
}

Dialog::~Dialog() {
  if (socket->isOpen()) {
    socket->close();
  }

  delete file;
  delete ui;
}

void Dialog::readSocket() {
  QByteArray buffer;

  QDataStream socketStream(socket);
  socketStream.setVersion(QDataStream::Qt_6_8);

  socketStream.startTransaction();
  socketStream >> buffer;

  if (!socketStream.commitTransaction()) {
    QString message = QString("Waiting ...");
    out << message;
    emit newMessage(message);
    return;
  }
  QString message = QString("(Server)%1:%2 ::\n%3")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort())
                        .arg(QString::fromStdString(buffer.toStdString()));

  out << message << "\n";
  emit newMessage(message);
}

void Dialog::discardSocket() {
  socket->deleteLater();
  socket = nullptr;
}

void Dialog::displayError(QAbstractSocket::SocketError socketError) {
  switch (socketError) {
  case QAbstractSocket::HostNotFoundError:
    QMessageBox::information(this, "TCPClient", "The host was not found.");
    break;
  case QAbstractSocket::ConnectionRefusedError:
    QMessageBox::information(this, "TCPClient",
                             "The connection was refused by the peer. ");
    break;
  default:
    QMessageBox::information(this, "QTCPClient",
                             QString("The following error occurred: %1.")
                                 .arg(socket->errorString()));
    break;
  }
}

void Dialog::on_pushButton_sendMessage_clicked() {
  if (socket) {
    if (socket->isOpen()) {
      QString str = ui->textEdit_message->toPlainText();

      QDataStream socketStream(socket);
      socketStream.setVersion(QDataStream::Qt_6_8);

      QByteArray byteArray = str.toUtf8();
      QString myMessage = "(me)";
      myMessage.append(socket->localAddress().toString() + ":" +
                       QString::number(socket->localPort()) + " ::\n");
      myMessage.append(str);

      ui->textBrowser_receivedMessages->append(myMessage);
      socketStream << byteArray;

      out << myMessage << "\n";
      ui->textEdit_message->clear();
    } else {
      QMessageBox::critical(this, "TCPClient",
                            "Socket doesn't seem to be opened");
    }
  } else {
    QMessageBox::critical(this, "TCPClient", "Not connected");
  }
}

void Dialog::displayMessage(const QString &str) {
  ui->textBrowser_receivedMessages->append(str);
}
