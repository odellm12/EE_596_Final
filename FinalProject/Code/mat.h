#ifndef MAT_H
#define MAT_H


#ifndef WINNT
extern "C" {
#endif
#include <stdlib.h>
#include <stdio.h>
#ifndef hpux
#include <math.h>
#endif
#ifndef WINNT
}
#endif

#ifdef hpux
#include <math.h>
#endif

typedef double Element;

class MatData
{
public:
    Element* elements;
    int size, refs;

public:
    MatData(int length);
    ~MatData();
};

class Mat
{
public:
    MatData* data;
    int rows, cols, size;

    void WritableCopy();

public:
    Mat();
    Mat(const Mat& mat);
    Mat(int length);
    Mat(int height, int width);
    ~Mat();

    Element& operator()(int i) {
        if (data->refs > 1) WritableCopy();
        return data->elements[i];
    }
    Element& operator()(int i, int j) {
        if (data->refs > 1) WritableCopy();
        return data->elements[i * cols + j];
    }
    Mat& operator=(const Mat& src);
};

Mat& operator+=(Mat& dest, const Mat& src);
Mat& operator-=(Mat& dest, const Mat& src);
Mat& operator*=(Mat& dest, Element c);
Mat& operator/=(Mat& dest, Element c);
Mat operator+(const Mat& mat1, const Mat& mat2);
Mat operator-(const Mat& mat1, const Mat& mat2);
Mat operator*(const Mat& mat1, const Mat& mat2);
Mat operator*(const Mat& mat, Element c);
Mat operator*(Element c, const Mat& mat);
Mat operator/(const Mat& mat, Element c);

typedef Mat Vec;

Vec Uninitialized(int length);
Vec Zero(int length);
Vec BasisVector(int length, int number);

Mat Uninitialized(int height, int width);
Mat Zero(int height, int width);
Mat Identity(int size);

Mat Transpose(const Mat& mat);
Mat MultiplyTerms(const Mat& mat1, const Mat& mat2);
Mat Normalize(const Mat& mat);
Mat LUInverse(Mat a, double* logDet = NULL);

Vec Cross3D(const Mat& v1, const Mat& v2);
Element Cross(const Mat& v1, const Mat& v2);
Element Dot(const Mat& mat1, const Mat& mat2);
Element Abs(const Mat& mat);

void PrintMatrix(const Mat& mat);
void PrintVector(const Mat& vec);

Vec LoadVec(FILE* inf);
Vec LoadVec(char* name);
Mat LoadMat(FILE* inf);
Mat LoadMat(char* name);

void SaveVec(FILE* outf, const Mat& vec);
void SaveVec(char* name, const Mat& vec);
void SaveMat(FILE* outf, const Mat& mat);
void SaveMat(char* name, const Mat& mat);

Vec Point2D(double x, double y);
Vec Point3D(double x, double y, double z);

Mat Translate2D(double x, double y);
Mat Scale2D(double amount);
Mat Rotate2D(double angle);

#endif
