//
// Created by zhanglu on 22/03/2020.
//

#include "codeRecognizor.h"

int main()
{
    // char file_path[] = "/home/psdz/zhanglu/project/AirConditionerFCode/all.txt";
    // char out_path[] = "/home/psdz/zhanglu/project/AirConditionerFCode/out2.txt";
    // RUN_FILE(file_path, out_path);

    // char img_path[] = "/home/psdz/zhanglu/project/AirConditionerFCode/demo_data/L629_164724.684.jpg";
    // char img_path[] = "/home/psdz/zhanglu/project/AirConditionerFCode/demo_data/40_181147.817.jpg";
    // char out_path[] = "/home/psdz/zhanglu/project/AirConditionerFCode/out3.txt";
    // RUN_IMG(img_path, out_path);

    char txt_path[] = "/home/psdz/zhanglu/project/AirConditionerFCode/out2.txt";
    float ratio = performanceCheck(txt_path);
    printf("final ratio: %f\n", ratio);
    return 0 ;
}
