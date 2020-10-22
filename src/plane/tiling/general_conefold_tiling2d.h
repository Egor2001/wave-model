#ifndef WAVE_MODEL_TILING_GENERAL_CONEFOLD_TILING2D_H_
#define WAVE_MODEL_TILING_GENERAL_CONEFOLD_TILING2D_H_

#include "logging/macro.h"

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
struct WmGeneralConeFoldTiling2D
{
    struct Test;

    static constexpr size_t NTileRank = NR;
    static constexpr size_t NDepth = 1u << NTileRank;

    enum EType
    {
        TYPE_A, TYPE_B, TYPE_C, TYPE_D, TYPE_N
    };

    // indexation:
    // [ 23]
    // [01 ]

    static constexpr EType TypeMtx[5][4] = {
        /* [TYPE_A] = */ { TYPE_N, TYPE_A, TYPE_A, TYPE_B },
        /* [TYPE_B] = */ { TYPE_B, TYPE_C, TYPE_C, TYPE_C },
        /* [TYPE_C] = */ { TYPE_C, TYPE_C, TYPE_C, TYPE_C },
        /* [TYPE_D] = */ { TYPE_C, TYPE_D, TYPE_D, TYPE_N },
        /* [TYPE_N] = */ { TYPE_N, TYPE_N, TYPE_N, TYPE_N }
    };

    // TODO: to generate code for the each case
    template<size_t NRank, typename TStencil, typename TGeneralLayer>
    static void traverse(const TStencil& stencil, 
                         TGeneralLayer* layers) noexcept
    {
        // guaranteed to be 2's power
        static constexpr size_t NQuadCnt = 
            TGeneralLayer::NDomainLengthY / TGeneralLayer::NDomainLengthX;

        traverse_quad<NRank, 0, NQuadCnt>(stencil, layers);
    }

    // TODO: to replace length with rank
    template<size_t NChunkLength, size_t NRank, 
             size_t NQuadIdx, size_t NQuadCnt,
             typename TStencil, typename TGeneralLayer>
    static void traverse_chunk(const TStencil& stencil, 
                               TGeneralLayer* layers) noexcept
    {
        if constexpr (NChunkLength == 0)
        {
            traverse_quad<NRank, NQuadIdx, NQuadCnt>(stencil, layers);
        }
        else
        {
            static constexpr size_t NHalfLength = NChunkLength / 2;

            // calculate bottom - to top
            traverse_chunk<NHalfLength, NRank, 
                           NQuadIdx + NHalfLength, NQuadCnt>(stencil, layers);
            traverse_chunk<NHalfLength, NRank, 
                           NQuadIdx, NQuadCnt>(stencil, layers);
        }
    }

    template<size_t NRank, size_t NQuadIdx, size_t NQuadCnt,
             typename TStencil, typename TGeneralLayer>
    static void traverse_quad(const TStencil& stencil, 
                              TGeneralLayer* layers) noexcept
    {
        static constexpr size_t NLess = NRank - 1;
        static constexpr int64_t NIdx = (1u << NRank) * NQuadIdx;

        // such lambda call is guarenteed to be constexpr but ?: is not
        static constexpr EType NYTypeA = []() {
            if constexpr (NQuadIdx == 0) return TYPE_A;
            else                         return TYPE_C;
        }();

        // such lambda call is guarenteed to be constexpr but ?: is not
        static constexpr EType NYTypeB = []() {
            if constexpr (NQuadIdx == 0) return TYPE_B;
            else                         return TYPE_C;
        }();

        // such lambda call is guarenteed to be constexpr but ?: is not
        static constexpr EType NYTypeD = []() {
            if constexpr (NQuadIdx + 1 == NQuadCnt) return TYPE_D;
            else                                    return TYPE_C;
        }();

        int64_t x_off_1 = TGeneralLayer::template off_right<NLess>(NIdx, 1);
        int64_t y_off_1 = TGeneralLayer::template off_bottom<NLess>(NIdx, 1);
        int64_t x_off_2 = TGeneralLayer::template off_right<NLess>(NIdx, 2);
        int64_t y_off_2 = TGeneralLayer::template off_bottom<NLess>(NIdx, 2);

        // diag 4
        proc_fold<NLess, TYPE_D, NYTypeD>
            (NIdx + x_off_2 + y_off_2, stencil, layers);

        // diag 3
        proc_fold<NLess, TYPE_B, NYTypeD>
            (NIdx + x_off_1 + y_off_2, stencil, layers);
        proc_fold<NLess, TYPE_D, NYTypeB>
            (NIdx + x_off_2 + y_off_1, stencil, layers);

        // diag 2
        proc_fold<NLess, TYPE_B, NYTypeB>
            (NIdx + x_off_1 + y_off_1, stencil, layers);
        proc_fold<NLess, TYPE_A, NYTypeD>
            (NIdx + y_off_2,           stencil, layers);
        proc_fold<NLess, TYPE_D, NYTypeA>
            (NIdx + x_off_2,           stencil, layers);

        // diag 1
        proc_fold<NLess, TYPE_A, NYTypeB>
            (NIdx + y_off_1,           stencil, layers);
        proc_fold<NLess, TYPE_B, NYTypeA>
            (NIdx + x_off_1,           stencil, layers);

        // diag 0
        proc_fold<NLess, TYPE_A, NYTypeA>
            (NIdx,                     stencil, layers);
    }

    template<size_t NRank, EType NXType, EType NYType, 
             typename TStencil, typename TGeneralLayer>
    static void proc_fold(int64_t idx, 
            const TStencil& stencil, TGeneralLayer* layers) noexcept
    {
        static constexpr size_t NLess = NRank - 1;

        // TODO: to generate code instead of this
        if constexpr (NXType == TYPE_N || NYType == TYPE_N)
            return;

        WM_ASSERT(idx >= 0, "index must be non-negative");

        if constexpr (NRank == 0u)
        {
            calc_cell<NXType, NYType>(idx, stencil, layers);
        }
        else
        {
            // TODO: to optimize for z-order case
            int64_t x_dec = TGeneralLayer::template off_left<NLess>(idx, 1);
            int64_t y_dec = TGeneralLayer::template off_top<NLess>(idx, 1);
            int64_t x_inc = TGeneralLayer::template off_right<NLess>(idx, 1);
            int64_t y_inc = TGeneralLayer::template off_bottom<NLess>(idx, 1);

            proc_fold<NLess, TypeMtx[NXType][1], TypeMtx[NYType][1]>
                (idx, stencil, layers); // XY

            proc_fold<NLess, TypeMtx[NXType][0], TypeMtx[NYType][1]>
                (idx + x_dec, stencil, layers); //0Y

            proc_fold<NLess, TypeMtx[NXType][1], TypeMtx[NYType][0]>
                (idx + y_dec, stencil, layers); // X0

            proc_fold<NLess, TypeMtx[NXType][0], TypeMtx[NYType][0]>
                (idx + x_dec + y_dec, stencil, layers); // 00

            // upper layer
            if constexpr (NRank <= NTileRank)
            {
                layers += (1 << (NLess));

                proc_fold<NLess, TypeMtx[NXType][3], TypeMtx[NYType][3]>
                    (idx + x_inc + y_inc, stencil, layers); // XY

                proc_fold<NLess, TypeMtx[NXType][2], TypeMtx[NYType][3]>
                    (idx + y_inc, stencil, layers); //0Y

                proc_fold<NLess, TypeMtx[NXType][3], TypeMtx[NYType][2]>
                    (idx + x_inc, stencil, layers); // X0

                proc_fold<NLess, TypeMtx[NXType][2], TypeMtx[NYType][2]>
                    (idx, stencil, layers); // 00
            }
        }
    }

    template<EType NXType, EType NYType, 
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

        stencil.template apply<NXSide, NYSide>(idx, layers);
    }
};

template<size_t NR>
struct WmGeneralConeFoldTiling2D<NR>::Test
{
    template<typename TStream>
    struct TestStencil
    {
        struct TData {};

        explicit TestStencil(TStream& stream_ref):
            stream(stream_ref)
        {}

        template<int NXSide, int NYSide, typename TGeneralLayer>
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
        stream << "BEGIN WmGeneralConeFoldTiling2D::Test::test_traverse()\n"; 
        TestStencil<TStream> stencil(stream);
        static TGeneralLayer<typename TestStencil<TStream>::TData, 1u << NRank> 
            layers[WmGeneralConeFoldTiling2D::NDepth] = {};

        WmGeneralConeFoldTiling2D::traverse<NRank>(stencil, layers);
        stream << "END WmGeneralConeFoldTiling2D::Test::test_traverse()\n";

        return stream;
    }
};

} // namespace wave_model

#endif // WAVE_MODEL_TILING_GENERAL_CONEFOLD_TILING2D_H_
