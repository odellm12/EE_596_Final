#ifndef EyeIncluded
#define EyeIncluded

int SearchLeftEye(QImage* image, QImage* mask, double eyewidth,
    int* xeye, int* yeye, int leftEye);

int SearchEyes(int numScales, QImage* imagePyramid[], QImage* mask,
    int xx, int yy, int s, int basenet,
    int* eyex1, int* eyey1, int* eyex2, int* eyey2);

#endif
