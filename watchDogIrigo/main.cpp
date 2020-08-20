#include "main.h"
#include <QCoreApplication>
#include <QDebug>

void main::init(){

    m_processIrigo = new QProcess(this);
    m_watchDogFolder = new QDir;
    m_watchDogFolder->mkpath("C://FTP//watchDog");
    m_watchDogFile = new QFile("C://FTP//watchDog//compteur.txt");

    m_watchDogFile->open(QIODevice::ReadWrite);
    m_watchDogFile->write("0");
    m_watchDogFile->close();

    m_fileDebug = new QFile("C://FTP//watchDog//debug.txt");
    m_fileDebug->open(QIODevice::ReadWrite);


    m_compteurTimer = new QTimer;
    m_compteurTimer->setInterval(1000);
    m_compteurTimer->start();

    connect(m_compteurTimer, SIGNAL(timeout()), this, SLOT(onCompteurTimeOut()));
    connect(m_processIrigo, SIGNAL(readyReadStandardOutput()), this, SLOT(onReadyReadStandardOutput()));
}
void main::onCompteurTimeOut(){

    m_watchDogFile->open(QIODevice::ReadWrite);
    addToCompteur();
    qDebug() << "one second past" << m_index;
    m_watchDogFile->close();
    //    QByteArray indexString;
    //    indexString = indexString.append(QString::number(m_index));
    //    m_watchDogFile->write(indexString);
    //    m_watchDogFolder->refresh();
    //    m_readIndex = m_watchDogFile->readAll();
}

void main::addToCompteur(){

    m_readIndex = m_watchDogFile->readAll();
    m_index = m_readIndex.toInt();
    m_index++;
    m_readIndex.clear();
    QString iS = QString::number(m_index);
    m_readIndex.append(iS);
    m_watchDogFile->resize(0);
    m_watchDogFile->write(m_readIndex);

    if (m_readIndex == "30"){
        qDebug()<< "Serveur mort";
        QString command = ("C://ServeurIrigo//release//ServeurIrigo.exe");
        QStringList  params("-c");
        params.append("-Verb runAs");
        //        m_processIrigo->QProcess::start(command, params); MARCHE AUSSI
        m_processIrigo->kill();
        m_processIrigo->start(command, params);
        int status = m_processIrigo->exitStatus();
        qDebug() << "valeur sortie:" << status;
        m_watchDogFile->resize(0);
        m_watchDogFile->write("0");
    }
    cleanFile();
}

void main::onReadyReadStandardOutput(){
    qDebug() << Q_FUNC_INFO;
    QByteArray readProcess;
    readProcess = m_processIrigo->readAllStandardOutput();
    qDebug() << readProcess;
    m_fileDebug->write(readProcess);
    m_fileDebug->close();
}

void main::cleanFile(){
    if (m_watchDogFile->isOpen()){
        int fileSize = m_watchDogFile->size();
        m_watchDogFile->write(" ", fileSize);
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    class main m;
    m.init();
    return a.exec();
}










