set_project("CppNetworkProgramming")
set_languages("c++2a")

add_rules("plugin.compile_commands.autoupdate")
add_requires("boost", { configs = { all = true } })

includes("asio")
includes("beast")
includes("xiaopeng")
