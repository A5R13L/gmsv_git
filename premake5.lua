PROJECT_GENERATOR_VERSION = 3
local gmcommon = "./garrysmod_common_64"
include(gmcommon)
newoption({
	trigger = "tag_version",
	value = "string",
	description = "The current tag version of the repository"
})

local function PostSetup(bits)
	includedirs{"source/thirdparty/libgit2/include"}
	if bits == 64 then
		libdirs{"source/thirdparty/libgit2/libs/x64"}
	else
		libdirs{"source/thirdparty/libgit2/libs/x32"}
	end

	links{"git2", "ssl", "crypto", "z", "pthread"}
	defines{"GIT_VERSION=\"" .. (_OPTIONS["tag_version"] or "unknown") .. "\""}
end

CreateWorkspace({
	name = "git_64",
	abi_compatible = false,
	path = "projects/x64/" .. os.target() .. "/" .. _ACTION
})

CreateProject({
	serverside = true,
	source_path = "source",
	manual_files = false
})

IncludeHelpersExtended()
IncludeSDKCommon()
IncludeSDKTier0()
IncludeSDKTier1()
IncludeDetouring()
IncludeScanning()
files({"source/git/**/*.*"})
PostSetup(64)
gmcommon = "./garrysmod_common_32"
include(gmcommon)
CreateWorkspace({
	name = "git_32",
	abi_compatible = false,
	path = "projects/x32/" .. os.target() .. "/" .. _ACTION
})

CreateProject({
	serverside = true,
	source_path = "source",
	manual_files = false
})

IncludeHelpersExtended()
IncludeSDKCommon()
IncludeSDKTier0()
IncludeSDKTier1()
IncludeDetouring()
IncludeScanning()
files({"source/**/*.*"})
PostSetup(32)