#ifndef GPIO_DOMAIN_H__
#define GPIO_DOMAIN_H__

#include <string>
#include <functional>
#include <thread>
#include <list>
#include <map>
#include "gpio.h"

namespace gpio {

  using Notify = 
    std::function<void(const std::string&, const State)>;

  using Predicate = std::function<bool()>;

  class GpioDomain {
    bool m_run;
    std::thread m_thread;
    std::unique_ptr<Gpio> m_irq_gpio;

    std::map<std::string, 
      std::pair<std::unique_ptr<Gpio>, Notify>> m_gpio_db;

    Predicate m_predicate;
    
    public:
      struct Cfg {
        std::string gpio_name;
        unsigned    gpio;
        Notify      notify = Notify();
        Direction   dir = Direction::E_IN;
      };
   
      void Init(const std::list<Cfg> &cfg,
          unsigned irq_gpio = 0, Predicate p = [](){ return true; }) {
        for (auto&& e : cfg) {
          m_gpio_db.emplace(e.gpio_name, 
               std::make_pair(std::make_unique<Gpio>(e.gpio, e.dir), e.notify));
        }

        if (irq_gpio) {
          m_irq_gpio = std::move(std::make_unique<Gpio>(irq_gpio,
                Direction::E_IN, Edge::E_BOTH));
          m_thread = std::move(std::thread(&GpioDomain::WaitForIrq, this));
        }

        m_predicate = p;
      }

      void WaitForIrq() {
        std::map<std::string, State> gpio_state_prev;
        std::map<std::string, State> gpio_state;

        for (auto&& e : m_gpio_db) {
          gpio_state_prev.emplace(e.first, e.second.first->GetState());
        }

        while (m_run) {
          if (!m_irq_gpio->Irq().first) continue;
          if (!m_predicate()) continue;

          for (auto&& e : m_gpio_db) {
            gpio_state.emplace(e.first, e.second.first->GetState());
          }
          
          for (auto&& e : gpio_state_prev) {
            if (e.second != gpio_state[e.first]) {
              m_gpio_db[e.first].second(e.first, gpio_state[e.first]);
            }
          }

          gpio_state_prev = std::move(gpio_state);
        }
      }

      void SetState(const std::string &gpio_name, State state) {
        m_gpio_db[gpio_name].first->SetState(state);
      }

      State GetState(const std::string &gpio_name) {
        return m_gpio_db[gpio_name].first->GetState();
      }
  
      GpioDomain() : m_run(true) {}

     ~GpioDomain() {
        m_run = false;

        if (m_thread.joinable()) {
          m_thread.join();
        }
      }
  };

} /* gpio */

#endif /* GPIO_DOMAIN_H__ */
