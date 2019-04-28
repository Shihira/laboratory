#!/usr/bin/env python

### Copyright (c) Shihira Fung, 2015 <fengzhiping@hotmail.com>
### This launcher is compatible with both python 2.x and 3.x

import json
import platform
from os import path, listdir, system
from functools import reduce

#### Options
USERNAME = "Shihira"
VERSION = ""
GAMEDIR = ".minecraft"
JAVA_PATH = "java"
#JAVA_PATH = "/usr/lib/jvm/java-7-openjdk/bin/java"
PLATFORM = platform.system().lower()
MEMORY = 1024

#### DO NOT MODIFY

if not VERSION:
    # use the latest
    VERSION = sorted(listdir(path.join(GAMEDIR, "versions")))[-1]

JAVA_ARGS = [
        "-Xmx%dm" % MEMORY,
        "-Dfml.ignoreInvalidMinecraftCertificates=true",
        "-Dfml.ignorePatchDiscrepancies=true",
        "-Djava.library.path=%s" % path.join(GAMEDIR, "natives"),
    ]

CP_SEP = ";" if PLATFORM == "windows" else ":"

class MinecraftArguments:
    fields = {
            "auth_player_name": None,
            "version_name": None,
            "game_directory": None,
            "assets_root": None,
            "assets_index_name": None,
            "auth_uuid": "{}",
            "auth_access_token": "{}",
            "user_properties": "{}",
            "user_type": "{}",
            "version_type": "{}",
        }

    def __init__(self):
        self.fields["auth_player_name"] = USERNAME
        self.fields["version_name"] = VERSION
        self.fields["game_directory"] = GAMEDIR
        self.fields["assets_root"] = path.join(GAMEDIR, "assets")

    def process(self, arg_str):
        return reduce(lambda arg, s: arg.replace("${%s}"%s, self.fields[s]),
            self.fields.keys(), arg_str)


class Version:
    def select_library(self, name):
        lib_dir_list = name.split(":")
        lib_dir_list = lib_dir_list[0].split(".") + lib_dir_list[1:]
        lib_dir_list = [GAMEDIR, "libraries"] + lib_dir_list

        try:
            # proper version
            lib_version, lib_dir_list = lib_dir_list[-1], lib_dir_list[:-1]
            lib_ava_ver = listdir(path.sep.join(lib_dir_list))
            if lib_version not in lib_ava_ver:
                print("[WARNING] %s-%s is not found for. Use %s instead." %
                        (".".join(lib_dir_list[2:]), lib_version, lib_ava_ver[0]))
                lib_version = lib_ava_ver[0]
            lib_dir = path.sep.join(lib_dir_list + [lib_version])

            jar_ava_ver = listdir(lib_dir)
            jar_version = jar_ava_ver[0]

            lib_path = path.join(lib_dir, jar_version)

            return [lib_path]
        except Exception as e:
            print("[ERROR] cannot handle with library %s" % name)

            return []

    def get_single_version_libraries(self):
        libraries = []

        for lib in self.profile["libraries"]:
            libraries += self.select_library(lib["name"])

        return libraries

    def get_version_main_jar(self):
        main_jar_version = ""
        if "jar" in self.profile:
            main_jar_version = self.profile["jar"]
        else:
            main_jar_version = self.version_name

        jar = path.join(self.version_dir, "..", main_jar_version, main_jar_version + ".jar")

        if not path.isfile(jar):
            print("[ERROR] Unable to locate main jar " + jar)

        return jar

    def __init__(self, version_name):
        self.version_name = version_name
        self.version_dir = path.join(GAMEDIR, "versions", version_name)
        self.profile = json.load(open(path.join(
            self.version_dir, version_name + ".json")))

        if "inheritsFrom" in self.profile:
            parent_version = Version(self.profile["inheritsFrom"])

            for k, v in parent_version.profile.items():
                if isinstance(v, list) and k in self.profile:
                    self.profile[k] += v
                elif k not in self.profile:
                    self.profile[k] = v

    def get_libraries(self):
        libraries = self.get_single_version_libraries()
        libraries += [self.get_version_main_jar()]

        return libraries

class CommandLine:
    def __init__(self, version, mcargs):
        self.version = version
        self.mcargs = mcargs

    def build(self):
        self.mcargs.fields["assets_index_name"] = \
            self.version.profile["assets"]

        return '%s %s -cp "%s" %s %s' % (
                JAVA_PATH,
                " ".join(JAVA_ARGS),
                CP_SEP.join(self.version.get_libraries()),
                self.version.profile["mainClass"],
                self.mcargs.process(self.version.profile["minecraftArguments"])
            )

if __name__ == '__main__':
    version = Version(VERSION)
    mcargs = MinecraftArguments()
    cmdline = CommandLine(version, mcargs).build()

    print(cmdline)
    system(cmdline)

