

#ifdef __GNUC__
#include <string.h>
#endif

#ifdef hpux
#include <math.h>
#endif

#include <assert.h>
#include <stdio.h>
#ifndef hpux
#include <math.h>
#endif
#include <stdlib.h>
#ifndef __GNUC__
#include <string.h>
#include <limits.h>
#endif
#if defined(sparc) || defined(hpux) || defined(linux)
#include <values.h>
#ifndef INT_MAX
#define INT_MAX MAXINT
#endif
#endif
#ifdef sgi
#include <ulocks.h>
#include <task.h>
#include <assert.h>
#endif

#if defined(sgi) && !defined(NoTcl)
#include "parallel.hpp"
#endif

#undef random
#define random rand

#include "system.h"
#include "fast2.h"

void FastDebugNetwork(FastForwardStruct* net)
{
    /*
    fprintf(stdout, "network=%08X\n", (int)net);
    fprintf(stdout, "numUnits=%d\n", net->numUnits);
    fprintf(stdout, "numConns=%d\n", net->numConns);
    fprintf(stdout, "numGroups=%d\n", net->numGroups);
    fprintf(stdout, "firstInputs=%d\n", net->firstInput);
    fprintf(stdout, "firstHiddens=%d\n", net->firstHidden);
    fprintf(stdout, "firstOutputs=%d\n", net->firstOutput);
    fprintf(stdout, "groups:\n");
    for (int i = 0; i < net->numGroups; i++)
        fprintf(stdout, " (%d,%d)-(%d,%d) %d %d\n",
            net->groupInfo[i].startx, net->groupInfo[i].starty,
            net->groupInfo[i].endx, net->groupInfo[i].endy,
            net->groupInfo[i].numUnits, net->groupInfo[i].firstUnit);
    fprintf(stdout, "activations:\n");
    for (int i = 0; i < net->numUnits; i++)
        fprintf(stdout, " %g %d %d %d\n", net->unitList[i].activation,
            net->unitInfo[i].group, net->unitInfo[i].x, net->unitInfo[i].y);
    fprintf(stdout, "\n\n");
    */
}

FastForwardStruct::~FastForwardStruct()
{
    if (unitList != NULL) delete[] unitList;
    if (unitList != NULL) delete[] connList;
    if (connections != NULL) delete[] connections;
    if (unitInfo != NULL) delete[] unitInfo;
    if (groupInfo != NULL) delete[] groupInfo;
    if (trainBuffer != NULL) delete[] trainBuffer;
    if (unitTrain != NULL) delete[] unitTrain;
    if (connTrain != NULL) delete[] connTrain;
}

// USED
// Compute a forward pass through the given neural network, and return
// the value of the first output unit (which is most useful if the network
// only has a single output).
float FastForwardPass(FastForwardStruct* net)
{
    FastForwardUnit* unit;
    for (unit = net->unitList + net->firstHidden; unit < net->unitList + net->numUnits;
        unit++)
    {
        switch (unit->type) {
        case TanhUnitType: {
            // tanh() activation function, compute weighted sum of inputs
            float tot = 0.0;
            FastForwardConnection* conn;
            for (conn = unit->connections; conn < unit->connections + unit->numInputs;
                conn++)
                tot += conn->weight * conn->from->activation;
            // Non-linearity
            unit->activation = tanh(tot);
            break;
        }
        case LinearUnitType: {
            // linear activation function, compute weighted sum of inputs
            float tot = 0.0;
            FastForwardConnection* conn;
            for (conn = unit->connections; conn < unit->connections + unit->numInputs;
                conn++)
                tot += conn->weight * conn->from->activation;
            unit->activation = tot;
            break;
        }
        case NoisyOrUnitType: {
            // Noisy or, computed by 1-product(1-input)
            float tot = 2.0;
            FastForwardConnection* conn;
            for (conn = unit->connections; conn < unit->connections + unit->numInputs;
                conn++)
                if (conn->from != net->unitList)
                    tot *= 0.5 - 0.5 * conn->from->activation;
            unit->activation = 1.0 - tot;
            break;
        }
        case MultiplyUnitType: {
            float tot = 1;
            FastForwardConnection* conn;
            for (conn = unit->connections; conn < unit->connections + unit->numInputs;
                conn++)
                if (conn->from != net->unitList)
                    tot *= conn->from->activation;
            unit->activation = tot;
            break;
        }
        case SumUnitType: {
            float tot = 0;
            FastForwardConnection* conn;
            for (conn = unit->connections; conn < unit->connections + unit->numInputs;
                conn++)
                if (conn->from != net->unitList)
                    tot += conn->from->activation;
            unit->activation = tot;
            break;
        }
        case Vote3UnitType: {
            // Voting among three inputs, computed as a "soft" boolean function,
            // in which AND is represented by multiplication of numbers in the
            // range 0-1, and NOT is 1-num.
            FastForwardConnection* conn = unit->connections;
            if (conn->from == net->unitList) conn++;
            float y1 = 0.5 + 0.5 * (conn++)->from->activation;
            if (conn->from == net->unitList) conn++;
            float y2 = 0.5 + 0.5 * (conn++)->from->activation;
            if (conn->from == net->unitList) conn++;
            float y3 = 0.5 + 0.5 * (conn++)->from->activation;
            unit->activation = 1.0 - 2.0 * (1.0 - y1 * y2) * (1.0 - y2 * y3) * (1.0 - y1 * y3);
            break;
        }
        default: {
            fprintf(stderr, "Unknown unit type %d (FastForwardPass)\n", unit->type);
            exit(1);
            break;
        }
        }
    }
    return net->unitList[net->firstOutput].activation;
}

void FastCompileNetwork(FastForwardStruct* net, char* filename, char* netname,
    double max)
{
    FILE* outf = fopen(filename, "w");
    fprintf(outf, "#include <math.h>\n");
    fprintf(outf, "void %s(int *inputs, float *outputs) {\n", netname);
    double inScale = 1.0 / max;
    double inOffset = 0;
    for (int i = net->firstHidden; i < net->numUnits; i++)
    {
        FastForwardUnit* unit = net->unitList + i;
        double offset = 0.0;
        if (i >= net->firstOutput)
            fprintf(outf, "  outputs[%d]=tanhf(", i - net->firstOutput); else
            fprintf(outf, "  float u%d=tanhf(", i);
        for (FastForwardConnection* conn = unit->connections;
            conn < unit->connections + unit->numInputs;
            conn++) {
            int from = conn->from - net->unitList;
            if (from == 0) offset += conn->from->activation * conn->weight; else {
                if (from < net->firstHidden) {
                    offset += inOffset * conn->weight;
                    fprintf(outf, "(float)%e*(float)inputs[%d]+\n", conn->weight * inScale, from - 1);
                }
                else
                    fprintf(outf, "(float)%e*u%d+\n", conn->weight, from);
            }
        }
        fprintf(outf, "%e);\n", offset);
    }
    fprintf(outf, "}\n\n");
    fclose(outf);
}

void FastCompileNetwork2(FastForwardStruct* net, char* filename,
    char* netname, int max)
{
    FILE* outf = fopen(filename, "w");
    fprintf(outf, "#include <math.h>\n");
    fprintf(outf, "void %s(int *inputs, float *outputs) {\n", netname);
    for (int i = net->firstHidden; i < net->firstOutput; i++)
        fprintf(outf, "  long a%d, u%d;\n", i, i);
    for (int i = net->firstOutput; i < net->numUnits; i++)
        fprintf(outf, "  long a%d;\n", i);
    for (int i = net->firstHidden; i < net->numUnits; i++)
    {
        FastForwardUnit* unit = net->unitList + i;
        long offset;
        fprintf(outf, "  a%d=", i);
        for (FastForwardConnection* conn = unit->connections;
            conn < unit->connections + unit->numInputs;
            conn++) {
            int from = conn->from - net->unitList;
            if (from == 0) {
                offset = (long)floor(0.5 + conn->from->activation * conn->weight *
                    65536.0 * 65536.0);
            }
            else if (from < net->firstHidden) {
                long val = (long)floor(0.5 + conn->weight * 65536.0 * 65536.0 / max);
                fprintf(outf, "%ldL*(long)inputs[%d]+\n", val, from - 1);
            }
            else {
                long val = (long)floor(0.5 + conn->weight * 65536.0);
                fprintf(outf, "%ldL*u%d+\n", val, from);
            }
        }
        fprintf(outf, "%ldL;\n", offset);
        if (i >= net->firstOutput) {
            fprintf(outf, "  outputs[%d]=(float)(tanh(a%d/(65536.0*65536.0)));\n",
                i - net->firstOutput, i);
        }
        else {
            fprintf(outf,
                "  u%d=(long)floor(0.5+65536.0*tanh(a%d/(65536.0*65536.0)));\n",
                i, i);
        }
    }
    fprintf(outf, "}\n\n");
    fclose(outf);
}

// USED
// Allocate space for the training patterns and activations of each unit
// for the training patterns.
void FastMakeNetTrainable(FastForwardStruct* net, int patterns)
{
    // If we allocated before, free it up
    if (net->patterns > 0) {
        delete[] net->trainBuffer;
        delete[] net->unitTrain;
        delete[] net->connTrain;
        net->trainBuffer = NULL;
        net->unitTrain = NULL;
        net->connTrain = NULL;
    }
    net->patterns = patterns;
    if (patterns > 0) {
        // Buffer to hold the activations and deltas for each unit
        int bufferLen = patterns * (2 * (net->numHiddens + net->numOutputs) +
            net->numInputs);
        net->trainBuffer = new float[bufferLen];
        for (int p = 0; p < bufferLen; p++) net->trainBuffer[p] = 0.0;
        // Structure for pointing to activation and delta buffers
        net->unitTrain = new FastUnitTrain[net->numUnits];
        // Structure for holding previous deltas
        net->connTrain = new FastConnectionTrain[net->numConns];
        int conns = 0;
        float* bufferPtr = net->trainBuffer;
        for (int i = 0; i < net->numUnits; i++) {
            net->unitList[i].train = net->unitTrain + i;
#ifdef NewTrain
            net->unitTrain[i].numOutputs = 0;
#endif
            net->unitTrain[i].conns = net->connTrain + conns;
            net->unitTrain[i].activations = bufferPtr;
            bufferPtr += patterns;
            if (net->unitList[i].type == InputUnitType) {
                net->unitTrain[i].delta = NULL;
            }
            else {
                net->unitTrain[i].delta = bufferPtr;
                bufferPtr += patterns;
            }
#ifndef NewTrain
            for (int c = 0; c < net->unitList[i].numInputs; c++)
                net->connTrain[conns + c].prevDelta = 0.0;
            conns += net->unitList[i].numInputs;
#endif
            if (i == 0)                    // Set the bias unit activations to 1
                for (int p = 0; p < patterns; p++)
                    net->unitList[i].train->activations[p] = 1.0;
        }
#ifdef NewTrain
        for (int c = 0; c < net->numConns; c++)
            net->connections[c].from->train->numOutputs++;
        for (i = 0; i < net->numUnits; i++)
            if (net->unitList[i].train->numOutputs > 0) {
                net->unitList[i].train->conns = net->connTrain + conns;
                conns += net->unitList[i].train->numOutputs;
                net->unitList[i].train->numOutputs = 0;
            }
            else
                net->unitList[i].train->conns = NULL;
        for (c = 0; c < net->numConns; c++) {
            int from = net->connList[c].from, to = net->connList[c].to;
            FastConnectionTrain* ct = net->unitList[from].train->conns +
                net->unitList[from].train->numOutputs;
            ct->weight = &(net->connList[c].conn->weight);
            ct->prevDelta = 0;
            ct->to = net->unitList + net->connList[c].to;
            net->unitList[from].train->numOutputs++;
        }
#endif
    }
    else {
        // Clear out pointers
        for (int i = 0; i < net->numUnits; i++)
            net->unitList[i].train = NULL;
    }
}

// USED
// Train a network on a batch of examples (which are assumed to already
// be written into the network), using the given learning rate and momentum
void FastTrainBatch(FastForwardStruct* net,
    double rate, double momentum, double decay)
{
    // First we do a forward pass: For each hidden or output unit
    for (FastForwardUnit* unit = net->unitList + net->firstHidden;
        unit < net->unitList + net->numUnits;
        unit++)
    {
        switch (unit->type) {     // Switch based on non-linearity type
        case TanhUnitType: {
            // tanh() activation function
            for (int p = 0; p < net->patterns; p++) {
                // Weighted sum of inputs
                float tot = 0.0;
                FastForwardConnection* conn;
                for (conn = unit->connections; conn < unit->connections + unit->numInputs;
                    conn++)
                    tot += conn->weight * conn->from->train->activations[p];
                // Non-linearity
                unit->train->activations[p] = tanh(tot);
            }
            break;
        }
        case LinearUnitType: {
            // tanh() activation function
            for (int p = 0; p < net->patterns; p++) {
                // Weighted sum of inputs
                float tot = 0.0;
                FastForwardConnection* conn;
                for (conn = unit->connections; conn < unit->connections + unit->numInputs;
                    conn++)
                    tot += conn->weight * conn->from->train->activations[p];
                // Non-linearity
                unit->train->activations[p] = tot;
            }
            break;
        }
        case NoisyOrUnitType: {
            // Noisy or, computed by 1-product(1-input)
            for (int p = 0; p < net->patterns; p++) {
                float tot = 2.0;
                FastForwardConnection* conn;
                for (conn = unit->connections; conn < unit->connections + unit->numInputs;
                    conn++)
                    if (conn->from != net->unitList)
                        tot *= 0.5 - 0.5 * conn->from->train->activations[p];
                unit->train->activations[p] = 1.0 - tot;
            }
            break;
        }
        case MultiplyUnitType: {
            for (int p = 0; p < net->patterns; p++) {
                float tot = 1;
                FastForwardConnection* conn;
                for (conn = unit->connections; conn < unit->connections + unit->numInputs;
                    conn++)
                    if (conn->from != net->unitList)
                        tot *= conn->from->train->activations[p];
                unit->train->activations[p] = tot;
            }
            break;
        }
        case SumUnitType: {
            for (int p = 0; p < net->patterns; p++) {
                float tot = 0;
                FastForwardConnection* conn;
                for (conn = unit->connections; conn < unit->connections + unit->numInputs;
                    conn++)
                    if (conn->from != net->unitList)
                        tot += conn->from->train->activations[p];
                unit->train->activations[p] = tot;
            }
            break;
        }
        case Vote3UnitType: {
            // Voting among three inputs, computed as a "soft" boolean function,
            // in which AND is represented by multiplication of numbers in the
            // range 0-1, and NOT is 1-num.
            for (int p = 0; p < net->patterns; p++) {
                FastForwardConnection* conn = unit->connections;
                if (conn->from == net->unitList) conn++;
                float y1 = 0.5 + 0.5 * (conn++)->from->train->activations[p];
                if (conn->from == net->unitList) conn++;
                float y2 = 0.5 + 0.5 * (conn++)->from->train->activations[p];
                if (conn->from == net->unitList) conn++;
                float y3 = 0.5 + 0.5 * (conn++)->from->train->activations[p];
                unit->train->activations[p] =
                    1.0 - 2.0 * (1.0 - y1 * y2) * (1.0 - y2 * y3) * (1.0 - y1 * y3);
            }
            break;
        }
        default: {
            fprintf(stderr, "Unknown unit type %d (Sweep Forward)\n", unit->type);
            exit(1);
            break;
        }
        }
    }

#ifndef NewTrain
    // Initialize the deltas to zero for all units except the outputs
    for (FastForwardUnit* unit = net->unitList + net->firstHidden;
        unit < net->unitList + net->firstOutput; unit++)
        for (int p = 0; p < net->patterns; p++)
            unit->train->delta[p] = 0.0;
#endif

    // The output deltas are the difference between the desired activation
    // (which is placed in the delta field) and the actual activation
    for (FastForwardUnit* unit = net->unitList + net->firstOutput; unit < net->unitList + net->numUnits;
        unit++) {
        for (int p = 0; p < net->patterns; p++)
            unit->train->delta[p] -= unit->train->activations[p];
    }

    // Now we do a backward pass, for all units except the inputs
    for (FastForwardUnit* unit = net->unitList + net->numUnits - 1;
        unit >= net->unitList + net->firstHidden;
        unit--)
    {
        switch (unit->type) {
#ifndef NewTrain
        case TanhUnitType: {
            // First, add the tanh'(sum weighted inputs) term to delta for unit
            for (int p = 0; p < net->patterns; p++) {
                unit->train->delta[p] = (1.0 - unit->train->activations[p] *
                    unit->train->activations[p]) *
                    unit->train->delta[p];
            }
            // Learning rate for connections into unit is overall learning
            // rate divided by number of inputs
            double thisRate = rate / unit->numInputs;

            for (int c = 0; c < unit->numInputs; c++) {
                FastForwardConnection* conn = unit->connections + c;
                FastForwardUnit* from = conn->from;
                float delta = 0;
                if (from->type == InputUnitType) {
                    for (int p = 0; p < net->patterns; p++) {
                        // Accumulate the weight change for this connection
                        delta += from->train->activations[p] * unit->train->delta[p];
                    }
                }
                else {
                    for (int p = 0; p < net->patterns; p++) {
                        // Accumulate the weight change for this connection
                        delta += from->train->activations[p] * unit->train->delta[p];
                        // Propogate delta to nodes feeding this one
                        from->train->delta[p] += conn->weight * unit->train->delta[p];
                    }
                }
                // Apply the weight change, taking into account the previous
                // step and momentum
                conn->weight +=
                    thisRate * (delta + momentum * unit->train->conns[c].prevDelta);
                unit->train->conns[c].prevDelta = delta;
                conn->weight -= conn->weight * decay;
            }
            break;
        }
#else
        case TanhUnitType: {
            // First, add the tanh'(sum weighted inputs) term to delta for unit
            for (int p = 0; p < net->patterns; p++) {
                FastConnectionTrain* conn = unit->train->conns;
                double delta = 0;
                for (int c = 0; c < unit->train->numOutputs; c++, conn++)
                    delta += (*conn->weight) * conn->to->train->delta[p];
                unit->train->delta[p] = delta *
                    (1.0 - unit->train->activations[p] * unit->train->activations[p]);
            }
            // Learning rate for connections into unit is overall learning
            // rate divided by number of inputs
            double thisRate = rate / unit->numInputs;
            for (int c = 0; c < unit->numInputs; c++) {
                FastForwardConnection* conn = unit->connections + c;
                FastForwardUnit* from = conn->from;
                float delta = 0;
                // Accumulate the weight change for this connection
                for (int p = 0; p < net->patterns; p++)
                    delta += from->train->activations[p] * unit->train->delta[p];
                // Apply the weight change, taking into account the previous
                // step and momentum
                conn->weight +=
                    thisRate * (delta + momentum * unit->train->conns[c].prevDelta);
                unit->train->conns[c].prevDelta = delta;
                conn->weight -= conn->weight * decay;
            }
            break;
        }
#endif
        case LinearUnitType: {
            // Learning rate for connections into unit is overall learning
            // rate divided by number of inputs
            double thisRate = rate / unit->numInputs;

            for (int c = 0; c < unit->numInputs; c++) {
                FastForwardConnection* conn = unit->connections + c;
                FastForwardUnit* from = conn->from;
                float delta = 0;
                if (from->type == InputUnitType) {
                    for (int p = 0; p < net->patterns; p++) {
                        // Accumulate the weight change for this connection
                        delta += from->train->activations[p] * unit->train->delta[p];
                    }
                }
                else {
                    for (int p = 0; p < net->patterns; p++) {
                        // Accumulate the weight change for this connection
                        delta += from->train->activations[p] * unit->train->delta[p];
                        // Propogate delta to nodes feeding this one
                        from->train->delta[p] += conn->weight * unit->train->delta[p];
                    }
                }
                // Apply the weight change, taking into account the previous
                // step and momentum
                conn->weight +=
                    thisRate * (delta + momentum * unit->train->conns[c].prevDelta);
                unit->train->conns[c].prevDelta = delta;
                conn->weight -= conn->weight * decay;
            }
            break;
        }
        case NoisyOrUnitType: {
            for (int p = 0; p < net->patterns; p++) {
                FastForwardConnection* conn, * conn2;
                for (conn = unit->connections; conn < unit->connections + unit->numInputs;
                    conn++)
                {
                    float tot = unit->train->delta[p];
                    for (conn2 = unit->connections;
                        conn2 < unit->connections + unit->numInputs;
                        conn2++)
                        if (conn2->from != net->unitList && conn != conn2)
                            tot *= 0.5 - 0.5 * conn2->from->train->activations[p];
                    conn->from->train->delta[p] += tot;
                }
            }
            break;
        }
        case MultiplyUnitType: {
            for (int p = 0; p < net->patterns; p++) {
                FastForwardConnection* conn, * conn2;
                float tot = unit->train->delta[p] * unit->train->activations[p];
                for (conn = unit->connections; conn < unit->connections + unit->numInputs;
                    conn++) {
                    if (conn->from->type != InputUnitType) {
                        float tot = unit->train->delta[p];
                        for (conn2 = unit->connections;
                            conn2 < unit->connections + unit->numInputs;
                            conn2++)
                            if (conn != conn2)
                                tot *= conn2->from->train->activations[p];
                        conn->from->train->delta[p] += tot;
                    }
                }
            }
            break;
        }
        case SumUnitType: {
            for (int p = 0; p < net->patterns; p++) {
                FastForwardConnection* conn, * conn2;
                float tot = unit->train->delta[p] * unit->train->activations[p];
                for (conn = unit->connections; conn < unit->connections + unit->numInputs;
                    conn++) {
                    if (conn->from->type != InputUnitType)
                        conn->from->train->delta[p] += unit->train->delta[p];
                }
            }
            break;
        }
        case Vote3UnitType: {
            for (int p = 0; p < net->patterns; p++) {
                FastForwardConnection* conn = unit->connections;
                if (conn->from == net->unitList) conn++;
                float y1 = 0.5 + 0.5 * (conn++)->from->train->activations[p];
                if (conn->from == net->unitList) conn++;
                float y2 = 0.5 + 0.5 * (conn++)->from->train->activations[p];
                if (conn->from == net->unitList) conn++;
                float y3 = 0.5 + 0.5 * (conn++)->from->train->activations[p];
                conn = unit->connections;
                if (conn->from == net->unitList) conn++;
                (conn++)->from->train->delta[p] -= unit->train->delta[p] *
                    (1 - y2 * y3) * (-y2 * (1 - y1 * y3) - y3 * (1 - y1 * y2));
                if (conn->from == net->unitList) conn++;
                (conn++)->from->train->delta[p] -= unit->train->delta[p] *
                    (1 - y1 * y3) * (-y1 * (1 - y2 * y3) - y3 * (1 - y1 * y2));
                if (conn->from == net->unitList) conn++;
                (conn++)->from->train->delta[p] -= unit->train->delta[p] *
                    (1 - y1 * y2) * (-y2 * (1 - y1 * y3) - y1 * (1 - y2 * y3));
            }
            break;
        }
        default: {
            fprintf(stderr, "Unknown unit type %d (Sweep Backward)\n", unit->type);
            exit(1);
            break;
        }
        }
    }
}

FastForwardStruct* FastLoad(char* netFile)
{
    char* netList[1];
    netList[0] = netFile;
    FastForwardStruct* net = FastLoadMerge(1, netList);
    FastLoadMergeWeights(net, 1, netList);
    return net;
}

// USED
// Given a list network file names, add a ".wet" to the name, and load
// the weights from those files.  This function loads multiple sets of
// weights into a single network, and assuming that the same set of network
// architectures has been loaded by FastLoadMerge.
void FastLoadMergeWeights(FastForwardStruct* fastnet,
    int numNetworks, char* nets[])
{
    int conn = 0;
    for (int net = 0; net < numNetworks; net++)
    {
        char name[1024];
        sprintf(name, "%s.wet", nets[net]);
        FILE* wet = fopen(name, "r");
        if (wet == NULL) continue;
        int weights;
        fscanf(wet, "%*d epochs\n%d weights\n", &weights);
        for (int i = 0; i < weights; i++)
            fscanf(wet, "%f\n", &(fastnet->connList[conn++].conn->weight));
        fclose(wet);
    }
}

// USED
// Write the weights of a NN
void FastWriteWeights(FastForwardStruct* fastnet, FILE* outf)
{
    fprintf(outf, "0 epochs\n%d weights\n", fastnet->numConns);
    for (int i = 0; i < fastnet->numConns; i++)
        fprintf(outf, "%e\n", fastnet->connList[i].conn->weight);
}

// USED
// Read the weights of a NN
void FastReadWeights(FastForwardStruct* fastnet, FILE* inf)
{
    int weights, conn = 0;
    fscanf(inf, "%*d epochs\n%d weights\n", &weights);
    for (int i = 0; i < weights; i++)
        fscanf(inf, "%f\n", &(fastnet->connList[conn++].conn->weight));
}

// USED
// Initialize the weights of a NN randomly, in the range -(1/num inputs) to
// (1/num inputs), where num inputs is the number of connections going into
// the unit this connection feeds into.
void FastRandomizeWeights(FastForwardStruct* fastnet)
{
    for (int i = 0; i < fastnet->numConns; i++) {
        double max = 1.0 / sqrt(double(fastnet->unitList[fastnet->connList[i].to].numInputs));
        fastnet->connList[i].conn->weight =
            (2.0 * ((double)(random()) / (double)(INT_MAX)) - 1.0) * max;
    }
}

// USED
// Load one or more networks into a single network.  The input is the number
// of networks to load, and either a list of names of network files (to which
// ".net" will be appended), or a list of FILE pointers from which the networks
// will be read.  The networks are merged such that they share inputs, then
// the hidden units are grouped together, and finally the outputs from each
// network are listed one after another.
FastForwardStruct* FastLoadMerge(int numNetworks, char* nets[], FILE** netFile)
{
    // If a vector of FILE pointers was not given, use the network names in
    // the nets array to open the files
    int filesGiven = (netFile != NULL);
    if (!filesGiven) {
        netFile = new FILE * [numNetworks];
        for (int net = 0; net < numNetworks; net++) {
            char name[1024];
            sprintf(name, "%s.net", nets[net]);
            netFile[net] = fopen(name, "r");
            if (netFile[net] == NULL) {
                fprintf(stderr, "Could not open %s\n", name);
            }
            assert(netFile[net] != NULL);
        }
    }
    // Record some numbers for each network
    int* netInputs = new int[numNetworks];
    int* netHiddens = new int[numNetworks];
    int* netOutputs = new int[numNetworks];
    int* netUnits = new int[numNetworks];
    int* netConns = new int[numNetworks];
    int totalUnits = 0, totalInputs = 0, totalHiddens = 0, totalOutputs = 0;
    int totalConns = 0;

    // Read in the number of units and number of inputs for each network
    // The inputs are merged together, but all other units must be
    // represented separately
    for (int net = 0; net < numNetworks; net++) {
        fscanf(netFile[net], "%d units\n%d inputs\n",
            &netUnits[net], &netInputs[net]);
        totalInputs = netInputs[net];
        totalUnits += netUnits[net] - netInputs[net];
    }

    // Read x,y,group information for each input unit
    int* unitx = new int[totalUnits + totalInputs];
    int* unity = new int[totalUnits + totalInputs];
    int* unitg = new int[totalUnits + totalInputs];
    for (int net = 0; net < numNetworks; net++) {
        for (int i = 0; i < netInputs[net]; i++)
            fscanf(netFile[net], "%*d %d %d %d\n",
                unitg + i, unity + i, unitx + i);
    }

    // Read the number of hidden units, the hidden units themselves,
    // and the number of output units
    int ptr = totalInputs;
    for (int net = 0; net < numNetworks; net++) {
        fscanf(netFile[net], "\n%d hiddens\n", &netHiddens[net]);
        totalHiddens += netHiddens[net];
        for (int i = 0; i < netHiddens[net]; i++) {
            fscanf(netFile[net], "%*d %d %d %d\n",
                unitg + ptr, unity + ptr, unitx + ptr);
            ptr++;
        }
        fscanf(netFile[net], "\n%d outputs\n", &netOutputs[net]);
        totalOutputs += netOutputs[net];
    }

    // Read information on the output units, and the number of connections
    for (int net = 0; net < numNetworks; net++) {
        for (int i = 0; i < netOutputs[net]; i++) {
            fscanf(netFile[net], "%*d %d %d %d\n",
                unitg + ptr, unity + ptr, unitx + ptr);
            ptr++;
        }
        fscanf(netFile[net], "\n%d conns\n", &netConns[net]);
        totalConns += netConns[net];
    }

    // Begin creating the data structure
    FastForwardStruct* fastnet = new FastForwardStruct;
    totalUnits += totalInputs;
    fastnet->numUnits = totalUnits;
    fastnet->numInputs = totalInputs;
    fastnet->numHiddens = totalHiddens;
    fastnet->numOutputs = totalOutputs;
    fastnet->numGroups = 1;

    // Figure out how many groups there were
    for (int i = 0; i < fastnet->numUnits; i++)
        if (unitg[i] + 1 > fastnet->numGroups)
            fastnet->numGroups = unitg[i] + 1;

    fastnet->unitInfo = new FastUnitInfo[fastnet->numUnits];
    fastnet->groupInfo = new FastGroupInfo[fastnet->numGroups];
    fastnet->patterns = 0;
    fastnet->trainBuffer = NULL;
    fastnet->unitTrain = NULL;
    fastnet->connTrain = NULL;
    fastnet->firstInput = 1;
    fastnet->firstHidden = totalInputs;
    fastnet->firstOutput = totalInputs + totalHiddens;
    fastnet->unitList = new FastForwardUnit[fastnet->numUnits];
    fastnet->connList = new FastForwardConnectionRef[totalConns];
    fastnet->connections = new FastForwardConnection[totalConns];
    fastnet->numConns = totalConns;

    for (int i = 0; i < fastnet->numGroups; i++) {
        fastnet->groupInfo[i].startx = -1;
        fastnet->groupInfo[i].endx = -1;
        fastnet->groupInfo[i].starty = -1;
        fastnet->groupInfo[i].endy = -1;
        fastnet->groupInfo[i].numUnits = 0;
        fastnet->groupInfo[i].firstUnit = -1;
    }

    for (int i = 0; i < fastnet->numUnits; i++) {
        // Initialize each unit
        fastnet->unitList[i].numInputs = 0;
        fastnet->unitList[i].activation = 1.0;
        if (i == 0) fastnet->unitList[i].type = BiasUnitType; else
            if (i < totalInputs) fastnet->unitList[i].type = InputUnitType; else
                fastnet->unitList[i].type = TanhUnitType;
        fastnet->unitInfo[i].x = unitx[i];
        fastnet->unitInfo[i].y = unity[i];
        fastnet->unitInfo[i].group = unitg[i];
        // Figure out the bounding dimensions of each group
        if (fastnet->groupInfo[unitg[i]].startx == -1 ||
            fastnet->groupInfo[unitg[i]].startx > unitx[i])
            fastnet->groupInfo[unitg[i]].startx = unitx[i];
        if (fastnet->groupInfo[unitg[i]].starty == -1 ||
            fastnet->groupInfo[unitg[i]].starty > unity[i])
            fastnet->groupInfo[unitg[i]].starty = unity[i];
        if (fastnet->groupInfo[unitg[i]].endx == -1 ||
            fastnet->groupInfo[unitg[i]].endx < unitx[i])
            fastnet->groupInfo[unitg[i]].endx = unitx[i];
        if (fastnet->groupInfo[unitg[i]].endy == -1 ||
            fastnet->groupInfo[unitg[i]].endy < unity[i])
            fastnet->groupInfo[unitg[i]].endy = unity[i];
        fastnet->groupInfo[unitg[i]].numUnits++;
        if (fastnet->groupInfo[unitg[i]].firstUnit == -1)
            fastnet->groupInfo[unitg[i]].firstUnit = i;
    }

    int firstHidden = totalInputs;
    int firstOutput = totalInputs + totalHiddens;

    if (!filesGiven) {
        // If we are reading from files, then try locating the .type file
        for (int net = 0; net < numNetworks; net++) {
            char name[1024];
            sprintf(name, "%s.type", nets[net]);
            FILE* inf = fopen(name, "r");
            if (inf == NULL) continue;
            for (int i = 0; i < netInputs[net]; i++) fscanf(inf, "%*d");
            for (int i = 0; i < netHiddens[net]; i++)
                fscanf(inf, "%d", &(fastnet->unitList[firstHidden++].type));
            for (int i = 0; i < netOutputs[net]; i++)
                fscanf(inf, "%d", &(fastnet->unitList[firstOutput++].type));
            fclose(inf);
        }
    }

    // Read in the connections
    firstHidden = totalInputs;
    firstOutput = totalInputs + totalHiddens;
    int firstConn = 0;
    for (int net = 0; net < numNetworks; net++) {
        for (int i = 0; i < netConns[net]; i++) {
            int from, to;
            fscanf(netFile[net], "%d %d -1\n", &from, &to);
            if (from >= netHiddens[net] + netInputs[net])
                from += firstOutput - netHiddens[net] - netInputs[net]; else
                if (from >= netInputs[net])
                    from += firstHidden - netInputs[net];
            if (to >= netHiddens[net] + netInputs[net])
                to += firstOutput - netHiddens[net] - netInputs[net]; else
                if (to >= netInputs[net])
                    to += firstHidden - netInputs[net];
            fastnet->unitList[to].numInputs++;
            fastnet->connList[firstConn].from = from;
            fastnet->connList[firstConn].to = to;
            firstConn++;
        }
        if (!filesGiven) fclose(netFile[net]);
        firstHidden += netHiddens[net];
        firstOutput += netOutputs[net];
    }

    // Count the number of connections going into each unit
    firstConn = 0;
    for (int i = 0; i < totalUnits; i++) {
        if (fastnet->unitList[i].numInputs == 0)
            fastnet->unitList[i].connections = NULL; else {
            fastnet->unitList[i].connections = fastnet->connections + firstConn;
            firstConn += fastnet->unitList[i].numInputs;
            fastnet->unitList[i].numInputs = 0;
        }
    }

    // Associate the connections going into each unit with the unit itself
    for (int i = 0; i < totalConns; i++) {
        FastForwardConnection* conn =
            fastnet->unitList[fastnet->connList[i].to].connections +
            (fastnet->unitList[fastnet->connList[i].to].numInputs++);
        conn->from = fastnet->unitList + fastnet->connList[i].from;
        fastnet->connList[i].conn = conn;
    }

    // Clean up
    if (!filesGiven) delete[] netFile;
    delete[] netInputs;
    delete[] netHiddens;
    delete[] netOutputs;
    delete[] netUnits;
    delete[] netConns;
    delete[] unitx;
    delete[] unity;
    delete[] unitg;
    return fastnet;
}

void FastWriteTypes(FastForwardStruct* net, char* filename)
{
    FILE* outf = fopen(filename, "w");
    for (int i = 0; i < net->numUnits; i++)
        fprintf(outf, "%d\n", net->unitList[i].type);
    fclose(outf);
}

void FastWriteActivations(FastForwardStruct* net, char* filename)
{
    FILE* outf = fopen(filename, "w");
    for (int i = 0; i < net->numUnits; i++)
        fprintf(outf, "i=%d, t=%d, a=%g\n", i, net->unitList[i].type,
            net->unitList[i].activation);
    fclose(outf);
}

void FastReadActivations(FastForwardStruct* net, char* filename)
{
    FILE* inf = fopen(filename, "r");
    for (int i = 0; i < net->numUnits; i++)
        fscanf(inf, "i=%*d, t=%*d, a=%g\n", &(net->unitList[i].activation));
    fclose(inf);
}

void FastWriteNet(FastForwardStruct* net, FILE* outf)
{
    fprintf(outf, "%d units\n", net->numUnits);
    fprintf(outf, "%d inputs\n", net->firstHidden);
    for (int i = 0; i < net->firstHidden; i++) fprintf(outf, "0 0 0 0\n");
    fprintf(outf, "\n%d hiddens\n", net->firstOutput - net->firstHidden);
    for (int i = 0; i < net->firstOutput - net->firstHidden; i++)
        fprintf(outf, "0 0 0 0\n");
    fprintf(outf, "\n%d outputs\n", net->numUnits - net->firstOutput);
    for (int i = 0; i < net->numUnits - net->firstOutput; i++)
        fprintf(outf, "0 0 0 0\n");
    fprintf(outf, "\n%d conns\n", net->numConns);
    for (int i = 0; i < net->numConns; i++) fprintf(outf, "%d %d -1\n",
        net->connList[i].from,
        net->connList[i].to);
}

void FastWriteNet(FastForwardStruct* net, char* filename)
{
    FILE* outf = fopen(filename, "w");
    if (outf == NULL) exit(1);
    FastWriteNet(net, outf);
    fclose(outf);
}

float* FastInputWeights(FastForwardStruct* net, int from, int to,
    int* width, int* height, int* num)
{
    *width = net->groupInfo[from].endx - net->groupInfo[from].startx + 1;
    *height = net->groupInfo[from].endy - net->groupInfo[from].starty + 1;
    *num = net->groupInfo[to].numUnits;
    float* buf = (float*)malloc(sizeof(float) * (*width) * (*height) * (*num));
    int i;
    for (int i = 0; i < (*width) * (*height) * (*num); i++) buf[i] = 0;
    for (int i = 0; i < net->numConns; i++) {
        if (net->unitInfo[net->connList[i].to].group == to &&
            net->unitInfo[net->connList[i].from].group == from) {
            int band = net->connList[i].to - net->groupInfo[to].firstUnit;
            int x = net->unitInfo[net->connList[i].from].x - net->groupInfo[from].startx;
            int y = net->unitInfo[net->connList[i].from].y - net->groupInfo[from].starty;
            //      if (x==0 && y==0) fprintf(stderr,"foo: %d (to=%d,band=%d)\n",i,to,band);
            buf[band * (*width) * (*height) + y * (*width) + x] =
                net->connList[i].conn->weight;
        }
    }
    return buf;
}

float* FastOutputWeights(FastForwardStruct* net, int from, int to,
    int* width, int* height, int* num)
{
    *width = net->groupInfo[to].endx - net->groupInfo[to].startx + 1;
    *height = net->groupInfo[to].endy - net->groupInfo[to].starty + 1;
    *num = net->groupInfo[from].numUnits + 1;
    float* buf = (float*)malloc(sizeof(float) * (*width) * (*height) * (*num));
    int i;
    for (int i = 0; i < (*width) * (*height) * (*num); i++) buf[i] = 0;
    for (int i = 0; i < net->numConns; i++) {
        int fromGroup = net->unitInfo[net->connList[i].from].group;
        if (fromGroup == from || fromGroup == 0) {
            int band = (fromGroup == 0) ? 0 :
                1 + net->connList[i].from - net->groupInfo[from].firstUnit;
            int x = net->unitInfo[net->connList[i].to].x - net->groupInfo[to].startx;
            int y = net->unitInfo[net->connList[i].to].y - net->groupInfo[to].starty;
            buf[band * (*width) * (*height) + y * (*width) + x] =
                net->connList[i].conn->weight;
        }
    }
    return buf;
}
