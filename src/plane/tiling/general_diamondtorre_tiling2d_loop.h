#ifndef WAVE_MODEL_TILING_GENERAL_DIAMONDTORRE_TILING2D_H_
#define WAVE_MODEL_TILING_GENERAL_DIAMONDTORRE_TILING2D_H_

#include "logging/macro.h"

#include <vector>
#include <algorithm>

#include <cstdint>

namespace wave_model {

// TYPES
// [x]______AA|___
// [0] ... AAA|A
// [1]      AA|BB
// [2] ...   B|BBB
// [3]      BB|BB
// [4] ... BBB|B  
// [5]      BB|CC 
// [6] ...   C|CCC
// [7]______DD|CC_
// [x]     DDD|D
// [x]      DD|

template<size_t NR>
struct WmGeneralDiamondTorreTiling2D
{
    struct Test;

    static constexpr size_t NTileRank = NR;
    static constexpr size_t NTileSize = 1u << NTileRank;

    static_assert(NTileRank > 0, "NTileRank must be positive");

    enum EType
    {
        TYPE_A, TYPE_B, TYPE_C, TYPE_D, TYPE_N
    };

    // TODO: to generate code for the each case
    template<size_t NRank, typename TStencil, typename TGeneralLayer>
    static void traverse(const TStencil& stencil, 
                         TGeneralLayer* layers) noexcept
    {
        // guaranteed to be 2's power
        static constexpr size_t NLineCnt = 
            TGeneralLayer::NDomainLengthX / (1u << NTileRank);

        int64_t left_idx = 
            TGeneralLayer::template 
            off_right<TGeneralLayer::NDomainRankX>(0, 1) + 
            TGeneralLayer::template 
            off_bottom<TGeneralLayer::NDomainRankY>(0, 1);

        int64_t right_idx = left_idx + 
            TGeneralLayer::template off_right<NTileRank - 1>(left_idx, 1) +
            TGeneralLayer::template off_top<NTileRank - 1>(left_idx, 1);

        proc_line<NRank, true>(right_idx, 0, TGeneralLayer::NDomainLengthX + 
                NTileSize / 2, stencil, layers);
        proc_line<NRank, false>(left_idx, 0, TGeneralLayer::NDomainLengthX, 
                stencil, layers);
        for (int64_t col = TGeneralLayer::NDomainLengthX - NTileSize; 
             col >= 0; col -= NTileSize)
        {
            right_idx += TGeneralLayer::template 
                off_left<NTileRank>(right_idx, 1);
            left_idx += TGeneralLayer::template 
                off_left<NTileRank>(left_idx, 1);

            proc_line<NRank, true>(right_idx, 0, col + 
                    NTileSize / 2, stencil, layers);
            proc_line<NRank, false>(left_idx, 0, col, 
                    stencil, layers);
        }

        right_idx += TGeneralLayer::template off_left<NTileRank>(right_idx, 1);
        left_idx += TGeneralLayer::template off_left<NTileRank>(left_idx, 1);

        proc_line<NRank, false>(left_idx, 0, 
                NTileSize / 2, stencil, layers);
        proc_line<NRank, true>(right_idx, 0, 
                0, stencil, layers);

        for (size_t cur_time = 0; cur_time < (1u << NRank);)
        {
            cur_time += NTileSize / 2;
            proc_line<NRank, true>(right_idx, cur_time % TStencil::NMod, 
                    NTileSize / 2, stencil, layers);

            cur_time += NTileSize / 2;
            proc_line<NRank, false>(left_idx, cur_time % TStencil::NMod, 
                    0, stencil, layers);
        }
    }

    template<size_t NRank, bool NIsWide, 
             typename TStencil, typename TGeneralLayer>
    static void proc_line(int64_t idx, int64_t layer_idx, int64_t coord, 
            const TStencil& stencil, TGeneralLayer* layers) noexcept
    {
        if constexpr (NType == TYPE_N)
            return;

        WM_ASSERT(idx >= 0, "index must be non-negative");

        int64_t max_row = (NIsWide ? 
                TGeneralLayer::NDomainLengthY / (1u << NTileRank) + 1 : 
                TGeneralLayer::NDomainLengthY / (1u << NTileRank));

        static constexpr size_t NTypeTop = []() -> size_t {
            if constexpr (NIsWide) return TYPE_A;
            else return TYPE_D;
        }();

        static constexpr size_t NTypeBottom = []() -> size_t {
            if constexpr (NIsWide) return TYPE_B;
            else return TYPE_C;
        }();

        proc_tile<NRank, NTypeBottom>(idx, layer_idx, coord, stencil, layers);
        for (int64_t row = 1; row < max_row; ++row)
        {
            idx += TGeneralLayer::template off_top<NTileRank>(idx, 1);
            proc_tile<NRank, TYPE_B>(idx, layer_idx, coord, stencil, layers);
        }

        idx += TGeneralLayer::template off_top<NTileRank>(idx, 1);
        proc_tile<NRank, NTypeTop>(idx, layer_idx, coord, stencil, layers);
    }

    template<size_t NRank, EType NType, 
             typename TStencil, typename TGeneralLayer>
    proc_tile(int64_t idx, int64_t layer_idx, int64_t coord, 
            const TStencil& stencil, TGeneralLayer* layers) noexcept
    {
        for (; coord < TGeneralLayer::NDomainLengthX + NTileSize - 1; ++coord)
        {
            proc_fold<NTileRank, NType>(idx, layer_idx, coord, stencil, layers);
            layer_idx = (layer_idx + 1) % TStencil::NMod;
        }
    }

    template<size_t NCurRank, EType NType, 
             typename TStencil, typename TGeneralLayer>
    proc_fold(int64_t idx, int64_t layer_idx, int64_t coord, 
            const TStencil& stencil, TGeneralLayer* layers) noexcept
    {
        if constexpr (NCurRank == 1)
        {
            if constexpr (NType == TYPE_D || NType == TYPE_N)
                return;

            switch (coord)
            {
                case 0:
                    calc_cell<>
                    break;
                case 1:
                    calc_cell<>
                    calc_cell<>
                    break;
                case TGeneralLayer::NDomainLengthX - 1:
                    calc_cell<>
                    calc_cell<>
                    break;
                case TGeneralLayer::NDomainLengthX:
                    calc_cell<>
                    break;
                default:
                    calc_cell<>
                    calc_cell<>
                    break;
            }
        }
        else
        {
        }
    }

    template<EType NType, size_t NLayerIdx, 
             typename TStencil, typename TGeneralLayer>
    static void calc_cell(int64_t idx, 
            const TStencil& stencil, TGeneralLayer* layers) noexcept
    {
        if constexpr (NType == TYPE_N)
            return;

        static constexpr int NXSide = []() -> int {
            if constexpr (NType == TYPE_A) return -1;
            else if constexpr (NType == TYPE_C) return 1;

            return 0;
        }();

        static constexpr int NYSide = 
            static_cast<int>(NYType) - static_cast<int>(TYPE_C);

        stencil.template apply<NXSide, NYSide, NLayerIdx>(idx, layers);
    }
};

template<size_t NR>
struct WmGeneralDiamondTorreTiling2D<NR>::Test
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
        stream << "BEGIN WmGeneralDiamondTorreTiling2D::Test::test_traverse()\n"; 
        TestStencil<TStream> stencil(stream);
        static TGeneralLayer<typename TestStencil<TStream>::TData, NRank> 
            layers[TestStencil<TStream>::NDepth] = {};

        WmGeneralDiamondTorreTiling2D::traverse<NRank>(stencil, layers);
        stream << "END WmGeneralDiamondTorreTiling2D::Test::test_traverse()\n";

        return stream;
    }
};

} // namespace wave_model

#endif // WAVE_MODEL_TILING_GENERAL_DIAMONDTORRE_TILING2D_H_
