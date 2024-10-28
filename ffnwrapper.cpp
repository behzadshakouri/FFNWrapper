#include "ffnwrapper.h"
#include <mlpack.hpp>

using namespace mlpack;
using namespace std;

FFNWrapper::FFNWrapper():FFN<>()
{

}

FFNWrapper::FFNWrapper(const FFNWrapper &rhs):FFN<>(rhs)
{
    ModelStructure = rhs.ModelStructure;
    input_data = rhs.input_data;
    output_data = rhs.output_data;
}

FFNWrapper& FFNWrapper::operator=(const FFNWrapper& rhs)
{
    FFN<>::operator=(rhs);
    ModelStructure = rhs.ModelStructure;
    input_data = rhs.input_data;
    output_data = rhs.output_data;
    return *this;
}
FFNWrapper::~FFNWrapper()
{

}

bool Train()
{

    // Load the training set and testing set.
    arma::mat trainData;
    data::Load("thyroid_train.csv", trainData, true);
    arma::mat testData;
    data::Load("thyroid_test.csv", testData, true);

    FFNWrapper F;
    FFNWrapper F1 = F;
    FFNWrapper F2(F1);
    // Split the labels from the training set and testing set respectively.
    // Decrement the labels by 1, so they are in the range 0 to (numClasses - 1).
    arma::mat trainLabels = trainData.row(trainData.n_rows - 1) - 1;
    arma::mat testLabels = testData.row(testData.n_rows - 1) - 1;
    trainData.shed_row(trainData.n_rows - 1);
    testData.shed_row(testData.n_rows - 1);

    // Initialize the network.
    FFN<> model;
    model.Add<Linear>(8);
    model.Add<Sigmoid>();
    model.Add<Linear>(3);
    model.Add<LogSoftMax>();

    // Train the model.
    model.Train(trainData, trainLabels);

    // Use the Predict method to get the predictions.
    arma::mat predictionTemp;
    model.Predict(testData, predictionTemp);

    /*
    Since the predictionsTemp is of dimensions (3 x number_of_data_points)
    with continuous values, we first need to reduce it to a dimension of
    (1 x number_of_data_points) with scalar values, to be able to compare with
    testLabels.

    The first step towards doing this is to create a matrix of zeros with the
    desired dimensions (1 x number_of_data_points).

    In predictionsTemp, the 3 dimensions for each data point correspond to the
    probabilities of belonging to the three possible classes.
  */
    arma::mat prediction = arma::zeros<arma::mat>(1, predictionTemp.n_cols);

    // Find index of max prediction for each data point and store in "prediction"
    for (size_t i = 0; i < predictionTemp.n_cols; ++i)
    {
        prediction(i) = arma::as_scalar(arma::find(
            arma::max(predictionTemp.col(i)) == predictionTemp.col(i), 1));
    }

    /*
    Compute the error between predictions and testLabels,
    now that we have the desired predictions.
  */
    size_t correct = arma::accu(prediction == testLabels);
    double classificationError = 1 - double(correct) / testData.n_cols;

    // Print out the classification error for the testing dataset.
    std::cout << "Classification Error for the Test set: " << classificationError << std::endl;
    //return 0;

    return true;
}
