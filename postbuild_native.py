Import("env")
import os
import shutil
import subprocess
import urllib.request
import zipfile


def ensure_import_library(project_dir, dll_name, package_dir):
    lib_name = os.path.splitext(dll_name)[0] + ".a"
    lib_path = os.path.join(project_dir, "external", package_dir, lib_name)
    if os.path.exists(lib_path):
        return lib_path

    def_file = os.path.join(project_dir, os.path.splitext(dll_name)[0] + ".def")
    dll_path = os.path.join(project_dir, "external", package_dir, "x86_64-w64-mingw32", "bin", dll_name)
    if not os.path.exists(dll_path):
        return None

    cmd = ["gendef", "-a", dll_path]
    subprocess.run(cmd, cwd=project_dir, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if not os.path.exists(def_file):
        return None

    cmd = ["dlltool", "-d", def_file, "-l", lib_path]
    subprocess.run(cmd, cwd=project_dir, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if os.path.exists(def_file):
        os.remove(def_file)
    return lib_path


def ensure_runtime_dll(project_dir, archive_url, dll_name, package_dir):
    target_dir = os.path.join(project_dir, "external", package_dir, "x86_64-w64-mingw32", "bin")
    target_path = os.path.join(target_dir, dll_name)
    if os.path.exists(target_path):
        return target_path

    os.makedirs(target_dir, exist_ok=True)
    cache_dir = os.path.join(project_dir, ".pio", "downloads", "sdl_runtime")
    os.makedirs(cache_dir, exist_ok=True)
    archive_path = os.path.join(cache_dir, os.path.basename(archive_url))

    if not os.path.exists(archive_path):
        print(f"Downloading {archive_url} -> {archive_path}")
        urllib.request.urlretrieve(archive_url, archive_path)

    with zipfile.ZipFile(archive_path) as archive:
        names = [name for name in archive.namelist() if os.path.basename(name).lower() == dll_name.lower()]
        if not names:
            raise FileNotFoundError(f"{dll_name} not found in {archive_path}")

        archive.extractall(path=cache_dir)
        extracted_path = os.path.join(cache_dir, dll_name)
        if not os.path.exists(extracted_path):
            extracted_path = os.path.join(cache_dir, names[0])
        if not os.path.exists(extracted_path):
            raise FileNotFoundError(f"Extracted DLL not found for {dll_name} in {archive_path}")

        shutil.copy2(extracted_path, target_path)
        print(f"Extracted {dll_name} -> {target_path}")

        for member in archive.namelist():
            if not member.lower().endswith('.dll'):
                continue
            basename = os.path.basename(member)
            if basename.lower() == dll_name.lower():
                continue
            archive.extract(member, path=cache_dir)
            source_path = os.path.join(cache_dir, member)
            if os.path.exists(source_path):
                shutil.copy2(source_path, os.path.join(target_dir, basename))
                print(f"Extracted helper DLL {basename} -> {target_dir}")

    if os.path.exists(archive_path):
        os.remove(archive_path)
    return target_path


def copy_dlls(env):
    project_dir = env.subst("$PROJECT_DIR")
    build_dir = env.subst("$BUILD_DIR")
    os.makedirs(build_dir, exist_ok=True)

    dll_sources = [
        ensure_runtime_dll(project_dir, "https://github.com/libsdl-org/SDL/releases/download/release-3.4.14/SDL3-3.4.14-win32-x64.zip", "SDL3.dll", "SDL3"),
        ensure_runtime_dll(project_dir, "https://github.com/libsdl-org/SDL_ttf/releases/download/release-3.2.2/SDL3_ttf-3.2.2-win32-x64.zip", "SDL3_ttf.dll", "SDL3_ttf"),
        ensure_runtime_dll(project_dir, "https://github.com/libsdl-org/SDL_image/releases/download/release-3.4.4/SDL3_image-3.4.4-win32-x64.zip", "SDL3_image.dll", "SDL3_image"),
    ]

    for dll in dll_sources:
        if os.path.exists(dll):
            dest = os.path.join(build_dir, os.path.basename(dll))
            if not os.path.exists(dest):
                shutil.copy2(dll, dest)
                print(f"Copied {os.path.basename(dll)} -> {build_dir}")

    ensure_import_library(project_dir, "SDL3.dll", "SDL3")
    ensure_import_library(project_dir, "SDL3_ttf.dll", "SDL3_ttf")
    ensure_import_library(project_dir, "SDL3_image.dll", "SDL3_image")


copy_dlls(env)