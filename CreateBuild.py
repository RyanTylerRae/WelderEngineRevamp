import atexit
import os
import platform
import re
import sys
import zipfile

BuildPlatformWin32 = "Win32"
BuildPlatformEmscripten32 = "Emscripten32"

# Current version of the emscripten SDK that we build with.
EmscriptenVersion = "1.38.12"

PlatformsBuilt = 0

# Easily zip up a directory, but the don't include the full path in the zip.
def ZipDirectory(zip, path, relative="", filter=None):
  if not os.path.isdir(path):
    raise Exception("Attempting to zip '" + path + "' failed because it was not a directory")
  for root, dirs, files in os.walk(path):
    for file in files:
      if not filter or filter(root, file):
        zip.write(os.path.join(root, file), os.path.join(relative, os.path.relpath(root, path), file))

# Make sure we don't include any build artifacts (for all platforms).
def ArtifactFilter(root, filename):
  if filename.endswith(".pdb"):
    return False
  if filename.endswith(".ilk"):
    return False
  if filename.endswith(".exp"):
    return False
  if filename.endswith(".lib"):
    return False
  if filename.endswith(".wast"):
    return False
  if filename == "BuildInfo.data":
    return False
  return True

# Zero Build filter (includes artifact filter).
def ZeroEditorFilter(root, filename):
  if os.path.dirname(root) == "ZeroLauncherResources":
    return False
  return True

# Returns the platform independent build string (for example: "1.4.0.847.5411767e62d3-").
def ReadPlatformIndependentBuildVersion():
  # Get the build version of the engine (generated by cmake).
  print "Reading BuildVersion.inl"
  buildVersionFile = open("Systems/Engine/BuildVersion.inl","r")
  buildVersionContents = buildVersionFile.read()

  # Extract the important pieces of the build version.
  major = re.search('ZeroMajorVersion ([0-9]+)', buildVersionContents).group(1)
  minor = re.search('ZeroMinorVersion ([0-9]+)', buildVersionContents).group(1)
  patch = re.search('ZeroPatchVersion ([0-9]+)', buildVersionContents).group(1)
  revision = re.search('ZeroRevisionId ([0-9]+)', buildVersionContents).group(1)
  shortChangeSet = re.search('ZeroShortChangeSet ([0-9a-fA-F]+)', buildVersionContents).group(1)

  # Build up the build version string without platform (includes the ending dash).
  versionStringPlatformIndependent = major + "." + minor + "." + patch + "." + revision + "." + shortChangeSet + "-"
  return versionStringPlatformIndependent

def RunCmake(cmakeCmdFile):
  cmakeCmdFile = os.path.abspath(cmakeCmdFile)
  os.system(cmakeCmdFile)
  print "Completed RunCmake " + cmakeCmdFile

def RunMsbuild(slnFile):
  slnFile = os.path.abspath(slnFile)
  os.system("C:/Progra~2/MSBuild/14.0/Bin/amd64/msbuild /verbosity:quiet /maxcpucount:8 /target:Build /property:Configuration=Release /consoleloggerparameters:ErrorsOnly \"" + slnFile + "\"")
  print "Completed RunMsbuild " + slnFile

def RunEmmake(makeDirectory):
  if (os.path.isfile("C:/emsdk/emscripten/" + EmscriptenVersion + "/emmake")):
    print "Check: 'emmake' exists"
  if (os.path.isfile("C:/emsdk/emscripten/" + EmscriptenVersion + "/emmake.bat")):
    print "Check: 'emmake.bat' exists"
  makeDirectory = os.path.abspath(makeDirectory)
  os.system("emmake mingw32-make -j --directory=\"" + makeDirectory + "\"")
  print "Completed RunEmmake " + makeDirectory

def GetZerobuildPath(buildPlatform):
  # Build up the full build version string (for example: "1.4.0.847.5411767e62d3-Win32").
  versionStringPlatformIndependent = ReadPlatformIndependentBuildVersion()
  versionString = versionStringPlatformIndependent + buildPlatform
  
  outputZerobuild = os.path.abspath("Develop." + versionString + ".zerobuild")
  return outputZerobuild
  
def CreateSingleFileZerobuild(buildPlatform, filePath):
  outputZerobuild = GetZerobuildPath(buildPlatform)
  
  zip = zipfile.ZipFile(outputZerobuild, "w", zipfile.ZIP_DEFLATED)
  zip.write(filePath, os.path.basename(filePath))
  print "Completed CreateSingleFileZerobuild " + outputZerobuild

def CreateZeroEditorZerobuild(buildPlatform, buildOutput):
  outputZerobuild = GetZerobuildPath(buildPlatform)
  print "Outputting " + outputZerobuild
  zip = zipfile.ZipFile(outputZerobuild, "w", zipfile.ZIP_DEFLATED)
  ZipDirectory(zip, "Resources", "Resources", ZeroEditorFilter)
  ZipDirectory(zip, "Data", "Data")

  ZipDirectory(zip, buildOutput, "", ArtifactFilter)
  print "Completed CreateZeroEditorZerobuild " + outputZerobuild
  
def VerifyPlatform(buildPlatform, system):
  if (platform.system() == system):
    print "Building " + buildPlatform + " on " + system
    return True
  else:
    print buildPlatform + " must be built on " + system + " (current is " + platform.system() + ")"
    return False

def PlatformCompleted(buildPlatform):
  global PlatformsBuilt
  PlatformsBuilt += 1
  print "Completed " + buildPlatform
  
def BuildWin32():
  if (not VerifyPlatform(BuildPlatformWin32, "Windows")):
    return
  
  RunCmake("CMakeBuild/GenerateVS2015.cmd")
  RunMsbuild("CMakeBuild/VS2015_MSVC_Windows/Zero.sln")
  CreateZeroEditorZerobuild(BuildPlatformWin32, "BuildOutput/Out/Windows_VS_2015/Release/ZeroEditor/")
  PlatformCompleted(BuildPlatformWin32)

def BuildEmscripten32():
  if (not VerifyPlatform(BuildPlatformEmscripten32, "Windows")):
    return
  
  # Example: C:\emsdk\emscripten\1.38.12
  emscriptenDir = os.environ.get('EMSCRIPTEN')
  emsdkDir = None
  
  # The emsdk directory is two directories above the emscripten directory.
  if (emscriptenDir):
    emsdkDir = os.path.join(emscriptenDir, "..", "..")
  else:
    emsdkDir = "C:/emsdk"
    emscriptenDir = os.path.join(emsdkDir, "emscripten", EmscriptenVersion)
    # Git will create the directory for us.
    os.system("git clone https://github.com/juj/emsdk.git C:/emsdk")
    
    print "Setting Emscripten PATH variables"
    os.environ["PATH"] += os.pathsep + "C:/emsdk"
    os.environ["PATH"] += os.pathsep + "C:/emsdk/clang/e" + EmscriptenVersion + "_64bit"
    os.environ["PATH"] += os.pathsep + "C:/emsdk/node/8.9.1_64bit/bin"
    os.environ["PATH"] += os.pathsep + "C:/emsdk/python/2.7.13.1_64bit/python-2.7.13.amd64"
    os.environ["PATH"] += os.pathsep + "C:/emsdk/java/8.152_64bit/bin"
    os.environ["PATH"] += os.pathsep + "C:/emsdk/emscripten/" + EmscriptenVersion
    
    print "Setting Emscripten environment variables"
    os.environ["EMSDK"                      ] = "C:/emsdk"
    os.environ["EM_CONFIG"                  ] = "C:/Users/appveyor/.emscripten"
    os.environ["LLVM_ROOT"                  ] = "C:/emsdk/clang/e" + EmscriptenVersion + "_64bit"
    os.environ["EMSCRIPTEN_NATIVE_OPTIMIZER"] = "C:/emsdk/clang/e" + EmscriptenVersion + "_64bit/optimizer.exe"
    os.environ["BINARYEN_ROOT"              ] = "C:/emsdk/clang/e" + EmscriptenVersion + "_64bit/binaryen"
    os.environ["EMSDK_NODE"                 ] = "C:/emsdk/node/8.9.1_64bit/bin/node.exe"
    os.environ["EMSDK_PYTHON"               ] = "C:/emsdk/python/2.7.13.1_64bit/python-2.7.13.amd64/python.exe"
    os.environ["JAVA_HOME"                  ] = "C:/emsdk/java/8.152_64bit"
    os.environ["EMSCRIPTEN"                 ] = "C:/emsdk/emscripten/" + EmscriptenVersion
  
  # Check for sh.exe (it cannot be allowed to exist in the PATH).
  # The correct logic here would be to parse all directories in the PATH and then walk through all files in each directory, looking for sh.exe.
  # For now, since we're targing the build server (AppVeyor) we know where the instance of sh.exe is within the path.
  shPath = "C:/Program Files/Git/usr/bin/sh.exe"
  if (os.path.isfile(shPath)):
    backupPath = shPath + ".bak"
    os.rename(shPath, backupPath)
    def RenameShBack():
      os.rename(backupPath, shPath)
    atexit.register(RenameShBack)

  workingDir = os.getcwd()
  
  # Install names such as sdk-1.38.12-64bit
  installName = "sdk-" + EmscriptenVersion + "-64bit"

  os.chdir(emsdkDir)
  os.system("git pull")
  os.system("emsdk update-tags")
  os.system("emsdk install " + installName + " --global")
  os.system("emsdk activate " + installName + " --global")
  os.system("emsdk_env.bat")
  
  # Recall back to the working directory, everything is in our path now.
  os.chdir(workingDir)
  
  RunCmake("CMakeBuild/GenerateMingw_Emscripten.cmd")
  RunEmmake("CMakeBuild/Mingw_Emscripten/")
  CreateSingleFileZerobuild(BuildPlatformEmscripten32, "BuildOutput/Out/Emscripten/Debug/ZeroEditor/ZeroEditor.html")
  PlatformCompleted(BuildPlatformEmscripten32)

# The user can pass an argument to say which platform we build.
platformToBuild = None
if (len(sys.argv) >= 2):
  platformToBuild = sys.argv[1]

if (not platformToBuild or platformToBuild == BuildPlatformWin32):
  BuildWin32()
  
if (not platformToBuild or platformToBuild == BuildPlatformEmscripten32):
  BuildEmscripten32()

if (PlatformsBuilt == 0):
  print "No platforms were built. Was the passed in platform name valid?"