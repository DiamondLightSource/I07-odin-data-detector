/*
 * ThresholdBinPlugin.h
 *
 *  Created on: 11 Mar 2026
 *      Author: Famous Alele
 */

#ifndef ODINDATA_ThresholdBinPLUGIN_H
#define ODINDATA_ThresholdBinPLUGIN_H

#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
using namespace log4cxx;
using namespace log4cxx::helpers;

#include "ClassLoader.h"
#include "FrameProcessorPlugin.h"
#include <boost/bimap.hpp>

namespace FrameProcessor {

/**
 * A plugin representing a ThresholdBin consisting of a bidirectional map, a vector and a histogram map.
 *
 * The keys/values of the maps are `std::string` (the name of a ThresholdBin bin) and `uint64_t` (the number of counts)
 * a pixel must have to be placed in that bin.
 *
 * This plugin enables:
 *   - Lookup of uint64_t by std::string key
 *   - Lookup of std::string by uint64_t key
 *   - Random access of uint64_t values by index (in an ordered vector of uint64_t)
 *
 * It can store the ThresholdBin threshold definitions and increment the correct bin of a corresponding `Sum` struct
 * according to those definitions and a given number of counts in a pixel.
 *
 * \param[in] index - Index of threshold to lookup
 */

class ThresholdBinPlugin : public FrameProcessorPlugin {
public:
    ThresholdBinPlugin();
    ~ThresholdBinPlugin();
    void process_frame(boost::shared_ptr<Frame> frame);
    void configure(OdinData::IpcMessage& config, OdinData::IpcMessage& reply);
    void requestConfiguration(OdinData::IpcMessage& reply);

    int get_version_major();
    int get_version_minor();
    int get_version_patch();
    std::string get_version_short();
    std::string get_version_long();

    // Config Strings
    static const std::string CONFIG_THRESHOLDBIN_PARAM;

private:
    // Plugin specific API
    template <typename PixelType> void calculate_sum(boost::shared_ptr<Frame>& frame);
    /**
     * Add calculated parameters to Frame
     *
     * \param[in] frame - Pointer to a Frame object.
     */
    void add_data_to_frame(boost::shared_ptr<Frame>& frame)
    {
        for (auto& bin : this->histogram_) {
            frame->meta_data().set_parameter<uint64_t>(bin.first, bin.second);
            bin.second = 0; // reset the histogram's counter for each param to 0!
        }
    }
    /** Pointer to logger */
    LoggerPtr logger_;
    /** Mutex used to make this class thread safe */
    boost::recursive_mutex mutex_;
    boost::bimap<std::string, uint64_t> name_threshold_bimap_;
    std::unordered_map<std::string, uint64_t> histogram_;
    std::vector<uint64_t> threshold_vector_;
};

}
#endif // end ODINDATA_ThresholdBinPLUGIN_H
