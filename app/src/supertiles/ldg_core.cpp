#include "../../include/supertiles/ldg_core.hpp"

int main(int argc, const char **argv) {

#ifndef NO_OMP
    if (false)
        omp_set_num_threads(1);
#endif

    const std::string e(argv[0]);
    if (e.size() >= 3 && std::string("f32") == e.substr(e.size() - 3, 3)) {
        std::cout << "f32" << std::endl;
        supertiles::place::run_f32(argc, argv);
    } else {
        std::cout << "f64" << std::endl;
        supertiles::place::run(argc, argv);
    }
    return 0;
}

