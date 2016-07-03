#ifndef I2C_DOMAIN_H__
#define I2C_DOMAIN_H__

#include <map>
#include <list>
#include <memory>

#include "i2c_bus.h"

namespace i2c {

  class I2CDomain {

    std::map<std::string, 
      std::unique_ptr<I2CBus>> m_bus_db;
  
    public:
      struct Cfg {
        std::string bus_name;
        unsigned    bus;
        Notify      notify;
        Range       range;
      };

      void Init(const std::list<Cfg> &cfg) {
        for (auto&& e : cfg) {
          m_bus_db.emplace(e.bus_name, 
              std::make_unique<I2CBus>(e.bus_name,
                e.bus, e.notify, e.range));
        }
      }
    
      void Scan(const std::string &bus_name) {
        m_bus_db[bus_name]->Scan();
      }

      void Scan() {
        for (auto&& e : m_bus_db) {
          e.second->Scan();
        }
      }
  };

} /* i2c */

#endif /* I2C_DOMAIN_H__ */
