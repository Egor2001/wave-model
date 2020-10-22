#ifndef WAVE_MODEL_LAYER_AVX_LAYER2D_H_
#define WAVE_MODEL_LAYER_AVX_LAYER2D_H_

#include "logging/macro.h"

#include "general_linear_layer2d.h"

namespace wave_model {

template<class TD, size_t ND, template<class, size_t, size_t> typename TL>
using WmAvxLayer2D = TL<TD, ND, ND * 4>;

template<class TD, size_t ND>
using WmAvxLinearLayer2D = WmAvxLayer2D<TD, ND, WmGeneralLinearLayer2D>;

} // namespace wave_model

#endif // WAVE_MODEL_LAYER_AVX_LAYER2D_H_
