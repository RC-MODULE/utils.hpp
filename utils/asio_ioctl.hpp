#ifndef __asio_ioctl_hpp__baf5c87b_c6a2_4781_b9e7_ee8537bdf323__
#define __asio_ioctl_hpp__baf5c87b_c6a2_4781_b9e7_ee8537bdf323__

#include <asio.hpp>

namespace utils {

template<int Code, typename Data>
struct ioctl_write_buffer {
  int operator()(int fd) const { return ::ioctl(fd, Code, &data); }
  Data data;
};

template<int Code, typename Data>
struct ioctl_write_buffer<Code, std::reference_wrapper<Data>> {
  int operator()(int fd) const { return ::ioctl(fd, Code, &data.get()); }
  std::reference_wrapper<Data> data;
};

template<int Code>
struct ioctl_write_buffer<Code, void> {
  int operator()(int fd) const { return ::ioctl(fd, Code); }
};

template<int Code, typename Data>
auto make_ioctl_write_buffer(Data&& d) -> ioctl_write_buffer<Code, typename std::decay<Data>::type> { return {std::forward<Data>(d)}; }

template<int Code>
ioctl_write_buffer<Code, void> make_ioctl_write_buffer() { return ioctl_write_buffer<Code, void>{}; }

}

namespace asio { namespace detail {

template<int Code, typename Data>
class descriptor_write_op_base<utils::ioctl_write_buffer<Code, Data>> : public ::asio::detail::reactor_op {
public:
  descriptor_write_op_base(int descriptor,
      utils::ioctl_write_buffer<Code, Data> const& buffer, func_type complete_func)
    : reactor_op(&descriptor_write_op_base::do_perform, complete_func),
      descriptor(descriptor),
      buffer(buffer)
  {}

  static bool do_perform(reactor_op* base) {
    descriptor_write_op_base* o(static_cast<descriptor_write_op_base*>(base));

    o->ec_ = asio::error_code();

    if(o->buffer(o->descriptor)) o->ec_ = asio::error_code(errno, asio::error::get_system_category());

    if(!o->ec_)
      o->bytes_transferred_ = sizeof(std::uint32_t);

    return o->ec_ != error::would_block;
  }
private:
  int descriptor;
  utils::ioctl_write_buffer<Code, Data> buffer;
};

template<int Code, typename Data>
class buffer_sequence_adapter<const_buffer, utils::ioctl_write_buffer<Code, Data>> {
public:
  static bool all_empty(utils::ioctl_write_buffer<Code,Data> const&) { return false; }
};

}}

#endif
