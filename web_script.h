#ifndef WEB_SCRIPT_H
#define WEB_SCRIPT_H

#include <Arduino.h>

const char WEB_JS[] PROGMEM = R"js(
(()=>{const u=()=>{fetch('/status').then(r=>r.json()).then(s=>{var el,pf;
if((el=document.getElementById('cur')))el.textContent=s.currentStep;
if((el=document.getElementById('max')))el.textContent=s.maxSteps;
if((el=document.getElementById('pos')))el.textContent=s.position;
if((pf=document.getElementById('progress-fill')))pf.style.width=s.position+'%';
var cal=s.mode==='CALIBRATE';
if((el=document.getElementById('mode'))){el.textContent=s.mode;el.className='badge '+(cal?'badge-calibrate':'badge-normal');}
var mw=document.getElementById('mode-wrap');if(mw)mw.style.display=cal?'block':'none';
var pw=document.getElementById('pos-wrap');if(pw)pw.style.display=cal?'none':'block';
var sb=document.getElementById('btnStop');if(sb)sb.classList.toggle('blink',!!s.moving);
var sl=document.getElementById('status-line');if(sl){
if(s.msg&&s.msg!=='Stopped'){var isErr=s.msg.indexOf('too small')>-1||s.msg.indexOf('failed')>-1;sl.textContent=s.msg;sl.className='status-line '+(isErr?'sl-err':'sl-ok');}
else{var b=s.rssi>-50?'\u25CF\u25CF\u25CF\u25CF':s.rssi>-65?'\u25CF\u25CF\u25CF\u25CB':s.rssi>-75?'\u25CF\u25CF\u25CB\u25CB':'\u25CF\u25CB\u25CB\u25CB';sl.textContent='WiFi '+b+' '+s.rssi+' dBm';sl.className='status-line';}}
var cs=document.getElementById('calSave');if(cs)cs.style.display=cal?'block':'none';
var st=document.getElementById('calStart'),sp=document.getElementById('calStop');
if(st&&sp){st.style.display=cal?'none':'inline-block';sp.style.display=cal?'inline-block':'none';sp.disabled=cal&&s.maxSteps===0;}
});};setInterval(u,2000);window.addEventListener('load',u);
document.addEventListener('click',(e)=>{var b=e.target.closest('[data-act]');if(b){if(b.disabled)return;var act=b.getAttribute('data-act');
if(act==='/factory'){if(!confirm('Factory reset will erase Wi-Fi and config. Continue?'))return;}
fetch(act,{method:'POST'}).then(u).catch(()=>{alert('Error!');});e.preventDefault();}});})();
)js";

#endif
