Import("env")
from os.path import join

def _make_hex(source, target, env):
    build_dir = env.subst("$BUILD_DIR")
    elf = join(build_dir, "firmware.elf")
    hex_ = join(build_dir, "firmware.hex")
    objcopy = env.subst("$OBJCOPY")
    env.Execute(f'"{objcopy}" -O ihex "{elf}" "{hex_}"')

env.AddPostAction("$BUILD_DIR/firmware.elf", _make_hex)