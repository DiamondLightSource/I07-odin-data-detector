/*
 * ThresholdBinPlugin.cpp
 *
 *  Created on: 16 Mar 2026
 *      Author: Famous Alele
 */
#include "ThresholdBinPlugin.h"
#include "DebugLevelLogger.h"
#include "version.h"

namespace FrameProcessor {

const std::string ThresholdBinPlugin::CONFIG_THRESHOLDBIN_PARAM { "thresholdbin" };

ThresholdBinPlugin::ThresholdBinPlugin()
{
    // Setup logging for the class
    logger_ = Logger::getLogger("FP.ThresholdBinPlugin");
    LOG4CXX_TRACE(logger_, "ThresholdBinPlugin constructor.");
    this->threshold_vector_.reserve(64);
}

ThresholdBinPlugin::~ThresholdBinPlugin()
{
    LOG4CXX_TRACE(logger_, "ThresholdBinPlugin destructor.");
}

template <class PixelType> void ThresholdBinPlugin::calculate_sum(boost::shared_ptr<Frame>& frame)
{
    const PixelType* data = static_cast<const PixelType*>(frame->get_image_ptr());
    const size_t elements_count = frame->get_image_size() / sizeof(data[0]);
    const size_t sz = this->threshold_vector_.size();
    for (size_t pixel_index = 0; pixel_index < elements_count; ++pixel_index) {
        PixelType counts = data[pixel_index];
        uint64_t prev_thresh = this->threshold_vector_[0];
        if (sz > 0 && counts > prev_thresh) {
            size_t i = 1;
            for (; i < sz; ++i) {
                if (counts <= this->threshold_vector_[i]) {
                    // Pixel does not fit in this bin - increment previous one
                    uint64_t temp
                        = ++this->histogram_[this->name_threshold_bimap_.right.find(this->threshold_vector_[i - 1])
                                                 ->second];
                    break;
                }
                prev_thresh = threshold_vector_[i];
                // Continue iterating because Pixel fits in this bin or a higher one
            }
            i == sz
                && ++this->histogram_[this->name_threshold_bimap_.right.find(this->threshold_vector_[sz - 1])
                                          ->second]; // No higher bins to check - increment the last bin in the vector
        }
    }
}

/**
 * Calculate the sum of each pixel based on the data type
 *
 * \param[in] frame - Pointer to a Frame object.
 */
void ThresholdBinPlugin::process_frame(boost::shared_ptr<Frame> frame)
{
    boost::lock_guard<boost::recursive_mutex> lock(mutex_);
    LOG4CXX_TRACE(logger_, "Received a new frame...");
    switch (frame->get_meta_data().get_data_type()) {
    case raw_8bit: {
        this->calculate_sum<uint8_t>(frame);
        this->add_data_to_frame(frame);
    } break;
    case raw_16bit: {
        this->calculate_sum<uint16_t>(frame);
        this->add_data_to_frame(frame);
    } break;
    case raw_32bit: {
        this->calculate_sum<uint32_t>(frame);
        this->add_data_to_frame(frame);
    } break;
    case raw_64bit: {
        this->calculate_sum<uint64_t>(frame);
        this->add_data_to_frame(frame);
    } break;
    default:
        LOG4CXX_ERROR(
            logger_,
            "ThresholdBinPlugin doesn't support data type:"
                << get_type_from_enum(static_cast<DataType>(frame->get_meta_data().get_data_type()))
        );
    }
    this->push(frame);
}

/**
 * Set configuration options for this Plugin.
 *
 * \param[in] config - IpcMessage containing configuration data.
 * \param[out] reply - Response IpcMessage.
 */
void ThresholdBinPlugin::configure(OdinData::IpcMessage& config, OdinData::IpcMessage& reply)
{
    try {
        if (config.has_param(CONFIG_THRESHOLDBIN_PARAM)) {
            OdinData::IpcMessage histogram(config.get_param<const rapidjson::Value&>(CONFIG_THRESHOLDBIN_PARAM));
            std::vector<std::string> histogram_bins = histogram.get_param_names();
            // Update histogram bins
            boost::lock_guard<boost::recursive_mutex> lock(mutex_);
            for (auto& bin_name : histogram.get_param_names()) {
                uint64_t bin_threshold = histogram.get_param<uint64_t>(bin_name);
                LOG4CXX_INFO(logger_, "Threshold " << bin_name << " set to " << bin_threshold);
                // Check for clashes
                if (this->name_threshold_bimap_.right.count(bin_threshold))
                    throw std::runtime_error("A ThresholdBin bin with given bin_threshold already exists");
                // Add to underlying containers
                this->histogram_.emplace(bin_name, 0);
                this->name_threshold_bimap_.insert({ std::move(bin_name), bin_threshold });

                // - Add threshold to the vector
                this->threshold_vector_.push_back(bin_threshold);
            }
            // sort the threshold vector
            std::sort(this->threshold_vector_.begin(), this->threshold_vector_.end());
        }
    } catch (std::runtime_error& e) {
        std::stringstream ss;
        ss << "Bad ctrl msg: " << e.what();
        this->set_error(ss.str());
        throw;
    }
}

/**
 * Get the configuration values for this plugin
 *
 * \param[out] reply - Response IpcMessage.
 */
void ThresholdBinPlugin::requestConfiguration(OdinData::IpcMessage& reply)
{
    auto&& prefix = get_name() + '/' + CONFIG_THRESHOLDBIN_PARAM + '/';
    for (auto& bin : name_threshold_bimap_.left)
        reply.set_param(prefix + bin.first, bin.second);
}

int ThresholdBinPlugin::get_version_minor()
{
    return ODIN_DATA_VERSION_MINOR;
}

int ThresholdBinPlugin::get_version_major()
{
    return ODIN_DATA_VERSION_MAJOR;
}

int ThresholdBinPlugin::get_version_patch()
{
    return ODIN_DATA_VERSION_PATCH;
}

std::string ThresholdBinPlugin::get_version_short()
{
    return ODIN_DATA_VERSION_STR_SHORT;
}

std::string ThresholdBinPlugin::get_version_long()
{
    return ODIN_DATA_VERSION_STR;
}

}
