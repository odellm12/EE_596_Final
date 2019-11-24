#ifndef ImageIncluded
#define ImageIncluded

extern "C" {
#include "tclHash.h"
};

#define NotAnImageType 0
#define QImageType 1
#define FloatImageType 2

typedef unsigned char Byte;

class Image
{
public:
    void* link;
    char* name;
    int width, height, size;

    Image(char* name);
    virtual ~Image();
    virtual int GetType();
    virtual void ImageToBytes(unsigned char* data, int zoom,
        double min, double max);
    virtual void* GetBuffer() { return NULL; }
    void Change();
};

class QImage : public Image
{
public:
    Byte* buffer;

    QImage(char* name);
    virtual ~QImage();
    virtual void* GetBuffer() { return buffer; }
    virtual int GetType();
    virtual void ImageToBytes(unsigned char* data, int zoom,
        double min, double max);
    double interpolate(double x, double y);
    double interpolateExtend(double x, double y);
    double interpolateLogPolar(double x, double y);
    // USED: return the i'th pixel
    unsigned char& data(int i) { return buffer[i]; }
    // USED: return the pixel at (i,j)
    unsigned char& data(int i, int j) { return buffer[i + j * width]; }
    void NewBuffer(Byte* buf, int w, int h);
    Byte* AllocBuffer(int w, int h);
    void FreeBuffer();
    void ZeroBuffer(int w, int h, unsigned char fill = 0);
};

class FloatImage : public Image
{
public:
    float* buffer;

    FloatImage(char* name);
    virtual ~FloatImage();
    virtual int GetType();
    virtual void* GetBuffer() { return buffer; }
    virtual void ImageToBytes(unsigned char* data, int zoom,
        double min, double max);
    double interpolate(double i, double j);
    float& data(int i) { return buffer[i]; }
    float& data(int i, int j) { return buffer[i + j * width]; }
    void NewBuffer(float* buf, int w, int h);
    void FreeBuffer();
    void ZeroBuffer(int w, int h);
};

typedef void (ImageDataFunction)(void* image);
extern ImageDataFunction* UnlinkImageDataFunction;
extern ImageDataFunction* RedrawImageDataFunction;

Image* FindImageWithName(char* name);
QImage* FindQImageWithName(char* name);
FloatImage* FindFloatImageWithName(char* name);

void RotateQImage2(QImage* dest, QImage* src, double angle);
void ZoomQImage(QImage* image, double scale, QImage* src);
void FillRectangle(QImage* image,
    int x1, int y1, int x2, int y2, int color);
void SubSampleWithAveraging(QImage* dest, QImage* src, int scale);
void ResampleQImage(QImage* image, QImage* src,
    int newWidth, int newHeight, double scale);
void ReduceSize(QImage* dest, QImage* src);
void SmoothQImage(QImage* image);
void SaveQImagesPpm(QImage* red, QImage* green, QImage* blue,
    FILE* outf);
void SaveQImagePgm(QImage* image, FILE* outf);
void LoadQImagePgm(QImage* image, FILE* inf);
void LoadQImageRaw(QImage* dest, int width, int height, FILE* inf);
void LoadQImageHsi(QImage* dest, FILE* inf);
void CopyQImage(QImage* dest, QImage* src);
void DifferenceImage(QImage* dest, QImage* src1, QImage* src2);
void ExtrantRectangle(QImage* dest, QImage* src,
    int x, int y, int width, int height);
void ExtractAndZoom(QImage* dest, QImage* src, double scale,
    int minX, int minY, int maxX, int maxY);
void DifferenceFloatImage(QImage* dest, QImage* src1, FloatImage* src2);
void CentroidImage(QImage* image, double* ax, double* ay,
    double* vx, double* vy);
void AddImages(QImage* dest, QImage* src1, QImage* src2);
void Threshold(QImage* image, int threshold, int inv);
void SmartThreshold(QImage* image);
void MirrorQImage(QImage* image);
void MirrorFloatImage(FloatImage* image);
double CountAboveZero(QImage* image);
void EvolveBackgroundImage(FloatImage* background, QImage* dat,
    double rate);
void FindMotion(QImage* dest, QImage* src1, FloatImage* src2,
    int threshold, unsigned char maskValue);
void FindMotion(QImage* dest, QImage* src1, QImage* src2,
    int threshold, unsigned char maskValue);

void Image_Init_NoTcl();

void ExtractAndRotate(QImage* dest, QImage* src, int x, int y,
    int width, int height, double angle);
void ExtractRectangle(QImage* dest, QImage* src, int x, int y,
    int width, int height);

struct WidthHeight {
    int width, height;
};

#endif
