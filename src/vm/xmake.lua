set_project("vm")

add_rules("mode.debug", "mode.release")
add_requires("doctest")
-- add_defines("MACRO_NAME=1")
-- add_defines("MACRO_NAME=\"SOME STRING\"")

target("vm")
    set_languages("cxx20")
    set_kind("binary")
    set_warnings("all", "extra", "pedantic", "error")

    add_includedirs("src")
    add_files("src/**.cpp")
    add_packages("doctest")

    if is_plat("windows") then
        add_cxflags("/utf-8", {force = true})
        add_cxxflags("/utf-8", {force = true})
        add_cxxflags("/wd4068", {force = true}) -- for #pragma clang
    end

    after_build(function (target)
        os.cp(target:targetfile(), "bin/")
    end)
target_end()
