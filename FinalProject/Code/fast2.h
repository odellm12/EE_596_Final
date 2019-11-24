#ifndef FAST2_H
#define FAST2_H

#ifndef WINNT
extern "C" {
#endif
#include <stdio.h>
#ifndef WINNT
}
#endif

#define TanhUnitType 0
#define SigmoidUnitType 1
#define NoisyOrUnitType 2
#define NoisyAndUnitType 3
#define BiasUnitType 5
#define InputUnitType 5
#define Vote3UnitType 6
#define LinearUnitType 7
#define MultiplyUnitType 8
#define SumUnitType 9

#ifdef sgi
#define MaxParallel 4
#endif

//#define NewTrain

struct FastForwardUnit;

struct FastForwardConnection {
    float weight;
    FastForwardUnit* from;
};

#ifndef NewTrain
struct FastConnectionTrain {
    float prevDelta;
#ifdef sgi
    float partialDelta[MaxParallel];
#endif
};
#endif

#ifdef NewTrain
struct FastConnectionTrain {
    float prevDelta;
    float* weight;
    FastForwardUnit* to;
};
#endif

struct FastUnitTrain {
    FastConnectionTrain* conns;
    int numOutputs;
    float* activations;
    float* delta;
};

struct FastForwardConnectionRef {
    FastForwardConnection* conn;
    int from, to;
};

struct FastForwardUnit {
    float activation;
    int numInputs, type;
    FastForwardConnection* connections;
    FastUnitTrain* train;
};

struct FastUnitInfo {
    int x, y, group;
};

struct FastGroupInfo {
    int startx, starty, endx, endy;
    int numUnits;
    int firstUnit;
};

struct FastForwardStruct {
    int patterns;
    int numUnits, numInputs, numHiddens, numOutputs;
    int firstInput;
    int firstHidden;
    int firstOutput;
    int numConns;
    int numGroups;
    FastUnitInfo* unitInfo;
    FastGroupInfo* groupInfo;
    float* trainBuffer;
    FastForwardUnit* unitList;
    FastForwardConnectionRef* connList;
    FastForwardConnection* connections;
    FastUnitTrain* unitTrain;
    FastConnectionTrain* connTrain;
    ~FastForwardStruct();
};

FastForwardStruct* FastLoad(char* netFile);
FastForwardStruct* FastLoadMerge(int numNetworks, char* nets[],
    FILE** netFILE = NULL);
void FastLoadMergeWeights(FastForwardStruct* net,
    int numNetworks, char* nets[]);
void FastReadWeights(FastForwardStruct* net, FILE* inf);
void FastWriteWeights(FastForwardStruct* net, FILE* outf);
void FastRandomizeWeights(FastForwardStruct* net);
float FastForwardPass(FastForwardStruct* net);

void FastMakeNetTrainable(FastForwardStruct* net, int patterns);
void FastTrainBatch(FastForwardStruct* net,
    double rate, double momentum, double decay);
void FastTrainBatchParallelPartial(FastForwardStruct* net,
    int id, int processors);
void FastTrainBatchParallelCombine(FastForwardStruct* net,
    double rate, double momentum, double decay,
    int processors);
void FastWriteTypes(FastForwardStruct* net, char* filename);
void FastWriteNet(FastForwardStruct* net, FILE* outf);
void FastWriteNet(FastForwardStruct* net, char* filename);

void FastWriteActivations(FastForwardStruct* net, char* filename);
void FastReadActivations(FastForwardStruct* net, char* filename);

void FastDebugNetwork(FastForwardStruct* net);

void FastCompileNetwork(FastForwardStruct* net, char* filename, char* netname,
    double max);
void FastCompileNetwork2(FastForwardStruct* net, char* filename,
    char* netname, int max);

float* FastInputWeights(FastForwardStruct* net, int from, int to,
    int* width, int* height, int* num);
float* FastOutputWeights(FastForwardStruct* net, int from, int to,
    int* width, int* height, int* num);

#endif
