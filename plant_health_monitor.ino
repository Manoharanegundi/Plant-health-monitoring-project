 #include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

/* -------- WiFi Credentials -------- */


WebServer server(80);

/* -------- DHT22 -------- */
#define DHTPIN   4
#define DHTTYPE  DHT22
DHT dht(DHTPIN, DHTTYPE);

/* -------- Sensor Pins -------- */
#define SOIL_PIN1  34
#define SOIL_PIN2  32
#define LDR_PIN    35

/* -------- Sensor Values -------- */
int   soilValue1  = 0;
int   soilValue2  = 0;
int   lightValue  = 0;
float temperature = 0.0;
float humidity    = 0.0;

/* -------- History -------- */
#define HISTORY_LEN 10
int   soil1History[HISTORY_LEN] = {0};
int   soil2History[HISTORY_LEN] = {0};
int   lightHistory[HISTORY_LEN] = {0};
float tempHistory[HISTORY_LEN]  = {0};
float humHistory[HISTORY_LEN]   = {0};
int   historyIndex = 0;
bool  historyFull  = false;

/* ================================================================
   HTML stored in PROGMEM — never allocated on heap as one block
   Split into 3 parts to stay within compiler string limits
   ================================================================ */

// Part 1: Head + CSS + splash
const char HTML_P1[] PROGMEM = R"rawhtml(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>PlantWatch</title>
<link href="https://fonts.googleapis.com/css2?family=Nunito:wght@400;600;700;800&family=DM+Mono:wght@400;500&display=swap" rel="stylesheet">
<style>
:root{--bg:#0d1a0f;--bg2:#111f13;--sur:#162019;--sur2:#1e2e21;--bdr:rgba(58,191,110,0.13);--txt:#d6edd9;--mut:#607d63;--acc:#3abf6e;--acc2:#2da05c;--lf:#5dd98a;--gld:#d4a017;--org:#e05c2a;--sky:#4aafde}
*{box-sizing:border-box;margin:0;padding:0}
body{background:var(--bg);color:var(--txt);font-family:'Nunito',sans-serif;min-height:100vh;overflow-x:hidden}
body::before{content:'';position:fixed;inset:0;background:radial-gradient(ellipse 80% 60% at 20% 10%,rgba(58,191,110,0.07) 0%,transparent 70%),radial-gradient(ellipse 60% 50% at 80% 80%,rgba(45,160,92,0.05) 0%,transparent 70%);pointer-events:none;z-index:0}

/* SPLASH */
#splash{position:fixed;inset:0;z-index:9999;background:var(--bg);display:flex;flex-direction:column;align-items:center;justify-content:center;gap:0;transition:opacity 0.7s}
#splash.hidden{opacity:0;pointer-events:none}
.sp-logo{font-size:48px;font-weight:800;margin-bottom:6px}.sp-logo span{color:var(--acc)}
.sp-sub{color:var(--mut);font-size:15px;margin-bottom:32px}
.sp-cards{display:grid;grid-template-columns:repeat(3,1fr);gap:14px;width:720px;max-width:94vw;margin-bottom:32px}
.sp-card{border-radius:14px;overflow:hidden;border:1px solid var(--bdr);height:170px;background:var(--sur2);position:relative;display:flex;align-items:center;justify-content:center}
.sp-card svg{width:100%;height:100%}
.sp-tag{position:absolute;bottom:10px;left:10px;background:rgba(0,0,0,0.6);color:var(--lf);font-size:11px;font-weight:700;padding:3px 9px;border-radius:20px;letter-spacing:.8px;text-transform:uppercase}
.sp-ico{position:absolute;top:10px;right:10px;font-size:20px;background:rgba(0,0,0,0.5);border-radius:50%;width:32px;height:32px;display:flex;align-items:center;justify-content:center}
.sp-facts{display:flex;gap:18px;margin-bottom:24px;flex-wrap:wrap;justify-content:center}
.sp-fact{background:var(--sur2);border:1px solid var(--bdr);border-radius:30px;padding:6px 14px;font-size:12px;color:var(--mut)}
.sp-fact span{color:var(--acc);font-weight:700}
.sp-bar-lbl{font-size:12px;color:var(--mut);letter-spacing:1px;text-transform:uppercase;margin-bottom:8px}
.sp-bar{width:240px;height:5px;background:var(--sur2);border-radius:3px;overflow:hidden}
.sp-bar-fill{height:100%;background:linear-gradient(90deg,var(--acc2),var(--lf));width:0%;animation:spLoad 2.6s ease forwards}
@keyframes spLoad{to{width:100%}}

/* NAV */
.topbar{position:sticky;top:0;z-index:100;display:flex;align-items:center;justify-content:space-between;padding:0 22px;height:54px;background:rgba(13,26,15,0.94);backdrop-filter:blur(14px);border-bottom:1px solid var(--bdr)}
.logo{display:flex;align-items:center;gap:9px;font-weight:800;font-size:19px}
.logo-ic{width:30px;height:30px;background:linear-gradient(135deg,var(--acc),var(--acc2));border-radius:7px;display:flex;align-items:center;justify-content:center;font-size:15px}
.logo span{color:var(--acc)}
.nav-r{display:flex;align-items:center;gap:12px;font-size:13px;color:var(--mut)}
.rdot{width:7px;height:7px;border-radius:50%;background:var(--acc);animation:pulse 2s infinite}
@keyframes pulse{0%,100%{opacity:1;transform:scale(1)}50%{opacity:.4;transform:scale(.75)}}
.hbadge{padding:4px 12px;border-radius:20px;font-size:12px;font-weight:700;background:rgba(58,191,110,.14);color:var(--lf);border:1px solid rgba(58,191,110,.28)}
.btnr{padding:6px 13px;background:var(--sur2);border:1px solid var(--bdr);border-radius:7px;color:var(--txt);font-size:12px;font-family:inherit;cursor:pointer}
.btnr:hover{background:#253828}

/* LAYOUT */
.layout{display:flex;min-height:calc(100vh - 54px);position:relative;z-index:1}

/* SIDEBAR */
.sb{width:216px;flex-shrink:0;background:rgba(22,32,25,.8);border-right:1px solid var(--bdr);padding:18px 0;display:flex;flex-direction:column;gap:2px}
.sb-sec{padding:7px 16px 3px;font-size:10px;font-weight:700;letter-spacing:1.3px;color:var(--mut);text-transform:uppercase;margin-top:8px}
.sb-item{display:flex;align-items:center;gap:9px;padding:9px 16px;font-size:13px;color:var(--mut);cursor:pointer;border-left:3px solid transparent;transition:all .15s;border-radius:0 8px 8px 0;margin-right:8px}
.sb-item:hover{background:rgba(58,191,110,.06);color:var(--txt)}
.sb-item.active{background:rgba(58,191,110,.12);color:var(--acc);border-left-color:var(--acc);font-weight:700}
.sb-item .ic{font-size:15px;width:20px;text-align:center}
.sb-sens{padding:0 10px;margin-top:4px;display:flex;flex-direction:column;gap:2px}
.sr{display:flex;align-items:center;justify-content:space-between;padding:7px 8px;border-radius:8px;font-size:12px}
.sr:hover{background:rgba(58,191,110,.05)}
.sdot{width:8px;height:8px;border-radius:50%;flex-shrink:0}
.sname{flex:1;padding:0 7px;color:var(--txt)}
.sval{font-family:'DM Mono',monospace;font-size:12px;font-weight:500}
.sb-hs{margin:10px 10px 0;background:var(--sur2);border:1px solid var(--bdr);border-radius:11px;padding:11px 13px}
.sh-t{font-size:10px;font-weight:700;text-transform:uppercase;letter-spacing:1px;color:var(--mut);margin-bottom:7px}
.sh-r{display:flex;align-items:center;gap:9px}
.sh-s{font-size:34px;font-weight:800;color:var(--acc);font-family:'DM Mono',monospace}
.sh-l{font-size:12px;color:var(--mut)}
.sh-b{height:4px;background:rgba(255,255,255,.06);border-radius:2px;overflow:hidden;margin-top:7px}
.sh-bf{height:100%;background:linear-gradient(90deg,var(--acc2),var(--lf));border-radius:2px;transition:width 1s ease}
.sb-ft{margin-top:auto;padding:14px 16px;font-size:11px;color:var(--mut);border-top:1px solid var(--bdr);line-height:1.9}

/* MAIN */
.main{flex:1;padding:22px 26px;display:flex;flex-direction:column;gap:20px;overflow-y:auto}
.pt{font-size:19px;font-weight:800;display:flex;align-items:center;gap:9px}
.pt .sub{color:var(--mut);font-weight:400;font-size:14px}
.ur{display:flex;align-items:center;gap:6px;font-size:12px;color:var(--mut);margin-top:3px}

/* TAB */
.tp{display:none;flex-direction:column;gap:20px}
.tp.active{display:flex}

/* PANEL */
.panel{background:var(--sur);border:1px solid var(--bdr);border-radius:13px;padding:20px 22px}
.ph{display:flex;align-items:center;justify-content:space-between;margin-bottom:16px}
.ptl{font-size:12px;font-weight:700;letter-spacing:.8px;text-transform:uppercase;color:var(--mut)}
.lb{font-size:11px;color:var(--acc);font-weight:700;display:flex;align-items:center;gap:5px}
.lb::before{content:'';width:6px;height:6px;border-radius:50%;background:var(--acc);animation:pulse 1.5s infinite}

/* STAT ROW */
.str{display:grid;grid-template-columns:repeat(4,1fr);gap:13px}
.stc{background:var(--sur);border:1px solid var(--bdr);border-radius:13px;padding:16px 18px}
.stl{font-size:10px;font-weight:700;letter-spacing:1.1px;text-transform:uppercase;color:var(--mut);margin-bottom:7px}
.stv{font-size:30px;font-weight:800;font-family:'DM Mono',monospace;line-height:1}
.stm{font-size:12px;color:var(--mut);margin-top:5px}

/* BAR ROWS */
.sbr{display:flex;align-items:center;gap:13px;padding:9px 0;border-bottom:1px solid var(--bdr)}
.sbr:last-child{border-bottom:none}
.sbdot{width:9px;height:9px;border-radius:50%;flex-shrink:0}
.sbn{width:160px;font-size:13px;font-weight:600}
.sbt{flex:1;height:6px;background:rgba(255,255,255,.06);border-radius:3px;overflow:hidden}
.sbf{height:100%;border-radius:3px;transition:width .6s ease}
.sbv{width:85px;text-align:right;font-family:'DM Mono',monospace;font-size:13px;font-weight:500}

/* CARDS GRID */
.cg{display:grid;grid-template-columns:repeat(3,1fr);gap:14px}
.card{background:var(--sur);border:1px solid var(--bdr);border-radius:13px;padding:18px;position:relative;overflow:hidden;transition:transform .2s,border-color .2s}
.card:hover{transform:translateY(-2px);border-color:rgba(58,191,110,.22)}
.ch{display:flex;justify-content:space-between;align-items:flex-start;margin-bottom:11px}
.cn{font-size:13px;font-weight:700}
.cs{font-size:11px;color:var(--mut);margin-top:2px}
.cbg{font-size:10px;font-weight:700;padding:3px 7px;border-radius:6px;letter-spacing:.5px}
.cv{font-size:34px;font-weight:800;font-family:'DM Mono',monospace;line-height:1;margin-bottom:5px}
.cu{font-size:12px;color:var(--mut);margin-bottom:12px}
.cbw{display:flex;flex-direction:column;gap:4px}
.cbm{display:flex;justify-content:space-between;font-size:11px;color:var(--mut)}
.cbt{height:6px;background:rgba(255,255,255,.06);border-radius:3px;overflow:hidden}
.cbf{height:100%;border-radius:3px;transition:width .8s ease}

/* SECTION TITLE */
.sect{display:flex;align-items:center;gap:9px;font-size:15px;font-weight:700}
.sect::before{content:'';width:4px;height:18px;background:var(--acc);border-radius:2px}

/* SOIL */
.sg{display:grid;grid-template-columns:1fr 1fr;gap:18px}
.sc{background:var(--sur);border:1px solid var(--bdr);border-radius:13px;padding:22px}
.sct{font-size:12px;font-weight:700;text-transform:uppercase;letter-spacing:.8px;color:var(--mut);margin-bottom:14px}
.bm{display:flex;flex-direction:column;align-items:center;gap:11px;padding:14px 0}
.mc{width:112px;height:112px;border-radius:50%;display:flex;align-items:center;justify-content:center;flex-direction:column;border:3px solid}
.mv{font-size:26px;font-weight:800;font-family:'DM Mono',monospace}
.ml{font-size:10px;color:var(--mut)}
.sir{display:grid;grid-template-columns:1fr 1fr;gap:9px;margin-top:7px}
.sib{background:var(--sur2);border-radius:9px;padding:11px;text-align:center}
.sil{font-size:10px;color:var(--mut);text-transform:uppercase;letter-spacing:.7px}
.siv{font-size:17px;font-weight:800;font-family:'DM Mono',monospace;margin-top:3px}
.cb{margin-top:14px}
.cl{font-size:12px;color:var(--mut);margin-bottom:7px}
.ct{height:11px;background:rgba(255,255,255,.06);border-radius:5px;overflow:hidden}
.cf{height:100%;border-radius:5px;transition:width .8s ease}
.cm{display:flex;justify-content:space-between;font-size:10px;color:var(--mut);margin-top:3px}
.rb{background:rgba(58,191,110,.06);border:1px solid rgba(58,191,110,.18);border-radius:11px;padding:14px;margin-top:14px}
.rt{font-size:11px;font-weight:700;color:var(--acc);text-transform:uppercase;letter-spacing:1px;margin-bottom:7px}
.rd{font-size:13px;color:var(--txt);line-height:1.6}

/* TRENDS */
.cc{background:var(--sur);border:1px solid var(--bdr);border-radius:13px;padding:22px}
.ctit{font-size:12px;font-weight:700;text-transform:uppercase;letter-spacing:.8px;color:var(--mut);margin-bottom:18px}
svg.spk{width:100%;height:110px;overflow:visible}
.cl2{display:flex;gap:18px;flex-wrap:wrap;margin-top:11px}
.li{display:flex;align-items:center;gap:6px;font-size:12px;color:var(--mut)}
.ld{width:9px;height:9px;border-radius:50%}
.tg{display:grid;grid-template-columns:1fr 1fr;gap:18px}
.msr{display:grid;grid-template-columns:repeat(4,1fr);gap:11px}
.ms{background:var(--sur);border:1px solid var(--bdr);border-radius:11px;padding:13px 15px}
.msl{font-size:10px;color:var(--mut);text-transform:uppercase;letter-spacing:.8px}
.msv{font-size:20px;font-weight:800;font-family:'DM Mono',monospace;margin-top:5px}
.mss{font-size:11px;color:var(--mut);margin-top:2px}

/* MAP */
.ml2{display:grid;grid-template-columns:2fr 1fr;gap:18px}
.gm{background:var(--sur);border:1px solid var(--bdr);border-radius:13px;padding:22px}
.mt{font-size:12px;font-weight:700;text-transform:uppercase;letter-spacing:.8px;color:var(--mut);margin-bottom:18px}
svg.ghm{width:100%;border-radius:9px;overflow:visible}
.msb{display:flex;flex-direction:column;gap:13px}
.msc{background:var(--sur);border:1px solid var(--bdr);border-radius:11px;padding:14px;cursor:pointer;transition:all .2s}
.msc:hover{border-color:rgba(58,191,110,.28);transform:translateX(3px)}
.msc.hl{border-color:var(--acc);background:rgba(58,191,110,.06)}
.msh{display:flex;align-items:center;gap:9px;margin-bottom:9px}
.msd{width:9px;height:9px;border-radius:50%}
.msn{font-size:13px;font-weight:700}
.msv2{font-family:'DM Mono',monospace;font-size:19px;font-weight:700}
.mss2{font-size:11px;color:var(--mut);margin-top:2px}
.mleg{background:var(--sur);border:1px solid var(--bdr);border-radius:11px;padding:14px}
.mlt{font-size:10px;font-weight:700;text-transform:uppercase;letter-spacing:1px;color:var(--mut);margin-bottom:11px}
.mlr{display:flex;align-items:center;gap:7px;margin-bottom:7px;font-size:12px;color:var(--txt)}
.mldot{width:9px;height:9px;border-radius:50%}
.spulse{animation:sp 2s infinite}
@keyframes sp{0%,100%{opacity:1}50%{opacity:.35}}

/* ENV */
.eg{display:grid;grid-template-columns:1fr 1fr;gap:18px}
.ec{background:var(--sur);border:1px solid var(--bdr);border-radius:13px;padding:22px}
.etit{font-size:12px;font-weight:700;text-transform:uppercase;letter-spacing:.8px;color:var(--mut);margin-bottom:14px}
.ebig{font-size:50px;font-weight:800;font-family:'DM Mono',monospace;line-height:1}
.esub{font-size:13px;color:var(--mut);margin-top:5px;margin-bottom:14px}
.vpd{background:var(--sur2);border:1px solid var(--bdr);border-radius:11px;padding:14px;text-align:center}
.vpdl{font-size:11px;color:var(--mut);text-transform:uppercase;letter-spacing:1px;margin-bottom:5px}
.vpdv{font-size:30px;font-weight:800;font-family:'DM Mono',monospace;color:var(--acc)}
.vpds{font-size:11px;color:var(--mut);margin-top:3px}
.dr{display:grid;grid-template-columns:1fr 1fr;gap:11px;margin-top:14px}
.db{background:var(--sur2);border-radius:9px;padding:11px;text-align:center}
.dl{font-size:10px;color:var(--mut);text-transform:uppercase;letter-spacing:.8px}
.dv{font-size:19px;font-weight:800;font-family:'DM Mono',monospace;margin-top:3px;color:var(--sky)}

/* ALERTS */
.ag{display:grid;grid-template-columns:1fr 1fr;gap:14px}
.alc{background:var(--sur);border:1px solid var(--bdr);border-radius:13px;padding:18px;border-left:4px solid var(--acc)}
.alc.warn{border-left-color:var(--gld)}
.alc.danger{border-left-color:var(--org)}
.alh{display:flex;align-items:center;gap:9px;margin-bottom:9px}
.ali{font-size:18px}
.alt{font-size:13px;font-weight:700}
.alv{font-family:'DM Mono',monospace;font-size:26px;font-weight:700;margin-bottom:3px}
.ald{font-size:13px;color:var(--mut);line-height:1.5}
.thr{display:flex;gap:8px;margin-top:11px;flex-wrap:wrap}
.tc{background:var(--sur2);border-radius:20px;padding:3px 9px;font-size:11px;color:var(--mut)}

/* TOOLTIP */
.tt{position:fixed;background:var(--sur2);border:1px solid var(--bdr);border-radius:9px;padding:9px 13px;font-size:12px;pointer-events:none;opacity:0;transition:opacity .2s;z-index:999;min-width:130px}
.tt.show{opacity:1}
.ttn{font-weight:700;margin-bottom:3px}
.ttv{font-family:'DM Mono',monospace;font-size:15px;font-weight:500}
.tts{color:var(--mut);font-size:11px;margin-top:2px}

/* RANGE BAR HELPER */
.rb2{background:rgba(255,255,255,.04);border-radius:9px;padding:13px;margin-top:9px}
.rb2 .rl{font-size:11px;color:var(--mut);margin-bottom:7px}
</style>
</head>
<body>
<div id="splash">
  <div class="sp-logo">🌿 Plant<span>Watch</span></div>
  <div class="sp-sub">Real-time Greenhouse Health Monitor</div>
  <div class="sp-facts">
    <div class="sp-fact">🌡️ Temperature <span>monitoring</span></div>
    <div class="sp-fact">💧 Soil <span>moisture</span></div>
    <div class="sp-fact">☀️ Light <span>intensity</span></div>
    <div class="sp-fact">💨 Humidity <span>tracking</span></div>
  </div>
  <div class="sp-cards">
    <div class="sp-card">
      <svg viewBox="0 0 280 170" xmlns="http://www.w3.org/2000/svg">
        <rect width="280" height="170" fill="#0d1a0f"/>
        <ellipse cx="140" cy="132" rx="100" ry="22" fill="#1a2e1c"/>
        <rect x="40" y="125" width="200" height="45" fill="#1a2e1c" rx="3"/>
        <ellipse cx="85" cy="110" rx="7" ry="10" fill="#4aafde" opacity=".7"/>
        <ellipse cx="140" cy="104" rx="7" ry="10" fill="#4aafde" opacity=".7"/>
        <ellipse cx="195" cy="110" rx="7" ry="10" fill="#4aafde" opacity=".7"/>
        <line x1="140" y1="125" x2="140" y2="60" stroke="#3abf6e" stroke-width="4" stroke-linecap="round"/>
        <path d="M140 97 Q114 78 106 52 Q130 63 140 90" fill="#3abf6e" opacity=".9"/>
        <path d="M140 82 Q166 63 174 37 Q151 48 140 76" fill="#5dd98a" opacity=".85"/>
        <rect x="156" y="95" width="5" height="32" rx="2.5" fill="#d4a017"/>
        <circle cx="158" cy="94" r="5" fill="#d4a017"/>
      </svg>
      <div class="sp-tag">Soil Moisture</div>
      <div class="sp-ico">💧</div>
    </div>
    <div class="sp-card">
      <svg viewBox="0 0 280 170" xmlns="http://www.w3.org/2000/svg">
        <rect width="280" height="170" fill="#0d1a0f"/>
        <circle cx="225" cy="42" r="34" fill="rgba(212,160,23,0.12)"/>
        <circle cx="225" cy="42" r="20" fill="rgba(212,160,23,0.22)"/>
        <circle cx="225" cy="42" r="12" fill="#d4a017" opacity=".7"/>
        <line x1="225" y1="14" x2="225" y2="6" stroke="#d4a017" stroke-width="2.5" stroke-linecap="round" opacity=".6"/>
        <line x1="246" y1="21" x2="252" y2="15" stroke="#d4a017" stroke-width="2.5" stroke-linecap="round" opacity=".6"/>
        <line x1="254" y1="42" x2="262" y2="42" stroke="#d4a017" stroke-width="2.5" stroke-linecap="round" opacity=".6"/>
        <rect x="52" y="32" width="15" height="78" rx="7" fill="#1e2e21" stroke="#3abf6e" stroke-width="1.5"/>
        <rect x="56" y="86" width="7" height="18" rx="0" fill="#e05c2a"/>
        <circle cx="59" cy="110" r="8" fill="#e05c2a"/>
        <line x1="67" y1="42" x2="71" y2="42" stroke="#3abf6e" stroke-width="1.2" opacity=".5"/>
        <line x1="67" y1="58" x2="71" y2="58" stroke="#3abf6e" stroke-width="1.2" opacity=".5"/>
        <line x1="67" y1="74" x2="71" y2="74" stroke="#3abf6e" stroke-width="1.2" opacity=".5"/>
        <line x1="140" y1="150" x2="140" y2="72" stroke="#3abf6e" stroke-width="5" stroke-linecap="round"/>
        <path d="M140 115 Q105 92 92 57 Q125 72 140 108" fill="#3abf6e"/>
        <path d="M140 97 Q175 72 188 37 Q156 52 140 92" fill="#5dd98a"/>
        <ellipse cx="140" cy="155" rx="65" ry="14" fill="#1a2e1c"/>
        <text x="88" y="152" fill="#e05c2a" font-size="20" font-family="monospace" font-weight="700">24°C</text>
      </svg>
      <div class="sp-tag">Temperature</div>
      <div class="sp-ico">🌡️</div>
    </div>
    <div class="sp-card">
      <svg viewBox="0 0 280 170" xmlns="http://www.w3.org/2000/svg">
        <rect width="280" height="170" fill="#0d1a0f"/>
        <polygon points="112,0 168,0 208,170 72,170" fill="rgba(212,160,23,0.07)"/>
        <polygon points="127,0 153,0 183,170 97,170" fill="rgba(212,160,23,0.05)"/>
        <circle cx="135" cy="38" r="2" fill="#d4a017" opacity=".5"/>
        <circle cx="152" cy="66" r="1.5" fill="#d4a017" opacity=".4"/>
        <circle cx="124" cy="90" r="2" fill="#d4a017" opacity=".35"/>
        <rect x="122" y="124" width="38" height="26" rx="5" fill="#1e2e21" stroke="#d4a017" stroke-width="1.5"/>
        <circle cx="141" cy="137" r="7" fill="#d4a017" opacity=".6"/>
        <circle cx="141" cy="137" r="3.5" fill="#d4a017"/>
        <line x1="128" y1="150" x2="128" y2="162" stroke="#607d63" stroke-width="2"/>
        <line x1="141" y1="150" x2="141" y2="162" stroke="#607d63" stroke-width="2"/>
        <line x1="154" y1="150" x2="154" y2="162" stroke="#607d63" stroke-width="2"/>
        <line x1="82" y1="170" x2="82" y2="97" stroke="#3abf6e" stroke-width="4" stroke-linecap="round"/>
        <path d="M82 135 Q59 116 52 87 Q74 97 82 128" fill="#3abf6e"/>
        <path d="M82 114 Q105 94 112 65 Q91 76 82 109" fill="#5dd98a"/>
        <text x="183" y="97" fill="#d4a017" font-size="18" font-family="monospace" font-weight="700">2048</text>
        <text x="183" y="113" fill="#607d63" font-size="11" font-family="Nunito">LUX</text>
      </svg>
      <div class="sp-tag">Light (LDR)</div>
      <div class="sp-ico">☀️</div>
    </div>
  </div>
  <div class="sp-bar-lbl">Initializing sensors...</div>
  <div class="sp-bar"><div class="sp-bar-fill"></div></div>
</div>

<div class="tt" id="tt"><div class="ttn" id="tt-n"></div><div class="ttv" id="tt-v"></div><div class="tts" id="tt-s"></div></div>

<div class="topbar">
  <div class="logo"><div class="logo-ic">🌿</div>Plant<span>Watch</span></div>
  <div class="nav-r">
    <div class="hbadge" id="top-hb">● HEALTHY</div>
    <div style="display:flex;align-items:center;gap:6px"><div class="rdot"></div><span id="rcd">Auto-refresh in 5s</span></div>
    <button class="btnr" onclick="fetchData()">Refresh Now</button>
  </div>
</div>

<div class="layout">
<div class="sb">
  <div class="sb-sec">Dashboard</div>
  <div class="sb-item active" onclick="showTab('map',this)"><span class="ic">🗺</span> Sensor Map</div>
  <div class="sb-item" onclick="showTab('overview',this)"><span class="ic">⊞</span> Overview</div>
  <div class="sb-item" onclick="showTab('soil',this)"><span class="ic">💧</span> Soil Data</div>
  <div class="sb-item" onclick="showTab('trends',this)"><span class="ic">📈</span> Trends</div>
  <div class="sb-item" onclick="showTab('env',this)"><span class="ic">🌤</span> Environment</div>
  <div class="sb-item" onclick="showTab('alerts',this)"><span class="ic">🔔</span> Alerts</div>
  <div class="sb-sec" style="margin-top:10px">Sensors</div>
  <div class="sb-sens">
    <div class="sr"><div class="sdot" id="d-s1" style="background:#3abf6e"></div><div class="sname">Soil 1</div><div class="sval" id="v-s1" style="color:#3abf6e">--</div></div>
    <div class="sr"><div class="sdot" id="d-s2" style="background:#3abf6e"></div><div class="sname">Soil 2</div><div class="sval" id="v-s2" style="color:#3abf6e">--</div></div>
    <div class="sr"><div class="sdot" id="d-lt" style="background:#4aafde"></div><div class="sname">Light</div><div class="sval" id="v-lt" style="color:#4aafde">--</div></div>
    <div class="sr"><div class="sdot" id="d-tp" style="background:#3abf6e"></div><div class="sname">Temp</div><div class="sval" id="v-tp" style="color:#3abf6e">--</div></div>
    <div class="sr"><div class="sdot" id="d-hm" style="background:#3abf6e"></div><div class="sname">Humidity</div><div class="sval" id="v-hm" style="color:#3abf6e">--</div></div>
  </div>
  <div class="sb-hs">
    <div class="sh-t">Plant Health</div>
    <div class="sh-r"><div class="sh-s" id="sb-hs">--</div><div class="sh-l">/ 100</div></div>
    <div class="sh-b"><div class="sh-bf" id="sb-hb" style="width:0%"></div></div>
  </div>
  <div class="sb-ft">● 5 sensors online<br>Refreshes every 5s<br>ESP32 · Station A</div>
</div>
<div class="main">)rawhtml";

// Part 2: Tab pages HTML
const char HTML_P2[] PROGMEM = R"rawhtml(
<!-- MAP TAB (default) -->
<div class="tp active" id="tab-map">
  <div><div class="pt">Sensor Map <span class="sub">/ Greenhouse Layout</span></div><div class="ur">📍 Physical placement of all sensors</div></div>
  <div class="str">
    <div class="stc"><div class="stl">Soil Zone A</div><div class="stv" id="mp-s1" style="color:#3abf6e">--</div><div class="stm" id="mp-s1s">raw ADC</div></div>
    <div class="stc"><div class="stl">Soil Zone B</div><div class="stv" id="mp-s2" style="color:#d4a017">--</div><div class="stm" id="mp-s2s">raw ADC</div></div>
    <div class="stc"><div class="stl">Temperature</div><div class="stv" id="mp-tp" style="color:#3abf6e">--</div><div class="stm" id="mp-tps">°C</div></div>
    <div class="stc"><div class="stl">Humidity</div><div class="stv" id="mp-hm" style="color:#3abf6e">--</div><div class="stm" id="mp-hms">%</div></div>
  </div>
  <div class="ml2">
    <div class="gm">
      <div class="mt">Greenhouse Station A — Top View</div>
      <svg class="ghm" viewBox="0 0 520 340" xmlns="http://www.w3.org/2000/svg">
        <rect x="20" y="20" width="480" height="300" rx="16" fill="#162019" stroke="rgba(58,191,110,0.18)" stroke-width="2"/>
        <line x1="260" y1="20" x2="260" y2="320" stroke="rgba(58,191,110,0.06)" stroke-width="1" stroke-dasharray="6,5"/>
        <line x1="20" y1="170" x2="500" y2="170" stroke="rgba(58,191,110,0.06)" stroke-width="1" stroke-dasharray="6,5"/>
        <rect x="38" y="36" width="196" height="118" rx="10" fill="rgba(58,191,110,0.07)" stroke="rgba(58,191,110,0.22)" stroke-width="1.5"/>
        <text x="136" y="96" text-anchor="middle" fill="rgba(58,191,110,0.4)" font-size="11" font-family="Nunito" font-weight="700">Zone A</text>
        <rect x="282" y="36" width="196" height="118" rx="10" fill="rgba(212,160,23,0.07)" stroke="rgba(212,160,23,0.22)" stroke-width="1.5"/>
        <text x="380" y="96" text-anchor="middle" fill="rgba(212,160,23,0.4)" font-size="11" font-family="Nunito" font-weight="700">Zone B</text>
        <rect x="38" y="188" width="444" height="112" rx="10" fill="rgba(74,175,222,0.04)" stroke="rgba(74,175,222,0.14)" stroke-width="1.5"/>
        <text x="260" y="248" text-anchor="middle" fill="rgba(255,255,255,0.2)" font-size="11" font-family="Nunito">Walkway / Control Zone</text>
        <text x="66" y="72" font-size="18" text-anchor="middle">🌱</text><text x="104" y="72" font-size="18" text-anchor="middle">🌿</text>
        <text x="142" y="72" font-size="18" text-anchor="middle">🌱</text><text x="180" y="72" font-size="18" text-anchor="middle">🌿</text>
        <text x="66" y="108" font-size="18" text-anchor="middle">🌿</text><text x="104" y="108" font-size="18" text-anchor="middle">🌱</text>
        <text x="142" y="108" font-size="18" text-anchor="middle">🌿</text><text x="180" y="108" font-size="18" text-anchor="middle">🌱</text>
        <text x="310" y="72" font-size="18" text-anchor="middle">🌿</text><text x="348" y="72" font-size="18" text-anchor="middle">🌱</text>
        <text x="386" y="72" font-size="18" text-anchor="middle">🌿</text><text x="424" y="72" font-size="18" text-anchor="middle">🌱</text>
        <text x="310" y="108" font-size="18" text-anchor="middle">🌱</text><text x="348" y="108" font-size="18" text-anchor="middle">🌿</text>
        <text x="386" y="108" font-size="18" text-anchor="middle">🌱</text><text x="424" y="108" font-size="18" text-anchor="middle">🌿</text>
        <g style="cursor:pointer" onmouseenter="showTT(event,'Soil Sensor 1')" onmouseleave="hideTT()">
          <circle cx="136" cy="142" r="18" fill="rgba(58,191,110,0.15)" stroke="#3abf6e" stroke-width="2" class="spulse"/>
          <circle cx="136" cy="142" r="8" fill="#3abf6e"/>
          <text x="136" y="168" text-anchor="middle" fill="#3abf6e" font-size="10" font-family="monospace" font-weight="600" id="ml-s1">--</text>
        </g>
        <g style="cursor:pointer" onmouseenter="showTT(event,'Soil Sensor 2')" onmouseleave="hideTT()">
          <circle cx="380" cy="142" r="18" fill="rgba(212,160,23,0.15)" stroke="#d4a017" stroke-width="2" class="spulse" style="animation-delay:.6s"/>
          <circle cx="380" cy="142" r="8" fill="#d4a017"/>
          <text x="380" y="168" text-anchor="middle" fill="#d4a017" font-size="10" font-family="monospace" font-weight="600" id="ml-s2">--</text>
        </g>
        <g style="cursor:pointer" onmouseenter="showTT(event,'DHT22')" onmouseleave="hideTT()">
          <circle cx="260" cy="248" r="18" fill="rgba(224,92,42,0.15)" stroke="#e05c2a" stroke-width="2" class="spulse" style="animation-delay:1.2s"/>
          <circle cx="260" cy="248" r="8" fill="#e05c2a"/>
          <text x="260" y="274" text-anchor="middle" fill="#e05c2a" font-size="10" font-family="monospace" font-weight="600" id="ml-dht">--°C</text>
        </g>
        <g style="cursor:pointer" onmouseenter="showTT(event,'LDR Light')" onmouseleave="hideTT()">
          <circle cx="460" cy="248" r="18" fill="rgba(212,160,23,0.15)" stroke="#d4a017" stroke-width="2" class="spulse" style="animation-delay:1.8s"/>
          <circle cx="460" cy="248" r="8" fill="#d4a017"/>
          <text x="460" y="274" text-anchor="middle" fill="#d4a017" font-size="10" font-family="monospace" font-weight="600" id="ml-ldr">--</text>
        </g>
        <rect x="36" y="240" width="70" height="40" rx="6" fill="#1e2e21" stroke="rgba(58,191,110,0.28)" stroke-width="1.5"/>
        <text x="71" y="258" text-anchor="middle" fill="rgba(255,255,255,0.6)" font-size="9" font-family="Nunito" font-weight="600">ESP32</text>
        <text x="71" y="272" text-anchor="middle" fill="#3abf6e" font-size="8" font-family="monospace">● ONLINE</text>
        <line x1="106" y1="250" x2="136" y2="160" stroke="rgba(58,191,110,0.3)" stroke-width="1.5" stroke-dasharray="4,3"/>
        <line x1="106" y1="250" x2="380" y2="160" stroke="rgba(212,160,23,0.3)" stroke-width="1.5" stroke-dasharray="4,3"/>
        <line x1="106" y1="260" x2="242" y2="248" stroke="rgba(224,92,42,0.3)" stroke-width="1.5" stroke-dasharray="4,3"/>
        <line x1="106" y1="266" x2="442" y2="252" stroke="rgba(212,160,23,0.25)" stroke-width="1.5" stroke-dasharray="4,3"/>
        <text x="22" y="14" fill="rgba(255,255,255,0.3)" font-size="9" font-family="Nunito">NORTH</text>
        <text x="22" y="338" fill="rgba(255,255,255,0.3)" font-size="9" font-family="Nunito">SOUTH</text>
      </svg>
    </div>
    <div class="msb">
      <div class="msc" id="mc-s1" onclick="hlSensor('s1')"><div class="msh"><div class="msd" style="background:#3abf6e"></div><div class="msn">Soil Sensor 1</div></div><div class="msv2" id="mc-s1v" style="color:#3abf6e">--</div><div class="mss2">Zone A · PIN 34</div></div>
      <div class="msc" id="mc-s2" onclick="hlSensor('s2')"><div class="msh"><div class="msd" style="background:#d4a017"></div><div class="msn">Soil Sensor 2</div></div><div class="msv2" id="mc-s2v" style="color:#d4a017">--</div><div class="mss2">Zone B · PIN 32</div></div>
      <div class="msc" id="mc-dht" onclick="hlSensor('dht')"><div class="msh"><div class="msd" style="background:#e05c2a"></div><div class="msn">DHT22 Sensor</div></div><div class="msv2" id="mc-dhtv" style="color:#e05c2a">--</div><div class="mss2">Temp+Hum · PIN 4</div></div>
      <div class="msc" id="mc-ldr" onclick="hlSensor('ldr')"><div class="msh"><div class="msd" style="background:#d4a017"></div><div class="msn">LDR Light Sensor</div></div><div class="msv2" id="mc-ldrv" style="color:#d4a017">--</div><div class="mss2">Light · PIN 35</div></div>
      <div class="mleg"><div class="mlt">Legend</div>
        <div class="mlr"><div class="mldot" style="background:#3abf6e"></div>Soil Sensor</div>
        <div class="mlr"><div class="mldot" style="background:#e05c2a"></div>Temp/Humidity</div>
        <div class="mlr"><div class="mldot" style="background:#d4a017"></div>Light Sensor</div>
        <div class="mlr"><div class="mldot" style="background:rgba(255,255,255,0.3)"></div>ESP32 Controller</div>
      </div>
    </div>
  </div>
</div>

<!-- OVERVIEW TAB -->
<div class="tp" id="tab-overview">
  <div><div class="pt">Overview <span class="sub">/ Greenhouse Station A</span></div><div class="ur">🕐 Last updated <span id="ov-lu">just now</span></div></div>
  <div class="str">
    <div class="stc"><div class="stl">Health Score</div><div class="stv" id="ov-sc" style="color:#3abf6e">--</div><div class="stm" id="ov-scm">/ 100</div></div>
    <div class="stc"><div class="stl">Best Soil</div><div class="stv" id="ov-bs" style="color:#3abf6e">--</div><div class="stm" id="ov-bsm">Loading...</div></div>
    <div class="stc"><div class="stl">Temperature</div><div class="stv" id="ov-tp" style="color:#d4a017">--</div><div class="stm" id="ov-tpm">Loading...</div></div>
    <div class="stc"><div class="stl">Humidity</div><div class="stv" id="ov-hm" style="color:#3abf6e">--</div><div class="stm" id="ov-hmm">Loading...</div></div>
  </div>
  <div class="panel"><div class="ph"><div class="ptl">All Sensors</div><div class="lb">Live</div></div><div id="ov-bars"></div></div>
  <div class="sect">Sensor Breakdown</div>
  <div class="cg" id="ov-cards"></div>
</div>

<!-- SOIL TAB -->
<div class="tp" id="tab-soil">
  <div><div class="pt">Soil Data <span class="sub">/ Moisture Analysis</span></div><div class="ur">💧 Detailed soil moisture readings</div></div>
  <div class="sg">
    <div class="sc">
      <div class="sct">Soil Sensor 1 — Zone A</div>
      <div class="bm"><div class="mc" id="s1c" style="border-color:#3abf6e;color:#3abf6e"><div class="mv" id="s1r">--</div><div class="ml">RAW ADC</div></div><div style="font-size:19px;font-weight:800" id="s1sb">--</div></div>
      <div class="sir"><div class="sib"><div class="sil">Moisture %</div><div class="siv" id="s1p">--</div></div><div class="sib"><div class="sil">Status</div><div class="siv" id="s1st">--</div></div></div>
      <div class="cb"><div class="cl">Dryness Level</div><div class="ct"><div class="cf" id="s1b" style="width:0%;background:#3abf6e"></div></div><div class="cm"><span>WET</span><span>MOIST</span><span>DRY</span></div></div>
      <div class="rb"><div class="rt">💡 Recommendation</div><div class="rd" id="s1rec">Loading...</div></div>
    </div>
    <div class="sc">
      <div class="sct">Soil Sensor 2 — Zone B</div>
      <div class="bm"><div class="mc" id="s2c" style="border-color:#d4a017;color:#d4a017"><div class="mv" id="s2r">--</div><div class="ml">RAW ADC</div></div><div style="font-size:19px;font-weight:800" id="s2sb">--</div></div>
      <div class="sir"><div class="sib"><div class="sil">Moisture %</div><div class="siv" id="s2p">--</div></div><div class="sib"><div class="sil">Status</div><div class="siv" id="s2st">--</div></div></div>
      <div class="cb"><div class="cl">Dryness Level</div><div class="ct"><div class="cf" id="s2b" style="width:0%;background:#d4a017"></div></div><div class="cm"><span>WET</span><span>MOIST</span><span>DRY</span></div></div>
      <div class="rb"><div class="rt">💡 Recommendation</div><div class="rd" id="s2rec">Loading...</div></div>
    </div>
  </div>
  <div class="panel"><div class="ph"><div class="ptl">Sensor Comparison</div><div class="lb">Live</div></div>
    <div style="display:grid;grid-template-columns:1fr 1fr 1fr;gap:18px;text-align:center">
      <div><div style="font-size:11px;color:var(--mut);text-transform:uppercase;letter-spacing:1px;margin-bottom:7px">Sensor 1</div><div style="font-size:30px;font-weight:800;font-family:'DM Mono',monospace" id="cp1">--</div></div>
      <div><div style="font-size:11px;color:var(--mut);text-transform:uppercase;letter-spacing:1px;margin-bottom:7px">Difference</div><div style="font-size:30px;font-weight:800;font-family:'DM Mono',monospace" id="cpd">--</div><div style="font-size:11px;color:var(--mut);margin-top:3px">ADC units</div></div>
      <div><div style="font-size:11px;color:var(--mut);text-transform:uppercase;letter-spacing:1px;margin-bottom:7px">Sensor 2</div><div style="font-size:30px;font-weight:800;font-family:'DM Mono',monospace" id="cp2">--</div></div>
    </div>
    <div style="margin-top:18px">
      <div style="font-size:12px;color:var(--mut);margin-bottom:7px">Zone A</div><div class="ct" style="height:8px"><div class="cf" id="cb1" style="width:0%;background:#3abf6e"></div></div>
      <div style="font-size:12px;color:var(--mut);margin-bottom:7px;margin-top:11px">Zone B</div><div class="ct" style="height:8px"><div class="cf" id="cb2" style="width:0%;background:#d4a017"></div></div>
    </div>
  </div>
</div>

<!-- TRENDS TAB -->
<div class="tp" id="tab-trends">
  <div><div class="pt">Trends <span class="sub">/ Last 10 Readings</span></div><div class="ur">📊 Historical sensor data</div></div>
  <div class="msr">
    <div class="ms"><div class="msl">Avg Soil 1</div><div class="msv" id="tr-s1">--</div><div class="mss">raw ADC</div></div>
    <div class="ms"><div class="msl">Avg Soil 2</div><div class="msv" id="tr-s2">--</div><div class="mss">raw ADC</div></div>
    <div class="ms"><div class="msl">Avg Temp</div><div class="msv" id="tr-tp">--</div><div class="mss">°C</div></div>
    <div class="ms"><div class="msl">Avg Humidity</div><div class="msv" id="tr-hm">--</div><div class="mss">%</div></div>
  </div>
  <div class="tg">
    <div class="cc"><div class="ctit">Soil Moisture</div><svg class="spk" id="ch-soil" viewBox="0 0 400 110"></svg><div class="cl2"><div class="li"><div class="ld" style="background:#3abf6e"></div>Sensor 1</div><div class="li"><div class="ld" style="background:#d4a017"></div>Sensor 2</div></div></div>
    <div class="cc"><div class="ctit">Temperature</div><svg class="spk" id="ch-temp" viewBox="0 0 400 110"></svg><div class="cl2"><div class="li"><div class="ld" style="background:#e05c2a"></div>°C</div></div></div>
    <div class="cc"><div class="ctit">Humidity</div><svg class="spk" id="ch-hum" viewBox="0 0 400 110"></svg><div class="cl2"><div class="li"><div class="ld" style="background:#4aafde"></div>%</div></div></div>
    <div class="cc"><div class="ctit">Light Intensity</div><svg class="spk" id="ch-light" viewBox="0 0 400 110"></svg><div class="cl2"><div class="li"><div class="ld" style="background:#d4a017"></div>LDR ADC</div></div></div>
  </div>
</div>

<!-- ENVIRONMENT TAB -->
<div class="tp" id="tab-env">
  <div><div class="pt">Environment <span class="sub">/ Climate Analysis</span></div><div class="ur">🌤 Temperature, humidity, VPD, dew point</div></div>
  <div class="eg">
    <div class="ec">
      <div class="etit">Temperature</div>
      <div class="ebig" id="en-tp" style="color:#e05c2a">--°C</div>
      <div class="esub" id="en-tps">Loading...</div>
      <div class="rb2"><div class="rl">OPTIMAL RANGE: 15°C – 30°C</div><div class="ct" style="height:8px"><div class="cf" id="en-tpb" style="width:0%;background:#e05c2a"></div></div><div style="display:flex;justify-content:space-between;font-size:10px;color:var(--mut);margin-top:3px"><span>0°C</span><span>25°C</span><span>50°C</span></div></div>
    </div>
    <div class="ec">
      <div class="etit">Humidity</div>
      <div class="ebig" id="en-hm" style="color:#4aafde">--%</div>
      <div class="esub" id="en-hms">Loading...</div>
      <div class="rb2"><div class="rl">OPTIMAL RANGE: 40% – 70%</div><div class="ct" style="height:8px"><div class="cf" id="en-hmb" style="width:0%;background:#4aafde"></div></div><div style="display:flex;justify-content:space-between;font-size:10px;color:var(--mut);margin-top:3px"><span>0%</span><span>50%</span><span>100%</span></div></div>
    </div>
    <div class="ec">
      <div class="etit">VPD &amp; Derived Values</div>
      <div class="vpd"><div class="vpdl">Vapour Pressure Deficit</div><div class="vpdv" id="en-vpd">-- kPa</div><div class="vpds" id="en-vpds">Optimal: 0.8–1.2 kPa</div></div>
      <div class="dr">
        <div class="db"><div class="dl">Dew Point</div><div class="dv" id="en-dew">--°C</div></div>
        <div class="db"><div class="dl">Heat Index</div><div class="dv" id="en-hi" style="color:#e05c2a">--°C</div></div>
      </div>
    </div>
    <div class="ec">
      <div class="etit">Light Level</div>
      <div class="ebig" id="en-lt" style="color:#d4a017">--</div>
      <div class="esub" id="en-lts">Loading...</div>
      <div class="rb2"><div class="rl">ADC RANGE: 0 – 4095</div><div class="ct" style="height:8px"><div class="cf" id="en-ltb" style="width:0%;background:#d4a017"></div></div><div style="display:flex;justify-content:space-between;font-size:10px;color:var(--mut);margin-top:3px"><span>LOW</span><span>MODERATE</span><span>BRIGHT</span></div></div>
      <div style="margin-top:12px;padding:11px;background:rgba(212,160,23,0.07);border:1px solid rgba(212,160,23,0.18);border-radius:9px">
        <div style="font-size:11px;font-weight:700;color:#d4a017;text-transform:uppercase;letter-spacing:1px;margin-bottom:5px">💡 Advice</div>
        <div style="font-size:13px;color:var(--txt)" id="en-lta">Loading...</div>
      </div>
    </div>
  </div>
</div>

<!-- ALERTS TAB -->
<div class="tp" id="tab-alerts">
  <div><div class="pt">Alerts <span class="sub">/ Threshold Monitor</span></div><div class="ur">🔔 Real-time sensor threshold alerts</div></div>
  <div class="panel"><div class="ph"><div class="ptl">System Status</div><div class="lb">Live</div></div>
    <div style="display:flex;align-items:center;gap:14px">
      <div style="font-size:46px;font-weight:800;color:var(--acc);font-family:'DM Mono',monospace" id="al-sc">--</div>
      <div><div style="font-size:17px;font-weight:700" id="al-st">Loading...</div><div style="font-size:13px;color:var(--mut);margin-top:3px" id="al-sm">Checking...</div></div>
    </div>
  </div>
  <div class="ag" id="al-cards"></div>
</div>

</div></div>)rawhtml";

// Part 3: JavaScript + closing tags
const char HTML_P3[] PROGMEM = R"rawhtml(
<script>
var D=null,cd=5;
setTimeout(function(){
  var s=document.getElementById('splash');
  s.classList.add('hidden');
  setTimeout(function(){s.style.display='none';},700);
},2800);
function showTab(n,el){
  document.querySelectorAll('.tp').forEach(function(p){p.classList.remove('active');});
  document.querySelectorAll('.sb-item').forEach(function(s){s.classList.remove('active');});
  document.getElementById('tab-'+n).classList.add('active');
  el.classList.add('active');
  if(D) upAll(D);
}
function sc(v){return v>3000?'#e05c2a':v>1500?'#d4a017':'#3abf6e';}
function ss(v){return v>3000?'DRY':v>1500?'MOIST':'WET';}
function lc(v){return v>3000?'#d4a017':v>1500?'#3abf6e':'#4aafde';}
function ls(v){return v>3000?'BRIGHT':v>1500?'MODERATE':'LOW';}
function tc(t){return t>35?'#e05c2a':t>25?'#d4a017':t>15?'#3abf6e':'#4aafde';}
function ts(t){return t>35?'HOT':t>25?'WARM':t>15?'NORMAL':'COLD';}
function hc(h){return h>80?'#d4a017':h>40?'#3abf6e':'#e05c2a';}
function hs(h){return h>80?'HIGH':h>40?'OPTIMAL':'LOW';}
function pct(v){return Math.round(v/4095*100);}
function clp(v,a,b){return Math.min(Math.max(v,a),b);}
function avg(a){return a.length?(a.reduce(function(x,y){return x+y;},0)/a.length):0;}
function srec(v){
  if(v>3000) return 'Soil is very DRY. Water immediately to prevent wilting.';
  if(v>1500) return 'Soil moisture is moderate. Monitor and water soon.';
  return 'Soil moisture is optimal. No immediate watering needed.';
}
function lrec(v){
  if(v>3000) return 'HIGH light. Consider shade cloth to prevent scorching.';
  if(v>1500) return 'Moderate light — good for most plants.';
  return 'LOW light. Consider supplemental lighting for light-loving plants.';
}
function hlth(d){
  var s=100;
  if(d.soil1>3000)s-=25; else if(d.soil1>1500)s-=8;
  if(d.soil2>3000)s-=25; else if(d.soil2>1500)s-=8;
  if(d.temp>40||d.temp<5)s-=20; else if(d.temp>35||d.temp<10)s-=10;
  if(d.hum>90||d.hum<20)s-=15; else if(d.hum>80||d.hum<30)s-=8;
  return Math.max(0,Math.min(100,s));
}
function vpd(t,h){
  var es=0.6108*Math.exp(17.27*t/(t+237.3));
  return (es-es*h/100).toFixed(2);
}
function dewp(t,h){
  var g=(17.27*t/(237.3+t))+Math.log(h/100);
  return (237.3*g/(17.27-g)).toFixed(1);
}
function hi(t,h){
  if(t<27) return t.toFixed(1);
  return (-8.78+1.61*t+2.34*h-0.146*t*h-0.0123*t*t-0.0164*h*h+0.00221*t*t*h+0.00072*t*h*h-0.00000358*t*t*h*h).toFixed(1);
}
function upSB(d){
  function set(id,did,val,col){
    document.getElementById(id).textContent=val;
    document.getElementById(id).style.color=col;
    document.getElementById(did).style.background=col;
  }
  set('v-s1','d-s1',d.soil1,sc(d.soil1));
  set('v-s2','d-s2',d.soil2,sc(d.soil2));
  set('v-lt','d-lt',d.light,lc(d.light));
  set('v-tp','d-tp',d.temp+'°',tc(d.temp));
  set('v-hm','d-hm',d.hum+'%',hc(d.hum));
  var s=hlth(d);
  var col=s>=80?'#3abf6e':s>=50?'#d4a017':'#e05c2a';
  document.getElementById('sb-hs').textContent=s;
  document.getElementById('sb-hs').style.color=col;
  document.getElementById('sb-hb').style.width=s+'%';
  document.getElementById('top-hb').textContent='● '+(s>=80?'EXCELLENT':s>=50?'FAIR':'POOR');
  document.getElementById('top-hb').style.color=col;
}
function upMap(d){
  document.getElementById('ml-s1').textContent=d.soil1;
  document.getElementById('ml-s2').textContent=d.soil2;
  document.getElementById('ml-dht').textContent=d.temp+'°C';
  document.getElementById('ml-ldr').textContent=d.light;
  document.getElementById('mc-s1v').textContent=d.soil1+' · '+ss(d.soil1);
  document.getElementById('mc-s2v').textContent=d.soil2+' · '+ss(d.soil2);
  document.getElementById('mc-dhtv').textContent=d.temp+'°C / '+d.hum+'%';
  document.getElementById('mc-dhtv').style.color=tc(d.temp);
  document.getElementById('mc-ldrv').textContent=d.light+' · '+ls(d.light);
  document.getElementById('mp-s1').textContent=d.soil1;
  document.getElementById('mp-s1').style.color=sc(d.soil1);
  document.getElementById('mp-s1s').textContent=ss(d.soil1);
  document.getElementById('mp-s2').textContent=d.soil2;
  document.getElementById('mp-s2').style.color=sc(d.soil2);
  document.getElementById('mp-s2s').textContent=ss(d.soil2);
  document.getElementById('mp-tp').textContent=d.temp+'°C';
  document.getElementById('mp-tp').style.color=tc(d.temp);
  document.getElementById('mp-tps').textContent=ts(d.temp);
  document.getElementById('mp-hm').textContent=d.hum+'%';
  document.getElementById('mp-hm').style.color=hc(d.hum);
  document.getElementById('mp-hms').textContent=hs(d.hum);
}
function upOV(d){
  var s=hlth(d); var sc2=s>=80?'#3abf6e':s>=50?'#d4a017':'#e05c2a';
  document.getElementById('ov-sc').textContent=s; document.getElementById('ov-sc').style.color=sc2;
  document.getElementById('ov-scm').textContent=s>=80?'Excellent':s>=50?'Fair':'Poor';
  var bv=d.soil1<=d.soil2?d.soil1:d.soil2; var bn=d.soil1<=d.soil2?'Soil 1':'Soil 2';
  document.getElementById('ov-bs').textContent=bn; document.getElementById('ov-bs').style.color=sc(bv);
  document.getElementById('ov-bsm').innerHTML='▲ '+ss(bv)+' · '+bv;
  document.getElementById('ov-tp').textContent=d.temp+'°C'; document.getElementById('ov-tp').style.color=tc(d.temp);
  document.getElementById('ov-tpm').textContent='● '+ts(d.temp);
  document.getElementById('ov-hm').textContent=d.hum+'%'; document.getElementById('ov-hm').style.color=hc(d.hum);
  document.getElementById('ov-hmm').textContent='● '+hs(d.hum);
  document.getElementById('ov-lu').textContent='just now';
  var bars=[['Soil 1',sc(d.soil1),pct(d.soil1),d.soil1],['Soil 2',sc(d.soil2),pct(d.soil2),d.soil2],
    ['Light',lc(d.light),pct(d.light),d.light],['Temp',tc(d.temp),clp(Math.round(d.temp/50*100),0,100),d.temp+'°C'],
    ['Humidity',hc(d.hum),clp(Math.round(d.hum),0,100),d.hum+'%']];
  var bh='';
  bars.forEach(function(b){bh+='<div class="sbr"><div class="sbdot" style="background:'+b[1]+'"></div><div class="sbn">'+b[0]+'</div><div class="sbt"><div class="sbf" style="width:'+b[2]+'%;background:'+b[1]+'"></div></div><div class="sbv" style="color:'+b[1]+'">'+b[3]+'</div></div>';});
  document.getElementById('ov-bars').innerHTML=bh;
  var cards=[{n:'Soil Moisture 1',s:'Capacitive',b:ss(d.soil1),v:d.soil1,u:'raw ADC',c:sc(d.soil1),p:pct(d.soil1)},
    {n:'Soil Moisture 2',s:'Capacitive',b:ss(d.soil2),v:d.soil2,u:'raw ADC',c:sc(d.soil2),p:pct(d.soil2)},
    {n:'Light Intensity',s:'LDR Sensor',b:ls(d.light),v:d.light,u:'raw ADC',c:lc(d.light),p:pct(d.light)},
    {n:'Temperature',s:'DHT22',b:ts(d.temp),v:d.temp+'°',u:'Celsius',c:tc(d.temp),p:clp(Math.round(d.temp/50*100),0,100)},
    {n:'Humidity',s:'DHT22',b:hs(d.hum),v:d.hum+'%',u:'Relative %',c:hc(d.hum),p:clp(Math.round(d.hum),0,100)}];
  var ch='';
  cards.forEach(function(c){
    ch+='<div class="card" style="border-top:3px solid '+c.c+'"><div class="ch"><div><div class="cn">'+c.n+'</div><div class="cs">'+c.s+'</div></div><div class="cbg" style="background:'+c.c+'22;color:'+c.c+'">'+c.b+'</div></div><div class="cv" style="color:'+c.c+'">'+c.v+'</div><div class="cu">'+c.u+'</div><div class="cbw"><div class="cbm"><span>0</span><span>'+c.p+'%</span><span>100</span></div><div class="cbt"><div class="cbf" style="width:'+c.p+'%;background:'+c.c+'"></div></div></div></div>';
  });
  document.getElementById('ov-cards').innerHTML=ch;
}
function upSoil(d){
  function setS(pre,val){
    var col=sc(val);
    document.getElementById(pre+'r').textContent=val;
    var circle=document.getElementById(pre+'c');
    circle.style.borderColor=col; circle.style.color=col;
    document.getElementById(pre+'sb').textContent=ss(val); document.getElementById(pre+'sb').style.color=col;
    document.getElementById(pre+'p').textContent=(100-pct(val))+'%'; document.getElementById(pre+'p').style.color=col;
    document.getElementById(pre+'st').textContent=ss(val); document.getElementById(pre+'st').style.color=col;
    document.getElementById(pre+'b').style.width=pct(val)+'%'; document.getElementById(pre+'b').style.background=col;
    document.getElementById(pre+'rec').textContent=srec(val);
  }
  setS('s1',d.soil1); setS('s2',d.soil2);
  document.getElementById('cp1').textContent=d.soil1;
  document.getElementById('cp2').textContent=d.soil2;
  document.getElementById('cpd').textContent=Math.abs(d.soil1-d.soil2);
  document.getElementById('cb1').style.width=pct(d.soil1)+'%';
  document.getElementById('cb2').style.width=pct(d.soil2)+'%';
}
function drawSpk(id,datasets,yMin,yMax,colors){
  var svg=document.getElementById(id); if(!svg) return;
  var W=400,H=100,pad=10,html='',range=yMax-yMin||1;
  datasets.forEach(function(data,di){
    if(!data||data.length<2) return;
    var n=data.length,col=colors[di]||'#3abf6e';
    var pts=data.map(function(v,i){return (pad+(i/(n-1))*(W-2*pad))+','+(H-pad-((v-yMin)/range)*(H-2*pad));}).join(' ');
    var lp=pts.split(' ').pop().split(',');
    html+='<polygon points="'+pad+','+(H-pad)+' '+pts+' '+(pad+(W-2*pad))+','+(H-pad)+'" fill="'+col+'" fill-opacity="0.10"/>';
    html+='<polyline points="'+pts+'" fill="none" stroke="'+col+'" stroke-width="2" stroke-linejoin="round" stroke-linecap="round"/>';
    html+='<circle cx="'+lp[0]+'" cy="'+lp[1]+'" r="4" fill="'+col+'"/>';
  });
  for(var i=0;i<=4;i++){var y=pad+(i/4)*(H-2*pad);html='<line x1="'+pad+'" y1="'+y+'" x2="'+(W-pad)+'" y2="'+y+'" stroke="rgba(255,255,255,0.05)" stroke-width="1"/>'+html;}
  svg.innerHTML=html;
}
function upTrends(d){
  var h=d.history;
  if(!h||!h.soil1||h.soil1.length<2){
    ['ch-soil','ch-temp','ch-hum','ch-light'].forEach(function(id){
      var el=document.getElementById(id);
      if(el) el.innerHTML='<text x="200" y="55" text-anchor="middle" fill="rgba(255,255,255,0.3)" font-size="12" font-family="Nunito">Collecting data...</text>';
    });
    return;
  }
  document.getElementById('tr-s1').textContent=Math.round(avg(h.soil1));
  document.getElementById('tr-s2').textContent=Math.round(avg(h.soil2));
  document.getElementById('tr-tp').textContent=avg(h.temp).toFixed(1)+'°';
  document.getElementById('tr-hm').textContent=avg(h.hum).toFixed(1)+'%';
  var allS=h.soil1.concat(h.soil2);
  drawSpk('ch-soil',[h.soil1,h.soil2],Math.min.apply(null,allS)-100,Math.max.apply(null,allS)+100,['#3abf6e','#d4a017']);
  drawSpk('ch-temp',[h.temp],Math.min.apply(null,h.temp)-2,Math.max.apply(null,h.temp)+2,['#e05c2a']);
  drawSpk('ch-hum',[h.hum],Math.min.apply(null,h.hum)-5,Math.max.apply(null,h.hum)+5,['#4aafde']);
  drawSpk('ch-light',[h.light],Math.min.apply(null,h.light)-100,Math.max.apply(null,h.light)+100,['#d4a017']);
}
function upEnv(d){
  var tCol=tc(d.temp),hCol=hc(d.hum),lCol=lc(d.light);
  document.getElementById('en-tp').textContent=d.temp+'°C'; document.getElementById('en-tp').style.color=tCol;
  document.getElementById('en-tps').textContent=ts(d.temp)+(d.temp>=15&&d.temp<=30?' — Ideal':'— Outside ideal range');
  document.getElementById('en-tpb').style.width=clp(Math.round(d.temp/50*100),0,100)+'%'; document.getElementById('en-tpb').style.background=tCol;
  document.getElementById('en-hm').textContent=d.hum+'%'; document.getElementById('en-hm').style.color=hCol;
  document.getElementById('en-hms').textContent=hs(d.hum)+(d.hum>=40&&d.hum<=70?' — Ideal':'— Outside ideal range');
  document.getElementById('en-hmb').style.width=clp(d.hum,0,100)+'%'; document.getElementById('en-hmb').style.background=hCol;
  var vpdVal=vpd(d.temp,d.hum);
  document.getElementById('en-vpd').textContent=vpdVal+' kPa';
  document.getElementById('en-vpd').style.color=vpdVal>=0.8&&vpdVal<=1.2?'#3abf6e':vpdVal<0.4?'#4aafde':'#e05c2a';
  document.getElementById('en-vpds').textContent=vpdVal<0.4?'Too low — mould risk':vpdVal<=1.2?'Optimal range':'High — wilting risk';
  document.getElementById('en-dew').textContent=dewp(d.temp,d.hum)+'°C';
  document.getElementById('en-hi').textContent=hi(d.temp,d.hum)+'°C';
  document.getElementById('en-lt').textContent=d.light; document.getElementById('en-lt').style.color=lCol;
  document.getElementById('en-lts').textContent=ls(d.light)+' light';
  document.getElementById('en-ltb').style.width=pct(d.light)+'%'; document.getElementById('en-ltb').style.background=lCol;
  document.getElementById('en-lta').textContent=lrec(d.light);
}
function upAlerts(d){
  var s=hlth(d); var col=s>=80?'#3abf6e':s>=50?'#d4a017':'#e05c2a';
  document.getElementById('al-sc').textContent=s; document.getElementById('al-sc').style.color=col;
  document.getElementById('al-st').textContent=s>=80?'All Systems Healthy':s>=50?'Attention Required':'Critical Issues Detected';
  var issues=[];
  if(d.soil1>3000)issues.push('Soil A DRY');
  if(d.soil2>3000)issues.push('Soil B DRY');
  if(d.temp>35)issues.push('Temp HIGH');
  if(d.hum<30)issues.push('Humidity LOW');
  document.getElementById('al-sm').textContent=issues.length?issues.join(' · '):'No critical alerts';
  var vpdV=parseFloat(vpd(d.temp,d.hum));
  var defs=[
    {n:'Soil Zone A',i:'💧',v:d.soil1+' ADC',desc:srec(d.soil1),sev:d.soil1>3000?'danger':d.soil1>1500?'warn':'ok',t:['WET <1500','MOIST 1500–3000','DRY >3000']},
    {n:'Soil Zone B',i:'💧',v:d.soil2+' ADC',desc:srec(d.soil2),sev:d.soil2>3000?'danger':d.soil2>1500?'warn':'ok',t:['WET <1500','MOIST 1500–3000','DRY >3000']},
    {n:'Temperature',i:'🌡️',v:d.temp+'°C',desc:d.temp>35?'Too hot — heat stress risk.':'Temperature in safe range.',sev:d.temp>35||d.temp<10?'danger':d.temp>30||d.temp<15?'warn':'ok',t:['COLD <15°C','NORMAL 15–35°C','HOT >35°C']},
    {n:'Humidity',i:'💨',v:d.hum+'%',desc:d.hum<30?'Critically low — mist immediately.':d.hum>85?'Too high — fungal risk.':'Humidity acceptable.',sev:d.hum>85||d.hum<30?'danger':d.hum>70||d.hum<40?'warn':'ok',t:['LOW <40%','OPTIMAL 40–80%','HIGH >80%']},
    {n:'Light',i:'☀️',v:d.light+' ADC',desc:lrec(d.light),sev:d.light>3500?'warn':d.light<200?'warn':'ok',t:['LOW <1500','MODERATE 1500–3000','BRIGHT >3000']},
    {n:'VPD',i:'🌬️',v:vpdV+' kPa',desc:'Affects plant transpiration rate.',sev:vpdV>2?'danger':vpdV<0.4?'warn':'ok',t:['Ideal 0.8–1.2 kPa','Low <0.4 kPa','High >2 kPa']}
  ];
  var html='';
  defs.forEach(function(a){
    html+='<div class="alc '+a.sev+'"><div class="alh"><div class="ali">'+a.i+'</div><div class="alt">'+a.n+'</div></div><div class="alv">'+a.v+'</div><div class="ald">'+a.desc+'</div><div class="thr">'+a.t.map(function(t){return '<div class="tc">'+t+'</div>';}).join('')+'</div></div>';
  });
  document.getElementById('al-cards').innerHTML=html;
}
function upAll(d){
  upSB(d); upMap(d); upOV(d); upSoil(d); upTrends(d); upEnv(d); upAlerts(d);
}
function fetchData(){
  fetch('/data')
    .then(function(r){return r.json();})
    .then(function(d){D=d; upAll(d); cd=5;})
    .catch(function(e){console.error(e);});
}
function showTT(evt,name){
  if(!D) return;
  var t=document.getElementById('tt');
  var vals={'Soil Sensor 1':[D.soil1,ss(D.soil1)],'Soil Sensor 2':[D.soil2,ss(D.soil2)],'DHT22':[D.temp+'°C / '+D.hum+'%',ts(D.temp)],'LDR Light':[D.light,ls(D.light)]};
  var v=vals[name]||['--','--'];
  document.getElementById('tt-n').textContent=name;
  document.getElementById('tt-v').textContent=v[0];
  document.getElementById('tt-s').textContent=v[1];
  t.style.left=(evt.clientX+12)+'px'; t.style.top=(evt.clientY-8)+'px';
  t.classList.add('show');
}
function hideTT(){document.getElementById('tt').classList.remove('show');}
function hlSensor(id){
  document.querySelectorAll('.msc').forEach(function(c){c.classList.remove('hl');});
  document.getElementById('mc-'+id).classList.add('hl');
}
setInterval(function(){
  cd--;
  document.getElementById('rcd').textContent='Auto-refresh in '+Math.max(0,cd)+'s';
  if(cd<=0) fetchData();
},1000);
fetchData();
</script>
</body></html>)rawhtml";

/* ================================================================
   WiFi
   ================================================================ */
void connectWiFi() {
  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500); Serial.print("."); attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected! IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nWiFi FAILED. Restarting...");
    delay(1000); ESP.restart();
  }
}

/* ================================================================
   JSON endpoint
   ================================================================ */
void handleData() {
  String json = "{";
  json += "\"soil1\":"  + String(soilValue1)      + ",";
  json += "\"soil2\":"  + String(soilValue2)      + ",";
  json += "\"light\":"  + String(lightValue)      + ",";
  json += "\"temp\":"   + String(temperature, 1)  + ",";
  json += "\"hum\":"    + String(humidity, 1)     + ",";

  int count = historyFull ? HISTORY_LEN : historyIndex;
  json += "\"history\":{";
  auto arr = [&](const char* key, auto* hist) {
    json += "\""; json += key; json += "\":[";
    for (int i = 0; i < count; i++) {
      int idx = historyFull ? (historyIndex + i) % HISTORY_LEN : i;
      json += String(hist[idx]);
      if (i < count - 1) json += ",";
    }
    json += "]";
  };
  arr("soil1", soil1History); json += ",";
  arr("soil2", soil2History); json += ",";
  arr("light", lightHistory); json += ",";

  json += "\"temp\":[";
  for (int i = 0; i < count; i++) {
    int idx = historyFull ? (historyIndex + i) % HISTORY_LEN : i;
    json += String(tempHistory[idx], 1);
    if (i < count - 1) json += ",";
  }
  json += "],\"hum\":[";
  for (int i = 0; i < count; i++) {
    int idx = historyFull ? (historyIndex + i) % HISTORY_LEN : i;
    json += String(humHistory[idx], 1);
    if (i < count - 1) json += ",";
  }
  json += "]}}";

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

/* ================================================================
   Root handler — streams PROGMEM parts directly, zero heap copy
   ================================================================ */
void handleRoot() {
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html; charset=utf-8", "");

  // Stream each PROGMEM part in small buffer chunks
  const char* parts[3] = { HTML_P1, HTML_P2, HTML_P3 };
  const int BUFSIZE = 512;
  char buf[BUFSIZE + 1];

  for (int p = 0; p < 3; p++) {
    const char* ptr = parts[p];
    int total = strlen_P(ptr);
    int sent = 0;
    while (sent < total) {
      int chunk = min(BUFSIZE, total - sent);
      memcpy_P(buf, ptr + sent, chunk);
      buf[chunk] = '\0';
      server.sendContent(buf);
      sent += chunk;
      delay(1); // feed watchdog
    }
  }
  server.sendContent(""); // end chunked
}

/* ================================================================
   Setup
   ================================================================ */
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== PlantWatch v3 ===");
  Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
  analogReadResolution(12);
  dht.begin();
  connectWiFi();
  server.on("/",     handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.printf("Server started. Free heap: %d bytes\n", ESP.getFreeHeap());
  Serial.println("Open: http://" + WiFi.localIP().toString());
}

/* ================================================================
   Loop
   ================================================================ */
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost. Reconnecting...");
    connectWiFi();
  }

  server.handleClient();

  soilValue1 = analogRead(SOIL_PIN1);
  soilValue2 = analogRead(SOIL_PIN2);
  lightValue  = analogRead(LDR_PIN);

  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t)) temperature = t;
  if (!isnan(h)) humidity    = h;

  soil1History[historyIndex] = soilValue1;
  soil2History[historyIndex] = soilValue2;
  lightHistory[historyIndex] = lightValue;
  tempHistory[historyIndex]  = temperature;
  humHistory[historyIndex]   = humidity;
  historyIndex = (historyIndex + 1) % HISTORY_LEN;
  if (historyIndex == 0) historyFull = true;

  Serial.printf("S1:%d S2:%d Light:%d Temp:%.1f Hum:%.1f  Heap:%d\n",
    soilValue1, soilValue2, lightValue, temperature, humidity, ESP.getFreeHeap());

  delay(2000);
}
