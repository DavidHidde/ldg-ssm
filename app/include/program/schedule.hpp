#ifndef SCHEDULE_HPP
#define SCHEDULE_HPP

namespace program
{
    /**
     *  The schedule that should be run for the LDG implementation.
     */
    struct Schedule
    {
        size_t number_of_passes;            // Number of ssm::sort calls.


        size_t passes_per_checkpoint;       // Number of passes that should be passed before an intermediate checkpoint should be made.
        size_t iterations_per_checkpoint;   // Number of iterations within a height that should be passed before an intermediate checkpoint should be made.
    };
}// ldg

#endif //SCHEDULE_HPP