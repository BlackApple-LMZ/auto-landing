//
// Created by limz on 2021/4/08.
//

#include "model_base.h"
#include<opencv2/opencv.hpp>

bool uffModelBase::build()
{
    auto builder = SampleUniquePtr<nvinfer1::IBuilder>(nvinfer1::createInferBuilder(gLogger.getTRTLogger()));
    if (!builder)
    {
        return false;
    }
    auto network = SampleUniquePtr<nvinfer1::INetworkDefinition>(builder->createNetwork());
    if (!network)
    {
        return false;
    }
    auto config = SampleUniquePtr<nvinfer1::IBuilderConfig>(builder->createBuilderConfig());
    if (!config)
    {
        return false;
    }
    auto parser = SampleUniquePtr<nvuffparser::IUffParser>(nvuffparser::createUffParser());
    if (!parser)
    {
        return false;
    }
	//这个是在构建network的模型
    constructNetwork(parser, network);
    builder->setMaxBatchSize(mParams_.batchSize);
    config->setMaxWorkspaceSize(16_MiB);
	
    config->setFlag(BuilderFlag::kGPU_FALLBACK);
    if (mParams_.fp16)
    {
        config->setFlag(BuilderFlag::kFP16);
    }
    if (mParams_.int8)
    {
        config->setFlag(BuilderFlag::kINT8);
    }

    samplesCommon::enableDLA(builder.get(), config.get(), mParams_.dlaCore);
	//根据network和config构建engine用来infer
    mEngine_ = std::shared_ptr<nvinfer1::ICudaEngine>(builder->buildEngineWithConfig(*network, *config), samplesCommon::InferDeleter());

    if (!mEngine_)
    {
        return false;
    }

    return true;
}

bool uffModelBase::infer()
{
    // Create RAII buffer manager object
    samplesCommon::BufferManager buffers(mEngine_, mParams_.batchSize);

    auto context = SampleUniquePtr<nvinfer1::IExecutionContext>(mEngine_->createExecutionContext());
    if (!context)
    {
        return false;
    }

    if (!processInput(buffers, mParams_.inputTensorNames))
    {
        return false;
    }
    // Copy data from host input buffers to device input buffers
    buffers.copyInputToDevice();

    const auto t_start = std::chrono::high_resolution_clock::now();
    // Execute the inference work
    if (!context->execute(mParams_.batchSize, buffers.getDeviceBindings().data()))
    {
        return false;
    }
    const auto t_end = std::chrono::high_resolution_clock::now();
    const float ms = std::chrono::duration<float, std::milli>(t_end - t_start).count();

	
    // Copy data from device output buffers to host output buffers
    buffers.copyOutputToHost();

	if (!verifyOutput(buffers, mParams_.outputTensorNames))
	{
		return false;
	}

	gLogInfo << "Average runs is " << ms << " ms." << std::endl;
    return true;
}

bool uffModelBase::teardown()
{
    nvuffparser::shutdownProtobufLibrary();
    return true;
}

void uffModelBase::initParams(const samplesCommon::Args& args)
{
    if (args.dataDirs.empty()) //!< Use default directories if user hasn't provided paths
    {
		mParams_.dataDirs.push_back("E:\\project\\tensorRT_test\\tensorRT_test\\data\\heading");
		mParams_.dataDirs.push_back("E:\\project\\tensorRT_test\\tensorRT_test\\model\\");
    }
    else //!< Use the data directory provided by the user
    {
		mParams_.dataDirs = args.dataDirs;
    }

	mParams_.uffFileName = locateFile("heading.uff", mParams_.dataDirs);
	mParams_.inputTensorNames.push_back("Placeholder");
	mParams_.batchSize = 10;
	mParams_.outputTensorNames.push_back("output/add");
	mParams_.dlaCore = args.useDLACore;
	mParams_.int8 = args.runInInt8;
	mParams_.fp16 = args.runInFp16;

    return ;
}
bool uffModelBase::initParams(string paramFile)
{
	cv::FileStorage fsSettings(paramFile.c_str(), cv::FileStorage::READ);
	if (!fsSettings.isOpened()) {
		cerr << "Failed to open settings file at: " << paramFile << endl;
		return false;
	}

	int dataNum = fsSettings["DataDir.num"];
	for (int i = 0; i < dataNum; i++) {
		mParams_.dataDirs.push_back(fsSettings["DataDir.dir" + to_string(i + 1)]);
	}
	mParams_.uffFileName = fsSettings["Model.modelname"];

	int inputNum = fsSettings["Input.num"];
	for (int i = 0; i < inputNum; i++) {
		mParams_.inputTensorNames.push_back(fsSettings["Input.tensorName" + to_string(i + 1)]);
	}

	int outputNum = fsSettings["Output.num"];
	for (int i = 0; i < outputNum; i++) {
		mParams_.outputTensorNames.push_back(fsSettings["Output.tensorName" + to_string(i + 1)]);
	}

	mParams_.batchSize = fsSettings["Params.batchSize"];
	mParams_.dlaCore = fsSettings["Params.dlaCore"];
	int int8 = fsSettings["Params.int8"];
	mParams_.int8 = int8;
	int fp16 = fsSettings["Params.fp16"];
	mParams_.fp16 = fp16;

	return true;
}

