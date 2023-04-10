_DEFAULTS = {
	["deps"] = "."
}

newoption {
	trigger     = "deps",
	value       = "DIR",
	description = "Root directory containing dependencies."
}

solution "BMASHINA"
	configurations { "Debug", "Release" }

	configuration "macosx"
		platforms { "x64", "ARM64" }
	configuration {}
		platforms { "x64" }

	configuration "Debug"
		defines { "DEBUG" }
		symbols "On"

	configuration "Release"
		defines { "NDEBUG" }
		optimize "On"

	project "bmashina"
		language "C++"
		kind "SharedLib"

		cppdialect "C++17"

		configuration "Debug"
			targetsuffix "_debug"
			objdir "obj/bmashina/debug"
			targetdir "bin"
		configuration "Release"
			objdir "obj/bmashina/release"
			targetdir "bin"
		configuration "windows"
			defines { "BMASHINA_BUILDING_WINDOWS" }
		configuration "macosx"
			systemversion "10.7"
		configuration {}
			runtime "release"

		location "lmashina"

		files {
			"bmashina/include/**.hpp",
			"lmashina/include/**.hpp",
			"lmashina/source/**.cpp",
			"lmashina/source/**.hpp"
		}

		includedirs {
			path.join(_OPTIONS["deps"] or _DEFAULTS["deps"], "include"),
			"bmashina/include",
			"lmashina/include"
		}

		libdirs {
			path.join(_OPTIONS["deps"] or _DEFAULTS["deps"], "lib")
		}

		links { "lua51" }
