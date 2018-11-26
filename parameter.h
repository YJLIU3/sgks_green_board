#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#define DEBUG_MSG true


const Size CALIBRATOR_BOARD_SIZE = Size(10, 4);

const Size BOARD_REAL_SIZE = Size(20, 20);

#define detect_corner_method 1 

#define beta_front 60.0 
#define beta_rear 60.0 

#define offsize_a 0
#define offsize_b 120

#define space_x 320
#define space_y 640

#define offsize_xx 60 + offsize_a
#define offsize_yy space_y

#define grid_size 15
#define grid_rows 18
#define grid_cols 9
#define dstImg_rows grid_size*(grid_rows - 1)
#define dstImg_cols grid_size*(grid_cols - 1)
#define image_size Size(dstImg_rows, dstImg_cols)

#define Size_BG Size(space_x, space_y)


const Size WEIGHT_BIGSIZE = Size(space_x, space_y);
const Size WEIGHT_BIGSIZE_AVI = Size(1280, 720); 
#define timeRatioThrdd 100.0

