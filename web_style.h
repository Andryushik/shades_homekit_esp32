#ifndef WEB_STYLE_H
#define WEB_STYLE_H

#include <Arduino.h>

const char WEB_CSS[] PROGMEM = R"css(
*{box-sizing:border-box}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;margin:0;padding:16px;background:#0f172a;color:#e2e8f0}
.container{max-width:420px;margin:0 auto}
.card{background:#1e293b;border:1px solid #334155;border-radius:12px;padding:20px;margin:12px 0}
h1{margin:0 0 12px;font-size:22px;color:#f1f5f9;text-align:center;font-weight:600}
h3{margin:0 0 12px;font-size:16px;color:#cbd5e1;font-weight:500}
.badge{display:inline-block;padding:4px 12px;border-radius:9999px;font-size:14px;font-weight:600;text-transform:uppercase;letter-spacing:.5px}
.badge-normal{background:#065f4620;color:#34d399;border:1px solid #065f4650}
.badge-calibrate{background:#92400e20;color:#fbbf24;border:1px solid #92400e50;animation:pulse 1.5s infinite}
.progress-bar{background:#0f172a;border-radius:6px;height:8px;margin:12px 0;overflow:hidden;border:1px solid #334155}
.progress-fill{height:100%;background:linear-gradient(90deg,#3b82f6,#22d3ee);transition:width .3s;border-radius:6px}
.pos-text{text-align:center;font-size:32px;font-weight:700;color:#f1f5f9;margin:4px 0;font-variant-numeric:tabular-nums}
.pos-unit{font-size:16px;color:#64748b}
.pos-label{text-align:center;font-size:12px;color:#64748b;margin-bottom:8px}
.status-line{margin:10px 0;padding:0;text-align:center;font-size:12px;color:#cbd5e1;min-height:16px}
.sl-ok{color:#4ade80}
.sl-err{color:#f87171}
.btn-row{display:flex;gap:8px;margin:10px 0}
.btn{flex:1;min-width:0;padding:10px 8px;border:1px solid #475569;border-radius:8px;font-size:14px;font-weight:500;cursor:pointer;transition:all .15s;text-align:center;background:#334155;color:#e2e8f0}
.btn:hover{background:#475569;border-color:#64748b}
.btn:active{transform:scale(.97)}
.btn:disabled{opacity:.35;cursor:not-allowed;transform:none}
.btn-blue{background:#3b82f6;border-color:#3b82f6;color:#fff}
.btn-blue:hover{background:#2563eb}
.btn-green{background:#059669;border-color:#059669;color:#fff}
.btn-green:hover{background:#047857}
.btn-red{background:#dc2626;border-color:#dc2626;color:#fff}
.btn-red:hover{background:#b91c1c}
.btn-amber{background:#d97706;border-color:#d97706;color:#fff}
.btn-amber:hover{background:#b45309}
.blink{animation:pulse 1s infinite}
@keyframes pulse{0%{opacity:1}50%{opacity:.5}100%{opacity:1}}
.code-box{background:#0f172a;border:1px solid #334155;border-radius:8px;padding:16px;text-align:center;margin:8px 0}
.code-box code{font-size:28px;font-weight:700;color:#f1f5f9;letter-spacing:4px}
.meta{font-size:12px;color:#cbd5e1;text-align:center;margin-top:10px;font-variant-numeric:tabular-nums}
.meta code{color:#cbd5e1;background:none;font-size:12px}
)css";

#endif
