//------------------------------------------------------------------------
// Copyright(c) 2023 Baron von Wenzelheimer.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace BaronVonWentz {
//------------------------------------------------------------------------
static const Steinberg::FUID kMorsalyzerProcessorUID (0xF6686E71, 0xFB6C5B4C, 0xBFA6CCDC, 0xD705EAE2);
static const Steinberg::FUID kMorsalyzerControllerUID (0x330B72D5, 0xF7A65306, 0xA703CA66, 0x42D27678);

#define MorsalyzerVST3Category "Fx"

enum GainParams : Steinberg::Vst::ParamID {
	kParamGainId = 102,
	kParamRateId = 103,
	kParamRUnitId = 104,
	kParamEnvelopeId = 105,
	kParamInvertId = 106,
};

//------------------------------------------------------------------------
} // namespace BaronVonWentz
