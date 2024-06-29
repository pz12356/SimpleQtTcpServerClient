#include "dialog.h"
#include "./ui_dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::Dialog), file(new QFile) {
  ui->setupUi(this);
  m_server = new QTcpServer();

  QString history;
  file->setFileName("history.txt");

  out.setDevice(file);

  if (!file->open(QIODevice::ReadWrite | QIODevice::Text)) {
    history = "Unknonw error caused history couldn't be loaded\n";
  } else {
    QTextStream stream(file);
    history = stream.readAll();
    history.append("------------History-----------\n");
  }

  if (m_server->listen(QHostAddress::LocalHost, 8888)) {
    connect(this, &Dialog::newMessage, this, &Dialog::displayMessage);
    connect(m_server, &QTcpServer::newConnection, this, &Dialog::newConnection);

    emit newMessage(history);
  } else {
    QMessageBox::critical(this, "TCPServer",
                          QString("Unable to start the server: %1.")
                              .arg(m_server->errorString()));
    exit(EXIT_FAILURE);
  }
}

Dialog::~Dialog() {
  for (QTcpSocket *socket : connection_set) {
    socket->close();
    socket->deleteLater();
  }

  m_server->close();
  m_server->deleteLater();

  delete file;
  delete ui;
}

void Dialog::newConnection() {
  while (m_server->hasPendingConnections())
    appendToSocketList(m_server->nextPendingConnection());
}

void Dialog::appendToSocketList(QTcpSocket *socket) {
  connection_set.insert(socket);
  connect(socket, &QTcpSocket::readyRead, this, &Dialog::readSocket);
  connect(socket, &QTcpSocket::disconnected, this, &Dialog::discardSocket);
  ui->comboBox_receiver->addItem(socket->peerAddress().toString() + ":" +
                                 QString::number(socket->peerPort()));

  QString message = QString("(System) Client %1:%2 has just entered the room")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort());

  out << message << "\n";
  displayMessage(message);
}

void Dialog::readSocket() {
  auto *socket = qobject_cast<QTcpSocket *>(sender());

  QByteArray buffer;

  QDataStream socketStream(socket);
  socketStream.setVersion(QDataStream::Qt_6_8);

  socketStream.startTransaction();
  socketStream >> buffer;

  if (!socketStream.commitTransaction()) {
    QString message =
        QString("%1 :: Waiting ..").arg(socket->socketDescriptor());
    out << message;
    emit newMessage(message);
    return;
  }

  QString message = QString("(Client)%1:%2 ::\n%3")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort())
                        .arg(QString::fromStdString(buffer.toStdString()));

  out << message << "\n";
  emit newMessage(message);
}

void Dialog::discardSocket() {
  auto *socket = qobject_cast<QTcpSocket *>(sender());
  if (connection_set.contains(socket)) {
    QString message = QString("(System) A client %1 has just left the room")
                          .arg(socket->peerAddress().toString() + ":" +
                               QString::number(socket->peerPort()));

    displayMessage(message);
    out << message << "\n";
    connection_set.remove(socket);
  }
  refreshComboBox();
  socket->deleteLater();
}

void Dialog::on_pushButton_sendMessage_clicked() {
  QString receiver = ui->comboBox_receiver->currentText();
  if (receiver == "Broadcast") {
    for (QTcpSocket *socket : connection_set) {
      sendMessage(socket);
    }
  } else {
    for (QTcpSocket *socket : connection_set) {
      if (socket->peerAddress().toString() + ":" +
              QString::number(socket->peerPort()) ==
          receiver) {
        sendMessage(socket);
        break;
      }
    }
  }
  ui->textEdit_message->clear();
}

void Dialog::sendMessage(QTcpSocket *socket) {
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
      socketStream.setVersion(QDataStream::Qt_6_8);
      socketStream << byteArray;

      out << myMessage << "\n";
    } else {
      QMessageBox::critical(this, "QTCPServer",
                            "Socket doesn't seem to be opened");
    }
  } else {
    QMessageBox::critical(this, "QTCPServer", "Not connected");
  }
}

void Dialog::displayMessage(const QString &str) const {
  ui->textBrowser_receivedMessages->append(str);
}

void Dialog::refreshComboBox() {
  ui->comboBox_receiver->clear();
  ui->comboBox_receiver->addItem("Broadcast");
  foreach (QTcpSocket *socket, connection_set)
    ui->comboBox_receiver->addItem(socket->peerAddress().toString() + ":" +
                                   QString::number(socket->peerPort()));
}
