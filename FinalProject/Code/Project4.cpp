#include "mainwindow.h"
#include "math.h"
#include "ui_mainwindow.h"
#include "img.h"
#include "mat.h"
#include "list.h"
#include "search.h"
#include "follow.h"
#include "structs.h"
#include "C:\Git Workspace\ComputerVision-master/Algorithms/Detection/Rowley/detect/im_face.cc"
#include "context.h"
#include <QtGui>
#include <QByteArray>
#include <QSize>
#include <QList>
#include <QApplication>
#include <QDir>
#include <math.h>
#include <D:\aminegar\Downloads\opencv\include\opencv\cv.h>
#include <D:\aminegar\Downloads\opencv\include\opencv2\highgui\highgui.hpp>
#include <sstream>
#include <string>
#include <QApplication>
#include <stdio.h>
#include <stdlib.h>
#include "C:\Git Workspace\EE_596_Final\EE_596_Final\FinalProject\Code\stasm4/stasm_lib.h"

void Informedia_OutputDetection(ClientData data, QImage* image,
    int x, int y, int width, int height,
    int level,
    double scale, double output, int orientation);

void Informedia_SaveDetections(ClientData data, QImage* image,
    int x, int y, int width, int height,
    int level,
    double scale, double output, int orientation);
//NOT USED FOR NOW
void FindFaces(QImage inImage, double minScale, double maxScale, QImage *displayImage)
{
    int w = inImage.width();
    int h = inImage.height();
    double scaleMulti = 1.26;
    double scale;
    int r, c;

    // Serach in scale space
    for(scale=minScale;scale<maxScale;scale*=scaleMulti)
    {
        // Find size of bounding box
        int faceSize = (int) scale;
        int stepSize = max(2, faceSize/8);

        // For every possible position
        for(r=0;r<h-faceSize;r+=stepSize)
            for(c=0;c<w-faceSize;c+=stepSize)
            {
                // call the Stasm Function
                //*********************TODO**********************************
            }
    }
}
/***********************************************************************
  This is the only file you need to change for your assignment. The
  other files control the UI (in case you want to make changes.)
************************************************************************/
// Convert an image to grayscale
void MainWindow::BlackWhiteImage(QImage *image)
{
    for(int r=0;r<image->height();r++)
        for(int c=0;c<image->width();c++)
        {
            QRgb pixel = image->pixel(c, r);
            double red = (double) qRed(pixel);
            double green = (double) qGreen(pixel);
            double blue = (double) qBlue(pixel);

            // Compute intensity from colors - these are common weights
            double intensity = 0.3*red + 0.6*green + 0.1*blue;

            image->setPixel(c, r, qRgb( (int) intensity, (int) intensity, (int) intensity));
        }
}

// Downsample the image by 1/2
void MainWindow::HalfImage(QImage &image)
{
    int w = image.width();
    int h = image.height();
    QImage buffer = image.copy();

    // Reduce the image size.
    image = QImage(w/2, h/2, QImage::Format_RGB32);

    // Copy every other pixel
    for(int r=0;r<h/2;r++)
        for(int c=0;c<w/2;c++)
             image.setPixel(c, r, buffer.pixel(c*2, r*2));
}

// Round float values to the nearest integer values and make sure the value lies in the range [0,255]
QRgb restrictColor(double red, double green, double blue)
{
    int r = (int)(floor(red+0.5));
    int g = (int)(floor(green+0.5));
    int b = (int)(floor(blue+0.5));

    return qRgb(max(0, min(255, r)), max(0, min(255, g)), max(0, min(255, b)));
}

// Normalize the values of the kernel to sum-to-one
void NormalizeKernel(double *kernel, int kernelWidth, int kernelHeight)
{
    double denom = 0.000001; int i;
    for(i=0; i<kernelWidth*kernelHeight; i++)
        denom += kernel[i];
    for(i=0; i<kernelWidth*kernelHeight; i++)
        kernel[i] /= denom;
}

// Here is an example of blurring an image using a mean or box filter with the specified radius.
// This could be implemented using separable filters to make it much more efficient, but it's not done here.
// Note: This function is written using QImage form of the input image. But all other functions later use the double form
void MainWindow::MeanBlurImage(QImage *image, int radius)
{
    if(radius == 0)
        return;
    int size = 2*radius + 1; // This is the size of the kernel

    // Note: You can access the width and height using 'imageWidth' and 'imageHeight' respectively in the functions you write
    int w = image->width();
    int h = image->height();

    // Create a buffer image so we're not reading and writing to the same image during filtering.
    // This creates an image of size (w + 2*radius, h + 2*radius) with black borders (zero-padding)
    QImage buffer = image->copy(-radius, -radius, w + 2*radius, h + 2*radius);

    // Compute the kernel to convolve with the image
    double *kernel = new double [size*size];

    for(int i=0;i<size*size;i++)
        kernel[i] = 1.0;

    // Make sure kernel sums to 1
    NormalizeKernel(kernel, size, size);

    // For each pixel in the image...
    for(int r=0;r<h;r++)
    {
        for(int c=0;c<w;c++)
        {
            double rgb[3];
            rgb[0] = rgb[1] = rgb[2] = 0.0;

            // Convolve the kernel at each pixel
            for(int rd=-radius;rd<=radius;rd++)
                for(int cd=-radius;cd<=radius;cd++)
                {
                     // Get the pixel value
                     //For the functions you write, check the ConvertQImage2Double function to see how to get the pixel value
                     QRgb pixel = buffer.pixel(c + cd + radius, r + rd + radius);

                     // Get the value of the kernel
                     double weight = kernel[(rd + radius)*size + cd + radius];

                     rgb[0] += weight*(double) qRed(pixel);
                     rgb[1] += weight*(double) qGreen(pixel);
                     rgb[2] += weight*(double) qBlue(pixel);
                }
            // Store the pixel in the image to be returned
            // You need to store the RGB values in the double form of the image
            image->setPixel(c, r, restrictColor(rgb[0],rgb[1],rgb[2]));
        }
    }
    // Clean up (use this carefully)
    delete[] kernel;
}

// Convert QImage to a matrix of size (imageWidth*imageHeight)*3 having double values
void MainWindow::ConvertQImage2Double(QImage image)
{
    // Global variables to access image width and height
    imageWidth = image.width();
    imageHeight = image.height();
    // Initialize the global matrix holding the image
    // This is how you will be creating a copy of the original image inside a function
    // Note: 'Image' is of type 'double**' and is declared in the header file (hence global variable)
    // So, when you create a copy (say buffer), write "double** buffer = new double ....."
    Image = new double* [imageWidth*imageHeight];
    for (int i = 0; i < imageWidth*imageHeight; i++)
            Image[i] = new double[3];

    // For each pixel
    for (int r = 0; r < imageHeight; r++)
        for (int c = 0; c < imageWidth; c++)
        {
            // Get a pixel from the QImage form of the image
            QRgb pixel = image.pixel(c,r);

            // Assign the red, green and blue channel values to the 0, 1 and 2 channels of the double form of the image respectively
            Image[r*imageWidth+c][0] = (double) qRed(pixel);
            Image[r*imageWidth+c][1] = (double) qGreen(pixel);
            Image[r*imageWidth+c][2] = (double) qBlue(pixel);
        }
}

// Convert the matrix form of the image back to QImage for display
void MainWindow::ConvertDouble2QImage(QImage *image)
{
    for (int r = 0; r < imageHeight; r++)
        for (int c = 0; c < imageWidth; c++)
            image->setPixel(c, r, restrictColor(Image[r*imageWidth+c][0], Image[r*imageWidth+c][1], Image[r*imageWidth+c][2]));
}

/**************************************************
 Neural Network-Based Face Detection - Henry A. Rowley
 https://courses.cs.washington.edu/courses/cse577/05sp/papers/rowley.pdf
**************************************************/

void MainWindow::RowleyFaceDetection(QImage *image, int num)
/*
 * image: input image in matrix form of size (imageWidth*imageHeight)*3 having double values
 * num: In case we need some type of user set parameter
*/
// Global variables to access image width and height
{
 /*   const int minSize(0); // Where is this coming from ? TODO
    const int maxSize(qMin(imageWidth, imageHeight));
    const int minX(0);
    const int maxX(imageWidth);
    const int minY(0);
    const int maxY(imageHeight);

    BlackWhiteImage(image);
    QImage buffer = image->copy();
    int levels = 0;
    int w = imageHeight, h = imageWidth;
    while (w >= 20 && h >= 20) {
        w = (int)(w / 1.2);
        h = (int)(h / 1.2);
        levels++;
    }
    int scaleRange = 1;
    int minLevel = (int)floor(log(minSize / 20.0) / log(1.2));
    if (minLevel < 0) minLevel = 0;
    int maxLevel = (int)ceil(log(maxSize / 20.0) / log(1.2));
    if (maxLevel >= levels) maxLevel = levels - 1;
    if (levels > maxLevel + 1 + scaleRange) levels = maxLevel + 1 + scaleRange;

    QImage* mainmask = new QImage(NULL);
    QImage** imagePyramid = new QImage * [levels];
    //ConvertQImage2Double(*image);

    imagePyramid[0] = image;
    for (int i = 1; i < levels; i++)
    {
        imagePyramid[i] = new QImage(NULL);
        ReduceSize(imagePyramid[i], imagePyramid[i - 1]);
    }
    QImage* levelmask = new QImage(NULL);
    QImage* mask = FindQImageWithName("facemask");
    QImage* eyemask = FindQImageWithName("eyemask");
    List<Detection> alldetections[1];
    for (int i = maxLevel; i >= minLevel; i--)
        {
            ResampleQImage(levelmask, mainmask,
                imagePyramid[i]->imageWidth, imagePyramid[i]->imageHeight,
                pow(1.2, -i));
            List<Detection> detections;
            SearchUmeEvenFasterRegion(imagePyramid[i], levelmask,
                minX, maxX, minY, maxY, i,
                Informedia_SaveDetections,
                (ClientData)(&detections));
            for (Detection* detect = detections.first; detect != NULL;
                detect = detect->next)
            {
                int newX, newY, newS;
                if (FindNewLocation(levels, imagePyramid, mask,
                    detect->x, detect->y, detect->s, 6, 6, 1, 2, 1,
                    &newX, &newY, &newS))
                {
                    Informedia_SaveDetections((ClientData)alldetections,
                        imagePyramid[newS],
                        newX, newY, 20, 20, newS,
                        pow(1.2, newS), 1.0, 0);
                    FillRectangle(mainmask,
                        (int)(pow(1.2, newS) * newX + 0.5),
                        (int)(pow(1.2, newS) * newY + 0.5),
                        (int)(pow(1.2, newS) * (newX + 20) + 0.5),
                        (int)(pow(1.2, newS) * (newY + 20) + 0.5),
                        255);
                }
            }
        }
        List<Detection> results;
        FuzzyVote2(w, h, 1, alldetections,
            Informedia_SaveDetections, (ClientData)(&results),
            2, 1, 1, 1, 0, mask);
        QList<AlgDetOutput> foundObjs;
        for (Detection* d = results.first; d != NULL; d = d->next)
        {
            FaceEyeLocation location;
            int found = SearchEyes(levels, imagePyramid, eyemask,
                d->x, d->y, d->s, 3,
                &(location.rightx),
                &(location.righty),
                &(location.leftx),
                &(location.lefty));
            if (found) {
                location.left = (location.leftx == -1) ? 0 : 1;
                location.right = (location.rightx == -1) ? 0 : 1;
            }
            else {
                location.left = -1;
                location.right = -1;
            }
            double s = pow(1.2, d->s);
            location.x1 = (int)floor(d->x * s + 0.5);
            location.y1 = (int)floor(d->y * s + 0.5);
            location.x2 = (int)floor((d->x + 20) * s + 0.5);
            location.y2 = (int)floor((d->y + 20) * s + 0.5);
            AlgDetOutput objLoc;
            objLoc.Rectangle.setLeft(location.x1);
            objLoc.Rectangle.setTop(location.y1);
            objLoc.Rectangle.setRight(location.x2);
            objLoc.Rectangle.setBottom(location.y2);
            if (location.left == 1)
            {
                objLoc.Points.insert(algorithms::detection::LeftEyePoint, QPoint(location.leftx, location.lefty));
            }
            if (location.right == 1)
            {
                objLoc.Points.insert(algorithms::detection::RightEyePoint, QPoint(location.rightx, location.righty));
            }
            foundObjs << objLoc;
        }

        for (int i = 1; i < levels; i++) delete imagePyramid[i];
        delete[] imagePyramid;
        delete mainmask;
        delete levelmask;
        delete image;
        */
    static const char* path = "D:\aminegar\Pictures\My-Passport-Pictures\cell phone\IMG_0030.JPG"; //UPDATE LATER TO THE qimage saved path!
    cv::Mat_<unsigned char> img(cv::imread(path, cv::IMREAD_GRAYSCALE));

    if (!img.data)
    {
        printf("Cannot load %s\n", path);
        exit(1);
    }

    int foundface;
    float landmarks[2 * stasm_NLANDMARKS]; // x,y coords

    if (!stasm_search_single(&foundface, landmarks,
                             (char*)img.data, img.cols, img.rows, path, "../data"))
    {
        printf("Error in stasm_search_single: %s\n", stasm_lasterr());
        exit(1);
    }

    if (!foundface)
         printf("No face found in %s\n", path);
    else
    {
        // draw the landmarks on the image as white dots
        stasm_force_points_into_image(landmarks, img.cols, img.rows);
        for (int i = 0; i < stasm_NLANDMARKS; i++)
            img(cvRound(landmarks[i*2+1]), cvRound(landmarks[i*2])) = 255;
    }

    cv::imshow("stasm minimal", img);
    cv::waitKey();
}

/**************************************************
Google API!
**************************************************/
void MainWindow::GoogleFaceDetection(double** image, int num)
/*
 * image: input image in matrix form of size (imageWidth*imageHeight)*3 having double values
 * num: In case we need some type of user set parameter
*/
{
    // code
}
