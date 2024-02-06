add_executable(santa_fight_system)

# must match with pio filename and executable name from above
pico_generate_pio_header(santa_fight_system ${CMAKE_CURRENT_LIST_DIR}/hsync.pio)
pico_generate_pio_header(santa_fight_system ${CMAKE_CURRENT_LIST_DIR}/vsync.pio)
pico_generate_pio_header(santa_fight_system ${CMAKE_CURRENT_LIST_DIR}/rgb.pio)

# must match with executable name and source file names
target_sources(santa_fight_system PRIVATE santa_fight.c vga16_graphics.c, teenyat.c)

# must match with executable name
target_link_libraries(santa_fight_system PRIVATE pico_stdlib hardware_pio hardware_dma hardware_adc hardware_irq)

# must match with executable name
pico_add_extra_outputs(santa_fight_system)
