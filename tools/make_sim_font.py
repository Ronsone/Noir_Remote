"""Rebuild the simulator's icon font (simulator/fonts/mdi20.c) from the glyphs the firmware uses (ico20 in
firmware/noir-remote.yaml). Run from anywhere after tools/check_icons.py --fix. Needs Node.js (npx)."""
import pathlib
import re
import subprocess

ROOT = pathlib.Path(__file__).resolve().parent.parent
yaml = (ROOT / "firmware/noir-remote.yaml").read_text(encoding="utf-8")
seg = yaml[yaml.index("id: ico20"):]
seg = seg[:seg.index("]")]
codes = sorted(set(re.findall(r"\\U000(F[0-9A-F]{4})", seg)))
subprocess.run(["npx", "-y", "lv_font_conv@1.5.2", "--font", "firmware/fonts/mdi-subset.ttf",
                "-r", ",".join("0x" + c.lower() for c in codes), "--size", "20", "--bpp", "4", "--format", "lvgl",
                "--no-compress", "-o", "simulator/fonts/mdi20.c", "--lv-include", "lvgl.h"], cwd=ROOT, check=True)
print(f"simulator/fonts/mdi20.c: {len(codes)} icons")
