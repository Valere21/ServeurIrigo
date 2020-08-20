#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAbstractSocket>
#include <QTcpServer>
#include <QModbusServer>
#include <QTcpSocket>
#include <QTimer>
#include <QFile>
#include <QModbusDevice>

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void initServer();
    void checkConnection();
    void getDatas();
    void fileInit(QString folderPath, QString fileName, int fileIndex);
    void newFile(QString folderPath, QString fileName);
    QByteArray  splitIt(QByteArray input);
    bool checkList(QList<QByteArray>);

    //QByteArray splitRead(QByteArray input, QByteArray data);

public slots:
    void onNewConnection();
    void onReadyRead();
    void socketError(QAbstractSocket::SocketError socketError);
    void socketStateChanged(QAbstractSocket::SocketState socketState);
    void socketDisconnected();
    void onNewMinute();

private slots:

    void on_btQuitter_clicked();
    void on_btRestart_clicked();
    void on_btPause_clicked();
    void on_btLecture_clicked();

protected:
    QString m_fichier;
    Ui::MainWindow *ui;
    QTcpServer  m_server;
    QTcpServer  *m_iServer = nullptr;
    QTcpSocket *m_socketAnswer;
    QList<QTcpSocket*>  _socketsList;
    QTcpSocket *m_socket;
    QFile *m_outFile;
    QTimer *timerMinute;
    int m_index = 0;
};

#endif // MAINWINDOW_H
