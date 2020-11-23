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

// OFFSETS
// [...]      AA|                  DD  FF|
// [...] ... AAA|A        CC      DDDDFFF|F
// [...]      AA|BB  ... CCCC ...  DDEEFF|GG
// [...] ...   B|BBB      CC        EEEEG|GGG
// [...]        |BB                  EE  |GG

template<size_t NR>
struct WmGeneralDiamondTorreTiling2D
{
    struct Test;

    static constexpr size_t NTileRank = NR;
    static constexpr int64_t NTileSize = 1u << NTileRank;

    static_assert(NTileRank > 0, "NTileRank must be positive");

    enum ELine : uint8_t
    {
        LINE_R, LINE_L
    };

    enum EType : uint8_t
    {
        TYPE_A, TYPE_B, TYPE_C, TYPE_D, 
        TYPE_N
    };

    enum EOffset : uint8_t
    {
        OFFSET_A, OFFSET_B, OFFSET_C, OFFSET_D, OFFSET_E, OFFSET_F, OFFSET_G, 
        OFFSET_N
    };

    static constexpr EType TopTypeArr[5] = {
        /* [NType == TYPE_A] = */ TYPE_N,
        /* [NType == TYPE_B] = */ TYPE_B,
        /* [NType == TYPE_C] = */ TYPE_B,
        /* [NType == TYPE_D] = */ TYPE_C,
        /* [NType == TYPE_N] = */ TYPE_N
    };

    static constexpr EType MiddleTypeArr[5] = {
        /* [NType == TYPE_A] = */ TYPE_A,
        /* [NType == TYPE_B] = */ TYPE_B,
        /* [NType == TYPE_C] = */ TYPE_B,
        /* [NType == TYPE_D] = */ TYPE_D,
        /* [NType == TYPE_N] = */ TYPE_N
    };

    static constexpr EType BottomTypeArr[5] = {
        /* [NType == TYPE_A] = */ TYPE_B,
        /* [NType == TYPE_B] = */ TYPE_B,
        /* [NType == TYPE_C] = */ TYPE_C,
        /* [NType == TYPE_D] = */ TYPE_N,
        /* [NType == TYPE_N] = */ TYPE_N
    };

    static constexpr EOffset LeftOffsetArr[8] = {
        /* [OFFSET_A] = */ OFFSET_N,
        /* [OFFSET_B] = */ OFFSET_A,
        /* [OFFSET_C] = */ OFFSET_C,
        /* [OFFSET_D] = */ OFFSET_C,
        /* [OFFSET_E] = */ OFFSET_C,
        /* [OFFSET_F] = */ OFFSET_D,
        /* [OFFSET_G] = */ OFFSET_F,
        /* [OFFSET_N] = */ OFFSET_N
    };

    static constexpr EOffset HalfLeftOffsetArr[8] = {
        /* [OFFSET_A] = */ OFFSET_N,
        /* [OFFSET_B] = */ OFFSET_B,
        /* [OFFSET_C] = */ OFFSET_C,
        /* [OFFSET_D] = */ OFFSET_C,
        /* [OFFSET_E] = */ OFFSET_C,
        /* [OFFSET_F] = */ OFFSET_E,
        /* [OFFSET_G] = */ OFFSET_G,
        /* [OFFSET_N] = */ OFFSET_N
    };

    static constexpr EOffset MiddleOffsetArr[8] = {
        /* [OFFSET_A] = */ OFFSET_A,
        /* [OFFSET_B] = */ OFFSET_C,
        /* [OFFSET_C] = */ OFFSET_C,
        /* [OFFSET_D] = */ OFFSET_C,
        /* [OFFSET_E] = */ OFFSET_D,
        /* [OFFSET_F] = */ OFFSET_F,
        /* [OFFSET_G] = */ OFFSET_N,
        /* [OFFSET_N] = */ OFFSET_N
    };

    static constexpr EOffset HalfRightOffsetArr[8] = {
        /* [OFFSET_A] = */ OFFSET_B,
        /* [OFFSET_B] = */ OFFSET_C,
        /* [OFFSET_C] = */ OFFSET_C,
        /* [OFFSET_D] = */ OFFSET_C,
        /* [OFFSET_E] = */ OFFSET_E,
        /* [OFFSET_F] = */ OFFSET_G,
        /* [OFFSET_G] = */ OFFSET_N,
        /* [OFFSET_N] = */ OFFSET_N
    };

    static constexpr EOffset RightOffsetArr[8] = {
        /* [OFFSET_A] = */ OFFSET_C,
        /* [OFFSET_B] = */ OFFSET_C,
        /* [OFFSET_C] = */ OFFSET_C,
        /* [OFFSET_D] = */ OFFSET_D,
        /* [OFFSET_E] = */ OFFSET_F,
        /* [OFFSET_F] = */ OFFSET_N,
        /* [OFFSET_G] = */ OFFSET_N,
        /* [OFFSET_N] = */ OFFSET_N
    };

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
        proc_line<NRank, OFFSET_G, LINE_R>
            (col + NTileSize / 2, right_idx, 0, stencil, layers);
        proc_line<NRank, OFFSET_F, LINE_L>
            (col, left_idx, 0, stencil, layers);

        right_idx += TGeneralLayer::template 
            off_left<NTileRank>(right_idx, 1);
        left_idx += TGeneralLayer::template 
            off_left<NTileRank>(left_idx, 1);

        proc_line<NRank, OFFSET_E, LINE_R>
            (col + NTileSize / 2, right_idx, 0, stencil, layers);
        proc_line<NRank, OFFSET_D, LINE_L>
            (col, left_idx, 0, stencil, layers);

        for (col = col - 2 * NTileSize; col >= NTileSize; col -= NTileSize)
        {
            right_idx += TGeneralLayer::template 
                off_left<NTileRank>(right_idx, 1);
            left_idx += TGeneralLayer::template 
                off_left<NTileRank>(left_idx, 1);

            proc_line<NRank, OFFSET_C, LINE_R>
                (col + NTileSize / 2, right_idx, 0, stencil, layers);
            proc_line<NRank, OFFSET_C, LINE_L>
                (col, left_idx, 0, stencil, layers);
        }

        right_idx += TGeneralLayer::template 
            off_left<NTileRank>(right_idx, 1);
        left_idx += TGeneralLayer::template 
            off_left<NTileRank>(left_idx, 1);

        proc_line<NRank, OFFSET_B, LINE_R>
            (col + NTileSize / 2, right_idx, 0, stencil, layers);
        proc_line<NRank, OFFSET_A, LINE_L>
            (col, left_idx, 0, stencil, layers);

        for (size_t cur_time = 0; cur_time < (1u << NRank);)
        {
            cur_time += NTileSize / 2;
            proc_line<NRank, OFFSET_A, LINE_R>
                (0, left_idx, cur_time, stencil, layers);

            cur_time += NTileSize / 2;
            proc_line<NRank, OFFSET_A, LINE_L>
                (0, left_idx, cur_time, stencil, layers);
        }
    }

    template<size_t NRank, EOffset NOffset, ELine NType, 
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
                off_top<NTileRank>(idx, 1);

            proc_pole<NRank, NOffset, TYPE_B>
                (coord, idx, layer_idx, stencil, layers);
        }

        if constexpr (NType == LINE_L)
        {
            idx += TGeneralLayer::template 
                off_top<NTileRank>(idx, 1);

            proc_pole<NRank, NOffset, TYPE_A>
                (coord, idx, layer_idx, stencil, layers);
        }
    }

    template<size_t NRank, EOffset NOffset, EType NType, 
             typename TStencil, typename TGeneralLayer>
    static void proc_pole(int64_t coord, int64_t idx, int64_t layer_idx, 
                          const TStencil& stencil, TGeneralLayer* layers) 
                          noexcept
    {
        call_fold<TStencil::NMod - 1, NOffset, NType>
            (idx, layer_idx, stencil, layers);

        layer_idx += NTileSize;

        int64_t max_coord = 
            std::min(TGeneralLayer::NDomainLengthX - NTileSize, 
                     (1 << NRank) - layer_idx);

        coord += NTileSize;
        for (; coord < max_coord; coord += NTileSize)
        {
            idx += TGeneralLayer::template off_right<NTileRank>(idx, 1);
            call_fold<TStencil::NMod - 1, OFFSET_C, NType>
                (idx, layer_idx, stencil, layers);
            layer_idx += NTileSize;
        }

        if (coord < TGeneralLayer::NDomainLengthX - NTileSize / 2)
        {
            idx += TGeneralLayer::template off_right<NTileRank>(idx, 1);
            call_fold<TStencil::NMod - 1, OFFSET_D, NType>
                (idx, layer_idx, stencil, layers);
            layer_idx += NTileSize;

            idx += TGeneralLayer::template off_right<NTileRank>(idx, 1);
            call_fold<TStencil::NMod - 1, OFFSET_F, NType>
                (idx, layer_idx, stencil, layers);
            layer_idx += NTileSize;
        }
        else
        {
            idx += TGeneralLayer::template off_right<NTileRank>(idx, 1);
            call_fold<TStencil::NMod - 1, OFFSET_E, NType>
                (idx, layer_idx, stencil, layers);
            layer_idx += NTileSize;

            idx += TGeneralLayer::template off_right<NTileRank>(idx, 1);
            call_fold<TStencil::NMod - 1, OFFSET_G, NType>
                (idx, layer_idx, stencil, layers);
            layer_idx += NTileSize;
        }
    }

    template<size_t NLayerIdx, EOffset NOffset, EType NType, 
             typename TStencil, typename TGeneralLayer>
    static void call_fold(int64_t idx, int64_t layer_idx, 
                          const TStencil& stencil, TGeneralLayer* layers) 
                          noexcept
    {
        if (layer_idx == NLayerIdx)
        {
            proc_fold<NTileRank, NLayerIdx, NOffset, NType>
                (idx, stencil, layers);
        }
        else if constexpr (NLayerIdx != 0)
        {
            call_fold<NLayerIdx - 1, NOffset, NType>
                (idx, layer_idx, stencil, layers);
        }
    }

    template<size_t NRank, int64_t NLayerIdx, EOffset NOffset, EType NType, 
             typename TStencil, typename TGeneralLayer>
    static void proc_fold(int64_t idx, 
                          const TStencil& stencil, TGeneralLayer* layers) 
                          noexcept
    {
        if constexpr (NType == TYPE_N || NOffset == OFFSET_N)
            return;

        static constexpr EType NTopType    = TopTypeArr[NType];
        static constexpr EType NMiddleType = MiddleTypeArr[NType];
        static constexpr EType NBottomType = BottomTypeArr[NType];

        static constexpr EOffset NLeftOffset      = LeftOffsetArr[NOffset];
        static constexpr EOffset NHalfLeftOffset  = HalfLeftOffsetArr[NOffset];
        static constexpr EOffset NMiddleOffset    = MiddleOffsetArr[NOffset];
        static constexpr EOffset NHalfRightOffset = HalfRightOffsetArr[NOffset];
        static constexpr EOffset NRightOffset     = RightOffsetArr[NOffset];

        static constexpr size_t NLess = NRank - 1;
        static constexpr size_t NNextLayerIdx = 
            (NLayerIdx + (1 << NLess)) % TStencil::NMod;

        if constexpr (NRank == 1)
        {
            if constexpr (NType == TYPE_D)
                return;

            int64_t x_sub = TGeneralLayer::template off_left<NLess>(idx, 1);
            int64_t x_add = TGeneralLayer::template off_right<NLess>(idx, 1);

            calc_cell<NLayerIdx, NMiddleOffset, NType>
                (idx, stencil, layers);
            calc_cell<NLayerIdx, NLeftOffset, NType>
                (idx + x_sub, stencil, layers);

            calc_cell<NNextLayerIdx, NRightOffset, NType>
                (idx + x_add, stencil, layers);
            calc_cell<NNextLayerIdx, NMiddleOffset, NType>
                (idx, stencil, layers);
        }
        else
        {
            int64_t y_sub = 
                TGeneralLayer::template off_top<NLess - 1>(idx, 1);
            int64_t y_add = 
                TGeneralLayer::template off_bottom<NLess - 1>(idx, 1);

            int64_t x_sub_1 = 
                TGeneralLayer::template off_left<NLess - 1>(idx, 1);
            int64_t x_sub_2 = 
                TGeneralLayer::template off_left<NLess>(idx, 1);

            int64_t x_add_1 = 
                TGeneralLayer::template off_right<NLess - 1>(idx, 1);
            int64_t x_add_2 = 
                TGeneralLayer::template off_right<NLess>(idx, 1);

            proc_fold<NLess, NLayerIdx, NHalfLeftOffset, NBottomType>
                (idx + y_add + x_sub_1, stencil, layers);
            proc_fold<NLess, NLayerIdx, NMiddleOffset, NMiddleType>
                (idx,                   stencil, layers);
            proc_fold<NLess, NLayerIdx, NLeftOffset, NMiddleType>
                (idx         + x_sub_2, stencil, layers);
            proc_fold<NLess, NLayerIdx, NHalfLeftOffset, NTopType>
                (idx + y_sub + x_sub_1, stencil, layers);

            proc_fold<NLess, NNextLayerIdx, NHalfRightOffset, NBottomType>
                (idx + y_add + x_add_1, stencil, layers);
            proc_fold<NLess, NNextLayerIdx, NRightOffset, NMiddleType>
                (idx         + x_add_2, stencil, layers);
            proc_fold<NLess, NNextLayerIdx, NMiddleOffset, NMiddleType>
                (idx,                   stencil, layers);
            proc_fold<NLess, NNextLayerIdx, NHalfRightOffset, NTopType>
                (idx + y_sub + x_add_1, stencil, layers);
        }
    }

    template<size_t NLayerIdx, EOffset NOffset, EType NType, 
             typename TStencil, typename TGeneralLayer>
    static void calc_cell(int64_t idx, 
                          const TStencil& stencil, TGeneralLayer* layers) 
                          noexcept
    {
        if constexpr (NType == TYPE_D || NType == TYPE_N || 
                      NOffset == OFFSET_F || NOffset == OFFSET_G || 
                      NOffset == OFFSET_N)
            return;

        static constexpr int NXSide = []() -> int {
            if constexpr (NOffset == OFFSET_A) return -1;
            if constexpr (NOffset == OFFSET_B) return -1;
            if constexpr (NOffset == OFFSET_C) return 0;
            if constexpr (NOffset == OFFSET_D) return 1;
            if constexpr (NOffset == OFFSET_E) return 1;

            return 0;
        }();

        static constexpr int NYSide = []() -> int {
            if constexpr (NType == TYPE_A) return -1;
            if constexpr (NType == TYPE_B) return 0;
            if constexpr (NType == TYPE_C) return 1;

            return 0;
        }();

        idx += TGeneralLayer::template off_right<0>(idx, 1) + 
               TGeneralLayer::template off_bottom<0>(idx, 1);

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
