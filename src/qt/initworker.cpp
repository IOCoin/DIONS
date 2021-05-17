#include "initworker.h"

void InitWorker::initialize() { this->obj_->complete_init(this->dir_); emit completed(); }
