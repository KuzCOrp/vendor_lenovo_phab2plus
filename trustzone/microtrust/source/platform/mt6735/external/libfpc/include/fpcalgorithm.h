#ifndef FPCALGORITHM_H
#define FPCALGORITHM_H

#include <dlfcn.h>

#include "fpc_ialgorithm.h"

class FpcAlgorithm : public ialgorithm_t  {
public:
    FpcAlgorithm();
    ~FpcAlgorithm();
    bool loaded();
private:
    void* lib_handle_;
    int error_;
};

#endif // FPCALGORITHM_H
