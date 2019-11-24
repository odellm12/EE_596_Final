#include "img.h"
// Image library that can be used with Tcl code (no Tk dependencies)
//#define UseXpm

#ifdef __GNUC__
#include <string.h>
#endif

#ifdef hpux
#include <math.h>
#endif

#ifndef hpux
#include <math.h>
#endif
#ifndef __GNUC__
#include <string.h>
#endif
#ifdef UseXpm
#include "xpm.h"
#endif
#include <assert.h>

#include "system.hpp"
#include "gif.hpp"
#include "fft.hpp"

ImageDataFunction* UnlinkImageDataFunction;
ImageDataFunction* RedrawImageDataFunction;

static Tcl_HashTable ImageTable;

Image::Image(char* n)
{
    if (n == NULL) name = NULL; else
    {
        name = strdup(n);
        Tcl_HashEntry* entry = Tcl_FindHashEntry(&ImageTable, name);
        if (entry != NULL)
        {
            delete (Image*)(Tcl_GetHashValue(entry));
        }
        int newentry;
        entry = Tcl_CreateHashEntry(&ImageTable, name, &newentry);
        if (!newentry)
        {
            fprintf(stderr, "Create image: duplicate name '%s'.\n", name);
            exit(1);
        }
        Tcl_SetHashValue(entry, this);
    }
    width = 0;
    height = 0;
    size = 0;
    link = NULL;
}

Image::~Image()
{
    if (link != NULL) UnlinkImageDataFunction(link);
    if (name != NULL)
    {
        Tcl_HashEntry* entry = Tcl_FindHashEntry(&ImageTable, name);
        if (entry != NULL) Tcl_DeleteHashEntry(entry);
        free(name);
    }
}

int Image::GetType()
{
    return NotAnImageType;
}

void Image::ImageToBytes(unsigned char* data, int zoom,
    double min, double max)
{
}

void Image::Change()
{
    if (link != NULL) RedrawImageDataFunction(link);
}

// USED
// Create an image object with byte pixels
QImage::QImage(char* name) : Image(name)
{
    buffer = NULL;
}

// USED
// Discard an image object with byte pixels
QImage::~QImage()
{
    if (buffer != NULL) free(buffer);
}

// USED
// Indicate that this image has byte pixels
int QImage::GetType()
{
    return QImageType;
}

double QImage::interpolate(double x, double y)
{
    int left = (int)(floor(x));
    int right = (int)(ceil(x));
    int top = (int)(floor(y));
    int bottom = (int)(ceil(y));
    double fracLeft = x - left;
    double fracTop = y - top;
    double ll, lr, ul, ur;
    if (left < 0 || right >= width || top < 0 || bottom >= height) return 0;
    ll = data(left, bottom);
    lr = data(right, bottom);
    ul = data(left, top);
    ur = data(right, top);
    return (fracTop * ((1.0 - fracLeft) * ll + fracLeft * lr) +
        (1.0 - fracTop) * ((1.0 - fracLeft) * ul + fracLeft * ur));
}

double QImage::interpolateLogPolar(double x, double y)
{
    x = fmod(x, width);
    if (x < 0) x += width;
    int left = ((int)(floor(x))) % width;
    int right = ((int)(ceil(x))) % width;
    int top = (int)(floor(y));
    int bottom = (int)(ceil(y));
    double fracLeft = x - left;
    double fracTop = y - top;
    double ll, lr, ul, ur;
    if (top < 0 || bottom >= height) return 0;
    ll = data(left, bottom);
    lr = data(right, bottom);
    ul = data(left, top);
    ur = data(right, top);
    return (fracTop * ((1.0 - fracLeft) * ll + fracLeft * lr) +
        (1.0 - fracTop) * ((1.0 - fracLeft) * ul + fracLeft * ur));
}

double QImage::interpolateExtend(double x, double y)
{
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x > width - 1) x = width - 1;
    if (y > height - 1) y = height - 1;
    int left = (int)(floor(x));
    int right = (int)(ceil(x));
    int top = (int)(floor(y));
    int bottom = (int)(ceil(y));
    double fracLeft = x - left;
    double fracTop = y - top;
    double ll, lr, ul, ur;
    ll = data(left, bottom);
    lr = data(right, bottom);
    ul = data(left, top);
    ur = data(right, top);
    return (fracTop * ((1.0 - fracLeft) * ll + fracLeft * lr) +
        (1.0 - fracTop) * ((1.0 - fracLeft) * ul + fracLeft * ur));
}

void QImage::ImageToBytes(unsigned char* data, int zoom,
    double min, double max)
{
    if (buffer == NULL) return;
    if (zoom == 0) memcpy(data, buffer, size);
    if (zoom > 0)
    {
        int block = zoom;
        for (int y = 0; y < height; y++)
            for (int yb = 0; yb < block; yb++)
            {
                unsigned char* bufferPtr = buffer + y * width;
                for (int x = 0; x < width; x++)
                {
                    for (int xb = 0; xb < block; xb++)
                        *(data++) = *bufferPtr;
                    bufferPtr++;
                }
            }
    }
    if (zoom < 0)
    {
        int block = -zoom;
        for (int y = 0; y < height; y += block)
            for (int x = 0; x < width; x += block)
                *(data++) = this->data(x, y);
    }
}

// USED
// Initialize the image object, using the given data as the image
void QImage::NewBuffer(Byte* buf, int w, int h)
{
    if (buffer != buf)
    {
        if (buffer != NULL) free(buffer);
        buffer = buf;
        width = w;
        height = h;
        size = width * height;
    }
    Change();
}

// USED
// Allocate a new buffer for the image, of the given size.  If the
// buffer size is the same as the current buffer, just use that instead
// of allocating new memory.  This call should be followed by a call to
// QImage::NewBuffer() to tell the image to use the new buffer.
Byte* QImage::AllocBuffer(int w, int h)
{
    if (width == w && height == h && buffer != NULL)
        return buffer; else
        return (Byte*)malloc(w * h);
}

void QImage::FreeBuffer()
{
    if (buffer != NULL) free(buffer);
    buffer = NULL;
    size = 0;
    width = 0;
    height = 0;
}

FloatImage::FloatImage(char* name) : Image(name)
{
    buffer = NULL;
}

FloatImage::~FloatImage()
{
    free(buffer);
}

int FloatImage::GetType()
{
    return FloatImageType;
}

double FloatImage::interpolate(double x, double y)
{
    int left = (int)(floor(x));
    int right = (int)(ceil(x));
    int top = (int)(floor(y));
    int bottom = (int)(ceil(y));
    double fracLeft = x - left;
    double fracTop = y - top;
    double ll, lr, ul, ur;
    if (left < 0 || right >= width || top < 0 || bottom >= height) return 0;
    ll = data(left, bottom);
    lr = data(right, bottom);
    ul = data(left, top);
    ur = data(right, top);
    return (fracTop * ((1.0 - fracLeft) * ll + fracLeft * lr) +
        (1.0 - fracTop) * ((1.0 - fracLeft) * ul + fracLeft * ur));
}

void FloatImage::ImageToBytes(unsigned char* data, int zoom,
    double min, double max)
{
    if (buffer == NULL) return;
    if (min == 0 && max == 0) {
        min = buffer[0], max = buffer[0];
        for (int i = 1; i < size; i++)
        {
            double val = buffer[i];
            if (val < min) min = val;
            if (max < val) max = val;
        }
        if (max == min) max = min + 1;
    }
    if (zoom == 0)
    {
        for (int i = 0; i < size; i++)
            data[i] = (int)floor(0.5 + 255.0 * (buffer[i] - min) / (max - min));
    }
    if (zoom > 0)
    {
        int block = zoom;
        for (int y = 0; y < height; y++)
            for (int yb = 0; yb < block; yb++)
            {
                float* bufferPtr = buffer + y * width;
                for (int x = 0; x < width; x++)
                {
                    unsigned char val = (int)floor(0.5 + 255.0 * (*(bufferPtr++) - min) /
                        (max - min));
                    for (int xb = 0; xb < block; xb++) *(data++) = val;
                }
            }
    }
    if (zoom < 0)
    {
        int block = -zoom;
        for (int y = 0; y < height; y += block)
            for (int x = 0; x < width; x += block)
                *(data++) = (int)floor(0.5 + 255.0 * (this->data(x, y) - min) / (max - min));
    }
}

void FloatImage::NewBuffer(float* buf, int w, int h)
{
    if (buffer != NULL) free(buffer);
    buffer = buf;
    width = w;
    height = h;
    size = width * height;
    Change();
}

void FloatImage::FreeBuffer()
{
    if (buffer != NULL) free(buffer);
    buffer = NULL;
    size = 0;
    width = 0;
    height = 0;
}

// USED
// Look through the list of images trying to find one with the
// given name, and return a pointer to that image.  Return NULL
// if no such image is found.
Image* FindImageWithName(char* name)
{
    Tcl_HashEntry* entry = Tcl_FindHashEntry(&ImageTable, name);
    if (entry == NULL) return NULL;
    return (Image*)(Tcl_GetHashValue(entry));
}

// USED
// Look through the list of images, trying to find one with the given
// name and that also has byte pixels.  Return that image, or NULL if
// no such image is found.
QImage* FindQImageWithName(char* name)
{
    Image* image = FindImageWithName(name);
    if (image == NULL || image->GetType() != QImageType) return NULL;
    return (QImage*)image;
}

FloatImage* FindFloatImageWithName(char* name)
{
    Image* image = FindImageWithName(name);
    if (image == NULL || image->GetType() != FloatImageType) return NULL;
    return (FloatImage*)image;
}

void RotateQImage2(QImage* dest, QImage* src, double angle)
{
    double c = cos(angle);
    double s = sin(angle);
    Byte* buf = (Byte*)malloc(src->width * src->height);
    int ptr = 0;
    double hx = src->width / 2;
    double hy = src->height / 2;
    for (int y = 0; y < src->height; y++)
        for (int x = 0; x < src->width; x++)
            buf[ptr++] = (int)floor(0.5 + src->interpolate((x - hx) * c + (y - hy) * s + hx,
                -(x - hx) * s + (y - hy) * c + hy));
    dest->NewBuffer(buf, src->width, src->height);
}

void ZoomQImage(QImage* image, double scale, QImage* src)
{
    int newWidth = (int)floor(src->width / scale);
    int newHeight = (int)floor(src->height / scale);
    Byte* newdata = (Byte*)malloc(sizeof(Byte) * newWidth * newHeight);
    for (int x = 0; x < newWidth; x++)
        for (int y = 0; y < newHeight; y++)
            newdata[x + y * newWidth] = (int)floor(0.5 +
                src->interpolate(x * scale, y * scale));
    image->NewBuffer(newdata, newWidth, newHeight);
}

// USED
// Fill a rectangular region of a byte image with the given value
void FillRectangle(QImage* image, int x1, int y1, int x2, int y2, int color)
{
    for (int y = y1; y < y2; y++)
        if (y >= 0 && y < image->height)
            for (int x = x1; x < x2; x++)
                if (x >= 0 && x < image->width)
                    image->data(x, y) = color;
    image->Change();
}

void SubSampleWithAveraging(QImage* dest, QImage* src, int scale)
{
    Byte* newdata = (Byte*)malloc((src->width / scale) * (src->height / scale));
    int ptr = 0;
    for (int y = 0; y < src->height / scale; y++)
        for (int x = 0; x < src->width / scale; x++)
        {
            double tot = 0;
            for (int i = x * scale; i < (x + 1) * scale; i++)
                for (int j = y * scale; j < (y + 1) * scale; j++)
                    tot += src->data(i, j);
            newdata[ptr++] = (int)floor(0.5 + tot / (scale * scale));
        }
    dest->NewBuffer(newdata, src->width / scale, src->height / scale);
}

#ifdef notdef
void ResampleQImage(QImage* image, QImage* src,
    int newWidth, int newHeight, double scale)
{
    Byte* buf = image->AllocBuffer(newWidth, newHeight);
    for (int i = 0; i < newWidth * newHeight; i++)
        buf[i] = 255;
    image->NewBuffer(buf, newWidth, newHeight);
    for (int x = 0; x < src->width; x++)
        for (int y = 0; y < src->height; y++)
            if (!src->data(x, y))
            {
                int nx = (int)(0.5 + x * scale);
                int ny = (int)(0.5 + y * scale);
                if (nx < image->width && ny < image->height)
                    image->data(nx, ny) = 0;
            }
    image->Change();
}
#endif

// USED
// Resample an image, according to the following rule: if any of the
// pixels in the source image which coorespond to a pixel in the
// destination image are zero, then set the destination pixel to zero.
// Otherwise, set the pixel to 255.  scale<1 makes the image smaller.
// The idea is that 0 means a pixel location should be scanned, while
// 255 means that a face has already been found and need not be checked
// again.  Destination can be the same as the source.
// The scaling factor is determined by SCALE, and the size of the
// destination image is controlled by newWith and newHeight;
void ResampleQImage(QImage* image, QImage* src,
    int newWidth, int newHeight, double scale)
{
    Byte* buf = image->AllocBuffer(newWidth, newHeight);  // Allocate destination
    Byte* ptr = buf;
    for (int i = 0; i < newWidth * newHeight; i++) buf[i] = 255;

    for (int y = 0; y < newHeight; y++)      // Scan over lower resolution image
        for (int x = 0; x < newWidth; x++) {
            int x1 = (int)(0.5 + x / scale);
            int y1 = (int)(0.5 + y / scale);
            if (x1 >= src->width) x1 = src->width - 1;
            if (y1 >= src->height) y1 = src->height - 1;
            int x2 = (int)(0.5 + (x + 1) / scale);
            int y2 = (int)(0.5 + (y + 1) / scale);
            if (x2 >= src->width) x2 = src->width - 1;
            if (y2 >= src->height) y2 = src->height - 1;
            int i, j;
            for (j = y1; j <= y2; j++)      // Scan over corresponding pixels in hi res
                for (i = x1; i <= x2; i++)
                    if (!src->data(i, j)) {  // If any pixel is zero, make new pixel zero
                        *ptr = 0;
                        goto next;
                    }
        next:
            ptr++;
        }
    image->NewBuffer(buf, newWidth, newHeight);
}

// USED
// Scale the given byte image down by a factor of 1.2, putting the result
// in the given destination (which can be the same as the source).  The
// scaling uses bilinear interpolation, implemented by two steps of linear
// interpolation: first scaling in the X direction, then in the Y direction.
void ReduceSize(QImage* dest, QImage* src)
{
    int width = src->width;
    int height = src->height;
    int newWidth = (int)floor(width / 1.2);
    int newHeight = (int)floor(height / 1.2);
    //  double scaleX=(double)(src->width-1)/(double)(newWidth-1);
    //  double scaleY=(double)(src->height-1)/(double)(newHeight-1);
    double scaleX = 1.2, scaleY = 1.2;

    // First scale in X
    Byte* tmp = (Byte*)malloc(height * newWidth);
    int x, y;
    Byte* inptr = src->buffer;
    for (y = 0; y < height; y++) {
        for (x = 0; x < newWidth; x++) {
            int pos = (int)(x * scaleX);
            double frac = (x * scaleX) - pos;
            double val = (1.0 - frac) * inptr[pos] + frac * inptr[pos + 1];
            if (val < 0) val = 0;
            if (val > 255) val = 255;
            tmp[y + x * height] = (int)(val + 0.5);
        }
        inptr += width;
    }

    // Then scale in Y
    Byte* tmp2 = dest->AllocBuffer(newWidth, newHeight);
    inptr = tmp;
    for (y = 0; y < newWidth; y++) {
        for (x = 0; x < newHeight; x++) {
            int pos = (int)(x * scaleY);
            double frac = (x * scaleY) - pos;
            double val = (1.0 - frac) * inptr[pos] + frac * inptr[pos + 1];
            if (val < 0) val = 0;
            if (val > 255) val = 255;
            tmp2[y + x * newWidth] = (int)(val + 0.5);
        }
        inptr += height;
    }
    dest->NewBuffer(tmp2, newWidth, newHeight);
    free(tmp);
}

#ifdef notdef
// Version that actually does smoothing
int ReduceSize(QImage* dest, QImage* src)
{
    int width = src->width;
    int height = src->height;
    int newWidth = (int)floor(width / 1.2);
    int newHeight = (int)floor(height / 1.2);
    Byte* tmp = (Byte*)malloc(height * newWidth);
    double* scan = (double*)malloc(width * sizeof(double));
#define size 3
#define len 7
    //  int size=3;
    //  int len=7;
    double kernel[7] = { 0.09749,-0.12662,0.14628,0.76569,0.14628,-0.12662,0.09749 };
    double div[4];
    int x, y;
    double tot = 0;
    for (x = size * 2; x > size; x--) tot += kernel[x];
    for (x = 0; x < size; x++)
    {
        tot += kernel[size - x];
        div[x + 1] = tot;
    }
    Byte* inptr = src->buffer;
    for (y = 0; y < height; y++)
    {
        double* optr = scan;
        for (x = size; x >= 1; x--)
        {
            double tot = 0;
            double* kptr = kernel + x;
            Byte* iptr = inptr;
            for (int i = 0; i < len - x; i++)
                tot += (*(kptr++)) * (*(iptr++));
            *(optr++) = tot / div[x];
        }
        for (x = size; x < width - size; x++)
        {
            double tot = 0;
            double* kptr = kernel;
            Byte* iptr = inptr++;
            for (int i = 0; i < len; i++)
                tot += (*(kptr++)) * (*(iptr++));
            *(optr++) = tot;
        }
        for (x = 1; x <= size; x++)
        {
            double tot = 0;
            double* kptr = kernel;
            Byte* iptr = inptr++;
            for (int i = 0; i < len - x; i++)
                tot += (*(kptr++)) * (*(iptr++));
            *(optr++) = tot / div[x];
        }
        inptr += size;
        for (x = 0; x < newWidth; x++)
        {
            int pos = (int)(x * 1.2);
            double frac = (x * 1.2) - pos;
            double val = (1.0 - frac) * scan[pos] + frac * scan[pos + 1];
            if (val < 0) val = 0;
            if (val > 255) val = 255;
            tmp[y + x * height] = (int)val;
        }
    }
    free(scan);
    scan = (double*)malloc(height * sizeof(double));
    Byte* tmp2 = dest->AllocBuffer(newWidth, newHeight);
    inptr = tmp;
    for (y = 0; y < newWidth; y++)
    {
        double* optr = scan;
        for (x = size; x >= 1; x--)
        {
            double tot = 0;
            double* kptr = kernel + x;
            Byte* iptr = inptr;
            for (int i = 0; i < len - x; i++)
                tot += (*(kptr++)) * (*(iptr++));
            *(optr++) = tot / div[x];
        }
        for (x = size; x < height - size; x++)
        {
            double tot = 0;
            double* kptr = kernel;
            Byte* iptr = inptr++;
            for (int i = 0; i < len; i++)
                tot += (*(kptr++)) * (*(iptr++));
            *(optr++) = tot;
        }
        for (x = 1; x <= size; x++)
        {
            double tot = 0;
            double* kptr = kernel;
            Byte* iptr = inptr++;
            for (int i = 0; i < len - x; i++)
                tot += (*(kptr++)) * (*(iptr++));
            *(optr++) = tot / div[x];
        }
        inptr += size;
        for (x = 0; x < newHeight; x++)
        {
            int pos = (int)(x * 1.2);
            double frac = (x * 1.2) - pos;
            double val = (1.0 - frac) * scan[pos] + frac * scan[pos + 1];
            if (val < 0) val = 0;
            if (val > 255) val = 255;
            tmp2[y + x * newWidth] = (int)(val + 0.5);
        }
    }
    dest->NewBuffer(tmp2, newWidth, newHeight);
    free(tmp);
    free(scan);
#undef len
#undef size
}
#endif

#ifdef notdef
void SmoothQImage(QImage* image)
{
    Byte* newdata = (Byte*)malloc(image->size);
    int center = 3;
    int size = 3;
    double kernel[7] = { 0.1061,
                      -0.1378,
                      0.1592,
                      0.8333,
                      0.1592,
                      -0.1378,
                      0.1061 };
    double realtot = 1;
    int x, y;
    for (y = 0; y < image->height; y++)
        for (x = 0; x < image->width; x++)
        {
            double tot = 0;
            double div = 0;
            for (int i = -size; i <= size; i++)
                if (x + i >= 0 && x + i < image->width)
                {
                    tot += kernel[i + center] * image->data(x + i, y);
                    div += kernel[i + center];
                }
            tot = tot * realtot / div;
            if (tot < 0) tot = 0;
            if (tot > 255) tot = 255;
            newdata[y * image->width + x] = (int)tot;
        }

    for (y = 0; y < image->height; y++)
        for (x = 0; x < image->width; x++)
        {
            double tot = 0;
            double div = 0;
            for (int i = -size; i <= size; i++)
                if (y + i >= 0 && y + i < image->height)
                {
                    tot += kernel[i + center] * newdata[(y + i) * image->width + x];
                    div += kernel[i + center];
                }
            tot = tot * realtot / div;
            if (tot < 0) tot = 0;
            if (tot > 255) tot = 255;
            image->data(x, y) = (int)tot;
        }
    free(newdata);
    image->Change();
}
#endif

void SmoothQImage(QImage* image)
{
    Byte* newdata = (Byte*)malloc(image->size);
    int center = 3;
    int size = 3;
    double kernel[7] = { 0.1061,
                      -0.1378,
                      0.1592,
                      0.8333,
                      0.1592,
                      -0.1378,
                      0.1061 };
    double realtot = 1;
    int x, y;
    for (y = 0; y < image->height; y++)
        for (x = 0; x < image->width; x++)
        {
            double tot = 0;
            double div = 0;
            for (int i = -size; i <= size; i++)
                if (x + i >= 0 && x + i < image->width)
                {
                    tot += kernel[i + center] * image->data(x + i, y);
                    div += kernel[i + center];
                }
            tot = tot * realtot / div;
            if (tot < 0) tot = 0;
            if (tot > 255) tot = 255;
            newdata[y * image->width + x] = (int)tot;
        }

    for (y = 0; y < image->height; y++)
        for (x = 0; x < image->width; x++)
        {
            double tot = 0;
            double div = 0;
            for (int i = -size; i <= size; i++)
                if (y + i >= 0 && y + i < image->height)
                {
                    tot += kernel[i + center] * newdata[(y + i) * image->width + x];
                    div += kernel[i + center];
                }
            tot = tot * realtot / div;
            if (tot < 0) tot = 0;
            if (tot > 255) tot = 255;
            image->data(x, y) = (int)tot;
        }
    free(newdata);
    image->Change();
}

void SaveQImagesPpm(QImage* red, QImage* green, QImage* blue,
    FILE* outf)
{
    fprintf(outf, "P6\n%d %d\n255\n", red->width, red->height);
    unsigned char* buf = (unsigned char*)malloc(red->size * 3);
    unsigned char* ptr = buf, * r = red->buffer, * g = green->buffer, * b = blue->buffer;
    for (int i = 0; i < red->size; i++)
    {
        *(ptr++) = *(r++);
        *(ptr++) = *(g++);
        *(ptr++) = *(b++);
    }
    fwrite((char*)buf, red->size * 3, 1, outf);
    free((char*)buf);
}

void SaveQImagePgm(QImage* image, FILE* outf)
{
    fprintf(outf, "P5\n%d %d\n255\n", image->width, image->height);
    fwrite((char*)(image->buffer), image->size, 1, outf);
}

// USED
// Given an image and a FILE pointer, load an image in raw pgm format.
// An image buffer of the appropriate size is allocated.
void LoadQImagePgm(QImage* image, FILE* inf)
{
    int tokens = 0;
    int dataStart = 0;
    // Read the header, ignoring comments
    do {
        char line[80];
        //    dataStart=ftell(inf);
        fgets(line, 80, inf);
        char* comment = strchr(line, '#');
        if (comment != NULL) *comment = 0;
        char* token = NULL, * input = &line[0];
        while (tokens < 4 && (token = strtok(input, " \t\n\r")) != NULL) {
            switch (tokens) {
            case 0:
                if (strcmp(token, "P5") != 0) {
                    fprintf(stderr, "Bad PGM file.\n");
                    exit(1);
                }
                break;
            case 1:
                image->width = atoi(token);
                break;
            case 2:
                image->height = atoi(token);
                break;
            case 3:
                dataStart += (token - &line[0]) + strlen(token) + 1;
                break;
            default:
                break;
            }
            tokens++;
            input = NULL;
        }
    } while (tokens < 4);
    //  fseek(inf,dataStart,SEEK_SET);
      // Read the image data itself
    if (image->buffer != NULL) free(image->buffer);
    image->size = image->width * image->height;
    image->buffer = (Byte*)malloc(image->size);
    fread((char*)image->buffer, 1, image->size, inf);
    image->Change();
}

#include <io.h>

char* myfgets(char* buf, int num, FILE* inf)
{
    int fd = fileno(inf);
    int i = 0;
    do {
        read(fd, buf + i, 1);
        i++;
    } while (i < num - 1 && buf[i - 1] != 10);
    buf[i] = 0;
    fseek(inf, tell(fd), SEEK_SET);
    return buf;
}

int myfread(char* buf, size_t size, size_t num, FILE* inf)
{
    int fd = fileno(inf);
    fprintf(stderr, "myfread at %d/%d\n", (int)tell(fd), (int)ftell(inf));
    int i;
    for (i = 0; i < num; i++)
        if (read(fd, buf + size * i, size) < size) break;
    perror("what happened");
    fseek(inf, tell(fd), SEEK_SET);
    return i;
}

unsigned char* LoadPgmStreamToBuffer(FILE* inf, int* width, int* height)
{
    int tokens = 0;
    int dataStart = 0;
    do {
        char line[80];
        //    dataStart=ftell(inf);
        fgets(line, 80, inf);
        char* comment = strchr(line, '#');
        if (comment != NULL) *comment = 0;
        char* token = NULL, * input = &line[0];
        while (tokens < 4 && (token = strtok(input, " \t\n\r")) != NULL)
        {
            switch (tokens)
            {
            case 0:
                if (strcmp(token, "P5") != 0)
                {
                    fprintf(stderr, "Bad PGM file.\n");
                    exit(1);
                }
                break;
            case 1:
                *width = atoi(token);
                break;
            case 2:
                *height = atoi(token);
                break;
            case 3:
                dataStart += (token - &line[0]) + strlen(token) + 1;
                break;
            default:
                break;
            }
            tokens++;
            input = NULL;
        }
    } while (tokens < 4);
    //  fseek(inf,dataStart,SEEK_SET);
    Byte* buffer = (Byte*)malloc((*width) * (*height));
    fread((char*)buffer, 1, (*width) * (*height), inf);
    return buffer;
}

unsigned char* LoadPpmStreamToBuffer(FILE* inf, int* width, int* height)
{
    int tokens = 0;
    int dataStart = 0;
    do {
        char line[80];
        //    dataStart=ftell(inf);
        fgets(line, 80, inf);
        char* comment = strchr(line, '#');
        if (comment != NULL) *comment = 0;
        char* token = NULL, * input = &line[0];
        while (tokens < 4 && (token = strtok(input, " \t\n\r")) != NULL)
        {
            switch (tokens)
            {
            case 0:
                if (strcmp(token, "P6") != 0)
                {
                    fprintf(stderr, "Bad PPM file.\n");
                    exit(1);
                }
                break;
            case 1:
                *width = atoi(token);
                break;
            case 2:
                *height = atoi(token);
                break;
            case 3:
                dataStart += (token - &line[0]) + strlen(token) + 1;
                break;
            default:
                break;
            }
            tokens++;
            input = NULL;
        }
    } while (tokens < 4);
    //  fseek(inf,dataStart,SEEK_SET);
    Byte* buffer = (Byte*)malloc((*width) * (*height) * 3);
    fread((char*)buffer, 1, (*width) * (*height) * 3, inf);
    return buffer;
}

unsigned char* LoadPgmToBuffer(char* filename, int* width, int* height)
{
    FILE* inf = fopen(filename, "rb");
    assert(inf != NULL);
    unsigned char* buffer = LoadPgmStreamToBuffer(inf, width, height);
    fclose(inf);
    return buffer;
}

unsigned char* LoadPpmToBuffer(char* filename, int* width, int* height)
{
    FILE* inf = fopen(filename, "rb");
    assert(inf != NULL);
    unsigned char* buffer = LoadPpmStreamToBuffer(inf, width, height);
    fclose(inf);
    return buffer;
}

void ByteToFloatImage(QImage* bi, FloatImage* fi)
{
    float* buf = (float*)malloc(bi->size * sizeof(float));
    for (int i = 0; i < bi->size; i++)
        buf[i] = bi->data(i) / 127.0 - 1.0;
    fi->NewBuffer(buf, bi->width, bi->height);
}

void FFTFloatImage(FloatImage* src1, FloatImage* src2, int forward)
{
    int width = src1->width;
    int height = src1->height;
    Complex* row = new Complex[width];
    Complex* col = new Complex[height];
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            row[i].a = src1->data(i + j * width);
            row[i].b = src2->data(i + j * width);
        }
        ComputeFFT(row, width, forward);
        for (int i = 0; i < width; i++)
        {
            src1->data(i + j * width) = row[i].a;
            src2->data(i + j * width) = row[i].b;
        }
    }
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            col[j].a = src1->data(i + j * width);
            col[j].b = src2->data(i + j * width);
        }
        ComputeFFT(col, height, forward);
        for (int j = 0; j < height; j++)
        {
            src1->data(i + j * width) = col[j].a;
            src2->data(i + j * width) = col[j].b;
        }
    }
    delete[] row;
    delete[] col;
    src1->Change();
    src2->Change();
}

void PowerSpectrumFloatImage(FloatImage* dest, FloatImage* src)
{
    float* buf = (float*)malloc(src->size * sizeof(float));
    Complex* data = new Complex[src->size];
    Complex* row = new Complex[src->width];
    Complex* col = new Complex[src->height];
    for (int j = 0; j < src->height; j++)
    {
        for (int i = 0; i < src->width; i++)
            row[i].a = src->data(i + j * src->width);
        ComputeFFT(row, src->width, 1);
        for (int i = 0; i < src->width; i++)
            data[i + j * src->width] = row[i];
    }
    for (int i = 0; i < src->width; i++)
    {
        for (int j = 0; j < src->height; j++)
            col[j] = data[i + j * src->width];
        ComputeFFT(col, src->height, 1);
        for (int j = 0; j < src->height; j++)
            buf[i + j * src->width] = Abs(col[j]);
    }
    delete[] row;
    delete[] col;
    delete[] data;
    dest->NewBuffer(buf, src->width, src->height);
}

void LoadQImageRaw(QImage* image, int width, int height, FILE* inf)
{
    Byte* newdata = image->AllocBuffer(width, height);
    fread((char*)newdata, width, height, inf);
    image->NewBuffer(newdata, width, height);
}

void LoadQImageHsi(QImage* image, FILE* inf)
{
    int foo;
    fread((char*)&foo, sizeof(int), 1, inf);
    fread((char*)&foo, sizeof(int), 1, inf);
    fread((char*)&image->height, sizeof(int), 1, inf);
    fread((char*)&image->width, sizeof(int), 1, inf);
    for (int i = 0; i < 39; i++)
        fread((char*)&foo, sizeof(int), 1, inf);
    if (image->buffer != NULL) free(image->buffer);
    image->size = image->width * image->height;
    image->buffer = (Byte*)malloc(image->size);
    int num;
    if ((num = fread((char*)image->buffer, 1, image->size, inf)) != image->size)
    {
        fprintf(stderr, "Claims to have read wrong amount: %d/%d\n",
            num, image->size);
    }
    image->Change();
}

void FloatImage::ZeroBuffer(int w, int h)
{
    if (buffer != NULL) free(buffer);
    width = w;
    height = h;
    size = w * h;
    buffer = (float*)malloc(sizeof(float) * size);
    for (int i = 0; i < size; i++) buffer[i] = 0.0;
    Change();
}

// USED
// Create data for image, and fill with zeros (or whatever value is passed
// in).  Any previous data for image is discarded.
void QImage::ZeroBuffer(int w, int h, unsigned char fill)
{
    if (buffer != NULL) free(buffer);
    width = w;
    height = h;
    size = w * h;
    buffer = (Byte*)malloc(sizeof(Byte) * size);
    for (int i = 0; i < size; i++) buffer[i] = fill;
    Change();
}

void CopyQImage(QImage* dest, QImage* src)
{
    Byte* newdata = dest->AllocBuffer(src->width, src->height);
    Byte* to = newdata;
    Byte* from = src->buffer;
    for (int i = 0; i < src->size; i++)
        *(to++) = *(from++);
    dest->NewBuffer(newdata, src->width, src->height);
}

void DifferenceImage(QImage* dest, QImage* src1, QImage* src2)
{
    Byte* newdata = dest->AllocBuffer(src1->width, src1->height);
    Byte* ptr1 = src1->buffer;
    Byte* ptr2 = src2->buffer;
    Byte* ptr = newdata;
    for (int i = 0; i < src1->size; i++)
        *(ptr++) = abs((int)(*(ptr1++)) - (int)(*(ptr2++)));
    dest->NewBuffer(newdata, src1->width, src1->height);
}

void LaplacianImage(QImage* dest, QImage* src)
{
    Byte* newdata = (Byte*)malloc(src->width * src->height);
    int ptr = 0;
    for (int y = 0; y < src->height; y++)
        for (int x = 0; x < src->width; x++)
        {
            int cur = src->data(x, y);
            int val = -4 * cur;
            if (x > 0) val += src->data(x - 1, y); else val += cur;
            if (y > 0) val += src->data(x, y - 1); else val += cur;
            if (x < src->width - 1) val += src->data(x + 1, y); else val += cur;
            if (y < src->height - 1) val += src->data(x, y + 1); else val += cur;
            newdata[ptr++] = (abs(val) >> 2);
        }
    dest->NewBuffer(newdata, src->width, src->height);
}

void ExtractRectangle(QImage* dest, QImage* src, int x, int y,
    int width, int height)
{
    Byte* buf = dest->AllocBuffer(width, height);
    dest->NewBuffer(buf, width, height);
    for (int j = y; j < y + height; j++)
    {
        int jj = j; if (jj < 0) jj = 0; if (jj >= src->height) jj = src->height - 1;
        for (int i = x; i < x + width; i++)
        {
            int ii = i; if (ii < 0) ii = 0; if (ii >= src->width) ii = src->width - 1;
            dest->data(i - x, j - y) = src->data(ii, jj);
        }
    }
    dest->Change();
}

void ExtractAndRotate(QImage* dest, QImage* src, int x, int y,
    int width, int height, double angle)
{
    Byte* buf = dest->AllocBuffer(width, height);
    dest->NewBuffer(buf, width, height);
    for (int j = 0; j < height; j++)
        for (int i = 0; i < width; i++) {
            double rx = x + (i - width / 2 + 0.5) * cos(angle) - (j - height / 2) * sin(angle);
            double ry = y + (i - width / 2 + 0.5) * sin(angle) + (j - height / 2) * cos(angle);
            int value = (int)floor(src->interpolateExtend(rx, ry) + 0.5);
            if (value < 0) value = 0;
            if (value > 255) value = 255;
            dest->data(i, j) = value;
        }
    dest->Change();
}

void ExtractAndZoom(QImage* dest, QImage* src, double scale,
    int minX, int minY, int maxX, int maxY)
{
    int width = (int)floor(0.5 + (maxX - minX) / scale);
    int height = (int)floor(0.5 + (maxY - minY) / scale);
    Byte* buf = (Byte*)malloc(width * height);
    int ptr = 0;
    if (0) {
        for (int y = 0; y < height; y++) for (int x = 0; x < width; x++)
            buf[ptr++] = (int)(src->interpolateExtend(x * scale + minX, y + scale * minY));
    }
    else {
        for (int y = 0; y < height; y++)
        {
            int y1 = (int)(0.5 + y * scale + minY);
            int y2 = (int)(0.5 + (y + 1) * scale + minY);
            if (y2 >= src->height) y2 = src->height - 1;
            for (int x = 0; x < width; x++)
            {
                int x1 = (int)(0.5 + x * scale + minX);
                int x2 = (int)(0.5 + (x + 1) * scale + minX);
                if (x2 >= src->width) x2 = src->width - 1;
                int num = (x2 - x1) * (y2 - y1);
                int tot = num >> 1;
                for (int j = y1; j < y2; j++) for (int i = x1; i < x2; i++)
                    tot += src->data(i, j);
                tot /= num;
                if (tot > 255) tot = 255;
                buf[ptr++] = tot;
            }
        }
    }
    dest->NewBuffer(buf, width, height);
}

void DifferenceFloatImage(QImage* dest, QImage* src1, FloatImage* src2)
{
    Byte* newdata = dest->AllocBuffer(src1->width, src1->height);
    Byte* ptr1 = src1->buffer;
    float* ptr2 = src2->buffer;
    Byte* ptr = newdata;
    for (int i = 0; i < src1->size; i++)
        *(ptr++) = abs((int)(*(ptr1++)) - (int)(*(ptr2++)));
    dest->NewBuffer(newdata, src1->width, src1->height);
}

void FindMotion(QImage* dest, QImage* src1, FloatImage* src2,
    int threshold, unsigned char maskValue)
{
    Byte* ptr1 = src1->buffer;
    float* ptr2 = src2->buffer;
    Byte* ptr = dest->buffer;
    for (int i = 0; i < src1->size; i++)
    {
        if (abs((int)(*(ptr1++)) - (int)(*(ptr2++))) >= threshold)
            *ptr &= maskValue;
        ptr++;
    }
    dest->Change();
}

void FindMotion(QImage* dest, QImage* src1, QImage* src2,
    int threshold, unsigned char maskValue)
{
    Byte* ptr1 = src1->buffer;
    Byte* ptr2 = src2->buffer;
    Byte* ptr = dest->buffer;
    for (int i = 0; i < src1->size; i++)
    {
        if (abs((int)(*(ptr1++)) - (int)(*(ptr2++))) >= threshold)
            *ptr &= maskValue;
        ptr++;
    }
    dest->Change();
}

void CentroidImage(QImage* image, double* ax, double* ay,
    double* vx, double* vy)
{
    double avgX = 0, avgY = 0;
    double varX = 0, varY = 0;
    double tot = 0;
    Byte* valPtr = image->buffer;
    for (int y = 0; y < image->height; y++)
        for (int x = 0; x < image->width; x++)
        {
            double val = *(valPtr++);
            tot += val;
            avgX += x * val;
            varX += x * x * val;
            avgY += y * val;
            varY += y * y * val;
        }
    if (tot == 0) tot = 1;
    avgX /= tot;
    avgY /= tot;
    varX = sqrt(varX / tot - avgX * avgX);
    varY = sqrt(varY / tot - avgY * avgY);
    *ax = avgX;
    *ay = avgY;
    *vx = varX;
    *vy = varY;
}

void AddQImages(QImage* dest, QImage* src1, QImage* src2)
{
    Byte* newdata = dest->AllocBuffer(src1->width, src1->height);
    for (int i = 0; i < src1->size; i++)
        newdata[i] = (src1->data(i) >> 1) + (src2->data(i) >> 1);
    dest->NewBuffer(newdata, src1->width, src1->height);
}

void AddFloatImages(FloatImage* dest, FloatImage* src1, FloatImage* src2)
{
    float* newdata = (float*)malloc(sizeof(float) * src1->width * src1->height);
    for (int i = 0; i < src1->size; i++)
        newdata[i] = src1->data(i) + src2->data(i);
    dest->NewBuffer(newdata, src1->width, src1->height);
}

void Threshold(QImage* image, int threshold, int inv)
{
    if (inv)
    {
        for (int i = 0; i < image->size; i++)
            if (image->data(i) >= threshold) image->data(i) = 0; else
                image->data(i) = 255;
    }
    else
    {
        for (int i = 0; i < image->size; i++)
            if (image->data(i) >= threshold) image->data(i) = 255; else
                image->data(i) = 0;
    }
    image->Change();
}

void SmartThreshold(QImage* image)
{
    double hist[256];
    double total = image->size;
    for (int i = 0; i < 256; i++) hist[i] = 0;
    for (int i = 0; i < image->size; i++)
        hist[image->data(i)]++;
    int bestk = 0;
    int min = 0, max = 0;
    for (int i = 0; i < 256; i++) if (hist[i] > 0) max = i;
    for (int i = 256; i >= 0; i--) if (hist[i] > 0) min = i;
    if (min == max) bestk = max - 1; else
    {
        double average = 0.0;
        for (int i = 0; i < 256; i++)
        {
            average += i * hist[i];
            hist[i] /= total;
        }
        average /= total;
        double measure = 0.0;
        bestk = -1;
        double classProb = 0.0;
        double classAverage = 0.0;
        for (int k = min; k < max; k++)
        {
            classProb += hist[k];
            classAverage += k * hist[k];
            double m = (average * classProb - classAverage) *
                (average * classProb - classAverage) / (classProb * (1.0 - classProb));
            if (bestk == -1 || m > measure)
            {
                bestk = k;
                measure = m;
            }
        }
    }
    //  fprintf(stderr,"threshold=%d\n",bestk);
    for (int i = 0; i < image->size; i++)
        if (image->data(i) > bestk) image->data(i) = 255; else
            image->data(i) = 0;
    image->Change();
}

void MirrorQImage(QImage* image)
{
    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width / 2; x++)
        {
            Byte tmp = image->data(x, y);
            image->data(x, y) = image->data(image->width - x - 1, y);
            image->data(image->width - x - 1, y) = tmp;
        }
    }
    image->Change();
}

void MirrorFloatImage(FloatImage* image)
{
    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width / 2; x++)
        {
            float tmp = image->data(x, y);
            image->data(x, y) = image->data(image->width - x - 1, y);
            image->data(image->width - x - 1, y) = tmp;
        }
    }
    image->Change();
}

double CountAboveZero(QImage* image)
{
    int tot = 0;
    for (int i = 0; i < image->size; i++)
        if (image->data(i) > 0) tot++;
    return (double)tot / (double)image->size;
}

void EvolveBackgroundImage(FloatImage* background, QImage* dat,
    double rate)
{
    for (int i = 0; i < background->size; i++)
        background->data(i) = background->data(i) * (1 - rate) + dat->data(i) * rate;
    background->Change();
}

void Image_Init_NoTcl()
{
    static int initialized = 0;
    if (!initialized) {
        Tcl_InitHashTable(&ImageTable, TCL_STRING_KEYS);
        initialized = 1;
    }
}

#include "img.hpp"
