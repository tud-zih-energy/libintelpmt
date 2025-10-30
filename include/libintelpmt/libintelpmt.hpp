#include <libintelpmt/device.hpp>

#include <vector>
#include <memory>
namespace intelpmt
{
std::vector <std::unique_ptr<Device>> get_pmt_devices();
}
