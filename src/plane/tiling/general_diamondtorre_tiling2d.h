#ifndef WAVE_MODEL_TILING_GENERAL_DIAMONDTORRE_TILING2D_H_
#define WAVE_MODEL_TILING_GENERAL_DIAMONDTORRE_TILING2D_H_

#include "logging/macro.h"

#include <vector>
#include <algorithm>

#include <cstdint>

namespace wave_model {

// [ 00 ]
// [0000]
// [ 0X ]

template<size_t NR>
struct WmGeneralDiamondTorreTiling2D
{
    struct Test;

    static constexpr size_t NTileRank = NR;
    // static constexpr size_t NDepth = 1u << NTileRank;

    static_assert(NTileRank > 0, "NTileRank must be positive");

    enum EType
    {
        TYPE_A, TYPE_B, TYPE_C, TYPE_N
    };

    static constexpr EType TypeRightArr[] = {
        /* [TYPE_A] = */ TYPE_A,
        /* [TYPE_B] = */ TYPE_B,
        /* [TYPE_C] = */ TYPE_C,
        /* [TYPE_N] = */ TYPE_N,
    };

    static constexpr EType TypeLeftArr[] = {
        /* [TYPE_A] = */ TYPE_N,
        /* [TYPE_B] = */ TYPE_B,
        /* [TYPE_C] = */ TYPE_B,
        /* [TYPE_N] = */ TYPE_N,
    };

    // TODO: to generate code for the each case
    template<size_t NRank, typename TStencil, typename TGeneralLayer>
    static void traverse(const TStencil& stencil, 
                         TGeneralLayer* layers) noexcept
    {
        // guaranteed to be 2's power
        static constexpr size_t NLineCnt = 
            TGeneralLayer::NDomainLengthX / (1u << NTileRank);

        // FIXME:
        static constexpr size_t NIdx = 
            TGeneralLayer::template off_right<NRank>(0, 1) +
            TGeneralLayer::template off_bottom<NRank>(0, 1);

        // FIXME: (1u << NRank)
        traverse_lines<NLineCnt, NRank, true>(NIdx, stencil, layers);
        traverse_bound<(1u << NRank), NRank>(0, stencil, layers);
    }

    template<size_t NLineCnt, size_t NRank, bool NIsTypeC, 
             typename TStencil, typename TGeneralLayer>
    static void traverse_lines(int64_t idx, int64_t layer_idx, 
            const TStencil& stencil, TGeneralLayer* layers) noexcept
    {
        int64_t left_idx = idx;
        int64_t right_idx = idx + TGeneralLayer::template 
            off_right<NTileRank + NChunkRank - 1>(idx, 1);

        if constexpr (NLineCnt == 1)
        {
            // call right to left
            if constexpr (NIsTypeC)
            {
                // type C case
                proc_line<NRank, TYPE_RC, false>
                    (right_idx, layer_idx, stencil, layers);
                proc_line<NRank, TYPE_LC, true>
                    (left_idx, layer_idx, stencil, layers);
            }
            else
            {
                // type B case
                proc_line<NRank, TYPE_RB, false>
                    (right_idx, layer_idx, stencil, layers);
                proc_line<NRank, TYPE_LB, true>
                    (left_idx, layer_idx, stencil, layers);
            }
        }
        else
        {
            // traverse right to left
            traverse_lines<NLineCnt / 2, NRank, NIsTypeC>
                (right_idx, layer_idx, stencil, layers);
            traverse_lines<NLineCnt / 2, NRank, NIsTypeC>
                (left_idx, layer_idx, stencil, layers);

            // proc most - left (type A case)
            if constexpr (NLineCnt % 2)
            {
                proc_line<NRank, TYPE_RA, false>
                    (right_idx, layer_idx, stencil, layers);
                proc_line<NRank, TYPE_LA, true>
                    (left_idx, layer_idx, stencil, layers);
            }
        }
    }

    template<size_t NLineCnt, size_t NRank, EType NType, 
             typename TStencil, typename TGeneralLayer>
    static void traverse_bound(int64_t idx, int64_t layer_idx, 
            const TStencil& stencil, TGeneralLayer* layers) noexcept
    {
        static constexpr size_t NMod = TStencil::NDepth;

        int64_t left_idx = idx;
        int64_t right_idx = idx + TGeneralLayer::template 
            off_top<NTileRank - 1>(idx, 1);

        if constexpr (NLineCnt == 1)
        {
            layer_idx = (layer_idx + (1u << (NTileRank - 1))) % NMod;
            proc_line<NRank, TYPE_LA, false>
                (right_idx, layer_idx, stencil, layers);

            layer_idx = (layer_idx + (1u << (NTileRank - 1))) % NMod;
            proc_line<NRank, TYPE_LA, true>
                (left_idx, layer_idx, stencil, layers);
        }
        else
        {
            traverse_bound<NLineCnt / 2, NRank>
                (idx, layer_idx, stencil, layers);

            layer_idx = (layer_idx + NLineCnt * (1u << (NTileRank - 1))) % NMod;

            traverse_bound<NLineCnt / 2, NRank>
                (idx, layer_idx, left_idx, stencil, layers);
        }
    }

    template<size_t NRank, EType NType, bool NIsWide, 
             typename TStencil, typename TGeneralLayer>
    static void proc_line(int64_t idx, int64_t layer_idx,
            const TStencil& stencil, TGeneralLayer* layers) noexcept
    {
        static constexpr size_t NLess = NRank - 1;
        static constexpr size_t NMod = TStencil::NDepth;

        // TODO: to generate code instead of this
        if constexpr (NXType == TYPE_N || NYType == TYPE_N)
            return;

        WM_ASSERT(idx >= 0, "index must be non-negative");

        for (int64_t row_num = 0; 
             row_num <= (1u << NRank - NTileRank); ++row_num)
        {
            calc_cell<NXType, NYType, NLayerIdx>(idx, stencil, layers);
            idx += TGeneralLayer::template off_top<NTileRank>(idx, 1);
        }

        if constexpr (NRank == 0u)
        {
            calc_cell<NXType, NYType, NLayerIdx>(idx, stencil, layers);
        }
        else
        {
            // TODO: to optimize for z-order case
            int64_t x_dec = TGeneralLayer::template off_left<NLess>(idx, 1);
            int64_t y_dec = TGeneralLayer::template off_top<NLess>(idx, 1);
            int64_t x_inc = TGeneralLayer::template off_right<NLess>(idx, 1);
            int64_t y_inc = TGeneralLayer::template off_bottom<NLess>(idx, 1);

            proc_fold<NLess, TypeMtx[NXType][1], TypeMtx[NYType][1], NLayerIdx>
                (idx, stencil, layers); // XY

            proc_fold<NLess, TypeMtx[NXType][0], TypeMtx[NYType][1], NLayerIdx>
                (idx + x_dec, stencil, layers); //0Y

            proc_fold<NLess, TypeMtx[NXType][1], TypeMtx[NYType][0], NLayerIdx>
                (idx + y_dec, stencil, layers); // X0

            proc_fold<NLess, TypeMtx[NXType][0], TypeMtx[NYType][0], NLayerIdx>
                (idx + x_dec + y_dec, stencil, layers); // 00

            // upper layer
            if constexpr (NRank <= NTileRank)
            {
                proc_fold<NLess, TypeMtx[NXType][3], TypeMtx[NYType][3], 
                          (NLayerIdx + (1 << NLess)) % NMod>
                    (idx + x_inc + y_inc, stencil, layers); // XY

                proc_fold<NLess, TypeMtx[NXType][2], TypeMtx[NYType][3], 
                          (NLayerIdx + (1 << NLess)) % NMod>
                    (idx + y_inc, stencil, layers); //0Y

                proc_fold<NLess, TypeMtx[NXType][3], TypeMtx[NYType][2], 
                          (NLayerIdx + (1 << NLess)) % NMod>
                    (idx + x_inc, stencil, layers); // X0

                proc_fold<NLess, TypeMtx[NXType][2], TypeMtx[NYType][2], 
                          (NLayerIdx + (1 << NLess)) % NMod>
                    (idx, stencil, layers); // 00
            }
        }
    }

    template<EType NXType, EType NYType, size_t NLayerIdx, 
             typename TStencil, typename TGeneralLayer>
    static void calc_cell(int64_t idx, 
            const TStencil& stencil, TGeneralLayer* layers) noexcept
    {
        if constexpr (NXType == TYPE_A || NXType == TYPE_N ||
                      NYType == TYPE_A || NYType == TYPE_N)
            return;

        // TODO: to refactor with normal switch
        static constexpr int NXSide = 
            static_cast<int>(NXType) - static_cast<int>(TYPE_C);

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
