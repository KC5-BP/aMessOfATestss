#ifndef SERVER_H
#define SERVER_H

#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QtNetwork/QTcpServer>

class Server : public QDialog
{
    Q_OBJECT

public:
    Server(QWidget *parent = nullptr);
    ~Server();

private slots:
    void toggleConnection(void);
    void connectionSucessToClient(void);
    void sendToClient(void);
    void readCltRequest(void);

private:
    void initServer(QString &ipAddressOut);
    void initServer(QString ipAddressIn, qint16 port);

    QGridLayout *mainLayout = nullptr;
    QPushButton *quitBtn    = nullptr;

    /* Socket stuffs */
    bool serverStatus     = false;
    QTcpServer *tcpServer = nullptr;
    QDataStream inStream;
    QTcpSocket *cltConnection = nullptr;

    /* Data showing elements */
    QLabel    *ipLbl = nullptr, *portLbl = nullptr;
    QTextEdit *dataRecvTxtBox = nullptr;

    /* Interacting elements */
    QLineEdit   *ipLinEdit      = nullptr, *portLinEdit = nullptr;
    QPushButton *activateSvrBtn = nullptr;  /* Also used to kill
                                             * server (toggle effect) */
    QLineEdit   *dataToSendLinEdit = nullptr;
    QPushButton *sendDataBtn       = nullptr;
};
#endif // SERVER_H
