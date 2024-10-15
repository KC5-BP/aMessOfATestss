#ifndef CLIENT_H
#define CLIENT_H

#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QtNetwork/QTcpSocket>

class Client : public QDialog
{
    Q_OBJECT

public:
    Client(QWidget *parent = nullptr);
    ~Client();

private slots:
    void toggleConnection(bool notifyServer = true);
    void sendToServer(void);
    void readSrvResponse(void);
    void displayError(QAbstractSocket::SocketError socketError);

private:
    QGridLayout *mainLayout = nullptr;
    QPushButton *quitBtn    = nullptr;

    /* Socket stuffs */
    bool socketConnectStatus = false;
    QTcpSocket *tcpSocket    = nullptr;
    QDataStream inStream;

    /* Data showing elements */
    QLabel    *ipLbl = nullptr, *portLbl = nullptr;
    QTextEdit *dataRecvTxtBox = nullptr;

    /* Interacting elements */
    QLineEdit   *ipLinEdit  = nullptr, *portLinEdit = nullptr;
    QPushButton *connectBtn = nullptr;  /* Also used as
                                         * disconnect btn (toggle effect) */
    QLineEdit   *dataToSendLinEdit = nullptr;
    QPushButton *sendDataBtn       = nullptr;
};
#endif // CLIENT_H
