#ifndef GeneticAlgorithm_H
#define GeneticAlgorithm_H

#include <vector>

#include "Binary.h"
#include "individual.h"

struct GeneticAlgorithmsettings
{
    unsigned int totalpopulation = 40;
    unsigned int generations = 50;
};

using namespace std;

template<class T>
class GeneticAlgorithm
{
public:
    GeneticAlgorithm();
    void Optimize();
    void AssignFitnesses();
    void Initialize();
    vector<Individual> Individuals;
    T model;
    vector<T> models;
    GeneticAlgorithmsettings Settings;



};

#include <ga.hpp>

#endif // GeneticAlgorithm_H