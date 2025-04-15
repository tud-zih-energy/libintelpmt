#include <libintelpmt/libintelpmt.hpp>

#include <algorithm>
#include <iostream>
#include <thread>

void usage() {
  std::cout << "USAGE: pmttool command [options]..." << std::endl;
  std::cout << std::endl;
  std::cout << "Available commands:" << std::endl;
  std::cout << "\tlist " << std::endl;
  std::cout << "\tread [device] [counter_name]" << std::endl;
}

int main(int argc, char **argv) {
  if (argc == 1) {
    usage();
    return 0;
  }

  std::string command = argv[1];
  if (command == "list") {
    auto devices = intelpmt::get_pmt_devices();
    std::cout << "List: " << std::endl;

    for (auto &dev : devices) {
      std::cout << std::endl;
      std::cout << dev->get_path().string() << std::hex << "("
                << dev->get_uniqueid() << "):" << std::endl;
      for (auto &ev : dev->get_counters()) {
        std::cout << "\t" << ev << std::endl;
      }
    }

  } else if (command == "read") {
    if (argc != 4) {
      std::cout << "ERROR: \"read\" requires exactly two arguments: "
                << std::endl;
      std::cout << std::endl;
      usage();
      return 0;
    }

    std::string device_path = argv[2];
    std::string counter = argv[3];
    auto devices = intelpmt::get_pmt_devices();
    auto dev =
        std::find_if(devices.begin(), devices.end(), [&device_path](auto& arg) {
          return arg->get_path() == device_path;
        });

    if (dev == devices.end()) {
      std::cout << "Unknown device: " << device_path << std::endl;
      return -1;
    }

    uint64_t counter_id = 0;
    try {
      counter_id = dev->get()->get_counter_id_by_name(counter);
    } catch (std::out_of_range &e) {
      std::cout << "Unknown counter: " << counter << std::endl;
      return -1;
    }

    intelpmt::DeviceInstance instance(*dev);

    std::cout << "Reading: " << counter
              << " every second until Ctrl+C is pressed" << std::endl;

    while (true) {
        double val = instance.read_counter(counter_id);
        if(dev->get()->get_unit_by_id(counter_id).unit == "enum")
        {   
            std::cout << dev->get()->get_unit_by_id(counter_id).print_function(val) << std::endl;
        }
        else
        {
          std::cout << val << " " << dev->get()->get_unit_by_id(counter_id).unit
                    << std::endl;
        }
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;

  }

  else {
    std::cout << "Unknown command: " << command << std::endl;
    return -1;
  }
}
