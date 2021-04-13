//
// Created by limz on 2021/4/01.
//

#ifndef UFF_MODEL_HEADING_H
#define UFF_MODEL_HEADING_H

#include "argsParser.h"
#include "buffers.h"
#include "common.h"
#include "logger.h"

#include "NvInfer.h"
#include "NvUffParser.h"
#include <cuda_runtime_api.h>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unordered_map>
#include <vector>

#include "model_base.h"

const std::string gSampleName = "TensorRT.heading_predict";
const float PI = 3.1415926;

class uffModel : public uffModelBase
{
    template <typename T>
    using SampleUniquePtr = std::unique_ptr<T, samplesCommon::InferDeleter>;

public:
	uffModel()
	{

	};
private:
    // Parses a Uff model for predict heading and creates a TensorRT network
    void constructNetwork(SampleUniquePtr<nvuffparser::IUffParser>& parser, SampleUniquePtr<nvinfer1::INetworkDefinition>& network);

    // Reads the input and mean data, preprocesses, and stores the result in a managed buffer
	virtual bool processInput(const samplesCommon::BufferManager& buffers, const std::vector<std::string>& inputTensorName) const;

	virtual bool verifyOutput(const samplesCommon::BufferManager& buffers, const std::vector<std::string>& outputTensorName);

	float heading_ = 0.0;
	int index_ = 0;
};

#endif