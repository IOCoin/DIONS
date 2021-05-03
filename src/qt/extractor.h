#ifndef EXTRACTOR_H
#define EXTRACTOR_H

class Worker : public QObject
 {
     Q_OBJECT
     QThread workerThread;

 public slots:
     void doWork(const QString &parameter) {
         // ...
         emit resultReady(result);
     }

 signals:
     void resultReady(const QString &result);
 };

 class Controller : public QObject
 {
     Q_OBJECT
     QThread workerThread;
 public:
     Controller() {
         Worker *worker = new Worker;
         worker->moveToThread(&workerThread);
         connect(workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
         connect(this, SIGNAL(operate(QString)), worker, SLOT(doWork(QString)));
         connect(worker, SIGNAL(resultReady(QString)), this, SLOT(handleResults(QString)));
         workerThread.start();
     }
     ~Controller() {
         workerThread.quit();
         workerThread.wait();
     }
 public slots:
     void handleResults(const QString &);
 signals:
     void operate(const QString &);
 };

#endif
