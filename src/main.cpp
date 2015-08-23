/**
 * Author: rodrigo
 * 2015
 */
#include <stdlib.h>
#include <iostream>
#include <pcl/io/pcd_io.h>
#include <string>
#include "Config.h"
#include "Helper.h"
#include "Extractor.h"
#include "Hist.h"
#include "Writer.h"
#include "Calculator.h"
#include "ClosestPermutation.h"
#include "KMeans.h"

#define CONFIG_LOCATION "./config/config"

void writeOuput(const pcl::PointCloud<pcl::PointNormal>::Ptr &_cloud, const std::vector<BandPtr> &_bands, const std::vector<Hist> &_angleHistograms, const ExecutionParams &_params)
{
	pcl::PointCloud<pcl::PointXYZRGBNormal>::Ptr coloredCloud = Helper::createColorCloud(_cloud, Helper::getColor(0));
	pcl::io::savePCDFileASCII("./output/cloud.pcd", *coloredCloud);

	(*coloredCloud)[_params.targetPoint].rgb = Helper::getColor(255, 0, 0);
	pcl::io::savePCDFileASCII("./output/pointPosition.pcd", *coloredCloud);

	std::vector<pcl::PointCloud<pcl::PointNormal>::Ptr> planes = Extractor::getBandPlanes(_bands, _params);

	pcl::PointCloud<pcl::PointNormal>::Ptr patch = Extractor::getNeighbors(_cloud, _cloud->at(_params.targetPoint), _params.patchSize);
	pcl::io::savePCDFileASCII("./output/patch.pcd", *patch);

	std::ofstream sequences;
	sequences.open("./output/sequences", std::fstream::out);

	for (size_t i = 0; i < _bands.size(); i++)
	{
		if (!_bands[i]->data->empty())
		{
			char name[100];
			sprintf(name, "./output/band%d.pcd", (int) i);
			pcl::io::savePCDFileASCII(name, *Helper::createColorCloud(_bands[i]->data, Helper::getColor(i + 1)));

			sequences << "band " << i << ": " << _bands[i]->sequenceString << "\n";

			sprintf(name, "./output/planeBand%d.pcd", (int) i);
			pcl::io::savePCDFileASCII(name, *planes[i]);
		}
	}

	sequences.close();

	// Write histogram data
	double limit = M_PI;
	Writer::writeHistogram("angles", "Angle Distribution", _angleHistograms, DEG2RAD(20), -limit, limit);
}

int main(int _argn, char **_argv)
{
	try
	{
		if (system("rm -rf ./output/*") != 0)
			std::cout << "WARNING: can't clean output folder\n";

		std::cout << "Loading configuration file\n";
		if (!Config::load(CONFIG_LOCATION, _argn, _argv))
			throw std::runtime_error("Can't read configuration file at " CONFIG_LOCATION);

		ExecutionParams params = Config::getExecutionParams();
		pcl::PointCloud<pcl::PointNormal>::Ptr cloud(new pcl::PointCloud<pcl::PointNormal>());

		// Load cloud
		if (!Helper::loadCloud(cloud, params))
			throw std::runtime_error("Can't load cloud");
		std::cout << "Loaded " << cloud->size() << " points in cloud\n";

		if (params.normalExecution)
		{
			std::cout << "Target point: " << params.targetPoint << "\n";

			pcl::PointNormal target = cloud->points[params.targetPoint];
			std::vector<BandPtr> bands = Calculator::calculateDescriptor(cloud, target, params);

			// Calculate histograms
			std::cout << "Generating angle histograms\n";
			std::vector<Hist> histograms;
			Calculator::calculateAngleHistograms(bands, histograms, params.useProjection);

			// Write output
			std::cout << "Writing output\n";
			writeOuput(cloud, bands, histograms, params);
		}
		else
		{
			std::cout << "Clustering\n";

			size_t cloudSize = cloud->size();
			size_t sequenceSize = Calculator::calculateSequenceLength(params);
			cv::Mat descriptors = cv::Mat::zeros(cloudSize, sequenceSize * params.bandNumber, CV_32FC1);

			if (!Helper::loadClusteringCache(descriptors, params))
			{
				std::cout << "Cache not found, calculating descriptors\n";

				// Extract the descriptors
				for (size_t i = 0; i < cloudSize; i++)
				{
					pcl::PointNormal target = cloud->points[i];
					std::vector<BandPtr> bands = Calculator::calculateDescriptor(cloud, target, params);

					for (size_t j = 0; j < bands.size(); j++)
						memcpy(&descriptors.at<float>(i, j * sequenceSize), &bands[j]->sequenceVector[0], sizeof(float) * sequenceSize);
				}

				// Save cache to file
				Helper::writeClusteringCache(descriptors, params);
			}

			// Create an 'elbow criterion' graph
			if (params.showElbow)
				Helper::generateElbowGraph(descriptors, params);

			// Make clusters of data
			cv::Mat labels, centers;
			int attempts = 3;
			switch (params.implementation)
			{
				case CLUSTERING_OPENCV:
				{
					cv::kmeans(descriptors, params.clusters, labels, cv::TermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, params.maxIterations, params.stopThreshold), attempts, cv::KMEANS_PP_CENTERS, centers);
					break;
				}
				case CLUSTERING_CUSTOM:
				{
					ClosestPermutation metric(params.bandNumber);
					KMeans::findClusters(descriptors, params.clusters, metric, attempts, params.maxIterations, params.stopThreshold, labels, centers);
					break;
				}
			}

			// Color the data according to the clusters
			pcl::PointCloud<pcl::PointXYZRGBNormal>::Ptr colored = Helper::createColorCloud(cloud, Helper::getColor(0));
			for (int i = 0; i < labels.rows; i++)
				(*colored)[i].rgb = Helper::getColor(labels.at<int>(i) + 1);
			pcl::io::savePCDFileASCII("./output/clusters.pcd", *colored);
		}
	}
	catch (std::exception &e)
	{
		std::cout << "ERROR: " << e.what() << std::endl;
	}

	std::cout << "Finished\n";
	return EXIT_SUCCESS;
}
