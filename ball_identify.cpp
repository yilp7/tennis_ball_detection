#include <stdio.h>
#include <iostream>
// use namedWindow, createTrackbar, destroyAllWindows
#include "opencv2/highgui.hpp"
// use Mat, Scalar, inRange, Point2f, cvRound
#include "opencv2/core.hpp"
// use cvtColor, GaussianBlur, HoughCircle, circle
// and getStructuringElement, morphologyEx
#include "opencv2/imgproc.hpp"
// #include <opencv2/core.hpp>

using namespace std;
using namespace cv;

void find_balls(char* filename, char* out_name, Scalar hsv_min, Scalar hsv_max, Mat se1, Mat se2);

int main(int argc, char** argv){

	char* filename = (char*)malloc(50*sizeof(char));
	char* out_name = (char*)malloc(50*sizeof(char));

	// windows for image display
	// namedWindow("input", WINDOW_KEEPRATIO);
	// namedWindow("HSV", WINDOW_KEEPRATIO);
	// namedWindow("after color filtering", WINDOW_KEEPRATIO);
	// resizeWindow("input", 800, 800*ratio);
	// resizeWindow("HSV", 800, 800*ratio);
	// resizeWindow("after color filtering", 800, 800);

	// load threshold from slider bars in the 2 parameters
	Scalar hsv_min = Scalar(0.11*256, 0.60*256, 0.20*256, 0);
	Scalar hsv_max = Scalar(0.25*256, 1.00*256, 1.00*256, 0);

	// TO-DO: magic numbers
	Mat se1 = getStructuringElement(MORPH_RECT, cv::Size(11, 11));
	Mat se2 = getStructuringElement(MORPH_RECT, cv::Size(21, 21));

	while(scanf("%s", filename) != EOF){
		find_balls(filename, out_name, hsv_min, hsv_max, se1, se2);
	}

	// TO-DO: release memory
	// free();
	// destroyAllWindows();

	return 0;
}

void find_balls(char* filename, char* out_name, Scalar hsv_min, Scalar hsv_max, Mat se1, Mat se2){
	// load image
	Mat frame = imread(filename, IMREAD_COLOR);
	int height = frame.rows, width = frame.cols;
	float ratio = height*1.0/width;

	// convert to HSV format
	Mat hsv_frame = Mat(height, width, CV_8UC3);
	Mat mask = Mat(height, width, CV_8UC1);

	// convert color space to HSV (much easier to filter colors)
	cvtColor(frame, hsv_frame, COLOR_BGR2HSV);
	// imwrite("hsv.jpg", hsv_frame);

	// filter out colors out of range
	inRange(hsv_frame, hsv_min, hsv_max, mask);

	// smooth the image: Gaussian blur
	// TO-DO: sigma_x, sigma_y
	GaussianBlur(mask, mask, cv::Size(15, 15), 0, 0);

	// remove the curving line on the move
	morphologyEx(mask, mask, MORPH_CLOSE, se2);
	morphologyEx(mask, mask, MORPH_OPEN, se1);

	// hough transform to detect circle
	vector<Vec3f> circles;
	HoughCircles(mask, circles, HOUGH_GRADIENT, 4,
		mask.rows/10, 100, 40, 0, 100);

	for(int i = 0; i < circles.size(); i++){
		// get the center and radius
		Point2f center(circles[i][0], circles[i][1]);
		float radius = circles[i][2];
		// printf("ball: x=%f y=%f r=%f\n", center.x, center.y, radius);

		// TO-DO: check if is false circle


		// draw the circle center
		circle(frame, center, 2, Scalar(0,255,0), -1, 8, 0);
		// draw the circle outline
		circle(frame, center, radius, Scalar(0,0,255), 2, 8, 0);
	}

	sprintf(out_name, "out/%s", strrchr(filename, '/')+1);
	imwrite(out_name, frame);
	sprintf(out_name, "mask/%s", strrchr(filename, '/')+1);
	Mat out_mask = Mat(800, 800, CV_8UC1);
	resize(mask, out_mask, cv::Size(800, 800));
	imwrite(out_name, out_mask);
	// imshow("input", frame); // original input with detected ball overlay
	// for test purpose
	// imshow("HSV", hsv_frame); // original input in HSV color space
	// imshow("after color filtering", mask); // the input after color filtering

	waitKey(0);
}























