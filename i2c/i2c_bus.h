#ifndef I2C_BUS_H__
#define I2C_BUS_H__

#include <functional>
#include <string>
#include <sstream>
#include <map>
#include <list>
#include <memory>
#include <algorithm>
#include <iterator>
#include <iostream>

#include "i2c_device.h"
#include "utility.h"

namespace i2c {

  enum class Event {
    E_NEW,
    E_REMOVED
  };
  
  using Notify = std::function<void(const Event, 
      const std::string&, const std::list<std::weak_ptr<I2CDevice>>&)>;

  using Range = std::pair<unsigned, unsigned>;
  
  class I2CBus {
  
    std::string m_bus_name;
    unsigned    m_bus;
    Notify      m_notify;
    std::string m_cmd;

    std::map<unsigned, 
      std::shared_ptr<I2CDevice>> m_device_db;
  
    public:
      I2CBus(std::string bus_name, unsigned int bus,
          Notify notify, Range range = {0x00, 0x77})
        : m_bus_name(bus_name), m_bus(bus), m_notify(notify) {
        m_cmd = std::string("i2cdetect -y " + std::to_string(m_bus)
            + " " + to_hex(range.first) + " " + to_hex(range.second));
      }

      void Scan() {
        std::list<unsigned> devs = GetI2CDevs();
  
        std::list<unsigned> devs_curr;
        std::transform(m_device_db.begin(), m_device_db.end(), 
            std::inserter(devs_curr, devs_curr.end()),
            [] (std::pair<const unsigned, std::shared_ptr<I2CDevice>> &p)
            -> unsigned { return p.first; }
        );
  
        devs.sort();
        devs_curr.sort();
  
        std::list<unsigned> devs_removed;
        std::set_difference(devs_curr.begin(), 
            devs_curr.end(), devs.begin(), devs.end(),
            std::inserter(devs_removed, devs_removed.end()));
  
        std::list<unsigned> devs_new;
        std::set_difference(devs.begin(), devs.end(), 
            devs_curr.begin(), devs_curr.end(),
            std::inserter(devs_new, devs_new.end()));
        
        std::list<std::weak_ptr<I2CDevice>> devs_notify_removed;
        for (auto&& e : devs_removed) {
          devs_notify_removed.push_back(m_device_db[e]);
        }

        if (!devs_notify_removed.empty()) {
          m_notify(Event::E_REMOVED, m_bus_name, devs_notify_removed);
        }

        for (auto&& e : devs_removed) {
          m_device_db.erase(e);
        }

        std::list<std::weak_ptr<I2CDevice>> devs_notify_new;
        for (auto&& e : devs_new) {
          auto sp = std::make_shared<I2CDevice>(m_bus, e);

          m_device_db.emplace(e, sp);
          devs_notify_new.push_back(sp);
        }

        if (!devs_notify_new.empty()) {
          m_notify(Event::E_NEW, m_bus_name, devs_notify_new);
        }
      }
  
    private:
      std::list<unsigned> GetI2CDevs() {
        std::string s = exec(m_cmd.c_str());
        std::list<unsigned> result;
  
        std::string line;
        std::istringstream iss(s);
        std::getline(iss, line);
  
        while (std::getline(iss, line)) {
  
          for (unsigned i = 0; i < 16; i++) {
            std::string e = line.substr(4 + (i * 3), 2);
  
            unsigned dev;
            std::istringstream iss(e);
            iss >> std::hex >> dev;
  
            if (iss) result.push_back(dev);
          }
        }

        return std::move(result);
      }
  };

} /* i2c */

#endif /* I2C_BUS_H__ */
