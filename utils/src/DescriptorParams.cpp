/**
 * Author: rodrigo
 * 2016
 */
#include "DescriptorParams.hpp"
#include <plog/Log.h>
#include <pcl/pcl_macros.h>


Params::DescriptorType DescriptorParams::toType(const std::string &type_)
{
	if (boost::iequals(type_, "DCH") || boost::iequals(type_, Params::descType[Params::DESCRIPTOR_DCH]))
		return Params::DESCRIPTOR_DCH;
	else if (boost::iequals(type_, "SHOT") || boost::iequals(type_, Params::descType[Params::DESCRIPTOR_SHOT]))
		return Params::DESCRIPTOR_SHOT;
	else if (boost::iequals(type_, "USC") || boost::iequals(type_, Params::descType[Params::DESCRIPTOR_USC]))
		return Params::DESCRIPTOR_USC;
	else if (boost::iequals(type_, "PFH") || boost::iequals(type_, Params::descType[Params::DESCRIPTOR_PFH]))
		return Params::DESCRIPTOR_PFH;
	else if (boost::iequals(type_, "FPFH") || boost::iequals(type_, Params::descType[Params::DESCRIPTOR_FPFH]))
		return Params::DESCRIPTOR_FPFH;
	else if (boost::iequals(type_, "ROPS") || boost::iequals(type_, Params::descType[Params::DESCRIPTOR_ROPS]))
		return Params::DESCRIPTOR_ROPS;
	else if (boost::iequals(type_, "SpinImage") || boost::iequals(type_, Params::descType[Params::DESCRIPTOR_SPIN_IMAGE]))
		return Params::DESCRIPTOR_SPIN_IMAGE;

	LOGW << "Wrong descriptor type (" << type_ << "), assuming DCH";
	return Params::DESCRIPTOR_DCH;
}

DescriptorParamsPtr DescriptorParams::create(const Params::DescriptorType type_)
{
	switch (type_)
	{
	default:
	case Params::DESCRIPTOR_UNKNOWN:
		LOGW << "Bad descriptor type for params instantiation, assuming DCH";

	case Params::DESCRIPTOR_DCH:
		return DescriptorParamsPtr(new DCHParams());

	case Params::DESCRIPTOR_SHOT:
		return DescriptorParamsPtr(new SHOTParams());

	case Params::DESCRIPTOR_USC:
		return DescriptorParamsPtr(new USCParams());

	case Params::DESCRIPTOR_PFH:
		return DescriptorParamsPtr(new PFHParams());

	case Params::DESCRIPTOR_FPFH:
		return DescriptorParamsPtr(new FPFHParams());

	case Params::DESCRIPTOR_ROPS:
		return DescriptorParamsPtr(new ROPSParams());

	case Params::DESCRIPTOR_SPIN_IMAGE:
		return DescriptorParamsPtr(new SpinImageParams());
	}
}


/**************************************************/
/**************************************************/
void DCHParams::load(const YAML::Node &config_)
{
	searchRadius = config_["searchRadius"].as<float>();
	bandNumber = config_["bandNumber"].as<int>();
	bandWidth = config_["bandWidth"].as<float>();
	bidirectional = config_["bidirectional"].as<bool>();
	useProjection = config_["useProjection"].as<bool>();
	binNumber = config_["binNumber"].as<float>();
	stat = Params::toStatType(config_["stat"].as<std::string>());
}

std::string DCHParams::toString() const
{
	std::stringstream stream;
	stream << std::boolalpha
		   << "type:" << Params::descType[type]
		   << " searchRadius:" << searchRadius
		   << " bandNumber:" << bandNumber
		   << " bandWidth:" << bandWidth
		   << " bidirectional:" << bidirectional
		   << " useProjection:" << useProjection
		   << " binNumber:" << binNumber
		   << " stat:" << Params::stat[stat];
	return stream.str();
}

YAML::Node DCHParams::toNode() const
{
	std::string sType = Params::descType[type].substr(11);
	std::string statString = Params::toString(stat);

	YAML::Node node;
	node["type"] = sType;
	node[sType]["searchRadius"] = searchRadius;
	node[sType]["bandNumber"] = bandNumber;
	node[sType]["bandWidth"] = bandWidth;
	node[sType]["bidirectional"] = bidirectional;
	node[sType]["useProjection"] = useProjection;
	node[sType]["binNumber"] = binNumber;
	node[sType]["stat"] = statString;

	return node;
}

int DCHParams::sizePerBand() const
{
	switch (stat)
	{
	default:
	case Params::STAT_MEAN:
	case Params::STAT_MEDIAN:
		return binNumber;

	case Params::STAT_HISTOGRAM_10:
		return ceil(M_PI / DEG2RAD(10));

	case Params::STAT_HISTOGRAM_20:
		return ceil(M_PI / DEG2RAD(20));

	case Params::STAT_HISTOGRAM_30:
		return ceil(M_PI / DEG2RAD(30));

	case Params::STAT_HISTOGRAM_BIN_10:
		return ceil(M_PI / DEG2RAD(10)) * binNumber;

	case Params::STAT_HISTOGRAM_BIN_20:
		return ceil(M_PI / DEG2RAD(20)) * binNumber;

	case Params::STAT_HISTOGRAM_BIN_30:
		return ceil(M_PI / DEG2RAD(30)) * binNumber;
	}
}

float DCHParams::binSize() const
{
	return searchRadius / binNumber;
}

float DCHParams::bandsAngleRange() const
{
	if (bidirectional)
		return M_PI;
	else
		return 2 * M_PI;
}

float DCHParams::bandsAngleStep() const
{
	return bandsAngleRange() / bandNumber;
}


/**************************************************/
/**************************************************/
void SHOTParams::load(const YAML::Node &config_)
{
	searchRadius = config_["searchRadius"].as<float>();
}

std::string SHOTParams::toString() const
{
	std::stringstream stream;
	stream << std::boolalpha
		   << "type:" << Params::descType[type]
		   << " searchRadius:" << searchRadius;
	return stream.str();
}

YAML::Node SHOTParams::toNode() const
{
	std::string sType = Params::descType[type].substr(11);

	YAML::Node node;
	node["type"] = sType;
	node[sType]["searchRadius"] = searchRadius;

	return node;
}


/**************************************************/
/**************************************************/
void USCParams::load(const YAML::Node &config_)
{
	searchRadius = config_["searchRadius"].as<float>();
}

std::string USCParams::toString() const
{
	std::stringstream stream;
	stream << std::boolalpha
		   << "type:" << Params::descType[type]
		   << " searchRadius:" << searchRadius;
	return stream.str();
}

YAML::Node USCParams::toNode() const
{
	std::string sType = Params::descType[type].substr(11);

	YAML::Node node;
	node["type"] = sType;
	node[sType]["searchRadius"] = searchRadius;

	return node;
}


/**************************************************/
/**************************************************/
void PFHParams::load(const YAML::Node &config_)
{
	searchRadius = config_["searchRadius"].as<float>();
}

std::string PFHParams::toString() const
{
	std::stringstream stream;
	stream << std::boolalpha
		   << "type:" << Params::descType[type]
		   << " searchRadius:" << searchRadius;
	return stream.str();
}

YAML::Node PFHParams::toNode() const
{
	std::string sType = Params::descType[type].substr(11);

	YAML::Node node;
	node["type"] = sType;
	node[sType]["searchRadius"] = searchRadius;

	return node;
}


/**************************************************/
/**************************************************/
void FPFHParams::load(const YAML::Node &config_)
{
	searchRadius = config_["searchRadius"].as<float>();
}

std::string FPFHParams::toString() const
{
	std::stringstream stream;
	stream << std::boolalpha
		   << "type:" << Params::descType[type]
		   << " searchRadius:" << searchRadius;
	return stream.str();
}

YAML::Node FPFHParams::toNode() const
{
	std::string sType = Params::descType[type].substr(11);

	YAML::Node node;
	node["type"] = sType;
	node[sType]["searchRadius"] = searchRadius;

	return node;
}


/**************************************************/
/**************************************************/
void ROPSParams::load(const YAML::Node &config_)
{
	searchRadius = config_["searchRadius"].as<float>();
}

std::string ROPSParams::toString() const
{
	std::stringstream stream;
	stream << std::boolalpha
		   << "type:" << Params::descType[type]
		   << " searchRadius:" << searchRadius;
	return stream.str();
}

YAML::Node ROPSParams::toNode() const
{
	std::string sType = Params::descType[type].substr(11);

	YAML::Node node;
	node["type"] = sType;
	node[sType]["searchRadius"] = searchRadius;

	return node;
}


/**************************************************/
/**************************************************/
void SpinImageParams::load(const YAML::Node &config_)
{
	searchRadius = config_["searchRadius"].as<float>();
	imageWidth = config_["imageWidth"].as<int>();
}

std::string SpinImageParams::toString() const
{
	std::stringstream stream;
	stream << std::boolalpha
		   << "type:" << Params::descType[type]
		   << " searchRadius:" << searchRadius
		   << " imageWidth:" << imageWidth;
	return stream.str();
}

YAML::Node SpinImageParams::toNode() const
{
	std::string sType = Params::descType[type].substr(11);

	YAML::Node node;
	node["type"] = sType;
	node[sType]["searchRadius"] = searchRadius;
	node[sType]["imageWidth"] = imageWidth;

	return node;
}