#ifndef LDG_CORE_ROW_MAJOR_ITERATOR_HPP
#define LDG_CORE_ROW_MAJOR_ITERATOR_HPP

#include <vector>
#include <cstddef>
#include <memory>

namespace shared
{
    /**
     * Simple iterator for iterating over row-major arrays. in the quad tree
     * @tparam DataType Type of the underlying data.
     */
    template<typename DataType>
    class RowMajorIterator
    {
        size_t grid_num_cols;
        size_t projected_num_rows;
        size_t projected_num_cols;
        size_t offset;
        std::shared_ptr<std::vector<DataType>> data;
        std::shared_ptr<std::vector<size_t>> assignment;
        size_t index;

        size_t currentIndex();

    public:
        RowMajorIterator(
            size_t grid_num_cols,
            size_t projected_num_rows,
            size_t projected_num_cols,
            size_t offset,
            const std::shared_ptr<std::vector<DataType>> &data,
            const std::shared_ptr<std::vector<size_t>> &assignment,
            size_t index
        );

        RowMajorIterator begin();

        RowMajorIterator end();

        DataType &getValue();

        bool operator==(RowMajorIterator const &rhs);

        RowMajorIterator &operator++();
    };

} // shared

#endif //LDG_CORE_ROW_MAJOR_ITERATOR_HPP
