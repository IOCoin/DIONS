#include "deleteworker.h"
#include "JlCompress.h"
#include<QFile>
#include<QDir>
#include <boost/filesystem.hpp>

void DeleteWorker::init(QFileInfo& f1) 
{
  QString path = f1.filePath();
  this->qfi1_= path + "/blk0001.dat";
  this->qfi2_= path + "/txleveldb";
  std::cout << "file " << (this->qfi1_).toStdString() << std::endl;
  std::cout << "dir  " << (this->qfi2_).toStdString() << std::endl;
}
void DeleteWorker::removeFiles() 
{
  QFile::remove(this->qfi1_);

  //QDir txdbDir((this->qfi2_).toStdString().c_str());
  //txdbDir.removeRecursively();

  boost::filesystem::remove_all((this->qfi2_).toStdString().c_str());

  emit completed();
}
