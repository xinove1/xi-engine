#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./src/modules/core.h"

#define NOB_IMPLEMENTATION
#include "./nob.h"

internal void build_linux_static();
internal void build_linux_hot();
internal void build_and_run_hot();
internal void build_windows();
internal void build_web();
internal void raylib_go_rebuild_urself();

#define flag_compare(flag, ...) strcmp_many(flag, ((const char*[]){__VA_ARGS__}), (sizeof((const char*[]){__VA_ARGS__})/sizeof(const char*)))
#define da_append_cstrs(da, cstrs) nob_da_append_many(&(da), cstrs, count_of(cstrs)) 
b32 strcmp_many(const cstr *str, const cstr **str_many, i32 str_many_count);
b32 nob_proc_is_running(Nob_Proc proc);

const global cstr *ProjectOutputName = "game";

const global cstr *DebugFlags[] = {
	"-g3",
	"-Wall",
	"-Wextra",
	"-Wno-unused-parameter",
	"-Wno-unused-function",
        //"-fsanitize=address", // BUG  remeber that this causes leak on xilib, so not good for long running the game, only for testing for leaks
	"-fsanitize=undefined",
	"-fsanitize-trap",
	"-DDEBUG_BUILD",
        //"-Wconversion",
	//"-Wno-sign-conversion",
	//"-Werror",
	//"-Wdouble-promotion",
};
const global cstr *SharedFlags[] = { "-I./src/modules/", "-I./external/raylib-5.0/src"};
const global cstr *HotFlags[] = {"-DHOT_RELOAD", "-Wl,-rpath=./build/", "-lraylib_linux",  "-L./build/", };

const global cstr *Src_EngineLayer[] = {"./src/main.c", };
const global cstr *Dep_EngineLayer[] = {"./src/main.h"};

const global cstr *Src_Modules[] = { "./src/implementations.c", };
const global cstr *Dep_Modules[] = { 
	"./src/modules/ui.h", "./src/modules/core.h", 
	"./src/modules/list.h", "./src/modules/arenas.h", 
	"./src/modules/collision.h", "./src/modules/raymath_short.h",
	"./src/modules/input.h",
};

const global cstr *Src_Game[] = { 
	"./src/game_code/game.c",
	"./src/game_code/entity_and_map.c",
	"./src/game_code/utils.c",
	"./src/game_code/editor.c",
};
const global cstr *Dep_Game[] = { "./src/game_code/game.h" };

// TODO  Be able to toggle it with command line flag
global b32 Debug = false; 
global b32 ReLaunch = true;

int main(int argc, char *argv[]) 
{
	NOB_GO_REBUILD_URSELF(argc, argv);

	if (!nob_mkdir_if_not_exists("./build")) return 1;

	nob_shift_args(&argc, &argv); // get program name out
	
	raylib_go_rebuild_urself();
	
	cstr *flag;
	if (argc == 0) flag = "hot_run"; // default build
	else flag = nob_shift_args(&argc, &argv);

	if (flag_compare(flag, "hot")) {
		build_linux_hot();
	} 
	else if (flag_compare(flag, "linux_static")) {
		build_linux_static();
	}
	else if (flag_compare(flag, "web")) {
		// TODO  Implement web build
		printf("build web not implemented\n");
		//build_web();
	}
	else if (flag_compare(flag, "windows")) {
		build_windows();
	} 
	else if (flag_compare(flag, "hot_run", "rh")) {
		build_and_run_hot();
	}
	else if (flag_compare(flag, "run", "r")) {
		Nob_Cmd cmd = {0};
		nob_cmd_append(&cmd, "./build/game");
		nob_cmd_run_sync(cmd);
		return (0);
	} 
	else if (flag_compare(flag, "clean")) {
		Nob_Cmd cmd = {0};
		nob_cmd_append(&cmd, "rm");
		// NOTE  Hardcoded name instead of using ProjectOutPutName
		nob_cmd_append(&cmd, "./build/game");
		nob_cmd_append(&cmd, "./build/game.so");
		nob_cmd_append(&cmd, "./build/libxilib.so");
		if (!nob_cmd_run_sync(cmd)) return (1);
		nob_cmd_free(cmd);
	}
	else if (flag_compare(flag, "fclean")) {
		Nob_Cmd cmd = {0};
		nob_cmd_append(&cmd, "rm");
		nob_cmd_append(&cmd, "./build/libraylib_linux.so");
		nob_cmd_append(&cmd, "./build/libraylib_linux.a");
		nob_cmd_append(&cmd, "./build/libraylib_web.a");
		nob_cmd_append(&cmd, "./build/libraylib_windows.a");
		if (!nob_cmd_run_sync(cmd)) return (1);
		nob_cmd_free(cmd);
	} 

	else if (flag_compare(flag, "help", "-help", "--help", "-h")) {
		printf("--------------Help--------------\n");
		printf("Please fill this eventually lol\n");
	} 
	else {
		printf("\n");
		nob_log(NOB_ERROR, "flag %s unrecognized. run with '-help' for the available build options \n", flag);
	}

	if (argc != 0) {
		nob_log(NOB_INFO, "Only 1 flag alowed for now\n");
	}
}

internal void build_engine_layer(Nob_Cmd *cmd);
internal void build_modules(Nob_Cmd *cmd);
internal void build_game_lib(Nob_Cmd *cmd);

internal void build_and_run_hot()
{
	Nob_Cmd cmd = {0};
	{
		Debug = true;
		build_modules(&cmd);
		build_engine_layer(&cmd);
		build_game_lib(&cmd);
	}

	cmd.count = 0;
	nob_cmd_append(&cmd, "./build/game");
	Nob_Proc proc = nob_cmd_run_async(cmd);

	Nob_Cmd dep_engine = {0}; da_append_cstrs(dep_engine, Src_EngineLayer); da_append_cstrs(dep_engine, Dep_EngineLayer);
	Nob_Cmd dep_modules = {0}; da_append_cstrs(dep_modules, Src_Modules); da_append_cstrs(dep_modules, Dep_Modules);
	Nob_Cmd dep_game = {0}; da_append_cstrs(dep_game, Src_Game); da_append_cstrs(dep_game, Dep_Game);

	while (nob_proc_is_running(proc)) {
		b32 re_launch = false;
		usleep(10 * 1000);

		// nob_needs_rebuild return is not a bool :<(
		if (nob_needs_rebuild("./build/game.so", dep_game.items, dep_game.count) > 0) {
			printf("------------ REBUILDING GAME LIB --------------- \n");
			build_game_lib(&cmd);
			printf("------------------------------------------------ \n");
		}
		if (nob_needs_rebuild("./build/game", dep_engine.items, dep_engine.count) > 0) {
			printf("------------ REBUILDING Engine layer --------------- \n");
			if (ReLaunch) re_launch = true;
			else printf("------------ This will not re-launch the game------- \n");
			build_engine_layer(&cmd);
			printf("------------------------------------------------ \n");
		}
		if (nob_needs_rebuild("./build/libxilib.so", dep_modules.items, dep_modules.count) > 0) {
			printf("------------ REBUILDING Modules --------------- \n");
			if (ReLaunch) re_launch = true;
			else printf("------------ This will not re-launch the game------- \n");
			build_modules(&cmd);
			printf("------------------------------------------------ \n");
		}
		if (re_launch) {
			if (kill(proc, SIGKILL) < 0) {
				nob_log(NOB_ERROR, "Killing current instance of game before re-launch failed: \n %s \n", strerror(errno));
				exit(1);
			}

			cmd.count = 0;
			nob_cmd_append(&cmd, "./build/game");
			proc = nob_cmd_run_async(cmd);
		};
	}
}

internal void build_linux_hot()
{
	Nob_Cmd cmd = {0};

	Debug = true;

	printf("Building modules\n");
	build_modules(&cmd);

	printf("Building engine layer \n");
	build_engine_layer(&cmd);

	printf("Building game lib\n");
	build_game_lib(&cmd);

	nob_cmd_free(cmd);
}

internal void build_engine_layer(Nob_Cmd *cmd)
{
	cmd->count = 0;
	nob_cmd_append(cmd, "cc", "-std=c99");
	nob_cmd_append(cmd, "-lGL", "-lm", "-lpthread", "-ldl", "-lrt", "-lX11",); // raylib
	nob_da_append_many(cmd, SharedFlags, count_of(SharedFlags));
	if (Debug) nob_da_append_many(cmd, DebugFlags, count_of(DebugFlags));
	nob_da_append_many(cmd, HotFlags, count_of(HotFlags));

	nob_da_append_many(cmd, Src_EngineLayer, count_of(Src_EngineLayer));
	
	nob_cmd_append(cmd, "-lxilib");

	Nob_String_Builder name = {0};
	nob_sb_append_cstr(&name, "./build/");
	nob_sb_append_cstr(&name, ProjectOutputName);
	nob_sb_append_null(&name);
	nob_cmd_append(cmd, "-o", name.items);

	if (!nob_cmd_run_sync(*cmd)) exit(1);
	nob_sb_free(name);
}

internal void build_modules(Nob_Cmd *cmd)
{
	cmd->count = 0;
	nob_cmd_append(cmd, "cc", "-std=c99");
	nob_cmd_append(cmd, "-lGL", "-lm", "-lpthread", "-ldl", "-lrt", "-lX11",); // raylib
	nob_da_append_many(cmd, SharedFlags, count_of(SharedFlags));
	if (Debug) nob_da_append_many(cmd, DebugFlags, count_of(DebugFlags));
	nob_da_append_many(cmd, HotFlags, count_of(HotFlags));
	nob_da_append_many(cmd, Src_Modules, count_of(Src_Modules));
	nob_cmd_append(cmd, "-fpic", "-shared");
	nob_cmd_append(cmd, "-o", "./build/libxilib.so");

	if (!nob_cmd_run_sync(*cmd)) exit(1);
}

internal void build_game_lib(Nob_Cmd *cmd)
{
	cmd->count = 0;
	nob_cmd_append(cmd, "cc", "-std=c99");
	nob_cmd_append(cmd, "-lGL", "-lm", "-lpthread", "-ldl", "-lrt", "-lX11",); // raylib
	nob_da_append_many(cmd, SharedFlags, count_of(SharedFlags));
	if (Debug) nob_da_append_many(cmd, DebugFlags, count_of(DebugFlags));
	nob_da_append_many(cmd, HotFlags, count_of(HotFlags));
	nob_da_append_many(cmd, Src_Game, count_of(Src_Game));
	nob_cmd_append(cmd, "-fpic", "-shared");
	nob_cmd_append(cmd, "-o", "./build/libxilib.so");
	
	Nob_String_Builder name = {0};
	nob_sb_append_cstr(&name, "./build/");
	nob_sb_append_cstr(&name, ProjectOutputName);
	nob_sb_append_cstr(&name, ".so");
	nob_sb_append_null(&name);
	nob_cmd_append(cmd, "-o", name.items);

	if (!nob_cmd_run_sync(*cmd)) exit(1);
	nob_sb_free(name);
}

internal void build_linux_static()
{
	Nob_Cmd cmd = {0};

	nob_cmd_append(&cmd, "cc");

	// Flags
	nob_cmd_append(&cmd, "-std=c99");
	nob_cmd_append(&cmd, "-lGL", "-lm", "-lpthread", "-ldl", "-lrt", "-lX11",); // raylib
	nob_da_append_many(&cmd, SharedFlags, count_of(SharedFlags));
	if (Debug) nob_da_append_many(&cmd, DebugFlags, count_of(DebugFlags));
	
	// Sources
	nob_da_append_many(&cmd, Src_Game, count_of(Src_Game));
	nob_da_append_many(&cmd, Src_EngineLayer, count_of(Src_EngineLayer));
	nob_da_append_many(&cmd, Src_Modules, count_of(Src_Modules));
	
	nob_cmd_append(&cmd, "./build/libraylib_linux.a");

	Nob_String_Builder name = {0};
	nob_sb_append_cstr(&name, "./build/");
	nob_sb_append_cstr(&name, ProjectOutputName);
	nob_sb_append_null(&name);
	nob_cmd_append(&cmd, "-o", name.items);

	if (!nob_cmd_run_sync(cmd)) exit(1);
	nob_sb_free(name);
}

internal void build_windows()
{
	Nob_Cmd cmd = {0};

	nob_cmd_append(&cmd, "x86_64-w64-mingw32-gcc");

	// Flags
	nob_cmd_append(&cmd, "-std=c99");
	nob_da_append_many(&cmd, SharedFlags, count_of(SharedFlags));
	if (Debug) nob_da_append_many(&cmd, DebugFlags, count_of(DebugFlags)); // NOTE  Untested 

	// Sources
	nob_da_append_many(&cmd, Src_Game, count_of(Src_Game));
	nob_da_append_many(&cmd, Src_EngineLayer, count_of(Src_EngineLayer));
	nob_da_append_many(&cmd, Src_Modules, count_of(Src_Modules));

	nob_cmd_append(&cmd, "./build/libraylib_windows.a"); // NOTE  has to be before raylib flags for whateaver reason
	nob_cmd_append(&cmd, "-lopengl32", "-lgdi32", "-lwinmm");

	// Prefix project output name with proper extension
	Nob_String_Builder name = {0};
	nob_sb_append_cstr(&name, "./build/");
	nob_sb_append_cstr(&name, ProjectOutputName);
	nob_sb_append_cstr(&name, ".exe");
	nob_sb_append_null(&name);
	nob_cmd_append(&cmd, "-o", name.items);

	if (!nob_cmd_run_sync(cmd)) exit(1);
	nob_sb_free(name);
}

internal void raylib_go_rebuild_urself()
{
	// TODO  refator to use only one Nob_cmd, reset count to 0 to use
	if (!nob_file_exists("./build/libraylib_linux.so"))
	{
		nob_log(NOB_INFO, "Building raylib linux shared -----\n");
		Nob_Cmd cmd = {0};
		nob_cmd_append(&cmd, "make", "-C", "./external/raylib-5.0/src/");
		nob_cmd_append(&cmd, "RAYLIB_LIBTYPE=SHARED");
		if (!nob_cmd_run_sync(cmd)) exit(1);

		if (!nob_copy_file("./external/raylib-5.0/src/libraylib.so", "./build/libraylib_linux.so"))
			exit(1);

		//nob_cmd_free(cmd);
		cmd.count = 0;
		nob_cmd_append(&cmd, "make", "-C", "./external/raylib-5.0/src/");
		nob_cmd_append(&cmd, "clean");
		if (!nob_cmd_run_sync(cmd)) exit(1);

		nob_cmd_free(cmd);

		nob_log(NOB_INFO, "---------------------------------\n");
	}

	if (!nob_file_exists("./build/libraylib_linux.a"))
	{
		nob_log(NOB_INFO, "Building raylib linux static -----\n");
		Nob_Cmd cmd = {0};
		nob_cmd_append(&cmd, "make", "-C", "./external/raylib-5.0/src/");
		if (!nob_cmd_run_sync(cmd)) exit(1);

		if (!nob_copy_file("./external/raylib-5.0/src/libraylib.a", "./build/libraylib_linux.a"))
			exit(1);

		//nob_cmd_free(cmd);
		cmd.count = 0;
		nob_cmd_append(&cmd, "make", "-C", "./external/raylib-5.0/src/");
		nob_cmd_append(&cmd, "clean");
		if (!nob_cmd_run_sync(cmd)) exit(1);

		nob_cmd_free(cmd);

		nob_log(NOB_INFO, "---------------------------------\n");
	}

	if (!nob_file_exists("./build/libraylib_windows.a"))
	{
		nob_log(NOB_INFO, "Building raylib windows -----\n");
		Nob_Cmd cmd = {0};
		nob_cmd_append(&cmd, "make", "-C", "./external/raylib-5.0/src/");
		nob_cmd_append(&cmd, "OS=Windows_NT", "CC=x86_64-w64-mingw32-gcc", "AR=x86_64-w64-mingw32-ar");
		if (!nob_cmd_run_sync(cmd)) exit(1);

		if (!nob_copy_file("./external/raylib-5.0/src/libraylib.a", "./build/libraylib_windows.a"))
			exit(1);

		//nob_cmd_free(cmd);
		cmd.count = 0;
		nob_cmd_append(&cmd, "make", "-C", "./external/raylib-5.0/src/");
		nob_cmd_append(&cmd, "clean");
		if (!nob_cmd_run_sync(cmd)) exit(1);

		nob_cmd_free(cmd);

		nob_log(NOB_INFO, "---------------------------------\n");
	}

	if (!nob_file_exists("./build/libraylib_web.a"))
	{
		nob_log(NOB_INFO, "Building raylib web -----\n");
		Nob_Cmd cmd = {0};
		nob_cmd_append(&cmd, "make", "-C", "./external/raylib-5.0/src/");
		// NOTE  hard coded paths
		nob_cmd_append(&cmd, "PLATFORM=PLATFORM_WEB", "-B", "EMSDK_PATH=/home/xinove/stuff/emsdk",  "PYTHON_PATH=/usr/bin/python", "NODE_PATH=/home/xinove/stuff/emsdk/node/16.20.0_64bit/bin");
		if (!nob_cmd_run_sync(cmd)) exit(1);

		if (!nob_copy_file("./external/raylib-5.0/src/libraylib.a", "./build/libraylib_web.a"))
			exit(1);

		//nob_cmd_free(cmd);
		cmd.count = 0;
		nob_cmd_append(&cmd, "make", "-C", "./external/raylib-5.0/src/");
		nob_cmd_append(&cmd, "clean");
		if (!nob_cmd_run_sync(cmd)) exit(1);

		nob_cmd_free(cmd);

		nob_log(NOB_INFO, "---------------------------------\n");
	}
}


// -----------
b32 nob_proc_is_running(Nob_Proc proc) 
{
	int wstatus = 0;

	if (waitpid(proc, &wstatus, WNOHANG) < 0) {
		nob_log(NOB_ERROR, "could not wait on command (pid %d): %s", proc, strerror(errno));
		return (false);
	}

	if (WIFEXITED(wstatus)) {
		int exit_status = WEXITSTATUS(wstatus);
		if (exit_status != 0) {
			nob_log(NOB_ERROR, "command exited with exit code %d", exit_status);
			return (false);
		}
	}
	if (WIFSIGNALED(wstatus)) {
		nob_log(NOB_ERROR, "command process was terminated by %s", strsignal(WTERMSIG(wstatus)));
		return (false);
	}

	return (true);
}


b32 strcmp_many(const cstr *str, const cstr **str_many, i32 str_many_count)
{
	for (i32 i = 0; i < str_many_count; i++) {
		if (!strcmp(str, str_many[i])) return (true);
	}
	return (false);
}

// -----------
