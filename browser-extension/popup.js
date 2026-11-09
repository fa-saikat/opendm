// OpenDM Browser Integration
// SPDX-License-Identifier: GPL-3.0-or-later

"use strict";

const api = globalThis.browser ?? globalThis.chrome;

const DEFAULTS = { captureDownloads: true, minSizeKiB: 0 };
const SETUP_URL = "https://github.com/fa-saikat/opendm#browser-integration";

const capture = document.getElementById("captureDownloads");
const minSize = document.getElementById("minSizeKiB");
const status = document.getElementById("status");

async function loadSettings() {
  const s = { ...DEFAULTS, ...(await api.storage.local.get(Object.keys(DEFAULTS))) };
  capture.checked = Boolean(s.captureDownloads);
  minSize.value = String(s.minSizeKiB);
}

capture.addEventListener("change", () => {
  api.storage.local.set({ captureDownloads: capture.checked });
});

minSize.addEventListener("change", () => {
  const n = Math.max(0, Math.floor(Number(minSize.value) || 0));
  minSize.value = String(n);
  api.storage.local.set({ minSizeKiB: n });
});

function showStatus(text, cls, withLink) {
  status.textContent = text;
  status.className = `status ${cls}`;
  if (withLink) {
    const a = document.createElement("a");
    a.href = SETUP_URL;
    a.target = "_blank";
    a.rel = "noopener";
    a.textContent = "Setup instructions";
    status.append(" ", a);
  }
}

async function checkHost() {
  let res;
  try {
    res = await api.runtime.sendMessage({ type: "host-info" });
  } catch (err) {
    res = { ok: false, error: String(err) };
  }
  if (res && res.ok && res.reply && res.reply.status === "ok") {
    const v = res.reply.version ? ` ${res.reply.version}` : "";
    showStatus(`Connected to OpenDM${v}.`, "ok", false);
  } else {
    showStatus("OpenDM's native host isn't installed or can't start.", "bad", true);
  }
}

loadSettings().catch(() => {});
checkHost();
