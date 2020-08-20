#ifndef MAIN_H
#define MAIN_H
#include <QFile>
#include <QDir>
#include <QTimer>
#include <QProcess>

namespace Ui { class Main; }

class main : public QObject
{
    Q_OBJECT

public:

    void init();
    void checkCompteur();
    void addToCompteur();
    void cleanFile();


public slots :

    void onCompteurTimeOut();
    void onReadyReadStandardOutput();

private:

    int m_index = 0;
    int m_intIndex = 0;
    QByteArray m_readIndex;

    QFile *m_fileDebug;
    QFile *m_watchDogFile;
    QDir  *m_watchDogFolder;
    QTimer *m_compteurTimer;
    QProcess *m_processIrigo;

};


#endif // MAIN_H
