# Icons: 24-unit, stroke 1.5, round caps (same feel as Lucide home icons)
ICON = {
 'unit': '<rect x="3" y="4" width="18" height="7.5" rx="2"/><path d="M6.5 9h11"/><path d="M8 14.5c-.9 1.3-.9 2.7 0 4"/><path d="M12 14.5c-.9 1.3-.9 2.7 0 4 .9 1.3.9 2.2 0 3"/><path d="M16 14.5c-.9 1.3-.9 2.7 0 4"/>',
 'flake': '<path d="M12 3v18"/><path d="M4.2 7.5l15.6 9"/><path d="M4.2 16.5l15.6-9"/><path d="M9.8 4.6L12 6.8l2.2-2.2"/><path d="M9.8 19.4L12 17.2l2.2 2.2"/>',
 'breeze': '<path d="M3 8.5h10.5a2.5 2.5 0 1 0-2.5-2.5"/><path d="M3 12.5h15a2.5 2.5 0 1 1-2.5 2.5"/><path d="M3 16.5h7"/>',
}
MODE = {  # mode icons for the airco screen (no text)
 'auto': '<circle cx="12" cy="12" r="8"/><path d="M9 15l3-7 3 7"/><path d="M10.2 12.6h3.6"/>',
 'cool': ICON['flake'],
 'heat': '<circle cx="12" cy="12" r="3.5"/><path d="M12 3v2M12 19v2M3 12h2M19 12h2M5.6 5.6l1.4 1.4M17 17l1.4 1.4M5.6 18.4L7 17M17 7l1.4-1.4"/>',
 'dry': '<path d="M12 3.5c3 3.6 5 6.4 5 9a5 5 0 0 1-10 0c0-2.6 2-5.4 5-9z"/>',
 'fan': ICON['breeze'],
}
def svg(body, size=21, color='#E4E7EC', sw=1.5):
    return f'<svg width="{size}" height="{size}" viewBox="0 0 24 24" fill="none" stroke="{color}" stroke-width="{sw}" stroke-linecap="round" stroke-linejoin="round">{body}</svg>'
HOME_I = {
 'tv':'<rect x="3" y="5" width="18" height="12" rx="2"/><path d="M8 21h8M12 17v4"/>',
 'audio':'<path d="M2 10v3M6 6v11M10 3v18M14 8v7M18 5v13M22 10v3"/>',
 'light':'<path d="M9 18h6M10 22h4M12 2a7 7 0 0 0-4 12.7V16h8v-1.3A7 7 0 0 0 12 2z"/>',
 'scenes':'<path d="M12 2 2 7l10 5 10-5z"/><path d="m2 17 10 5 10-5"/><path d="m2 12 10 5 10-5"/>',
}
def status(): return '<div class="sb"><span>20:53</span><span>78%</span></div>'
def tile(x,y,w,h,ic,name):
    return f'<div class="t" style="left:{x}px;top:{y}px;width:{w}px;height:{h}px">{svg(HOME_I[ic])}<div class="tn">{name}</div></div>'
def home(airco_html):
    return f'''<div class="ph">{status()}<div class="b">
<div class="h1">NOIR</div><div class="sub">Thuis</div>
{tile(0,52,104,56,'tv','TV')}{tile(112,52,104,56,'audio','Audio')}
{tile(0,114,104,56,'light','Verlichting')}{tile(112,114,104,56,'scenes','Scènes')}
{airco_html}</div>{bottom()}</div>'''
def bottom(): return '<div class="bot"><div class="bb">← Terug</div><div class="bb">⌂ Thuis</div></div>'

# ---------- Variant A: Stil (glow + live temp) ----------
A_tile = f'''<div class="t aA" style="left:0;top:176px;width:216px;height:50px">
<div style="position:absolute;left:12px;top:14px">{svg(ICON['unit'])}</div>
<div class="tn" style="left:42px;top:9px;position:absolute">Airco</div>
<div class="mut" style="left:42px;top:28px;position:absolute">Koelen</div>
<div class="tbig" style="right:14px;top:8px">21<span class="deg">°</span></div></div>'''
def modes(active, style=''):
    s=''
    for i,m in enumerate(['auto','cool','heat','dry','fan']):
        on = m==active
        s+=f'<div class="md{" on" if on else ""}" style="left:{i*44}px">{svg(MODE[m],20,"#0B0D10" if on else "#BFC2C9")}</div>'
    return f'<div style="position:absolute;top:{style or 150}px;left:0;width:216px;height:36px">{s}</div>'
def pills(y):
    return f'''<div class="pl" style="top:{y}px;left:0">{svg(ICON['breeze'],16,'#BFC2C9')}<span>Auto</span></div>
<div class="pl" style="top:{y}px;left:112px">{svg('<path d="M4 7c4-3 12-3 16 0M6 12c3-2 9-2 12 0M8 17c2-1 6-1 8 0"/>',16,'#BFC2C9')}<span>Swing</span></div>'''
A_screen = f'''<div class="ph">{status()}<div class="b">
<div class="h1">Airco</div><div class="sub">Koelen · kamer 23°</div>
<div class="pw on" style="left:164px;top:0">{svg('<path d="M12 3v8"/><path d="M6.3 6.8a8 8 0 1 0 11.4 0"/>',18,'#0B0D10')}</div>
<div class="glowA"></div>
<div class="pm" style="left:0;top:66px">{svg('<path d="M6 12h12"/>',22)}</div>
<div class="bigT" style="top:58px">21<span class="deg2">°</span></div>
<div class="pm" style="left:166px;top:66px">{svg('<path d="M6 12h12M12 6v12"/>',22)}</div>
{modes('cool')}{pills(198)}</div>{bottom()}</div>'''

# ---------- Variant B: Boog (arc dial) ----------
def arc(cx,cy,r,frac,sw,col='#8FD3FF',track='rgba(255,255,255,.08)'):
    import math
    a0, sweep = 135, 270
    def pt(a): return (cx+r*math.cos(math.radians(a)), cy+r*math.sin(math.radians(a)))
    x0,y0=pt(a0); x1,y1=pt(a0+sweep); xe,ye=pt(a0+sweep*frac)
    big = 1 if sweep*frac>180 else 0
    return (f'<path d="M{x0:.1f} {y0:.1f} A{r} {r} 0 1 1 {x1:.1f} {y1:.1f}" stroke="{track}" stroke-width="{sw}" fill="none" stroke-linecap="round"/>'
            f'<path d="M{x0:.1f} {y0:.1f} A{r} {r} 0 {big} 1 {xe:.1f} {ye:.1f}" stroke="url(#g)" stroke-width="{sw}" fill="none" stroke-linecap="round"/>')
GRAD='<defs><linearGradient id="g" x1="0" x2="1"><stop offset="0" stop-color="#5AA9FF"/><stop offset="1" stop-color="#B8F0FF"/></linearGradient></defs>'
B_tile = f'''<div class="t" style="left:0;top:176px;width:216px;height:50px">
<div style="position:absolute;left:12px;top:14px">{svg(ICON['flake'])}</div>
<div class="tn" style="left:42px;top:9px;position:absolute">Airco</div>
<div class="mut" style="left:42px;top:28px;position:absolute">Koelen</div>
<svg style="position:absolute;right:8px;top:3px" width="44" height="44" viewBox="0 0 44 44">{GRAD}{arc(22,23,17,.45,3)}<text x="22" y="28" text-anchor="middle" font-size="14" fill="#F2F3F5" font-family="Inter">21</text></svg></div>'''
B_screen = f'''<div class="ph">{status()}<div class="b">
<div class="h1">Airco</div><div class="sub">Koelen · kamer 23°</div>
<div class="pw on" style="left:164px;top:0">{svg('<path d="M12 3v8"/><path d="M6.3 6.8a8 8 0 1 0 11.4 0"/>',18,'#0B0D10')}</div>
<svg style="position:absolute;left:43px;top:44px" width="130" height="112" viewBox="0 0 130 112">{GRAD}{arc(65,62,50,.45,6)}</svg>
<div class="bigT" style="top:70px;font-size:44px">21<span class="deg2">°</span></div>
<div class="pm sm" style="left:0;top:96px">{svg('<path d="M6 12h12"/>',20)}</div>
<div class="pm sm" style="left:176px;top:96px">{svg('<path d="M6 12h12M12 6v12"/>',20)}</div>
{modes('cool',158)}{pills(202)}</div>{bottom()}</div>'''

# ---------- Variant C: IJslijn (accent line + breeze) ----------
C_tile = f'''<div class="t aC" style="left:0;top:176px;width:216px;height:50px">
<div style="position:absolute;left:12px;top:14px">{svg(ICON['breeze'])}</div>
<div class="tn" style="left:42px;top:15px;position:absolute">Airco</div>
<div class="tbig" style="right:14px;top:8px;color:#CFEFFF">21<span class="deg">°</span></div>
<div class="lineC"></div></div>'''
C_screen = f'''<div class="ph">{status()}<div class="b">
<div class="h1">Airco</div><div class="sub">Koelen · kamer 23°</div>
<div class="pw on" style="left:164px;top:0">{svg('<path d="M12 3v8"/><path d="M6.3 6.8a8 8 0 1 0 11.4 0"/>',18,'#0B0D10')}</div>
<div class="card" style="top:50px">
 <div class="bigT" style="top:6px;font-size:52px;left:0;width:150px;text-align:left;padding-left:16px">21<span class="deg2">°</span></div>
 <div class="pmv" style="right:8px;top:6px">{svg('<path d="M6 12h12M12 6v12"/>',18)}</div>
 <div class="pmv" style="right:8px;top:48px">{svg('<path d="M6 12h12"/>',18)}</div>
 <div class="lineC" style="left:16px;right:60px;bottom:10px"></div></div>
{modes('cool',150)}{pills(198)}</div>{bottom()}</div>'''

def col(title, desc, a, b):
    return f'<div class="col"><div class="vt">{title}</div><div class="vd">{desc}</div><div class="row">{a}{b}</div></div>'

ICONROW = ''.join(f'<div class="ic"><div class="icb">{svg(ICON[k],42,"#E4E7EC",1.5)}</div><div class="vd" style="text-align:center">{n}</div></div>' for k,n in [('unit','1 · Unit + lucht'),('flake','2 · Dunne vlok'),('breeze','3 · Bries')])

html = f'''<!DOCTYPE html><html><head><style>
@font-face{{font-family:Inter;src:url(Inter-Regular.ttf)}}
body{{margin:0;background:#0c0d10;font-family:Inter;color:#F2F3F5;padding:28px}}
.ph{{width:240px;height:320px;position:relative;background:url(obsidian.png);overflow:hidden;border-radius:6px;box-shadow:0 0 0 1px #222}}
.sb{{position:absolute;top:6px;left:12px;right:12px;display:flex;justify-content:space-between;font-size:11px;color:#BFC2C9}}
.b{{position:absolute;left:12px;top:26px;width:216px;height:240px}}
.h1{{font-size:20px;position:absolute;top:0}} .sub{{font-size:12px;color:#8A8F98;position:absolute;top:26px}}
.t{{position:absolute;background:rgba(26,28,32,.88);border:1px solid rgba(255,255,255,.07);border-radius:12px;box-sizing:border-box;padding:8px 10px}}
.t>svg{{display:block}} .tn{{font-size:14px;margin-top:4px}} .mut{{font-size:11px;color:#8A8F98}}
.tbig{{position:absolute;font-size:28px;font-weight:300;letter-spacing:-1px}} .deg{{font-size:16px;vertical-align:top;color:#8A8F98}}
.aA{{background:radial-gradient(120px 60px at 100% 0%,rgba(110,190,255,.22),transparent 70%),rgba(26,28,32,.9);border-color:rgba(150,210,255,.18)}}
.aC{{overflow:hidden}} .lineC{{position:absolute;left:12px;right:12px;bottom:0;height:2px;border-radius:2px;background:linear-gradient(90deg,#5AA9FF,#B8F0FF 60%,transparent)}}
.bot{{position:absolute;left:12px;right:12px;bottom:8px;display:flex;gap:8px}} .bb{{flex:1;height:38px;border-radius:12px;background:rgba(26,28,32,.88);border:1px solid rgba(255,255,255,.07);display:flex;align-items:center;justify-content:center;font-size:13px}}
.pw{{position:absolute;width:52px;height:34px;border-radius:17px;display:flex;align-items:center;justify-content:center;background:rgba(26,28,32,.9);border:1px solid rgba(255,255,255,.1)}}
.pw.on{{background:#E9F6FF}}
.pm{{position:absolute;width:50px;height:50px;border-radius:25px;background:rgba(26,28,32,.9);border:1px solid rgba(255,255,255,.1);display:flex;align-items:center;justify-content:center}}
.pm.sm{{width:40px;height:40px;border-radius:20px}}
.pmv{{position:absolute;width:40px;height:36px;border-radius:12px;background:rgba(255,255,255,.05);display:flex;align-items:center;justify-content:center}}
.bigT{{position:absolute;left:54px;width:108px;text-align:center;font-size:56px;font-weight:300;letter-spacing:-2px;line-height:1}}
.deg2{{font-size:22px;vertical-align:top;color:#8FD3FF;margin-left:2px}}
.glowA{{position:absolute;left:38px;top:40px;width:140px;height:100px;background:radial-gradient(closest-side,rgba(110,190,255,.20),transparent)}}
.md{{position:absolute;width:40px;height:36px;border-radius:12px;background:rgba(26,28,32,.9);border:1px solid rgba(255,255,255,.07);display:flex;align-items:center;justify-content:center;box-sizing:border-box}}
.md.on{{background:linear-gradient(135deg,#B8F0FF,#7CC4FF);border:none}}
.pl{{position:absolute;width:104px;height:34px;border-radius:12px;background:rgba(26,28,32,.9);border:1px solid rgba(255,255,255,.07);display:flex;gap:6px;align-items:center;justify-content:center;font-size:12px;color:#E4E7EC;box-sizing:border-box}}
.card{{position:absolute;left:0;width:216px;height:88px;border-radius:14px;background:rgba(26,28,32,.9);border:1px solid rgba(150,210,255,.14);overflow:hidden}}
.grid{{display:flex;gap:44px}} .col{{}} .row{{display:flex;gap:16px}}
.vt{{font-size:18px;margin-bottom:4px}} .vd{{font-size:12px;color:#8A8F98;margin-bottom:12px;width:496px}}
.icons{{display:flex;gap:28px;margin-bottom:28px}} .ic{{width:130px}} .ic .vd{{width:130px}} .icb{{width:130px;height:90px;border-radius:14px;background:#17191d;display:flex;align-items:center;justify-content:center;margin-bottom:6px}}
</style></head><body>
<div class="vt" style="font-size:22px;margin-bottom:14px">Airco · NOIR-varianten</div>
<div class="icons">{ICONROW}</div>
<div class="grid">
{col('A · Stil', 'Unit-icoon, zachte gloed, live temperatuur. Scherm: groot getal, modi als icoon.', home(A_tile), A_screen)}
{col('B · Boog', 'Dunne vlok, mini-boog op de tegel. Scherm: draaiknop-boog rond de temperatuur.', home(B_tile), B_screen)}
{col('C · IJslijn', 'Bries-icoon, ijsblauwe lijn als accent. Scherm: één kaart met getal en +/−.', home(C_tile), C_screen)}
</div></body></html>'''
open('index.html','w').write(html)
