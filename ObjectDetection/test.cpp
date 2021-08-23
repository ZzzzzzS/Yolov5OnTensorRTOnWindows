/*
 * @Author: 周子顺
 * @Date: 2021-08-18 15:31:58
 * @LastEditors: 周子顺
 * @LastEditTime: 2021-08-18 15:31:58
 */
#include "Detector.h"
#include "opencv2/videoio.hpp"
#include <ctime>

//int main()
//{
//	Detector Detect("test.engine");
//	cv::VideoCapture capture;
//	capture.open(0);
//
//	cv::Mat frame;
//	while (true)
//	{
//		if (!capture.read(frame))
//			continue;
//		auto tag1 = clock();
//		auto object=Detect.Detect(frame);
//		auto tag2 = clock();
//		std::cout << tag2 - tag1 << "ms" << std::endl;
//		Detect.DrawRectangle(frame, object);
//		cv::imshow("ok", frame);
//		cv::waitKey(1);
//	}
//}