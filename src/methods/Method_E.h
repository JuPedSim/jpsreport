#ifndef METHOD_E_H_
#define METHOD_E_H_

#include "../general/Macros.h"
#include "MeasurementArea.h"
#include "PedData.h"
#include "tinyxml.h"

#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>

class Method_E
{
public:
    Method_E();
    virtual ~Method_E();
    bool Process();
};

#endif /* METHOD_E_H_ */