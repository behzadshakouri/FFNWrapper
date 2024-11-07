#include "modelcreator.h"

ModelCreator::ModelCreator()
{

}

bool ModelCreator::CreateModel(model_structure *modelstructure)
{
    vector<int> columns = convertToBase(parameters[0],2);

    //column selection
    for (unsigned int i=0; i<columns.size(); i++)
    {
        if (columns[i]==1) modelstructure->inputcolumns.push_back(i);
    }
    modelstructure->input_lag_multiplier = parameters[1];
    //lag selection
    for (int i=0; i<total_number_of_columns; i++)
    {
        vector<int> lags_onoff = convertToBase(parameters[i+2],lag_frequency);
        vector<int> lags;
        for (unsigned int j=0; j<lags_onoff.size(); j++)
        {
            if (lags_onoff[j]==1 && columns[i]==1) lags.push_back(j*modelstructure->input_lag_multiplier);
        }
        if (lags.size()!=0)
            modelstructure->lags.push_back(lags);
    }
    // nodes in hidden layers;
    vector<int> nodes = convertToBase(parameters[2+total_number_of_columns],max_number_of_nodes_in_layers);
    modelstructure->n_layers = nodes.size();
    modelstructure->n_nodes.resize(nodes.size());
    for (unsigned int i=0; i<nodes.size(); i++)
    {
        modelstructure->n_nodes[i] = nodes[i]+1;
    }
    return true;
}

bool ModelCreator::SetParameters(model_structure *modelstructure)
{
    if (modelstructure->InputTimeSeries==nullptr && total_number_of_columns==0)
    {
        cout<<"Input time series or total number of columns must be provided"<<endl;
        return false;
    }
    else if (total_number_of_columns!=0)
    {

    }
    else
    {
        total_number_of_columns = modelstructure->InputTimeSeries->nvars;
    }

    parameters.resize(ParametersSize());

    // column selection
    for (unsigned int i=0; i<modelstructure->inputcolumns.size(); i++)
        parameters[0]+=pow(2,modelstructure->inputcolumns[i]);
    parameters[1]=modelstructure->input_lag_multiplier;

    //lag selection
    int counter = 0;
    for (int i=0; i<total_number_of_columns; i++)
    {
        if (modelstructure->inputcolumns[counter]==i)
        {   for (int j=0; j<modelstructure->lags[counter].size(); j++)
            {
                parameters[i+2]+=pow(lag_frequency,modelstructure->lags[counter][j]/modelstructure->input_lag_multiplier);

            }
            counter++;
        }
    }

    //nodes in hidden layers
    for (int i=0; i<modelstructure->n_layers; i++)
        parameters[2+total_number_of_columns]+=(modelstructure->n_nodes[i]-1) * pow(max_number_of_nodes_in_layers,i);

    return true;

}

int ModelCreator::ParametersSize()
{
    int out = 2;
    out+=total_number_of_columns;
    out++;
    return out;
}

std::vector<int> convertToBase(unsigned long int number, int base) {
    std::vector<int> result;

    // Handle zero case
    if (number == 0) {
        result.push_back(0);
        return result;
    }

    // Convert the number to the specified base
    while (number > 0) {
        int remainder = number % base;
        result.push_back(remainder);
        number /= base;
    }

    return result;
}
