/**
 * Author: rodrigo
 * 2016
 */
#include <boost/test/unit_test.hpp>
#include <pcl/io/pcd_io.h>
#include "Extractor.hpp"
#include "CloudFactory.hpp"
#include "DCH.hpp"
#include "FPFH.hpp"

/**************************************************/
// Auxiliary method defined to be used while testing
static bool diffThanZero(const float value_)
{
	return value_ > 0 || value_ < 0;
}
/**************************************************/

/**************************************************/
// Fixture definition
struct DCHFixture
{
	DCHFixture()
	{
		targetPoint = 100;
		normalEstimationRadius = -1;

		params = new DCHParams();
		params->searchRadius = 5;
		params->bandNumber = 5;
		params->bandWidth = 1;
		params->bidirectional = false;
		params->useProjection = false;
		params->binNumber = 5;
		params->stat = Params::STAT_MEAN;

		paramsPtr = DescriptorParamsPtr(params);
	}
	~ DCHFixture() { }

	int targetPoint;
	double normalEstimationRadius;
	DCHParams *params;
	DescriptorParamsPtr paramsPtr;
};
/**************************************************/

/**************************************************/
BOOST_AUTO_TEST_SUITE(DCH_class_suite)

BOOST_FIXTURE_TEST_CASE(calculateDescriptor, DCHFixture)
{
	// Generate cloud
	pcl::PointCloud<pcl::PointNormal>::Ptr cloud = CloudFactory::createHorizontalPlane(-50, 50, 200, 300, 30, 20000);
	pcl::PointNormal point = cloud->at(targetPoint);

	std::vector<BandPtr> descriptor1 = DCH::calculateDescriptor(cloud, paramsPtr, targetPoint);
	std::vector<BandPtr> descriptor2 = DCH::calculateDescriptor(cloud, paramsPtr, point);

	// Check sizes
	BOOST_CHECK_EQUAL(descriptor1.size(), params->bandNumber);
	BOOST_CHECK_EQUAL(descriptor1.size(), descriptor2.size());

	// Check the fields are being filled
	for (int i = 0; i < params->bandNumber; i++)
		BOOST_CHECK(!descriptor1[i]->descriptor.empty());
}

BOOST_FIXTURE_TEST_CASE(fillDescriptor_plane, DCHFixture)
{
	targetPoint = 10081;

	// Generate cloud
	pcl::PointCloud<pcl::PointNormal>::Ptr cloud = CloudFactory::createHorizontalPlane(-50, 50, 200, 300, 30, 20000);

	// Extract bands
	pcl::PointNormal point = cloud->at(targetPoint);
	std::vector<BandPtr> bands = Extractor::getBands(cloud, point, params);
	DCH::fillDescriptor(bands, paramsPtr);

	int bandSize = params->sizePerBand();
	for (int i = 0; i < params->bandNumber; i++)
	{
		BOOST_CHECK_EQUAL(bands[i]->descriptor.size(), bandSize);
		BOOST_CHECK(std::find_if(bands[i]->descriptor.begin(), bands[i]->descriptor.end(), diffThanZero) == bands[i]->descriptor.end());
	}
}

BOOST_FIXTURE_TEST_CASE(fillDescriptor_halfSphere, DCHFixture)
{
	targetPoint = 10577;
	// params->sequenceBin = 0.5;

	// Generate cloud
	pcl::PointCloud<pcl::PointNormal>::Ptr cloud = CloudFactory::createSphereSection(M_PI, 10, Eigen::Vector3f(0, 0, 0), 20000);

	// Extract bands
	pcl::PointNormal point = cloud->at(targetPoint);
	std::vector<BandPtr> bands = Extractor::getBands(cloud, point, params);
	DCH::fillDescriptor(bands, paramsPtr);

	int bandSize = params->sizePerBand();
	for (int i = 0; i < params->bandNumber; i++)
	{
		BOOST_CHECK_EQUAL(bands[i]->descriptor.size(), bandSize);
		BOOST_CHECK(std::find_if(bands[i]->descriptor.begin(), bands[i]->descriptor.end(), diffThanZero) != bands[i]->descriptor.end());
	}
}

BOOST_AUTO_TEST_SUITE_END()
/**************************************************/

/**************************************************/
BOOST_AUTO_TEST_SUITE(FPFH_class_suite)

BOOST_AUTO_TEST_CASE(FPFH_copyCloud)
{
	size_t descriptorSize = sizeof(pcl::FPFHSignature33) / sizeof(float);
	BOOST_CHECK_EQUAL(descriptorSize, 33);

	pcl::PointCloud<pcl::FPFHSignature33>::Ptr cloud(new pcl::PointCloud<pcl::FPFHSignature33>());

	for (int k = 0; k < 5; k++)
	{
		pcl::FPFHSignature33 point;
		size_t j = 0;
		for (size_t i = k * descriptorSize; i < (k + 1) * descriptorSize; i++)
			point.histogram[j++] = 0.0 + i;

		cloud->push_back(point);
	}

	cv::Mat descriptors;
	FPFH::copyCloud(cloud, descriptors);

	for (size_t i = 0; i < cloud->size(); i++)
	{
		for (size_t j = 0; j < descriptorSize; j++)
			BOOST_CHECK_CLOSE(descriptors.at<float>(i, j), i * descriptorSize + j, 1e-5);
	}
}

BOOST_AUTO_TEST_CASE(FPFH_point_cpy)
{
	size_t descriptorSize = sizeof(pcl::FPFHSignature33) / sizeof(float);
	BOOST_CHECK_EQUAL(descriptorSize, 33);

	pcl::PointCloud<pcl::FPFHSignature33>::Ptr cloud(new pcl::PointCloud<pcl::FPFHSignature33>());

	for (int k = 0; k < 5; k++)
	{
		pcl::FPFHSignature33 point;
		size_t j = 0;
		for (size_t i = k * descriptorSize; i < (k + 1) * descriptorSize; i++)
			point.histogram[j++] = 0.0 + i;

		cloud->push_back(point);
	}


	for (size_t i = 0; i < cloud->size(); i++)
	{
		Eigen::VectorXf descriptor_ = Eigen::VectorXf(descriptorSize);
		FPFH_POINT_CPY(descriptor_, cloud->at(i), descriptorSize);

		for (size_t j = 0; j < descriptorSize; j++)
			BOOST_CHECK_CLOSE(descriptor_(j), i * descriptorSize + j, 1e-5);

	}
}

BOOST_AUTO_TEST_SUITE_END()
/**************************************************/

/**************************************************/
BOOST_AUTO_TEST_SUITE(Extractor_class_suite)

BOOST_FIXTURE_TEST_CASE(getBands_no_bidirectional, DCHFixture)
{
	params->bidirectional = false;

	// Generate cloud
	pcl::PointCloud<pcl::PointNormal>::Ptr cloud = CloudFactory::createHorizontalPlane(-50, 50, 200, 300, 30, 3000);
	pcl::PointNormal point = cloud->at(targetPoint);

	// Extract bands
	std::vector<BandPtr> bands = Extractor::getBands(cloud, point, params);

	// Check number of bands is ok
	BOOST_CHECK_EQUAL(bands.size(), params->bandNumber);

	// Check bands are ok
	Eigen::Vector3f pointNormal = point.getNormalVector3fMap();
	for (int i = 0; i < params->bandNumber; i++)
	{
		Eigen::Vector3f normal = bands[i]->plane.normal();
		Eigen::Vector3f nextNormal = bands[(i + 1) % params->bandNumber]->plane.normal();

		// Check the angular difference is less than a 0.5 percent
		BOOST_CHECK_CLOSE(Utils::angle(normal, nextNormal), params->bandsAngleStep(), 0.5);
		// Check the angular difference is less than 0.005 radians (0.28 degrees aprox)
		BOOST_CHECK_SMALL(fabs(Utils::angle(normal, nextNormal) - params->bandsAngleStep()), 5E-3);

		// Check each band's plane is perpendicular (goes along splitting the band in two)
		BOOST_CHECK_SMALL(pointNormal.dot(normal), 1E-10f);
	}
}

BOOST_FIXTURE_TEST_CASE(getBands_bidirectional, DCHFixture)
{
	params->bidirectional = true;

	// Generate cloud
	pcl::PointCloud<pcl::PointNormal>::Ptr cloud = CloudFactory::createHorizontalPlane(-50, 50, 200, 300, 30, 3000);
	pcl::PointNormal point = cloud->at(targetPoint);

	// Extract bands
	std::vector<BandPtr> bands = Extractor::getBands(cloud, point, params);

	// Check number of bands is ok
	BOOST_CHECK_EQUAL(bands.size(), params->bandNumber);

	// Extract bands
	bands = Extractor::getBands(cloud, point, params);

	// Check bands are ok
	Eigen::Vector3f pointNormal = point.getNormalVector3fMap();
	for (int i = 0; i < params->bandNumber; i++)
	{
		Eigen::Vector3f normal = bands[i]->plane.normal();
		Eigen::Vector3f nextNormal = bands[(i + 1) % params->bandNumber]->plane.normal();

		float step = i != params->bandNumber - 1 ? params->bandsAngleStep() : M_PI - params->bandsAngleStep();

		// Check the angular difference is less than a 0.5 percent
		BOOST_CHECK_CLOSE(Utils::angle(normal, nextNormal), step, 0.5);
		// Check the angular difference is less than 0.005 radians (0.28 degrees approx)
		BOOST_CHECK_SMALL(fabs(Utils::angle(normal, nextNormal) - step), 5E-3);

		// Check each band's plane is perpendicular (goes along splitting the band in two)
		BOOST_CHECK_SMALL(pointNormal.dot(normal), 1E-10f);
	}
}

BOOST_AUTO_TEST_SUITE_END()
