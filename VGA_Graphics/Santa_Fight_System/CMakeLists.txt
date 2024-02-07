add_executable(santa_fight_system)

# must match with pio filename and executable name from above
pico_generate_pio_header(santa_fight_system ${CMAKE_CURRENT_LIST_DIR}/hsync.pio)
pico_generate_pio_header(santa_fight_system ${CMAKE_CURRENT_LIST_DIR}/vsync.pio)
pico_generate_pio_header(santa_fight_system ${CMAKE_CURRENT_LIST_DIR}/rgb.pio)

# must match with executable name and source file names
target_sources(santa_fight_system PRIVATE santa_fight.c vga16_graphics.c)

# must match with executable name
target_link_libraries(santa_fight_system PRIVATE pico_stdlib hardware_pio hardware_dma hardware_adc hardware_irq)

pico_enable_stdio_usb(santa_fight_system 1)
pico_enable_stdio_uart(santa_fight_system 0)

# must match with executable name
pico_add_extra_outputs(santa_fight_system)
