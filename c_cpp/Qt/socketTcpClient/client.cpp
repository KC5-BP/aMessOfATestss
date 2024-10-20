/**
 * Use QByteArray type to exchange between Server and Client,
 * as a QString stores character in 16bits
 * instead of 8bits, like a classic "char *" in C
 *
 * Thus, enabling C socket client programming simplification
 * as it does not need to take one character over 2 [i*2]
 */
#include "client.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QtNetwork/QTcpSocket>
#include <QTextEdit>

Client::Client(QWidget *parent) : QDialog(parent) {
    /* *** Interactives ****** */
    /* ****** Buttons */
    quitBtn = new QPushButton("Quit");
    connect(quitBtn, &QPushButton::clicked, this, &Client::close);

    connectBtn  = new QPushButton("Connect");
    connectBtn->setMinimumSize(50, 50);
    /* Little trick: "clicked" will be false on event trigger...
     *               So instead of simply having:
     *                   connect(.. , .. , this, &Client::toggleConnection);
     *               and implicitly having false as function's argument,
     *               use lambda expression to explicitly set it.            */
    connect(connectBtn, &QPushButton::clicked, this, [=](bool clicked) {
        Client::toggleConnection(true);
    });

    sendDataBtn = new QPushButton("Send data");
    sendDataBtn->setMinimumSize(50, 50);
    sendDataBtn->setEnabled(false);
    connect(sendDataBtn, &QPushButton::clicked, this, &Client::sendToServer);

    /* ****** Lines Edit */
    ipLinEdit   = new QLineEdit("127.0.0.1");
    portLinEdit = new QLineEdit("5000");

    /* *** Displays ****** */
    ipLbl   = new QLabel("IP   : ");
    ipLbl->setFont({ "Source Code Pro" });
    portLbl = new QLabel("Port : ");
    portLbl->setFont({ "Source Code Pro" });
    dataToSendLinEdit = new QLineEdit;
    dataRecvTxtBox    = new QTextEdit;

    /* *** Socket ****** */
    tcpSocket = new QTcpSocket(this);
    inStream.setDevice(tcpSocket);
    connect(tcpSocket, &QIODevice::readyRead, this, &Client::readSrvResponse);
    connect(tcpSocket, &QAbstractSocket::errorOccurred,
            this, &Client::displayError);

    /* *** Layout & Window ****** */
    mainLayout = new QGridLayout(this);
    mainLayout->addWidget(ipLbl, 0, 0, 1, 2);
    mainLayout->addWidget(ipLinEdit, 0, 2, 1, 2);
    mainLayout->addWidget(portLbl, 1, 0, 1, 2);
    mainLayout->addWidget(portLinEdit, 1, 2, 1, 2);
    mainLayout->addWidget(connectBtn, 2, 0, 1, 4);
    mainLayout->addWidget(dataToSendLinEdit, 3, 0, 1, 3);
    mainLayout->addWidget(sendDataBtn, 3, 3);
    mainLayout->addWidget(dataRecvTxtBox, 4, 0, 1, 4);
    mainLayout->addWidget(quitBtn, 5, 1, 1, 2);

    setWindowTitle("QTcpSocket: Client");
    resize(480, 320);
}

Client::~Client() {
    delete(quitBtn);
    delete(connectBtn);
    delete(sendDataBtn);
    delete(ipLinEdit);
    delete(portLinEdit);
    delete(ipLbl);
    delete(portLbl);
    delete(dataToSendLinEdit);
    delete(dataRecvTxtBox);
    delete(tcpSocket);
    delete(mainLayout);
}

void Client::displayError(QAbstractSocket::SocketError socketError) {
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;

    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("QTcpSocket: Client"),
                                 tr("The host was not found. Please check the "
                                    "host name and port settings."));
        break;

    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("QTcpSocket: Client"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the fortune server is running, "
                                    "and check that the host name (%1) and "
                                    "port (%2) settings are correct.")
                                     .arg(ipLinEdit->text())
                                     .arg(portLinEdit->text()));

        /* Cancel actions attempted during connection */
        ipLinEdit->setEnabled(socketConnectStatus);
        portLinEdit->setEnabled(socketConnectStatus);
        socketConnectStatus = !socketConnectStatus;
        sendDataBtn->setEnabled(socketConnectStatus);
        connectBtn->setText("Connect");
        break;

    default:
        QMessageBox::information(this, tr("QTcpSocket: Client"),
                                 tr("The following error occurred: %1.")
                                     .arg(tcpSocket->errorString()));
    }
}

void Client::readSrvResponse(void) {
    inStream.startTransaction();

    QByteArray streamAsBytes;
    inStream >> streamAsBytes;

    if ( ! inStream.commitTransaction() )   return;

    /** Detect server's death or forced leave
     *  + toLower() Hypothesis:
     *  Upper -> Lower is optimized because UpperCase + offset = LowerCase
     *  instead of a substraction LowerCase - offset = UpperCase */
    if (streamAsBytes.toLower() == QByteArray("leaving"))
        toggleConnection(false);    /* A bit dirty, but for testing,
                                     * it does the trick
                                     * => Real action is closing connection */

    dataRecvTxtBox->append(QString("SRV: %1").arg(streamAsBytes));
}

void Client::sendToServer(void) {
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    /* /!\ Does not work properly using C string (out << "Hi";) /!\ */
    out << dataToSendLinEdit->text().toUtf8();

    /* Check input to also toggle connection Client side if entered manually */
    if (dataToSendLinEdit->text().toLower() == QString("leaving"))
        toggleConnection();

    tcpSocket->write(block);
    tcpSocket->flush();
}

void Client::toggleConnection(bool notifyServer) {
    ipLinEdit->setEnabled(socketConnectStatus);
    portLinEdit->setEnabled(socketConnectStatus);

    socketConnectStatus = !socketConnectStatus;

    sendDataBtn->setEnabled(socketConnectStatus);

    if (socketConnectStatus) {
        tcpSocket->connectToHost(ipLinEdit->text(), portLinEdit->text().toInt());
        connectBtn->setText("Disconnect");
    } else {
        if (notifyServer) {
            /* Send a leaving message to inform the server and
             * allow it to reset its state. */
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);

            /* /!\ Does not work properly using C string (out << "Hi";) /!\ */
            out << QByteArray("Leaving");

            tcpSocket->write(block);
            tcpSocket->flush();
        }
        tcpSocket->close();
        connectBtn->setText("Connect");
    }
}
