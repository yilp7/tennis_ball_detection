#include <set>
#include <queue>
#include <vector>
#include <utility>
#include <iostream>
#include <sys/time.h>

#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

using namespace std;
using namespace cv;

#define PI 3.14159265358979323846

typedef pair<int, int> Pair2i;      // coord (x, y)
typedef pair<Pair2i, float> Pairc;  // circle (center, raius)

Mat frame, hsv_frame, mask, output;
Vec3b get_hsv(Mat image);
Scalar hsv_min = Scalar(0.11*256, 0.60*256, 0.40*256, 0);
Scalar hsv_max = Scalar(0.25*256, 1.00*256, 1.00*256, 0);

void find_circles(Mat& img, vector<Pairc>& c);
void process_node(Mat& m, queue<Pair2i>& q, int x, int y);
void print_circles(vector<Pairc> circles){
	for(Pairc p: circles){
		printf("(%d, %d), %.2f\n", p.first.first, p.first.second, p.second);
	}
}

int main(int argc, char** argv){
	char* filename = (char*)malloc(50*sizeof(char));
	char* out_name = (char*)malloc(50*sizeof(char));
	
	while(scanf("%s", filename) != EOF){
		// struct timeval tv1,tv2;
		// double time;
		// gettimeofday(&tv1, 0);

		// load image
		frame = imread(filename, IMREAD_COLOR);
		// resize(frame, frame, cv::Size(800, 800));
		int height = frame.rows, width = frame.cols;
		float ratio = height*1.0/width;
		// resize(frame, frame, cv::Size(1600, 1600*ratio));

		// create hsv format matrices
		hsv_frame = Mat(height, width, CV_8UC3);
		mask = Mat(height, width, CV_8UC1);

		// convert color space to HSV (much easier to filter colors)
		cvtColor(frame, hsv_frame, COLOR_BGR2HSV);

		// filter out colors out of range
		inRange(hsv_frame, hsv_min, hsv_max, mask);

		// smooth the image: Gaussian blur
		// TO-DO: sigma_x, sigma_y
		GaussianBlur(mask, mask, cv::Size(15, 15), 0, 0);
		// sprintf(out_name, "mask%s", strrchr(filename, '/'));
		// imwrite(out_name, mask);

		// gettimeofday(&tv2, 0);
		// time = tv2.tv_sec-tv1.tv_sec + (tv2.tv_usec-tv1.tv_usec)/1000000.0;
		// printf("time taken (process mat): %f\n", time);

		vector<Pairc> circles;
		find_circles(mask, circles);

		// TO-DO: determine if false circle

		// change - theo
		int x, y, h, w;
		Rect circle_rect;

		for(Pairc p: circles){
			Point2f center(p.first.first, p.first.second);
			float radius = p.second;
			// draw the circle center;
			// circle(frame, center, 2, Scalar(0,255,0), -1, 8, 0);
			// draw the circle outline

			x = (center.x - radius);
			y = (center.y - radius);
			h = radius * 2;
			w = radius * 2;
			circle_rect = Rect(x, y, h, w);

			Vec3b hsv = get_hsv(frame(circle_rect));
			rectangle(frame, circle_rect, Scalar(255,255,255), 2, 8, 0);
			circle(frame, center, radius, Scalar(0,0,255), 2, 8, 0);
		}		

		sprintf(out_name, "out%s", strrchr(filename, '/'));
		printf("%s: %s\n", "Scan output: ", out_name);
		imwrite(out_name, frame);

	}

	free(filename);
	free(out_name);
	
	return 0;
}

void find_circles(Mat& img, vector<Pairc>& circles){
	// struct timeval tv1,tv2;
	// double time;
	// gettimeofday(&tv1, 0);

	Mat f = img.clone();
	int height = img.rows, width = img.cols;
	queue<Pair2i> q;

	int top, bottom, left, right, area;
	int x, y, flag = 0;
	Pair2i center;

	for(int i = 0; i < height; i++){
		for(int j = 0; j < width; j++){
			if(!f.at<unsigned char>(i, j)) continue;
			area = 0;
			top = i;
			left = right = j;
			
			// use BFS to find connected components
			q.push(make_pair(i, j));
			while(1){
				x = q.front().first, y = q.front().second;
				area++;
				q.pop();
				if(y < left)  left = y;
				if(y > right) right = y;

				for(int dx = -1; dx <= 1; dx++){
					for(int dy = -1; dy <= 1; dy++){
						process_node(f, q, x+dx, y+dy);
					}
				}
				// search ended
				if(q.empty()){
					bottom = x;
					break;
				}
			}
			if(area < 100) continue;
		
			center.first = (left + right) / 2;
			center.second = (top + bottom) / 2;
			circles.push_back(make_pair(center, sqrt(area / PI)));
		}
	}
	// gettimeofday(&tv2, 0);
	// time = tv2.tv_sec-tv1.tv_sec + (tv2.tv_usec-tv1.tv_usec)/1000000.0;
	// printf("time taken (find circle): %f\n", time);
}

Vec3b get_hsv(Mat image){
	// Mat copy_img = image;
	// Rect roi = Rect(10,20,40,60);
	// Mat roi_img = copy_img(region);
	// rectangle(copy_img, roi, Scalar(255,255,255), 1, 8, 0);
	Mat HSV;
	// Mat RGB = copy_img(roi); // use your x and y value

	cvtColor(image, HSV, COLOR_BGR2HSV);
	Vec3b hsv = HSV.at<Vec3b>(0,0);
	int H = hsv.val[0]; //hue
	int S = hsv.val[1]; //saturation
	int V = hsv.val[2]; //value
	printf("H: %i\tS: %i\tV: %i\n", H, S, V);
	return hsv;
}

void process_node(Mat& m, queue<Pair2i>& q, int x, int y){
	if(x < m.rows && y > 0 && y < m.cols && m.at<unsigned char>(x, y)){
		q.push(make_pair(x, y));
		m.at<unsigned char>(x, y) = 0;
	}
}


