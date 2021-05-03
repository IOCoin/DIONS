#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include<QString>
#include<QFile>
#include<QFileInfo>
#include<iostream>
#include<fstream>
#include<boost/iostreams/filtering_streambuf.hpp>
#include<boost/iostreams/copy.hpp>
#include<boost/iostreams/filter/gzip.hpp>

class ArchiveExtractor : public QObject
{
  Q_OBJECT
  public:
    explicit ArchiveExtractor(QFileInfo&,QString&,QObject* parent=0);
    ~ArchiveExtractor() { }

    void run();

  public slots:
  signals:
    void finished();
  private: 
    QFileInfo fileDest_;
    QString   dir_;

};

#endif
