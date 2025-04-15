#include <cassert>
#include <cstdint>
#include <vector>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <array>
#include <stdexcept>
#include <string>

extern "C" {
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
}

#pragma once

namespace intelpmt {


class DeviceInstance;
typedef double (*counter_read_f)(DeviceInstance &, uint64_t input1, uint64_t input2);

struct Sample {
  uint64_t offset; // in _bit_
  uint64_t size;   // in _bit_
};

struct Unit {
    const char * unit;
    const char * (*print_function)(double);
};

struct Counter {
  counter_read_f read_function;
  std::array<uint64_t, 2> sensors;
};
class Device {
public:
  Device(std::filesystem::path path) : path_(path) {}


  virtual uint64_t get_counter_id_by_name(const std::string &name)  = 0;

  virtual const Counter& get_counter_by_id(uint64_t id) = 0;

  virtual const Sample& get_sample_by_id(uint64_t id) = 0;

  virtual std::vector<std::string> get_counters() = 0;
  virtual const struct Unit& get_unit_by_id(uint64_t id) = 0;

  const std::filesystem::path get_path() const { return path_; }
  
  const uint64_t get_uniqueid()
  {
      return uniqueid_;
  }


protected:
  std::filesystem::path path_;
  uint64_t uniqueid_;
};

class DeviceInstance {
public:
  DeviceInstance(std::unique_ptr<Device> &device) : device_(device) {
    fd_ = open((device_->get_path() / "telem").c_str(), O_RDONLY);

    if (fd_ == -1) {
      throw std::system_error(errno, std::system_category(), strerror(errno));
    }
    std::ifstream size_stream(device_->get_path() / "size");
    size_stream >> buf_size_;

    if (buf_size_ == 0) {
      throw std::runtime_error(std::string("Could not read ") +
                               (device_->get_path() / "size").string());
    }

    buf_ = (char *)mmap(NULL, buf_size_, PROT_READ, MAP_SHARED, fd_, 0);
    if (buf_ == MAP_FAILED) {
      throw std::system_error(errno, std::system_category(), strerror(errno));
    }
  }

  DeviceInstance(DeviceInstance &other) = delete;
  DeviceInstance &operator=(DeviceInstance &other) = delete;

  DeviceInstance(DeviceInstance &&other) : device_(other.device_) {
    buf_ = std::move(other.buf_);
    buf_size_ = std::move(other.buf_size_);
    std::swap(fd_, other.fd_);

    other.buf_ = nullptr;
  }

  DeviceInstance &operator=(DeviceInstance &&other) {
    if (buf_ != nullptr) {
      munmap(buf_, buf_size_);
      buf_ = nullptr;
    }

    if (fd_ != -1) {
      close(fd_);
      fd_ = -1;
    }

    std::swap(device_, other.device_);

    buf_ = std::move(other.buf_);
    buf_size_ = std::move(other.buf_size_);

    std::swap(fd_, other.fd_);

    other.buf_ = nullptr;

    return *this;
  }

  ~DeviceInstance() {
    if (buf_ != nullptr) {
      munmap(buf_, buf_size_);
      buf_ = nullptr;
    }

    if (fd_ != -1) {
      close(fd_);
      fd_ = -1;
    }
  }

  const std::unique_ptr<Device> &get_device() const { return device_; }

  uint64_t read_sample(uint64_t event) {
    uint64_t res;

    Sample sample = device_->get_sample_by_id(event);

    // Assumption of this code: Intel packs the values weirdly inside
    // 64-bit words, but values do _not_ span multiple words.
    // This assertion protects this assumption
    assert((sample.offset % 8) + sample.size <= 64);
    size_t byte_size = 0;

    if (sample.size % 8 == 0) {
      byte_size = sample.size / 8;
    } else {
      byte_size = (sample.size / 8) + 1;
    }

    memcpy(&res, buf_ + (sample.offset / 8), byte_size);

    res >>= sample.offset % 8;
    uint64_t mask = (1ULL << sample.size) - 1;
    return res & mask;
  }

  double read_counter(uint64_t counter_id) {
    const struct Counter counter = device_->get_counter_by_id(counter_id);
    return counter.read_function(*this, counter.sensors[0], counter.sensors[1]);
  }

private:
  std::unique_ptr<Device> &device_;

  char *buf_ = nullptr;
  size_t buf_size_ = 0;
  int fd_ = -1;
};
} // namespace intelpmt
