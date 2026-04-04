#ifndef WEB_HTML_H
#define WEB_HTML_H

#include <Arduino.h>

const char WEB_HTML_HEAD[] PROGMEM =
    "<!doctype html><html><head>"
    "<meta charset=\"UTF-8\">"
    "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
    "<title>Shades</title><style>";

// CSS goes between HEAD and BODY_START

const char WEB_HTML_BODY[] PROGMEM = R"html(
</style></head><body>
<div class="container">
<div class="card">
<h1>Roller Shades</h1>
<div style="text-align:center;margin:6px 0" id="mode-wrap"><span class="badge MODE_CLASS" id='mode'>MODE_PLACEHOLDER</span></div>
<div id="pos-wrap">
<div class="pos-text"><span id='pos'>POS_PLACEHOLDER</span><span class="pos-unit">%</span></div>
<div class="pos-label">Position</div>
<div class="progress-bar"><div class="progress-fill" id="progress-fill" style="width:POS_PLACEHOLDER%"></div></div>
</div>
<div class="status-line" id="status-line"></div>
</div>

<div class="card">
<h3>🎛️ Control</h3>
<div class="btn-row">
<button class="btn btn-green" data-act='/cal/up/start'>⬆️ Open</button>
<button class="btn" data-act='/cal/down/start'>⬇️ Close</button>
<button class="btn btn-red" id='btnStop' data-act='/cal/hold/stop'>⏹️ Stop</button>
</div>
<div class="btn-row">
<button class="btn btn-blue" data-act='/preset/30'>30%</button>
<button class="btn btn-blue" data-act='/preset/50'>50%</button>
<button class="btn btn-blue" data-act='/preset/70'>70%</button>
</div>
</div>

<div class="card">
<h3>⚙️ Calibration</h3>
<div class="btn-row">
<button class="btn btn-amber" id='calStart' data-act='/cal/start'>▶️ Start</button>
<button class="btn" id='calStop' data-act='/cal/stop' style="display:none">⏹️ Exit</button>
</div>
<div id='calSave' style="display:none;margin-top:10px">
<div class="btn-row">
<button class="btn btn-green" data-act='/cal/saveTop'>📍 Save Top</button>
<button class="btn" data-act='/cal/saveBottom'>📍 Save Bottom</button>
</div>
</div>
<div class="meta">Step: <code id='cur'>CUR_PLACEHOLDER</code> / <code id='max'>MAX_PLACEHOLDER</code></div>
</div>

<div class="card" id="homekit-card" style="HOMEKIT_DISPLAY">
<h3>🍎 HomeKit Pairing</h3>
<div class="code-box">
<div style="font-size:12px;color:#64748b;margin-bottom:8px">Setup Code</div>
<code>281-42-814</code>
<div style="font-size:11px;color:#475569;margin-top:8px">Home app &rarr; Add Accessory &rarr; Enter Code</div>
</div>
</div>

<div class="card">
<h3>🔧 System</h3>
<div class="btn-row">
<button class="btn" data-act='/reboot'>🔄 Reboot</button>
<button class="btn btn-red" data-act='/factory'>⚠️ Factory Reset</button>
</div>
</div>
</div>
<script>
)html";

// JS goes between BODY and END

const char WEB_HTML_END[] PROGMEM = "</script></body></html>";

#endif
