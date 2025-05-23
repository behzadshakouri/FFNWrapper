#ifndef FFNWRAPPER_H
#define FFNWRAPPER_H
#define MLPACK_ENABLE_ANN_SERIALIZATION
#include <mlpack.hpp>
#include <vector>
#include <BTCSet.h>
#include "cmodelstructure.h"
#include <gnuplot-iostream.h>

using namespace mlpack;
using namespace std;
using namespace arma;



class FFNWrapper : FFN<MeanSquaredError>
{
public:
    FFNWrapper();
    FFNWrapper(const FFNWrapper &F);
    FFNWrapper& operator=(const FFNWrapper& rhs);
    virtual ~FFNWrapper();
    bool DataProcess();
    mat A;

    bool Shifter();
    bool Initiate();
    bool Train();
    bool Test();
    bool PerformanceMetrics();
    bool DataSave();
    bool Plotter();
    bool Optimizer();
    CModelStructure ModelStructure;
    CTimeSeriesSet<double> *data;
    CTimeSeriesSet<double> *data2;
    CTimeSeriesSet<double> GetInputData()
    {
        return CTimeSeriesSet<double>(TestInputData,ModelStructure.dt,ModelStructure.lags);
    }
    CTimeSeriesSet<double> GetOutputData()
    {
        return CTimeSeriesSet<double>(TestOutputData,ModelStructure.dt,ModelStructure.lags);

    }

    mat Prediction;
    double nMSE = -999;
    double _R2 = -999;

private:
    mat TrainInputData;
    mat TrainOutputData;
    mat TestInputData;
    mat TestOutputData;


};


#endif // FFNWRAPPER_H
