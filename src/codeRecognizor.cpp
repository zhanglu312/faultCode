//
// Created by zhanglu on 22/03/2020.
//
#include "codeRecognizor.h"


int numClassifier(cv::Mat image, cv::Mat mean, cv::dnn::Net net)
{
    Mat dst(cv::Size(30, 50), CV_32FC3);
    for (int i = 0; i < 50; i++)
    {
        float* temp_row = dst.ptr<float>(i);
        for (int j = 0; j < 30; j++)
        {
            int ind = i*3*30+3*j;
            int a = image.data[ind];
            int b = mean.data[ind];
            temp_row[j*3] = a-b;

            a = image.data[ind+1];
            b = mean.data[ind+1];
            temp_row[j*3+1] = a-b;

            a = image.data[ind+2];
            b = mean.data[ind+2];
            temp_row[j*3+2] = a-b;
        }
    }

    cv::Mat inputBlob = cv::dnn::blobFromImage(dst, 1.0, cv::Size(30,50), cv::Scalar(0,0,0),false);

    net.setInput(inputBlob,"data");
    cv::Mat prob = net.forward();

    float max_score = -1.0f;
    int ind = -1;
    for (int i = 0; i < 19; i++)
    {
       if (prob.at<float>(0,i) > max_score)
       {
            max_score = prob.at<float>(0,i);
            ind = i;
       }
    }

    return ind;
}


void screenDetection(cv::Mat img, std::vector<Rect>& screen)
{
    CascadeClassifier screen_cascade;
    if (!screen_cascade.load("model/cascade.xml")) { 
        printf("--(!)Error loading xml\n"); 
        return; 
    }

    cv::Mat gray;
    if (img.channels() > 1) cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);       
    else img.copyTo(gray);

    screen_cascade.detectMultiScale(gray, screen, 1.1, 2, CV_HAAR_FIND_BIGGEST_OBJECT, Size(300, 200), Size(1200, 800));

    return;
}

void fineLocal(cv::Mat img, cv::Rect& screen)
{
    float border_scale = 0.2;
    int border_w = round(screen.width*border_scale / (2*(1+2*border_scale)));  
    int border_h = round(screen.height*border_scale / (2*(1+2*border_scale)));
    screen.x = screen.x + border_w;
    screen.y = screen.y + border_h;
    screen.width = screen.width - 2*border_w;
    screen.height = screen.height - 2*border_h;

    Mat gray;
    if (img.channels() > 1) cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    else img.copyTo(gray);

    for (int i = 0; i < LOCAL_ITER; i++) {
        // printf("x_start: %d, y_start: %d, x_end: %d, y_end: %d\n", screen.x, screen.y, screen.x+screen.width-1, screen.y+screen.height-1);
        cv::Mat screenCut;
        cv::Mat temp(screen.size(), gray.type());
        temp = gray(screen);
        temp.copyTo(screenCut);

        cv::Mat dst;
        cv::threshold(screenCut, dst, 0, 255, CV_THRESH_OTSU);
        

        float acc_x = 0;
        float acc_y = 0;
        int num = 0;
        for (int p = 0; p < dst.rows; p++) {
            for (int q = 0; q < dst.cols; q++) {
                if (dst.data[p*dst.cols + q] < 125) {
                    num++;
                    acc_x += q;
                    acc_y += p;
                }
            }
        }

        if (num == 0) return;
        acc_x /= num;
        acc_y /= num;  

        // printf("rows: %d, cols: %d\n", dst.rows, dst.cols); 
        // printf("acc_x: %f, acc_y: %f\n", acc_x, acc_y);  
        float w_half = (acc_x > screen.width-1-acc_x) ? acc_x : screen.width-1-acc_x;
        float h_half = (acc_y > screen.height-1-acc_y) ? acc_y : screen.height-1-acc_y;
        // printf("w_half: %f, h_half: %f\n", w_half, h_half);  

        int x_start = round(screen.x + acc_x - w_half);
        int y_start = round(screen.y + acc_y - h_half);
        x_start = (x_start < 0) ? 0 : x_start;
        y_start = (y_start < 0) ? 0 : y_start;
        int x_end = round(screen.x + acc_x + w_half);
        int y_end = round(screen.y + acc_y + h_half);
        x_end = (x_end >= gray.cols) ? (gray.cols-1) : x_end;
        y_end = (y_end >= gray.rows) ? (gray.rows-1) : y_end;

        // printf("x_start: %d, y_start: %d, x_end: %d, y_end: %d\n", x_start, y_start, x_end, y_end);  
        // imshow("temp", dst);
        // waitKey(0);

        screen.x = x_start;
        screen.y = y_start;
        screen.width = x_end - x_start + 1;
        screen.height = y_end - y_start + 1;
    }
    return;
}

cv::Mat rectifyScreen(cv::Mat screen)
{
    float border_scale = 0.2;
    int thresh = 2;
    Mat dst;
    if (screen.channels() > 1) cv::cvtColor(screen, dst, cv::COLOR_BGR2GRAY);
    else screen.copyTo(dst);

    int col_start = round(dst.cols/3);
    int col_end = 2*col_start;
    int row_start = round(dst.rows/3);
    int row_end = 2*row_start;
    int center_x = round(dst.cols/2);
    int center_y = round(dst.rows/2);

    // fit line
    std::vector<cv::Point> pts1, pts2;
    for (int i = col_start; i <= col_end; i++) {
        
        int line1_acc = 0;
        for (int j = center_y; j >= 0; j--) {
            if (dst.data[j*dst.cols + i] > 125) line1_acc++;
            if (line1_acc > thresh) {
                pts1.push_back(cv::Point(i,j));
                break;
            }   
        }

        int line2_acc = 0;
        for (int j = center_y; j < dst.rows; j++) {
            if (dst.data[j*dst.cols + i] > 125) line2_acc++;
            if (line2_acc > thresh) {
                pts2.push_back(cv::Point(i,j));
                break;
            }   
        }
    }

    cv::Vec4f line1, line2;
    if (pts1.size() > 2) cv::fitLine(pts1,line1,cv::DIST_HUBER,0,0.01,0.01);        
    if (pts2.size() > 2) cv::fitLine(pts2,line2,cv::DIST_HUBER,0,0.01,0.01);        

    std::vector<cv::Point> pts3,pts4;
    for (int i = row_start; i <= row_end; i++) {
        int line3_acc = 0;
        for (int j = center_x; j >= 0; j--) {
            if (dst.data[i*dst.cols + j] > 125) line3_acc++;
            if (line3_acc > thresh) {
                pts3.push_back(cv::Point(i,j));
                break;
            }   
        }

        int line4_acc = 0;
        for (int j = center_x; j < dst.cols; j++) {
            if (dst.data[i*dst.cols + j] > 125) line4_acc++;
            if (line4_acc > thresh) {
                pts4.push_back(cv::Point(i, j));
                break;
            }   
        }
    }

    cv::Vec4f line3, line4;
    if (pts3.size() > 2) cv::fitLine(pts3,line3,cv::DIST_HUBER,0,0.01,0.01);        
    if (pts4.size() > 2) cv::fitLine(pts4,line4,cv::DIST_HUBER,0,0.01,0.01);        
            
    // corner computation
    cv::Point corner13, corner14, corner23, corner24;
    if (pts1.size() > 2 && pts3.size() > 2) {
        float k1 = line1[1] / line1[0];
        float b1 = line1[3] + k1*(-line1[2]);
        float k2 = line3[1] / line3[0];
        float b2 = line3[3] + k2*(-line3[2]);

        float y = (b1+k1*b2) / (1-k1*k2);
        float x = k2*y+b2;

        corner13.x = x;
        corner13.y = y;
    }

    if (pts1.size() > 2 && pts4.size() > 2) {
        float k1 = line1[1] / line1[0];
        float b1 = line1[3] + k1*(-line1[2]);
        float k2 = line4[1] / line4[0];
        float b2 = line4[3] + k2*(-line4[2]);

        float y = (b1+k1*b2) / (1-k1*k2);
        float x = k2*y+b2;

        corner14.x = x;
        corner14.y = y;
    }

    if (pts2.size() > 2 && pts3.size() > 2) {
        float k1 = line2[1] / line2[0];
        float b1 = line2[3] + k1*(-line2[2]);
        float k2 = line3[1] / line3[0];
        float b2 = line3[3] + k2*(-line3[2]);

        float y = (b1+k1*b2) / (1-k1*k2);
        float x = k2*y+b2;

        corner23.x = x;
        corner23.y = y;
    }

    if (pts1.size() > 2 && pts4.size() > 2) {
        float k1 = line2[1] / line2[0];
        float b1 = line2[3] + k1*(-line2[2]);
        float k2 = line4[1] / line4[0];
        float b2 = line4[3] + k2*(-line4[2]);

        float y = (b1+k1*b2) / (1-k1*k2);
        float x = k2*y+b2;

        corner24.x = x;
        corner24.y = y;
    }

    // affine transform
    std::vector<cv::Point2f> corners(4);
    corners[0] = corner13;
    corners[1] = corner14;
    corners[2] = corner23;
    corners[3] = corner24;
    std::vector<cv::Point2f> corners_trans(4);
    corners_trans[0] = cv::Point2f(0, 0);
    corners_trans[1] = cv::Point2f(599, 0);
    corners_trans[2] = cv::Point2f(0, 399);
    corners_trans[3] = cv::Point2f(599, 399);
    cv::Mat transform = cv::getPerspectiveTransform(corners, corners_trans);
    cv::Mat rec_screen = cv::Mat::zeros(400, 600, CV_8UC3);
    cv::warpPerspective(screen, rec_screen, transform, rec_screen.size());

    return rec_screen;

}

cv::Mat numCut(cv::Mat screen, bool& exist_flag)
{
    int screen_border = 0; 
    int num_x = 310 + screen_border;
    int num_y = 275 + screen_border;
    int num_w = 115;
    int num_h = 50;
    int border = 3;
    int acc_thresh = 2;
    int count_thresh = 0;
    int col_start=-1, col_end=-1, row_start=-1, row_end=-1;
    // cut num image
    cv::Mat numRough;
    cv::Mat temp(cv::Size(num_w, num_h), screen.type());
    temp = screen(cv::Rect(num_x, num_y, num_w, num_h));
    temp.copyTo(numRough);
    // imshow("numRough", numRough);
    // waitKey(0);
   
    // convert 2 gray
    Mat dst;
    if (numRough.channels() > 1) cv::cvtColor(numRough, dst, cv::COLOR_BGR2GRAY);
    else numRough.copyTo(dst);

    // otsu thresh
    Mat numThresh;
    threshold(dst, numThresh, 0, 255, CV_THRESH_OTSU);
    // imshow("numThresh", numThresh);
    // waitKey(0);


    // edege judgement
    int count = 0;
    for (int i = 0; i < num_w; i++) {
        int temp_acc = 0;
        for (int j = 0; j < num_h; j++) {
            if (numThresh.data[j*num_w+i] < 125) temp_acc++;
        }

        if (temp_acc <= acc_thresh) count++;
        else {
            if (count > count_thresh || i == 0) {
                col_start = i;
                break;
            }
            count = 0;
        }
    }

    count = 0;
    for (int i = num_w-1; i >= 0; i--) {
        int temp_acc = 0;
        for (int j = 0; j < num_h; j++) {
            if (numThresh.data[j*num_w+i] < 125) temp_acc++;
        }

        if (temp_acc <= acc_thresh) count++;
        else {
            if (count > count_thresh || i == num_w-1) {
                col_end = i;
                break;
            }
            count = 0;
        }
    }

    count = 0;
    for (int i = 0; i < num_h; i++) {
        int temp_acc = 0;
        for (int j = 0; j < num_w; j++) {
            if (numThresh.data[i*num_w+j] < 125) temp_acc++;   
        }

        if (temp_acc <= acc_thresh) count++;
        else {
            if (count > count_thresh || i == 0) {
                row_start = i;
                break;
            }
            count = 0;
        }
    }

    count = 0;
    for (int i = num_h-1; i >= 0; i--) {
        int temp_acc = 0;
        for (int j = 0; j < num_w; j++) {
            if (numThresh.data[i*num_w+j] < 125) temp_acc++;   
        }

        if (temp_acc <= acc_thresh) count++;
        else {
            if (count > count_thresh || i == num_h-1) {
                row_end = i;
                break;
            }
            count = 0;
        }
    }
   
    num_x = num_x + col_start - border;
    num_y = num_y + row_start - border;
    num_w = col_end - col_start + 1 + 2* border;
    num_h = row_end - row_start + 1 + 2* border;
    cv::Mat numRec;
    // printf("col_start: %d, col_end: %d, row_start: %d, row_end %d\n", col_start, col_end, row_start, row_end);
    // printf("num_w: %d, num_h: %d\n", num_w, num_h);


    // if (0) {
    if (num_w < 30 || num_h < 30 || col_start < 0 || col_end < 0 || row_start < 0 || row_end < 0) {
        exist_flag = false;
        num_w = 1;
        num_h = 1;
        num_x = 0;
        num_y = 0;    
    } else exist_flag = true;
    

    cv::Mat temp2(cv::Size(num_w, num_h), screen.type());
    temp2 = screen(cv::Rect(num_x, num_y, num_w, num_h));
    temp2.copyTo(numRec);
    Mat cutResize;
    cv::resize(numRec,cutResize,cv::Size(num_w*50/num_h,50));

    return cutResize;
}

int FaultCodeRecognize(cv::Mat numImg, char *out)
{
    int overlap = 5;
    int interval = 30;       
    Mat temp_num(cv::Size(30, 50), numImg.type());
    cv::dnn::Net net;
    std::string prototxt = "model/fault_code.prototxt";
    std::string caffemodel = "model/fault_code.caffemodel";
    net = cv::dnn::readNetFromCaffe(prototxt, caffemodel);
    
    // mean data accept
    std::fstream file;
    file.open("faultModel/bgr.txt");
    Mat mean_data = Mat::zeros(50, 30, CV_8UC3);
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 30; j++) {
            char temp[5];
            file>>temp;
            mean_data.data[i*3*30+3*j] = atoi(temp);
            file>>temp;
            mean_data.data[i*3*30+3*j+1] = atoi(temp);
            file>>temp;
            mean_data.data[i*3*30+3*j+2] = atoi(temp);
        }
    }
    file.close();

    if (numImg.cols > 3*interval+overlap) {
        Mat c1;
        temp_num = numImg(cv::Rect(0, 0, 30, 50));
        temp_num.copyTo(c1);
        int ind1 = numClassifier(c1, mean_data, net);

        Mat c2;
        temp_num = numImg(cv::Rect(interval-overlap, 0, 30, 50));
        temp_num.copyTo(c2);
        int ind2 = numClassifier(c2, mean_data, net);

        Mat c3;
        temp_num = numImg(cv::Rect(interval*2+overlap, 0, 30, 50));
        temp_num.copyTo(c3);
        int ind3 = numClassifier(c3, mean_data, net);

        Mat c4;
        temp_num = numImg(cv::Rect(numImg.cols-interval, 0, 30, 50));
        temp_num.copyTo(c4);
        int ind4 = numClassifier(c4, mean_data, net);

        *out = LABEL[ind1];
        *(out+1) = LABEL[ind2];
        *(out+2) = '-';
        *(out+3) = LABEL[ind3];
        *(out+4) = LABEL[ind4];
        return 5;
    }
    else if (numImg.cols > 45) {
        Mat c1;
        temp_num = numImg(cv::Rect(0, 0, 30, 50));
        temp_num.copyTo(c1);
        int ind1 = numClassifier(c1, mean_data, net);

        Mat c2;
        temp_num = numImg(cv::Rect(numImg.cols-interval, 0, 30, 50));
        temp_num.copyTo(c2);
        int ind2 = numClassifier(c2, mean_data, net);
        
        *out = LABEL[ind1];
        *(out+1) = LABEL[ind2];
        return 2; 
    }


}


void RUN_FILE(char* file_path, char* outpath)
{
    std::fstream fileName;
    fileName.open(file_path);
    if (!fileName.is_open()) {
        printf("failed to open INPUT txt file!\n");
        return;
    }            
    FILE *fp=fopen(outpath,"w");
    if (fp == NULL) {
        printf("failed to open OUTPUT txt file!\n");
        return;
    }

    char name_temp[100];
    int file_ind = 0;
    while(fileName.getline(name_temp, 100)) {
        printf("Index:%d, %s\n", file_ind, name_temp);
        file_ind++;

        char inCode[50];
        int len = strlen(name_temp);
        int in_start, in_end;
        for (int i = len-1; i >= 0; i--) {
            if (name_temp[i] == '/') {
                in_start = i+1;
                break;
            }
        }

        for (int i = in_start; i <= len-1; i++) {
            inCode[i-in_start] = name_temp[i];
        }
        inCode[len-in_start] = 0;

        char result[200];
        sprintf(result,"%s ", inCode);

        // char result[200];
        // sprintf(result,"%s: ", name_temp);

        cv::Mat img, gray;
        std::vector<Rect> screen;
        img = imread(name_temp);
        if(img.empty()) continue;
        if (img.channels() > 1) cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);       
        else img.copyTo(gray);   
        screenDetection(gray, screen); 


        if (screen.size() > 0) {
            fineLocal(gray, screen[0]);

            cv::Mat screenCut;
            cv::Mat temp(screen[0].size(), img.type());
            temp = img(screen[0]);
            temp.copyTo(screenCut);
            cv::Mat rec_screen = rectifyScreen(screenCut);

            bool exist_flag;
            cv::Mat numImg = numCut(rec_screen, exist_flag);

            if (exist_flag) {
                char out[5];
                int outNum = FaultCodeRecognize(numImg, out);
                for (int i = 0; i < outNum; i++) {
                    sprintf(result,"%s%c", result, out[i]);    
                }
            } else sprintf(result,"%s%d", result, -2);  
        } else sprintf(result,"%s%d", result, -1);
        fprintf(fp,"%s\n", result);
    }
    fileName.close();
    fclose(fp);

    return;
}


void RUN_IMG(char* imgName, char* outpath)
{
    FILE *fp=fopen(outpath,"a+");
    if (fp == NULL) {
        printf("failed to open OUTPUT txt file!\n");
        return;
    }

    char inCode[50];
    int len = strlen(imgName);
    int in_start, in_end;
    for (int i = len-1; i >= 0; i--) {
        if (imgName[i] == '/') {
            in_start = i+1;
            break;
        }
    }

    for (int i = in_start; i <= len-1; i++) {
        inCode[i-in_start] = imgName[i];
    }
    inCode[len-in_start] = 0;

    char result[200];
    sprintf(result,"%s ", inCode);

    cv::Mat img, gray;
    std::vector<Rect> screen;
    img = imread(imgName);
    if(img.empty()) {
        fclose(fp);
        return;
    }
    if (img.channels() > 1) cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);       
    else img.copyTo(gray);   
    screenDetection(gray, screen);

    if (screen.size() > 0) {
        fineLocal(gray, screen[0]); 

        cv::Mat screenCut;
        cv::Mat temp(screen[0].size(), img.type());
        temp = img(screen[0]);
        temp.copyTo(screenCut);
        // imshow("temp", temp);
        // waitKey(0);

        cv::Mat rec_screen = rectifyScreen(screenCut);
        // imshow("temp", rec_screen);
        // waitKey(0);
        // printf("%d, %d\n", rec_screen.cols, rec_screen.rows);

        bool exist_flag;
        cv::Mat numImg = numCut(rec_screen, exist_flag);
        // imshow("temp", numImg);
        // waitKey(0);

        if (exist_flag) {
            char out[5];
            int outNum = FaultCodeRecognize(numImg, out);
            for (int i = 0; i < outNum; i++) {
                // printf("%c", out[i]); 
                sprintf(result,"%s%c", result, out[i]);   
            }  
        } else sprintf(result,"%s%d", result, -2);//printf("no fault code\n");
        
    } else sprintf(result,"%s%d", result, -1);//printf("no screen detection\n");
    
    fprintf(fp,"%s\n", result);
    fclose(fp);

    return;
}

float performanceCheck(char *path)
{
    float ratio = 0.0f;
    std::fstream fileName;
    fileName.open(path);
    if (!fileName.is_open()) {
        printf("failed to open INPUT txt file!\n");
        return -1.0f;
    }            

    char name_temp[150];
    int allNum = 0;
    int falseNum = 0;
    while(fileName.getline(name_temp, 150)) {
        int len = strlen(name_temp);
        char outCode[10];
        char inCode[10];
        
        int out_start, in_start, in_end;
        for (int i = len-1; i >= 0; i--)
        {
            if (name_temp[i] == ' ') out_start = i+1;
            if (name_temp[i] == '_') in_end = i-1;
        }
        in_start = 0;


        for (int i = in_start; i <= in_end; i++) {
            inCode[i-in_start] = name_temp[i];
        }
        inCode[in_end-in_start+1] = 0;
        for (int i = out_start; i <= len-1; i++) {
            outCode[i-out_start] = name_temp[i];
        }
        outCode[len-out_start] = 0;
        int res = strcmp(inCode, outCode);

        if (res != 0) falseNum++;
        allNum++;
        // printf("%d %s, %d %s, rsult: %d\n", in_start, inCode, out_start, outCode, res);
    }
    ratio = 1-float(falseNum) / float(allNum);
    return ratio;
}
