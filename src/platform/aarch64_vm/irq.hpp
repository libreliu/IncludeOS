
class IrqHandler
{
    uint32_t decode(uint32_t irq);
    virtual enable(uint32_t irq);
    virtual disable(uint32_t irq);
};
