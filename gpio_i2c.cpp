#include <iostream>
#include <stdexcept>
#include <chrono>
#include <string>
#include <memory>
#include <tuple>
#include <list>

#include "gpio_domain.h"
#include "i2c_domain.h"

int main(int argc, char *argv[]) {

  i2c::I2CDomain i2c_domain;
  gpio::GpioDomain gpio_domain;

  try {

    auto gpio = [&](const std::string &gpio, gpio::State s) {
      std::cerr << __PRETTY_FUNCTION__ << std::endl;
      std::cerr << "Gpio : " << gpio << '\n';
      std::cerr << "State: " <<
        (s == gpio::State::E_UP ? "Up" : "Down") << '\n';

      std::map<std::string, std::string> map = {
        { "button_1", "i2c_bus_1" },
        { "button_2", "i2c_bus_2" },
        { "button_3", "i2c_bus_3" },
      };

      i2c_domain.Scan(map.at(gpio));
    };

    auto i2c = [](const i2c::Event e, const std::string &bus,
        const std::list<std::weak_ptr<i2c::I2CDevice>> &devs) {

      std::cerr << __PRETTY_FUNCTION__ << std::endl;

      std::cerr << "Bus  : " << bus << "\n";
      std::cerr << "Event: " <<
        (e == i2c::Event::E_NEW ? "new" : "removed") << '\n';

      for (auto &&e : devs) {
        if (auto sp = e.lock()) {
          unsigned bus;
          unsigned dev;

          std::tie(bus, dev) = sp->GetInfo();
          std::cerr << "  -- Device: " << bus << '.' << dev << '\n';

          //std::string msg("Hello");
          //for (unsigned char e : msg) {
          //  sp->Write(0x00, e);
          //}
        }
      }

      std::cerr << "\n\n";
    };

    std::list<i2c::I2CDomain::Cfg> i2c_cfg = {
      { "i2c_bus_1", 1, i2c },
      { "i2c_bus_2", 2, i2c },
      { "i2c_bus_3", 3, i2c },
    };

    i2c_domain.Init(i2c_cfg);
    i2c_domain.Scan();

    std::list<gpio::GpioDomain::Cfg> gpio_cfg = {
      { "button_1",  5, gpio },
      { "button_2",  6, gpio },
      { "button_3", 13, gpio },
    };

    gpio_domain.Init(gpio_cfg, 19, [] { 
        std::cerr << __PRETTY_FUNCTION__ << std::endl;
        return true;
    });

    while (true) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }

  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
  } catch (...) {
    std::cerr << "Unexpected error!\n";
  }

  return 0;
} 
