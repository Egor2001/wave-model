#ifndef WAVE_MODEL_PARALLEL_ABSTRACT_NODE_H_
#define WAVE_MODEL_PARALLEL_ABSTRACT_NODE_H_

namespace wave_model {

class WmAbstractNode /* abstract */
{
public:
    WmAbstractNode() = default;

    virtual ~WmAbstractNode() = default;
    virtual void execute() = 0;
    virtual WmAbstractNode* proceed() const = 0;

protected:
    WmAbstractNode             (const WmAbstractNode&) = default;
    WmAbstractNode& operator = (const WmAbstractNode&) = default;

    WmAbstractNode             (WmAbstractNode&&) noexcept = default;
    WmAbstractNode& operator = (WmAbstractNode&&) noexcept = default;
};

} // namespace wave_model

#endif // WAVE_MODEL_PARALLEL_ABSTRACT_NODE_H_
