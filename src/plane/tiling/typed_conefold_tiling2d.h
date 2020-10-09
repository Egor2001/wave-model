#ifndef WAVE_MODEL_TILING_CONEFOLD_TILING2D_H_
#define WAVE_MODEL_TILING_CONEFOLD_TILING2D_H_

#include <vector>
#include <algorithm>

#include <cstdint>

namespace wave_model {

//       [ AB]
// [A] = [NA ]
//
//       [ CC]
// [B] = [BC ]
//
//       [ CC]
// [C] = [CC ]
//
//       [ DN]
// [D] = [CD ]

template<size_t NR>
struct WmConeFoldTiling2D
{
    static constexpr size_t NTileRank = NR;
    static constexpr size_t NDepth = 1u << NTileRank;

    enum EType
    {
        TYPE_A, TYPE_B, TYPE_C, TYPE_D, TYPE_N
    };

    // indexation:
    // [ 23]
    // [01 ]

    static constexpr EType TypeMtx[4][5] = {
        { TYPE_N, TYPE_A, TYPE_A, TYPE_B },
        { TYPE_B, TYPE_C, TYPE_C, TYPE_C },
        { TYPE_C, TYPE_C, TYPE_C, TYPE_C },
        { TYPE_C, TYPE_D, TYPE_D, TYPE_N },
        { TYPE_N, TYPE_N, TYPE_N, TYPE_N },
    };

    // TODO: to generate code for the each case
    template<size_t NRank, typename TStencil, typename TLayer>
    static void traverse(const TStencil& stencil, TLayer* layers) noexcept
    {
        static constexpr size_t NLess = NRank - 1;

        int64_t x_off_1 = TLayer::template off_right<NLess>(0, 1);
        int64_t y_off_1 = TLayer::template off_bottom<NLess>(0, 1);
        int64_t x_off_2 = TLayer::template off_right<NLess>(0, 2);
        int64_t y_off_2 = TLayer::template off_bottom<NLess>(0, 2);

        // diag 4
        proc_fold<NLess, TYPE_D, TYPE_D>(x_off_2 + y_off_2, stencil, layers);

        // diag 3
        proc_fold<NLess, TYPE_B, TYPE_D>(x_off_1 + y_off_2, stencil, layers);
        proc_fold<NLess, TYPE_D, TYPE_B>(x_off_2 + y_off_1, stencil, layers);

        // diag 2
        proc_fold<NLess, TYPE_B, TYPE_B>(x_off_1 + y_off_1, stencil, layers);
        proc_fold<NLess, TYPE_A, TYPE_D>(y_off_2,           stencil, layers);
        proc_fold<NLess, TYPE_D, TYPE_A>(x_off_2,           stencil, layers);

        // diag 1
        proc_fold<NLess, TYPE_A, TYPE_B>(y_off_1,           stencil, layers);
        proc_fold<NLess, TYPE_B, TYPE_A>(x_off_1,           stencil, layers);

        // diag 0
        proc_fold<NLess, TYPE_A, TYPE_A>(0,                 stencil, layers);
    }

    template<size_t NRank, EType NXType, EType NYType, 
             typename TStencil, typename TLayer>
    static void proc_fold(int64_t idx, 
            const TStencil& stencil, TLayer* layers) noexcept
    {
        static constexpr size_t NLess = NRank - 1;

        // TODO: to generate code instead of this
        if constexpr (NXType == TYPE_N || NYType == TYPE_N)
            return;

        WM_ASSERT(idx >= 0, "index must be non-negative");

        if constexpr (NRank == 0u)
        {
            stencil.apply<NXType, NYType>(idx, layers);
        }
        else
        {
            // TODO: to optimize for z-order case
            int64_t x_off = TLayer::template off_right<NLess>(idx, 1);
            int64_t y_off = TLayer::template off_bottom<NLess>(idx, 1);

            proc_fold<NLess, TypeMtx[NXType][1], TypeMtx[NYType][1]>
                (idx + x_off + y_off, stencil, layers); // XY

            proc_fold<NLess, TypeMtx[NXType][0], TypeMtx[NYType][1]>
                (idx + y_off, stencil, layers); // 0Y

            proc_fold<NLess, TypeMtx[NXType][1], TypeMtx[NYType][0]>
                (idx + x_off, stencil, layers); //X0

            proc_fold<NLess, TypeMtx[NXType][0], TypeMtx[NYType][0]>
                (idx, stencil, layers); // 00

            if constexpr (NRank <= NTileRank)
            {
                layers += (1 << (NLess));
                idx += x_off + y_off;
                x_off = TLayer::template off_right<NLess>(idx, 1);
                y_off = TLayer::template off_bottom<NLess>(idx, 1);

                proc_fold<NLess, TypeMtx[NXType][3], TypeMtx[NYType][3]>
                    (idx + x_off + y_off, stencil, layers); // XY

                proc_fold<NLess, TypeMtx[NXType][2], TypeMtx[NYType][3]>
                    (idx + y_off, stencil, layers); // 0Y

                proc_fold<NLess, TypeMtx[NXType][3], TypeMtx[NYType][2]>
                    (idx + x_off, stencil, layers); //X0

                proc_fold<NLess, TypeMtx[NXType][2], TypeMtx[NYType][2]>
                    (idx, stencil, layers); // 00
            }
        }
    }

    template<EType NXType, EType NYType, typename TStencil, typename TLayer>
    static void calc_cell(int64_t idx, 
            const TStencil& stencil, TLayer* layers) noexcept
    {
        if constexpr (NXType == TYPE_A || NXType == TYPE_N ||
                      NYType == TYPE_A || NYType == TYPE_N)
            return;

        // TODO: to refactor with normal switch
        static constexpr int NXDiff = 
            static_cast<int>(NXType) - static_cast<int>(TYPE_C);

        static constexpr int NYDiff = ;
            static_cast<int>(NYType) - static_cast<int>(TYPE_C);

        stencil.apply<NXDiff, NYDiff>(idx, layers);
    }
};

} // namespace wave_model
