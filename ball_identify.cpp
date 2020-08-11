#include <stdio.h>
#include <iostream>
// use VideoCapture
#include "opencv2/videoio.hpp"
// use namedWindow, createTrackbar, destroyAllWindows
#include "opencv2/highgui.hpp"
// use FileStorage, Mat, Scalar, inRange, Point2f, cvRound
#include "opencv2/core.hpp"
// use cvtColor, GaussianBlur, HoughCircle, circle
#include "opencv2/imgproc.hpp"
// #include <opencv2/core.hpp>

using namespace std;
using namespace cv;

int main(int argc, char** argv){

	// parameters of thte image working on
	int height, width;
	int t1min, t2min, t3min, t1max, t2max, t3max;

	Mat threshold = Mat(2, 3, CV_32FC1);

	FileStorage temp = FileStorage("threshold_matrix", FileStorage::READ);

	// load previous values of the threshold if exist
	if(temp.isOpened()){
		temp["threshold_matrix"] >> threshold;
		cout << threshold << endl;
		// t1min = threshold.at<int>(0, 0); t2min = threshold.at<int>(0, 1); t3min = threshold.at<int>(0, 2);
		// t1max = threshold.at<int>(1, 0); t2max = threshold.at<int>(1, 1); t3max = threshold.at<int>(1, 2);
	}
	temp.release();

	// index = 0 opens default camera
	// VideoCapture capture = VideoCapture(0);
	// if(!capture.isOpened()){
	// 	printf("ERROR: capture is NULL\n");
	// 	return -1;
	// }

	Mat frame = imread(argv[1], IMREAD_COLOR);
	// capture.read(frame);

	// create the window where imaged will be shown
	namedWindow("Camera", WINDOW_AUTOSIZE);
	namedWindow("HSV", WINDOW_AUTOSIZE);
	namedWindow("F1", WINDOW_AUTOSIZE);
	namedWindow("F2", WINDOW_AUTOSIZE);
	namedWindow("F3", WINDOW_AUTOSIZE);

	// create trackbars
	createTrackbar("t1min", "F1", &t1min, 260 , NULL);
	createTrackbar("t1max", "F1", &t1max, 260,  NULL);
	createTrackbar("t2min", "F2", &t2min, 260 , NULL);
	createTrackbar("t2max", "F2", &t2max, 260,  NULL);
	createTrackbar("t3min", "F3", &t3min, 260 , NULL);
	createTrackbar("t3max", "F3", &t3max, 260,  NULL);

	// load threshold from slider bars in the 2 parameters
	Scalar hsv_min = Scalar(t1min, t2min, t3min, 0);
	Scalar hsv_max = Scalar(t1max, t2max, t3max, 0);

	// image data
	height = frame.rows;
	width = frame.cols;

	// initialize different images
	Mat hsv_frame = Mat(height, width, CV_32FC1);
	Mat mask = Mat(height, width, CV_32FC1);
	Mat mask1 = Mat(height, width, CV_32FC1);
	Mat mask2 = Mat(height, width, CV_32FC1);
	Mat mask3 = Mat(height, width, CV_32FC1);
	Mat filtered = Mat(height, width, CV_32FC1);

	int c = 1;
	while(c--){
		// update threshold from slider bars
		hsv_min = Scalar(t1min, t2min, t3min, 0);
		hsv_max = Scalar(t1max, t2max, t3max, 0);

		// get 1 frame
		// if(!capture.read(frame)){
		// 	printf("ERROR: frame is NULL\n");
		// 	break;
		// }

		// convert color space to HSV (much easier to filter colors)
		cvtColor(frame, hsv_frame, COLOR_BGR2HSV);

		// filter out colors out of range
		inRange(hsv_frame, hsv_min, hsv_max, mask);

		// for visual purpose
		// --------start--------
		// split(split image into 3 1-D images)
		vector<Mat> masks = {mask1, mask2, mask3};
		split(hsv_frame, masks);

		// filter out colors out of range
		inRange(mask1, Scalar(t1min, 0, 0, 0), Scalar(t1max, 0, 0, 0), mask1);
		inRange(mask2, Scalar(t2min, 0, 0, 0), Scalar(t2max, 0, 0, 0), mask2);
		inRange(mask3, Scalar(t3min, 0, 0, 0), Scalar(t3max, 0, 0, 0), mask3);
		// ---------end---------

		// memory for hough circles

		// hough detector works better with some smoothing of the image
		// TO-DO sigma_x, sigma_y
		GaussianBlur(mask, mask, cv::Size(9, 9), 0, 0);

		// hough transform to detect circle
		vector<Vec3f> circles;
		HoughCircles(mask, circles, HOUGH_GRADIENT, 2,
			mask.rows/4, 200, 100);

		for(int i = 0; i < circles.size(); i++){
			// get the center and radius
			Point2f center(circles[i][0], circles[i][1]);
			float radius = circles[i][2];
			printf("ball: x=%f y=%f r=%f\n", center.x, center.y, radius);

			// draw the circle center
			circle(frame, center, 2, Scalar(0,255,0), -1, 8, 0);
			// draw the circle outline
			circle(frame, center, radius, Scalar(0,0,255), 2, 8, 0);
		}

		imshow("input", frame); // original input with detected ball overlay
		// for test purpose
		// --------start--------
		imshow("HSV", hsv_frame); // original input in HSV color space
		imshow("after color filtering", mask); // the input after color filtering
		// individual filters
		// imshow("F1", mask1);
		// imshow("F2", mask2);
		// imshow("F3", mask3);

		waitKey(0);
	}

	// save threshold values
	FileStorage out = FileStorage("threshold_matrix", FileStorage::WRITE);
	out << "threshold_matrix" << threshold;

	// release memory
	out.release();
	// capture.release();
	destroyAllWindows();

	return 0;
}

























