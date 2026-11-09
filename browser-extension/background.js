// OpenDM Browser Integration
// Copyright (C) 2026 FA Saikat
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Hands browser downloads, links and videos to the OpenDM download manager
// through its native messaging host (io.github.opendm). OpenDM was created by
// Nader Mahbub Khan (https://github.com/nadermkhan/opendm).
//
// Runs as a Chrome MV3 service worker and as a Firefox MV3 background script:
// every listener is registered synchronously at the top level and no state is
// kept in globals that must survive a restart.

"use strict";

// Firefox promises the `browser` namespace; Chrome only has `chrome`.
const api = globalThis.browser ?? globalThis.chrome;

const HOST = "io.github.opendm";

const DEFAULTS = {
  captureDownloads: true,
  // Downloads smaller than this (when the size is known) stay in the browser.
  minSizeKiB: 0,
};

// Statuses the host returns when OpenDM has taken over a download.
const ACCEPTED = new Set(["ok", "prompting", "started", "downloading"]);

const MENU_LINK = "opendm-link";
const MENU_MEDIA = "opendm-media";
const MENU_PAGE_VIDEO = "opendm-page-video";

const YOUTUBE_PAGES = [
  "*://www.youtube.com/watch*",
  "*://m.youtube.com/watch*",
  "*://music.youtube.com/watch*",
  "*://www.youtube.com/shorts/*",
];

async function getSettings() {
  try {
    return { ...DEFAULTS, ...(await api.storage.local.get(Object.keys(DEFAULTS))) };
  } catch {
    return { ...DEFAULTS };
  }
}

function isHttpUrl(url) {
  return typeof url === "string" && /^https?:\/\//i.test(url);
}

function baseName(path) {
  if (!path) return "";
  const parts = path.split(/[\\/]/);
  return parts[parts.length - 1] || "";
}

async function cookieHeader(url, storeId) {
  try {
    const query = { url };
    if (storeId) query.storeId = storeId;
    const cookies = await api.cookies.getAll(query);
    return cookies.map((c) => `${c.name}=${c.value}`).join("; ");
  } catch {
    return "";
  }
}

async function sendToHost(message) {
  return api.runtime.sendNativeMessage(HOST, message);
}

// Builds and sends a "download" request. Returns the host's reply, or throws
// if the native host is not installed or failed.
async function sendDownload({ url, referrer, filename, storeId }) {
  const message = {
    action: "download",
    url,
    user_agent: navigator.userAgent,
  };
  if (referrer && isHttpUrl(referrer)) message.referer = referrer;
  if (filename) message.filename = filename;
  const cookies = await cookieHeader(url, storeId);
  if (cookies) message.cookies = cookies;
  return sendToHost(message);
}

// ---------- Capturing browser downloads ----------

async function handOver(item) {
  const settings = await getSettings();
  if (!settings.captureDownloads) return;
  if (item.incognito) return; // Private windows stay private.
  if (item.byExtensionId) return; // Started by an extension; leave it alone.
  if (item.state && item.state !== "in_progress") return;

  const url = item.finalUrl || item.url;
  if (!isHttpUrl(url)) return; // blob:, data:, file: etc. only exist in the browser.

  const size = item.totalBytes > 0 ? item.totalBytes : item.fileSize;
  if (settings.minSizeKiB > 0 && size > 0 && size < settings.minSizeKiB * 1024) return;

  // Hold the download while OpenDM decides; resume it if OpenDM declines.
  let paused = false;
  try {
    await api.downloads.pause(item.id);
    paused = true;
  } catch {
    // Already finished or not pausable; a tiny file isn't worth taking over.
    return;
  }

  let reply = null;
  try {
    reply = await sendDownload({
      url,
      referrer: item.referrer,
      filename: baseName(item.filename),
      storeId: item.cookieStoreId,
    });
  } catch (err) {
    console.warn("OpenDM: native host unavailable, keeping the download in the browser.", err);
  }

  if (reply && ACCEPTED.has(reply.status)) {
    try {
      await api.downloads.cancel(item.id);
      await api.downloads.erase({ id: item.id });
    } catch (err) {
      console.warn("OpenDM: could not remove the browser's copy of the download.", err);
    }
    return;
  }

  if (paused) {
    try {
      await api.downloads.resume(item.id);
    } catch (err) {
      console.warn("OpenDM: could not resume the browser download.", err);
    }
  }
}

api.downloads.onCreated.addListener((item) => {
  handOver(item).catch((err) => console.error("OpenDM:", err));
});

// ---------- Context menu ----------

async function createMenus() {
  await api.contextMenus.removeAll();
  api.contextMenus.create({
    id: MENU_LINK,
    title: "Download link with OpenDM",
    contexts: ["link"],
  });
  api.contextMenus.create({
    id: MENU_MEDIA,
    title: "Download with OpenDM",
    contexts: ["image", "video", "audio"],
  });
  api.contextMenus.create({
    id: MENU_PAGE_VIDEO,
    title: "Download this video with OpenDM",
    contexts: ["page"],
    documentUrlPatterns: YOUTUBE_PAGES,
  });
}

const createMenusSafely = () => createMenus().catch((err) => console.error("OpenDM:", err));
api.runtime.onInstalled.addListener(createMenusSafely);
api.runtime.onStartup.addListener(createMenusSafely);

api.contextMenus.onClicked.addListener((info, tab) => {
  let url = "";
  if (info.menuItemId === MENU_LINK) url = info.linkUrl;
  else if (info.menuItemId === MENU_MEDIA) url = info.srcUrl;
  else if (info.menuItemId === MENU_PAGE_VIDEO) url = info.pageUrl;
  if (!isHttpUrl(url)) return;

  sendDownload({
    url,
    referrer: info.pageUrl,
    storeId: tab && tab.cookieStoreId,
  }).catch((err) => console.warn("OpenDM: native host unavailable.", err));
});

// ---------- Popup ----------

api.runtime.onMessage.addListener((msg, _sender, sendResponse) => {
  if (msg && msg.type === "host-info") {
    sendToHost({ action: "host_info" })
      .then((reply) => sendResponse({ ok: true, reply }))
      .catch((err) => sendResponse({ ok: false, error: String((err && err.message) || err) }));
    return true; // Responds asynchronously.
  }
  return false;
});
