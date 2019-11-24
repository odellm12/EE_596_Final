#ifndef WINNT
extern "C" {
#endif
#include <stdlib.h>
#include <stdio.h>
#ifndef hpux
#include <math.h>
#endif
#include <assert.h>
#ifndef WINNT
}
#endif

#ifdef hpux
#include <math.h>
#endif

#include "mat.h"
#include "system.h"

// Utility functions

void Crash(char* msg)
{
    fprintf(stderr, msg);
    exit(1);
}

// MatData functions

MatData::MatData(int length)
{
    size = length;
    elements = (Element*)malloc(length * sizeof(Element));
    if (elements == NULL)
        Crash("malloc() failed in allocating matrix/vector data.\n");
    refs = 1;
}

MatData::~MatData()
{
    free(elements);
}

void AddRef(MatData* data)
{
    if (data == NULL) return;
    data->refs++;
}

void DelRef(MatData* data)
{
    if (data == NULL) return;
    data->refs--;
    if (data->refs == 0) delete data;
}

// Mat functions

Mat::Mat()
{
    rows = 0;
    cols = 0;
    size = 0;
    data = NULL;
}

Mat::Mat(const Mat& mat)
{
    rows = mat.rows;
    cols = mat.cols;
    size = mat.size;
    data = mat.data;
    AddRef(data);
}

Mat::Mat(int height, int width)
{
    rows = height;
    cols = width;
    size = height * width;
    data = new MatData(size);
    Element* ptr = data->elements;
    for (int i = 0; i < size; i++) *(ptr++) = 0.0;
}

Mat::Mat(int length)
{
    rows = length;
    cols = 1;
    size = length;
    data = new MatData(length);
    Element* ptr = data->elements;
    for (int i = 0; i < size; i++) *(ptr++) = 0.0;
}

Mat::~Mat()
{
    DelRef(data);
}

void Mat::WritableCopy()
{
    if (data == NULL || data->refs == 1) return;
    MatData* oldData = data;
    data = new MatData(size);
    Element* e1 = data->elements;
    Element* e2 = oldData->elements;
    for (int i = 0; i < size; i++) *(e1++) = *(e2++);
    DelRef(oldData);
}

// Operators

Mat& Mat::operator=(const Mat& src)
{
    DelRef(data);
    rows = src.rows;
    cols = src.cols;
    size = src.size;
    data = src.data;
    AddRef(data);
    return *this;
}

Mat& operator+=(Mat& dest, const Mat& src)
{
    if (dest.rows != src.rows || dest.cols != src.cols)
        Crash("Mismatched sizes when +=ing matrices/vectors.\n");
    dest.WritableCopy();
    Element* e1 = dest.data->elements;
    Element* e2 = src.data->elements;
    for (int i = 0; i < dest.size; i++)
        (*(e1++)) += (*(e2++));
    return dest;
}

Mat& operator-=(Mat& dest, const Mat& src)
{
    if (dest.rows != src.rows || dest.cols != src.cols)
        Crash("Mismatched sizes when -=ing matrices/vectors.\n");
    dest.WritableCopy();
    Element* e1 = dest.data->elements;
    Element* e2 = src.data->elements;
    for (int i = 0; i < dest.size; i++)
        (*(e1++)) -= (*(e2++));
    return dest;
}

Mat& operator*=(Mat& dest, Element c)
{
    dest.WritableCopy();
    Element* e1 = dest.data->elements;
    for (int i = 0; i < dest.size; i++)
        (*(e1++)) *= c;
    return dest;
}

Mat& operator/=(Mat& dest, Element c)
{
    dest.WritableCopy();
    Element* e1 = dest.data->elements;
    for (int i = 0; i < dest.size; i++)
        (*(e1++)) /= c;
    return dest;
}

Mat operator+(const Mat& mat1, const Mat& mat2)
{
    if (mat1.rows != mat2.rows || mat1.cols != mat2.cols)
        Crash("Mismatched sizes when adding matrices/vectors.\n");
    Mat ans(mat1.rows, mat1.cols);
    Element* e0 = ans.data->elements;
    Element* e1 = mat1.data->elements;
    Element* e2 = mat2.data->elements;
    for (int i = 0; i < mat1.size; i++) *(e0++) = (*(e1++)) + (*(e2++));
    return ans;
}

Mat operator-(const Mat& mat1, const Mat& mat2)
{
    if (mat1.rows != mat2.rows || mat1.cols != mat2.cols)
        Crash("Mismatched sizes when subtracting matrices/vectors.\n");
    Mat ans(mat1.rows, mat1.cols);
    Element* e0 = ans.data->elements;
    Element* e1 = mat1.data->elements;
    Element* e2 = mat2.data->elements;
    for (int i = 0; i < mat1.size; i++) *(e0++) = (*(e1++)) - (*(e2++));
    return ans;
}

Mat operator*(const Mat& mat1, const Mat& mat2)
{
    if (mat1.cols != mat2.rows)
        Crash("Mismatched sizes when multiplying matrices/vectors.\n");
    Mat ans(mat1.rows, mat2.cols);
    Element* e0 = ans.data->elements;
    if (mat2.cols == 1)
    {
        Element* e1 = mat1.data->elements;
        for (int i = 0; i < mat1.rows; i++)
        {
            Element tot = 0.0;
            Element* e2 = mat2.data->elements;
            for (int j = 0; j < mat1.cols; j++)
                tot += (*(e1++)) * (*(e2++));
            *(e0++) = tot;
        }
    }
    else if (mat1.rows == 1)
    {
        for (int i = 0; i < mat2.cols; i++)
        {
            Element tot = 0.0;
            Element* e1 = mat1.data->elements;
            Element* e2 = mat2.data->elements + i;
            for (int j = 0; j < mat2.rows; j++)
            {
                tot += (*(e1++)) * (*e2);
                e2 += mat2.cols;
            }
            *(e0++) = tot;
        }
    }
    else if (mat1.cols == 1)
    {
        Element* e1 = mat1.data->elements;
        for (int i = 0; i < mat1.rows; i++)
        {
            Element val = *(e1++);
            Element* e2 = mat2.data->elements;
            for (int j = 0; j < mat2.cols; j++)
                *(e0++) = val * (*(e2++));
        }
    }
    else
    {
        Element* e1base = mat1.data->elements;
        for (int i = 0; i < mat1.rows; i++)
        {
            for (int j = 0; j < mat2.cols; j++)
            {
                Element tot = 0;
                Element* e1 = e1base;
                Element* e2 = mat2.data->elements + j;
                for (int k = 0; k < mat1.cols; k++)
                {
                    tot += (*(e1++)) * (*e2);
                    e2 += mat2.cols;
                }
                *(e0++) = tot;
            }
            e1base += mat1.cols;
        }
    }
    return ans;
}

Mat operator*(Element c, const Mat& mat)
{
    Mat ans(mat.rows, mat.cols);
    Element* e0 = ans.data->elements;
    Element* e1 = mat.data->elements;
    for (int i = 0; i < mat.size; i++) *(e0++) = (*(e1++)) * c;
    return ans;
}

Mat operator*(const Mat& mat, Element c)
{
    Mat ans(mat.rows, mat.cols);
    Element* e0 = ans.data->elements;
    Element* e1 = mat.data->elements;
    for (int i = 0; i < mat.size; i++) *(e0++) = (*(e1++)) * c;
    return ans;
}

Mat operator/(const Mat& mat, Element c)
{
    Mat ans(mat.rows, mat.cols);
    Element* e0 = ans.data->elements;
    Element* e1 = mat.data->elements;
    for (int i = 0; i < mat.size; i++) *(e0++) = (*(e1++)) / c;
    return ans;
}

// Functions

Mat Uninitialized(int length)
{
    Mat ans;
    ans.rows = length;
    ans.cols = 1;
    ans.size = length;
    ans.data = new MatData(ans.size);
    return ans;
}

Mat Uninitialized(int height, int width)
{
    Mat ans;
    ans.rows = height;
    ans.cols = width;
    ans.size = height * width;
    ans.data = new MatData(ans.size);
    return ans;
}

Mat Zero(int length)
{
    Mat ans(length);
    Element* ptr = ans.data->elements;
    for (int i = 0; i < length; i++)
        *(ptr++) = 0.0;
    return ans;
}

Mat Zero(int height, int width)
{
    Mat ans(height, width);
    Element* ptr = ans.data->elements;
    for (int i = 0; i < ans.size; i++)
        *(ptr++) = 0.0;
    return ans;
}

Mat Identity(int size)
{
    Mat ans(size, size);
    Element* ptr = ans.data->elements;
    for (int i = 0; i < ans.size; i++)
        *(ptr++) = 0.0;
    ptr = ans.data->elements;
    for (int i = 0; i < size; i++)
    {
        *ptr = 1.0;
        ptr += size + 1;
    }
    return ans;
}

Mat BasisVector(int length, int number)
{
    Mat ans(length);
    Element* ptr = ans.data->elements;
    for (int i = 0; i < length; i++)
        *(ptr++) = 0.0;
    ans.data->elements[number] = 1.0;
    return ans;
}

Mat Transpose(const Mat& mat)
{
    if (mat.rows == 1 || mat.cols == 1)
    {
        Mat ans = mat;
        ans.rows = mat.cols;
        ans.cols = mat.rows;
        return ans;
    }
    else
    {
        Mat ans(mat.cols, mat.rows);
        Element* e0 = ans.data->elements;
        for (int i = 0; i < ans.rows; i++)
        {
            Element* e1 = mat.data->elements + i;
            for (int j = 0; j < ans.cols; j++)
            {
                *(e0++) = *e1;
                e1 += mat.cols;
            }
        }
        return ans;
    }
}

Mat MultiplyTerms(const Mat& mat1, const Mat& mat2)
{
    if (mat1.rows != mat2.rows || mat1.cols != mat2.cols)
        Crash("Mismatched sizes in multiplying term by term.\n");
    Mat ans(mat1.rows, mat1.cols);
    Element* e0 = ans.data->elements;
    Element* e1 = mat1.data->elements;
    Element* e2 = mat2.data->elements;
    for (int i = 0; i < mat1.size; i++)
        (*(e0++)) = (*(e1++)) * (*(e2++));
    return ans;
}

Mat Normalize(const Mat& mat)
{
    Element* ptr = mat.data->elements;
    Element tot = 0.0;
    for (int i = 0; i < mat.size; i++)
    {
        tot += (*ptr) * (*ptr);
        ptr++;
    }
    if (tot == 0.0) tot = 0.0; else tot = 1.0 / sqrt(tot);
    Mat ans(mat.rows, mat.cols);
    ptr = mat.data->elements;
    Element* dest = ans.data->elements;
    for (int i = 0; i < mat.size; i++)
        *(dest++) = tot * (*(ptr++));
    return ans;
}

Element Dot(const Mat& mat1, const Mat& mat2)
{
    if (mat1.rows != mat2.rows || mat1.cols != mat2.cols)
        Crash("Mismatched sizes in dot product.\n");
    Element* e1 = mat1.data->elements;
    Element* e2 = mat2.data->elements;
    Element tot = 0.0;
    for (int i = 0; i < mat1.size; i++)
        tot += (*(e1++)) * (*(e2++));
    return tot;
}

Element Cross(const Mat& v1, const Mat& v2)
{
    return (v1.data->elements[0]) * (v2.data->elements[1]) -
        (v1.data->elements[1]) * (v2.data->elements[0]);
}

Vec Cross3D(const Mat& v1, const Mat& v2)
{
    Vec ans = Zero(4);
    ans(0) = v1.data->elements[1] * v2.data->elements[2] -
        v1.data->elements[2] * v2.data->elements[1];
    ans(1) = v1.data->elements[2] * v2.data->elements[0] -
        v1.data->elements[0] * v2.data->elements[2];
    ans(2) = v1.data->elements[0] * v2.data->elements[1] -
        v1.data->elements[1] * v2.data->elements[0];
    return ans;
}

Element Abs(const Mat& mat)
{
    Element* ptr = mat.data->elements;
    Element tot = 0.0;
    for (int i = 0; i < mat.size; i++)
    {
        tot += (*ptr) * (*ptr);
        ptr++;
    }
    return sqrt(tot);
}

Mat LUInverse(Mat a, double* det)
{
    a.WritableCopy();
    double determinant = 1.0;
    double sign = 1.0;
    int i, j, k;
    int n = a.rows;
    Mat inv = Identity(n);
    for (i = 0; i < n; i++)
    {
        Element* pPtr = a.data->elements + (i + i * n);
        Element scale = *pPtr;
        int pivot = i;
        for (j = i + 1; j < n; j++)
        {
            pPtr += n;
            if (fabs(*pPtr) > fabs(scale))
            {
                scale = *pPtr;
                pivot = j;
            }
        }
        if (pivot != i)
        {
            sign = -sign;
            Element* a1 = a.data->elements + (i + i * n);
            Element* a2 = a.data->elements + (i + pivot * n);
            Element* i1 = inv.data->elements + i * n;
            Element* i2 = inv.data->elements + pivot * n;
            for (j = 0; j < n; j++) {
                Element tmp = *i1; *(i1++) = *i2; *(i2++) = tmp;
            }
            for (j = i; j < n; j++) {
                Element tmp = *a1; *(a1++) = *a2; *(a2++) = tmp;
            }
        }
        Element* aPtr = a.data->elements + (i + i * n);
        Element* invPtr = inv.data->elements + (i * n);
        *(aPtr++) = 1;
        if (scale == 0.0)
            Crash("Singular matrix!\n");
        determinant += log(fabs(scale));
        if (scale < 0) sign = -sign;
        for (j = 0; j < n; j++) *(invPtr++) /= scale;
        for (j = i + 1; j < n; j++) *(aPtr++) /= scale;
        for (k = i + 1; k < n; k++)
        {
            Element* aSrc = a.data->elements + (i + i * n);
            Element* aDest = a.data->elements + (i + k * n);
            Element* invSrc = inv.data->elements + (i * n);
            Element* invDest = inv.data->elements + (k * n);
            scale = *aDest;
            if (scale != 0.0)
            {
                for (j = 0; j < n; j++) *(invDest++) -= scale * (*(invSrc++));
                for (j = i + 1; j < n; j++) *(++aDest) -= scale * (*(++aSrc));
            }
        }
    }
    for (i = n - 1; i > 0; i--)
    {
        for (k = i - 1; k >= 0; k--)
        {
            Element* invSrc = inv.data->elements + (i * n);
            Element* invDest = inv.data->elements + (k * n);
            Element scale = a(k, i);
            if (scale != 0.0)
                for (j = 0; j < n; j++)
                    *(invDest++) -= scale * (*(invSrc++));
        }
    }
    if (det != NULL) *det = determinant;
    if (sign < 0 && det != NULL)
        fprintf(stderr, "Determinant negative!\n");
    return inv;
}

void PrintMatrix(const Mat& mat)
{
    fprintf(stderr, "(");
    for (int i = 0; i < mat.rows; i++)
    {
        if (i > 0) fprintf(stderr, " ");
        fprintf(stderr, "(");
        for (int j = 0; j < mat.cols; j++)
        {
            fprintf(stderr, "%8.3f", mat.data->elements[i * mat.cols + j]);
            if (j < mat.cols - 1) fprintf(stderr, ", ");
        }
        fprintf(stderr, ")");
        if (i < mat.rows - 1) fprintf(stderr, "\n");
    }
    fprintf(stderr, ")\n");
    fflush(stderr);
}

void PrintVector(const Mat& vec)
{
    fprintf(stderr, "(");
    for (int i = 0; i < vec.rows; i++)
    {
        fprintf(stderr, "%8.3f", vec.data->elements[i]);
        if (i < vec.rows - 1) fprintf(stderr, ", ");
    }
    fprintf(stderr, ")\n");
    fflush(stderr);
}

Vec LoadVec(FILE* inf)
{
    int length;
    assert(inf != NULL);
    //  fprintf(stderr,"loc=%d\n",ftell(inf));
    fread((char*)&length, sizeof(int), 1, inf);
    //  fprintf(stderr,"length=%08X\n",length);
    assert(length != 0);
    //  fprintf(stderr,"LoadVec length=%d, inf=%08X\n",length,inf);
    Mat ans(length);
    double* tem = new double[length];
    fread((char*)tem, sizeof(double), length, inf);
    for (int i = 0; i < length; i++)
        ans.data->elements[i] = ntohd(tem[i]);
    delete[] tem;
    return ans;
}

Vec LoadVec(char* name)
{
    FILE* inf = fopen(name, "r");
    if (inf == NULL) Crash("Cannot open file to load vector.\n");
    Mat ans = LoadVec(inf);
    fclose(inf);
    return ans;
}

Mat LoadMat(FILE* inf)
{
    int height, width;
    fread((char*)&height, sizeof(int), 1, inf);
    fread((char*)&width, sizeof(int), 1, inf);
    Mat ans(height, width);
    double* tem = new double[width * height];
    fread((char*)tem, sizeof(double), ans.size, inf);
    for (int i = 0; i < width * height; i++) ans.data->elements[i] = ntohd(tem[i]);
    delete[] tem;
    return ans;
}

Mat LoadMat(char* name)
{
    FILE* inf = fopen(name, "r");
    if (inf == NULL) Crash("Cannot open file to load matrix.\n");
    Mat ans = LoadMat(inf);
    fclose(inf);
    return ans;
}

void SaveVec(FILE* outf, const Mat& vec)
{
    int t = vec.rows;
    fwrite((char*)&t, sizeof(int), 1, outf);
    double* tem = new double[vec.size];
    for (int i = 0; i < vec.size; i++) tem[i] = htond(vec.data->elements[i]);
    fwrite((char*)tem, sizeof(double), vec.size, outf);
    delete[] tem;
}

void SaveVec(char* name, const Mat& vec)
{
    FILE* outf = fopen(name, "w");
    if (outf == NULL) Crash("Cannot open file to save vector.\n");
    SaveVec(outf, vec);
    fclose(outf);
}

void SaveMat(FILE* outf, const Mat& mat)
{
    int t = mat.rows;
    fwrite((char*)&t, sizeof(int), 1, outf);
    t = mat.cols;
    fwrite((char*)&t, sizeof(int), 1, outf);
    double* tem = new double[mat.size];
    for (int i = 0; i < mat.size; i++) tem[i] = htond(mat.data->elements[i]);
    fwrite((char*)tem, sizeof(double), mat.size, outf);
    delete[] tem;
}

void SaveMat(char* name, const Mat& mat)
{
    FILE* outf = fopen(name, "w");
    if (outf == NULL) Crash("Cannot open file to save matrix.\n");
    SaveMat(outf, mat);
    fclose(outf);
}

Vec Point2D(double x, double y)
{
    Vec ans(2);
    ans(0) = x;
    ans(1) = y;
    return ans;
}

Vec Point3D(double x, double y, double z)
{
    Vec ans(3);
    ans(0) = x;
    ans(1) = y;
    ans(2) = z;
    return ans;
}

Mat Translate2D(double x, double y)
{
    Mat trans = Identity(3);
    trans(0, 2) = x;
    trans(1, 2) = y;
    return trans;
}

Mat Scale2D(double amount)
{
    Mat scale = Identity(3);
    scale(0, 0) = amount;
    scale(1, 1) = amount;
    return scale;
}

Mat Rotate2D(double angle)
{
    Mat rot = Identity(3);
    rot(0, 0) = cos(angle); rot(0, 1) = -sin(angle);
    rot(1, 0) = sin(angle); rot(1, 1) = cos(angle);
    return rot;
}
