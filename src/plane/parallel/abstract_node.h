#ifndef WAVE_MODEL_PARALLEL_ABSTRACT_NODE_H_
#define WAVE_MODEL_PARALLEL_ABSTRACT_NODE_H_

/**
 * @file
 * @author Egor Elchinov <elchinov.es@gmail.com>
 * @version 2.0
 */

/// @brief
namespace wave_model {

/**
 * @brief Abstact class describing node of a grid
 * One grid node is guaranteed to execute on single thread/core/unit.
 * As a good practice, it's better to have all external 
 * synchronization with other nodes implemented out of the node class.
 */
class WmAbstractNode /* abstract */
{
public:
    WmAbstractNode() = default;

    virtual ~WmAbstractNode() = default;

    /**
     * @brief Forces node to execute.
     * Expects to be non-blocking by itself;
     */
    virtual void execute() = 0;

protected:
    WmAbstractNode             (const WmAbstractNode&) = default;
    WmAbstractNode& operator = (const WmAbstractNode&) = default;

    WmAbstractNode             (WmAbstractNode&&) noexcept = default;
    WmAbstractNode& operator = (WmAbstractNode&&) noexcept = default;
};

} // namespace wave_model

#endif // WAVE_MODEL_PARALLEL_ABSTRACT_NODE_H_
