#ifndef I2C_DEVICE_H__
#define I2C_DEVICE_H__

#include <string>
#include <sstream>
#include <tuple>

#include "utility.h"

namespace i2c {

  class I2CDevice {
   
    unsigned m_bus;
    unsigned m_device;

    std::string m_read_cmd;
    std::string m_write_cmd;

    std::string m_mode;

    public:
      void Read(unsigned address, unsigned char &v) {
        std::string s = exec(std::string(m_read_cmd
              + to_hex(address) + m_mode).c_str());

        unsigned value;
        std::istringstream iss(s);
        iss >> std::hex >> value;

        v = value;
      }

      void Read(unsigned address, unsigned short &v) {
        std::string s = exec(std::string(m_read_cmd 
              + to_hex(address) + " w").c_str());

        std::istringstream iss(s);
        iss >> std::hex >> v;
      }

      void Write(unsigned address, unsigned char v) {
        std::string s = exec(std::string(m_write_cmd 
              + to_hex(address) + " " 
              + to_hex(v) + " b").c_str());
      }

      void Write(unsigned address, unsigned short v) {
        std::string s = exec(std::string(m_write_cmd 
              + to_hex(address) + " " 
              + to_hex(v) + " w").c_str());
      }

      std::tuple<unsigned, unsigned> GetInfo() {
        return std::make_tuple(m_bus, m_device);
      }

      void Mode(bool mode) {
        m_mode = std::string(mode ? " c" : " b");
      }

      I2CDevice(unsigned bus, unsigned device)
        : m_bus(bus), m_device(device), m_mode(" b") {
          m_read_cmd = std::string("i2cget -y " 
              + std::to_string(m_bus) + " " 
              + to_hex(m_device) + " ");

          m_write_cmd = std::string("i2cset -y " 
              + std::to_string(m_bus) + " " 
              + to_hex(m_device) + " ");
        }
  };

} /* i2c */

#endif /* I2C_DEVICE_H__ */
