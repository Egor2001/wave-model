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

    enum ELine
    {
        LINE_R, LINE_L
    };

    enum EType
    {
        TYPE_A, TYPE_B, TYPE_C, TYPE_D, TYPE_N
    };

    // TODO: to generate code for the each case
    template<size_t NRank, typename TStencil, typename TGeneralLayer>
    static void traverse(const TStencil& stencil, TGeneralLayer* layers) 
                         noexcept
    {
        int64_t left_idx = 
            TGeneralLayer::template 
            off_right<TGeneralLayer::NDomainRankX>(0, 1) + 
            TGeneralLayer::template 
            off_bottom<TGeneralLayer::NDomainRankY>(0, 1);

        int64_t right_idx = left_idx + 
            TGeneralLayer::template off_right<NTileRank - 1>(left_idx, 1) +
            TGeneralLayer::template off_top<NTileRank - 1>(left_idx, 1);

        int64_t col = TGeneralLayer::NDomainLengthX;
        proc_line<NRank, 1 + NTileSize / 2, LINE_R>
            (col + NTileSize / 2, right_idx, 0, stencil, layers);
        proc_line<NRank, 1, LINE_L>
            (col, left_idx, 0, stencil, layers);

        for (col = col - NTileSize; col >= NTileSize; col -= NTileSize)
        {
            right_idx += TGeneralLayer::template 
                off_left<NTileRank>(right_idx, 1);
            left_idx += TGeneralLayer::template 
                off_left<NTileRank>(left_idx, 1);

            proc_line<NRank, 0, LINE_R>
                (col + NTileSize / 2, right_idx, 0, stencil, layers);
            proc_line<NRank, 0, LINE_L>
                (col, left_idx, 0, stencil, layers);
        }

        right_idx += TGeneralLayer::template 
            off_left<NTileRank>(right_idx, 1);
        left_idx += TGeneralLayer::template 
            off_left<NTileRank>(left_idx, 1);

        proc_line<NRank, 1 - NTileSize / 2, LINE_R>
            (col + NTileSize / 2, right_idx, 0, stencil, layers);
        proc_line<NRank, 1 - NTileSize, LINE_L>
            (col, left_idx, 0, stencil, layers);

        for (size_t cur_time = 0; cur_time < (1u << NRank);)
        {
            cur_time += NTileSize / 2;
            proc_line<NRank, 1 - NTileSize, LINE_R>
                (0, left_idx, cur_time, stencil, layers);

            cur_time += NTileSize / 2;
            proc_line<NRank, 1 - NTileSize, LINE_L>
                (0, left_idx, cur_time, stencil, layers);
        }
    }

    template<size_t NRank, int NOffset, ELine NType, 
             typename TStencil, typename TGeneralLayer>
    static void proc_line(int64_t coord, int64_t idx, int64_t layer_idx, 
                          const TStencil& stencil, TGeneralLayer* layers) 
                          noexcept
    {
        WM_ASSERT(idx >= 0, "index must be non-negative");

        if constexpr (NType == LINE_R)
        {
            proc_pole<NRank, NOffset, TYPE_C>
                (coord, idx, layer_idx, stencil, layers);
        }
        else if constexpr (NType == LINE_L)
        {
            proc_pole<NRank, NOffset, TYPE_D>
                (coord, idx, layer_idx, stencil, layers);
        }

        for (int64_t row = TGeneralLayer::NDomainLengthY - NTileSize; 
             row > 0; row -= NTileSize)
        {
            idx += TGeneralLayer::template 
                off_top<NTileRank>(right_idx, 1);

            proc_pole<NRank, NOffset, TYPE_B>
                (coord, idx, layer_idx, stencil, layers);
        }

        if constexpr (NType == LINE_L)
        {
            idx += TGeneralLayer::template 
                off_top<NTileRank>(right_idx, 1);

            proc_pole<NRank, NOffset, TYPE_A>
                (coord, idx, layer_idx, stencil, layers);
        }
    }

    template<size_t NRank, int NOffset, EType NType, 
             typename TStencil, typename TGeneralLayer>
    proc_pole(int64_t coord, int64_t idx, int64_t layer_idx, 
              const TStencil& stencil, TGeneralLayer* layers) noexcept
    {
        proc_fold<NTileRank, NOffset, NType>
            (idx, layer_idx, stencil, layers);

        layer_idx += NTileSize;

        int64_t max_coord = 
            std::min(TGeneralLayer::NDomainLengthX, (1 << NRank) - layer_idx);

        coord += NTileSize;
        for (; coord < max_coord; coord += NTileSize)
        {
            idx += TGeneralLayer::template 
                off_right<NTileRank>(right_idx, 1);

            proc_fold<NTileRank, 0, NType>
                (idx, layer_idx, stencil, layers);

            layer_idx += NTileSize;
        }

        if (coord >= TGeneralLayer::NDomainLengthX)
        {
            idx += TGeneralLayer::template 
                off_right<NTileRank>(right_idx, 1);

            proc_fold<NTileRank, NTileSize + NOffset, NType>
                (idx, layer_idx, stencil, layers);
        }
    }

    template<size_t NRank, int NOffset, EType NType, 
             typename TStencil, typename TGeneralLayer>
    proc_fold(int64_t idx, int64_t layer_idx, 
              const TStencil& stencil, TGeneralLayer* layers) noexcept
    {
        if constexpr (NRank == 1)
        {
            if constexpr (NType == TYPE_D || NType == TYPE_N)
                return;

            constexpr auto FCaller = [](size_t rest) {
                if (layer_idx % TStencil::NMod == rest)
                    calc_cell<>();
            };

            if constexpr (NOffset ... && NType ...)
                calc_cell<NType>
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
