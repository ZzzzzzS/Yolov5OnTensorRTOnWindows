/*
 * @Author: 周子顺
 * @Date: 2021-08-18 16:02:05
 * @LastEditors: 周子顺
 * @LastEditTime: 2021-08-18 16:02:36
 */

#ifndef DETECTOR_H
#define DETECTOR_H

#include <vector>
#include <opencv2/opencv.hpp>
#include <NvInferRuntime.h>
#include "logging.h"
#include "yololayer.h"

class Detector
{
public:
    Detector(std::string FileName);
    ~Detector();

	bool LoadEngine(std::string FileName);
	std::vector<Yolo::Detection> Detect(cv::Mat& InputMat);

	void DrawRectangle(cv::Mat& InOutMat,std::vector<Yolo::Detection>& Object);
	
private:
	nvinfer1::IRuntime* runtime;
	nvinfer1::ICudaEngine* engine;
	nvinfer1::IExecutionContext* context;

	static const int DEVICE = 0;  // GPU id
	const float NMS_THRESH = 0.4;
	const float CONF_THRESH = 0.5;
	static const int BATCH_SIZE = 1;

	// stuff we know about the network and the input/output blobs
	int INPUT_H;// = Yolo::INPUT_H;
	int INPUT_W;// = Yolo::INPUT_W;
	int CLASS_NUM;// = Yolo::CLASS_NUM;
	int OUTPUT_SIZE;// = Yolo::MAX_OUTPUT_BBOX_COUNT * sizeof(Yolo::Detection) / sizeof(float) + 1;  // we assume the yololayer outputs no more than MAX_OUTPUT_BBOX_COUNT boxes that conf >= 0.1
	const char* INPUT_BLOB_NAME = "data";
	const char* OUTPUT_BLOB_NAME = "prob";
	
	Logger gLogger;

	std::string engine_name = "test.engine";

	float* data;
	float* prob;
	
	
	void* buffers[2];

	cudaStream_t stream;
	int inputIndex;
	int outputIndex;

private:
	void doInference(nvinfer1::IExecutionContext& context, cudaStream_t& stream, void **buffers, float* input, float* output, int batchSize);
};

#endif // !Detector