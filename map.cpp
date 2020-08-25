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

class tennis_map{

	// lengths in ft
	const float sideline = 78;
	const float baseline = 36;
	const float sidescreen = 120;
	const float backscreen = 60;
	const int multiplier = 8;
	
	Mat court;
	Point self_pos;
	Vector 

	void setup(){
		// self_pos = pos;
		int y = ((backscreen*multiplier) - (baseline*multiplier)) / 2;
		int x = ((sidescreen*multiplier) - (sideline*multiplier)) / 2;
		Point net1 = Point(480, 384);
		Point net2 = Point(480, 96);
		// printf("x: %i\ty: %i", x, y);
		court = Mat(backscreen*multiplier, sidescreen*multiplier, CV_8UC3, Scalar(153, 255, 102));
		Rect court_lines = Rect(x, y, sideline*multiplier, baseline*multiplier);
		line(court, net1, net2, Scalar(255,255,255), 2);
		rectangle(court, court_lines, Scalar(255,255,255), 2, 8, 0);
	}

	void get_tennis_court(){
		//
	}

	void get_self_pos(){
		//
	}	

	void get_tennis_pos(){
		//
	}



	public:
		int show(){
			imshow("Court Map", court);

			int key = waitKey(0);
			if(key == 'q'){
				return 0;
			}
		}

		vector_map(){
			setup();
		}
};

int main(){
	tennis_map map;
	map.show();
}

