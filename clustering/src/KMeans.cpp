/**
 * Author: rodrigo
 * 2015
 */
#include "KMeans.hpp"
#include <limits>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "Utils.hpp"
#include "Config.hpp"
#include "ClusteringUtils.hpp"


/********** DEBUG DATA GENERATION METHODS **********/
#define DEBUG_DATA_EXT			".dat"
#define DEBUG_PLOT_SCRIPT		"plot.script"
#define DEBUG_PLOT_DATA			"plotItems"
#define DEBUG_PLOT_CENTERS		"plotCenters" DEBUG_DATA_EXT
#define DEBUG_PLOT_CENTERS_IMG	"plotCentersImg" DEBUG_DATA_EXT


void DEBUG_getLimits(const cv::Mat &items_,
					 std::pair<std::pair<float, float>,
					 std::pair<float, float> > &limits_,
					 std::pair<float, float> &center_)
{
	float minX, minY, maxX, maxY, meanX, meanY;
	minX = minY = std::numeric_limits<float>::max();
	maxX = maxY = -std::numeric_limits<float>::max();
	meanX = meanY = 0;

	for (int i = 0; i < items_.rows; i++)
	{
		float x = items_.at<float>(i, 0);
		float y = items_.at<float>(i, 1);

		meanX += x;
		meanY += y;

		if (minX > x)
			minX = x;
		if (maxX < x)
			maxX = x;

		if (minY > y)
			minY = y;
		if (maxY < y)
			maxY = y;
	}

	limits_ = std::pair<std::pair<float, float>, std::pair<float, float> >(std::pair<float, float>(minX, maxX), std::pair<float, float>(minY, maxY));
	center_ = std::pair<float, float>(meanX / items_.rows, meanY / items_.rows);
}

void DEBUG_generateImage(const std::string &title_,
						 const cv::Mat &items_,
						 const cv::Mat &centers_,
						 const cv::Mat &labels_,
						 const std::pair<std::pair<float, float>, std::pair<float, float> > &limits_,
						 const std::pair<float, float> &center_,
						 const int attempt_)
{
	static int img = 0;
	static int lastAttempt = 0;

	bool centerTitles = Config::get()["kmeans"]["centerTitles"].as<bool>(false);
	bool dataTitles = Config::get()["kmeans"]["dataTitles"].as<bool>(false);
	bool identityTitle = Config::get()["kmeans"]["identityTitle"].as<bool>(true);
	bool useCentroid = boost::iequals(Config::get()["kmeans"]["display"].as<std::string>(), "centroid");
	float displayFactor = Config::get()["kmeans"]["displayFactor"].as<float>(1);

	if (lastAttempt != attempt_)
		img = 0;

	// Write centers to file
	std::fstream centersFile(DEBUG_DIR DEBUG_PLOT_CENTERS, std::ofstream::out);
	std::fstream centersImgFile(DEBUG_DIR DEBUG_PLOT_CENTERS_IMG, std::ofstream::out);
	for (int i = 0; i < centers_.rows; i++)
	{
		centersFile << std::fixed << std::setprecision(6) << centers_.at<float>(i, 0) << " " << centers_.at<float>(i, 1) << "\t";
		centersImgFile << std::fixed << std::setprecision(6) << centers_.at<float>(i, 1) << " " << centers_.at<float>(i, 0) << "\t";
	}
	centersFile << std::endl;
	centersImgFile << std::endl;
	centersFile.close();
	centersImgFile.close();

	// Generate files to write data
	std::vector<int> nPerCenter(centers_.rows);
	std::vector<std::fstream *> files;
	files.reserve(centers_.rows);
	for (int i = 0; i < centers_.rows; i++)
		files.push_back(new std::fstream(((std::string) DEBUG_DIR DEBUG_PLOT_DATA + boost::lexical_cast<std::string>(i) + DEBUG_DATA_EXT).c_str(), std::ofstream::out));

	// Write data to files
	for (int i = 0; i < items_.rows; i++)
	{
		(*files[labels_.at<int>(i)]) << std::fixed << std::setprecision(6) << items_.at<float>(i, 0) << " " << items_.at<float>(i, 1) << "\n";
		nPerCenter[labels_.at<int>(i)]++;
	}

	// Close files
	for (std::vector<std::fstream *>::iterator it = files.begin(); it != files.end(); it++)
		(*it)->close();

	// Prepare limits to plot
	float minX = limits_.first.first;
	float maxX = limits_.first.second;
	float deltaX = maxX - minX > 0 ? maxX - minX : 1;
	float middleX = (useCentroid ? center_.first : (minX + maxX) / 2);
	minX = middleX - deltaX * displayFactor;
	maxX = middleX + deltaX * displayFactor;

	float minY = limits_.second.first;
	float maxY = limits_.second.second;
	float deltaY = maxY - minY > 0 ? maxY - minY : 1;
	float middleY = (useCentroid ? center_.second : (minY + maxY) / 2);
	minY = middleY - deltaY * displayFactor;
	maxY = middleY + deltaY * displayFactor;

	// Generate script
	std::fstream scriptFile(DEBUG_DIR DEBUG_PLOT_SCRIPT, std::ofstream::out);
	scriptFile << "set title '" << title_ << " - img:" << img << " - att:" << attempt_ << "'\n";
	scriptFile << "set xlabel 'x'\n";
	scriptFile << "set ylabel 'y'\n\n";

	scriptFile << "set xrange [" << minX << ":" << maxX << "]\n";
	scriptFile << "set yrange [" << minY << ":" << maxY << "]\n";
	scriptFile << "set grid ytics xtics\n\n";

	scriptFile << "set key outside\n";
	scriptFile << "set terminal pngcairo dashed size 1280,720 enhanced font 'Verdana,9'\n";
	scriptFile << "set output '" DEBUG_DIR << std::setfill('0') << std::setw(2) << attempt_ << "_" << std::setw(4) << img << ".png'\n\n";

	scriptFile << "plot \\\n";
	scriptFile << "x " << (identityTitle ? "title 'Identity'" : "notitle") << " lt 2 lc rgb 'black', \\\n";

	for (int i = 0; i < centers_.rows; i++)
		if (nPerCenter[i] > 0)
			scriptFile << "'" DEBUG_DIR DEBUG_PLOT_DATA << i << DEBUG_DATA_EXT << "' using 1:2 " << (dataTitles ? "title 'cluster " + boost::lexical_cast<std::string>(i) + "'" : "notitle") << " pt 7 ps 1 lc rgb '#" << Utils::num2Hex(Utils::palette35(i)) << "', \\\n";

	for (int i = 0; i < centers_.rows; i++)
		scriptFile << "'" DEBUG_DIR DEBUG_PLOT_CENTERS "' using " << (2 * i) + 1 << ":" << (2 * i) + 2 << " " << (centerTitles ? "title 'center " + boost::lexical_cast<std::string>(i) + "'" : "notitle") << " pt 2 ps 2 lw 2 lc rgb '#" << Utils::num2Hex(Utils::palette35(i)) << "', \\\n";

	for (int i = 0; i < centers_.rows; i++)
		scriptFile << "'" DEBUG_DIR DEBUG_PLOT_CENTERS_IMG "' using " << (2 * i) + 1 << ":" << (2 * i) + 2 << " " << (centerTitles ? "title 'center " + boost::lexical_cast<std::string>(i) + "'" : "notitle") << " pt 1 ps 2 lw 2 lc rgb '#" << Utils::num2Hex(Utils::palette35(i)) << "'" << (i == centers_.rows - 1 ? "" : ", \\") << "\n";

	scriptFile.close();

	std::string cmd = "gnuplot " DEBUG_DIR DEBUG_PLOT_SCRIPT;
	if (system(cmd.c_str()) != 0)
		std::cout << "WARNING, bad return for command: " << cmd << "\n";

	img++;
}
/********** DEBUG DATA GENERATION METHODS **********/


void KMeans::searchClusters(ClusteringResults &results_,
							const cv::Mat &items_,
							const MetricPtr &metric_,
							const int ncluster_,
							const int attempts_,
							const int maxIterations_,
							const double _stopThreshold)
{
	KMeans::run(results_, items_, metric_, ncluster_, attempts_, maxIterations_, _stopThreshold);
}

void KMeans::stochasticSearchClusters(ClusteringResults &results_,
									  const cv::Mat &items_,
									  const MetricPtr &metric_,
									  const int ncluster_,
									  const int attempts_,
									  const int maxIterations_,
									  const double _stopThreshold,
									  const int sampleSize_)
{
	KMeans::run(results_, items_, metric_, ncluster_, attempts_, maxIterations_, _stopThreshold, sampleSize_);
}

void KMeans::run(ClusteringResults &results_,
				 const cv::Mat &items_,
				 const MetricPtr &metric_,
				 const int ncluster_,
				 const int attempts_,
				 const int maxIterations_,
				 const double stopThreshold_,
				 const int sampleSize_)
{
	/***** DEBUG *****/
	bool debugKmeans = Config::get()["kmeans"]["debugAlgorithm"].as<bool>(false);
	bool debug2D = debugKmeans && items_.cols == 2;
	metric_->setDebug(Config::get()["kmeans"]["debugMetric"].as<bool>(false));

	std::pair<std::pair<float, float>, std::pair<float, float> > limits;
	std::pair<float, float> center;
	std::fstream itemCountFile, updateLogFile, pointsAssocFile;

	if (debug2D)
		DEBUG_getLimits(items_, limits, center);
	if (debugKmeans)
	{
		itemCountFile.open(DEBUG_DIR "kmeans_itemsPerCluster", std::ofstream::out);
		updateLogFile.open(DEBUG_DIR "kmeans_updateCentersLog", std::ofstream::out);
		pointsAssocFile.open(DEBUG_DIR "kmeans_pointsAssociation", std::ofstream::out);
	}
	/***** DEBUG *****/

	results_.prepare(ncluster_, items_.rows, items_.cols);
	double minSSE = std::numeric_limits<double>::max();

	for (int i = 0; i < attempts_; i++)
	{
		std::cout << "\t** attempt " << i << std::endl;

		cv::Mat centers = cv::Mat::zeros(ncluster_, items_.cols, CV_32FC1);
		cv::Mat sample = sampleSize_ == -1 ? items_ : cv::Mat::zeros(sampleSize_, items_.cols, CV_32SC1);
		cv::Mat labels = cv::Mat::zeros(sample.rows, 1, CV_32SC1);
		std::vector<double> errorCurve;

		// Select some of the elements as the initial centers
		ClusteringUtils::getSampleItems(items_, centers);
		metric_->validateMeans(centers);


		/***** DEBUG *****/
		if (debugKmeans)
			updateLogFile << "Att: " << i << " - It: initial_values\n" << centers << "\n" << std::endl;
		/***** DEBUG *****/


		// Iterate until the desired max iterations
		for (int j = 0; j < maxIterations_; j++)
		{
			if (j % 50 == 0)
				std::cout << "\t\tit:" << j << std::endl;

			// Select sample points (if needed)
			if (sampleSize_ != -1)
				ClusteringUtils::getSampleItems(items_, sample);

			// Set labels
			for (int k = 0; k < sample.rows; k++)
				labels.at<int>(k) = findClosestCenter(sample.row(k), centers, metric_);


			/***** DEBUG *****/
			if (debugKmeans)
			{
				pointsAssocFile << "Att: " << i << " - It: " << j << "\n";
				for (int k = 0; k < sample.rows; k++)
					pointsAssocFile << k << "\t" << labels.at<int>(k) << "\t" << metric_->distance(sample.row(k), centers.row(labels.at<int>(k))) << "\n";
				pointsAssocFile << std::endl;

				std::vector<int> count = ClusteringUtils::itemsPerCluster(ncluster_, labels);
				itemCountFile << "Att: " << i << " - It: " << j << "\n";
				for (size_t k = 0; k < count.size(); k++)
					itemCountFile << "\tcluster " << k << ": " << count[k] << " points\n";
				itemCountFile << std::endl;
			}
			if (debug2D)
				DEBUG_generateImage("Data labeling", sample, centers, labels, limits, center, i);
			/***** DEBUG *****/


			// Store SSE evolution
			errorCurve.push_back(ClusteringUtils::getSSE(items_, labels, centers, metric_));

			// Updated centers and check if to stop
			if (updateCenters(centers, sample, labels, stopThreshold_, metric_))
			{
				std::cout << "\tthreshold reached --> [attempt: " << i << " - iteration: " << j << "]" << std::endl;
				break;
			}

			/***** DEBUG *****/
			if (debugKmeans)
				updateLogFile << "Att: " << i << " - It: " << j << " (after_update)\n" << centers << "\n" << std::endl;
			if (debug2D)
				DEBUG_generateImage("Updated centers", sample, centers, labels, limits, center, i);
			/***** DEBUG *****/
		}

		double error = ClusteringUtils::getSSE(items_, labels, centers, metric_);
		errorCurve.push_back(error);
		std::cout << "\tSSE: " << std::fixed << error << std::endl;

		/***** DEBUG *****/
		if (debugKmeans)
		{
			updateLogFile << "Att: " << i << " - It: final_state\n" << centers << "\n\n";
			itemCountFile.close();
			updateLogFile.close();
			pointsAssocFile.close();
		}
		if (debug2D)
			DEBUG_generateImage("Final state", sample, centers, labels, limits, center, i);
		/***** DEBUG *****/

		if (error < minSSE)
		{
			// Update the results
			labels.copyTo(results_.labels);
			centers.copyTo(results_.centers);
			results_.errorEvolution = errorCurve;

			// Update the control variable
			minSSE = error;
		}
	}

	// Print a report of the results
	std::cout << "KMeans finished -- SSE: " << minSSE << "\n";
	std::vector<int> zeroPointClusters;

	std::vector<int> itemCount = ClusteringUtils::itemsPerCluster(ncluster_, results_.labels);
	for (size_t i = 0; i < itemCount.size(); i++)
	{
		if (itemCount[i] == 0)
			zeroPointClusters.push_back(i);
		std::cout << "\tcluster " << i << ": " << itemCount[i] << " points\n";
	}

	// Report zero point clusters
	std::cout << zeroPointClusters.size() << " clusters with no points" << std::endl;
	for (size_t i = 0; i < zeroPointClusters.size(); i++)
		std::cout << "\tcluster " << zeroPointClusters[i] << ": 0 points\n";
}

bool KMeans::updateCenters(cv::Mat &centers_,
						   const cv::Mat &sample_,
						   const cv::Mat labels_,
						   const double stopThreshold_,
						   const MetricPtr &metric_)
{
	// Calculate updated positions for the centers
	cv::Mat newCenters = metric_->calculateMeans(centers_.rows, sample_, labels_, centers_);

	// Check if the "displacement" threshold was reached
	bool stop = ClusteringUtils::evaluateStopCondition(centers_, newCenters, stopThreshold_, metric_);

	// Update centers
	newCenters.copyTo(centers_);

	// Return the stop
	return stop;
}

int KMeans::findClosestCenter(const cv::Mat &vector_,
							  const cv::Mat &centers_,
							  const MetricPtr &metric_)
{
	int closestCenter = -1;

	// Find the closest centroid for the current descriptor
	double minDist = std::numeric_limits<double>::max();
	for (int i = 0; i < centers_.rows; i++)
	{
		double distance = metric_->distance(vector_, centers_.row(i));
		if (distance < minDist)
		{
			minDist = distance;
			closestCenter = i;
		}
	}

	return closestCenter;
}
