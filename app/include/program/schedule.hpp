#ifndef SCHEDULE_HPP
#define SCHEDULE_HPP

namespace program
{
    /**
     *  The schedule that should be run for the LDG implementation.
     *  This determines in which order the
     */
    struct Schedule
    {
        size_t number_of_passes;            // Number of ssm::sort calls.
        size_t max_iterations;              // Maximum number of iterations before the SSM should move to the next height.
        double iterations_change_factor;    // The factor by which to change the number of iterations after each pass.

        double distance_threshold;          // Minimum ratio of distance that should be changed before the SSM should move to the next height.
        double threshold_change_factor;     // The factor by which to change the distance threshold after each pass.

        size_t passes_per_checkpoint;       // Number of passes that should be passed before an intermediate checkpoint should be made.
        size_t iterations_per_checkpoint;   // Number of iterations within a height that should be passed before an intermediate checkpoint should be made.
    };
}// ldg

#endif //SCHEDULE_HPP