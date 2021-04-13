//
// Created by limz on 2021/4/01.
//

#include "uff_model_heading.h"
#include <opencv2/opencv.hpp>

void uffModel::constructNetwork(SampleUniquePtr<nvuffparser::IUffParser>& parser, SampleUniquePtr<nvinfer1::INetworkDefinition>& network)
{
    // two input(image and dropout param) and one output tensor
    assert(mParams_.inputTensorNames.size() == 1);
    assert(mParams_.outputTensorNames.size() == 1);

    // Register tensorflow input
    parser->registerInput(mParams_.inputTensorNames[0].c_str(), nvinfer1::Dims3(3, 66, 200), nvuffparser::UffInputOrder::kNCHW);

    parser->registerOutput(mParams_.outputTensorNames[0].c_str());

    parser->parse(mParams_.uffFileName.c_str(), *network, nvinfer1::DataType::kFLOAT);

    if (mParams_.int8)
    {
        samplesCommon::setAllTensorScales(network.get(), 127.0f, 127.0f);
    }
	mInputDims_ = network->getInput(0)->getDimensions();
}

bool uffModel::processInput(const samplesCommon::BufferManager& buffers, const std::vector<std::string>& inputTensorName) const
{
	//fill image input
	const int inputC = mInputDims_.d[0];
    const int inputH = mInputDims_.d[1];
    const int inputW = mInputDims_.d[2];

	// Fill data buffer
	float* hostDataBuffer = static_cast<float*>(buffers.getHostBuffer(inputTensorName[0]));
	//float pixelMean[3]{ 175.365f, 175.257f, 163.012f }; // In BGR order 
	float pixelMean[3]{ 0.0f, 0.0f, 0.0f }; // In BGR order 

	cv::Mat image = cv::imread("E:\\project\\auto landing\\auto landing\\auto landing\\image\\image.png");
	resize(image(cv::Rect(0, 240, image.cols, image.rows - 240)), image, cv::Size(inputW, inputH));
	//cv::imwrite("E:\\project\\tensorRT_test\\tensorRT_test\\data\\heading\\image.png", image);

	int channels = image.channels();
	int index = 0;
	int length = image.rows*image.cols;
	for (int i = 0; i < image.rows; i++) {
		uchar *pSrcData = image.ptr<uchar>(i);
		for (int j = 0, k = 1; j < image.cols; j++, index++, k += channels) {
			hostDataBuffer[0*length + index] = (float(pSrcData[k - 1]) - pixelMean[0]) / 255.0;
			hostDataBuffer[1*length + index] = (float(pSrcData[k]) - pixelMean[1]) / 255.0;
			hostDataBuffer[2*length + index] = (float(pSrcData[k + 1]) - pixelMean[2]) / 255.0;
		}
	}

    return true;
}

bool uffModel::verifyOutput(
	const samplesCommon::BufferManager& buffers, const std::vector<std::string>& outputTensorName)
{
	const float* pheading = static_cast<const float*>(buffers.getHostBuffer(mParams_.outputTensorNames[0]));
	heading_ = pheading[0];

	std::cout << "Predict heading: " << index_ << " " << heading_ * 180 / PI << std::endl;
	index_++;
	return true;
}
