add_requires("nlohmann_json")

target("http_server", function()
    set_kind("binary")
    set_encodings("source:utf-8")
    add_files("src/*.cpp")
    add_includedirs("include")
    add_packages("boost", "nlohmann_json")
end)
