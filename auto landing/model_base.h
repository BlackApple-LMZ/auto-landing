#pragma once

#include<iostream>
#include "argsParser.h"
#include "buffers.h"
#include "common.h"
#include "logger.h"

#include "NvInfer.h"
#include "NvUffParser.h"
#include <cuda_runtime_api.h>

using namespace std;

class uffModelBase {
	template <typename T>
	using SampleUniquePtr = std::unique_ptr<T, samplesCommon::InferDeleter>;
public:
	uffModelBase()
	{
		;
	}
	// Initializes members of the params struct using the command line args or yaml files
	virtual void initParams(const samplesCommon::Args& args);
	virtual bool initParams(string paramFile);

	// Builds the network engine
	virtual bool build();

	// Runs the TensorRT inference engine for this sample
	virtual bool infer();

	// Used to clean up any state created in the class
	virtual bool teardown();
private:
	// Parses a Uff model and creates a TensorRT network
	virtual void constructNetwork(SampleUniquePtr<nvuffparser::IUffParser>& parser, SampleUniquePtr<nvinfer1::INetworkDefinition>& network) = 0;

	// Reads the input and mean data, preprocesses, and stores the result in a managed buffer
	virtual bool processInput(const samplesCommon::BufferManager& buffers, const std::vector<std::string>& inputTensorName) const = 0;

	// Verifies that the output is correct and prints it
	virtual bool verifyOutput(const samplesCommon::BufferManager& buffers, const std::vector<std::string>& outputTensorName) = 0;
protected:
	//The TensorRT engine used to run the network
	std::shared_ptr<nvinfer1::ICudaEngine> mEngine_{ nullptr };

	samplesCommon::UffSampleParams mParams_;

	nvinfer1::Dims mInputDims_;
};

