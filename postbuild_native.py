Import("env")
import shutil
import os

def copy_dlls(source, target, env):
    project_dir = env.subst("$PROJECT_DIR")
    output_dir = os.path.dirname(str(target[0]))

    dll_sources = [
        os.path.join(project_dir, "external", "SDL3", "x86_64-w64-mingw32", "bin", "SDL3.dll"),
        os.path.join(project_dir, "external", "SDL_ttf", "x86_64-w64-mingw32", "bin", "SDL3_ttf.dll"),
        os.path.join(project_dir, "external", "SDL3_image", "x86_64-w64-mingw32", "bin", "SDL3_image.dll"),
    ]
    for dll in dll_sources:
        dest = os.path.join(output_dir, os.path.basename(dll))
        if os.path.exists(dest):
            continue
        if os.path.exists(dll):
            shutil.copy2(dll, output_dir)
            print(f"Copied {os.path.basename(dll)} -> {output_dir}")
        else:
            print(f"WARNING: expected DLL not found at {dll}")

env.AddPostAction("$PROGPATH", copy_dlls)