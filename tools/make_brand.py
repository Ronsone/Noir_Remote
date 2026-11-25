"""Make the brand images Home Assistant and HACS show for the integration.

Writes custom_components/noir_remote/brand/{icon,icon@2x,logo,logo@2x,dark_logo,dark_logo@2x}.png
Run from the repository root: python3 tools/make_brand.py
"""
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent.parent
FONT = ROOT / "firmware/fonts/Inter-Regular.ttf"
OUT = ROOT / "custom_components/noir_remote/brand"

BG_TOP = (30, 31, 35)
BG_BOTTOM = (8, 8, 10)
WHITE = (245, 245, 247)
INK = (17, 17, 20)
ACCENT = (110, 190, 220)
SS = 4  # supersampling for smooth edges


def icon(size: int) -> Image.Image:
    s = size * SS
    grad = Image.new("RGB", (1, s))
    for y in range(s):
        t = y / (s - 1)
        grad.putpixel((0, y), tuple(round(a + (b - a) * t) for a, b in zip(BG_TOP, BG_BOTTOM)))
    tile = grad.resize((s, s))
    mask = Image.new("L", (s, s), 0)
    ImageDraw.Draw(mask).rounded_rectangle((0, 0, s - 1, s - 1), radius=int(s * 0.22), fill=255)
    img = Image.new("RGBA", (s, s), (0, 0, 0, 0))
    img.paste(tile, (0, 0), mask)

    d = ImageDraw.Draw(img)
    font = ImageFont.truetype(str(FONT), int(s * 0.58))
    l, t, r, b = d.textbbox((0, 0), "N", font=font)
    x = (s - (r - l)) / 2 - l
    y = s * 0.44 - (b - t) / 2 - t
    d.text((x, y), "N", font=font, fill=WHITE)

    w, h, top = s * 0.30, max(SS, s * 0.022), s * 0.80
    d.rounded_rectangle(((s - w) / 2, top, (s + w) / 2, top + h), radius=h / 2, fill=ACCENT)
    return img.resize((size, size), Image.LANCZOS)


def logo(height: int, color) -> Image.Image:
    h = height * SS
    font = ImageFont.truetype(str(FONT), int(h * 0.62))
    tracking = int(h * 0.16)
    probe = ImageDraw.Draw(Image.new("L", (1, 1)))
    widths = [probe.textlength(c, font=font) for c in "NOIR"]
    pad = int(h * 0.08)
    w = int(sum(widths) + tracking * 3 + pad * 2)
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    l, t, r, b = d.textbbox((0, 0), "N", font=font)
    y = h * 0.42 - (b - t) / 2 - t
    x = pad
    for c, cw in zip("NOIR", widths):
        d.text((x, y), c, font=font, fill=color)
        x += cw + tracking
    lw, lh, top = h * 0.55, max(SS, h * 0.03), h * 0.84
    d.rounded_rectangle((pad, top, pad + lw, top + lh), radius=lh / 2, fill=ACCENT)
    return img.resize((w // SS, height), Image.LANCZOS)


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    icon(256).save(OUT / "icon.png", optimize=True)
    icon(512).save(OUT / "icon@2x.png", optimize=True)
    for name, color in (("logo", INK), ("dark_logo", WHITE)):
        logo(128, color).save(OUT / f"{name}.png", optimize=True)
        logo(256, color).save(OUT / f"{name}@2x.png", optimize=True)
    print("brand images written to", OUT)


if __name__ == "__main__":
    main()
