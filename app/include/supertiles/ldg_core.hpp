#ifndef __SUPERTILES_PLACE__
#define __SUPERTILES_PLACE__

#ifndef M_VEC
#define M_VEC
#endif

#include <iostream>
#include <algorithm>
#include <cmath>
#include "model/quad_tree.hpp"
#include <random>
#include <array>
#include "model/config_types.hpp"
#include "helper_asciiFile.h"
#include "helper_hungarian.h"
#include "helper_ChronoTimer.h"

#ifndef NO_OMP

#include <omp.h>

#endif

#include <bitset>

#include "util/create_assign_group_permutations.hpp"

#include "util/place/data.hpp"
#include "model/place/plan.hpp"

#include "util/adapt_assignment.hpp"

#include "model/place/log.hpp"
#include "model/place/term.hpp"
#include "helper_readFile.h"

#include "model/program_options.hpp"
#include "util/place/presentation.hpp"

#include "helper_bzip.h"
#include "helper_string.h"

#include "helper_random.h"

#include "helper_color/cm_map.h"
#include "helper_color/cm_gray.h"
#include "helper_color/cm_plasma.h"
#include "helper_color/cm_viridis.h"

#include "helper_CairoDraw.h"
#include <tuple>

#define CHECK_IMPROVEMENT 2
#define REVERT_ASSIGNMENT_MODE 0

#if CHECK_IMPROVEMENT == 1
#define CHECK_IMPROVEMENT_AFTER_BATCH
#elif CHECK_IMPROVEMENT == 2
#define CHECK_IMPROVEMENT_AFTER_PASS
#elif CHECK_IMPROVEMENT != 0
#error "INVALID CHECK IMPROVEMENT MODE"
#endif

#if REVERT_ASSIGNMENT_MODE == 0
#define REVERT_ASSIGNMENT_ADAPT
#elif REVERT_ASSIGNMENT_MODE == 1
#define REVERT_ASSIGNMENT_COPY
#else
#error "INVALID REVERT_ASSIGNMENT_MODE"
#endif

namespace supertiles
{
    namespace place
    {

        bool is_initAndExit(std::string loadAssignments)
        {
            return (loadAssignments == "INIT_AND_EXIT");
        }

        template<
            distFuncType_t DIST_FUNC, typename NodeLeafCounts, typename D_TILES,
            typename DIM, typename TERM, typename RNG
        >
        auto run_(
            const std::vector <D_TILES> &tiles,
            const size_t nTilesFull,
            size_t nElemsTile,
            const DIM &gridDim,
            int nNeighbors,
            double neighborFac,
            TERM term,
            RNG rng,
            const size_t planChunkSizeMax,
            size_t nTilesAssign,
            std::string loadAssignments,
            std::string writeIntermediateAssignments,
            const size_t maxBatchSize,
            const size_t maxNLevelsUp,
            const size_t maxNodeExchangeLevel
        )
        {
            using D = double;

            std::cout << "sizeof(D): " << sizeof(D) << std::endl;

            if (nTilesAssign == -1)
                nTilesAssign = nTilesFull;

            assert(nTilesAssign >= nTilesFull);

            Log<TERM, PassTimings> log(term);

            using ass_t = std::vector<uint32_t>;

            const auto qtNeighbors = getNeighborsQT(gridDim, getNeighborDeltas(nNeighbors));

            hassertm4(tiles.size() == nElemsTile * nTilesFull, tiles.size(), nElemsTile, nTilesFull, nTilesAssign);
            using IDX = int64_t;

            const size_t nTiles = helper::ii2n(gridDim);

            hassertm2(nTilesAssign <= nTiles, nTilesAssign, nTiles);
            hassertm2(nTilesFull <= nTilesAssign, nTilesFull, nTilesAssign);

            using QT = supertiles_QuadTree<IDX>;
            QT qt(nTiles);

#if CHECK_IMPROVEMENT == 0
            std::pair<ass_t, double> best_qtLeafAssignment(ass_t(), std::numeric_limits<double>::max());
#endif

            ass_t qtLeafAssignment;

            bool anyChange = false;

            if (loadAssignments != "" && !is_initAndExit(loadAssignments))
                qtLeafAssignment = read_qtLeafAssignment(loadAssignments, gridDim);

            if (qtLeafAssignment.empty()) {
                anyChange = true;
                qtLeafAssignment.resize(nTiles, voidTileIdx);

                assert(nTilesFull <= nTiles);
                assert(qt.nLeaves() == nTiles);

                const auto isRegular = Plans::isRegularMap<QT>(nTilesFull, nTiles);

                auto tileIdcs = helper::range_n(nTilesFull);

                helper::mrandom_shuffle
                    (
                        tileIdcs.begin(),
                        tileIdcs.end(),
                        rng
                    );

                size_t regularIdx = 0;
                for (const auto &idx: helper::range_n(nTiles))
                    if (isRegular[idx]) {
                        assert(regularIdx < tileIdcs.size());
                        qtLeafAssignment[idx] = tileIdcs[regularIdx];
                        regularIdx++;
                    }
                hassertm3(regularIdx == nTilesFull, regularIdx, nTilesFull, nTiles);
            }

            std::vector <D> supertiles;
#ifdef REVERT_ASSIGNMENT_COPY
            std::vector<D> supertiles_backup;
#endif

            NodeLeafCounts nodeLeafCounts;

            {
                helper::ChronoTimer timer;
                std::tie(supertiles, nodeLeafCounts) =
                    initSupertiles<NodeLeafCounts, D>
                        (qt, qtLeafAssignment, tiles, nElemsTile);
                std::cout << "initializing supertiles and nodeLeafCounts took s: " << timer.get_s() << std::endl;
            }

            const auto nodes2leaves = genMap_qt2leaves(qt);

#ifndef NDEBUG
            auto sanityCheck_nodeLeafCounts = [&]() {
                //
                // CHECK NODE LEAF COUNTS
                //
                for (const auto nodeId: helper::range_n(qt.nElems())) {
                    size_t cnt = 0;
                    const auto &leaves = nodes2leaves[nodeId];
                    for (
                        const auto leafId: leaves
                        )
                        if (qtLeafAssignment[leafId] != voidTileIdx)
                            cnt++;
                    hassertm3(
                        nodeLeafCounts(nodeId) == cnt,
                        nodeLeafCounts(nodeId),
                        cnt,
                        nodeId
                    );
                }
            };
            sanityCheck_nodeLeafCounts();
#endif

            //
            // assess cost
            //
            const auto regularLeafIndices = std::get<0>(Plans::regularTileIndices<QT>(nTilesAssign));

            auto supertilesCost_ = [
                &qt,
                &nElemsTile,
                &neighborFac,
                &nNeighbors,
                &qtNeighbors,
                &supertiles,
                &nodeLeafCounts,
                & regularLeafIndices,
                & maxNLevelsUp
            ]
                (size_t level) {
                std::vector <D> __ignore;
                return supertilesCost<DIST_FUNC>
                    (
                        __ignore,
                        regularLeafIndices,
                        supertiles.begin(),
                        nElemsTile,
                        nodeLeafCounts,
                        qt,
                        qtNeighbors[0],
                        nNeighbors,
                        neighborFac,
                        level,
                        maxNLevelsUp
                    );
            };

#ifdef ASSIGN_GROUP_PERMUTATIONS
            const auto assignmentCandidatesv = createAssignPermutations(planChunkSizeMax);
#endif


#if CHECK_IMPROVEMENT == 0
            std::get<0>(best_qtLeafAssignment)=qtLeafAssignment;
            std::get<1>(best_qtLeafAssignment)=supertilesCost_();
#endif

            if (term.maxNSeconds > 0 && term.maxNPasses && !is_initAndExit(loadAssignments)) {
                Plans plans(
                    qt,
                    nTilesAssign,
                    planChunkSizeMax,
                    maxNodeExchangeLevel
                );

                helper::ChronoTimer timer_cout;

                auto currentCost_0 = supertilesCost_(0);

                while (true) {

                    PassTimings passTimings;
                    helper::ChronoTimer adhocTimer;

                    helper::ChronoTimer passTimer;;

                    std::vector<uint32_t>::const_iterator plan_begin;
                    std::vector<uint32_t>::const_iterator plan_end;
                    bool aggregateExchangeMode;
                    size_t leafLevel;
                    size_t supertileLevelOffset;
                    size_t planChunkSize;

                    adhocTimer.restart();
                    std::tie(plan_begin, plan_end, aggregateExchangeMode, leafLevel, planChunkSize)
                        = plans(rng);

                    passTimings.plan = adhocTimer.get_s();

                    auto currentCost_leafLevel = currentCost_0;

                    if (leafLevel > 0)
                        currentCost_leafLevel = supertilesCost_(leafLevel);

                    std::vector<uint32_t>::const_iterator plan = plan_begin;

                    const size_t plan_size = plan_end - plan;

                    supertileLevelOffset = qt.getLevelOffset(leafLevel);

                    bool changeInPass = 0;

                    auto updateLog = [&]() {
                        helper::ChronoTimer timer;

#ifndef NDEBUG
                        {
                            // check whether iterative updates are actually accurate
                            auto
                                [supertiles_test, nodeLeafCounts_test]
                                = initSupertiles<NodeLeafCounts, D>(
                                qt, qtLeafAssignment,
                                tiles, nElemsTile
                            );

                            assert(nodeLeafCounts._leafCounts == nodeLeafCounts_test._leafCounts);

                            assert(supertiles_test.size() == supertiles.size());
                            for (
                                size_t i = 0; i < supertiles_test.size(); i++
                                )
                                hassertm2(
                                    helper::apprEq(
                                        supertiles_test[i],
                                        supertiles[i]
                                    ),
                                    supertiles_test[i],
                                    supertiles[i]
                                );

#if 0
                            supertiles=supertiles_test;
#endif
                        }
#endif
                        const auto cost = currentCost_0;

                        log(cost, changeInPass, leafLevel, aggregateExchangeMode, passTimings);

#if CHECK_IMPROVEMENT == 0
                        if(cost<best_qtLeafAssignment.second)
                          {
                            if(loadAssignments != ""/* && false*/)
                          {
                            helper::ChronoTimer timer;
                            std::cout << "write updated assignment to " << loadAssignments << std::endl;
                            // << "new cost: " << cost
                            // 	      << ", old cost: " << best_qtLeafAssignment.second << std::endl;
                            writeAssignment(loadAssignments);
                            //std::cout << "writing took " << timer.get_s() << std::endl;
                          }
                            best_qtLeafAssignment=std::make_pair(qtLeafAssignment, cost);
                          }
#endif

                        if (writeIntermediateAssignments != "" && term.maxNSeconds > 0 && term.maxNPasses > 0)
                            helper::bzip_compress(
                                qtLeafAssignment,
                                writeIntermediateAssignments + helper::leadingZeros(
                                    term.passN,
                                    6
                                ) + ".raw.bz2"
                            );
                    };

                    if (term.passN == 0)
                        updateLog();

                    assert(qtLeafAssignment.size() == qt.nNodesLevel(0));

                    auto movedFromQTPos = helper::range_n < std::vector < uint32_t >> (qt.nNodesLevel(leafLevel));

                    auto qtNodeAssignment_vanilla =
                        helper::range_n<ass_t>(qt.nNodesLevel(leafLevel));

                    helper::ChronoTimer timerAdapt;
                    timerAdapt.pause();
                    helper::ChronoTimer timerExchange;
                    timerExchange.pause();


#ifdef ASSIGN_GROUP_HUNGARIAN
                    const size_t batchSize = maxBatchSize;

                    const auto nodeLeafCounts_higherLevelCopy = nodeLeafCounts.higherLevelCopy(leafLevel);
                    const auto qt_higherLevelCopy = qt.higherLevelCopy(leafLevel);
                    const size_t nElemsConsidered = batchSize * planChunkSize;

                    auto revertAssignmentIfNotBetter = [&](
                        const size_t b_from,
                        const size_t b_to
                    ) {
                        const double prev_cost = currentCost_leafLevel;
                        const double post_cost = supertilesCost_(leafLevel);

                        if (post_cost >= prev_cost) {
                            // set aggregate exchange mode to true here even for level 0 because qtLeafAssignment actually needs to be changed
                            adaptAssignment(
                                plan_begin + b_from,
                                plan_begin + b_to,
                                supertiles, tiles, nodes2leaves, nodeLeafCounts, qt, nElemsTile, qtLeafAssignment,
                                helper::invertMap(movedFromQTPos), /*aggregateExchangeMode*/true, supertileLevelOffset
                            );
                            //#endif
#ifdef REVERT_ASSIGNMENT_COPY
                            supertiles=supertiles_backup;
                            hassertm2(currentCost==supertilesCost_(), currentCost, supertilesCost_());
#endif
                            return true;
                        } else {
                            currentCost_leafLevel = post_cost;
                            currentCost_0 = supertilesCost_(0);
                            return false;
                        }
                    };
#elif defined(ASSIGN_GROUP_PERMUTATIONS)
                    const size_t nElemsConsidered=planChunkSize;
#endif
#if defined(REVERT_ASSIGNMENT_COPY) && defined(CHECK_IMPROVEMENT_AFTER_PASS)
                    supertiles_backup=supertiles;
#endif
                    adhocTimer.restart();
                    for (size_t b_idx = 0; b_idx < plan_size; b_idx += nElemsConsidered) {
                        const auto b_from = b_idx;
                        auto b_to = std::min(b_idx + nElemsConsidered, plan_size);

                        if (plan_size - b_to < 0.5 * nElemsConsidered) {
                            b_to = plan_size;
                            b_idx = plan_size;
                        }

                        timerExchange.unpause();

                        std::vector<bool> changev(1, false);

#ifdef ASSIGN_GROUP_HUNGARIAN
                        const auto nThreads =
#ifndef NO_OMP
                            omp_get_max_threads();
#else
                        1;
#endif // NO_OMP
                        changev.resize(nThreads, false);

#ifndef NO_OMP
#pragma omp parallel for
#endif // NO_OMP
                        for (size_t b_idx_batch = b_from; b_idx_batch < b_to; b_idx_batch += planChunkSize)
#endif //ASSIGN_GROUP_HUNGARIAN
                        {
#ifdef ASSIGN_GROUP_HUNGARIAN
                            const auto threadId =
#ifndef NO_OMP
                                omp_get_thread_num();
#else
                            0;
#endif // NO_OMP


                            const auto begin_idx = b_idx_batch;
#else
                            const auto begin_idx = b_idx;
#endif // ASSIGN_GROUP_HUNGARIAN

                            assert(begin_idx < plan_size);

                            const auto nElemsSubPlan =
                                std::min(static_cast<size_t>(planChunkSize), plan_size - begin_idx);

                            if (nElemsSubPlan > 1) {
                                std::vector <IDX> qtIdxv(nElemsSubPlan);
                                for (uint32_t i = 0; i < nElemsSubPlan; i++) {
                                    const auto planIdx = begin_idx + i;
                                    assert(planIdx < plan_size);
                                    const auto &p = plan[planIdx];
                                    qtIdxv[i] = p;
                                }

                                assert(supertileLevelOffset == qt.getLevelOffset(leafLevel));

#ifdef ASSIGN_GROUP_HUNGARIAN
                                auto assignGroupPermutations_ = [&](auto &ass) {
                                    if (
                                        assignGroupPermutations
                                            <DIST_FUNC>
                                            (
                                                ass,
                                                movedFromQTPos,
                                                supertiles.begin() + supertileLevelOffset * nElemsTile,
                                                supertiles.begin(),
                                                nElemsTile,
                                                nodeLeafCounts_higherLevelCopy,
                                                nodeLeafCounts,
                                                nodes2leaves.begin() + supertileLevelOffset,
                                                qt_higherLevelCopy,
                                                qtIdxv,
                                                qtNeighbors[leafLevel],
                                                nNeighbors, neighborFac,
                                                maxNLevelsUp
                                            )
                                        ) {
                                        changev[threadId] = true;
                                    }
                                };

                                if (aggregateExchangeMode)
                                    assignGroupPermutations_(qtNodeAssignment_vanilla);
                                else
                                    assignGroupPermutations_(qtLeafAssignment);
#else
                                auto & ass = (!aggregateExchangeMode) ? qtLeafAssignment : qtNodeAssignment_vanilla;
                                changev[0] |=
                                  omp::assignGroupPermutations
                                  <DIST_FUNC>
                                  (ass,
                                   movedFromQTPos,
                                   supertiles.begin()+supertileLevelOffset*nElemsTile,
                                   supertiles.begin(),
                                   nElemsTile,
                                   nodeLeafCounts.higherLevelCopy(leafLevel),
                                   nodeLeafCounts,
                                   nodes2leaves.begin()+supertileLevelOffset,
                                   qt.higherLevelCopy(leafLevel), qtIdxv,
                                   qtNeighbors[leafLevel],
                                   nNeighbors, neighborFac,
                                   assignmentCandidatesv);
#endif
                            }
                        }
                        timerExchange.pause();

                        bool changeThis = false;
                        for (const auto e: changev) {
                            if (changeThis)
                                break;
                            changeThis = (e || changeThis);
                        }


                        if (changeThis) {
                            timerAdapt.unpause();
#ifndef NDEBUG
                            if (aggregateExchangeMode) {
                                for (auto it = plan_begin + b_from; it != plan_begin + b_to; it++)
                                    assert(qtNodeAssignment_vanilla[*it] == movedFromQTPos[*it]);
                            }
#endif

#if defined(REVERT_ASSIGNMENT_COPY) && defined(CHECK_IMPROVEMENT_AFTER_BATCH)
                            supertiles_backup=supertiles;
#endif

                            adaptAssignment(
                                plan_begin + b_from,
                                plan_begin + b_to,
                                supertiles, tiles, nodes2leaves, nodeLeafCounts, qt, nElemsTile, qtLeafAssignment,
                                movedFromQTPos, aggregateExchangeMode, supertileLevelOffset
                            );

#ifdef CHECK_IMPROVEMENT_AFTER_BATCH
                            changeInPass = changeInPass || (!revertAssignmentIfNotBetter(b_from, b_to));
#else
                            changeInPass = 1;
#endif
#ifndef NDEBUG
                            sanityCheck_nodeLeafCounts();
#endif

                            timerAdapt.pause();
                        }

                    }
                    passTimings.exchangeAdapt = adhocTimer.get_s();

                    passTimings.total = passTimer.get_s();

#ifdef CHECK_IMPROVEMENT_AFTER_PASS
                    changeInPass = !revertAssignmentIfNotBetter(0, plan_size);
#endif

                    if (timer_cout.get_s() > 10) {

                        std::cout << std::setprecision(std::numeric_limits<double>::max_digits10)
                            << "cost: " << currentCost_0 << std::endl;

                        std::cout
                            << " pass "
                            << term.passN
                            << " time "
                            << term.timer.get_s()
                            << "| pass took "
                            << passTimer.get_s()
                            << " plan level: "
                            << leafLevel
                            << " | change "
                            << changeInPass
                            << "| chunk size "
                            << planChunkSize
                            << std::endl;


                        std::cout << "unchanged since " << term.sameCnt << " passes, change " << changeInPass << "\n";

                        timer_cout.restart();

                    }

                    updateLog();

                    anyChange |= changeInPass;
                    if (term(changeInPass))
                        break;
                }
            }

#if CHECK_IMPROVEMENT == 0
            qtLeafAssignment=best_qtLeafAssignment.first;
#endif
            return std::make_tuple(qtLeafAssignment, supertilesCost_(0), term, log, anyChange);
        }

        template<typename D_ST, distFuncType_t distFuncType, typename D_TILE, typename NodeLeafCounts, typename PO>
        auto runMe_(PO po)
        {
            std::vector <D_TILE> tileData;
            size_t nTilesFull;
            V2<int> tileDim;
            V2 <size_t> gridDim;

            std::tie(tileData, nTilesFull, tileDim, gridDim)
                = initData<D_TILE>(po);

            std::cout << "identified grid dim " << gridDim << std::endl;
            supertiles::place::Term<double> term(po);

            const size_t nElemsTile = helper::ii2n(tileDim);

            double cost = 0.;
            bool anyChange = false;
            if ((po.termTime > 0 && po.termIterations > 0) || po.getCost || is_initAndExit(po.loadAssignments)) {
                helper::ChronoTimer timer;

                const auto rslt = run_<
                    distFuncType,
                    NodeLeafCounts
                >(
                    tileData,
                    nTilesFull,
                    nElemsTile,
                    gridDim,
                    po.nNeighbors,
                    po.neighborFac,
                    term,
                    std::mt19937(po.seed),
                    po.planChunkSize,
                    po.nTilesAssign,
                    po.loadAssignments,
                    po.writeIntermediateAssignments,
                    po.batchSize,
                    po.maxNLevelsUp,
                    po.maxNodeExchangeLevel
                );

                const auto &qtLeafAssignment = std::get<0>(rslt);
                cost = std::get<1>(rslt);
                const auto &term = std::get<2>(rslt);
                const auto &log = std::get<3>(rslt);
                anyChange = std::get<4>(rslt);

                std::cout
                    << "cost run: "
                    << cost
                    << " "
                    << std::get<2>(rslt).sameCnt
                    << " | took "
                    << timer.get_s()
                    << std::endl;

                if ((term.maxNSeconds > 0 && term.maxNPasses > 0)
                    || is_initAndExit(po.loadAssignments)) {
                    helper::bzip_compress(qtLeafAssignment, po.outDir + "/qtLeafAssignment.raw.bz2");
                    const bool append = (po.loadAssignments != "");
                    log.writeCSV(po.outDir + "/log.csv", append);
                }

                if (po.writeGridAssignmentTXT != "") {
                    // convert qtLeafAssignment to as
                    const auto leaf2gridPos =
                        helper::invertMap(gridPos2QTLeaves(gridDim));


                    std::vector <uint32_t> as;

                    as.resize(helper::ii2n(gridDim));
                    for (const auto &i: helper::range_n(as.size()))
                        as[leaf2gridPos[i]] = qtLeafAssignment[i];

                    {
                        const auto grid2leaves = gridPos2QTLeaves(gridDim);
                        std::vector <uint32_t> qtLeafAssignment_test(as.size());
                        for (const auto &i: helper::range_n(as.size()))
                            qtLeafAssignment_test[grid2leaves[i]] = as[i];
                        assert(qtLeafAssignment_test == qtLeafAssignment);
                    }

                    std::cout << "writeGridAssignmentTXT to " << po.writeGridAssignmentTXT << std::endl;
                    helper::writeASCIIv(as, po.writeGridAssignmentTXT);
                }
            }

            //
            // presentation
            //
            if (po.imgOut) {

                assert(po.loadAssignments != "");
                const std::vector <uint32_t> qtLeafAssignment
                    = read_qtLeafAssignment(po.loadAssignments, gridDim);

                hassertm3(
                    qtLeafAssignment.size() == helper::ii2n(gridDim),
                    qtLeafAssignment.size(),
                    gridDim,
                    helper::ii2n(gridDim));

                supertiles_QuadTree<int64_t> qt(gridDim);

                std::vector <D_ST> supertiles;
                NodeLeafCounts nodeLeafCounts;

                std::tie(supertiles, nodeLeafCounts) =
                    initSupertiles<NodeLeafCounts, D_ST>
                        (qt, qtLeafAssignment, tileData, nElemsTile);

                std::vector<double> leafCosts;

                size_t nTilesAssign = po.nTilesAssign;
                if (nTilesAssign == -1)
                    nTilesAssign = nTilesFull;

                assert(nTilesAssign >= nTilesFull);

                using DIM = typename std::remove_cv<decltype(gridDim)>::type;

                const auto nodeDisparities
                    = computeNodeDisparity<distFuncType>
                        (supertiles.begin(), nElemsTile, qt, qtLeafAssignment, nodeLeafCounts);

                if (po.writeDisparitesOnly != "") {
                    helper::writeFileAuto(nodeDisparities, po.writeDisparitesOnly);
                } else if (po.repFNames.empty()) {
                    drawAdaptiveGrids<distFuncType>(
                        po.outDir,
                        gridDim, qtLeafAssignment,
                        tileData,
                        helper::ii2n(tileDim),
                        tileData,
                        tileDim,
                        nodeDisparities,
                        leafCosts,
                        po.repAggregationType,
                        po.outputImgScale,
                        po.drawDisparityIndicator,
                        po.imgOutOpts,
                        nTilesAssign
                    );

                } else {
                    auto _draw = [&](auto rep_tileData) {
                        using R =
                            typename std::remove_const<typename std::remove_reference<decltype(rep_tileData[0])>::type>::type;
                        assert(po.repFNames.size() == 1);


                        auto rep_ud = supertiles::place::readUserData(po.repFNames.front());
                        hassertm2(
                            typeid(R).hash_code() == std::get<0>(rep_ud.getType()),
                            typeid(R).hash_code(),
                            std::get<0>(rep_ud.getType()));

                        const auto rep_fileNameBuf = rep_ud.genFileNames();
                        assert(rep_fileNameBuf.size() == 1);

                        const DIM rep_tileDim(rep_ud._volDim);

                        std::cout << "rep_tileDim: " << rep_tileDim << std::endl;

                        switch (rep_ud._volumeFormat) {
                            case volformat_raw:
                                helper::readFile2(rep_tileData, rep_fileNameBuf.front());
                                break;
                            case volformat_raw_bz2:
                                helper::bzip_decompress(rep_tileData, rep_fileNameBuf.front());
                                break;
                            default:
                                assert(false);
                        }

                        std::cout << "rep_tile data elements: " << rep_tileData.size() << std::endl;

                        drawAdaptiveGrids<distFuncType>(
                            po.outDir,
                            gridDim,
                            qtLeafAssignment,
                            tileData,
                            nElemsTile,
                            rep_tileData,
                            rep_tileDim,
                            nodeDisparities,
                            leafCosts,
                            po.repAggregationType,
                            po.outputImgScale,
                            po.drawDisparityIndicator,
                            po.imgOutOpts,
                            nTilesAssign
                        );
                    };

                    if (po.repAggregationType == repAggregationType_central) {
                        std::vector <V4<uint8_t>> rep_tileData;
                        _draw(rep_tileData);
                    } else if (po.repAggregationType == repAggregationType_average_colRGB) {
                        std::vector <V3<float>> rep_tileData;
                        _draw(rep_tileData);
                    } else
                        assert(false);


                }
            }
            return anyChange ? cost : -cost;
        }

        template<typename D_ST, typename ARGV>
        double runT(int argc, const ARGV argv)
        {
            const auto po = initPO(argc, argv);

            const auto feat_ud = readUserData(po.configFNames.front());
            const size_t feat_hashCode = std::get<0>(feat_ud.getType());

            if (helper::hc_eq<double>(feat_hashCode) || helper::hc_eq<float>(feat_hashCode)) {
                auto run__ = [&](auto f) {
                    using D =
                    decltype(f);
                    using QT = supertiles_QuadTree<int64_t>;

                    if (po.distFuncType == distFuncType_norm2) {
                        return runMe_<D_ST, distFuncType_norm2, D, NodeLeafCounts_part<QT>>(po);
                    } else if (po.distFuncType == distFuncType_cosine_normalized) {
                        return runMe_<D_ST, distFuncType_cosine_normalized, D, NodeLeafCounts_part<QT>>(po);
                    } else {
                        std::cerr << "invalid distFuncType provided.\n";
                        assert(false);
                    }
                    return 0.;
                };

#if 0
                run__(double(0));
#else
                if (helper::hc_eq<double>(feat_hashCode))
                    return run__(double(0));
                else if (helper::hc_eq<float>(feat_hashCode))
                    return run__(float(0));
                else
                    assert(false);
#endif
            } else
                assert(false);
            return 0.;
        }

        template<typename ARGV>
        double run(int argc, const ARGV argv)
        {
            return runT<double>(argc, argv);
        }

        template<typename ARGV>
        double run_f32(int argc, const ARGV argv)
        {
            return runT<float>(argc, argv);
        }
    }
}

#endif //__SUPERTILES_PLACE__
