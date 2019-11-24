
#ifdef __GNUC__
#include <string.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#ifndef hpux
#include <math.h>
#endif
#include <time.h>
#ifndef __GNUC__
#include <string.h>
#endif

#ifdef hpux
#include <math.h>
#endif

#include "img.h"
#include "myhebp.h"
#include "eye.h"
//#include "normface.hpp"
#include "system.h"
#include "mat.h"

struct FloatInt {
    float quality;
    int index;
};

int CompareFloatIntIncreasing(const void* p1, const void* p2);

// USED
// Search for the eyes in the face.  Inputs are the image pyramid,
// the face location and scale.  Returns with the two eye positions.
// If the return value is 0, then the face is considered to be too
// small for the eye detector to be effective.  If the function returns
// 1, then the eye detector has been run.  However, in this case the
// x and y positions being set to -1 indicate that no eye was detected.
int SearchEyes(int numScales, QImage* imagePyramid[], QImage* mask,
    int xx, int yy, int s, int basenet,
    int* eyex1, int* eyey1, int* eyex2, int* eyey2)
{
    // Figure out the scale at which to run the eye detector
    // (the "7" was determined empirically)
    int scale = s - 7;
    if (scale < 0 && scale >= -3) scale = 0;
    if (scale >= 0)
    {
        QImage* image = imagePyramid[scale];
        double x1 = 0, y1 = 0, n1 = 0, x2 = 0, y2 = 0, n2 = 0;

        int* tmp = new int[mask->size];      // Window for eye detector

        // Possible upper-left X positions for the left eye
        int startx1 = (int)floor(0.5 + xx * pow(1.2, s - scale)) - mask->width / 2;
        int endx1 = (int)floor(0.5 + (xx + 10) * pow(1.2, s - scale)) - mask->width / 2;
        if (startx1 < 0) startx1 = 0;
        if (endx1 >= image->width - mask->width + 1) endx1 = image->width - mask->width + 1;

        // Postible upper-left X positions for the right eye
        int startx2 = (int)floor(0.5 + (xx + 10) * pow(1.2, s - scale)) - mask->width / 2;
        int endx2 = (int)floor(0.5 + (xx + 20) * pow(1.2, s - scale)) - mask->width / 2;
        if (startx2 < 0) startx2 = 0;
        if (endx2 >= image->width - mask->width + 1) endx2 = image->width - mask->width + 1;

        // Possible upper-left Y positions for the eyes
        int starty = (int)floor(0.5 + yy * pow(1.2, s - scale)) - mask->height / 2;
        int endy = (int)floor(0.5 + (yy + 10) * pow(1.2, s - scale)) - mask->height;
        if (starty < 0) starty = 0;
        if (endy >= image->height - mask->height + 1) endy = image->height - mask->height + 1;

        for (int y = starty; y < endy; y++) {
            // Look for right eye on this scan line
            for (int x = startx2; x < endx2; x++) {
                int ptr = 0;
                int hist[256];
                for (int i = 0; i < 256; i++) hist[i] = 0;

                // Copy the window into tmp (using mirror image), and compute
                // the histogram over the entire window
                for (int j = 0; j < mask->height; j++) for (int i = mask->width - 1; i >= 0; i--) {
                    int data = image->data(i + x, j + y);
                    tmp[ptr++] = data;
                    hist[data]++;
                }

                // Compute cummulative histogram
                int map[256];
                int num = 0;
                for (int i = 0; i < 256; i++) {
                    map[i] = num;
                    num += hist[i];
                }
                int tot = num;
                for (int i = 255; i >= 0; i--) {
                    map[i] += num;
                    num -= hist[i];
                }

                // Apply the histogram equalization, and write window to network inputs
                double scaleFactor = 1.0 / tot;
                FastForwardUnit* unit = &(real_fast_list[basenet]->unitList[1]);
                for (int i = 0; i < mask->size; i++)
                    (unit++)->activation = map[tmp[i]] * scaleFactor - 1.0;

                // If the network responds positively, add the detection to centroid
                double output = FastForwardPass(real_fast_list[basenet]);
                if (output > 0) {
                    n2 += output;
                    x2 += output * (x + mask->width / 2) * pow(1.2, scale);
                    y2 += output * (y + mask->height / 2) * pow(1.2, scale);
                    /*
                  char name[1024];
                  sprintf(name,".t.c create line %g %g %g %g -fill yellow -tag face",
                  (x+mask->width/2)*pow(1.2,scale),
                  (y+mask->height/2)*pow(1.2,scale),
                  (x+mask->width/2)*pow(1.2,scale)+1,
                  (y+mask->height/2)*pow(1.2,scale));
                  Tcl_Eval(interp,name);
                  */
                }
            }

            // Look for left eye in this scan line
            for (int x = startx1; x < endx1; x++) {
                int ptr = 0;
                int hist[256];
                for (int i = 0; i < 256; i++) hist[i] = 0;

                // Copy the window into tmp (using mirror image), and compute
                // the histogram over the entire window
                for (int j = 0; j < mask->height; j++) for (int i = 0; i < mask->width; i++) {
                    int data = image->data(i + x, j + y);
                    tmp[ptr++] = data;
                    hist[data]++;
                }

                // Compute cummulative histogram
                int map[256];
                int num = 0;
                for (int i = 0; i < 256; i++) {
                    map[i] = num;
                    num += hist[i];
                }
                int tot = num;
                for (int i = 255; i >= 0; i--) {
                    map[i] += num;
                    num -= hist[i];
                }

                // Apply the histogram equalization, and write window to network inputs
                double scaleFactor = 1.0 / tot;
                FastForwardUnit* unit = &(real_fast_list[basenet]->unitList[1]);
                for (int i = 0; i < mask->size; i++)
                    (unit++)->activation = map[tmp[i]] * scaleFactor - 1.0;

                // If the network responds positively, add the detection to centroid
                double output = FastForwardPass(real_fast_list[basenet]);
                if (output > 0) {
                    n1 += output;
                    x1 += output * (x + mask->width / 2) * pow(1.2, scale);
                    y1 += output * (y + mask->height / 2) * pow(1.2, scale);
                    /*
                  char name[1024];
                  sprintf(name,".t.c create line %g %g %g %g -fill yellow -tag face",
                  (x+mask->width/2)*pow(1.2,scale),
                  (y+mask->height/2)*pow(1.2,scale),
                  (x+mask->width/2)*pow(1.2,scale)+1,
                  (y+mask->height/2)*pow(1.2,scale));
                  Tcl_Eval(interp,name);
                  */
                }
            }
        }
        // If the left eye was detected at least once, return centroid
        if (n1 > 0) {
            *eyex1 = (int)floor(0.5 + x1 / n1); *eyey1 = (int)floor(0.5 + y1 / n1);
        }
        else {
            *eyex1 = -1; *eyey1 = -1;
        }

        // If the right eye was detected at least once, return centroid
        if (n2 > 0) {
            *eyex2 = (int)floor(0.5 + x2 / n2); *eyey2 = (int)floor(0.5 + y2 / n2);
        }
        else {
            *eyex2 = -1; *eyey2 = -1;
        }
        delete[] tmp;
        return 1;
    }

    // Return fact that face was too small
    *eyex1 = -1; *eyey1 = -1;
    *eyex2 = -1; *eyey2 = -1;
    return 0;
}

#ifdef CompiledNets
extern "C" {
    void eye_net(int* inputs, float* outputs);
}

int SearchEyesCompiled(int numScales, QImage* imagePyramid[],
    QImage* mask,
    int xx, int yy, int s, int basenet,
    int* eyex1, int* eyey1, int* eyex2, int* eyey2)
{
    int scale = s - 7;
    if (scale < 0 && scale >= -3) scale = 0;
    if (scale >= 0)
    {
        double x1 = 0, y1 = 0, n1 = 0, x2 = 0, y2 = 0, n2 = 0;

        int* tmp = new int[mask->size];
        int startx1 = (int)floor(0.5 + xx * pow(1.2, s - scale)) - mask->width / 2;
        int endx1 = (int)floor(0.5 + (xx + 10) * pow(1.2, s - scale)) - mask->width / 2;
        int startx2 = (int)floor(0.5 + (xx + 10) * pow(1.2, s - scale)) - mask->width / 2;
        int endx2 = (int)floor(0.5 + (xx + 20) * pow(1.2, s - scale)) - mask->width / 2;
        int starty = (int)floor(0.5 + yy * pow(1.2, s - scale)) - mask->height / 2;
        int endy = (int)floor(0.5 + (yy + 10) * pow(1.2, s - scale)) - mask->height;
        QImage* image = imagePyramid[scale];
        if (startx1 < 0) startx1 = 0;
        if (endx1 >= image->width - mask->width + 1) endx1 = image->width - mask->width + 1;
        if (startx2 < 0) startx2 = 0;
        if (endx2 >= image->width - mask->width + 1) endx2 = image->width - mask->width + 1;
        if (starty < 0) starty = 0;
        if (endy >= image->height - mask->height + 1) endy = image->height - mask->height + 1;

        for (int y = starty; y < endy; y++)
        {
            {for (int x = startx2; x < endx2; x++)
            {
                int ptr = 0;
                int hist[256];
                for (int i = 0; i < 256; i++) hist[i] = 0;
                for (int j = 0; j < mask->height; j++) for (i = mask->width - 1; i >= 0; i--)
                {
                    int data = image->data(i + x, j + y);
                    tmp[ptr++] = data;
                    hist[data]++;
                }
                int map[256];
                int tot = 0;
                int* to = map;
                int* from = hist;
                for (i = 0; i < 256; i++) {
                    int old = tot;
                    tot += *(from++);
                    *(to++) = old + tot;
                }
                from = tmp;
                for (i = 0; i < mask->size; i++) {
                    *from = map[*from];
                    from++;
                }
                float output[1];
                eye_net(tmp, output);
                if (output[0] > 0)
                {
                    n2 += output[0];
                    x2 += output[0] * (x + mask->width / 2) * pow(1.2, scale);
                    y2 += output[0] * (y + mask->height / 2) * pow(1.2, scale);
                    /*          char name[1024];
                    sprintf(name,".t.c create line %g %g %g %g -fill yellow -tag face",
                            (x+mask->width/2)*pow(1.2,scale),
                            (y+mask->height/2)*pow(1.2,scale),
                            (x+mask->width/2)*pow(1.2,scale)+1,
                            (y+mask->height/2)*pow(1.2,scale));
                    Tcl_Eval(interp,name); */
                }
            }}

            {for (int x = startx1; x < endx1; x++)
            {
                int ptr = 0;
                int hist[256];
                for (int i = 0; i < 256; i++) hist[i] = 0;
                for (int j = 0; j < mask->height; j++) for (i = 0; i < mask->width; i++)
                {
                    int data = image->data(i + x, j + y);
                    tmp[ptr++] = data;
                    hist[data]++;
                }
                int map[256];
                int tot = -375;
                int* to = map;
                int* from = hist;
                for (i = 0; i < 256; i++) {
                    int old = tot;
                    tot += *(from++);
                    *(to++) = old + tot;
                }
                from = tmp;
                for (i = 0; i < mask->size; i++) {
                    *from = map[*from];
                    from++;
                }
                float output[1];
                eye_net(tmp, output);
                if (output[0] > 0)
                {
                    n1 += output[0];
                    x1 += output[0] * (x + mask->width / 2) * pow(1.2, scale);
                    y1 += output[0] * (y + mask->height / 2) * pow(1.2, scale);
                    /*          char name[1024];
                    sprintf(name,".t.c create line %g %g %g %g -fill yellow -tag face",
                            (x+mask->width/2)*pow(1.2,scale),
                            (y+mask->height/2)*pow(1.2,scale),
                            (x+mask->width/2)*pow(1.2,scale)+1,
                            (y+mask->height/2)*pow(1.2,scale));
                    Tcl_Eval(interp,name); */
                }
            }
            }
        }
        if (n1 > 0) {
            *eyex1 = (int)floor(0.5 + x1 / n1); *eyey1 = (int)floor(0.5 + y1 / n1);
        }
        else {
            *eyex1 = -1; *eyey1 = -1;
        }
        if (n2 > 0) {
            *eyex2 = (int)floor(0.5 + x2 / n2); *eyey2 = (int)floor(0.5 + y2 / n2);
        }
        else {
            *eyex2 = -1; *eyey2 = -1;
        }
        delete[] tmp;
        return 1;
    }
    *eyex1 = -1; *eyey1 = -1;
    *eyex2 = -1; *eyey2 = -1;
    return 0;
}
#endif

int SearchLeftEye(QImage* image, QImage* mask, double eyewidth,
    int* xeye, int* yeye, int leftEye)
{
    QImage* myimage = new QImage(NULL);
    CopyQImage(myimage, image);
    if (!leftEye) MirrorQImage(myimage);
    int centerScale = (int)(floor(0.5 + log(eyewidth / 22.0) / log(1.2)));
    int lowScale = centerScale - 2;
    int highScale = centerScale + 2;
    if (lowScale < 0) lowScale = 0;
    if (highScale < 0) highScale = 0;
    //  lowScale=0;
    //  highScale=100;
    double total = 0;
    double totalX = 0.0;
    double totalY = 0.0;
    int* tmp = new int[mask->size];
    for (int scale = 0; scale <= highScale &&
        myimage->width >= mask->width &&
        myimage->height >= mask->height;
        scale++) {
        if (scale >= lowScale) {
            for (int y = 0; y < myimage->height - mask->height + 1; y++)
                for (int x = 0; x < myimage->width - mask->width + 1; x++) {
                    int ptr = 0;
                    int hist[256];
                    for (int i = 0; i < 256; i++) hist[i] = 0;
                    for (int j = 0; j < mask->height; j++) for (int i = 0; i < mask->width; i++) {
                        int data = myimage->data(i + x, j + y);
                        tmp[ptr++] = data;
                        hist[data]++;
                    }
                    int map[256];
                    int tot = 0;
                    int* to = map;
                    int* from = hist;
                    for (int i = 0; i < 256; i++) {
                        int old = tot;
                        tot += *(from++);
                        *(to++) = old + tot;
                    }
                    double scaleFactor = 1.0 / tot;
                    FastForwardUnit* unit = &(real_fast_list[0]->unitList[1]);
                    for (int i = 0; i < mask->size; i++)
                        (unit++)->activation = map[tmp[i]] * scaleFactor - 1.0;

                    double output = FastForwardPass(real_fast_list[0]);
                    if (output > 0) {
                        /*            fprintf(stderr,"(%g,%g/%d,%d,%d)=%g\n",
                                            (x+mask->width/2)*pow(1.2,scale),
                                            (y+mask->height/2)*pow(1.2,scale),
                                            x+mask->width/2,y+mask->height/2,scale,
                                            output); */
                        total += output;
                        if (leftEye) {
                            totalX += output * (x + mask->width / 2) * pow(1.2, scale);
                        }
                        else {
                            totalX += output * (myimage->width - 1 - (x + mask->width / 2)) *
                                pow(1.2, scale);
                        }
                        totalY += output * (y + mask->height / 2) * pow(1.2, scale);
                    }
                }
        }
        ReduceSize(myimage, myimage);
    }
    if (total > 0) {
        *xeye = (int)floor(0.5 + totalX / total); *yeye = (int)floor(0.5 + totalY / total);
        delete[] tmp;
        delete myimage;
        return 1;
    }
    *xeye = -1; *yeye = -1;
    delete[] tmp;
    delete myimage;
    return 0;
}
