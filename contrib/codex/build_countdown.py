#!/usr/bin/env python3
"""Live countdown to the Restoration fork (block 1,000,000) for 23skidoo.info.
Computes ETA from a rolling height/time window. Run on a 1-minute cron."""
import subprocess, json, os, time

# SGF-HEADER-CONSTANTS
HEADER_HTML = """<header class="site-header">
<h1><a href="/">Cthulhu<img src="/static/img/off_240x240.png" alt="Offerings to Cthulhu — home"></a>Offerings</h1>
<p class="site-tagline">The time draws near.<br>The return of The Great Old One is upon us.</p>
</header>"""

HEADER_CSS = """

.site-header{text-align:center;margin:0 0 1.4em;padding-top:6px}
.site-header h1{font-size:2rem;letter-spacing:.04em;margin:0;font-weight:normal;text-shadow:0 0 22px rgba(95,208,160,.4);color:var(--ink,#cfeee0)}
.site-header h1 a{color:inherit;text-decoration:none;border:none !important;display:inline-block}
.site-header h1 a:hover{text-decoration:none;filter:brightness(1.1)}
.site-header h1 img{vertical-align:middle;max-width:130px;height:auto;margin:0 .3em;filter:drop-shadow(0 0 18px rgba(95,208,160,.25))}
.site-tagline{text-align:center;font-style:italic;color:#FFD700;border-top:1px solid #16302a;border-bottom:1px solid #16302a;padding:.85em 0;margin:1.2em auto 2em;max-width:720px;letter-spacing:.04em;font-size:.95rem;line-height:1.55}
@media (max-width:640px){.site-header h1{font-size:1.3rem}.site-header h1 img{max-width:64px;margin:0 .2em}.site-tagline{font-size:.78rem;padding:.55em 0;margin:.9em auto 1.2em}.wrap{padding:18px 14px 60px}}
"""

FOOTER_HTML = """<footer class="site-foot">
<p>&copy; 2026 <a href="https://subgenius.finance/">SubGenius.Finance</a></p>
<p><a href="https://subgenius.finance/">SubGenius.Finance</a>: Where Sub-Culture Becomes Capital</p>
<p class="off-chant">ph&rsquo;nglui mglw&rsquo;nafh Cthulhu R&rsquo;lyeh wgah&rsquo;nagl fhtagn &middot; Praise Cthulhu</p>
<ul class="off-socials" aria-label="Offerings to Cthulhu community links">
<li><a href="https://23skidoo.info/" target="_blank" rel="noopener" aria-label="Offerings to Cthulhu"><img src="https://23skidoo.info/static/img/off_240x240.png" alt="Offerings to Cthulhu"></a></li>
<li><a href="https://pool.23skidoo.info/" target="_blank" rel="noopener" aria-label="OFF Mining Pool"><img src="https://api.iconify.design/mdi/pickaxe.svg?color=%235fffd0" alt="Pool"></a></li>
<li><a href="https://23skidoo.info/codex/" target="_blank" rel="noopener" aria-label="The Codex"><img src="https://cdn.simpleicons.org/bookstack/5fffd0" alt="Codex"></a></li>
<li><a href="https://github.com/SubGeniusFinance/Offerings-to-Cthulhu" target="_blank" rel="noopener" aria-label="GitHub"><img src="https://cdn.simpleicons.org/github/5fffd0" alt="GitHub"></a></li>
</ul>
</footer>"""

FOOTER_CSS = """
.site-foot{text-align:center;color:var(--accent,#5fd0a0);font-size:.82rem;margin:3.2em auto 0;padding-top:1.8em;border-top:1px solid #16302a;line-height:1.85;max-width:720px}
.site-foot p{margin:.35em 0}
.site-foot a{color:var(--accent,#5fd0a0);text-decoration:none;border:none}
.site-foot a:hover{text-decoration:none;filter:brightness(1.1)}
@media (max-width:640px){.site-foot{font-size:.74rem;margin-top:2em}}

.site-foot .off-chant { font-style:italic; letter-spacing:.04em; }
.site-foot .off-socials { list-style:none; padding:0;
  display:flex; flex-wrap:wrap; gap:1.4em;
  position:fixed; bottom:1.2em; right:1.2em; z-index:100; margin:0; }
.site-foot .off-socials li { margin:0; padding:0; }
.site-foot .off-socials a { display:inline-flex; align-items:center;
  border:0 !important; text-decoration:none; }
.site-foot .off-socials img { height:28px; width:28px; opacity:.82;
  transition:opacity .2s, transform .2s; }
.site-foot .off-socials a:hover img { opacity:1; transform:translateY(-1px); }
@media (max-width:640px) { .site-foot .off-socials { gap:1em; }
  .site-foot .off-socials img { height:22px; width:22px; } }
"""

MQ_COUNTDOWN = "@media (max-width:640px){h1{font-size:1.5rem}.chant{font-size:.9rem;margin-bottom:1em}.countdown{padding:.85em 1em;gap:.3em;border-radius:36px;width:100%}.cd-blocks{font-size:1.35rem;letter-spacing:.06em;white-space:nowrap}.cd-tick{font-size:.82rem;letter-spacing:.16em;white-space:nowrap}.eta{font-size:.86rem;margin-bottom:.9em}.grid{gap:6px}.cell{min-width:0;padding:9px 11px;flex:1 1 90px}.cell b{font-size:1rem}.rules{padding:12px 16px;font-size:.9rem}}"



STATE_DIR = "/home/btcbob/codex"   # runtime cache (gitignored by location)
CLI   = "/home/btcbob/claude/offerings-master/src/Offerings-cli"
FORK  = 1000000
OUT   = "/var/www/23skidoo.info/awakening/index.html"
CACHE = os.path.join(STATE_DIR, "countdown_cache.json")

def cli(*a):
    return subprocess.check_output([CLI, *a], text=True, timeout=30).strip()

now = int(time.time())
tip = int(cli("getblockcount"))

# rolling samples for rate estimation
samples = []
if os.path.exists(CACHE):
    try: samples = json.load(open(CACHE))
    except Exception: samples = []
samples.append([now, tip])
samples = [s for s in samples if s[0] >= now - 6*3600][-400:]   # keep last 6h
json.dump(samples, open(CACHE, "w"))

rate = None  # blocks per second
if len(samples) >= 2 and samples[-1][1] > samples[0][1]:
    dt = samples[-1][0] - samples[0][0]
    dh = samples[-1][1] - samples[0][1]
    if dt > 0 and dh > 0:
        rate = dh / dt
if not rate:
    rate = 1.0 / 60.0   # fall back to the 60s block target

remaining = max(0, FORK - tip)
eta_secs  = remaining / rate if rate > 0 else 0
eta_epoch = now + eta_secs
eta_str   = time.strftime("%Y-%m-%d %H:%M UTC", time.gmtime(eta_epoch))
days      = eta_secs / 86400.0
bpm       = rate * 60.0
progress  = min(100.0, 100.0 * (tip - 0) / FORK) if FORK else 0
# progress within the *restoration window* feels better: from a recent baseline
BASE = 966413
win_prog = min(100.0, max(0.0, 100.0 * (tip - BASE) / (FORK - BASE)))

page = f"""<!DOCTYPE html>
<html lang="en"><head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<meta http-equiv="refresh" content="60">
<title>The Awakening &mdash; Offerings to Cthulhu</title>
<style>
  :root {{ --ink:#9fe8c9; --dim:#5fffd0; --panel:#0a1014; --accent:#5fd0a0; --warn:#5fffd0; }}
  *{{box-sizing:border-box}}
  html {{ background-color:#05080a; }}
  body {{ margin:0; min-height:100vh; background:transparent; position:relative;
    color:var(--ink); font-family:"Iowan Old Style","Palatino Linotype",Georgia,serif; line-height:1.6; }}
  /* mobile-safe full-bleed background via fixed pseudo-element */
  body::before {{ content:""; position:fixed; top:0; left:0;
    width:100vw; height:100vh; z-index:-1;
    background:linear-gradient(rgba(5,8,10,0.70), rgba(5,8,10,0.88)), url("/static/img/cthulhu-bg.jpg") center / cover no-repeat; }}
  .wrap {{ max-width:760px; margin:0 auto; padding:56px 24px 90px; text-align:center; }}
  .eyebrow {{ letter-spacing:.32em; text-transform:uppercase; font-size:.74rem; color:var(--dim); }}
  h1 {{ font-size:2.6rem; margin:.25em 0 .1em; text-shadow:0 0 22px rgba(95,208,160,.4); }}
  .chant {{ font-style:italic; color:var(--dim); margin-bottom:2.2em; }}
  .countdown {{ display:flex; flex-direction:column; align-items:center; justify-content:center;
    gap:.35em; width:min(620px,100%); margin:.6em auto 1.6em; padding:1.15em 2.2em;
    background:linear-gradient(180deg, rgba(10,16,20,.92), rgba(8,12,16,.92));
    border:1px solid rgba(95,208,160,.35); border-radius:999px;
    box-shadow: inset 0 0 26px rgba(95,208,160,.16), 0 0 42px rgba(95,208,160,.22);
    font-family:"JetBrains Mono","SF Mono",Menlo,Consolas,monospace; }}
  .cd-blocks {{ font-size:2.2rem; color:var(--accent); letter-spacing:.10em; line-height:1;
    text-shadow:0 0 18px rgba(95,208,160,.55); }}
  .cd-tick {{ font-size:1.05rem; color:var(--ink); letter-spacing:.22em; line-height:1;
    opacity:.9; }}
  .eta {{ color:var(--warn); font-size:1.05rem; margin-bottom:1.8em; }}
  .bar {{ height:14px; background:#0c1714; border:1px solid #16302a; border-radius:8px; overflow:hidden; margin:1.4em 0 .4em; }}
  .bar > i {{ display:block; height:100%; width:{win_prog:.3f}%;
    background:linear-gradient(90deg,#1f6f55,var(--accent)); box-shadow:0 0 16px var(--accent); }}
  .barlabel {{ font-size:.8rem; color:var(--dim); margin-bottom:2.6em; }}
  .grid {{ display:flex; gap:14px; justify-content:center; flex-wrap:wrap; margin-bottom:2.4em; }}
  .cell {{ background:var(--panel); border:1px solid #16302a; border-radius:10px; padding:14px 20px; min-width:150px; }}
  .cell b {{ display:block; font-family:monospace; font-size:1.5rem; color:var(--accent); }}
  .cell span {{ font-size:.7rem; text-transform:uppercase; letter-spacing:.12em; color:var(--dim); }}
  .rules {{ text-align:left; background:var(--panel); border:1px solid #16302a; border-radius:10px;
    padding:18px 26px; margin:0 auto 2em; max-width:620px; }}
  .rules li {{ margin:.4em 0; }}
  a {{ color:var(--accent); }}
  .foot {{ color:var(--dim); font-size:.8rem; font-style:italic; margin-top:2em; }}
{MQ_COUNTDOWN}{HEADER_CSS}{FOOTER_CSS}</style></head>
<body><div class="wrap">{HEADER_HTML}
  <div class="eyebrow">SubGenius.Finance &mdash; The Conclave</div>
  <h1>The Awakening</h1>
  <div class="chant">Restoration Hardfork v2.0.0 activates at block 1,000,000.</div>

  <div class="countdown"><span class="cd-blocks">{remaining:,} blocks</span><span class="cd-tick" id="cd-tick">&nbsp;</span></div>
  <div class="eta">ESTIMATED AWAKENING:<br><strong>{eta_str}</strong> &nbsp;({days:.1f} days)</div>

  <div class="bar"><i></i></div>
  <div class="barlabel">{win_prog:.2f}% of the Restoration window mined &mdash; block {tip:,} of 1,000,000</div>

  <div class="grid">
    <div class="cell"><b>{tip:,}</b><span>current height</span></div>
    <div class="cell"><b>{remaining:,}</b><span>blocks remaining</span></div>
    <div class="cell"><b>{bpm:.2f}</b><span>blocks / min</span></div>
  </div>

  <div class="rules">
    <div class="eyebrow" style="margin-bottom:.6em">What activates at the fork</div>
    <ul>
      <li>&#128274; Subsidy locks at <strong>1.5 OFF/block</strong> forever.</li>
      <li>&#128683; Eight <strong>attacker addresses banned</strong> in consensus forever.</li>
      <li>&#128367;&#65039; One-time <strong>150,000 OFF Restoration Tithe</strong> in the fork block.</li>
      <li>&#9876;&#65039; Coinbase splits <strong>7/8 miner, 1/8 Conclave Treasury</strong>.</li>
      <li>&#128370;&#65039; <strong>The Reclamation</strong> opens &mdash; open invitation for anyone who mined or held OFF on the original chain. Restitution + Worshipper Recognition (scaled by earliness &amp; depth); no calendar deadline. <a href="/bridge/">How it works &rarr;</a></li>
      <li>&#127744; Once the canon completes (~33 days past fork)<br>Conclave Pool&#39;s miner enters <strong>Phase B: The Dreaming</strong><br>Pool-solved blocks speak hash-seeded generative R&#39;lyehian; outsider-mined blocks stay silent.</li>
    </ul>
  </div>

  <p>The chain is already reciting the rite &mdash; read it assembling, block by block, in
     <a href="/codex/">The Chain Codex</a>.</p>

  <div class="foot">Updated {time.strftime('%Y-%m-%d %H:%M:%S UTC', time.gmtime(now))} &middot; ETA from a live {len(samples)}-sample rate window.<br>
    <em>Ph&#39;nglui mglw&#39;nafh Cthulhu R&#39;lyeh wgah&#39;nagl fhtagn.</em></div>
{FOOTER_HTML}
</div>
<script>
  // smooth client-side tick toward the ETA epoch
  var eta = {int(eta_epoch)} * 1000;
  function fmt(ms) {{
    if (ms <= 0) return "THE STARS ARE RIGHT";
    var s = Math.floor(ms/1000), d=Math.floor(s/86400); s-=d*86400;
    var h=Math.floor(s/3600); s-=h*3600; var m=Math.floor(s/60); s-=m*60;
    return d+"d "+h+"h "+m+"m "+s+"s";
  }}
  var tick = document.getElementById('cd-tick');
  setInterval(function(){{
    tick.textContent = fmt(eta - Date.now());
  }}, 1000);
</script>
</body></html>
"""
tmp = OUT + ".tmp"
open(tmp, "w").write(page)
os.replace(tmp, OUT)
print(f"countdown built: tip={tip} remaining={remaining} eta={eta_str} bpm={bpm:.2f}")
