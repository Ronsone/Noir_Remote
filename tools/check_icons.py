"""Check that every icon the screens use is in the remote's icon font, and list font glyphs nobody uses.

    python3 tools/check_icons.py          report
    python3 tools/check_icons.py --fix    rewrite the glyph list of ico20 in firmware/core/noir-core.yaml
"""
import pathlib
import re
import sys

FW = pathlib.Path(__file__).resolve().parent.parent / "firmware"
CORE = FW / "core"
yaml_path = CORE / "noir-core.yaml"
yaml = yaml_path.read_text(encoding="utf-8")
seg_start = yaml.index("id: ico20")
seg_end = yaml.index("]", yaml.index("glyphs:", seg_start)) + 1
font = set(re.findall(r"\\U000(F[0-9A-F]{4})", yaml[seg_start:seg_end]))

code = "".join(p.read_text(encoding="utf-8") for p in [CORE / "noir-ui.h", *sorted((CORE / "modules").glob("*.h"))])
used = set(re.findall(r"\\U000(F[0-9A-F]{4})", code))
table = {c.upper() for c in re.findall(r"0x(F[0-9A-Fa-f]{4})", (CORE / "mdi_icons.h").read_text(encoding="utf-8"))}
need = used | table

missing, unused = sorted(need - font), sorted(font - need)
print(f"font {len(font)} glyphs, needed {len(need)} (screens {len(used)}, Builder icons {len(table)})")
print("missing in the font:", missing or "none")
print("in the font but not used:", len(unused))
if "--fix" in sys.argv:
    glyphs = ", ".join(f'"\\U000{c}"' for c in sorted(need))
    line_start = yaml.index("glyphs:", seg_start)
    yaml = yaml[:line_start] + f"glyphs: [{glyphs}]" + yaml[seg_end:]
    yaml_path.write_text(yaml, encoding="utf-8")
    print("glyph list rewritten:", len(need))
sys.exit(1 if missing else 0)
