#ifndef WAVE_MODEL_TILING_GENERAL_REGULAR_TILING2D_H_
#define WAVE_MODEL_TILING_GENERAL_REGULAR_TILING2D_H_

#include "logging/macro.h"

#include <vector>
#include <algorithm>

#include <cstdint>

namespace wave_model {

struct WmGeneralRegularTiling2D
{
    struct Test;

    static constexpr size_t NTileRank = 0;

    enum EType
    {
        TYPE_A, TYPE_B, TYPE_C, TYPE_N
    };

    template<size_t NRank, typename TStencil, typename TGeneralLayer>
    static void traverse(const TStencil& stencil, TGeneralLayer* layers) 
                         noexcept
    {
        call_domain<TStencil::NMod - 1>(0, stencil, layers);
    }

    template<size_t NLayerIdx, typename TStencil, typename TGeneralLayer>
    static void call_domain(size_t layer_idx, 
                            const TStencil& stencil, TGeneralLayer* layers) 
                            noexcept
    {
        if (layer_idx == NLayerIdx)
        {
            int64_t idx = 0;

            proc_line<NLayerIdx, TYPE_A>(idx, stencil, layers);
            idx += TGeneralLayer::template off_bottom<0>(idx, 1);
            for (size_t row = 1; row < TGeneralLayer::NDomainLengthY - 1; ++row)
            {
                proc_line<NLayerIdx, TYPE_B>(idx, stencil, layers);
                idx += TGeneralLayer::template off_bottom<0>(idx, 1);
            }

            proc_line<NLayerIdx, TYPE_C>(idx, stencil, layers);
        }
        else if constexpr (NLayerIdx != 0)
        {
            call_domain<NLayerIdx - 1>(layer_idx, stencil, layers);
        }
    }

    template<size_t NLayerIdx, EType NType, 
             typename TStencil, typename TGeneralLayer>
    static void proc_line(int64_t idx, 
                          const TStencil& stencil, TGeneralLayer* layers) 
                          noexcept
    {
        calc_cell<TYPE_A, NType, NLayerIdx>(idx, stencil, layers);
        idx += TGeneralLayer::template off_right<0>(idx, 1);
        for (size_t col = 1; col < TGeneralLayer::NDomainLengthX - 1; ++col)
        {
            calc_cell<TYPE_B, NType, NLayerIdx>(idx, stencil, layers);
            idx += TGeneralLayer::template off_right<0>(idx, 1);
        }

        calc_cell<TYPE_C, NType, NLayerIdx>(idx, stencil, layers);
    }

    template<EType NXType, EType NYType, size_t NLayerIdx, 
             typename TStencil, typename TGeneralLayer>
    static void calc_cell(int64_t idx, 
                          const TStencil& stencil, TGeneralLayer* layers) 
                          noexcept
    {
        // TODO: to refactor with normal switch
        static constexpr int NXSide = 
            static_cast<int>(NXType) - static_cast<int>(TYPE_B);

        static constexpr int NYSide = 
            static_cast<int>(NYType) - static_cast<int>(TYPE_B);

        idx += TGeneralLayer::template off_right<0>(idx, 1) +
               TGeneralLayer::template off_bottom<0>(idx, 1);

        stencil.template apply<NXSide, NYSide, NLayerIdx>(idx, layers);
    }
};

struct WmGeneralRegularTiling2D::Test
{
    template<typename TStream>
    struct TestStencil
    {
        constexpr static size_t NDepth = 1;
        constexpr static size_t NMod = NDepth;

        struct TData {};

        explicit TestStencil(TStream& stream_ref):
            stream(stream_ref)
        {}

        template<int NXSide, int NYSide, size_t NLayerIdx, 
                 typename TGeneralLayer>
        void apply(size_t idx, [[maybe_unused]] TGeneralLayer* layer) const
        {
            stream << "TestStencil::apply<" << 
                NXSide << ", " << NYSide << ", " << "$TGeneralLayer" << 
                ">(" << idx << ", $layers)\n";
        }

        TStream& stream;
    };

    template<size_t NRank, template<typename, size_t> 
             typename TGeneralLayer, typename TStream>
    static TStream& test_traverse(TStream& stream) noexcept
    {
        stream << "BEGIN WmGeneralRegularTiling2D::Test::test_traverse()\n"; 
        TestStencil<TStream> stencil(stream);
        static TGeneralLayer<typename TestStencil<TStream>::TData, 1u << NRank> 
            layers[TestStencil<TStream>::NDepth] = {};

        WmGeneralRegularTiling2D::traverse<NRank>(stencil, layers);
        stream << "END WmGeneralRegularTiling2D::Test::test_traverse()\n";

        return stream;
    }
};

} // namespace wave_model

#endif // WAVE_MODEL_TILING_GENERAL_REGULAR_TILING2D_H_
