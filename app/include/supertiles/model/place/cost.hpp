#ifndef __SUPERTILES_PLACE_COST__
#define __SUPERTILES_PLACE_COST__

#include <array>
#include "helper_util.h"
#include "../config_types.hpp"
#include "../quad_tree.hpp"
#include <bitset>
#include <unordered_map>

namespace supertiles
{
    namespace place
    {
        const double m_eps = 1e-5;

        enum costVecMode_t
        {
            costVecMode_replace = 1,
            costVecMode_singleRoot = 3,
        };

        template<distFuncType_t DIST_FUNC, typename D>
        struct DistOp
        {
            void operator()(const D &v, const D &value)
            {
                if (DIST_FUNC == distFuncType_norm2) {
                    dists += (v - value) * (v - value);
                    cnt++;
                } else if (DIST_FUNC == distFuncType_cosine_normalized) {
                    dists += v * value;
                    mag0 += v * v;
                    mag1 += value * value;
                    cnt++;
                } else
                    assert(false);
            }

            template<bool squared = false>
            D get() const
            {
                assert(cnt > 0);
                if (DIST_FUNC == distFuncType_norm2) {
                    const auto d2 = (dists / cnt);

                    if (squared)
                        return d2;
                    else
                        return std::sqrt(d2);
                } else if (DIST_FUNC == distFuncType_cosine_normalized) {
                    if (mag0 == mag1 && dists == mag1)
                        return 0.;

                    hassertm(mag0 > 0, mag0);
                    hassertm(mag1 > 0, mag1);
                    auto d = dists / (std::sqrt(mag0) * std::sqrt(mag1));
                    hassertm4(d >= 0. && d <= 1 + m_eps, d, dists, mag0, mag1);
                    d = 1. - d;

                    if (squared)
                        return d * d;
                    else
                        return d;
                } else {
                    assert(false);
                    return -1.;
                }
            }

            D dists = 0.;
            D mag0 = 0.;
            D mag1 = 0.;

            uint32_t cnt = 0;
        };

        template<typename cost_t=double>
        class CostVecCache
        {
        public:
            template<typename I>
            auto key(supertiles_QuadTree_iterator_up<I> qt_it) const
            {
                return qt_it.idx;
            }

            template<typename K>
            std::pair<cost_t, bool> get(const K key) const
            {
                const auto it = m.find(key);
                if (it == m.end())
                    return std::make_pair(std::numeric_limits<cost_t>::lowest(), false);
                else
                    return std::make_pair(it->second, true);
            }

            template<typename K>
            void add(const K key, const cost_t value)
            {
                if (true) {
                    {
                        const bool success = m.emplace(key, value).second;
                        hassert(success);
                    }
                }
            }

            std::unordered_map<size_t, double> m;
        };

        template<typename cost_t=double>
        class CostVecCache_dummy
        {
        };

        template<
            costVecMode_t costVecMode, distFuncType_t DIST_FUNC, typename COST_CACHE,
            typename D_IT,
            typename QT_IT,
            typename NLC,
            typename NLC0
        >
        auto costVec(
            COST_CACHE &costCache,
            // the representation at these nodes are not
            // considered for cost calculation
            // (if replaceLeaves is set below, they are
            // not just omitted but replaced by other values)
            const std::vector <QT_IT> &qt_itv,
            // this defines the node that is primarily followed up the tree
            QT_IT qt_it_ref,
            // this specifies the tiles that are compared against the different
            // levels of the tree
            const std::vector <uint32_t> &tileIdx_ref,
            // this is the input data
            D_IT supertiles,
            D_IT supertiles_0,
            const size_t nElemsTile,
            const NLC &nodeLeafCounts,
            const NLC0 &nodeLeafCounts_0,
            size_t maxNLevelsUp/*=-1*/,
            //bool beginOneLevelUp=false,
            size_t beginNLevelsUp = 0,
            // replace the values of qt_itv when they are encountered
            std::vector <size_t> replaceLeaves = std::vector<size_t>()
        )
        {
            const auto N = qt_itv.size();
            using D =
                typename std::remove_const<typename std::remove_reference<decltype(supertiles[0])>::type>::type;

            constexpr bool mode_singleRoot =
                (costVecMode == costVecMode_singleRoot);
            constexpr bool mode_replace =
                (costVecMode == costVecMode_replace);

            D cost = 0.;

            std::vector <uint32_t> nodeLeafCounts_tileIdx_ref;
            nodeLeafCounts_tileIdx_ref.reserve(tileIdx_ref.size());
            for (
                const auto &t: tileIdx_ref
                ) {
                const auto lcnt = nodeLeafCounts_0(t);
                hassertm2(lcnt == 0 || lcnt == 1, lcnt, t);
                nodeLeafCounts_tileIdx_ref.push_back(lcnt);
            }

            //
            // all tiles are void
            //
            if (std::accumulate(
                nodeLeafCounts_tileIdx_ref.begin(),
                nodeLeafCounts_tileIdx_ref.end(),
                static_cast<uint32_t>(0)) == 0) {
                return cost;
            }

            auto tmp_qt_itv = qt_itv;
            std::vector<bool> sameNodeAsRef(qt_itv.size(), false);

            auto processCost = [](const auto &dists) {

                if (dists.cnt > 0)
                    return dists.get();
                else {
                    return static_cast<D>(0);
                }
            };
            size_t levelUpCnt = 0;
            auto levelUp = [&]() {
                //
                // go up one level
                //
                {
                    const bool success = qt_it_ref();
                    assert(mode_singleRoot || success);

                    if (mode_singleRoot && !success)
                        return false;
                }

                if (!mode_singleRoot)
                    for (
                        size_t i = 0; i < N; i++
                        ) {
                        const bool success = tmp_qt_itv[i]();
                        hassert(success);
                    }

                if (levelUpCnt >= maxNLevelsUp)
                    return false;

                levelUpCnt++;

                return true;
            };

            for (
                size_t i = 0; i < beginNLevelsUp; i++
                )
                levelUp();

            std::vector <std::tuple<size_t, D, uint32_t>> costs;

            while (true) {
                const auto key = costCache.key(qt_it_ref);
                {
                    double costRest;
                    bool success;
                    std::tie(costRest, success) =
                        costCache.get(key);
                    if (success) {
                        cost += costRest;
                        break;
                    }
                }

                const auto nodeLeafCounts_qt_it_ref = nodeLeafCounts(qt_it_ref.idx);

                D costLevel = 0.;
                if (nodeLeafCounts_qt_it_ref) {
                    if (!mode_singleRoot) {
                        bool all = true;
                        for (
                            size_t i = 0; i < N; i++
                            ) {
                            sameNodeAsRef[i] =
                                (tmp_qt_itv[i].idx == qt_it_ref.idx);
                            all = (all && sameNodeAsRef[i]);
                        }

                        if (all)
                            break;
                    }

                    std::vector <DistOp<DIST_FUNC, D>> dists(tileIdx_ref.size());

                    for (
                        const size_t &elemId: helper::range_n(nElemsTile)) {
                        auto getValueIdx = [&elemId, &nElemsTile]
                            (auto idx) { return idx * nElemsTile + elemId; };

                        auto getValue = [&getValueIdx, &supertiles]
                            (auto idx) {
                            return supertiles[getValueIdx(idx)];
                        };

                        auto getValue_0 = [&getValueIdx, &supertiles_0]
                            (auto idx) {
                            return supertiles_0[getValueIdx(idx)];
                        };

                        D value = getValue(qt_it_ref.idx);
                        int32_t nValidLeaves = nodeLeafCounts(qt_it_ref.idx);

                        if (mode_replace)
                            for (
                                size_t i = 0; i < N; i++
                                )
                                if (sameNodeAsRef[i]) {
                                    value -= getValue(qt_itv[i].idx);
                                    nValidLeaves -= nodeLeafCounts(qt_itv[i].idx);
                                    if (!replaceLeaves.empty()) {
                                        value += getValue(replaceLeaves[i]);
                                        nValidLeaves += nodeLeafCounts(replaceLeaves[i]);
                                    }
                                }

                        assert(replaceLeaves.empty() || nValidLeaves >= 0);

                        if (nValidLeaves == 0) {
                            break;
                        }

                        const auto v = value / nValidLeaves;

                        for (
                            const auto &ti: helper::range_n(tileIdx_ref.size()))
                            if (nodeLeafCounts_tileIdx_ref[ti]) {
                                const auto vt = getValue_0(tileIdx_ref[ti])
                                    / nodeLeafCounts_tileIdx_ref[ti];
                                dists[ti](vt, v);
                            }
                    }

                    for (const auto &dist: dists)
                        costLevel += processCost(dist);
                }

                cost += costLevel;
                costs.emplace_back(key, cost, qt_it_ref.idx);

                if (!levelUp())
                    break;
            }

            {
                D prevCostLevel = 0.;
                for (const auto [key, costLevel, nodeId]: costs) {
                    assert(cost >= costLevel);
                    costCache.add(key, cost - prevCostLevel);
                    prevCostLevel = costLevel;
                }
            }

            return cost;

        }

        template<
            distFuncType_t DIST_FUNC, typename IDCS,
            typename D,
            typename D_IT,
            typename NodeLeafCounts,
            typename QT
        >
        auto supertilesCost(
            std::vector <D> &costsLeaves,
            const IDCS &leafIdcs,
            D_IT supertiles,
            const size_t nElemsTile,
            const NodeLeafCounts &nodeLeafCounts,
            const QT &qt,
            const std::vector <uint32_t> &qtNeighbors,
            size_t nNeighbors,
            double neighborFac,
            size_t beginNLevelsUp,
            size_t maxNLevelsUp
        )
        {

            const bool considerHierarchy = (maxNLevelsUp > 0);

            // need to consider at least one of the two aspects to get
            // a meaningful result
            assert(nNeighbors > 0 || considerHierarchy);

#ifdef __NO_OMP
#error "SHOULD NOT BE DEFINED"
#endif

#ifdef NO_OMP
#define __NO_OMP
#endif

            constexpr
            size_t nChunks = 128;

            std::array <D, nChunks> costsChunks;

            const size_t chunkSize = helper::iDivUp(static_cast<size_t>(qt.nLeaves()), nChunks);

            using QT_IT = supertiles_QuadTree_iterator_up<int64_t>;

            // only leaf nodes should be queried in the following,
            // therefore the identify function should suffice
            const auto nodes2leaves = helper::range_n(qt.nLeaves());

            assert(costsLeaves.empty() || costsLeaves.size() == qt.nLeaves());

#ifndef __NO_OMP
#pragma omp parallel for
#endif
            for (size_t chunkId = 0; chunkId < nChunks; chunkId++) {
                auto &cost_alt = costsChunks[chunkId];
                cost_alt = 0;
                for (size_t l = chunkId * chunkSize; l < std::min((chunkId + 1) * chunkSize, leafIdcs.size()); l++) {
                    const auto leafIdx = leafIdcs[l];

                    CostVecCache<double> costCache;

                    const std::vector <uint32_t> tileIdx_ref(1, leafIdx);

                    // can begin one level up as base level is element itself by definition

                    D c = 0;

                    if (considerHierarchy)
                        c = costVec<costVecMode_singleRoot, DIST_FUNC>
                            (
                                costCache,
                                std::vector<QT_IT>(),
                                qt(leafIdx),
                                tileIdx_ref,
                                &supertiles[0],
                                &supertiles[0],
                                nElemsTile,
                                nodeLeafCounts,
                                nodeLeafCounts,
                                maxNLevelsUp,
                                std::max(beginNLevelsUp, static_cast<size_t>(1)));

                    if (neighborFac) {
                        for (
                            uint32_t i = 0; i < nNeighbors; i++
                            ) {
                            const auto neighbor = qtNeighbors[nNeighbors * leafIdx + i];

                            if (neighbor != -1) {
                                c += neighborFac *
                                    costVec<costVecMode_singleRoot, DIST_FUNC>
                                        (
                                            costCache,
                                            std::vector<QT_IT>(),
                                            qt(neighbor),
                                            tileIdx_ref,
                                            &supertiles[0],
                                            &supertiles[0],
                                            nElemsTile,
                                            nodeLeafCounts,
                                            nodeLeafCounts,
                                            maxNLevelsUp,
                                            beginNLevelsUp
                                        );
                            }
                        }
                    }

                    if (!costsLeaves.empty())
                        costsLeaves[leafIdx] = c;
                    cost_alt += c;
                }
            }
            return std::accumulate(costsChunks.begin(), costsChunks.end(), static_cast<D>(0));

#ifdef __NO_OMP
#undef __NO_OMP
#endif
        }

        template<typename IT>
        struct RangeLeaves
        {
            RangeLeaves(
                const IT b_in, const IT e_in,
                const size_t supertileLevelOffset_in,
                const std::vector <std::vector<uint32_t>> &
                nodes2leaves_in
            ) :
                b(b_in), e(e_in),
                supertileLevelOffset(supertileLevelOffset_in),
                nodes2leaves(nodes2leaves_in)
            {}

            template<typename I>
            auto operator[](const I &i) const
            {
                const auto nodeIdLevel = i / nLeavesPerNode();

                assert(nodeIdLevel < e - b);
                return b[nodeIdLevel] + (i - nodeIdLevel * nLeavesPerNode());
            }

            auto nLeavesPerNode() const
            {
                return nodes2leaves[supertileLevelOffset].size();
            }

            auto size() const
            {
                return (e - b) * nLeavesPerNode();
            }

            const IT b;
            const IT e;

            const size_t supertileLevelOffset;
            const std::vector <std::vector<uint32_t>> &
                nodes2leaves;
        };

        template<distFuncType_t DIST_FUNC, typename D_IT, typename NodeLeafCounts, typename QT>
        auto supertilesCost(
            D_IT supertiles,
            const size_t nElemsTile,
            const NodeLeafCounts &nodeLeafCounts,
            const QT &qt,
            const std::vector <uint32_t> &qtNeighbors,
            size_t nNeighbors,
            double neighborFac,
            size_t maxNLevelsUp
        )
        {

            using D =
                typename std::remove_const<typename std::remove_reference<decltype(supertiles[0])>::type>::type;
            std::vector <D> costsLeaves_IGNORE;
            return supertilesCost<DIST_FUNC>(
                costsLeaves_IGNORE,
                helper::RangeN<size_t>(qt.nLeaves()),
                supertiles,
                nElemsTile,
                nodeLeafCounts,
                qt,
                qtNeighbors,
                nNeighbors,
                neighborFac,
                0
            );
        }


        template<distFuncType_t DIST_FUNC, typename D_IT, typename QT, typename A, typename NodeLeafCounts>
        auto computeNodeDisparity(
            D_IT supertiles,
            const size_t nElemsTile,
            const QT &qt,
            const A &qtLeafAssignment,
            const NodeLeafCounts &nodeLeafCounts
        )
        {
            using D = double;
            std::vector <D> nodeDisparity(qt.nElems(), 0.);
            const auto nodes2leaves = genMap_qt2leaves(qt);

            std::vector<bool> isLeafVoid(qt.nLeaves());

            for (const auto &leafId: helper::range_n(qt.nLeaves())) {
                const auto tileId = qtLeafAssignment[leafId];
                isLeafVoid[leafId] = (tileId == voidTileIdx);
            }

            for (const auto &nodeId: helper::range_be(qt.nLeaves(), qt.nElems())) {
                auto &d = nodeDisparity[nodeId];
                const auto &leaves = nodes2leaves[nodeId];

                const auto it_ref_begin = supertiles + nodeId * nElemsTile;

                const auto nodeLeafCount = nodeLeafCounts(nodeId);

                size_t cnt = 0;
                for (const auto &leafId: leaves) {
                    if (isLeafVoid[leafId])
                        continue;

                    assert(nodeLeafCount > 0);
                    DistOp<DIST_FUNC, D> dist;
                    const auto it_begin = supertiles + leafId * nElemsTile;
                    const auto it_end = it_begin + nElemsTile;

                    auto it_ref = it_ref_begin;

                    for (auto it = it_begin; it != it_end; it++, it_ref++)
                        dist(*it, (*it_ref) / nodeLeafCount);

                    d += dist.get();
                    cnt++;
                }
                hassertm3(cnt == static_cast<decltype(cnt) > (nodeLeafCount), cnt, nodeLeafCount, nodeId);

                if (cnt > 0)
                    d /= cnt;
            }

            // normalize node disparities with maximum
            {
                const auto disparityMax = nodeDisparity.back();
                std::cout << "DISPARITY MAX: " << nodeDisparity.back() << std::endl;

                hassertm(disparityMax > 0, disparityMax);
                for (auto &e: nodeDisparity)
                    e /= disparityMax;
                hassertm(nodeDisparity.back() == 1., nodeDisparity.back());
            }

            return nodeDisparity;
        }

        auto getNeighborDeltas(int N_NBRS)
        {
            using I2 = V2<int64_t>;

            std::vector <I2> off;

            switch (N_NBRS) {
                case 0:
                    break;
                case 4:
                    off = {I2(-1, 0), I2(1, 0), I2(0, -1), I2(0, 1)};
                    break;
                case 8:
                    off =
                        {I2(-1, 0), I2(1, 0), I2(0, -1), I2(0, 1),
                         I2(1, -1), I2(1, 1), I2(-1, -1), I2(-1, 1)};
                    break;
                default:
                    assert(false);
            }

            return off;
        }
    }
}

#endif // __SUPERTILES_PLACE_COST__
