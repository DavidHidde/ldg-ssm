#ifndef __SUPERTILES_PLACE_UC_MCMC__
#define __SUPERTILES_PLACE_UC_MCMC__


#include "helper_color/cm_map.h"
#include "helper_color/cm_bivariate.h"

#ifdef USE_STRAX_MCMC
#include "strax/strax_mcmc.h"

using strax_mcmc_opts_t=strax_mcmc_opts<mcmc2col<double>,mcmc2height>;

#else

struct strax_mcmc_opts_t
{
};

#endif // USE_STRAX_MCMC

template<bool use_omp>
std::vector <V4<uint8_t>> strax_mcmc(std::vector <V4<uint8_t>> data, V2<int>, strax_mcmc_opts_t, std::mt19937)
{
    return data;
}

template<bool use_omp>
std::vector <V4<uint8_t>>
strax_mcmc(std::vector <V4<uint8_t>> data, V2<unsigned long>, strax_mcmc_opts_t, std::mt19937)
{
    return data;
}

template<bool use_omp>
std::vector <V4<uint8_t>> strax_mcmc(std::vector <V3<float>> data, V2<unsigned long>, strax_mcmc_opts_t, std::mt19937)
{
    assert(false);
    return std::vector < V4 < uint8_t >> ();
}

template<bool use_omp>
std::vector <V4<uint8_t>> strax_mcmc(std::vector<float> data, V2<int>, strax_mcmc_opts_t, std::mt19937)
{
    assert(false);
    return std::vector < V4 < uint8_t >> ();
}

namespace supertiles
{
    namespace place
    {


        // A generic sort function
        template<class T>
        T mcmc_binarize(T v)
        {
            return v;
        }

        // Template Specialization: A function
        // specialized for char data type
        template<>
        double mcmc_binarize<double>(double v)
        {
            return v > 0.5;
        }

        template<typename F>
        std::vector <V4<double>> mcmc2col_f(const F *feat, const size_t nElems, const uint32_t colMapMode)
        {
            std::vector <V3<double>> cm_mcmc(256);

            V3<double>
                a = V3<double>(-1., -1., -1.),
                b = V3<double>(-1., -1., -1.),
                c = V3<double>(-1., -1., -1.);

            const bool do_bivariateChannel = (colMapMode >= 10);

            switch (colMapMode) {
                case 0:
                    //brown
                    a = V3<double>
                        (.54, .4, .4);


                    // pinkish red
                    b = V3<double>
                        (1., 1., 1.);

                    // blue
                    c = V3<double>
                        (.62, .79, 1);
                    break;
                case 10:
                    // gray
                    a = V3<double>
                        (.4, .4, .4);
                    b = V3<double>
                        (1., 1., 1.);
                    break;
                case 1:
                case 11:
                    a = c = V3<double>(1., 1., 1.);
                    b = V3<double>(.61, .0, .42);
                    break;
                case 2:
                case 12:
                    a = V3<double>(.54, .4, .4);
                    b = V3<double>(.61, .0, .42);
                    c = V3<double>(.62, .79, 1);
                    break;
                case 3:
                    a = V3<double>
                        (.54, .4, .4);


                    b = V3<double>
                        (0., 0., 0.);

                    // blue
                    c = V3<double>
                        (.62, .79, 1);
                    break;
                default:
                    std::cerr << "UNSUPPORTED MCMC COL MAP MODE " << colMapMode << std::endl;
                    exit(-1);
                    break;
            };


            auto _createColMap = [&cm_mcmc](const auto a, const auto b, const auto c) {
                const size_t h = cm_mcmc.size() / 2;
                for (
                    const auto &i: helper::range_n(cm_mcmc.size())) {
                    auto &o = cm_mcmc[i];
                    if (a.x >= 0.) {
                        if (i < h)
                            o =
                                (i * b + (h - i - 1) * a)
                                    / static_cast<double>(h - 1);
                        else
                            o =
                                ((i - h) * c + (cm_mcmc.size() - i - 1) * b)
                                    / static_cast<double>(h - 1);
                    } else
                        o = V3<double>(i, i, i) / (cm_mcmc.size() - 1);
                }
            };

            _createColMap(a, b, c);

            std::vector <V4<double>> rep;
            rep.reserve(nElems);

            V2 <uint32_t> dim;
            dim.x = std::sqrt(nElems) + .5;
            dim.y = dim.x;

            std::cout << "mcmc2col_f nElems " << nElems << " dim.x " << dim.x << " dim.y " << dim.y << std::endl;

            const auto biColMap = cm_bi_josh < V3 < double >> (dim.x);

            // the assumption is that mcmc images should always be square
            assert(dim.x * dim.y == nElems);
            for (
                const auto i: helper::range_n(nElems)) {

                if (do_bivariateChannel) {
                    assert(i < biColMap.size());
                    _createColMap(a, b, biColMap[biColMap.size() - 1 - i]);
                }
                const auto e = feat[i];
                hassertm(e >= -0.0001 && e <= 1.0001, e);
                const auto col = cm_map_norm(e, cm_mcmc);
                rep.push_back(V4<double>(col.x, col.y, col.z, 1.));
            }
            return rep;
        }

        template<typename IT>
        std::vector <V4<double>> mcmc2col(IT /*feat*/, const size_t /*nElems*/, const uint32_t)
        {
            hassertm(false, "mcmc2col does not support this type");
            return std::vector < V4 < double >> ();
        }

        template<>
        std::vector <V4<double>> mcmc2col<double *>(double *feat, const size_t nElems, const uint32_t colMapMode)
        {
            return mcmc2col_f(feat, nElems, colMapMode);
        }

        template<>
        std::vector <V4<double>>
        mcmc2col<const double *>(const double *feat, const size_t nElems, const uint32_t colMapMode)
        {
            return mcmc2col_f(feat, nElems, colMapMode);
        }

        auto normRange(const size_t n)
        {
            std::vector<double> feat(n);

            for (
                auto it = feat.begin(); it != feat.end(); it++
                )
                *it = static_cast<double>
                (it - feat.begin())
                    / (feat.size() - 1);
            return feat;
        }

        void mcmc_cm(const std::string fname, const uint32_t colMapMode)
        {
            using CairoDraw_t = helper::CairoDraw <V2<double>>;
            CairoDraw_t cd(helper::cairoBackend_rec);
            const auto feat = normRange(128);

            const std::vector <V4<double>> cm =
                mcmc2col(&feat[0], feat.size(), colMapMode);

            const V2<double> pos(0, 0);
            const V2<double> dim(400, 40);
            const auto posTo = pos + dim;

            cairo_pattern_t *pat;
            //Create a new linear gradient cairo_pattern_t along the line defined by (x0, y0) and (x1, y1). Before using the gradient pattern, a number of color stops should be defined using cairo_pattern_add_color_stop_rgb() or cairo_pattern_add_color_stop_rgba().
            pat = cairo_pattern_create_linear(pos.x, pos.y, posTo.x, /*posTo.y*/pos.y);

            assert(cm.size() > 1);
            for (
                size_t i = 0; i < cm.size(); i += 4
                )
                cairo_pattern_add_color_stop_rgba
                    (
                        pat,
                        static_cast<double>(i) / (cm.size() - 1),
                        cm[i].x, cm[i].y, cm[i].z, 1.
                    );

            cairo_pattern_add_color_stop_rgba
                (pat, 1, cm.back().x, cm.back().y, cm.back().z, 1.);

            auto cr = cd.get();
            cairo_rectangle(cr, pos.x, pos.y, dim.x, dim.y);

            cairo_set_source(cr, pat);
            cairo_fill(cr);
            cairo_pattern_destroy(pat);

            const std::string o = fname + "_" + std::to_string(colMapMode) + ".pdf";
            std::cout << "write mcmc col map mode " << colMapMode << " to " << o << std::endl;
            cd.writePDF(o);
        }

        auto feat2rep_rgb(const std::vector<double> &feat)
        {
            // data shoudl be given as rgb
            assert(feat.size() % 3 == 0);

            std::vector <V4<double>> rep(feat.size() / 3);
            for (
                const auto &i: helper::range_n(rep.size())) {
                rep[i] = V4<double>(i * 3, i * 3 + 1, i * 3 + 2, 1);
            }
            return rep;
        }

        template<typename IT>
        std::vector <V4<double>> caltech2col(IT, const size_t)
        {
            throw "caltech2col does not support this type";
            return std::vector < V4 < double >> ();
        }

        template<typename IT>
        std::vector <V4<double>> colRGB2col(IT feat, const size_t)
        {
            hassertm(false, typeid(*feat).name());
            return std::vector < V4 < double >> ();
        }

        template<typename _DIM>
        auto chartPos_x(const size_t ix, const size_t n, const _DIM chartBoxDim)
        {
            const V2<double> pos(0, 0);
            return
                pos.x
                    + (chartBoxDim.x * ix) / (n - 1);
        }

        template<typename _DIM>
        auto chartPos_y(const double vy, const _DIM chartBoxDim)
        {
            const V2<double> pos(0, 0);
            return
                pos.y
                    + (1. - vy)
                    * chartBoxDim.y;
        }

        template<typename DATA, typename _DIM, typename CM, typename OPTS>
        void drawSignal(
            cairo_t *cr,
            const DATA data,
            const size_t n,
            const _DIM chartBoxDim,
            const CM &colMap,
            const OPTS &opts,
            double ref = std::numeric_limits<double>::max())
        {
            const double scaleFac = 0.05;

            cairo_set_line_width
                (cr, scaleFac * chartBoxDim.x * opts.chartLineWidthScale);

            //
            // draw signal
            //

            if (ref == std::numeric_limits<double>::max())
                ref = static_cast<double>(data[0]);
            const auto dev = std::max(ref, 1. - ref);

            for (const auto &j: helper::range_be(1, n)) {
                cairo_move_to(
                    cr, chartPos_x(j - 1, n, chartBoxDim),
                    chartPos_y(data[j - 1], chartBoxDim));
                cairo_line_to(
                    cr, chartPos_x(j, n, chartBoxDim),
                    chartPos_y(data[j], chartBoxDim));

                auto v = data[j];

                const auto m_delta = 0.0001;
                hassertm(v >= -m_delta && v < 1 + m_delta, v);

                const auto d = (v - ref) / dev;
                v = std::sqrt(std::abs(d));

                cairo_set_line_width
                    (cr, v * scaleFac * chartBoxDim.x * opts.chartLineWidthScale);

                if (d > 0.)
                    v *= -1.;

                v = 0.5 + v * 0.5;
                hassertm(v >= -m_delta && v < 1 + m_delta, v);
                const auto col = cm_map_norm(v, colMap);


                cairo_set_source_rgba(cr, col.x, col.y, col.z, 1.);
                cairo_stroke(cr);
            }

        }


        //
        // draw range of curves
        //
        template<typename TILE_IDS, typename FTD, typename CBD, typename CM, typename OPTS>
        void drawSignalRange(
            cairo_t *cr,
            const TILE_IDS &tileIds,
            const FTD &feat_tileData,
            const size_t nElemsFeatTile,
            const CBD &chartBoxDim,
            const CM &colMap, const OPTS &opts
        )
        {

            std::vector<double>
                minv(
                nElemsFeatTile,
                std::numeric_limits<double>::max());

            std::vector<double>
                maxv(
                nElemsFeatTile,
                std::numeric_limits<double>::lowest());


            ///////////////////////////////////
            cairo_pattern_t *pat;
            {

                const auto &cm = colMap;

                //Create a new linear gradient cairo_pattern_t along the line defined by (x0, y0) and (x1, y1). Before using the gradient pattern, a number of color stops should be defined using cairo_pattern_add_color_stop_rgb() or cairo_pattern_add_color_stop_rgba().
                const auto x = chartPos_x(0, nElemsFeatTile, chartBoxDim);


                pat = cairo_pattern_create_linear
                    (
                        x, chartPos_y(0, chartBoxDim),
                        x, chartPos_y(1., chartBoxDim));

                assert(cm.size() > 1);
                for (size_t i = 0; i < cm.size(); i += 4)
                    cairo_pattern_add_color_stop_rgba
                        (
                            pat,
                            static_cast<double>(cm.size() - 1 - i) / (cm.size() - 1),
                            cm[i].x, cm[i].y, cm[i].z, 1.
                        );

                cairo_pattern_add_color_stop_rgba
                    (pat, 1, cm.back().x, cm.back().y, cm.back().z, 1.);
            }

            cairo_set_source(cr, pat);

            for (const auto &i: helper::range_n(nElemsFeatTile)) {
                for (const auto tileId: tileIds) {
                    const double v = feat_tileData[tileId * nElemsFeatTile + i];
                    minv[i] = std::min(v, minv[i]);
                    maxv[i] = std::max(v, maxv[i]);

                }
                auto &a = minv[i];
                auto &b = maxv[i];

                const double minw = 0.1 * opts.chartLineWidthScale;

                if (b - a < minw) {
                    const auto m = (a + b) / 2.;
                    a = m - .5 * minw;
                    b = m + .5 * minw;
                }
            }



            //
            // actual drawing
            //
            cairo_move_to(cr, chartPos_x(0, nElemsFeatTile, chartBoxDim), chartPos_y(minv[0], chartBoxDim));
            for (
                const auto &j: helper::range_be(1, nElemsFeatTile))
                cairo_line_to(
                    cr, chartPos_x(j, nElemsFeatTile, chartBoxDim),
                    chartPos_y(minv[j], chartBoxDim));

            auto idcs = helper::range_n(nElemsFeatTile);
            std::reverse(idcs.begin(), idcs.end());

            for (const auto j: idcs)
                cairo_line_to(cr, chartPos_x(j, nElemsFeatTile, chartBoxDim), chartPos_y(maxv[j], chartBoxDim));

            cairo_close_path(cr);

            if (true)
                cairo_fill(cr);
            else {
                cairo_fill_preserve(cr);

                cairo_set_line_width(cr, 0.05 * chartBoxDim.x);
                cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 0.3);
                cairo_stroke(cr);
            }
            // apply and clear the pattern
            cairo_pattern_destroy(pat);
        }

        template<typename _DIM>
        void drawGrid(cairo_t *cr, const size_t n, const _DIM chartBoxDim, double scaleFac = 0.005)
        {
            cairo_set_source_rgba(cr, 0., 0, 0., 1.);
            cairo_set_line_width(cr, scaleFac * chartBoxDim.x);
            const size_t nLines = 8;
            for (
                const auto &y: helper::range_be(1, nLines)) {
                const auto vy = static_cast<double>(y) / nLines;

                const auto ypos = chartPos_y(vy, chartBoxDim);
                cairo_move_to(
                    cr, chartPos_x(0, n, chartBoxDim),
                    ypos
                );
                cairo_line_to(
                    cr, chartPos_x(n - 1, n, chartBoxDim),
                    ypos
                );
                cairo_stroke(cr);
            }
        }

        template<typename TILE_IDS, typename FTD, typename CBD, typename CM, typename OPTS>
        void drawSignalAll(
            cairo_t *cr,
            const TILE_IDS &tileIds,
            const FTD &feat_tileData,
            const size_t nElemsFeatTile,
            const CBD &chartBoxDim,
            const CM &colMap, const OPTS &opts
        )
        {
            //
            // draw box
            //


            cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

            // signal mode
            cairo_set_line_width(cr, chartBoxDim.x / 100.);
            cairo_rectangle(cr, 0., 0., chartBoxDim.x, chartBoxDim.y);
            cairo_set_source_rgba(cr, .0, .0, .0, 1.);
            cairo_stroke(cr);

            if (false) {
                cairo_set_line_width(cr, chartBoxDim.x / 10.);
                cairo_set_source_rgba(cr, .2, .2, .8, 1.);
                cairo_move_to(cr, 0., 0.);
                cairo_line_to(cr, chartBoxDim.x, chartBoxDim.y);
                cairo_stroke(cr);
            }
            
            //
            // draw hlines for orientation
            //
            drawGrid(cr, nElemsFeatTile, chartBoxDim);


            drawSignalRange(
                cr, tileIds, feat_tileData, nElemsFeatTile,
                chartBoxDim, colMap, opts
            );
        }

        template<typename CM, typename OPTS>
        void stock_cm(const std::string fname, const CM &colMap, const OPTS &opts)
        {
            using CairoDraw_t = helper::CairoDraw <V2<double>>;
            CairoDraw_t cd(helper::cairoBackend_rec);

            const auto feat = normRange(128);

            cairo_set_line_cap(cd.get(), CAIRO_LINE_CAP_ROUND);
            drawSignal(cd.get(), feat, feat.size(), V2<double>(512, 0.0001), colMap, opts, 0.5);

            const std::string o = fname + ".pdf";
            std::cout << "write stock col map to " << o << std::endl;
            cd.writePDF(o);
        }

    }
};

#endif //__SUPERTILES_PLACE_UC_MCMC__
