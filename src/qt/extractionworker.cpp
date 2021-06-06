#include "extractionworker.h"
#include "JlCompress.h"

extern std::atomic<bool>  extractionCompleted;
void ExtractionWorker::init(QFileInfo fileDest,QProgressBar* pb) 
{ 
  this->qfi_=fileDest;
  this->pb_=pb;
}
void ExtractionWorker::extract() 
{ 
  extractionCompleted=false;
  QStringList zextracted = JlCompress::extractDir(this,this->qfi_.filePath(), this->qfi_.path(),this->pb_);
  if(zextracted.isEmpty())
  {
    emit extractionempty();
  }

  emit completed();
}
