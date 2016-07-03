P_BASE := ${CURDIR}

P_INCLUDE_COMMON := ${P_BASE}/common
P_INCLUDE_GPIO := ${P_BASE}/gpio
P_INCLUDE_I2C := ${P_BASE}/i2c

CPP_FLAGS := -std=c++14 -ggdb -O0

INCS := -I $(P_INCLUDE_COMMON) \
        -I $(P_INCLUDE_GPIO)   \
        -I $(P_INCLUDE_I2C)    \

all: gpio_led gpio_i2c

gpio_led: gpio_led.cpp
	${CROSS_COMPILE}g++ $(CPP_FLAGS) $(INCS) -o $@ $^ -lpthread -ldl

gpio_i2c: gpio_i2c.cpp
	${CROSS_COMPILE}g++ $(CPP_FLAGS) $(INCS) -o $@ $^ -lpthread -ldl

clean:
	-rm -rf gpio_led
	-rm -rf gpio_i2c

