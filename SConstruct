"""
Copyright (c) 2012-2014 Stanford University

Permission to use, copy, modify, and distribute this software for any 
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR(S) DISCLAIM ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL AUTHORS BE LIABLE FOR 
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
"""

import sys
import os
import multiprocessing
import SCons.Util


## Helper Functions

def CheckPkgConfig(context):
    context.Message('Checking for pkg-config... ')
    ret = context.TryAction('pkg-config --version')[0]
    context.Result(ret)
    return ret

def CheckPkg(context, name):
    context.Message('Checking for %s... ' % name)
    ret = context.TryAction('pkg-config --exists \'%s\'' % name)[0]
    context.Result(ret)
    return ret

def CheckPkgMinVersion(context, name, version):
    context.Message('Checking %s-%s or greater... ' % (name, version))
    ret = context.TryAction('pkg-config --atleast-version \'%s\' \'%s\'' % (version, name))[0]
    context.Result(ret)
    return ret

## Configuration

opts = Variables('Local.sc')

opts.AddVariables(
    ("CC", "C Compiler"),
    ("CXX", "C++ Compiler"),
    ("AS", "Assembler"),
    ("LINK", "Linker"),
    ("NUMCPUS", "Number of CPUs to use for build (0 means auto)", 0, None, int),
    EnumVariable("BUILDTYPE", "Build type", "RELEASE", ["RELEASE", "DEBUG", "PERF"]),
    BoolVariable("VERBOSE", "Show full build information", 0),
    BoolVariable("WITH_GPROF", "Enable profiling", 0),
    BoolVariable("BUILD_BINARIES", "Build binaries", 1),
    PathVariable("PREFIX", "Installation target directory", "/usr/local/bin/", PathVariable.PathAccept),
    PathVariable("DESTDIR", "The root directory to install into. Useful mainly for binary package building", "", PathVariable.PathAccept),
)

env = Environment(options = opts,
                  tools = ['default'],
                  ENV = os.environ,
                  LINK = "g++")
Help(opts.GenerateHelpText(env))

# Copy environment variables
if "CC" in os.environ:
    env["CC"] = os.getenv('CC')
if "CXX" in os.environ:
    env["CXX"] = os.getenv('CXX')
if "AS" in os.environ:
    env["AS"] = os.getenv('AS')
if "LD" in os.environ:
    env["LINK"] = os.getenv('LD')
if "CFLAGS" in os.environ:
    env.Append(CCFLAGS = SCons.Util.CLVar(os.environ['CFLAGS']))
if "CPPFLAGS" in os.environ:
    env.Append(CPPFLAGS = SCons.Util.CLVar(os.environ['CPPFLAGS']))
if "CXXFLAGS" in os.environ:
    env.Append(CXXFLAGS = SCons.Util.CLVar(os.environ['CXXFLAGS']))
if "LDFLAGS" in os.environ:
    env.Append(LINKFLAGS = SCons.Util.CLVar(os.environ['LDFLAGS']))

env.Append(CFLAGS = [ "-Wall", "-Werror", "-Wextra", "-O3", "-std=c99", "-Wno-deprecated-declarations"])
env.Append(CXXFLAGS = [ "-Wall", "-Werror", "-Wextra", "-O3", "-std=c++11"])

# On Mac OS X, include homebrew openssl
if sys.platform == "darwin":
    env.Append(CFLAGS = [ "-I/usr/local/opt/openssl/include" ])
    env.Append(LINKFLAGS = [ "-L/usr/local/opt/openssl/lib" ])

if "WITH_GPROF" in env and env["WITH_GPROF"]:
    print("With gprof")
    env.Append(CFLAGS = [ "-pg" ])
    env.Append(CPPFLAGS = [ "-pg" ])
    env.Append(LINKFLAGS = [ "-pg" ])

if env["BUILDTYPE"] == "DEBUG":
    print("DEBUG MODE!")
    env.Append(CPPFLAGS = [ "-g", "-DDEBUG", "-Wall"])
elif env["BUILDTYPE"] == "RELEASE":
    env.Append(CPPFLAGS = ["-DNDEBUG", "-Wall" ])
else:
    print("Error BUILDTYPE must be RELEASE or DEBUG")
    sys.exit(-1)

if not env["VERBOSE"]:
    env["CCCOMSTR"] = "Compiling $SOURCE"
    env["SHCCCOMSTR"] = "Compiling $SOURCE"
    env["ARCOMSTR"] = "Creating library $TARGET"
    env["RANLIBCOMSTR"] = "Indexing library $TARGET"
    env["LINKCOMSTR"] = "Linking $TARGET"

def GetNumCPUs(env):
    if env["NUMCPUS"] > 0:
        return int(env["NUMCPUS"])
    return 2*multiprocessing.cpu_count()

env.SetOption('num_jobs', GetNumCPUs(env))

# Configuration
conf = env.Configure(custom_tests = { 'CheckPkgConfig' : CheckPkgConfig,
                                      'CheckPkg' : CheckPkg,
                                      'CheckPkgMinVersion' : CheckPkgMinVersion })

if not conf.CheckCC():
    print('Your C compiler and/or environment is incorrectly configured.')
    CheckFailed()

conf.Finish()

Export('env')

# Set compile options for binaries
env.Append(LIBS = ["balloon"], \
  LIBPATH = ['#build/libballoon'])

# Add header files
env.Append(CPPPATH = ["#include", "#."])

env.Append(CPPFLAGS = ['-pthread'])
env.Append(LIBS = ["crypto", "pthread", "m"])


# libballoon
SConscript('libballoon/SConscript', variant_dir='build/libballoon')

# Utilities
if env["BUILD_BINARIES"]:
    SConscript('balloon/SConscript', variant_dir='build/balloon')
    SConscript('btest/SConscript', variant_dir='build/btest')

