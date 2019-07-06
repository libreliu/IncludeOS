
#ifndef EMMC_HPP_INCLUDED
#define EMMC_HPP_INCLUDED

#include <string>
#include <hw/writable_blkdev.hpp>
#include <stdint.h>

namespace hw
{

class Rpi_Emmc : public Writable_Block_device
{
public:
    ~Rpi_Emmc();

    static Rpi_Emmc& get() noexcept;

    std::string device_name() const;

    const char *driver_name() const noexcept;

    block_t size() const noexcept;

    block_t block_size() const noexcept;

    void read(block_t blk, on_read_func reader)
    {
        read(blk, 1, std::move(reader));
    }

    void read(block_t blk, size_t count, on_read_func reader);

    buffer_t read_sync(block_t blk, size_t count);

    void write(block_t blk, buffer_t, on_write_func);

    bool write_sync(block_t blk, buffer_t);

    void deactivate() override;

protected:
    Rpi_Emmc() noexcept;

private:
    int id_;
};
}; // namespace hw

#endif