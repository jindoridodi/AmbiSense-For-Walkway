idf_component_register(
    SRCS "webui.c"
    INCLUDE_DIRS "include"
    REQUIRES settings auth netmgr ota board led_engine radar motion presence status_led
             esp_http_server esp_wifi esp_app_format esp_timer
             json log freertos esp_system
    EMBED_FILES "ui.html.gz" "ui.html"
)

set(identity_header "${CMAKE_CURRENT_BINARY_DIR}/build_identity.h")
add_custom_target(ambisense_build_identity ALL
    COMMAND ${CMAKE_COMMAND} -DREPO_DIR=${CMAKE_SOURCE_DIR}/..
            -DOUTPUT_FILE=${identity_header}
            -P ${CMAKE_CURRENT_LIST_DIR}/build_identity.cmake
    BYPRODUCTS ${identity_header}
)
add_dependencies(${COMPONENT_LIB} ambisense_build_identity)
target_include_directories(${COMPONENT_LIB} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
