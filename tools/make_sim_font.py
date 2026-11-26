"""Rebuild the simulator's icon font (simulator/fonts/mdi20.c) from the glyphs the firmware uses (ico20 in
firmware/core/noir-core.yaml), and simulator/fonts/mdi-web.ttf for the browser build. Run from anywhere after
tools/check_icons.py --fix. Needs Node.js (npx) and fonttools."""
import pathlib
import re
import subprocess

ROOT = pathlib.Path(__file__).resolve().parent.parent
yaml = (ROOT / "firmware/core/noir-core.yaml").read_text(encoding="utf-8")
seg = yaml[yaml.index("id: ico20"):]
seg = seg[:seg.index("]")]
codes = sorted(set(re.findall(r"\\U000(F[0-9A-F]{4})", seg)))
subprocess.run(["npx", "-y", "lv_font_conv@1.5.2", "--font", "firmware/fonts/mdi-subset.ttf",
                "-r", ",".join("0x" + c.lower() for c in codes), "--size", "20", "--bpp", "4", "--format", "lvgl",
                "--no-compress", "-o", "simulator/fonts/mdi20.c", "--lv-include", "lvgl.h"], cwd=ROOT, check=True)
print(f"simulator/fonts/mdi20.c: {len(codes)} icons")

# The same icons as a small TTF for the browser build (simulator --size / the NOIR phone panel renders them at any size)
try:
    from fontTools import subset
except ImportError:
    print("simulator/fonts/mdi-web.ttf not rebuilt: pip install fonttools")
else:
    opts = subset.Options()
    font = subset.load_font(str(ROOT / "firmware/fonts/mdi-subset.ttf"), opts)
    s = subset.Subsetter(opts)
    s.populate(unicodes=[int(c, 16) for c in codes])
    s.subset(font)
    subset.save_font(font, str(ROOT / "simulator/fonts/mdi-web.ttf"), opts)
    print(f"simulator/fonts/mdi-web.ttf: {len(codes)} icons")
