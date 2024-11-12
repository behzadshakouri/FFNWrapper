//Our main

#include <iostream>
#include <mlpack.hpp>
#include "ffnwrapper.h"
#include <BTCSet.h>
#include "modelcreator.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>

using namespace mlpack;
using namespace std;


int main()
{

    // Defining Model Structure
    CModelStructure mymodelstruct;
    mymodelstruct.n_layers = 1;
    mymodelstruct.n_nodes = {4};
    /*
    mymodelstruct.dt=0.01;
    */

    /*
    mymodelstruct.inputcolumns.push_back(1);
    mymodelstruct.n_input_layers=4;
    mymodelstruct.activation_function="Sigmoid";
    mymodelstruct.n_output_layers=1;
    */

    /*mymodelstruct.input_lag_multiplier = 5;
    modelCreator.max_number_of_nodes_in_layers = 7;
    mymodelstruct.n_layers = 2;
    mymodelstruct.n_nodes = {3,2};
    modelCreator.SetParameters(&mymodelstruct);
    */

    string path;
#ifdef Arash
    path = "/home/arash/Projects/FFNWrapper/";
#else
    path = "/home/behzad/Projects/FFNWrapper2/";
#endif

    mymodelstruct.inputaddress = path + "observedoutput.txt";
    mymodelstruct.testaddress = path + "observedoutput.txt";
    // Defining Inputs
    mymodelstruct.inputcolumns.push_back(0); // Input 1: D(2): Settling element (1)_Coagulant:external_mass_flow_timeseries
    mymodelstruct.inputcolumns.push_back(1); // Input 2: CV(50): Reactor (1)_Solids:inflow_concentration

    // Defining Output(s)
    mymodelstruct.outputcolumns.push_back(2); // Output: V(11): Settling element (1)_Solids:concentration

    //Lags definition
    vector<int> lag1; lag1.push_back(0); lag1.push_back(20); lag1.push_back(50);
    vector<int> lag2; lag2.push_back(0); lag2.push_back(10); lag2.push_back(30);
    mymodelstruct.lags.push_back(lag1);
    mymodelstruct.lags.push_back(lag2);

    //Model creator
    ModelCreator modelCreator;
    modelCreator.lag_frequency = 3;
    modelCreator.maximum_superficial_lag = 3;
    modelCreator.total_number_of_columns = 2;
    modelCreator.max_number_of_layers = 2;
    modelCreator.max_lag_multiplier = 6;

    QFile results(QString::fromStdString(path) + "modelresults.txt");
    QTextStream out;
    if (results.open(QIODevice::WriteOnly | QIODevice::Text)) {
        out.setDevice(&results);
    } else {
       // Handle file open error
       qDebug() << "Error opening file!";
       return 0;
    }

    for (int i=0; i<100; i++)

    {

    modelCreator.CreateRandomModelStructure(&mymodelstruct);

    CModelStructure mymodelstruct2;
    CModelStructure mymodelstruct3(mymodelstruct);
    modelCreator.CreateRandomModelStructure(&mymodelstruct2);

    //qDebug()<<"Model2 ?= Model1"<<(mymodelstruct2==mymodelstruct);
    //qDebug()<<"Model3 ?= Model1"<<(mymodelstruct3==mymodelstruct);

    // Running FFNWrapper
    if (mymodelstruct.ValidLags())
    {   FFNWrapper F;
        F.ModelStructure = mymodelstruct;
        F.Initiate();
        F.Training();
        F.Testing();
        F.PerformanceMetrics();

        qDebug()<< "i = " << i << ", " << mymodelstruct.ParametersToString() << ", MSE = " << F.nMSE << ", R2 = " << F._R2;
        out << "i = " << i << ", " << mymodelstruct.ParametersToString() << ", MSE = " << F.nMSE << ", R2 = " << F._R2 << "\n";

        F.DataSave();


        //data::Save("model.xml","model", F);
    }
    else
        i--;
    }
    results.close();

    return 0;
}
