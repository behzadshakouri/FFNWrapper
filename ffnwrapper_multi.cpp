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

#include <CTransformation.h>


FFNWrapper_Multi::FFNWrapper_Multi():FFN<MeanSquaredError>()
{

}

FFNWrapper_Multi::FFNWrapper_Multi(const FFNWrapper_Multi &rhs):FFN<MeanSquaredError>(rhs)
{
    ModelStructure = rhs.ModelStructure;
    TrainInputData = rhs.TrainInputData;
    TrainOutputData = rhs.TrainOutputData;
    TestInputData = rhs.TestInputData;
    TestOutputData = rhs.TestOutputData;

}

FFNWrapper_Multi& FFNWrapper_Multi::operator=(const FFNWrapper_Multi& rhs)
{
    FFN<MeanSquaredError>::operator=(rhs);
    ModelStructure = rhs.ModelStructure;
    TrainInputData = rhs.TrainInputData;
    TrainOutputData = rhs.TrainOutputData;
    TestInputData = rhs.TestInputData;
    TestOutputData = rhs.TestOutputData;

    return *this;
}
FFNWrapper_Multi::~FFNWrapper_Multi()
{

}


bool FFNWrapper_Multi::Initiate(bool dataprocess) // Initiating data
{

    DataProcess();

    //Initialize the network
    if (!dataprocess)
        FFN::operator=(FFN<MeanSquaredError>());

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

bool FFNWrapper_Multi::DataProcess()
{

    Shifter(datacategory::Train);
    Transformation();

    return true;
}



bool FFNWrapper_Multi::Shifter(datacategory DataCategory) // Shifting the data according to lags
{
    segment_sizes.clear();

    if (DataCategory == datacategory::Train)
    {
        TrainInputData.clear();
        TrainOutputData.clear();
        for (unsigned int i=0; i<ModelStructure.trainaddress.size(); i++)
        {   CTimeSeriesSet<double> InputTimeSeries(ModelStructure.trainaddress[i],true);

            //Shifting by lags definition (Inputs)

            mat TrainInputData1 = InputTimeSeries.ToArmaMatShifter(ModelStructure.inputcolumns, ModelStructure.lags);

            //CTimeSeriesSet<double> ShiftedInputs(TrainInputData,ModelStructure.dt,ModelStructure.lags);
            //ShiftedInputs.writetofile("ShiftedInputs.txt");

            //Shifting by lags definition (Outputs)
            mat TrainOutputData1 = InputTimeSeries.ToArmaMatShifterOutput(ModelStructure.outputcolumns, ModelStructure.lags);

            if (i==0)
            {
                TrainInputData = TrainInputData1;
                TrainOutputData = TrainOutputData1;
            }
            else
            {
                TrainInputData = arma::join_rows(TrainInputData, TrainInputData1); // Behzad, not sure if it should be join_cols or join_rows, we need to test
                TrainOutputData = arma::join_rows(TrainOutputData, TrainOutputData1);
            }
            segment_sizes.push_back(TrainInputData1.n_cols);

        }

        CTimeSeriesSet<double> ShiftedInputs(TrainInputData,ModelStructure.dt,ModelStructure.lags); // Behzad, This part is to test the shifter. We can comment out after the test.
        ShiftedInputs.writetofile("ShiftedInputsTrain.txt");
        CTimeSeriesSet<double> ShiftedOutputs = CTimeSeriesSet<double>::OutputShifter(TrainOutputData,ModelStructure.dt,ModelStructure.lags);
        ShiftedOutputs.writetofile("ShiftedOutputsTrain.txt");
    }
    else
    {
        TestInputData.clear();
        TestOutputData.clear();
        for (unsigned int i=0; i<ModelStructure.testaddress.size(); i++)
        {
            CTimeSeriesSet<double> InputTimeSeries(ModelStructure.testaddress[i],true);

            //Shifting by lags definition (Inputs)

            mat TestInputData1 = InputTimeSeries.ToArmaMatShifter(ModelStructure.inputcolumns, ModelStructure.lags);

            //CTimeSeriesSet<double> ShiftedInputs(TrainInputData,ModelStructure.dt,ModelStructure.lags);
            //ShiftedInputs.writetofile("ShiftedInputs.txt");

            //Shifting by lags definition (Outputs)
            mat TestOutputData1 = InputTimeSeries.ToArmaMatShifterOutput(ModelStructure.outputcolumns, ModelStructure.lags);

            if (i==0)
            {
                TestInputData = TestInputData1;
                TestOutputData = TestOutputData1;
            }
            else
            {
                TestInputData = arma::join_rows(TestInputData, TestInputData1); // Behzad, not sure if it should be join_cols or join_rows, we need to test
                TestOutputData = arma::join_rows(TestOutputData, TestOutputData1);
            }
            segment_sizes.push_back(TestInputData1.n_cols);
        }

        CTimeSeriesSet<double> ShiftedInputs(TestInputData,ModelStructure.dt,ModelStructure.lags); // Behzad, This part is to test the shifter. We can comment out after the test.
        ShiftedInputs.writetofile("ShiftedInputsTest.txt");
        CTimeSeriesSet<double> ShiftedOutputs = CTimeSeriesSet<double>::OutputShifter(TestOutputData,ModelStructure.dt,ModelStructure.lags);
        ShiftedOutputs.writetofile("ShiftedOutputsTest.txt");
    }
    return true;
}


bool FFNWrapper_Multi::Transformation()
{

    //Testing CTransformation class
    CTransformation transformer;
    arma::mat inputdata = TrainInputData;

    //std::cout << "Original Data:\n" << inputdata << std::endl;

    // Normalize data
    arma::mat normalizedInputData = transformer.normalize(inputdata);
    //std::cout << "Normalized Data:\n" << normalizedInputData << std::endl;
    normalizedInputData.save("/home/behzad/Projects/FFNWrapper2/ASM/Results/nid.txt", arma::file_type::raw_ascii);

    // Save parameters
    transformer.saveParameters("/home/behzad/Projects/FFNWrapper2/ASM/Results/scaling_params.txt");

    // Load parameters
    CTransformation newInputTransformer;
    newInputTransformer.loadParameters("/home/behzad/Projects/FFNWrapper2/ASM/Results/scaling_params.txt");

    // Inverse transform
    arma::mat restoredInputData = newInputTransformer.inverseTransform(normalizedInputData);
    //std::cout << "Restored Data:\n" << restoredInputData << std::endl;

    TrainInputData = normalizedInputData;

    return true;
}


bool FFNWrapper_Multi::Train()
{

    // Train the model
    FFN::Train(TrainInputData, TrainOutputData);

    return true;
}

bool FFNWrapper_Multi::Test() // Predicting test data
{
    // Use the Predict method to get the predictions.

    Shifter(datacategory::Test);
    Predict(TestInputData, Prediction);
    //cout << "Prediction:" << Prediction;

    return true;
}

bool FFNWrapper_Multi::PerformanceMetrics() // Calculating performance metrics
{

    CTimeSeriesSet<double> PredictionData (Prediction,ModelStructure.dt,ModelStructure.lags);
    vector<CTimeSeriesSet<double>> PredictionDataSplit = CTimeSeriesSet<double>::GetFromArmaMatandSplit(Prediction,ModelStructure.dt,ModelStructure.lags,segment_sizes);
    if (!silent)
        for (unsigned int i=0; i<PredictionDataSplit.size(); i++)
            PredictionDataSplit[i].writetofile(ModelStructure.outputpath + "Prediction_" + to_string(i) + ".txt");
    CTimeSeriesSet<double> TargetData = GetOutputData();

    vector<CTimeSeriesSet<double>> TargetDataSplit = CTimeSeriesSet<double>::GetFromArmaMatandSplit(TestOutputData,ModelStructure.dt,ModelStructure.lags,segment_sizes);
    if (!silent)
        for (unsigned int i=0; i<TargetDataSplit.size(); i++)
            TargetDataSplit[i].writetofile(ModelStructure.outputpath + "Target_" + to_string(i) + ".txt");
    nMSE = diff2(PredictionData.BTC[0],TargetData.BTC[0])/(norm2(TargetData.BTC[0])/TargetData.BTC[0].n);
    _R2 = R2(PredictionData.BTC[0],TargetData.BTC[0]);

    return true;
}


bool FFNWrapper_Multi::DataSave(datacategory DataCategory) // Saving data
{
    if (silent) return false;

    if (DataCategory==datacategory::Train)
    {   // Input/Output matrix checking
        TrainInputData.save(ModelStructure.outputpath + "TrainInputData.csv", arma::file_type::raw_ascii);
        TrainOutputData.save(ModelStructure.outputpath + "TrainOutputData.csv", arma::file_type::raw_ascii);

        vector<CTimeSeriesSet<double>> TrainInputSplit = CTimeSeriesSet<double>::GetFromArmaMatandSplit(TrainInputData,ModelStructure.dt,ModelStructure.lags,segment_sizes);
        for (unsigned int i=0; i<TrainInputSplit.size(); i++)
            TrainInputSplit[i].writetofile(ModelStructure.outputpath + "TrainInputDataTS_" + to_string(i) + ".csv");

        vector<CTimeSeriesSet<double>> TrainOutputSplit = CTimeSeriesSet<double>::GetFromArmaMatandSplit(TrainOutputData,ModelStructure.dt,ModelStructure.lags,segment_sizes);
        for (unsigned int i=0; i<TrainOutputSplit.size(); i++)
            TrainOutputSplit[i].writetofile(ModelStructure.outputpath + "TrainOutputDataTS_" + to_string(i) + ".csv");
    }
    else if (DataCategory==datacategory::Test)
    {   //Prediction results
        Prediction.save(ModelStructure.outputpath + "Prediction.csv",arma::file_type::raw_ascii);

        vector<CTimeSeriesSet<double>> PredictionSplit = CTimeSeriesSet<double>::GetFromArmaMatandSplit(Prediction,ModelStructure.dt,ModelStructure.lags,segment_sizes);
        for (unsigned int i=0; i<PredictionSplit.size(); i++)
            PredictionSplit[i].writetofile(ModelStructure.outputpath + "PredictionTS_" + to_string(i) + ".csv");

        TestInputData.save(ModelStructure.outputpath + "TestInputData.txt",arma::file_type::raw_ascii);
        TestOutputData.save(ModelStructure.outputpath + "TestOutputData.txt",arma::file_type::raw_ascii);

        CTimeSeriesSet<double> TestInputTS(TestInputData,ModelStructure.dt,ModelStructure.lags);
        TestInputTS.writetofile(ModelStructure.outputpath + "TestInputTS_All.csv");

        CTimeSeriesSet<double> TestOutputTS(TestOutputData,ModelStructure.dt,ModelStructure.lags);
        TestOutputTS.writetofile(ModelStructure.outputpath + "TestOutputTS_All.csv");

        vector<CTimeSeriesSet<double>> TestInputSplit = CTimeSeriesSet<double>::GetFromArmaMatandSplit(TestInputData,ModelStructure.dt,ModelStructure.lags,segment_sizes);
        for (unsigned int i=0; i<TestInputSplit.size(); i++)
            TestInputSplit[i].writetofile(ModelStructure.outputpath + "TestInputDataTS_" + to_string(i) + ".csv");


        vector<CTimeSeriesSet<double>> TestOutputSplit = CTimeSeriesSet<double>::GetFromArmaMatandSplit(TestOutputData,ModelStructure.dt,ModelStructure.lags,segment_sizes);
        for (unsigned int i=0; i<TestOutputSplit.size(); i++)
            TestOutputSplit[i].writetofile(ModelStructure.outputpath + "TestOutputDataTS_" + to_string(i) + ".csv");

        //Performance metrics
    }
    cout<<"nMSE = "<<nMSE<<endl;
    cout<<"R2 = "<<_R2<<endl;

    return true;
}


bool FFNWrapper_Multi:: Plotter() // Plotting the results
{
    for (unsigned int i=0; i<ModelStructure.observedaddress.size(); i++)
    {   CTimeSeriesSet<double> Observed(ModelStructure.observedaddress[i],true);

        CTimeSeriesSet<double> Predicted(ModelStructure.predictedaddress[i],true);

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
        gp << "set ylabel 'Concentration'\n";
        gp << "set grid\n";  // Optional: Add a grid for better visualization

        // Plot both datasets on the same plot
        gp << "plot '-' with lines title 'Observed', '-' with lines title 'Predicted'\n";
        gp.send1d(plotdata1);  // Send the first dataset (Observed)
        gp.send1d(plotdata2);  // Send the second dataset (Predicted)
    }
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
