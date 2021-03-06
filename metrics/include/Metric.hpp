/**
 * Author: rodrigo
 * 2015
 */
#pragma once

#include <iostream>
#include <opencv2/core/core.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <plog/Log.h>


enum MetricType
{
	METRIC_EUCLIDEAN,
	METRIC_CLOSEST_PERMUTATION,
	METRIC_CLOSEST_PERMUTATION_WITH_CONFIDENCE
};
static std::string metricType[] =
{
	BOOST_STRINGIZE(METRIC_EUCLIDEAN),
	BOOST_STRINGIZE(METRIC_CLOSEST_PERMUTATION),
	BOOST_STRINGIZE(METRIC_CLOSEST_PERMUTATION_WITH_CONFIDENCE)
};


// Metric class definition
class Metric
{
public:
	/**************************************************/
	virtual double distance(const cv::Mat &vector1_,
							const cv::Mat &vector2_) const = 0;


	/**************************************************/
	virtual cv::Mat calculateMeans(const int clusterNumber_,
								   const cv::Mat &items_,
								   const cv::Mat &labels_,
								   const cv::Mat &currentMeans_ = cv::Mat()) const = 0;


	/**************************************************/
	virtual MetricType getType() const = 0;


	/**************************************************/
	virtual std::vector<std::string> getConstructionParams() const = 0;


	/**************************************************/
	virtual void validateMeans(cv::Mat &means_) const = 0;


	/**************************************************/
	void setDebug(const bool &status_)
	{
		debugEnabled = status_;
	}


	/**************************************************/
	static MetricType toType(const std::string &type_)
	{
		if (boost::iequals(type_, "euclidean"))
			return METRIC_EUCLIDEAN;
		else if (boost::iequals(type_, "closest"))
			return METRIC_CLOSEST_PERMUTATION;
		else if (boost::iequals(type_, "closest_with_confidence"))
			return METRIC_CLOSEST_PERMUTATION_WITH_CONFIDENCE;

		LOGW << "Wrong metric type, assuming EUCLIDEAN";
		return METRIC_EUCLIDEAN;
	}


protected:
	Metric()
	{
		debugEnabled = false;
	}
	virtual ~Metric() {}

	// Debug generation flag
	bool debugEnabled;
};

// Metric's shared pointer
typedef boost::shared_ptr<Metric> MetricPtr;
