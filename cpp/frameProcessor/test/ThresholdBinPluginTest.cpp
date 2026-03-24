/*
 * ThresholdBinPluginTest.cpp
 *
 *  Created on: 16 Mar 2026
 *      Author: Famous Alele
 */

#include "ThresholdBinPlugin.h"
#include "DataBlockFrame.h"
#include "FrameProcessorDefinitions.h"
#include "IpcMessage.h"
#include <DebugLevelLogger.h>
#include <boost/test/unit_test.hpp>

class ThresholdBinPluginTestFixture {
public:
    ThresholdBinPluginTestFixture() :
        img { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 },
        dummy_data { 0, 0 },
        frame { boost::make_shared<FrameProcessor::DataBlockFrame>(
            FrameProcessor::FrameMetaData { 1,
                                            "raw",
                                            FrameProcessor::raw_16bit,
                                            "test",
                                            { 3, 4 },
                                            FrameProcessor::no_compression },
            static_cast<void*>(const_cast<unsigned short*>(img)),
            24
        ) },
        empty_frame { boost::make_shared<FrameProcessor::DataBlockFrame>(
            FrameProcessor::FrameMetaData { 1,
                                            "raw",
                                            FrameProcessor::raw_16bit,
                                            "test",
                                            { 2, 0 },
                                            FrameProcessor::no_compression },
            static_cast<void*>(const_cast<unsigned short*>(dummy_data)),
            4
        ) }
    {
        set_debug_level(3);
        thb_plugin.set_name("threshold_plugin");
    }

private:
    const unsigned short img[12];
    uint16_t dummy_data[2];

public:
    boost::shared_ptr<FrameProcessor::Frame> frame;
    boost::shared_ptr<FrameProcessor::Frame> empty_frame;
    FrameProcessor::ThresholdBinPlugin thb_plugin;
};

BOOST_FIXTURE_TEST_SUITE(ThresholdBinPluginUnitTest, ThresholdBinPluginTestFixture);

BOOST_AUTO_TEST_CASE(ThresholdBinPlugin_Frame)
{
    OdinData::IpcMessage reply;
    OdinData::IpcMessage cfg;
    BOOST_REQUIRE_NO_THROW(cfg.set_param(FrameProcessor::ThresholdBinPlugin::CONFIG_THRESHOLDBIN_PARAM + "/low2", 1));
    BOOST_REQUIRE_NO_THROW(cfg.set_param(FrameProcessor::ThresholdBinPlugin::CONFIG_THRESHOLDBIN_PARAM + "/low1", 5));
    BOOST_REQUIRE_NO_THROW(cfg.set_param(FrameProcessor::ThresholdBinPlugin::CONFIG_THRESHOLDBIN_PARAM + "/high1", 8));
    BOOST_REQUIRE_NO_THROW(cfg.set_param(FrameProcessor::ThresholdBinPlugin::CONFIG_THRESHOLDBIN_PARAM + "/high2", 10));
    BOOST_REQUIRE_NO_THROW(thb_plugin.configure(cfg, reply));
    thb_plugin.process_frame(frame);
    BOOST_CHECK_EQUAL(4, frame->get_meta_data().get_parameter<uint64_t>("low2"));
    BOOST_CHECK_EQUAL(3, frame->get_meta_data().get_parameter<uint64_t>("low1"));
    BOOST_CHECK_EQUAL(2, frame->get_meta_data().get_parameter<uint64_t>("high1"));
    BOOST_CHECK_EQUAL(2, frame->get_meta_data().get_parameter<uint64_t>("high2"));
}

BOOST_AUTO_TEST_CASE(ThresholdBinPlugin_FrameEmpty)
{
    // Configure thresholds
    OdinData::IpcMessage reply;
    OdinData::IpcMessage cfg;
    BOOST_REQUIRE_NO_THROW(cfg.set_param(FrameProcessor::ThresholdBinPlugin::CONFIG_THRESHOLDBIN_PARAM + "/low2", 1));
    BOOST_REQUIRE_NO_THROW(cfg.set_param(FrameProcessor::ThresholdBinPlugin::CONFIG_THRESHOLDBIN_PARAM + "/low1", 5));
    BOOST_REQUIRE_NO_THROW(cfg.set_param(FrameProcessor::ThresholdBinPlugin::CONFIG_THRESHOLDBIN_PARAM + "/high1", 8));
    BOOST_REQUIRE_NO_THROW(cfg.set_param(FrameProcessor::ThresholdBinPlugin::CONFIG_THRESHOLDBIN_PARAM + "/high2", 10));
    BOOST_REQUIRE_NO_THROW(thb_plugin.configure(cfg, reply));
    thb_plugin.process_frame(empty_frame);
    BOOST_CHECK_EQUAL(0, empty_frame->get_meta_data().get_parameter<uint64_t>("low2"));
    BOOST_CHECK_EQUAL(0, empty_frame->get_meta_data().get_parameter<uint64_t>("low1"));
    BOOST_CHECK_EQUAL(0, empty_frame->get_meta_data().get_parameter<uint64_t>("high1"));
    BOOST_CHECK_EQUAL(0, empty_frame->get_meta_data().get_parameter<uint64_t>("high2"));
}

BOOST_AUTO_TEST_CASE(ThresholdBinPlugin_UnsupportedDataType)
{
    OdinData::IpcMessage reply;
    OdinData::IpcMessage cfg;
    frame->meta_data().set_data_type(FrameProcessor::raw_float);
    BOOST_REQUIRE_NO_THROW(cfg.set_param(FrameProcessor::ThresholdBinPlugin::CONFIG_THRESHOLDBIN_PARAM + "/high2", 10));
    BOOST_REQUIRE_NO_THROW(thb_plugin.configure(cfg, reply));
    thb_plugin.process_frame(frame);
    BOOST_CHECK(!frame->get_meta_data().has_parameter("high2"));
}

BOOST_AUTO_TEST_SUITE_END(); // ThresholdBinPluginUnitTest
