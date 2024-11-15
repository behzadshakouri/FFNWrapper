#include "ffnwrapper_multi.h"
#include <mlpack.hpp>

using namespace mlpack;
using namespace std;
using namespace arma;

#include <mlpack/core.hpp>
#include <mlpack/methods/ann/ffn.hpp>
#include <mlpack/methods/ann/layer/layer.hpp>
#include <mlpack/methods/ann/loss_functions/mean_squared_error.hpp>
#include <armadillo>
using namespace mlpack::ann;

#include <QVector>
#include <iostream>
#include <cmath>
#include <gnuplot-iostream.h>

#include <ensmallen.hpp>  // Ensmallen header file

FFNWrapper_Multi::FFNWrapper_Multi():FFN<MeanSquaredError>()
{

}

FFNWrapper_Multi::FFNWrapper_Multi(const FFNWrapper_Multi &rhs):FFN<MeanSquaredError>(rhs)
{
    ModelStructure = rhs.ModelStructure;
    data = rhs.data;

}

FFNWrapper_Multi& FFNWrapper_Multi::operator=(const FFNWrapper_Multi& rhs)
{
    FFN<MeanSquaredError>::operator=(rhs);
    ModelStructure = rhs.ModelStructure;
    data = rhs.data;

    return *this;
}
FFNWrapper_Multi::~FFNWrapper_Multi()
{

}


bool FFNWrapper_Multi::DataProcess()
{

    // Load the whole data (OpenHydroQual output).
    //ModelStructure.InputTimeSeries = new CTimeSeriesSet<double>(ModelStructure.inputaddress,true);

    // Writing the data for checking
    data = new CTimeSeriesSet<double>(*ModelStructure.InputTimeSeries);

    ShifterAppend();

    return true;
}

bool FFNWrapper_Multi::Shifter()
{
       CTimeSeriesSet<double> InputTimeSeries(ModelStructure.inputaddress[0],true);

        //Shifting by lags definition (Inputs)
        TrainInputData = InputTimeSeries.ToArmaMatShifter(ModelStructure.inputcolumns, ModelStructure.lags);

        CTimeSeriesSet<double> ShiftedInputs(TrainInputData,ModelStructure.dt,ModelStructure.lags);
        //ShiftedInputs.writetofile("ShiftedInputs.txt");

        //Shifting by lags definition (Outputs)
        TrainOutputData = InputTimeSeries.ToArmaMatShifterOutput(ModelStructure.outputcolumns, ModelStructure.lags);

        CTimeSeriesSet<double> ShiftedOutputs = CTimeSeriesSet<double>::OutputShifter(TrainOutputData,ModelStructure.dt,ModelStructure.lags);
        //ShiftedOutputs.writetofile("ShiftedOutputs.txt");

    return true;
}

bool FFNWrapper_Multi::ShifterAppend()
{
    for (int i=0; i<ModelStructure.inputaddress.size(); i++)
    {
        CTimeSeriesSet<double> InputTimeSeries(ModelStructure.inputaddress[i],true);

        //Shifting by lags definition (Inputs)
        mat TrainInputData1 = InputTimeSeries.ToArmaMatShifter(ModelStructure.inputcolumns, ModelStructure.lags);
        if (TrainInputData.n_cols!=0)
            TrainInputData = join_rows(TrainInputData, TrainInputData1);
        else
            TrainInputData = TrainInputData1;

        CTimeSeriesSet<double> ShiftedInputs(TrainInputData,ModelStructure.dt,ModelStructure.lags);
        //ShiftedInputs.writetofile("ShiftedInputs.txt");

        //Shifting by lags definition (Outputs)

        mat TrainOutputData1 = InputTimeSeries.ToArmaMatShifterOutput(ModelStructure.outputcolumns, ModelStructure.lags);
        if (TrainOutputData.n_cols!=0)
            TrainOutputData = join_rows(TrainOutputData, TrainOutputData1);
        else
            TrainOutputData = TrainOutputData1;
        CTimeSeriesSet<double> ShiftedOutputs = CTimeSeriesSet<double>::OutputShifter(TrainOutputData,ModelStructure.dt,ModelStructure.lags);
        //ShiftedOutputs.writetofile("ShiftedOutputs.txt");
    }
    return true;
}

bool FFNWrapper_Multi::Initiate()
{
    DataProcess();

    // Initialize the network
    for (int layer = 0; layer<ModelStructure.n_layers; layer++)
    {
        Add<Linear>(ModelStructure.n_nodes[layer]); // Connection Layer : ModelStructure.n_input_layers
        Add<Sigmoid>(); // Activation Funchion
    }

   //model.Add<Linear>(3); // Connection Layer 2: ModelStructure.n_input_layers
    //model.Add<Sigmoid>(); // Activation Funchion 2
    Add<Linear>(TrainOutputData.n_rows); // Output Layer : ModelStructure.n_output_layers

    return true;
}

bool FFNWrapper_Multi::Training()
{

    // Train the model
    Train(TrainInputData, TrainOutputData);

    return true;
}

bool FFNWrapper_Multi::Testing()
{
    // Use the Predict method to get the predictions.

    ModelStructure.TestTimeSeries = new CTimeSeriesSet<double>(ModelStructure.testaddress,true);

    // Writing the data for checking
    data2 = new CTimeSeriesSet<double>(*ModelStructure.TestTimeSeries);

    TestInputData = ModelStructure.TestTimeSeries->ToArmaMatShifter(ModelStructure.inputcolumns, ModelStructure.lags);

    CTimeSeriesSet<double> ShiftedInputs(TestInputData,ModelStructure.dt,ModelStructure.lags);

    TestOutputData = ModelStructure.TestTimeSeries->ToArmaMatShifterOutput(ModelStructure.outputcolumns, ModelStructure.lags);

    CTimeSeriesSet<double> ShiftedOutputs = CTimeSeriesSet<double>::OutputShifter(TestOutputData,ModelStructure.dt,ModelStructure.lags);

    Predict(TestInputData, Prediction);
    //cout << "Prediction:" << Prediction;

    return true;
}

bool FFNWrapper_Multi::PerformanceMetrics()
{
    CTimeSeriesSet<double> PredictionData (Prediction,ModelStructure.dt,ModelStructure.lags);
    PredictionData.writetofile(ModelStructure.outputpath + "Prediction_" + to_string(ModelStructure.realization) + ".txt");
    CTimeSeriesSet<double> TargetData = GetOutputData();
    TargetData.writetofile(ModelStructure.outputpath + "Target_" + to_string(ModelStructure.realization) + ".txt");
    nMSE = diff2(PredictionData.BTC[0],TargetData.BTC[0])/(norm2(TargetData.BTC[0])/TargetData.BTC[0].n);
    _R2 = R2(PredictionData.BTC[0],TargetData.BTC[0]);

    return true;
}


bool FFNWrapper_Multi::DataSave()
{
    //Input data checking
    data->writetofile(ModelStructure.outputpath + "data_" + to_string(ModelStructure.realization) + ".csv");

    // Input/Output matrix checking
    TrainInputData.save(ModelStructure.outputpath + "TrainInputData_" + to_string(ModelStructure.realization) + ".csv", arma::file_type::raw_ascii);
    TrainOutputData.save(ModelStructure.outputpath + "TrainOutputData_" + to_string(ModelStructure.realization) + ".csv", arma::file_type::raw_ascii);

    CTimeSeriesSet<double> TrainInputDataTS(TrainInputData,ModelStructure.dt);
    TrainInputDataTS.writetofile(ModelStructure.outputpath + "TrainInputDataTS_" + to_string(ModelStructure.realization) + ".csv");

    CTimeSeriesSet<double> TrainOutputDataTS(TrainOutputData,ModelStructure.dt);
    TrainOutputDataTS.writetofile(ModelStructure.outputpath + "TrainOutputDataTS_" + to_string(ModelStructure.realization) + ".csv");

    //Prediction results
    Prediction.save(ModelStructure.outputpath + "Prediction_" + to_string(ModelStructure.realization) + ".csv",arma::file_type::raw_ascii);

    CTimeSeriesSet<double> PredictionTS(Prediction,ModelStructure.dt);
    PredictionTS.writetofile(ModelStructure.outputpath + "PredictionTS_" + to_string(ModelStructure.realization) + ".csv");

    TestInputData.save(ModelStructure.outputpath + "TestInputData_" + to_string(ModelStructure.realization) + ".txt",arma::file_type::raw_ascii);
    TestOutputData.save(ModelStructure.outputpath + "TestOutputData_" + to_string(ModelStructure.realization) + ".txt",arma::file_type::raw_ascii);

    CTimeSeriesSet<double> TestInputDataTS(TestInputData,ModelStructure.dt);
    TestInputDataTS.writetofile(ModelStructure.outputpath + "TestInputDataTS_" + to_string(ModelStructure.realization) + ".csv");

    CTimeSeriesSet<double> TestOutputDataTS(TestOutputData,ModelStructure.dt);
    TestOutputDataTS.writetofile(ModelStructure.outputpath + "TestOutputDataTS_" + to_string(ModelStructure.realization) + ".csv");

    //Performance metrics

    cout<<"nMSE = "<<nMSE<<endl;
    cout<<"R2 = "<<_R2<<endl;

    return true;
}


bool FFNWrapper_Multi:: Plotter()
{
    CTimeSeriesSet<double> Observed(ModelStructure.observedaddress,true);

    CTimeSeriesSet<double> Predicted(ModelStructure.predictedaddress,true);

    vector<pair<double, double>> plotdata1, plotdata2;
    for (int i=0; i<Observed.maxnumpoints(); i++)
    {
        plotdata1.push_back(make_pair(Observed.BTC[0].GetT(i),Observed.BTC[0].GetC(i)));

    }
    for (int i=0; i<Predicted.maxnumpoints(); i++)
    {
        plotdata2.push_back(make_pair(Predicted.BTC[0].GetT(i),Predicted.BTC[0].GetC(i)));
    }
    // Create a Gnuplot object
    Gnuplot gp;

    // Set titles and labels
    gp << "set title 'Comparison'\n";
    gp << "set xlabel 'Time'\n";
    gp << "set ylabel 'Solids Concentration'\n";
    gp << "set grid\n";  // Optional: Add a grid for better visualization

    // Plot both datasets on the same plot
    gp << "plot '-' with lines title 'Observed', '-' with lines title 'Predicted'\n";
    gp.send1d(plotdata1);  // Send the first dataset (Observed)
    gp.send1d(plotdata2);  // Send the second dataset (Predicted)

    return true;
}


bool FFNWrapper_Multi:: Optimizer()
{
/*
    // Define the objective function to minimize (f(x) = x^2 + 4x + 4).
    class QuadraticFunction
    {
    public:
        // Function value at a given point x.
        double Evaluate(const rowvec& parameters)
        {
            // f(x) = x^2 + 4x + 4
            double x = parameters(0);
            return x * x + 4 * x + 4;
        }

        // Gradient of the objective function.
        void Gradient(const rowvec& parameters, rowvec& gradient)
        {
            // Derivative of f(x) = 2x + 4
            double x = parameters(0);
            gradient(0) = 2 * x + 4;
        }
    };
        // Create an instance of the quadratic function.
        QuadraticFunction f;

        // Initial parameters (let's start at x = 10).
        rowvec initialPoint = {10};

        // Create the optimizer (using Stochastic Gradient Descent in this case).
        ens::SGD optimizer(0.1, 1000, 1e-6);

        // Optimize the function using the gradient descent algorithm.
        optimizer.Optimize(f, initialPoint);

        // Output the result.
        std::cout << "Optimal point: " << initialPoint(0) << std::endl;
        std::cout << "Optimal value: " << f.Evaluate(initialPoint) << std::endl;
*/
        return true;
}