#ifndef GPIO_H__
#define GPIO_H__

#include <iostream>
#include <string>
#include <functional>
#include <stdexcept>

namespace std {
  #include <stdio.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <poll.h>
}

namespace gpio {

  enum class Edge {
    E_RISING,
    E_FALLING,
    E_BOTH,
    E_NONE
  };

  enum class State {
    E_UP,
    E_DOWN
  };

  enum class Direction {
    E_IN,
    E_OUT,
  };

  class Gpio {
    int m_gpio_fd;
    unsigned m_gpio;

    public:
      Gpio(unsigned gpio, Direction dir = Direction::E_IN,
          Edge edge = Edge::E_NONE) : m_gpio(gpio) {
        Do("/sys/class/gpio/export");

        /* Sometimes it requires delay */
        std::usleep(100000);

        SetDir(dir);
        SetEdge(edge);

        std::string buf("/sys/class/gpio/gpio" +
          std::to_string(m_gpio) + "/value");

        m_gpio_fd = std::open(buf.c_str(), O_RDWR);
        if (m_gpio_fd < 0)
          throw std::runtime_error(
              std::string(__PRETTY_FUNCTION__)
              + ": Unable to open: " + buf);
      }

     ~Gpio() {
        std::close(m_gpio_fd);

        try { Do("/sys/class/gpio/unexport"); }
        catch (const std::exception &e) {
          std::cerr << e.what() << '\n';
        }
      }

      void SetState(State state) {

        std::lseek(m_gpio_fd, 0, SEEK_SET);
        std::string gpio_state(State::E_UP == state ? "1" : "0");
        int rc = std::write(m_gpio_fd, gpio_state.c_str(), gpio_state.length());
        if (rc < 0)
          throw std::runtime_error(
              std::string(__PRETTY_FUNCTION__)
              + ": Unable to write to GPIO: " + std::to_string(m_gpio));
      }

      State GetState() {
        char state;

        std::lseek(m_gpio_fd, 0, SEEK_SET);
        int rc = std::read(m_gpio_fd, &state, sizeof(state));
        if (rc < 0)
          throw std::runtime_error(
              std::string(__PRETTY_FUNCTION__)
              + ": Unable to get state of GPIO: " + std::to_string(m_gpio));

        if (state != '0') {
          return State::E_UP;
        } else {
          return State::E_DOWN;
        }
      }

      std::pair<bool, Edge> Irq() {
        struct std::pollfd fd_set = {
          .fd = m_gpio_fd,
          .events = POLLPRI
        };

        int rc = std::poll(&fd_set, 1, 0);
        if (rc >= 0) {
          if (fd_set.revents & POLLPRI) {
            char buf[64];

            std::lseek(fd_set.fd, 0, SEEK_SET);
            int len = std::read(fd_set.fd, &buf, sizeof(buf));

            return std::make_pair(true, (buf[len - 1] == '0')
                ? Edge::E_FALLING : Edge::E_RISING);
          }
        }

        return std::make_pair(false, Edge::E_FALLING);
      }

    private:
      void Do(const char* path) {
        int fd = std::open(path, O_WRONLY);
        if (fd < 0)
          throw std::runtime_error(
              std::string(__PRETTY_FUNCTION__)
              + ": Unable to open: " + path);

        std::string buf(std::to_string(m_gpio));
        std::write(fd, buf.c_str(), buf.length());
        std::close(fd);
      }

      void SetDir(Direction dir) {
        std::string buf("/sys/class/gpio/gpio" +
            std::to_string(m_gpio) + "/direction");

        int fd = std::open(buf.c_str(), O_WRONLY);
        if (fd < 0)
          throw std::runtime_error(
              std::string(__PRETTY_FUNCTION__)
              + ": Unable to open: " + buf);

        std::string direction(Direction::E_IN == dir ? "in" : "out");
        std::write(fd, direction.c_str(), direction.length());
        std::close(fd);
      }

      void SetEdge(Edge edge) {
        if (Edge::E_NONE != edge) {
          std::string buf("/sys/class/gpio/gpio" +
              std::to_string(m_gpio) + "/edge");
  
          int fd = std::open(buf.c_str(), O_WRONLY);
          if (fd < 0)
            throw std::runtime_error(
                std::string(__PRETTY_FUNCTION__)
                + ": Unable to open: " + buf);
  
          std::string gpio_edge;
          switch (edge) {
            case Edge::E_RISING: gpio_edge = "rising"; break;
            case Edge::E_FALLING: gpio_edge = "faling"; break;
            case Edge::E_BOTH:
            default:
              gpio_edge = "both";
          }
  
          std::write(fd, gpio_edge.c_str(), gpio_edge.length());
          std::close(fd);
        }
      }
  };

} /* gpio */

#endif /* GPIO_H__ */
