/* Face Detector Library

   By Henry A. Rowley (har@cs.cmu.edu, http://www.cs.cmu.edu/~har)
   Developed at Carnegie Mellon University for the face detection project.
   Code may be used, but please provide appropriate acknowledgements, and let
   har@cs.cmu.edu how you are using it.  Thanks!
   */

#ifndef DetectorIncluded
#define DetectorIncluded

#include "structs.h"

#include "context.h"
#include "../../../Common/Frame/Frame.h"

#include <QList>
#include <QSize>
#include <stdio.h>

   /* Search for an eye.  Given an image of pixels, run the eye detector
      everywhere in that image, and return the centroid of the eye
      detections.  Other inputs are: a guess about the width of the eye
      (at least 22 pixels), and whether you are looking for the left eye
      (in the image; this means looking for the person's right eye).
      Returns 1 if an eye is detected, 0 otherwise */
extern int SearchEye(int width, int height, unsigned char* buffer,
    int eyeWidthGuess, int leftEye,
    int* x, int* y);

/* This function is for the Informedia project.

   Function to count and locate all faces in an image.
   Input buffer is an array of unsigned char, 3 bytes per
   pixel indicating the red, green, and blue values.  The
   width and height are the width and height in pixels.
   The return value is the number of faces found.  If locs
   is NULL, the actual locations are discarded.  Otherwise,
   if no faces were found, *locs is set to NULL.  If faces
   were found, *locs is set to a pointer to an array of
   struct FaceLocation.  It is the responsibility of the caller
   to free() this array. */
extern int Informedia_FindFaces(int width, int height, unsigned char* buffer,
    struct FaceLocation** locs);

/* Just like Informedia_FindFaces, but uses compiled networks, if available. */
#ifdef CompiledNets
extern int Informedia_FindFaces_Compiled(int width, int height,
    unsigned char* buffer,
    struct FaceLocation** locs);
#endif

/* Just like Informedia_FindFaces, but takes images in RGBA format, 4
   bytes/pixel */
extern int Informedia_FindFacesRGBA(int width, int height,
    unsigned char* buffer,
    struct FaceLocation** locs);

/* Again, just like Informedia_FindFaces, but this one tries to detect
   profile views as well.  It uses the FaceLocationOrientation structure
   to return the face, along with a number saying which direction the
   face was looking.  This number is -2 for looking to the left, -1 for
   looking partially to the left, 0 for looking at the camera, 1 for
   looking partially to the right, and 2 for looking to the right. */
extern int Informedia_FindFacesOrientation(
    int width, int height, unsigned char* buffer,
    struct FaceLocationOrientation** locs);

/* This function is for the Informedia project.  It tries to provide more
   detections, as follows:  First, run the candidate detector everywhere,
   recording the detection locations.  Then, sort the candidates from best
   to worst, without applying a threshold.  Finally, apply the verification
   networks to these candidates, and send the results out via a callback
   function.  This callback function (which the user must provide) can
   return 1 (meaning continue searching) or 0 (stop now).

   Like the previous function, this one takes a color image as input.
   One new parameter: the number of verification networks that
   must respond positively for a candidate to be reported.  This must
   be a number between 0 and 2.  0 means report all candidates, no
   matter what; 1 means at least one verifier must give a response > 0,
   and 2 menas both must respond positively (at the same place).

   The arguments provided to the callback function are as follows:
   candidateRating: output of candidate network on face
   cx, yc, sc: location of candidate detection (upper left coordinates,
               size of search area; note that the search area is 30 pixels
               (scaled by the current resolution), and that the face is
               really a 20 pixel region somewhere inside that box.)
   verifyRating1: maximum output of verifier 1 over search area
   x1, y1, s1: location of maximum for verifier 1
   verifyRating2: maximum output of verifier 2 over search area
   x2, y2, s2: location of maximum for verifier 2

   The return value should be 1 to continue searching, or zero to
   have the search function (Informedia_FindFacesAllCandidates) return
   immediately.

   Although all the candidates returned will be unique, the more
   refined positions from the verification networks may not be--candidates
   that are close enough to one another may actually be the same face,
   and this will be picked up by the verification networks.
   */
typedef int (*Informedia_CandidateCallback)(double candidateRating,
    int xc, int yc, int sc,
    double verifyRating1,
    int x1, int y1, int s1,
    double verifyRating2,
    int x2, int y2, int s2);

extern void Informedia_FindFacesAllCandidates(int width, int height,
    unsigned char* buffer,
    int verificationThreshold,
    Informedia_CandidateCallback cb);

/* The previous versions of these functions (without the "Gray"
   designator) take RGB images, where each pixel is representated
   by three bytes.  The following two take grayscale images. */
extern int Informedia_FindFacesGray(int width, int height,
    unsigned char* buffer,
    struct FaceLocation** locs);

extern int Informedia_FindFacesOrientationGray(int width, int height,
    unsigned char* buffer,
    struct FaceLocationOrientation
    ** locs);

/* The second version is for the Xavier project.

   Function to count and locate all faces in an image.
   Input buffer is an array of unsigned char, 1 byte per
   pixel indicating the intensity.  The
   width and height are the width and height in pixels.
   The return value is the number of faces found.  If locs
   is NULL, the actual locations are discarded.  Otherwise,
   if no faces were found, *locs is set to NULL.  If faces
   were found, *locs is set to a pointer to an array of
   struct FaceLocation.  It is the responsibility of the caller
   to free() this array. */
extern int Xavier_FindAllFaces(int width, int height, unsigned char* buffer,
    struct FaceLocation** locs);

/* Same thing as above, but takes a color image as input, and
   uses skin color matching to reduce the search area */
extern int Xavier_FindAllFacesColor(int width, int height,
    unsigned char* buffer,
    struct FaceLocation** locs);

/* This function is also for the Xavier project.  In addition
   to the image buffer, it is given a region in the image which
   the center of the face must be located in, as well as a range
   of sizes for the face.  It returns 1 if there is a face, and
   0 if no faces are found.  The optimizationFlags variable controls
   various optimizations.  0 means use a conservative but slow method,
   while other values (defined below) give other behavoir.
   If prevImage is non-NULL, use motion to restrict the search.
   changeThreshold controls intensity change to detect motion; a good
   value to start with might be 20. */
extern int Xavier_FindOneFace(int width, int height, unsigned char* buffer,
    int minX, int maxX, int minY, int maxY,
    int minSize, int maxSize,
    int optimizationFlags,
    unsigned char* prevImage,
    int changeThreshold,
    struct FaceLocation* loc);

/* This function is also for the Xavier project.  In addition
   to the image buffer, it is given a region in the image which
   the center of the face must be located in, as well as a range
   of sizes for the face.  It returns 1 if there is a face, and
   0 if no faces are found.  The optimizationFlags variable controls
   various optimizations.  0 means use a conservative but slow method,
   while other values (defined below) give other behavoir.
   If prevImage is non-NULL, use motion to restrict the search.
   changeThreshold controls intensity change to detect motion; a good
   value to start with might be 20.
   This version uses color images; each pixel consists of three
   bytes, with the red, green and blue values. */
extern int Xavier_FindOneFaceColor(int width, int height,
    unsigned char* buffer,
    int minX, int maxX, int minY, int maxY,
    int minSize, int maxSize,
    int optimizationFlags,
    unsigned char* prevImage,
    int changeThreshold,
    struct FaceLocation* loc);

/* No optimizations */
#define Xavier_NoOptimizations 0

/* Try to verify candidate faces more quickly, may result in some
   missed faces.  Can be used with QuickVerifyNN. */
#define Xavier_QuickVerify 1

   /* Crop image before processing; doesn't help in practice and should
      not be used; motion, color, and QuickVerifyNN do not work with this. */
#define Xavier_CropImage 2

      /* Try to verify candidate faces more quickly, may result in some
         missed faces (uses a neural network).  Can be used with QuickVerify. */
#define Xavier_QuickVerifyNN 4

         /***********************************************/
         /* Interfaces for arbitration and slow version */
         /***********************************************/

         /* Function to count and locate all faces in an image.
            Input buffer is an array of unsigned char, 1 byte per
            pixel indicating the intensity.  The
            width and height are the width and height in pixels.
            The return value is the number of faces found.  If locs
            is NULL, the actual locations are discarded.  Otherwise,
            if no faces were found, *locs is set to NULL.  If faces
            were found, *locs is set to a pointer to an array of
            struct FaceLocation.  It is the responsibility of the caller
            to free() this array.  The mode parameters specifies which
            networks and which arbitration parameters to use.  The
            valid numbers are between 1 and 16, and are the same as
            the "System numbers" used in the tables in my paper. */
extern int Slow_FindAllFaces(int width, int height, unsigned char* buffer,
    int mode,
    struct FaceLocation** locs);

/*********************************************************/
/* Interfaces designed for the face/eye tracking project */
/*********************************************************/

/* Function to count and locate all faces in an image.
   Input buffer is an array of unsigned char, 1 byte per
   pixel indicating the intensity.  The
   width and height are the width and height in pixels.
   The return value is the number of faces found.  If locs
   is NULL, the actual locations are discarded.  Otherwise,
   if no faces were found, *locs is set to NULL.  If faces
   were found, *locs is set to a pointer to an array of
   struct FaceLocation.  It is the responsibility of the caller
   to free() this array.
   The structures will also contain the locations of the
   eyes, if they are found.  The left and right designations
   are from the user's point of view.  The flags left and right
   are set to 1 if that eye is detected, 0 if it is not detected,
   or -1 if the face is to small to attempt detection. */
extern int Track_FindAllFaces(int width, int height, unsigned char* buffer,
    struct FaceEyeLocation** locs);

/* Like the above function, but also takes parameters restricting the
   search to a rectangular region of the image, and restricts the size
   of the face that will be searched for.  The face must be completely
   within the specified window. */
extern QList<AlgDetOutput> Track_FindAllFacesRegion(const QSize& MinObjSize, int width, int height, char* buffer);

/* Function to count and locate all faces in an image.
   Input buffer is an array of unsigned char, 1 byte per
   pixel indicating the intensity.  The
   width and height are the width and height in pixels.
   The return value is the number of faces found.  If locs
   is NULL, the actual locations are discarded.  Otherwise,
   if no faces were found, *locs is set to NULL.  If faces
   were found, *locs is set to a pointer to an array of
   struct FaceLocation.  It is the responsibility of the caller
   to free() this array.
   The structures will also contain the locations of the
   eyes, if they are found.  The left and right designations
   are from the user's point of view.  The flags left and right
   are set to 1 if that eye is detected, 0 if it is not detected,
   or -1 if the face is to small to attempt detection.
   motionMask is an array of bytes.  If a byte is zero, then
   the function assumes there was motion there, and will check
   that portion of the image for faces.  Non-zero means no motion.
   If the motionMask is NULL, then the entire image is searched. */
extern int Siemens_FindAllFaces(int width, int height, unsigned char* buffer,
    unsigned char* motionMask,
    struct FaceEyeLocation** locs);

/***************************************************/
/* Interface for version using Genetic Programming */
/***************************************************/

/* This interface is for the Astro's Genetic Programming approach
   to the face detection problem.  The interface is pretty similar
   to the previous ones. */
extern int Astro_FindAllFaces(int width, int height, unsigned char* buffer,
    struct FaceLocation** locs);

/*************************************************/
/* Interface for the rotation invariant detector */
/*************************************************/

/* Function to count and locate all faces in an image.
   Input buffer is an array of unsigned char, 1 byte per
   pixel indicating the pixel's intensity.  The
   width and height are the width and height in pixels.
   The return value is the number of faces found.  If locs
   is NULL, the actual locations are discarded.  Otherwise,
   if no faces were found, *locs is set to NULL.  If faces
   were found, *locs is set to a pointer to an array of
   struct FaceLocation.  It is the responsibility of the caller
   to free() this array. */
extern int Rotation_FindAllFacesSlow(int width, int height,
    unsigned char* buffer,
    struct FaceLocationRotation** locs);

/* This function is like the previous one, but also extracts a 20x20
   pixel window around each face that has been detected, which can be
   used for recognition purposes. */
extern int Rotation_FindAllFacesSlowExtract(int width, int height,
    unsigned char* buffer,
    double threshold,
    int arbThresold,
    struct FaceLocationRotationExtract
    ** locs);

/************************************************/
/* Functions to manipulate the skin color model */
/************************************************/

/* This function makes the color mask more general.  It should be used
   when no faces are detected in a frame, to allow a wider range of skin
   tones.  amount ranges between 0 and 1, and specifies how much of
   the old color model should be replaced.  A good amount to use is 0.001 */
extern void BroadenColorMask(double amount);

/* This function should be called whenever a face is detected, and the
   color model should be updated with the new data.  width, height, and
   buffer specify the color image, in the same way as the detection
   functions.  x1,y1 to x2,y2 is the rectangle of the image which contains
   skin color.  Typically, for a (say) 20x20 pixel face, it is good to use
   the 10x10 pixel region centered on that face, and coorespondingly larger
   regions for larger faces.  The amount is between 0 and 1, and says how
   much of the old color model should be replaced; a good value to use is
   0.25. */
extern void UpdateColorMaskFromBuffer(int width, int height,
    unsigned char* buffer,
    int x1, int y1, int x2, int y2,
    double amount);

/*********************/
/* Utility Functions */
/*********************/

/* A function to load a .gif file into an array.  A pointer to an
   allocated buffer is returned; it is your responsibility to free() it. */
unsigned char* LoadGifToBuffer(char* filename, int* width, int* height);

/* A function to load a .pgm file into an array.  A pointer to an
   allocated buffer is returned; it is your responsibility to free() it. */
unsigned char* LoadPgmToBuffer(char* filename, int* width, int* height);

/* A function to load a .ppm file into an array.  A pointer to an
   allocated buffer is returned; it is your responsibility to free() it. */
unsigned char* LoadPpmToBuffer(char* filename, int* width, int* height);

/* A function to load a .pgm file into an array.  A pointer to an
   allocated buffer is returned; it is your responsibility to free() it. */
unsigned char* LoadPgmStreamToBuffer(FILE* inf, int* width, int* height);

/* A function to load a .ppm file into an array.  A pointer to an
   allocated buffer is returned; it is your responsibility to free() it. */
unsigned char* LoadPpmStreamToBuffer(FILE* inf, int* width, int* height);

/* Given the value of argv[0], set the detector paths to find the
   support files (NN architectures, etc). */
void SetDataPath(char* argv0);

#endif
