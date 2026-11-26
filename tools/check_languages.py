"""Check the language packs against the English reference: what is missing, what is left over.

    python3 tools/check_languages.py          all languages
    python3 tools/check_languages.py it       one language
"""
import json
import pathlib
import sys

DIR = pathlib.Path(__file__).resolve().parent.parent / "custom_components/noir_remote/languages"
ref = json.loads((DIR / "en.json").read_text(encoding="utf-8"))
codes = sys.argv[1:] or sorted(p.stem for p in DIR.glob("*.json") if p.stem != "en")
ok = True
for code in codes:
    f = DIR / f"{code}.json"
    if not f.exists():
        print(f"{code}: no file {f.name} - copy en.json to start")
        ok = False
        continue
    pack = json.loads(f.read_text(encoding="utf-8"))
    print(f"{code} ({pack.get('name', '?')})")
    for part in ("builder", "remote"):
        have, want = pack.get(part, {}), ref[part]
        missing = [k for k in want if not str(have.get(k, "")).strip()]
        extra = [k for k in have if k not in want]
        long_ = [k for k in want if part == "remote" and len(str(have.get(k, ""))) > max(28, 2 * len(want[k]))]
        print(f"  {part}: {len(want) - len(missing)}/{len(want)} translated")
        for k in missing:
            print(f"    missing:  {k!r}  (English: {want[k]!r})")
        for k in extra:
            print(f"    not used: {k!r}")
        for k in long_:
            print(f"    long for the screen? {k!r} -> {have[k]!r}")
        ok = ok and not missing
sys.exit(0 if ok else 1)
