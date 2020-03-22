//
// Created by zhanglu on 22/03/2020.
//
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
using namespace cv;
#define LOCAL_ITER 3

const char LABEL[19] = {'0','1','2','3','4','5','6','7','8','9','A','C','E','F','H','J','L','P','U'};

void RUN_FILE(char* file_path, char* outpath);
void RUN_IMG(char* imgName, char* outpath);
float performanceCheck(char *path);

