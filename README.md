该工程用于室内线控器的故障码识别

经过编译工程最终生成3个可执行文件
TEST_FILE、TEST_IMG、TEST_PERFORMANCE
运行时，可执行文件应与model文件夹放在同一路径下面，以避免信息读取错误

1、TEST_FILE 调用形式示例如下
./TEST_FILE input.txt out.txt

共包含两个输入参数，其中第一个为输入的txt文件（txt文件可以使用绝对路径也可以使用想对于可执行文件的相对路径），文件包括相关图片的路径，如下所示（这里采用的是绝对路径，使用想对于可执行文件的相对路径也可以）
/home/psdz/zhanglu/project/AirConditionerFCode/faultCodeReg/sample/4A_170023.811.jpg
/home/psdz/zhanglu/project/AirConditionerFCode/faultCodeReg/sample/92_103427.097.jpg

第二个参数为输出的.txt文件名，其输出格式如下所示
4A_170023.811.jpg 4A
92_103427.097.jpg 92
其中第一个为输入的文件名称，第二个为识别到的故障码

2、TEST_IMG 调用形式示例如下
./TEST_IMG sample/4A_170023.811.jpg test.txt
共包含两个输入参数，其中第一个为输入的.jpg文件（.jpg可以使用绝对路径也可以使用想对于可执行文件的相对路径），第二个为输出的.txt文件，输出的结果会添加在之前的结果之后，输出格式同上所示

3、TEST_PERFORMANCE 用于对运算的结果进行统计，调用形式示例如下
 ./TEST_PERFORMANCE out.txt
输入参数为TEST_FILE、TEST_IMG输出的相应格式的.txt文件，最终的识别率会在终端进行打印



