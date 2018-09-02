find_package (KDecoration2 REQUIRED)

find_package (Qt5 REQUIRED COMPONENTS
    Core
    Gui
)

find_package (KF5 REQUIRED COMPONENTS
    Config
    CoreAddons
    GuiAddons
    WindowSystem
)

set (decoration_SRCS
    CloseButton.cc
    Decoration.cc
    MaximizeButton.cc
    MinimizeButton.cc
    plugin.cc
)

add_library (materialdecoration MODULE
    ${decoration_SRCS}
)

target_link_libraries (materialdecoration
    PUBLIC
        Qt5::Core
        Qt5::Gui
        KF5::ConfigCore
        KF5::ConfigGui
        KF5::CoreAddons
        KF5::GuiAddons
        KF5::WindowSystem

    PRIVATE
        KDecoration2::KDecoration
)

install (TARGETS materialdecoration
         DESTINATION ${PLUGIN_INSTALL_DIR}/org.kde.kdecoration2)
