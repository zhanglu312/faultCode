//
// Created by zhanglu on 22/03/2020.
//

#include "codeRecognizor.h"

int main(int argc, char *argv[])
{
    // char file_path[] = "/home/psdz/zhanglu/project/AirConditionerFCode/all.txt";
    // char out_path[] = "/home/psdz/zhanglu/project/AirConditionerFCode/out2.txt";
    // RUN_FILE(file_path, out_path);

    if (argc != 3) {
        printf("wrong input number\n");
        return 0;
    }

    // char img_path[] = "/home/psdz/zhanglu/project/AirConditionerFCode/demo_data/L629_164724.684.jpg";
    char img_path[200]; 
    char out_path[200];
    sprintf(img_path, "%s", argv[1]);
    sprintf(out_path, "%s", argv[2]);
    RUN_IMG(img_path, out_path);

    // char txt_path[] = "/home/psdz/zhanglu/project/AirConditionerFCode/demo_data/43_092439.864.jpg";
    // float ratio = performanceCheck(txt_path);
    // printf("final ratio: %f\n", ratio);
    return 1;
}
