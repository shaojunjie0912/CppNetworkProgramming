target("sync_server", function()
    set_kind("binary")
    set_encodings("source:utf-8")
    add_files("./sync_server.cpp")
    add_packages("boost")
end)

target("sync_client", function()
    set_kind("binary")
    set_encodings("source:utf-8")
    add_files("./sync_client.cpp")
    add_packages("boost")
end)
