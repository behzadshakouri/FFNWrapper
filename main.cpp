//Our main

#include <mlpack.hpp>
#include <iostream>
#include "ffnwrapper_multi.h"
#include <BTCSet.h>
#include "modelcreator.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include "ga.h"

using namespace mlpack;
using namespace std;


int main()
{
    const char* ompThreads = std::getenv("OMP_NUM_THREADS");
        if (ompThreads) {
            std::cout << "OMP_NUM_THREADS is set to: " << ompThreads << std::endl;
        } else {
            std::cout << "OMP_NUM_THREADS is not set." << std::endl;
        }
    //Model creator (Random model structure)
    ModelCreator modelCreator;
    modelCreator.lag_frequency = 3;
    modelCreator.maximum_superficial_lag = 5;
    modelCreator.total_number_of_columns = 3;
    modelCreator.max_number_of_layers = 3;
    modelCreator.max_lag_multiplier = 10;
    modelCreator.max_number_of_nodes_in_layers = 10;




    string path;
#ifdef Arash
    path = "/home/arash/Projects/FFNWrapper/";
#else
    path = "/home/behzad/Projects/FFNWrapper2/";
#endif

    // Defining Model Structure
    CModelStructure_Multi mymodelstruct;
    mymodelstruct.n_layers = 1;
    mymodelstruct.n_nodes = {7};

    mymodelstruct.dt=0.01;
    string datapath = "/home/arash/Projects/FFNWrapper/";
    string buildpath = "build/Desktop_Qt_5_15_2_GCC_64bit-Debug/";

    bool randommodelstructure = true; // true for random model structure usage and false for no random model structure usage

    // Defining Inputs
    mymodelstruct.inputcolumns.push_back(0); // Input 0: Inflow
    mymodelstruct.inputcolumns.push_back(1); // Input 1: Settling element (1)_Coagulant:external_mass_flow_timeseries
    mymodelstruct.inputcolumns.push_back(2); // Input 2: Reactor (1)_Solids:inflow_concentration

    // Defining Output(s)
    mymodelstruct.outputcolumns.push_back(3); // Output: V(11): Settling element (1)_Solids:concentration

    //Lags definition
    vector<int> lag1; lag1.push_back(0);lag1.push_back(14);
    vector<int> lag2; lag2.push_back(14);
    vector<int> lag3; lag3.push_back(7);lag2.push_back(28);

    mymodelstruct.lags.push_back(lag1);
    mymodelstruct.lags.push_back(lag2);



    for (int r=0; r<2; r++)
    {
        mymodelstruct.inputaddress.push_back(datapath + "observedoutput_" + to_string(r) + ".txt");
        mymodelstruct.testaddress.push_back(datapath + "observedoutput_" + to_string(r) + ".txt");

        mymodelstruct.outputpath = path + "Results/";
        mymodelstruct.observedaddress.push_back(mymodelstruct.outputpath + "TestOutputDataTS_" + to_string(r) + ".csv");
        mymodelstruct.predictedaddress.push_back(mymodelstruct.outputpath + "PredictionTS_" + to_string(r) + ".csv");
    }

    GeneticAlgorithm<ModelCreator> GA;
    GA.Settings.outputpath = mymodelstruct.outputpath;
    GA.model = modelCreator;
    GA.model.FFN.ModelStructure = mymodelstruct;

    ModelCreator OptimizedModel = GA.Optimize();
    cout<<"Optimized Model Structure: " << OptimizedModel.FFN.ModelStructure.ParametersToString().toStdString()<<endl;

    //We can test here:

/*
    {

        QFile results(QString::fromStdString(path) + "modelresults.txt");
        QTextStream out;
        if (results.open(QIODevice::WriteOnly | QIODevice::Text)) {
            out.setDevice(&results);
        } else {
            // Handle file open error
            qDebug() << "Error opening file!";
            return 0;
        }

        if (randommodelstructure) {
            for (int i=0; i<1000; i++) // Random Model Structure Generation
            {

                modelCreator.CreateRandomModelStructure(&mymodelstruct);

                // Running FFNWrapper
                if (mymodelstruct.ValidLags())
                {   FFNWrapper_Multi F;
                    F.ModelStructure = mymodelstruct;
                    F.Initiate();
                    F.Train();
                    F.Test();
                    F.PerformanceMetrics();

                    qDebug()<< "i = " << i << ", " << mymodelstruct.ParametersToString() << ", nMSE = " << F.nMSE << ", R2 = " << F._R2;
                    out << "i = " << i << ", " << mymodelstruct.ParametersToString() << ", nMSE = " << F.nMSE << ", R2 = " << F._R2 << "\n";

                    F.DataSave(datacategory::Train);
                    F.DataSave(datacategory::Test);
                    //F.Plotter();
                    //F.Optimizer();

                    //data::Save("model.xml","model", F);
                }
                else
                    i--;
            }

        results.close();

        }

        else if (!randommodelstructure) {


            FFNWrapper_Multi F;
            F.ModelStructure = mymodelstruct;
            F.Initiate();
            F.Train();
            F.Test();
            F.PerformanceMetrics();

            qDebug()<< mymodelstruct.ParametersToString() << ", nMSE = " << F.nMSE << ", R2 = " << F._R2;
            out << mymodelstruct.ParametersToString() << ", nMSE = " << F.nMSE << ", R2 = " << F._R2 << "\n";

            F.DataSave(datacategory::Test);
            F.Plotter();
            //F.Optimizer();

            //data::Save("model.xml","model", F);

            results.close();
        }

        else
            cout << "No estimation implemented!";

    }
*/
    return 0;
}







//mymodelstruct.inputaddress = datapath + "observedoutput.txt";
//mymodelstruct.testaddress = datapath + "observedoutput.txt";

//mymodelstruct.observedaddress = path + buildpath + "TestOutputDataTS.csv";
//mymodelstruct.predictaddress = path + buildpath + "PredictionTS.csv";
/*


//CModelStructure mymodelstruct2;
//CModelStructure mymodelstruct3(mymodelstruct);
//modelCreator.CreateRandomModelStructure(&mymodelstruct2);

//qDebug()<<"Model2 ?= Model1"<<(mymodelstruct2==mymodelstruct);
//qDebug()<<"Model3 ?= Model1"<<(mymodelstruct3==mymodelstruct);


/*
     *
    // NEW MODEL

    // Defining Model Structure
    CModelStructure mymodelstruct2;

    mymodelstruct2.n_layers = 1;
    mymodelstruct2.n_nodes = {2};

    mymodelstruct2.dt=0.01;

    mymodelstruct2.inputaddress = path + "observedoutput.txt";
    mymodelstruct2.testaddress = path + "observedoutput.txt";

    // Defining Inputs
    mymodelstruct2.inputcolumns.push_back(0); // Input 1: D(2): Settling element (1)_Coagulant:external_mass_flow_timeseries
    mymodelstruct2.inputcolumns.push_back(1); // Input 2: CV(50): Reactor (1)_Solids:inflow_concentration

    //Lags definition
    vector<int> lag11; lag11.push_back(28); //lag1.push_back(20); lag1.push_back(50);
    vector<int> lag22; lag22.push_back(1); //lag2.push_back(10); lag2.push_back(30);
    mymodelstruct2.lags.push_back(lag11);
    mymodelstruct2.lags.push_back(lag22);

    // Defining Output(s)
    mymodelstruct2.outputcolumns.push_back(2); // Output: V(11): Settling element (1)_Solids:concentration

    FFNWrapper F2;
    F2.ModelStructure = mymodelstruct2;
    F2.Initiate();
    F2.Train();
    F2.Test();
    F2.PerformanceMetrics();
    F2.DataSave();
    F2.Plotter();
    F2.Optimizer();

    */
