/*
 * ThresholdBinPluginLib.cpp
 *
 *  Created on: 16 Mar 2026
 *      Author: Famous Alele
 */
#include "ClassLoader.h"
#include "ThresholdBinPlugin.h"

namespace FrameProcessor {
/**
 * Registration of this plugin through the ClassLoader. This macro
 * registers the class without needing to worry about name mangling
 */
REGISTER(FrameProcessorPlugin, ThresholdBinPlugin, "ThresholdBinPlugin");

} // namespace FrameProcessor
