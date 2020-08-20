#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QDebug>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QWidget>
#include <QTextBrowser>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QRegularExpressionMatchIterator>
#include <string.h>
#include <QListWidget>
#include <QFileSystemWatcher>
#include <QNetworkInterface>
#include <QTcpServer>
using namespace std;

//static int compteur = 0;                                                                                            //Compteur de lecture (actuellement limité à 16)

QList<QByteArray> listNomVal;                                                                                       //Déclaration en gloable de la liste de nom

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),ui(new Ui::MainWindow)
{    
    m_startDate = m_startDate.currentDate();
    m_watchDogFolder = new QDir("C:\\FTP\\watchDog");
    m_watchDogFile = new QFile("C:\\FTP\\watchDog\\compteur.txt");

    findOldFile();
    getPosition();
    directoryPerma();
    newFile();

    timerMinute = new QTimer(this);                                                                //set timer à 5s et le démarre
    timerMinute->setInterval(10000);
    timerMinute->start();
    connect(timerMinute, SIGNAL(timeout()), this, SLOT(onNewMinute()));

    initServer();
    ui->setupUi(this);

    listNomVal.clear();
    listNomVal.append("EA_CB_u_bac_1");
    listNomVal.append("EA_CB_u_bac_2");
    listNomVal.append("EA_CB_u_bac_3");
    listNomVal.append("EA_CB_u_bac_4");
    listNomVal.append("EA_CB_u_bac_5");
    listNomVal.append("EA_CB_u_bac_6");
    listNomVal.append("EA_CB_u_bac_7");
    listNomVal.append("EA_CB_u_bac_8");
    listNomVal.append("EA_CB_u_bac_9");
    listNomVal.append("EA_CB_u_bac_10");
    listNomVal.append("EA_CB_u_bac_11");
    listNomVal.append("EA_CB_u_bac_12");
    listNomVal.append("EA_CB_u_bac_13");
    listNomVal.append("EA_CB_u_bac_14");
    listNomVal.append("EA_CB_u_bac_15");
    listNomVal.append("EA_CB_u_bac_16");
    listNomVal.append("EA_CB_u_bac_17");
    listNomVal.append("EA_CB_u_bac_18");
    listNomVal.append("EA_CB_u_bac_19");
    listNomVal.append("EA_CB_u_bac_20");
    listNomVal.append("EA_CB_u_bac_21");
    listNomVal.append(("EA_CB_u_bac_moy"));
    listNomVal.append("EA_SIE_vit_train");
    listNomVal.append("EA_u_bat");
    listNomVal.append("EA_u_filtre");
    listNomVal.append("EA_u_front_1");
    listNomVal.append("EA_u_front_2");
    listNomVal.append("EA_u_panto");
    listNomVal.append("El_ackdef_arch");
    listNomVal.append("EL_cde_aps");
    listNomVal.append("El_cde_isol_arch");
    listNomVal.append("EL_cde_lac");
    listNomVal.append("El_cde_tract_bat_arch");
    listNomVal.append("El_vfh1_auto_arch");
    listNomVal.append("El_vfh2_auto_arch");
    listNomVal.append("El_vpb");
    listNomVal.append("El_vph");
    listNomVal.append("SL_SIE_fj_31");
    listNomVal.append("SL_SIE_ft_29");
    listNomVal.append("SL_SIE_hp_28");
    listNomVal.append("SL_SIE_rp_30");
    listNomVal.append("Vfb");

    qDebug() << "current date" << m_startDate;
    m_archive = true;
}

///////// Gestion du serveur et des connexions /////////

void MainWindow::initServer(){                                  //Fct d'initialisation des connections, init le serveur et le met en "mode" écoute

    qDebug() << "test initialisation du serveur";
    if(m_iServer){
        delete m_iServer;
        m_iServer = nullptr;
        qDebug() << "test destruction du serveur";
    }
    m_iServer =  new QTcpServer(this);
    m_iServer->listen(QHostAddress::Any, 502);
    qDebug() << "test lecture du serveur :" << m_iServer->isListening();

    connect(m_iServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));                                    //Connecte au slot de nouvelle connection
}



void MainWindow::onNewConnection()                                                  //When a new connection is available
{
    qDebug() << Q_FUNC_INFO << "Test de nouvelle connection disponible";

    m_socket = m_iServer->nextPendingConnection();                                   //le socket est associé à la prochaine connection que reçoit le serveur
    m_iServer->setMaxPendingConnections(1);                                         //Une seule connection à la fois (éviter les erreurs)

    checkConnection();
}

void MainWindow::checkConnection()                           //Vérifie l'adresse Ip
{
    QString adressIp = "::ffff:10.32.105.129";                 //Adresse IP connu
    QHostAddress adressIpUtf8 (adressIp);                     //
    QHostAddress socketIpObtain;                             //
    socketIpObtain = m_socket->peerAddress();               //Lit l'adresse IP du socket

    qDebug() << "Ip adress" << socketIpObtain;

    if (adressIpUtf8 == socketIpObtain){                   //Compare les deux adresses

        qDebug() << Q_FUNC_INFO << "Ip conforme";
        m_iServer->pauseAccepting();                         //Place les nouvelles connections en attente (si les adresse IP sont identiques cela implique que nous sommes sur le bon device)

        connect(m_socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));                                                                          //Connect au slot de lecture
        connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));                      //Slot d'erreur
        connect(m_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(socketStateChanged(QAbstractSocket::SocketState)));        //Slot de changement d'état
        connect(m_socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    }
    else {
        qDebug() << Q_FUNC_INFO << "Ip non conforme";
        ui->dataBox->setText(ui->dataBox->toPlainText().append("Ip non conforme"));
        m_socket->close();                                                        //Ferme le socket (l'adresse IP n'est pas bonne, donc nous ne sommes pas sur le bon device)
        m_iServer->resumeAccepting();                                               //Relance la lecture des nouvelles connections par le serveur | Dés qu'une nouvelle connection arrivera elle émettra de nouveaux le signal NewConnection, relançant ainsi tous le processus
    }
}


QByteArray MainWindow::splitIt(QByteArray input){

    QString format = "dd_MM-yy HH-mm-ss";
    QString date = QDateTime::currentDateTime().toString(format);
    QByteArray dateUtf8 = date.toUtf8();

    //QByteArray dataClock = input.mid(0,4);
    QByteArray dataDate = input.mid(0,20);          //récupère l'heure et la date
    QByteArray data = input.mid(20, -1);            //récupère la trame après les 4 mots contenant la date et l'heure
    //data.prepend(dataClock);                        //Première valeur (U_BAC_moy) à placer dans la liste
    QByteArray result;
    int i = 0;
    //----------------- Années --------------------------------- Mois ------------------------------ Jours ------------------------------- heures ----------------------------- minutes ----------------------------- secondes ---
    m_timeAuto = dataDate.mid(16,4).append(";").append(dataDate.mid(12,2)).append(";").append(dataDate.mid(14,2)).append(";").append(dataDate.mid(8,2).append(";").append(dataDate.mid(10,2)).append(";").append(dataDate.mid(1,2)));
    while (data.length()) {

        if (data.length() >= 4 && i < listNomVal.length())
        {

            //result.append(listNomVal.at(i)).append(";").append(data.mid(0, 4)).append(";").append(dateUtf8).append('\n');
            result.append(listNomVal.at(i)).append(";").append(data.mid(0, 4)).append(";").append(m_timeAuto).append(";latitude;").append(m_latitude).append(";longitude;") .append(m_longitude).append('\n');
            data.remove(0,4);

        }
        else
        {
            if (i >= listNomVal.length())
                qDebug()<<"depassement du nombre de nom de valeur";
            else
                qDebug()<<"aucun groupe de 4 disponible, skip de la fin";
            break;
        }
        i++;
    }
    return result;
}

void MainWindow::onReadyRead()
{
    QByteArray datas;

    ui->dataBox->setText(ui->dataBox->toPlainText().append("Trame_recu \n"));
    while (m_socket->bytesAvailable())
    {                                                                         //Tant que des données sont avalaible
        qDebug() << Q_FUNC_INFO << "Test de data disponible";

        findOldFile();

        datas = m_socket->readAll();
        datas = datas.toHex();
        ui->dataBox->setText(ui->dataBox->toPlainText().append("Trame_complète").append(QString(datas).append('\n')));           //Affiche dans l'ui au format Hexa (et plainText) les datas

        QByteArray Input = datas.mid(26, -1);                                     //Récupère la trame, après le 26ième caractère
        ui->dataBox->setText(ui->dataBox->toPlainText().append(QString("Trame_séparé ").append(Input).append('\n')));           //Affiche dans l'ui au format Hexa (et plainText) les datas

        QList<QByteArray> listInput;
        listInput.append(Input);

        m_index = 0;
        m_timeAuto.clear();
        m_file->write(splitIt(Input));
        //m_file->write("\n \n");
    }
    m_socket->close();
    m_iServer->close();

    m_folder->refresh();

    m_iServer->listen(QHostAddress::Any, 502);                                                                        //Le serveur écoute toutes les adresses sur le port 502
    m_socket->open(QIODevice::ReadWrite);

}

//////////// Gestion de l'arhive //////////////////////

void MainWindow::findOldFile(){
    qDebug() << Q_FUNC_INFO;
    int i = 0;
    QString index = QString::number(i);

    while (m_archive == true){
        qDebug() << "test de la fct";
        if (!QFile("C://FTP//ValeurIrigo_"+index+".txt").exists()){
            qDebug() << "old file not found !";
            qDebug() << "valeur index :" << index;
            i++;
            index = QString::number(i);
            if (index == "1000") m_archive = false;
        }
        //else {
        else if (QFile("C://FTP//ValeurIrigo_"+index+".txt").exists()){
            qDebug() << "old file found !";
            m_fileInfo = new QFileInfo("ValeurIrigo_"+index);
            checkDate();
            m_archive = false;
            return;
        }
    }
}


void MainWindow::checkDate(){
    qDebug() << Q_FUNC_INFO;
    QDateTime oldDate = m_fileInfo->birthTime();
    QDateTime currentDate = currentDate.currentDateTime();

    if (oldDate.daysTo(currentDate) > 5){
        qDebug() << "Archive supérieur à 5 jours, suppression";
        clearArchive();
    }
    else {
        qDebug() << "Archive inférieur à 5 jours, conservation";
    }
}


void MainWindow::clearArchive(){

    qDebug() << Q_FUNC_INFO;
    QString path = m_folderPath + "\\_Archives";
    QDir dir(path);
    dir.setNameFilters(QStringList() << "*.*");
    dir.setFilter(QDir::Files);
    foreach(QString dirFile, dir.entryList())
    {
        dir.remove(dirFile);
    }
}


//////////// Gestion des dossiers/fichiers //////////////////////

void MainWindow::directoryPerma(){

    m_folder = new QDir(m_folderPath);
    m_folder->mkpath(m_folderPath);
}

void MainWindow::newFile(){

    if (m_indexFile == 500000000) m_indexFile = 0;                                              //Si tous se passe bien et que le tram tourne pendent beaucoup trop longtemps..je réinitialise ce int, juste au cas ou

    QString nameFile = QString("ValeurIrigo_").append(QString::number(m_indexFile).append(" Tramway numero 1"));            //Définit le nom du fichier
    QString m_filePath = QString(m_folderPath ).append(nameFile).append(".txt");              //Ajoute l'extension du fichier

    m_file = new QFile(m_filePath);
    qDebug() << m_file;
    qWarning()<<" ouverture du fichier";

    m_file->open(QIODevice::ReadWrite | QIODevice::Append);
    if (!m_file->isOpen())
    {
        qWarning() << "Erreur d'ouverture du fichier";
    }
    m_indexFile++;
}

void MainWindow::onNewMinute(){                                                                    //Toute les 5s appelle la fonction new File
    qDebug() << Q_FUNC_INFO;
    m_file->close();
    m_watchDogFile->remove();
    if (!m_watchDogFile->isOpen()){
        m_watchDogFile->open(QIODevice::ReadWrite);
    }
    //m_watchDogFile->resize(0);
    m_watchDogFile->write("0");
    m_watchDogFile->close();

    qDebug()  << "10 second just past";
    //e();
    newFile();
}


///////////// Info GPS /////////////////////

QStringList MainWindow::getPosition(){

    qDebug() << Q_FUNC_INFO;

    QStringList portGPS = QGeoPositionInfoSource::availableSources();

    QVariantMap params;
    params["serialnmea.serial_port"];
    //QT_NMEA_SERIAL_PORT = ;
    QGeoPositionInfoSource *positionSource = QGeoPositionInfoSource::createSource("serialnmea", params,this);

    qDebug() << portGPS.size();
    qDebug() << "portGPS :" << portGPS.at(0);
    qDebug() << "postionSource :" << positionSource;
    //positionSource->startUpdates();


    m_geoPosition = new QGeoCoordinate();

    // m_geoPosition->setLatitude(latitude);
    // m_geoPosition->setLongitude(longitude);

    qDebug() << m_geoPosition->latitude();
    qDebug() << m_geoPosition->longitude();
    return  portGPS;
}



///////// Signal d'erreur du serveur ////////////

void    MainWindow::socketError(QAbstractSocket::SocketError socketError)
{
    qWarning()<<"ERROR CODE :" << socketError;
}

void    MainWindow::socketStateChanged(QAbstractSocket::SocketState socketState)
{
    qWarning()<<"socket state changed:" << socketState;
}

void    MainWindow::socketDisconnected()
{
    qWarning()<<"client socket disconnected";
}




////// Bouton //////////////

void MainWindow::on_btQuitter_clicked()
{
    QCoreApplication::exit(0);
}

void MainWindow::on_btLecture_clicked()
{
    qDebug() << "Lecture";
    m_file->open(QIODevice::ReadWrite | QIODevice::Append);
    initServer();

}
void MainWindow::on_btPause_clicked()
{
    qDebug() << "Pause";
    m_file->close();

    delete m_iServer;
    m_iServer = nullptr;

}
void MainWindow::on_btRestart_clicked()
{

    qDebug() << "Restart";
    m_file->close();
    m_file->open(QIODevice::ReadWrite | QIODevice::Append);
    if (m_iServer){
        delete m_iServer;
        m_iServer = nullptr;
    }

    initServer();
}



//////////// Destructeur ////////////////

MainWindow::~MainWindow()
{
    m_file->close();
    delete ui;
}




///////////////// lecture & écriture dans fichiers texte///////////////////////
/*          CODE FONCTIONNEL DE LA LECTURE
QByteArray MainWindow::splitIt(QByteArray input){

QString format = "dd_MM-yy HH-mm-ss";
QString date = QDateTime::currentDateTime().toString(format);
QByteArray dateUtf8 = date.toUtf8();

QByteArray data = input;
QByteArray result;
int i = 0;
while (data.length()) {

  if (data.length() >= 4 && i < listNomVal.length())
  {


    //result.append(listNomVal.at(i)).append(";").append(data.mid(0, 4)).append(";").append(dateUtf8).append('\n');
    result.append(listNomVal.at(i)).append(";").append(data.mid(0, 4)).append(";").append('\n');
    data.remove(0,4);

  }
  else
  {
    if (i >= listNomVal.length())
      qDebug()<<"depassement du nombre de nom de valeur";
    else
      qDebug()<<"aucun groupe de 4 disponible, skip de la fin";
    break;
  }
  i++;
}
return result;
}
////////////////////////////////////////////


void MainWindow::moveFile(){

  qDebug() << "fct moveFile";

  QString fileNamePerma = "ValeurIrigo_" + QString::number(m_indexFile);
  QString filePermaPath = m_folderPath .append(fileNamePerma).append(".txt");
  m_filePerma = new QFile(filePermaPath);

  m_file->copy(filePermaPath);

  //m_file->remove();
}



QByteArray MainWindow::spareIt(QByteArray Input){


  QString format = "dd_MM_yy HH-mm-ss-z";                                        //Configure le format -> ref doc QDateTime
  QString date = QDateTime::currentDateTime().toString(format);
  QByteArray dateUtf8 = date.toUtf8();

  QByteArray InputSpared;                                                        //Données découpé 4 par 4
  QByteArray resultData;                                                          //Donnée découpé 4 par 4, avec horodatage
  int init = 0;
  int index = 0;

  for (int i = 0; i != Input.length(); i++){
    InputSpared.append(Input.at(i));
    if (index == Input.length()) index = 0;
    init++;
    if (init == 4){
      resultData.append(listNomVal.at(index)).append(';').append(InputSpared).append(';').append(dateUtf8).append('\n');
      InputSpared.clear();
      index++;
      init = 0;
    }
  }
  return resultData;
}

void MainWindow::fileInit(QString folderPath, QString fileName, int fileIndex)
{
  qDebug() << "création dossier";
  bool creationState = false;
  QDir d(folderPath);                                              //La classe QDir fournit un accès au fichier, il est lié à un string Path pour lui indiquer quel fichier..
  if(!d.exists())                                                    //Si le chemin "d" n'existe pas ..
  {
    creationState = d.mkpath(folderPath);                                         //La fct mkpth permet de crée des fichiers si il en manque
    if (!creationState)
      qWarning()<<"Erreur de creation du fichier";
  }

  QString name = QString(fileName).append(QString::number(fileIndex));            //Définit le nom du fichier
  QString fichier = QString(folderPath).append(name).append(".txt");              //Ajoute l'extension du fichier

  m_outFile = new QFile(fichier);
  qWarning()<<" ouverture du fichier";
  m_outFile->open(QIODevice::ReadWrite | QIODevice::Append);
  if (!m_outFile->isOpen())
  {
    qWarning()<<"Erreur d'ouverture du fichier";
  }
}
}




void MainWindow::writeFile(QString folderPath, QString fileName)                  //Création d'un nouveau fihcier texte à chaque appel
{
  qDebug() << "création dossier";
  bool creationState = false;
  QDir d(folderPath);                                              //La classe QDir fournit un accès au fichier, il est lié à un string Path pour lui indiquer quel fichier..
  if(!d.exists())                                                    //Si le chemin "d" n'existe pas ..
  {
    creationState = d.mkpath(folderPath);                                         //La fct mkpth permet de crée des fichiers si il en manque
    if (!creationState)
      qWarning()<<"Erreur de creation du fichier";
  }

  QString name = QString(fileName).append(QString::number(m_indexFile));            //Définit le nom du fichier
  QString m_filePath = QString(folderPath).append(name).append(".txt");              //Ajoute l'extension du fichier

  m_outFile = new QFile(m_filePath);
  qWarning()<<" ouverture du fichier";

  m_outFile->open(QIODevice::ReadWrite | QIODevice::Append);
  if (!m_outFile->isOpen())
  {
    qWarning() << "Erreur d'ouverture du fichier";
  }
  m_indexFile++;
}

void MainWindow::onNewMinute(){                                                                    //Toute les 5s appelle la fonction new File

  qDebug()  << "5 second just past";
  writeFile("C:\\QtProject\\ValeurCompteurIrigo2\\","ValeurMinute");
}


QByteArray MainWindow::splitRead(QByteArray Input, QByteArray datas){

  int i = 0;
  QByteArray resultData;
  QList<QByteArray> li;
  QByteArray Out;                                                               //Out = Inout séparé en 4 par 4
  int tabLimit = listNomVal.count();

  for (int c = 0; c < Input.length(); c++)                                    //Pour c inférieur à la taille de Input, c++
  {
    Out += Input[c];                                                       //A Out, ajoute la valeur Input à la postion c
    //qDebug() << Out;
    if (i == tabLimit){ i = 0;}
    if ( c % 4 == 0)                                                        //  Si c modulo 4 (si le reste d'une division par 4 est égale à 0)
    {
      while (!checkList(li)){                                             //Tant que la liste n'est pas égale à 42
        Input = datas.mid(26, -1);
        QString format = "dd_MM_yy HH-mm-ss-z";                                        //Configure le format -> ref doc QDateTime
        QString date = QDateTime::currentDateTime().toString(format);
        QByteArray dateUtf8 = date.toUtf8();
        //li.append(listNomVal.at(i) + " ");
        li.append(Out);
        resultData.append(listNomVal.at(i) + ";" + Out).append(";" + dateUtf8 + "\n" );
        Out.clear();
        i++;
      }
    }
    // qDebug() << li;
  }
  return resultData;
}
*/
