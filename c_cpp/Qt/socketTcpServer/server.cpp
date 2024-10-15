#include "server.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QNetworkInterface>
#include <QMessageBox>

Server::Server(QWidget *parent) : QDialog(parent) {
    /* *** Interactives ****** */
    /* ****** Buttons */
    quitBtn = new QPushButton("Quit");
    connect(quitBtn, &QPushButton::clicked, this, &Server::close);

    activateSvrBtn = new QPushButton("Launch server into space");
    activateSvrBtn->setMinimumSize(50, 50);
    connect(activateSvrBtn, &QPushButton::clicked, this, &Server::toggleConnection);

    sendDataBtn = new QPushButton("Send data");
    sendDataBtn->setMinimumSize(50, 50);
    sendDataBtn->setEnabled(false);
    connect(sendDataBtn, &QPushButton::clicked, this, &Server::sendToClient);

    /* ****** Lines Edit */
    ipLinEdit   = new QLineEdit;
    ipLinEdit->setText("127.0.0.1");
    portLinEdit = new QLineEdit;
    portLinEdit->setText("5000");

    /* *** Displays ****** */
    ipLbl   = new QLabel("IP   : ");
    ipLbl->setFont({ "Source Code Pro" });
    portLbl = new QLabel("Port : ");
    portLbl->setFont({ "Source Code Pro" });
    dataToSendLinEdit = new QLineEdit;
    dataRecvTxtBox    = new QTextEdit;

    /* *** Socket ****** */
    tcpServer = new QTcpServer(this);
    connect(tcpServer, &QTcpServer::newConnection,
            this, &Server::connectionSucessToClient);

    /* *** Add elements to layout ****** */
    mainLayout = new QGridLayout(this);
    mainLayout->addWidget(ipLbl, 0, 0);
    mainLayout->addWidget(ipLinEdit, 0, 1);
    mainLayout->addWidget(portLbl, 1, 0);
    mainLayout->addWidget(portLinEdit, 1, 1);
    mainLayout->addWidget(activateSvrBtn, 2, 0, 1, 2);
    mainLayout->addWidget(dataToSendLinEdit, 3, 0);
    mainLayout->addWidget(sendDataBtn, 3, 1);
    mainLayout->addWidget(dataRecvTxtBox, 4, 0, 1, 2);
    mainLayout->addWidget(quitBtn, 5, 1);

    setWindowTitle("QTcpServer: Server");
    resize(480, 320);
}

Server::~Server() {
    delete(quitBtn);
    delete(activateSvrBtn);
    delete(sendDataBtn);
    delete(ipLinEdit);
    delete(portLinEdit);
    delete(ipLbl);
    delete(portLbl);
    delete(dataToSendLinEdit);
    delete(dataRecvTxtBox);
    delete(tcpServer);
    delete(mainLayout);
}

void Server::readCltRequest(void) {
    inStream.startTransaction();

    QString streamAsString;
    inStream >> streamAsString;

    if ( ! inStream.commitTransaction() )   return;

    /** Detect client's leave
     *  + toLower() Hypothesis:
     *  Upper -> Lower is optimized because UpperCase + offset = LowerCase
     *  instead of a substraction LowerCase - offset = UpperCase */
    if (streamAsString.toLower() == QString("leaving")) {
        cltConnection = nullptr;
        sendDataBtn->setEnabled(false);
    }

    dataRecvTxtBox->append("CLT: " + streamAsString);
}

void Server::sendToClient(void) {
    /* Check if a client has been accepted yet */
    if ( ! cltConnection )  return;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    /* /!\ Doesn't work properly using C string (out << "Hi";) /!\ */
    out << dataToSendLinEdit->text();

    cltConnection->write(block);
    cltConnection->flush();

    /* If server forces client to leave, then nullify client connection */
    if (dataToSendLinEdit->text().toLower() == QString("leaving")) {
        cltConnection = nullptr;
        sendDataBtn->setEnabled(false);
    }
}

void Server::connectionSucessToClient(void) {
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    /* /!\ Doesn't work properly using C string (out << "Hi";) /!\ */
    out << QString("Connection successful");

    if ( ! cltConnection ) {
        cltConnection = tcpServer->nextPendingConnection();
        connect(cltConnection, &QAbstractSocket::disconnected,
                cltConnection, &QObject::deleteLater);

        inStream.setDevice(cltConnection);
        connect(cltConnection, &QIODevice::readyRead,
                this, &Server::readCltRequest);
    }

    cltConnection->write(block);
    cltConnection->flush();

    sendDataBtn->setEnabled(true);

    dataRecvTxtBox->append("SRV: A client has established a connection");
}

void Server::toggleConnection(void) {
    ipLinEdit->setEnabled(serverStatus);
    portLinEdit->setEnabled(serverStatus);

    serverStatus = !serverStatus;

    if (serverStatus) {
        activateSvrBtn->setText("Kill server");

        if (ipLinEdit->text().isEmpty() || portLinEdit->text().isEmpty() ) {
            QString newIpAddr;
            initServer(newIpAddr);
            ipLinEdit->setText(newIpAddr);
            portLinEdit->setText(QString("%1").arg(tcpServer->serverPort()));
        } else {
            initServer(ipLinEdit->text(), portLinEdit->text().toInt());
        }
    } else {
        activateSvrBtn->setText("Launch server into space");
        sendDataBtn->setEnabled(false);
        if (cltConnection) {
            /* Send a leaving message to inform the client and
             * allow it to reset its state as
             * QTcpSocket::disconnectFromHost() is used below */
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);

            /* /!\ Does not work properly using C string (out << "Hi";) /!\ */
            out << QString("Leaving");

            cltConnection->write(block);
            cltConnection->flush();

            cltConnection->disconnectFromHost();
            cltConnection = nullptr;
        }
        tcpServer->close();
    }
}

void Server::initServer(QString &ipAddressOut) {
    if ( ! tcpServer->listen() ) {
        QMessageBox::critical(this, tr("Socket's server"),
                              tr("Unable to start the server: %1.")
                                .arg(tcpServer->errorString()));
        close();
        return;
    }

    const QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (const QHostAddress &entry : ipAddressesList) {
        if (entry != QHostAddress::LocalHost && entry.toIPv4Address()) {
            ipAddressOut = entry.toString();
            break;
        }
    }

    // if we did not find one, use IPv4 localhost
    if (ipAddressOut.isEmpty())
        ipAddressOut = QHostAddress(QHostAddress::LocalHost).toString();
}

void Server::initServer(QString ipAddressIn, qint16 port) {
    if ( ! tcpServer->listen(QHostAddress(ipAddressIn), port) ) {
        QMessageBox::critical(this, tr("Socket's server"),
                              tr("Unable to start the server: %1.")
                                .arg(tcpServer->errorString()));
        close();
        return;
    }
}
