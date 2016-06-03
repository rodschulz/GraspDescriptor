/**
 * Author: rodrigo
 * 2015
 */
#pragma once

#include <opencv2/core/core.hpp>
#include <boost/shared_ptr.hpp>

enum MetricType
{
	METRIC_EUCLIDEAN, METRIC_CLOSEST_PERMUTATION, METRIC_CLOSEST_PERMUTATION_WITH_CONFIDENCE
};
static std::string metricType[] = {BOOST_STRINGIZE(METRIC_EUCLIDEAN), BOOST_STRINGIZE(METRIC_CLOSEST_PERMUTATION), BOOST_STRINGIZE(METRIC_CLOSEST_PERMUTATION_WITH_CONFIDENCE)};

// Metric class definition
class Metric
{
public:
	// Returns the distance between the given vectors, according to the current metric
	virtual double distance(const cv::Mat &_vector1, const cv::Mat &_vector2) const = 0;

	// Returns the central point amongst the given items, according to the given labels
	virtual cv::Mat calculateCenters(const int _clusterNumber, const cv::Mat &_items, const cv::Mat &_labels, std::vector<int> &_itemsPerCenter) const = 0;

	// Returns the type of the current metric
	virtual MetricType getType() const = 0;

	// Evaluates the current metric according to the given testcase file
	static void evaluateMetricCases(const std::string &_resultsFilename, const std::string &_casesFilename, const MetricType &_metricType, const std::vector<std::string> &_args);

	// Calculates the central point amongst the given items, according to the given labels
	cv::Mat calculateCenters(const int _clusterNumber, const cv::Mat &_items, const cv::Mat &_labels) const
	{
		std::vector<int> itemCount;
		return calculateCenters(_clusterNumber, _items, _labels, itemCount);
	}

protected:
	// Constructor
	Metric();
	// Destructor
	virtual ~Metric();
};

// Metric's shared pointer
typedef boost::shared_ptr<Metric> MetricPtr;
