set_project("CppNetworkProgramming")
set_languages("c++2a")

add_rules("plugin.compile_commands.autoupdate")
add_requires("boost", { configs = { all = true } })

target("thread_pool", function()
    set_kind("binary")
    set_encodings("source:utf-8")
    add_files("src/*.cpp")
    add_includedirs("include")
    add_packages("boost")
end)
