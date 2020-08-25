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
#define max(a, b) (((a)<(b))?(a):(b))
#define min(a, b) (((a)>(b))?(a):(b))

typedef pair<int, int> Pair2i;      // coord (x, y)
typedef struct{
	int x;
	int y;
	float radius;
	float dist;
	float angle; // range from -π/2 to π/2 in rad
}Circle;

Mat frame, hsv_frame, mask, output;
// vector<float> hsv_min = {0.11*255, 0.60*255, 0.40*255, 0};
// vector<float> hsv_max = {0.25*255, 1.00*255, 1.00*255, 0};
Scalar hsv_min = Scalar(0.11*255, 0.60*255, 0.40*255, 0);
Scalar hsv_max = Scalar(0.25*255, 1.00*255, 1.00*255, 0);

void find_circles(Mat& img, vector<Circle*>& c);
void process_node(Mat& m, queue<Pair2i>& q, int x, int y);
void print_circles(vector<Circle*> circles){
	for(Circle* c: circles){
		printf("(%d, %d), %.2f\n", c->x, c->y, c->radius);
	}
}

int main(int argc, char** argv){
	char* filename = (char*)malloc(50*sizeof(char));
	char* out_name = (char*)malloc(50*sizeof(char));
	char* str = (char*)malloc(50*sizeof(char));
	
	while(scanf("%s", filename) != EOF){
		struct timeval tv1,tv2;
		double time;
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
		gettimeofday(&tv1, 0);
		cvtColor(frame, hsv_frame, COLOR_BGR2HSV);
		gettimeofday(&tv2, 0);
		time = tv2.tv_sec-tv1.tv_sec + (tv2.tv_usec-tv1.tv_usec)/1000000.0;
		// printf("time taken (convert to hsv): %f\n", time);

		// filter out colors out of range
		gettimeofday(&tv1, 0);
		inRange(hsv_frame, hsv_min, hsv_max, mask);
		gettimeofday(&tv2, 0);
		time = tv2.tv_sec-tv1.tv_sec + (tv2.tv_usec-tv1.tv_usec)/1000000.0;
		// printf("time taken (filter by color): %f\n", time);

		// smooth the image: Gaussian blur
		// TO-DO: sigma_x, sigma_y
		gettimeofday(&tv1, 0);
		GaussianBlur(mask, mask, cv::Size(15, 15), 0, 0);
		// sprintf(out_name, "mask%s", strrchr(filename, '/'));
		// imwrite(out_name, mask);
		gettimeofday(&tv2, 0);
		time = tv2.tv_sec-tv1.tv_sec + (tv2.tv_usec-tv1.tv_usec)/1000000.0;
		// printf("time taken (smooth image): %f\n", time);

		gettimeofday(&tv1, 0);
		vector<Circle*> circles;
		find_circles(mask, circles);
		gettimeofday(&tv2, 0);
		time = tv2.tv_sec-tv1.tv_sec + (tv2.tv_usec-tv1.tv_usec)/1000000.0;
		// printf("time taken (find balls): %f\n", time);

		gettimeofday(&tv1, 0);
		int i = 0;
		// TO-DO: determine if false circle
		for(Circle* c: circles){
			Point2f center(c->x, c->y);
			circle(frame, center, c->radius, Scalar(0, 0, 255), 2, 8, 0);

			sprintf(str, "%.3f", c->angle);
			// putText(frame, str, center, FONT_HERSHEY_SIMPLEX, 3, Scalar(0, 0, 0));

			int len = strrchr(filename, '.') - strrchr(filename, '/');
			sprintf(out_name, "roi%.*s-%d.jpeg", len, strrchr(filename, '/'), i++);
			// imwrite(out_name, frame(Rect2f(center.x - radius, center.y - radius, 2*radius, 2*radius)));
		}
		gettimeofday(&tv2, 0);
		time = tv2.tv_sec-tv1.tv_sec + (tv2.tv_usec-tv1.tv_usec)/1000000.0;
		// printf("time taken (draw circles): %f\n", time);

		gettimeofday(&tv1, 0);
		sprintf(out_name, "out%s", strrchr(filename, '/'));
		imwrite(out_name, frame);
		gettimeofday(&tv2, 0);
		time = tv2.tv_sec-tv1.tv_sec + (tv2.tv_usec-tv1.tv_usec)/1000000.0;
		// printf("time taken (write image): %f\n", time);

		// print_circles(circles);

		// gettimeofday(&tv2, 0);
		// time = tv2.tv_sec-tv1.tv_sec + (tv2.tv_usec-tv1.tv_usec)/1000000.0;
		// printf("time taken (total): %f\n", time);
	}

	// TO-DO: release
	free(filename);
	free(out_name);
	
	return 0;
}

void find_circles(Mat& img, vector<Circle*>& circles){
	Mat f = img.clone();
	int height = img.rows, width = img.cols;
	queue<Pair2i> q;

	int top, bottom, left, right, area;
	int x, y, flag = 0;

	for(int i = 0; i < height; i++){
		for(int j = 0; j < width; j++){
			if(!f.at<unsigned char>(i, j)) continue;

			area = 0; top = i; left = right = j;
			
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
				if(q.empty()){bottom = x; break;}
			}
			if(area < 100) continue;
		
			Circle* temp = (Circle*)malloc(sizeof(Circle));
			temp->x = (left + right) / 2;
			temp->y = (top + bottom) / 2;
			temp->radius = sqrt(area / PI);

			/* for distance (pinhole camera model):
			 *   x * d = X * f   -> d = X * f / x
			 * x: size of obj on img
			 * d: dist                -- also in mm
			 * X: actual size of obj  -- use 1.3 inch (33.02 mm) for radius
			 * f: focal length        -- use 3.95 mm for huawei mate 10 pro
			 * 
			 * sensor size: 5.0 mm x 3.0 mm
			 * max resolution: 5120 x 3840
			 * -> average: 1144.867 px/mm
			 */
			temp->dist = 33.02 * 3.95 / (temp->radius / 1144.867);
			temp->angle = PI/2 - atan2(temp->dist, temp->x - width/2);

			circles.push_back(temp);
		}
	}
}

void process_node(Mat& m, queue<Pair2i>& q, int x, int y){
	if(x < m.rows && y > 0 && y < m.cols && m.at<unsigned char>(x, y)){
		q.push(make_pair(x, y));
		m.at<unsigned char>(x, y) = 0;
	}
}


// TO-DO: find self position, use net or lines as baseline
void find_self(Mat& img){
	
}

























