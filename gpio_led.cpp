#include <iostream>
#include <stdexcept>
#include <chrono>
#include <string>
#include <memory>
#include <tuple>
#include <list>
#include <vector>

#include "gpio_domain.h"

int main(int argc, char *argv[]) {

  gpio::GpioDomain gpio_domain;

  try {

    auto stub = [&](const std::string &gpio, gpio::State s) {};

    auto gpio = [&](const std::string &gpio, gpio::State s) {
      std::cerr << __PRETTY_FUNCTION__ << std::endl;
      std::cerr << "Gpio : " << gpio << '\n';
      std::cerr << "State: " <<
        (s == gpio::State::E_UP ? "Up" : "Down") << '\n';

      std::map<std::string, std::string> map = {
        { "button_1", "led_1" },
        { "button_2", "led_2" },
        { "button_3", "led_3" },
      };

      gpio_domain.SetState(map.at(gpio), s);
    };

    std::list<gpio::GpioDomain::Cfg> gpio_cfg = {
      { "button_1",  5, gpio },
      { "button_2",  6, gpio },
      { "button_3", 13, gpio },

      { "led_1", 26, stub, gpio::Direction::E_OUT },
      { "led_2", 12, stub, gpio::Direction::E_OUT },
      { "led_3", 16, stub, gpio::Direction::E_OUT },
      { "led_4", 20, stub, gpio::Direction::E_OUT },
    };

    gpio_domain.Init(gpio_cfg, 19, [&] { 
        std::cerr << __PRETTY_FUNCTION__ << std::endl;

        static bool flag = true;
        std::vector<gpio::State> s = { gpio::State::E_UP, 
                                       gpio::State::E_DOWN };

        gpio_domain.SetState("led_4", s[flag = !flag]);
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
