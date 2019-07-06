#include <hw/emmc.hpp>
#include <hw/sd.h>
#include <fs/common.hpp>

namespace hw
{
    using block_t = uint64_t;
    using buffer_t = os::mem::buf_ptr;

    Rpi_Emmc::Rpi_Emmc() noexcept
    {
        static int counter = 0;
        id_ = counter++;
        sd_init();
    }

    Rpi_Emmc& Rpi_Emmc::get() noexcept {
        static Rpi_Emmc r;
        return r;
    }

    Rpi_Emmc::~Rpi_Emmc() {}

    std::string Rpi_Emmc::device_name() const
    {
        return "emmc1";
    }

    const char *Rpi_Emmc::driver_name() const noexcept
    {
        return "emmc1";
    }

    block_t Rpi_Emmc::size() const noexcept
    {
        return (block_t)128 * 1024 * 1024 * 1024 / block_size();
    }

    block_t Rpi_Emmc::block_size() const noexcept
    {
        return 512;
    }

    void Rpi_Emmc::read(block_t blk, size_t count, on_read_func reader)
    {
        // TODO: Add real Async Read
        reader( read_sync(blk, count) );
    }

    buffer_t Rpi_Emmc::read_sync(block_t blk, size_t count = 1)
    {
        printf("Calling read_sync()\n");
        unsigned char *buf = (unsigned char *)malloc(count * block_size());
        if (sd_readblock(blk, buf, count) == 0) {
            throw "SD Card Read Error";
        }
        return fs::construct_buffer(buf, buf + count * block_size());
    }

    void Rpi_Emmc::write(block_t blk, buffer_t buf, on_write_func writer)
    {
        // TODO: Add real Async Write
        writer( write_sync(blk, buf));
    }

    bool Rpi_Emmc::write_sync(block_t blk, buffer_t buf)
    {
        if (buf->size() != 512) {
            throw "Invalid buffer size";
        }

        sd_writeblock(buf->data(), blk, 1);
    }

    void Rpi_Emmc::deactivate() {

    }

} // namespace hw