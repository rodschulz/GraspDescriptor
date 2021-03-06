/**
 * Author: rodrigo
 * 2015
 */
#include "Config.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <math.h>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <yaml-cpp/yaml.h>
#include <plog/Log.h>
#include "MetricFactory.hpp"


Config::Config()
{
	debug = false;
	targetPoint = -1;
	normalEstimationRadius = -1;

	clusteringParams = NULL;
	cloudSmoothingParams = NULL;
	syntheticCloudParams = NULL;
}

bool Config::load(const std::string &filename_)
{
	bool loadOk = true;
	try
	{
		Config *instance = getInstance();

		YAML::Node config = YAML::LoadFile(filename_);
		instance->config = config;

		instance->debug = config["debug"].as<bool>(false);
		instance->targetPoint = config["targetPoint"].as<int>(-1);
		instance->normalEstimationRadius = config["normalEstimationRadius"].as<double>(-1);
		instance->cacheLocation = config["cacheLocation"].as<std::string>("");


		if (config["descriptor"])
		{
			std::string nodeName = config["descriptor"]["type"].as<std::string>();
			instance->descriptorParams = DescriptorParams::create(DescriptorParams::toType(nodeName));
			instance->descriptorParams->load(config["descriptor"][nodeName]);
		}


		if (config["labeler"] && config["labeler"]["descriptor"])
		{
			std::string nodeName = config["labeler"]["descriptor"]["type"].as<std::string>();
			instance->labelingDescriptorParams = DescriptorParams::create(DescriptorParams::toType(nodeName));
			instance->labelingDescriptorParams->load(config["labeler"]["descriptor"][nodeName]);
		}


		if (config["grasper"] && config["grasper"]["descriptor"])
		{
			std::string nodeName = config["grasper"]["descriptor"]["type"].as<std::string>();
			instance->graspingDescriptorParams = DescriptorParams::create(DescriptorParams::toType(nodeName));
			instance->graspingDescriptorParams->load(config["grasper"]["descriptor"][nodeName]);
		}


		if (config["clustering"])
		{
			YAML::Node clusteringConfig = config["clustering"];

			ClusteringParams *params = new ClusteringParams();
			params->implementation = Params::toClusteringImp(clusteringConfig["implementation"].as<std::string>());
			std::vector<std::string> metricDetails = clusteringConfig["metric"].as<std::vector<std::string> >();
			params->metric = MetricFactory::create(Metric::toType(metricDetails[0]), metricDetails);
			params->clusterNumber = clusteringConfig["clusterNumber"].as<int>();
			params->maxIterations = clusteringConfig["maxIterations"].as<int>();
			params->stopThreshold = clusteringConfig["stopThreshold"].as<double>();
			params->attempts = clusteringConfig["attempts"].as<int>();
			params->generateElbowCurve = clusteringConfig["generateElbowCurve"].as<bool>();
			params->generateDistanceMatrix = clusteringConfig["generateDistanceMatrix"].as<bool>();

			instance->clusteringParams = params;
		}


		if (config["cloudSmoothing"])
		{
			YAML::Node smoothingConfig = config["cloudSmoothing"];

			CloudSmoothingParams *params = new CloudSmoothingParams();
			params->useSmoothing = smoothingConfig["useSmoothing"].as<bool>();
			params->radius = smoothingConfig["radius"].as<double>();
			params->sigma = smoothingConfig["sigma"].as<double>();

			instance->cloudSmoothingParams = params;
		}


		if (config["syntheticCloud"])
		{
			YAML::Node synthCloudConfig = config["syntheticCloud"];

			SyntheticCloudsParams *params = new SyntheticCloudsParams();
			params->useSynthetic = synthCloudConfig["generateCloud"].as<bool>();
			params->synCloudType = Params::toSynCloudType(synthCloudConfig["type"].as<std::string>());

			instance->syntheticCloudParams = params;
		}
	}
	catch (std::exception &_ex)
	{
		LOGE << _ex.what() << std::endl;
		loadOk = false;
	}

	return loadOk;
}
