#pragma once

/* Structure used to return the location of a face */
struct FaceLocation {
    int x1, y1, x2, y2;
};

/* Structure used to return the location and orientation of a face,
   where orientation here means one of profile, half profile, or frontal.  */
struct FaceLocationOrientation {
    int x1, y1, x2, y2;
    int orientation;
};

/* Structure used to return the location, rotation, and size of a face */
struct FaceLocationRotation {
    int x, y;     /* Center of the face */
    double angle; /* Angle of face in radians,
                     0=upright, positive=counter-clockwise */
    int size;     /* Size of face in pixels */
};

/* Structure used to return the location, rotation, and size of a face,
   as well as a 20x20 window containing the detected face. */
struct FaceLocationRotationExtract {
    int x, y;     /* Center of the face */
    double angle; /* Angle of face in radians,
                     0=upright, positive=counter-clockwise */
    int size;     /* Size of face in pixels */
    unsigned char* extracted; /* 20x20 buffer containing the face */
};

/* Structure used to return the location of a face and eyes */
struct FaceEyeLocation {
    int x1, y1, x2, y2;
    int leftx, lefty, rightx, righty;
    int left, right;
};
