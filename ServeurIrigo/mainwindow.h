#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAbstractSocket>
#include <QTcpServer>
#include <QModbusServer>
#include <QTcpSocket>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QtPositioning>

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void initServer();                              //initialise le serveur
    void checkConnection();                         //vérifie l'ip du client
    void directoryPerma();
    void newFile();
    void moveFile();
    void findOldFile();
    void checkDate();
    void clearArchive();
    QStringList getPosition();

    void writeFile(QString folderPath, QString fileName);
    QByteArray spareIt(QByteArray Input);
    QByteArray  splitIt(QByteArray input);
    QByteArray splitRead(QByteArray Input, QByteArray datas);

public slots:
    void onNewConnection();                             //Setup les connections au serveur
    void onReadyRead();
    void socketError(QAbstractSocket::SocketError socketError);
    void socketStateChanged(QAbstractSocket::SocketState socketState);
    void socketDisconnected();

    void onNewMinute();                                //Setup le timeout du timer

private slots:

    void on_btQuitter_clicked();                        //Setup les boutons
    void on_btRestart_clicked();
    void on_btPause_clicked();
    void on_btLecture_clicked();

protected:
    Ui::MainWindow *ui;                                 //Setup interface utilisateur

    QTcpServer  *m_iServer = nullptr;                   //Setup le serveur
    QTcpSocket *m_socket = nullptr;
    QTcpSocket *m_socketAnswer;

    QTimer *timerMinute;                                //Setup timer

    QDir *m_watchDogFolder;
    QFile *m_watchDogFile;

    QFileInfo *m_fileInfo = nullptr;
    QDir *m_folder = nullptr;
    QFile *m_file = nullptr;

    QByteArray m_timeAuto;

    QString m_filePath;
    QString m_fileArchive;
    QString m_folderPath = "C:\\FTP\\";

    QGeoCoordinate *m_geoPosition;

    double m_indexFile = 0;                         //Sers à indexer les numéros de fichiers texte
    bool m_archive = true ;
    int m_indexData = 0;                            //Useless for now
    int m_index = 0;                                //Sers à indexer les noms lors de l'écriture dans le fichier texte

    QString m_latitude = "47.4825126";
    QString m_longitude = "-0.5484918999999999";

    QDate m_startDate;

};

#endif // MAINWINDOW_H
