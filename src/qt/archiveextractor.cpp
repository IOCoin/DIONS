#include "archiveextractor.h"

ArchiveExtractor::ArchiveExtractor(QFileInfo& fileDest,QString& dir,QObject* parent)
	    : QObject(parent), fileDest_(fileDest), dir_(dir)
{
}
void ArchiveExtractor::run() 
{ 
  std::ifstream ifs(fileDest_.filePath().toStdString(), std::ios_base::in | std::ios_base::binary);   
  boost::iostreams::filtering_streambuf<boost::iostreams::input> in__;
  in__.push(boost::iostreams::gzip_decompressor());
  in__.push(ifs);
  std::ofstream file(dir_.toStdString() + "/bootstrap.dat", std::ios_base::out | std::ios_base::binary);
  boost::iostreams::copy(in__, file);
  emit finished();
}
