var SETTINGS_PREFIX = 'pebbleTeams.';
var graphImage = require('./image');
var GRAPH_ROOT = 'https://graph.microsoft.com/v1.0';
var LOGIN_ROOT = 'https://login.microsoftonline.com';
// Public-client application IDs are not secrets, but this prototype should not
// ship a tenant-specific value by accident. If we get a dedicated Pebble Teams
// app registration, place its client ID here and the settings UI will become a
// one-tap user sign-in flow.
var BUILT_IN_CLIENT_ID = '10ded14b-8b85-4ffe-a778-b2ec3873c327';
var DEFAULT_TENANT = 'organizations';
var OLD_DEFAULT_SCOPES = [
  'offline_access',
  'User.Read',
  'User.ReadBasic.All',
  'Chat.ReadWrite',
  'ChatMessage.Send',
  'Tasks.ReadWrite'
].join(' ');
var DEFAULT_SCOPES = [
  'offline_access',
  'User.Read',
  'User.ReadBasic.All',
  'Chat.Read',
  'ChatMessage.Read',
  'ChatMessage.Send',
  'Tasks.ReadWrite'
].join(' ');
var DEFAULT_PLANNER_LABELS = ['Category 1', 'Category 2', 'Category 3', 'Category 4', 'Category 5', 'Category 6'];
var tokenMaps = {
  chat: {},
  message: {},
  plan: {},
  bucket: {},
  task: {},
  assignTeam: {},
  assignUser: {}
};
var reverseTokenMaps = {
  chat: {},
  message: {},
  plan: {},
  bucket: {},
  task: {},
  assignTeam: {},
  assignUser: {}
};
var tokenCounters = {
  chat: 0,
  message: 0,
  plan: 0,
  bucket: 0,
  task: 0,
  assignTeam: 0,
  assignUser: 0
};
var taskCache = {};
var bucketTaskCache = {};
var bucketPlanMap = {};
var bucketNameMap = {};
var planLabelCache = {};
var taskChecklistCache = {};
var taskAssignmentCache = {};
var chatPeerCache = {};
var chatPeerOrder = [];
var graphGroupChatRows = [];
var meCache = null;
var activeDeviceCode = null;
var pendingDeviceCodeCallback = null;
var tokenPromise = null;
var deviceCodeSeq = 0;

var mockChats = [
  {id: 'mock-chat-ops', title: 'IT Operations', preview: 'Maya: Planner card is due today; I added the rollout checklist.', kind: 'group', unread: true, unread_count: 3, updated: 500},
  {id: 'mock-chat-manager', title: 'Jordan Lee', preview: 'Can you send the Pebble Teams approval notes?', kind: 'oneOnOne', unread: true, unread_count: 1, updated: 400},
  {id: 'mock-chat-change', title: 'Change Advisory Group', preview: 'Alex: Meeting moved. See [Link] teams.microsoft.com', kind: 'group', unread: false, unread_count: 0, updated: 300}
];
var mockMessages = {
  'mock-chat-ops': [
    {id: '1001', sender: 'Maya', text: 'Morning. I added the Intune rollout checklist to Planner.', outgoing: false, meta: '9:04'},
    {id: '1002', sender: 'You', text: 'Great. I am testing the watch flow now.', outgoing: true, meta: '9:06|1', reactions: '👍'},
    {id: '1003', sender: 'Sam', text: 'The Teams order should follow desktop/mobile as closely as Graph allows.', outgoing: false, meta: '9:08'},
    {id: '1004', sender: 'You', text: 'This outgoing reply should be purple.', outgoing: true, meta: '9:10|1', reply_sender: 'Maya', reply_text: 'I added the Intune rollout checklist to Planner.'},
    {id: '1005', sender: 'Maya', text: 'Planner card is due today; I added the rollout checklist.', outgoing: false, meta: '9:11', reply_sender: 'You', reply_text: 'This outgoing reply should be purple.'}
  ],
  'mock-chat-manager': [
    {id: '2001', sender: 'Jordan', text: 'Can you send the Pebble Teams approval notes?', outgoing: false, meta: '8:42'},
    {id: '2002', sender: 'You', text: 'Yes. Version 0.2 starts wiring Microsoft Graph and Planner.', outgoing: true, meta: '8:44|1'},
    {id: '2003', sender: 'Jordan', text: 'The quoted reply should show inside this incoming grey bubble.', outgoing: false, meta: '8:46', reply_sender: 'You', reply_text: 'Version 0.2 starts wiring Microsoft Graph and Planner.'},
    {id: '2004', sender: 'You', text: 'This outgoing bubble should be light Teams purple with a darker quote.', outgoing: true, meta: '8:48|1', reply_sender: 'Jordan', reply_text: 'The quoted reply should show inside this incoming grey bubble.'},
    {id: '2005', sender: 'Jordan', text: 'Can you send the latest screenshots?', outgoing: false, meta: '8:49'}
  ],
  'mock-chat-change': [
    {id: '3001', sender: 'Alex', text: 'Meeting moved. See https://teams.microsoft.com/l/meetup-join/very/long/link/that/the/watch/should/shorten', outgoing: false, meta: 'Yesterday'},
    {id: '3002', sender: 'Priya', text: 'I edited the change record and marked the risky step as blocked.', outgoing: false, meta: 'Edited'}
  ]
};
var mockPlans = [
  {id: 'mock-plan-it', title: 'IT Rollout Plan', preview: 'Infrastructure team board - 4 active cards', kind: 'plan'},
  {id: 'mock-plan-sec', title: 'Security Review', preview: 'Security team board - due dates ready for pins', kind: 'plan'}
];
var mockBuckets = {
  'mock-plan-it': [
    {id: 'mock-bucket-now', title: 'In progress', preview: '2 cards in this bucket', kind: 'bucket'},
    {id: 'mock-bucket-next', title: 'Next', preview: '2 cards in this bucket', kind: 'bucket'}
  ],
  'mock-plan-sec': [
    {id: 'mock-bucket-review', title: 'Review', preview: '2 cards in this bucket', kind: 'bucket'}
  ]
};
var mockTasks = {
  'mock-bucket-now': [
    {id: 'mock-task-intune', title: 'Approve Intune rollout', preview: 'Due today - 50% complete', due: 'Today', percentComplete: 50, kind: 'task'},
    {id: 'mock-task-pbw', title: 'Test Pebble Teams PBW', preview: 'Checklist 2/4 - not complete', due: 'Tomorrow', percentComplete: 0, kind: 'task'}
  ],
  'mock-bucket-next': [
    {id: 'mock-task-consent', title: 'Approve Graph app consent', preview: 'Needs IT manager approval', due: 'Friday', percentComplete: 0, kind: 'task'},
    {id: 'mock-task-pins', title: 'Timeline pins for Planner', preview: 'Stretch after task writes', due: 'Next week', percentComplete: 0, kind: 'task'}
  ],
  'mock-bucket-review': [
    {id: 'mock-task-scopes', title: 'Review delegated scopes', preview: 'Chat.Read, ChatMessage.Read, Tasks.ReadWrite', due: 'Friday', percentComplete: 0, kind: 'task'},
    {id: 'mock-task-token', title: 'Token storage review', preview: 'Phone localStorage only in prototype', due: 'Next week', percentComplete: 0, kind: 'task'}
  ]
};
var mockTaskDetails = {
  'mock-task-intune': [
    {id: 'status', sender: 'Status', text: 'In progress - 50% complete', kind: 'status', outgoing: false, meta: 'Planner'},
    {id: 'notes', sender: 'Notes', text: 'Approve the staged Intune rollout after the helpdesk note is added. Keep this card pinned once Timeline support lands.', kind: 'notes', outgoing: false},
    {id: 'checklist', sender: 'Checklist', text: '[x] Pilot users\n[ ] Broad deployment\n[ ] Helpdesk note', kind: 'checklist', outgoing: false},
    {id: 'due', sender: 'Due', text: 'Today', kind: 'due', outgoing: false}
  ],
  'mock-task-pbw': [
    {id: 'status', sender: 'Status', text: 'Not started', kind: 'status', outgoing: false, meta: 'Planner'},
    {id: 'notes', sender: 'Notes', text: 'Use Emery as the design target, then sanity-check Gabbro before widening support.', kind: 'notes', outgoing: false},
    {id: 'checklist', sender: 'Checklist', text: '[x] Build PBW\n[x] Launch Emery emulator\n[ ] Add Graph auth\n[ ] Add Planner writes', kind: 'checklist', outgoing: false},
    {id: 'due', sender: 'Due', text: 'Tomorrow', kind: 'due', outgoing: false}
  ]
};

var reactionGlyphs = {
  like: '👍',
  heart: '❤',
  laugh: '😂',
  wow: '😱',
  sad: '😢',
  angry: '😡'
};

function graphReactionType(token) {
  if (reactionGlyphs[token]) {
    return reactionGlyphs[token];
  }
  return token && /[^\x00-\x7f]/.test(token) ? token : '';
}

function setting(name, fallback) {
  var value = localStorage.getItem(SETTINGS_PREFIX + name);
  return value === null || value === '' ? fallback : value;
}

function saveSetting(name, value) {
  if (value === undefined || value === null || value === '') {
    localStorage.removeItem(SETTINGS_PREFIX + name);
  } else {
    localStorage.setItem(SETTINGS_PREFIX + name, String(value));
  }
}

function removeSetting(name) {
  localStorage.removeItem(SETTINGS_PREFIX + name);
}

function splitSettingLines(value) {
  return String(value || '').split(/\r?\n|\|/).map(function(item) {
    return item.trim();
  }).filter(function(item) {
    return !!item;
  });
}

function plannerLabelsPacked() {
  var rows = splitSettingLines(setting('plannerLabels', ''));
  var labels = DEFAULT_PLANNER_LABELS.map(function(fallback, index) {
    return rows[index] || fallback;
  });
  return labels.slice(0, 6).join('|');
}

function plannerLabelName(index) {
  return plannerLabelsPacked().split('|')[index] || DEFAULT_PLANNER_LABELS[index] || ('Label ' + (index + 1));
}

function parseAssignContactLine(line) {
  var text = String(line || '').trim();
  var match = text.match(/^(.*?)\s*<([^>]+)>$/);
  var title = match ? match[1].trim() : text;
  var lookup = match ? match[2].trim() : text;
  if (!title) {
    title = lookup;
  }
  return title ? {title: title, lookup: lookup} : null;
}

function configuredAssignContacts() {
  return splitSettingLines(setting('assignContacts', '')).map(parseAssignContactLine).filter(function(contact) {
    return !!contact;
  }).slice(0, 12);
}

function assignContactLine(contact) {
  var title = contact && contact.title || '';
  var lookup = contact && contact.lookup || title;
  return title + (lookup && lookup !== title ? ' <' + lookup + '>' : '');
}

function configuredAssignRows() {
  return configuredAssignContacts().map(function(contact) {
    return {
      id: tokenFor('assignUser', 'configured:' + contact.lookup),
      title: contact.title,
      preview: contact.lookup,
      kind: 'assignMember'
    };
  });
}

function clearAuthState() {
  removeSetting('accessToken');
  removeSetting('refreshToken');
  removeSetting('expiresAt');
  removeSetting('deviceUserCode');
  removeSetting('deviceVerificationUri');
  removeSetting('deviceCodeExpiresAt');
  activeDeviceCode = null;
  tokenPromise = null;
  deviceCodeSeq += 1;
}

function useGraph() {
  return setting('mockMode', '') !== '1' && !!authClientId();
}

function authClientId() {
  return setting('clientId', BUILT_IN_CLIENT_ID);
}

function authTenantId() {
  return setting('tenantId', DEFAULT_TENANT);
}

function authScopes() {
  var scopes = setting('scopes', '');
  return !scopes || scopes === OLD_DEFAULT_SCOPES ? DEFAULT_SCOPES : scopes;
}

function graphConfigured() {
  return !!authClientId();
}

function deviceCodeActive() {
  return !!(activeDeviceCode && activeDeviceCode.device_code &&
    (!activeDeviceCode.expiresAt || Date.now() < activeDeviceCode.expiresAt));
}

function showActiveDeviceCode(status) {
  if (!deviceCodeActive()) {
    return;
  }
  if (status) {
    status('Go to microsoft.com/devicelogin');
    status('Code ' + (activeDeviceCode.user_code || 'pending'));
  }
  if (pendingDeviceCodeCallback) {
    pendingDeviceCodeCallback(activeDeviceCode);
  }
}

function clone(value) {
  return JSON.parse(JSON.stringify(value));
}

function htmlEscape(value) {
  return String(value || '').replace(/[&<>"']/g, function(ch) {
    return {'&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;'}[ch];
  });
}

function jsLiteral(value) {
  return JSON.stringify(String(value || ''));
}

function formEncode(values) {
  var parts = [];
  Object.keys(values).forEach(function(key) {
    if (values[key] !== undefined && values[key] !== null) {
      parts.push(encodeURIComponent(key) + '=' + encodeURIComponent(values[key]));
    }
  });
  return parts.join('&');
}

function http(method, url, body, headers) {
  return new Promise(function(resolve, reject) {
    var xhr = new XMLHttpRequest();
    xhr.open(method, url, true);
    Object.keys(headers || {}).forEach(function(name) {
      xhr.setRequestHeader(name, headers[name]);
    });
    xhr.onreadystatechange = function() {
      if (xhr.readyState !== 4) {
        return;
      }
      var text = xhr.responseText || '';
      var data = null;
      if (text) {
        try {
          data = JSON.parse(text);
        } catch (e) {
          data = {raw: text};
        }
      }
      if (xhr.status >= 200 && xhr.status < 300) {
        resolve(data || {});
      } else {
        var message = data && (data.error_description || (data.error && data.error.message) || data.error);
        var err = new Error(message || ('HTTP ' + xhr.status));
        err.status = xhr.status;
        err.data = data;
        reject(err);
      }
    };
    xhr.onerror = function() {
      reject(new Error('Network request failed'));
    };
    xhr.send(body || null);
  });
}

function identityUrl(path) {
  return LOGIN_ROOT + '/' + encodeURIComponent(authTenantId()) + '/oauth2/v2.0/' + path;
}

function saveTokens(data) {
  if (data.access_token) {
    saveSetting('accessToken', data.access_token);
    saveSetting('expiresAt', String(Date.now() + Math.max(60, Number(data.expires_in || 3600) - 90) * 1000));
    removeSetting('deviceUserCode');
    removeSetting('deviceVerificationUri');
    removeSetting('deviceCodeExpiresAt');
    activeDeviceCode = null;
  }
  if (data.refresh_token) {
    saveSetting('refreshToken', data.refresh_token);
  }
}

function tokenValid() {
  return setting('accessToken', '') && Number(setting('expiresAt', '0')) > Date.now();
}

function refreshToken() {
  var refresh = setting('refreshToken', '');
  if (!refresh) {
    return Promise.reject(new Error('Microsoft sign-in required'));
  }
  return http('POST', identityUrl('token'), formEncode({
    client_id: authClientId(),
    grant_type: 'refresh_token',
    refresh_token: refresh,
    scope: authScopes()
  }), {'Content-Type': 'application/x-www-form-urlencoded'}).then(function(data) {
    saveTokens(data);
    return setting('accessToken', '');
  });
}

function startDeviceCode(status) {
  if (!authClientId()) {
    return Promise.reject(new Error('Microsoft app ID missing'));
  }
  deviceCodeSeq += 1;
  var seq = deviceCodeSeq;
  removeSetting('deviceUserCode');
  removeSetting('deviceVerificationUri');
  removeSetting('deviceCodeExpiresAt');
  return http('POST', identityUrl('devicecode'), formEncode({
    client_id: authClientId(),
    scope: authScopes()
  }), {'Content-Type': 'application/x-www-form-urlencoded'}).then(function(data) {
    data.expiresAt = Date.now() + Math.max(60, Number(data.expires_in || 900)) * 1000;
    data.seq = seq;
    activeDeviceCode = data;
    saveSetting('deviceUserCode', data.user_code || '');
    saveSetting('deviceVerificationUri', data.verification_uri || data.verification_url || 'https://microsoft.com/devicelogin');
    saveSetting('deviceCodeExpiresAt', String(data.expiresAt));
    if (status) {
      status('Go to microsoft.com/devicelogin');
      status('Code ' + (data.user_code || 'in settings'));
    }
    if (pendingDeviceCodeCallback) {
      pendingDeviceCodeCallback(data);
    }
    return pollDeviceCode(status, seq);
  });
}

function pollDeviceCode(status, seq) {
  var code = activeDeviceCode;
  if (!code || !code.device_code) {
    return Promise.reject(new Error('Microsoft sign-in required'));
  }
  seq = seq || code.seq || deviceCodeSeq;
  var interval = Math.max(5, Number(code.interval || 5)) * 1000;
  var expiresAt = code.expiresAt || (Date.now() + Math.max(60, Number(code.expires_in || 900)) * 1000);

  function poll() {
    if (!activeDeviceCode || activeDeviceCode.device_code !== code.device_code || seq !== deviceCodeSeq) {
      return Promise.reject(new Error('Microsoft sign-in restarted'));
    }
    if (Date.now() >= expiresAt) {
      if (activeDeviceCode && activeDeviceCode.device_code === code.device_code) {
        activeDeviceCode = null;
        removeSetting('deviceUserCode');
        removeSetting('deviceVerificationUri');
        removeSetting('deviceCodeExpiresAt');
      }
      return Promise.reject(new Error('Microsoft sign-in expired'));
    }
    return http('POST', identityUrl('token'), formEncode({
      grant_type: 'urn:ietf:params:oauth:grant-type:device_code',
      client_id: authClientId(),
      device_code: code.device_code
    }), {'Content-Type': 'application/x-www-form-urlencoded'}).then(function(data) {
      if (!activeDeviceCode || activeDeviceCode.device_code !== code.device_code || seq !== deviceCodeSeq) {
        throw new Error('Microsoft sign-in restarted');
      }
      activeDeviceCode = null;
      saveTokens(data);
      if (status) {
        status('Microsoft connected');
      }
      return setting('accessToken', '');
    }).catch(function(err) {
      var errCode = err && err.data && err.data.error;
      if (errCode === 'authorization_pending' || errCode === 'slow_down') {
        if (status) {
          status('Go to microsoft.com/devicelogin');
          status('Code ' + (code.user_code || 'pending'));
        }
        return new Promise(function(resolve) {
          setTimeout(resolve, errCode === 'slow_down' ? interval + 5000 : interval);
        }).then(poll);
      }
      if (activeDeviceCode && activeDeviceCode.device_code === code.device_code) {
        activeDeviceCode = null;
        removeSetting('deviceUserCode');
        removeSetting('deviceVerificationUri');
        removeSetting('deviceCodeExpiresAt');
      }
      throw err;
    });
  }

  return poll();
}

function ensureToken(status, interactive) {
  if (!useGraph()) {
    return Promise.reject(new Error('Graph not configured'));
  }
  if (tokenValid()) {
    return Promise.resolve(setting('accessToken', ''));
  }
  if (tokenPromise) {
    return tokenPromise;
  }
  tokenPromise = refreshToken().catch(function(err) {
    if (interactive) {
      return startDeviceCode(status);
    }
    throw err;
  }).then(function(token) {
    tokenPromise = null;
    return token;
  }, function(err) {
    tokenPromise = null;
    throw err;
  });
  return tokenPromise;
}

function graph(path, options) {
  options = options || {};
  return ensureToken(options.status, false).then(function(token) {
    var headers = options.headers || {};
    headers.Authorization = 'Bearer ' + token;
    if (options.body !== undefined) {
      headers['Content-Type'] = 'application/json';
    }
    return http(options.method || 'GET', /^https?:\/\//.test(path) ? path : GRAPH_ROOT + path,
      options.body === undefined ? null : JSON.stringify(options.body), headers);
  });
}

function httpBinary(method, url, body, headers) {
  return new Promise(function(resolve, reject) {
    var xhr = new XMLHttpRequest();
    xhr.open(method, url, true);
    try {
      xhr.responseType = 'arraybuffer';
    } catch (e) {}
    Object.keys(headers || {}).forEach(function(name) {
      xhr.setRequestHeader(name, headers[name]);
    });
    xhr.onreadystatechange = function() {
      if (xhr.readyState !== 4) {
        return;
      }
      if (xhr.status >= 200 && xhr.status < 300) {
        var response = xhr.response;
        var bytes;
        var i;
        if (response && response.byteLength !== undefined) {
          bytes = new Uint8Array(response);
        } else {
          var text = xhr.responseText || '';
          bytes = new Uint8Array(text.length);
          for (i = 0; i < text.length; i += 1) {
            bytes[i] = text.charCodeAt(i) & 0xff;
          }
        }
        resolve(bytes);
      } else {
        reject(new Error('HTTP ' + xhr.status));
      }
    };
    xhr.onerror = function() {
      reject(new Error('Network request failed'));
    };
    xhr.send(body || null);
  });
}

function graphBinary(path, options) {
  options = options || {};
  return ensureToken(options.status, false).then(function(token) {
    var headers = options.headers || {};
    headers.Authorization = 'Bearer ' + token;
    return httpBinary(options.method || 'GET',
      /^https?:\/\//.test(path) ? path : GRAPH_ROOT + path,
      options.body || null, headers);
  });
}

function odataString(value) {
  return "'" + String(value || '').replace(/'/g, "''") + "'";
}

function pseudoGuid() {
  var template = 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx';
  return template.replace(/[xy]/g, function(ch) {
    var r = Math.floor(Math.random() * 16);
    var v = ch === 'x' ? r : ((r & 0x3) | 0x8);
    return v.toString(16);
  });
}

function graphMe(status) {
  if (meCache && meCache.id) {
    return Promise.resolve(meCache);
  }
  return graph('/me?$select=id,displayName,userPrincipalName', {status: status}).then(function(me) {
    meCache = me || {};
    return meCache;
  });
}

function userActionBody(status) {
  return graphMe(status).then(function(me) {
    var user = {id: me.id};
    var tenantId = setting('tenantId', '');
    if (/^[0-9a-f-]{36}$/i.test(tenantId)) {
      user.tenantId = tenantId;
    }
    return {user: user};
  });
}

function realMessageId(chatToken, messageToken) {
  var compound = realId('message', messageToken);
  var chatId = realId('chat', chatToken);
  var prefix = chatId + ':';
  if (compound.indexOf(prefix) === 0) {
    return compound.substring(prefix.length);
  }
  return compound.split(':').pop();
}

function graphChatUserAction(chatToken, action, extraBody, status) {
  return userActionBody(status).then(function(body) {
    Object.keys(extraBody || {}).forEach(function(key) {
      body[key] = extraBody[key];
    });
    return graph('/chats/' + encodeURIComponent(realId('chat', chatToken)) + '/' + action, {
      method: 'POST',
      status: status,
      body: body
    });
  });
}

function tokenFor(kind, realId) {
  realId = String(realId || '');
  if (!realId) {
    return '';
  }
  if (reverseTokenMaps[kind][realId]) {
    return reverseTokenMaps[kind][realId];
  }
  tokenCounters[kind] += 1;
  var token = kind.charAt(0) + tokenCounters[kind];
  reverseTokenMaps[kind][realId] = token;
  tokenMaps[kind][token] = realId;
  return token;
}

function realId(kind, token) {
  return tokenMaps[kind][String(token || '')] || String(token || '');
}

function stripHtml(value) {
  return String(value || '')
    .replace(/<br\s*\/?>/gi, '\n')
    .replace(/<\/p>/gi, '\n')
    .replace(/<[^>]+>/g, '')
    .replace(/&nbsp;/g, ' ')
    .replace(/&amp;/g, '&')
    .replace(/&lt;/g, '<')
    .replace(/&gt;/g, '>')
    .replace(/&quot;/g, '"')
    .replace(/&#39;/g, "'")
    .replace(/\n{3,}/g, '\n\n')
    .trim();
}

function stripSenderPrefix(preview, sender) {
  var text = String(preview || '').trim();
  var name = String(sender || '').trim();
  if (name && text.indexOf(name + ':') === 0) {
    text = text.substring(name.length + 1).trim();
  }
  return text;
}

function graphLastMessagePreviewText(chat) {
  var preview = chat && chat.lastMessagePreview;
  if (!preview) {
    return 'No recent message';
  }
  var body = preview.body && stripHtml(preview.body.content);
  if (body) {
    return body;
  }
  var media = messageMediaLabel(preview);
  if (media) {
    return media;
  }
  return 'Deleted message';
}

function chatLatestPreviewNeedsDetail(chat) {
  var preview = chat && chat.lastMessagePreview;
  if (!preview || !preview.body) {
    return false;
  }
  return !stripHtml(preview.body.content);
}

function messageSnippetText(message) {
  if (!message) {
    return '';
  }
  var body = message.deletedDateTime ? '' : stripHtml(message.body && message.body.content);
  if (body) {
    return body;
  }
  var media = messageMediaLabel(message);
  if (media) {
    return media;
  }
  return message.deletedDateTime ? 'Deleted message' : '';
}

function chatSortTimestamp(message) {
  var value = message && (message.createdDateTime || message.lastModifiedDateTime);
  var time = value ? Date.parse(value) : 0;
  return isNaN(time) ? 0 : time;
}

function extractHostedContentId(html) {
  var text = String(html || '');
  var match = text.match(/hostedContents\/([^"'<>/]+)\/\$value/i) ||
              text.match(/hostedcontents%2f([^"'<>%]+)%2f%24value/i);
  return match ? decodeURIComponent(match[1]) : '';
}

function attachmentName(attachment) {
  return String((attachment && (attachment.name || attachment.title || attachment.contentType)) || '').trim();
}

function attachmentIsMessageReference(attachment) {
  var type = String((attachment && attachment.contentType) || '').toLowerCase();
  return type === 'messagereference' || type.indexOf('messagereference') >= 0;
}

function attachmentLooksImage(attachment) {
  if (attachmentIsMessageReference(attachment)) {
    return false;
  }
  var name = attachmentName(attachment).toLowerCase();
  var type = String((attachment && attachment.contentType) || '').toLowerCase();
  return type.indexOf('image/') === 0 || /\.(png|jpe?g)$/i.test(name);
}

function attachmentLooksGif(attachment) {
  if (attachmentIsMessageReference(attachment)) {
    return false;
  }
  var name = attachmentName(attachment).toLowerCase();
  var type = String((attachment && attachment.contentType) || '').toLowerCase();
  return type === 'image/gif' || /\.gif$/i.test(name);
}

function hostedMediaLabel(html) {
  var text = String(html || '');
  var gif = text.match(/([^"'<>\\\/]+\.(?:gif))/i);
  if (gif && gif[1]) {
    return '[GIF] ' + gif[1];
  }
  var file = text.match(/([^"'<>\\\/]+\.(?:zip|7z|rar|pdf|docx?|xlsx?|pptx?|txt|csv|png|jpe?g|webp|heic|mp4|mov|gif))/i);
  if (file && file[1]) {
    return file[1];
  }
  if (extractHostedContentId(text)) {
    return '[Image]';
  }
  return /<attachment\b/i.test(text) ? '[Attachment]' : '';
}

function messageMediaLabel(message) {
  var attachments = message && message.attachments;
  if (!attachments || !attachments.length) {
    return hostedMediaLabel(message && message.body && message.body.content);
  }
  var mediaCount = 0;
  for (var i = 0; i < attachments.length; i += 1) {
    if (attachmentIsMessageReference(attachments[i])) {
      continue;
    }
    mediaCount += 1;
    var name = attachmentName(attachments[i]);
    if (name) {
      return attachmentLooksGif(attachments[i]) ? ('[GIF] ' + name) : name;
    }
  }
  if (mediaCount) {
    return mediaCount === 1 ? 'Media attachment' : (mediaCount + ' attachments');
  }
  return hostedMediaLabel(message && message.body && message.body.content);
}

function messageHasImage(message, html) {
  var attachments = message && message.attachments;
  if (extractHostedContentId(html)) {
    return true;
  }
  for (var i = 0; attachments && i < attachments.length; i += 1) {
    if (attachmentLooksImage(attachments[i]) || attachments[i].thumbnailUrl) {
      return true;
    }
  }
  return false;
}

function parseAttachmentContent(attachment) {
  var content = attachment && attachment.content;
  if (!content) {
    return null;
  }
  if (typeof content === 'object') {
    return content;
  }
  try {
    return JSON.parse(String(content));
  } catch (e) {
    return null;
  }
}

function messageReferenceContext(message) {
  var attachments = message && message.attachments;
  for (var i = 0; attachments && i < attachments.length; i += 1) {
    if (!attachmentIsMessageReference(attachments[i])) {
      continue;
    }
    var ref = parseAttachmentContent(attachments[i]);
    var senderUser = ref && ref.messageSender && ref.messageSender.user;
    var sender = senderUser && senderUser.displayName;
    var senderId = senderUser && senderUser.id;
    var preview = ref && (ref.messagePreview || ref.preview || ref.summary);
    if (preview) {
      return {
        sender: senderId && meCache && meCache.id && senderId === meCache.id ? 'You' : (sender || 'Reply'),
        text: stripHtml(preview),
        messageId: ref.messageId || ''
      };
    }
  }
  return null;
}

function extractReplyContext(html) {
  var text = String(html || '');
  var match = text.match(/<blockquote[^>]*>([\s\S]*?)<\/blockquote>/i) ||
              text.match(/<div[^>]*(?:reply|quote|quoted|original)[^>]*>([\s\S]*?)<\/div>/i) ||
              text.match(/<p[^>]*(?:reply|quote|quoted|original)[^>]*>([\s\S]*?)<\/p>/i);
  if (!match) {
    return null;
  }
  var body = stripHtml(match[1]).replace(/^Original message\s*/i, '').trim();
  return body ? {sender: 'Reply', text: body} : null;
}

function applyReplyFallback(row, replyMessage) {
  if (!row || !replyMessage || (row.reply_text && row.reply_text !== 'Quoted message')) {
    return;
  }
  var sender = replyMessage.from && replyMessage.from.user && replyMessage.from.user.displayName;
  var senderId = replyMessage.from && replyMessage.from.user && replyMessage.from.user.id;
  var body = stripHtml(replyMessage.body && replyMessage.body.content);
  if (body) {
    row.reply_sender = senderId && meCache && meCache.id && senderId === meCache.id ? 'You' : (sender || 'Reply');
    row.reply_text = body;
  }
}

function rememberChatPeer(chatToken, peer) {
  if (!chatToken || !peer || !peer.id) {
    return;
  }
  chatPeerCache[chatToken] = peer;
  chatPeerOrder = chatPeerOrder.filter(function(id) {
    return id !== chatToken;
  });
  chatPeerOrder.unshift(chatToken);
  if (chatPeerOrder.length > 16) {
    delete chatPeerCache[chatPeerOrder.pop()];
  }
}

function shortDate(value) {
  if (!value) {
    return '';
  }
  var d = new Date(value);
  if (isNaN(d.getTime())) {
    return '';
  }
  return (d.getMonth() + 1) + '/' + d.getDate();
}

function messageMetaTime(value, outgoing) {
  if (!value) {
    return outgoing ? '|1' : '';
  }
  var d = new Date(value);
  if (isNaN(d.getTime())) {
    return outgoing ? '|1' : '';
  }
  var now = new Date();
  var sameDay = d.getFullYear() === now.getFullYear() &&
                d.getMonth() === now.getMonth() &&
                d.getDate() === now.getDate();
  var text;
  if (sameDay) {
    var hours = d.getHours();
    var minutes = d.getMinutes();
    var suffix = hours >= 12 ? 'p' : 'a';
    hours = hours % 12;
    if (hours === 0) {
      hours = 12;
    }
    text = hours + ':' + (minutes < 10 ? '0' : '') + minutes + suffix;
  } else {
    text = (d.getMonth() + 1) + '/' + d.getDate();
  }
  return text + (outgoing ? '|1' : '');
}

function graphChatTitle(chat) {
  if (chat.topic) {
    return chat.topic;
  }
  if (chat.chatType === 'oneOnOne') {
    return '1:1 Chat';
  }
  return chat.chatType === 'meeting' ? 'Meeting Chat' : 'Group Chat';
}

function graphChatRow(chat, peer) {
  var preview = graphLastMessagePreviewText(chat);
  var from = chat.lastMessagePreview && chat.lastMessagePreview.from &&
             chat.lastMessagePreview.from.user && chat.lastMessagePreview.from.user.displayName;
  var token = tokenFor('chat', chat.id);
  if (chat.chatType === 'oneOnOne' && peer && peer.id) {
    rememberChatPeer(token, peer);
  }
  return {
    id: token,
    title: peer && peer.displayName ? peer.displayName : graphChatTitle(chat),
    preview: stripSenderPrefix(preview, from),
    unread: false,
    unread_count: 0,
    kind: chat.chatType || 'group'
  };
}

function enrichBlankChatPreview(chat, row, status) {
  if (!chat || !chat.id || !row || !chatLatestPreviewNeedsDetail(chat)) {
    return Promise.resolve(row);
  }
  return graph('/chats/' + encodeURIComponent(chat.id) + '/messages?$top=1&$orderby=createdDateTime desc', {
    status: status
  }).then(function(data) {
    var message = data && data.value && data.value[0];
    var snippet = messageSnippetText(message);
    if (snippet) {
      var from = message && message.from && message.from.user && message.from.user.displayName;
      row.preview = stripSenderPrefix(snippet, from);
    }
    return row;
  }).catch(function() {
    return row;
  });
}

function graphMessageRow(message, chatToken) {
  var from = message.from && message.from.user && message.from.user.displayName;
  var fromId = message.from && message.from.user && message.from.user.id;
  var html = message.body && message.body.content;
  var body = message.deletedDateTime ? '[Deleted]' : stripHtml(html);
  var mediaLabel = messageMediaLabel(message);
  if (mediaLabel && /^\[?unsupported message\]?$/i.test(body)) {
    body = '';
  }
  var reactions = (message.reactions || []).map(function(reaction) {
    return reactionGlyphs[reaction.reactionType] || reaction.reactionType || '';
  }).join('');
  var token = tokenFor('message', realId('chat', chatToken) + ':' + message.id);
  var graphReply = messageReferenceContext(message);
  var reply = graphReply || extractReplyContext(html);
  var row = {
    id: token,
    sender: from || 'Teams',
    text: body || mediaLabel || '[Unsupported message]',
    outgoing: !!(fromId && meCache && meCache.id && fromId === meCache.id),
    reactions: reactions,
    meta: messageMetaTime(message.createdDateTime, !!(fromId && meCache && meCache.id && fromId === meCache.id)),
    sort_ts: chatSortTimestamp(message)
  };
  if (reply) {
    row.reply_sender = reply.sender;
    row.reply_text = reply.text;
  } else if (message.replyToId) {
    row.reply_sender = 'Reply';
    row.reply_text = 'Quoted message';
    row._replyToId = message.replyToId;
  }
  if (messageHasImage(message, html)) {
    row.image_token = token;
  }
  return row;
}

function plannerHomeRow() {
  return {id: '__planner', title: 'Planner', preview: 'Team boards, cards, checklists, and due dates', kind: 'planner', unread: false, unread_count: 0};
}

function groupChatsHomeRow(count) {
  return {
    id: '__group_chats',
    title: 'Group Chats',
    preview: count === 1 ? '1 group chat' : (count + ' group chats'),
    kind: 'groupFolder',
    unread: false,
    unread_count: 0
  };
}

function composeMainChatRows(direct, groups, limit) {
  var max = Math.max(1, limit || 20);
  var rows = [plannerHomeRow()];
  var directLimit = groups && groups.length ? Math.max(0, max - 2) : Math.max(0, max - 1);
  rows = rows.concat((direct || []).slice(0, directLimit));
  if (groups && groups.length && rows.length < max) {
    rows.push(groupChatsHomeRow(groups.length));
  }
  return rows.slice(0, max);
}

function pinnedPlanIds() {
  try {
    return JSON.parse(localStorage.getItem(SETTINGS_PREFIX + 'pinnedPlans') || '[]');
  } catch (e) {
    return [];
  }
}

function savePinnedPlanIds(ids) {
  localStorage.setItem(SETTINGS_PREFIX + 'pinnedPlans', JSON.stringify(ids || []));
}

function applyPinnedPlanOrder(rows) {
  var pins = pinnedPlanIds();
  if (!pins.length) {
    return rows;
  }
  var pinned = [];
  var rest = [];
  (rows || []).forEach(function(row) {
    if (pins.indexOf(realId('plan', row.id)) >= 0) {
      row.preview = 'Pinned - ' + (row.preview || 'Team Planner board');
      pinned.push(row);
    } else {
      rest.push(row);
    }
  });
  if (!pinned.length) {
    return rest;
  }
  return pinned.concat([{id: '__pin_divider', title: 'Pinned planners', preview: '', kind: 'divider'}], rest);
}

function mockChatRows(limit) {
  var rows = clone(mockChats).sort(function(a, b) {
    return (b.updated || 0) - (a.updated || 0);
  });
  var direct = rows.filter(function(row) {
    return row.kind === 'oneOnOne';
  });
  var groups = rows.filter(function(row) {
    return row.kind !== 'oneOnOne';
  });
  graphGroupChatRows = groups;
  return composeMainChatRows(direct, groups, limit || 20);
}

function mockRows(items, kind) {
  return clone(items || []).map(function(item) {
    return {
      id: tokenFor(kind, item.id),
      title: item.title,
      preview: item.preview || '',
      kind: item.kind || kind,
      unread: false,
      unread_count: 0
    };
  });
}

function mockChatRowsForId(chatToken) {
  return mockMessages[realId('chat', chatToken)] || [];
}

function base64Bytes(value) {
  var text = String(value || '');
  if (typeof atob === 'function') {
    var binary = atob(text);
    var bytes = new Array(binary.length);
    for (var i = 0; i < binary.length; i++) {
      bytes[i] = binary.charCodeAt(i) & 0xff;
    }
    return typeof Uint8Array !== 'undefined' ? new Uint8Array(bytes) : bytes;
  } else if (typeof Buffer !== 'undefined') {
    return Array.prototype.slice.call(Buffer.from(text, 'base64'));
  }
  var alphabet = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';
  var out = [];
  var buffer = 0;
  var bits = 0;
  for (var j = 0; j < text.length; j++) {
    var ch = text.charAt(j);
    if (ch === '=') {
      break;
    }
    var val = alphabet.indexOf(ch);
    if (val < 0) {
      continue;
    }
    buffer = (buffer << 6) | val;
    bits += 6;
    if (bits >= 8) {
      bits -= 8;
      out.push((buffer >> bits) & 0xff);
    }
  }
  return typeof Uint8Array !== 'undefined' ? new Uint8Array(out) : out;
}

function mockAvatarBytes(chatToken, maxBytes) {
  var chatId = realId('chat', chatToken);
  var hasOneOnOne = mockChats.some(function(chat) {
    return chat.id === chatId && chat.kind === 'oneOnOne';
  });
  if (!hasOneOnOne) {
    return Promise.reject(new Error('No mock avatar for non-1:1 chat'));
  }
  var png = base64Bytes('iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAMAAABF0y+mAAAABlBMVEUAAABiZKenFz5pAAAAAnRSTlMA/1uRIrUAAABESURBVHic3dIxCgAgDATB5P+fFiRo0HMV7bx2umTNzuYxIME+jCzrbF2VNSXUFkq4sqof4f2FHr7Cz+ZMODBOcxO1WgFbwQIdmIj33wAAAABJRU5ErkJggg==');
  if (maxBytes && png.length > maxBytes) {
    return Promise.reject(new Error('Mock avatar exceeds watch budget'));
  }
  return Promise.resolve(png);
}

function mockTaskDetail(taskToken) {
  var id = realId('task', taskToken);
  return clone(mockTaskDetails[id] || [
    {id: 'status', sender: 'Status', text: 'Not started', kind: 'status', outgoing: false, meta: 'Planner'},
    {id: 'notes', sender: 'Notes', text: 'No notes yet', kind: 'notes', outgoing: false},
    {id: 'checklist', sender: 'Checklist', text: 'No checklist items', kind: 'checklist', outgoing: false},
    {id: 'due', sender: 'Due', text: 'No due date', kind: 'due', outgoing: false}
  ]);
}

function checklistLines(text) {
  if (!text || text === 'No checklist items') {
    return [];
  }
  return String(text).split(/\r?\n|\|/).filter(function(line) {
    return !!line;
  });
}

function makeChecklistLine(title) {
  return '[ ] ' + String(title || '').trim();
}

function mockTaskRow(task) {
  return {
    id: task.id,
    title: task.title || 'Untitled card',
    preview: task.preview || ((task.due ? 'Due ' + task.due + ' - ' : '') +
             (task.percentComplete >= 100 ? 'Complete' : (task.percentComplete || 0) + '%')),
    kind: 'task',
    unread: false,
    unread_count: 0
  };
}

function mockCreateTask(bucketToken, title) {
  var bucketId = realId('bucket', bucketToken);
  var rows = mockTasks[bucketId] || (mockTasks[bucketId] = []);
  var id = 'mock-task-' + Date.now();
  var task = {id: id, title: String(title || 'New card').trim() || 'New card', preview: 'New card - 0%', due: '', percentComplete: 0, kind: 'task'};
  rows.unshift(task);
  mockTaskDetails[id] = [
    {id: 'status', sender: 'Status', text: 'Not started', kind: 'status', outgoing: false, meta: 'Planner'},
    {id: 'notes', sender: 'Notes', text: 'No notes yet', kind: 'notes', outgoing: false},
    {id: 'checklist', sender: 'Checklist', text: 'No checklist items', kind: 'checklist', outgoing: false},
    {id: 'due', sender: 'Due', text: 'No due date', kind: 'due', outgoing: false}
  ];
  return Promise.resolve(mockTaskRow(task));
}

function mockCreateBucket(planToken, name) {
  var planId = realId('plan', planToken);
  var rows = mockBuckets[planId] || (mockBuckets[planId] = []);
  var id = 'mock-bucket-' + Date.now();
  var bucket = {
    id: id,
    title: String(name || 'New bucket').trim() || 'New bucket',
    preview: '0 cards in this bucket',
    kind: 'bucket'
  };
  rows.push(bucket);
  mockTasks[id] = [];
  return Promise.resolve(bucket);
}

function mockUpdateDetailRow(taskToken, kind, updater) {
  var id = realId('task', taskToken);
  var rows = mockTaskDetails[id] || mockTaskDetail(taskToken);
  var found = false;
  rows.forEach(function(row) {
    if (row.kind === kind || row.sender.toLowerCase() === kind) {
      updater(row);
      found = true;
    }
  });
  if (!found) {
    var row = {id: kind, sender: kind.charAt(0).toUpperCase() + kind.slice(1), text: '', kind: kind, outgoing: false};
    updater(row);
    rows.push(row);
  }
  mockTaskDetails[id] = rows;
  return Promise.resolve();
}

function findMockTask(taskToken) {
  var realTask = realId('task', taskToken);
  var found = null;
  Object.keys(mockTasks).forEach(function(bucket) {
    mockTasks[bucket].forEach(function(task) {
      if (task.id === realTask) {
        found = task;
      }
    });
  });
  return found;
}

function findMockTaskBucket(taskToken) {
  var realTask = realId('task', taskToken);
  var found = null;
  Object.keys(mockTasks).forEach(function(bucket) {
    mockTasks[bucket].forEach(function(task) {
      if (task.id === realTask) {
        found = bucket;
      }
    });
  });
  return found;
}

function removeMockTask(taskToken) {
  var realTask = realId('task', taskToken);
  Object.keys(mockTasks).forEach(function(bucket) {
    mockTasks[bucket] = mockTasks[bucket].filter(function(task) {
      return task.id !== realTask;
    });
  });
  delete mockTaskDetails[realTask];
  return Promise.resolve();
}

function updateMockTaskPreview(taskToken, preview) {
  var task = findMockTask(taskToken);
  if (task) {
    task.preview = preview;
  }
}

function setMockDetailText(taskToken, kind, sender, text) {
  return mockUpdateDetailRow(taskToken, kind, function(row) {
    row.sender = sender || row.sender;
    row.text = String(text || '').trim() || row.text;
  });
}

function mockPlannerWrite(taskToken, command, text, messageId) {
  var value = String(text || '').trim();
  var realTask = realId('task', taskToken);
  var rows = mockTaskDetails[realTask] || mockTaskDetail(taskToken);
  var task = findMockTask(taskToken);
  var index = parseInt(messageId, 10);

  if (command === 'edit_task_element') {
    rows.forEach(function(row) {
      if (row.id === messageId && value) {
        row.text = value;
      }
    });
    mockTaskDetails[realTask] = rows;
    return Promise.resolve();
  }
  if (command === 'delete_task_element') {
    if (messageId === 'notes') {
      return mockUpdateDetailRow(taskToken, 'notes', function(row) {
        row.sender = 'Notes';
        row.text = 'No notes yet';
      });
    }
    mockTaskDetails[realTask] = rows.filter(function(row) {
      return row.id !== messageId || row.id === 'status';
    });
    return Promise.resolve();
  }
  if (command === 'edit_checklist') {
    return mockUpdateDetailRow(taskToken, 'checklist', function(row) {
      row.text = value ? makeChecklistLine(value) : 'No checklist items';
    });
  }
  if (command === 'delete_checklist') {
    return mockUpdateDetailRow(taskToken, 'checklist', function(row) {
      row.text = 'No checklist items';
    });
  }
  if (command === 'edit_checklist_item') {
    return mockUpdateDetailRow(taskToken, 'checklist', function(row) {
      var lines = checklistLines(row.text);
      if (lines[index] && value) {
        var checked = lines[index].indexOf('[x]') === 0 || lines[index].indexOf('[X]') === 0;
        lines[index] = (checked ? '[x] ' : '[ ] ') + value;
        row.text = lines.length ? lines.join('\n') : 'No checklist items';
      }
    });
  }
  if (command === 'delete_checklist_item') {
    return mockUpdateDetailRow(taskToken, 'checklist', function(row) {
      var lines = checklistLines(row.text);
      if (lines[index]) {
        lines.splice(index, 1);
      }
      row.text = lines.length ? lines.join('\n') : 'No checklist items';
    });
  }
  if (command === 'assign_task') {
    return setMockDetailText(taskToken, 'assigned', 'Assigned', value || 'Unassigned');
  }
  if (command === 'set_task_start') {
    return setMockDetailText(taskToken, 'start', 'Start', value || 'No start date');
  }
  if (command === 'set_task_priority') {
    return setMockDetailText(taskToken, 'priority', 'Priority', value || 'Normal');
  }
  if (command === 'set_task_progress') {
    var percent = /complete|done|100/i.test(value) ? 100 : (/progress|50|half/i.test(value) ? 50 : 0);
    if (task) {
      task.percentComplete = percent;
      task.preview = percent >= 100 ? 'Complete' : percent + '% complete';
    }
    return mockUpdateDetailRow(taskToken, 'status', function(row) {
      row.text = percent >= 100 ? 'Complete' : (percent + '% complete');
    });
  }
  if (command === 'move_task_bucket') {
    var targetBucket = realId('bucket', messageId);
    var sourceBucket = findMockTaskBucket(taskToken);
    if (task && sourceBucket && targetBucket && mockTasks[targetBucket] && targetBucket !== sourceBucket) {
      mockTasks[sourceBucket] = (mockTasks[sourceBucket] || []).filter(function(row) {
        return row.id !== realTask;
      });
      mockTasks[targetBucket].unshift(task);
    }
    updateMockTaskPreview(taskToken, 'Bucket: ' + (value || 'unchanged'));
    return setMockDetailText(taskToken, 'bucket', 'Bucket', value || 'Current bucket');
  }
  if (command === 'add_task_label') {
    return mockUpdateDetailRow(taskToken, 'labels', function(row) {
      row.sender = 'Labels';
      row.text = row.text ? row.text + ', ' + value : value;
    });
  }
  return Promise.resolve();
}

function parseDueDateText(text) {
  var value = String(text || '').trim();
  var lower = value.toLowerCase();
  var d = new Date();
  if (!value || lower === 'none' || lower === 'clear') {
    return null;
  }
  if (lower === 'today') {
    // Keep today's local date.
  } else if (lower === 'tomorrow') {
    d.setDate(d.getDate() + 1);
  } else if (/^\d{4}-\d{2}-\d{2}$/.test(value)) {
    d = new Date(value + 'T17:00:00');
  } else {
    d = new Date(value);
  }
  if (isNaN(d.getTime())) {
    throw new Error('Use today, tomorrow, or YYYY-MM-DD');
  }
  d.setHours(17, 0, 0, 0);
  return d.toISOString();
}

function graphTaskDetails(taskToken, status) {
  return graph('/planner/tasks/' + encodeURIComponent(realId('task', taskToken)) + '/details', {
    status: status
  }).then(function(details) {
    cacheTaskDetails(taskToken, details);
    return details;
  });
}

function graphChatRowsFromChats(chats, status) {
  chats = chats || [];
  return graphMe(status).then(function(me) {
    return Promise.all(chats.map(function(chat) {
      if (chat.chatType !== 'oneOnOne') {
        return enrichBlankChatPreview(chat, graphChatRow(chat), status);
      }
      return graph('/me/chats/' + encodeURIComponent(chat.id) + '/members', {status: status}).then(function(memberData) {
        var peer = null;
        (memberData.value || []).forEach(function(member) {
          var userId = member.userId || member.id || member.email || member.mail || member.userPrincipalName;
          if (!peer && userId && userId !== me.id) {
            peer = {
              id: userId,
              displayName: member.displayName || member.email || member.userPrincipalName || '1:1 Chat',
              mail: member.email || member.mail || member.userPrincipalName || ''
            };
          }
        });
        return enrichBlankChatPreview(chat, graphChatRow(chat, peer), status);
      }).catch(function() {
        return enrichBlankChatPreview(chat, graphChatRow(chat), status);
      });
    }));
  });
}

function graphChats(limit, status) {
  var top = Math.max(1, Math.min(50, limit || 19));
  return graph('/me/chats?$expand=lastMessagePreview&$orderby=lastMessagePreview/createdDateTime desc&$top=' + top, {
    status: status
  }).then(function(data) {
    var chats = data.value || [];
    return graphChatRowsFromChats(chats, status).then(function(rows) {
      var direct = [];
      var groups = [];
      rows.forEach(function(row) {
        if (row.kind === 'oneOnOne') {
          direct.push(row);
        } else {
          groups.push(row);
        }
      });
      graphGroupChatRows = groups;
      return composeMainChatRows(direct, groups, limit || 20);
    });
  });
}

function graphGroupChats(limit, status) {
  return graph('/me/chats?$expand=lastMessagePreview&$orderby=lastMessagePreview/createdDateTime desc&$top=50', {
    status: status
  }).then(function(data) {
    var groups = (data.value || []).filter(function(chat) {
      return chat.chatType !== 'oneOnOne';
    });
    return graphChatRowsFromChats(groups, status);
  }).then(function(rows) {
    graphGroupChatRows = rows || [];
    return graphGroupChatRows.slice(0, limit || 20);
  });
}

function userPhotoBytes(userId, status) {
  return graphBinary('/users/' + encodeURIComponent(userId) + '/photo/$value', {
    status: status
  });
}

function peerPhotoBytes(chatToken, peer, status) {
  var tried = {};
  function tryId(id) {
    if (!id || tried[id]) {
      return Promise.reject(new Error('No photo id'));
    }
    tried[id] = true;
    return userPhotoBytes(id, status);
  }
  return tryId(peer && peer.id).catch(function(firstErr) {
    return tryId(peer && peer.mail).catch(function() {
      return graphMe(status).then(function(me) {
        return graph('/me/chats/' + encodeURIComponent(realId('chat', chatToken)) + '/members', {
          status: status
        }).then(function(memberData) {
          var candidates = [];
          (memberData.value || []).forEach(function(member) {
            var ids = [
              member.userId,
              member.id,
              member.email,
              member.mail,
              member.userPrincipalName
            ];
            ids.forEach(function(id) {
              if (id && id !== me.id && candidates.indexOf(id) < 0) {
                candidates.push(id);
              }
            });
          });
          function next(index) {
            if (index >= candidates.length) {
              throw firstErr;
            }
            return tryId(candidates[index]).catch(function() {
              return next(index + 1);
            });
          }
          return next(0);
        });
      });
    });
  });
}

function graphMessagesPage(chatToken, limit, anchorToken, direction, status) {
  var chatId = encodeURIComponent(realId('chat', chatToken));
  var wanted = Math.max(1, Math.min(50, limit || 20));
  var pageTop = Math.max(wanted, 20);
  var path = '/chats/' + chatId + '/messages?$top=' + pageTop + '&$orderby=createdDateTime desc';
  var anchorId = anchorToken ? realMessageId(chatToken, anchorToken) : '';
  var rows = [];
  var rawById = {};
  var seenAnchor = !anchorId;
  var pages = 0;

  function fillReplyFallbacks() {
    return Promise.all(rows.map(function(row) {
      if (!row._replyToId) {
        return Promise.resolve();
      }
      if (rawById[row._replyToId]) {
        applyReplyFallback(row, rawById[row._replyToId]);
        delete row._replyToId;
        return Promise.resolve();
      }
      return graph('/chats/' + chatId + '/messages/' + encodeURIComponent(row._replyToId), {
        status: status
      }).then(function(replyMessage) {
        applyReplyFallback(row, replyMessage);
        delete row._replyToId;
      }).catch(function() {
        delete row._replyToId;
      });
    }));
  }

  return graphMe(status).then(function() {
    function readPage(nextPath) {
      return graph(nextPath, {status: status}).then(function(data) {
        pages += 1;
        var values = data.value || [];
        for (var i = 0; i < values.length; i += 1) {
          var message = values[i];
          if (message && message.id) {
            rawById[message.id] = message;
          }
          if (anchorId && message.id === anchorId) {
            seenAnchor = true;
            if (direction === 'newer') {
              break;
            }
            continue;
          }
          if (!anchorId || (direction === 'newer' && !seenAnchor) ||
              (direction === 'older' && seenAnchor)) {
            rows.push(graphMessageRow(message, chatToken));
            if (rows.length >= wanted) {
              break;
            }
          }
        }
        if (rows.length >= wanted || (direction === 'newer' && seenAnchor) ||
            !data['@odata.nextLink'] || pages >= 5) {
          return fillReplyFallbacks().then(function() {
            return rows.reverse();
          });
        }
        return readPage(data['@odata.nextLink']);
      });
    }
    return readPage(path);
  });
}

function graphMessages(chatToken, limit, beforeToken, status) {
  return graphMessagesPage(chatToken, limit, beforeToken, beforeToken ? 'older' : 'initial', status);
}

function reactionActorId(reaction) {
  var user = reaction && reaction.user;
  return (user && user.user && user.user.id) || (user && user.id) || '';
}

function graphCurrentReactionType(chatToken, messageToken, status) {
  return graphMe(status).then(function(me) {
    return graph('/chats/' + encodeURIComponent(realId('chat', chatToken)) + '/messages/' +
      encodeURIComponent(realMessageId(chatToken, messageToken)), {status: status}).then(function(message) {
      var reactions = message.reactions || [];
      for (var i = 0; i < reactions.length; i += 1) {
        if (reactionActorId(reactions[i]) === me.id && reactions[i].reactionType) {
          return reactions[i].reactionType;
        }
      }
      return '';
    });
  });
}

function graphUnsetReaction(chatToken, messageToken, status) {
  return graphCurrentReactionType(chatToken, messageToken, status).then(function(reactionType) {
    if (!reactionType) {
      throw new Error('No reaction to remove');
    }
    return graph('/chats/' + encodeURIComponent(realId('chat', chatToken)) + '/messages/' +
      encodeURIComponent(realMessageId(chatToken, messageToken)) + '/unsetReaction', {
      method: 'POST',
      status: status,
      body: {reactionType: reactionType}
    });
  });
}

function firstImageAttachment(message) {
  var attachments = message && message.attachments;
  for (var i = 0; attachments && i < attachments.length; i += 1) {
    if (attachments[i].thumbnailUrl || attachmentLooksImage(attachments[i])) {
      return attachments[i];
    }
  }
  return null;
}

function graphAttachmentImageBytes(chatToken, messageToken, status) {
  return graph('/chats/' + encodeURIComponent(realId('chat', chatToken)) + '/messages/' +
    encodeURIComponent(realMessageId(chatToken, messageToken)), {status: status}).then(function(message) {
    var attachment = firstImageAttachment(message);
    var url = attachment && (attachment.thumbnailUrl || attachment.contentUrl);
    if (!url) {
      throw new Error(messageMediaLabel(message) || 'No image bytes');
    }
    return graphBinary(url, {status: status});
  });
}

function graphMessageImageBytes(chatToken, messageToken, status) {
  var chatId = encodeURIComponent(realId('chat', chatToken));
  var messageId = encodeURIComponent(realMessageId(chatToken, messageToken));
  return graph('/chats/' + chatId + '/messages/' + messageId + '/hostedContents', {
    status: status
  }).then(function(data) {
    var hosted = (data.value || [])[0];
    if (!hosted || !hosted.id) {
      return graphAttachmentImageBytes(chatToken, messageToken, status);
    }
    return graphBinary('/chats/' + chatId + '/messages/' + messageId + '/hostedContents/' +
      encodeURIComponent(hosted.id) + '/$value', {status: status});
  }).catch(function(err) {
    return graphAttachmentImageBytes(chatToken, messageToken, status).catch(function() {
      throw err;
    });
  });
}

function graphPlans(status) {
  return graph('/me/planner/plans', {status: status}).then(function(data) {
    var rows = (data.value || []).map(function(plan) {
      return {
        id: tokenFor('plan', plan.id),
        title: plan.title || 'Untitled plan',
        preview: 'Team Planner board',
        kind: 'plan',
        unread: false,
        unread_count: 0
      };
    });
    return applyPinnedPlanOrder(rows);
  });
}

function togglePlanPin(planToken) {
  var realPlan = realId('plan', planToken);
  var pins = pinnedPlanIds();
  var index = pins.indexOf(realPlan);
  if (index >= 0) {
    pins.splice(index, 1);
  } else {
    pins.unshift(realPlan);
  }
  savePinnedPlanIds(pins);
  return Promise.resolve(index >= 0 ? 'Planner unpinned' : 'Planner pinned');
}

function graphBuckets(planToken, status) {
  var planId = encodeURIComponent(realId('plan', planToken));
  return Promise.all([
    graph('/planner/plans/' + planId + '/details', {status: status}).catch(function() {
      return {};
    }),
    graph('/planner/plans/' + planId + '/buckets', {status: status})
  ]).then(function(results) {
    var details = results[0] || {};
    var data = results[1] || {};
    planLabelCache[realId('plan', planToken)] = details.categoryDescriptions || {};
    return (data.value || []).slice().reverse().map(function(bucket) {
      var token = tokenFor('bucket', bucket.id);
      bucketPlanMap[token] = realId('plan', planToken);
      bucketNameMap[bucket.id] = bucket.name || 'Untitled bucket';
      return {
        id: token,
        title: bucket.name || 'Untitled bucket',
        preview: 'Cards in this bucket',
        kind: 'bucket',
        unread: false,
        unread_count: 0
      };
    });
  });
}

function graphCreateBucket(planToken, name, status) {
  var planId = realId('plan', planToken);
  return graph('/planner/buckets', {
    method: 'POST',
    status: status,
    body: {
      name: String(name || 'New bucket').trim() || 'New bucket',
      planId: planId,
      orderHint: ' !'
    }
  }).then(function(bucket) {
    if (bucket && bucket.id) {
      var token = tokenFor('bucket', bucket.id);
      bucketPlanMap[token] = planId;
      bucketNameMap[bucket.id] = bucket.name || String(name || 'New bucket').trim() || 'New bucket';
    }
    return bucket;
  });
}

function graphTask(taskToken, status) {
  if (taskCache[taskToken]) {
    return Promise.resolve(taskCache[taskToken]);
  }
  return graph('/planner/tasks/' + encodeURIComponent(realId('task', taskToken)), {
    status: status
  }).then(function(task) {
    taskCache[taskToken] = task;
    return task;
  });
}

function graphPatchTask(taskToken, body, status) {
  return graphTask(taskToken, status).then(function(task) {
    return graph('/planner/tasks/' + encodeURIComponent(realId('task', taskToken)), {
      method: 'PATCH',
      status: status,
      headers: {
        'If-Match': task['@odata.etag'] || task.etag || '*',
        Prefer: 'return=representation'
      },
      body: body
    }).then(function(updated) {
      if (updated && updated.id) {
        taskCache[taskToken] = updated;
      } else {
        Object.keys(body || {}).forEach(function(key) {
          task[key] = body[key];
        });
        taskCache[taskToken] = task;
      }
      return taskCache[taskToken];
    });
  });
}

function cacheTaskDetails(taskToken, details) {
  var checklist = details && details.checklist || {};
  var cached = {etag: details && (details['@odata.etag'] || details.etag) || '*', ids: [], checked: [], titles: []};
  Object.keys(checklist).forEach(function(id) {
    var item = checklist[id];
    if (item && item.title) {
      cached.ids.push(id);
      cached.checked.push(!!item.isChecked);
      cached.titles.push(item.title);
    }
  });
  taskChecklistCache[taskToken] = cached;
  return details;
}

function graphPatchTaskDetails(taskToken, body, status) {
  return graphTaskDetails(taskToken, status).then(function(details) {
    return graph('/planner/tasks/' + encodeURIComponent(realId('task', taskToken)) + '/details', {
      method: 'PATCH',
      status: status,
      headers: {
        'If-Match': details['@odata.etag'] || details.etag || '*',
        Prefer: 'return=representation'
      },
      body: body
    }).then(function(updated) {
      if (updated && updated.id) {
        cacheTaskDetails(taskToken, updated);
      } else if (body && body.checklist) {
        delete taskChecklistCache[taskToken];
      }
      return updated;
    });
  });
}

function priorityLabel(priority) {
  if (priority <= 1) {
    return 'Urgent';
  }
  if (priority <= 3) {
    return 'Important';
  }
  if (priority >= 8) {
    return 'Low';
  }
  return 'Medium';
}

function labelTitle(planId, key) {
  var labels = planLabelCache[planId] || {};
  var match = String(key || '').match(/^category([1-6])$/);
  return labels[key] || (match ? plannerLabelName(Number(match[1]) - 1) : key.replace('category', 'Category '));
}

function labelKeyForTask(taskToken, text, messageId) {
  if (/^category[1-6]$/.test(String(messageId || ''))) {
    return String(messageId);
  }
  var task = taskCache[taskToken] || {};
  var labels = planLabelCache[task.planId] || {};
  var value = String(text || '').toLowerCase();
  var found = null;
  Object.keys(labels).forEach(function(key) {
    if (!found && String(labels[key] || '').toLowerCase() === value) {
      found = key;
    }
  });
  plannerLabelsPacked().split('|').forEach(function(label, index) {
    if (!found && String(label || '').toLowerCase() === value) {
      found = 'category' + (index + 1);
    }
  });
  return found || 'category1';
}

function graphTasks(bucketToken, status) {
  var bucketId = realId('bucket', bucketToken);
  return graph('/planner/buckets/' + encodeURIComponent(bucketId) + '/tasks', {status: status}).then(function(data) {
    var rows = [];
    (data.value || []).forEach(function(task) {
      var token = tokenFor('task', task.id);
      taskCache[token] = task;
      rows.push({
        id: token,
        title: task.title || 'Untitled task',
        preview: (task.dueDateTime ? 'Due ' + shortDate(task.dueDateTime) + ' - ' : '') +
                 (task.percentComplete >= 100 ? 'Complete' : (task.percentComplete || 0) + '%'),
        kind: 'task',
        unread: false,
        unread_count: 0
      });
    });
    bucketTaskCache[bucketToken] = rows;
    return rows;
  });
}

function graphTaskDetail(taskToken, status) {
  return graphTask(taskToken, status).then(function(task) {
    var rows = [
      {id: 'status', sender: 'Status', text: task.percentComplete >= 100 ? 'Complete' : ((task.percentComplete || 0) + '% complete'), kind: 'status', outgoing: false, meta: 'Planner'}
    ];
    return graph('/planner/tasks/' + encodeURIComponent(realId('task', taskToken)) + '/details', {status: status}).then(function(details) {
    var description = details.description || '';
    var checklist = details.checklist || {};
    var lines = [];
    cacheTaskDetails(taskToken, details);
    Object.keys(checklist).forEach(function(id) {
      var item = checklist[id];
      if (item && item.title) {
        lines.push((item.isChecked ? '[x] ' : '[ ] ') + item.title);
      }
    });
    if (task.startDateTime) {
      rows.push({id: 'start', sender: 'Start', text: shortDate(task.startDateTime), kind: 'start', outgoing: false});
    }
    if (typeof task.priority === 'number') {
      rows.push({id: 'priority', sender: 'Priority', text: priorityLabel(task.priority), kind: 'priority', outgoing: false});
    }
    if (task.bucketId) {
      rows.push({id: 'bucket', sender: 'Bucket', text: bucketNameMap[task.bucketId] || 'Current bucket', kind: 'bucket', outgoing: false});
    }
    var assigneeIds = Object.keys(task.assignments || {});
    if (assigneeIds.length) {
      return Promise.all(assigneeIds.map(function(userId) {
        return graph('/users/' + encodeURIComponent(userId) + '?$select=id,displayName,mail,userPrincipalName', {
          status: status
        }).catch(function() {
          return {id: userId, displayName: 'Assigned user'};
        });
      })).then(function(users) {
        taskAssignmentCache[taskToken] = {
          ids: users.map(function(user) { return user.id; }),
          names: users.map(function(user) { return user.displayName || user.mail || user.userPrincipalName || 'Assigned user'; })
        };
        rows.push({
          id: 'assigned',
          sender: 'Assigned',
          text: taskAssignmentCache[taskToken].names.join('|'),
          kind: 'assigned',
          outgoing: false
        });
        return finishTaskDetailRows(taskToken, task, rows, description, lines);
      });
    }
    delete taskAssignmentCache[taskToken];
    return finishTaskDetailRows(taskToken, task, rows, description, lines);
    });
  });
}

function finishTaskDetailRows(taskToken, task, rows, description, lines) {
  var applied = task.appliedCategories || {};
  var labelNames = Object.keys(applied).filter(function(key) {
    return applied[key];
  }).map(function(key) {
    return labelTitle(task.planId, key);
  });
  if (labelNames.length) {
    rows.push({id: 'labels', sender: 'Labels', text: labelNames.join(', '), kind: 'labels', outgoing: false});
  }
  rows.push({id: 'notes', sender: 'Notes', text: description || 'No notes yet', kind: 'notes', outgoing: false});
  rows.push({id: 'checklist', sender: 'Checklist', text: lines.length ? lines.join('\n') : 'No checklist items', kind: 'checklist', outgoing: false});
  rows.push({id: 'due', sender: 'Due', text: task.dueDateTime ? shortDate(task.dueDateTime) : 'No due date', kind: 'due', outgoing: false});
  return rows;
}

function toggleMockChecklist(taskToken, itemIndex) {
  var rows = mockTaskDetail(taskToken);
  var id = realId('task', taskToken);
  var source = mockTaskDetails[id] || rows;
  source.forEach(function(row) {
    if ((row.kind === 'checklist' || row.sender === 'Checklist') && row.text) {
      var lines = checklistLines(row.text);
      if (lines[itemIndex]) {
        if (lines[itemIndex].indexOf('[ ]') === 0) {
          lines[itemIndex] = '[x]' + lines[itemIndex].substring(3);
        } else if (lines[itemIndex].indexOf('[x]') === 0 || lines[itemIndex].indexOf('[X]') === 0) {
          lines[itemIndex] = '[ ]' + lines[itemIndex].substring(3);
        }
        row.text = lines.join('\n');
      }
    }
  });
  mockTaskDetails[id] = source;
  return Promise.resolve();
}

function graphToggleChecklist(taskToken, itemIndex, status) {
  var cached = taskChecklistCache[taskToken];
  if (!cached || !cached.ids || !cached.ids[itemIndex]) {
    return graphTaskDetail(taskToken, status).then(function() {
      return graphToggleChecklist(taskToken, itemIndex, status);
    });
  }
  var checklistId = cached.ids[itemIndex];
  var update = {};
  update[checklistId] = {isChecked: !cached.checked[itemIndex]};
  return graphPatchTaskDetails(taskToken, {checklist: update}, status).then(function() {
    if (taskChecklistCache[taskToken] && taskChecklistCache[taskToken].checked) {
      taskChecklistCache[taskToken].checked[itemIndex] = !cached.checked[itemIndex];
    }
  });
}

function graphCreateTask(bucketToken, title, status) {
  var bucketId = realId('bucket', bucketToken);
  var planId = bucketPlanMap[bucketToken];
  if (!planId) {
    return Promise.reject(new Error('Bucket plan not cached'));
  }
  return graph('/planner/tasks', {
    method: 'POST',
    status: status,
    body: {
      planId: planId,
      bucketId: bucketId,
      title: String(title || 'New card').trim() || 'New card'
    }
  }).then(function(task) {
    if (task && task.id) {
      var token = tokenFor('task', task.id);
      taskCache[token] = task;
    }
    return task;
  });
}

function graphAddTaskNote(taskToken, text, status) {
  return graphTaskDetails(taskToken, status).then(function(details) {
    var existing = details.description && details.description !== 'No notes yet' ? details.description : '';
    var next = existing ? existing + '\n' + String(text || '') : String(text || '');
    return graphPatchTaskDetails(taskToken, {description: next}, status);
  });
}

function graphAddChecklistItem(taskToken, text, status) {
  var checklistId = pseudoGuid();
  var update = {};
  update[checklistId] = {
    '@odata.type': 'microsoft.graph.plannerChecklistItem',
    title: String(text || '').trim() || 'Checklist item',
    isChecked: false,
    orderHint: ' !'
  };
  return graphPatchTaskDetails(taskToken, {checklist: update}, status);
}

function graphSetTaskDue(taskToken, text, status) {
  var due = parseDueDateText(text);
  return graphPatchTask(taskToken, {dueDateTime: due}, status);
}

function graphSetTaskPercent(taskToken, percent, status) {
  return graphPatchTask(taskToken, {percentComplete: percent}, status);
}

function graphCompleteTask(taskToken, status) {
  return graphSetTaskPercent(taskToken, 100, status);
}

function graphUncompleteTask(taskToken, status) {
  return graphSetTaskPercent(taskToken, 0, status);
}

function graphSetTaskStart(taskToken, text, status) {
  return graphPatchTask(taskToken, {startDateTime: parseDueDateText(text)}, status);
}

function progressPercentFromText(text) {
  var value = String(text || '').trim();
  return /complete|done|100/i.test(value) ? 100 : (/progress|50|half/i.test(value) ? 50 : 0);
}

function priorityFromText(text) {
  var priorityText = String(text || '').toLowerCase();
  if (/urgent|critical|highest/.test(priorityText)) {
    return 1;
  }
  if (/important|high/.test(priorityText)) {
    return 3;
  }
  if (/low/.test(priorityText)) {
    return 9;
  }
  return 5;
}

function resolveUserForAssignment(text, status) {
  var value = String(text || '').trim();
  if (!value || /^me$/i.test(value)) {
    return graphMe(status);
  }
  var filter = [
    'startswith(displayName,' + odataString(value) + ')',
    'startswith(mail,' + odataString(value) + ')',
    'startswith(userPrincipalName,' + odataString(value) + ')'
  ].join(' or ');
  return graph('/users?$select=id,displayName,mail,userPrincipalName&$top=5&$filter=' +
               encodeURIComponent(filter), {
    status: status
  }).then(function(data) {
    var rows = data.value || [];
    if (!rows.length) {
      throw new Error('No matching user for assignment');
    }
    return rows[0];
  });
}

function graphAssignTask(taskToken, contactToken, fallbackText, status) {
  var peer = chatPeerCache[contactToken] || null;
  var assignmentUserId = realId('assignUser', contactToken);
  var configuredLookup = assignmentUserId && assignmentUserId.indexOf('configured:') === 0 ?
    assignmentUserId.slice('configured:'.length) : '';
  var lookup = peer && peer.id ? '' : (configuredLookup || fallbackText || contactToken);
  var userPromise = peer && peer.id ? Promise.resolve(peer) :
    (assignmentUserId && assignmentUserId !== String(contactToken || '') && !configuredLookup ?
      Promise.resolve({id: assignmentUserId, displayName: fallbackText || 'Assigned user'}) :
      resolveUserForAssignment(lookup, status));
  return userPromise.then(function(user) {
    var assignments = {};
    assignments[user.id] = {
      '@odata.type': '#microsoft.graph.plannerAssignment',
      orderHint: ' !'
    };
    return graphPatchTask(taskToken, {assignments: assignments}, status);
  });
}

function graphAssignTeams(status) {
  return graph('/me/joinedTeams?$select=id,displayName,description&$top=12', {
    status: status
  }).then(function(data) {
    return (data.value || []).filter(function(team) {
      return team && team.id && team.displayName;
    }).map(function(team) {
      return {
        id: tokenFor('assignTeam', team.id),
        title: team.displayName,
        preview: team.description || 'Team members',
        kind: 'assignTeam'
      };
    });
  });
}

function graphAssignTeamMembers(teamToken, status) {
  var teamId = realId('assignTeam', teamToken);
  return graph('/teams/' + encodeURIComponent(teamId) +
               '/members/microsoft.graph.aadUserConversationMember?$select=id,displayName,email,userId,roles&$top=25', {
    status: status
  }).then(function(data) {
    return (data.value || []).filter(function(member) {
      return member && member.userId && member.displayName;
    }).map(function(member) {
      return {
        id: tokenFor('assignUser', member.userId),
        title: member.displayName,
        preview: member.email || 'Team member',
        kind: 'assignMember'
      };
    });
  });
}

function graphClearAssignments(taskToken, status) {
  return graphTask(taskToken, status).then(function(task) {
    var assignments = {};
    Object.keys(task.assignments || {}).forEach(function(userId) {
      assignments[userId] = null;
    });
    if (!Object.keys(assignments).length) {
      return Promise.resolve();
    }
    return graphPatchTask(taskToken, {assignments: assignments}, status);
  });
}

function graphRemoveAssignment(taskToken, indexText, status) {
  var cached = taskAssignmentCache[taskToken];
  var index = parseInt(indexText, 10);
  if (!cached || !cached.ids || !cached.ids[index]) {
    return graphTaskDetail(taskToken, status).then(function() {
      return graphRemoveAssignment(taskToken, indexText, status);
    });
  }
  var assignments = {};
  assignments[cached.ids[index]] = null;
  return graphPatchTask(taskToken, {assignments: assignments}, status);
}

function graphClearLabels(taskToken, status) {
  return graphTask(taskToken, status).then(function(task) {
    var categories = {};
    Object.keys(task.appliedCategories || {}).forEach(function(key) {
      if (task.appliedCategories[key]) {
        categories[key] = false;
      }
    });
    if (!Object.keys(categories).length) {
      return Promise.resolve();
    }
    return graphPatchTask(taskToken, {appliedCategories: categories}, status);
  });
}

function graphDeleteTask(taskToken, status) {
  return graphTask(taskToken, status).then(function(task) {
    return graph('/planner/tasks/' + encodeURIComponent(realId('task', taskToken)), {
      method: 'DELETE',
      status: status,
      headers: {'If-Match': task['@odata.etag'] || task.etag || '*'}
    }).then(function() {
      delete taskCache[taskToken];
      delete taskChecklistCache[taskToken];
    });
  });
}

function graphPlannerWrite(taskToken, command, text, messageId, status) {
  if (command === 'set_task_start') {
    return graphSetTaskStart(taskToken, text, status);
  }
  if (command === 'set_task_progress') {
    return graphPatchTask(taskToken, {percentComplete: progressPercentFromText(text)}, status);
  }
  if (command === 'set_task_priority') {
    return graphPatchTask(taskToken, {priority: priorityFromText(text)}, status);
  }
  if (command === 'move_task_bucket') {
    var bucketId = realId('bucket', messageId);
    if (!bucketId || bucketId === 'undefined') {
      return Promise.reject(new Error('No bucket selected'));
    }
    return graphPatchTask(taskToken, {bucketId: bucketId}, status);
  }
  if (command === 'add_task_label') {
    var labelKey = labelKeyForTask(taskToken, text, messageId);
    var categories = {};
    categories[labelKey] = true;
    return graphPatchTask(taskToken, {appliedCategories: categories}, status);
  }
  if (command === 'assign_task') {
    return graphAssignTask(taskToken, messageId || text, text, status);
  }
  if (command === 'unassign_task') {
    return graphRemoveAssignment(taskToken, messageId, status);
  }
  if (command === 'edit_task_element' && messageId === 'notes') {
    return graphPatchTaskDetails(taskToken, {description: String(text || '')}, status);
  }
  if (command === 'delete_task_element' && messageId === 'notes') {
    return graphPatchTaskDetails(taskToken, {description: ''}, status);
  }
  if (command === 'edit_task_element') {
    if (messageId === 'due') {
      return graphSetTaskDue(taskToken, text, status);
    }
    if (messageId === 'start') {
      return graphSetTaskStart(taskToken, text, status);
    }
    if (messageId === 'priority') {
      return graphPatchTask(taskToken, {priority: priorityFromText(text)}, status);
    }
    if (messageId === 'status') {
      return graphPatchTask(taskToken, {percentComplete: progressPercentFromText(text)}, status);
    }
    if (messageId === 'labels') {
      return graphPlannerWrite(taskToken, 'add_task_label', text, null, status);
    }
    if (messageId === 'assigned') {
      return graphAssignTask(taskToken, text, text, status);
    }
  }
  if (command === 'delete_task_element') {
    if (messageId === 'due') {
      return graphPatchTask(taskToken, {dueDateTime: null}, status);
    }
    if (messageId === 'start') {
      return graphPatchTask(taskToken, {startDateTime: null}, status);
    }
    if (messageId === 'priority') {
      return graphPatchTask(taskToken, {priority: 5}, status);
    }
    if (messageId === 'status') {
      return graphPatchTask(taskToken, {percentComplete: 0}, status);
    }
    if (messageId === 'labels') {
      return graphClearLabels(taskToken, status);
    }
    if (messageId === 'assigned') {
      return graphClearAssignments(taskToken, status);
    }
    if (messageId === 'bucket') {
      return Promise.reject(new Error('Planner tasks must stay in a bucket'));
    }
  }
  if (command === 'edit_checklist_item' || command === 'delete_checklist_item') {
    var cached = taskChecklistCache[taskToken];
    var itemIndex = parseInt(messageId, 10);
    if (!cached || !cached.ids || !cached.ids[itemIndex]) {
      return graphTaskDetail(taskToken, status).then(function() {
        return graphPlannerWrite(taskToken, command, text, messageId, status);
      });
    }
    var checklistId = cached.ids[itemIndex];
    var update = {};
    if (command === 'delete_checklist_item') {
      update[checklistId] = null;
    } else {
      update[checklistId] = {
        '@odata.type': 'microsoft.graph.plannerChecklistItem',
        title: String(text || '').trim() || cached.titles[itemIndex] || 'Checklist item',
        isChecked: !!cached.checked[itemIndex]
      };
    }
    return graphPatchTaskDetails(taskToken, {checklist: update}, status);
  }
  if (command === 'edit_checklist') {
    return graphTaskDetail(taskToken, status).then(function() {
      var cached = taskChecklistCache[taskToken] || {};
      var update = {};
      (cached.ids || []).forEach(function(id) {
        update[id] = null;
      });
      if (String(text || '').trim()) {
        update[pseudoGuid()] = {
          '@odata.type': 'microsoft.graph.plannerChecklistItem',
          title: String(text || '').trim(),
          isChecked: false,
          orderHint: ' !'
        };
      }
      return graphPatchTaskDetails(taskToken, {checklist: update}, status);
    });
  }
  if (command === 'delete_checklist') {
    return graphTaskDetail(taskToken, status).then(function() {
      var cached = taskChecklistCache[taskToken] || {};
      var update = {};
      (cached.ids || []).forEach(function(id) {
        update[id] = null;
      });
      return graphPatchTaskDetails(taskToken, {checklist: update}, status);
    });
  }
  return Promise.reject(new Error('This Planner action is mock-wired only for now: ' + command));
}

function inlineSettingsUrl(options, graphToken) {
  var tenantId = htmlEscape(authTenantId());
  var clientId = htmlEscape(authClientId());
  var scopes = htmlEscape(authScopes());
  var replies = htmlEscape(options.cannedReplies ? options.cannedReplies() : '');
  var labelJson = JSON.stringify(plannerLabelsPacked().split('|'));
  var assignJson = JSON.stringify(configuredAssignContacts().map(assignContactLine));
  var tokenLiteral = jsLiteral(graphToken || '');
  var rawCode = setting('deviceUserCode', '');
  var rawCodeExpires = Number(setting('deviceCodeExpiresAt', '0'));
  if (rawCode && rawCodeExpires && Date.now() >= rawCodeExpires) {
    rawCode = '';
    removeSetting('deviceUserCode');
    removeSetting('deviceVerificationUri');
    removeSetting('deviceCodeExpiresAt');
  }
  var code = htmlEscape(rawCode);
  var uri = htmlEscape(setting('deviceVerificationUri', 'https://microsoft.com/devicelogin'));
  var hasClient = graphConfigured();
  var mockChecked = setting('mockMode', '') === '1' || !hasClient ? ' checked' : '';
  var connectDisabled = hasClient ? '' : ' disabled';
  var signInLabel = code ? 'Refresh sign-in code' : 'Sign in with Microsoft';
  var setupMessage = hasClient ?
    '<p class=h>Uses your Microsoft work or school account. Access is limited to what your account can already reach.</p>' :
    '<p class=h>This build needs a public-client Microsoft app ID before sign-in can start. Add it once in Advanced app setup, or bake it into the build.</p>';
  var html = '<html><head><meta name=viewport content=width=device-width><style>' +
    'body{font-family:sans-serif;margin:16px;line-height:1.35;color:#242424;background:#f7f5ff}h1{color:#464775;margin-bottom:4px}.p{border:1px solid #d8d5f0;border-radius:8px;padding:12px;background:#fff;margin:12px 0}input,button{box-sizing:border-box;width:100%;padding:11px;margin:6px 0 12px;border-radius:6px}input{border:1px solid #c8c6c4}button{background:#6264a7;color:white;border:0;font-weight:bold}button:disabled{background:#c8c6c4}.secondary{background:#fff;color:#464775;border:1px solid #6264a7}.danger{background:#fff;color:#a4262c;border:1px solid #a4262c}.h{color:#616161;font-size:13px}.c{font-size:36px;font-weight:bold;color:#464775;letter-spacing:2px;margin:8px 0}.url{font-size:16px;font-weight:bold;color:#464775;word-break:break-word}label{font-weight:bold;display:block;margin-top:10px}details{margin-top:10px}summary{color:#464775;font-weight:bold;cursor:pointer}.row{display:flex;gap:8px;margin-top:8px}.row input{margin:0;flex:1}.row button{width:46px;flex:0 0 46px;margin:0;padding:0}.pill{display:flex;gap:8px;align-items:center;border:1px solid #d8d5f0;border-radius:6px;padding:9px;margin-top:8px}.pill span{flex:1}.pill button{width:38px;flex:0 0 38px;margin:0;padding:0}.suggest{border:1px solid #d8d5f0;border-radius:6px;background:#fff;margin:-6px 0 10px;max-height:210px;overflow:auto}.suggest div{padding:9px;border-bottom:1px solid #eee}.suggest div:last-child{border-bottom:0}.suggest b{display:block}.savebar{position:sticky;bottom:0;background:#f7f5ff;padding-top:8px}.toggle{display:flex;gap:8px;align-items:center}.toggle input{width:auto;margin:0}' +
    '</style></head><body><h1>Pebble Teams</h1><div class=p><b>Microsoft sign-in</b>' + setupMessage +
    '<button id=signin' + connectDisabled + '>' + signInLabel + '</button><button id=mock class=secondary>Use mock data</button><button id=signout class=danger>Sign out</button></div>' +
    (code ? '<div class=p><b>Finish sign-in</b><p class=h>Go to this URL and enter the code below:</p><a id=loginlink class=url href="' + uri + '">' + uri + '</a><div class=c>' + code + '</div><button id=openlogin class=secondary type=button>Open Microsoft page</button><p class=h>After Microsoft accepts the code, this app will continue automatically. Use Refresh sign-in code if this code fails.</p></div>' : '') +
    '<details class=p><summary>Canned replies</summary><div id=replyList></div><button id=addReply class=secondary type=button>Add reply</button><input id=r type=hidden value="' + replies + '"><div class=h>These appear in the watch reply menu.</div></details>' +
    '<details class=p><summary>Planner labels</summary><div id=labelList></div><div class=h>The six rows map to Planner category 1 through 6.</div></details>' +
    '<details class=p open><summary>Planner assign list</summary><div id=assignList></div><input id=contactSearch placeholder="Search people in Microsoft Graph"><div id=suggest class=suggest style=display:none></div><button id=manualAssign class=secondary type=button>Add typed entry</button><div class=h>Search uses your logged-in Microsoft account when available. Entries feed the watch Assign menu below Me.</div></details>' +
    '<details class=p><summary>Advanced app setup</summary><div class=h>No client secret or app-only access. These are delegated user scopes only.</div>' +
    '<label>Tenant</label><input id=t value="' + tenantId + '"><div class=h>Default is organizations for work/school sign-in.</div>' +
    '<label>Public client app ID</label><input id=c value="' + clientId + '"><div class=h>Needed once unless the app ID is built into this PBW.</div>' +
    '<label>Delegated scopes</label><input id=s value="' + scopes + '"><div class=h>Default scopes avoid app-only tenant permissions.</div>' +
    '<label class=toggle><input id=m type=checkbox' + mockChecked + '> Use mock data</label></details><div class=savebar><button id=save>Save Settings</button></div>' +
    '<script>var hasCode=' + (code ? 'true' : 'false') + ',graphToken=' + tokenLiteral + ',labels=' + labelJson + ',assigns=' + assignJson + ';function vals(el){var a=[];var rows=el.querySelectorAll("input");for(var i=0;i<rows.length;i++){var v=rows[i].value.trim();if(v){a.push(v)}}return a}function rowList(el,items,max){el.innerHTML="";items=(items&&items.length?items:[""]);for(var i=0;i<items.length&&(!max||i<max);i++){var row=document.createElement("div");row.className="row";var input=document.createElement("input");input.value=items[i];var del=document.createElement("button");del.type="button";del.className="danger";del.textContent="X";del.onclick=function(r){return function(){r.parentNode.removeChild(r);if(!el.children.length){rowList(el,[""],max)}}}(row);row.appendChild(input);row.appendChild(del);el.appendChild(row)}}function addRow(el,max){var v=vals(el);if(!max||v.length<max){v.push("");rowList(el,v,max)}}function assignVals(){var a=[];var rows=assignList.querySelectorAll(".pill span");for(var i=0;i<rows.length;i++){a.push(rows[i].textContent)}return a}function renderAssign(items){assignList.innerHTML="";items=(items||[]).filter(Boolean);items.forEach(function(v){var row=document.createElement("div");row.className="pill";var s=document.createElement("span");s.textContent=v;var b=document.createElement("button");b.type="button";b.className="danger";b.textContent="X";b.onclick=function(){row.parentNode.removeChild(row)};row.appendChild(s);row.appendChild(b);assignList.appendChild(row)})}function addAssign(v){v=(v||"").trim();if(!v){return}var a=assignVals();if(a.indexOf(v)<0){a.push(v);renderAssign(a)}contactSearch.value="";suggest.style.display="none"}function od(v){return String(v||"").replace(/\\x27/g,"\\x27\\x27")}function contactLine(u){var n=u.displayName||u.mail||u.userPrincipalName||"";var l=u.mail||u.userPrincipalName||u.id||n;return n+(l&&l!==n?" <"+l+">":"")}function showSuggest(rows){suggest.innerHTML="";if(!rows.length){suggest.style.display="none";return}rows.forEach(function(u){var d=document.createElement("div");var line=contactLine(u);d.innerHTML="<b>"+(u.displayName||line)+"</b><span class=h>"+(u.mail||u.userPrincipalName||"")+"</span>";d.onclick=function(){addAssign(line)};suggest.appendChild(d)});suggest.style.display="block"}function graphSearch(q){if(!graphToken||q.length<2){showSuggest([]);return}var f=\"startswith(displayName,\\x27\"+od(q)+\"\\x27) or startswith(mail,\\x27\"+od(q)+\"\\x27) or startswith(userPrincipalName,\\x27\"+od(q)+\"\\x27)\";fetch(\"https://graph.microsoft.com/v1.0/users?$select=id,displayName,mail,userPrincipalName&$top=8&$filter=\"+encodeURIComponent(f),{headers:{Authorization:\"Bearer \"+graphToken}}).then(function(r){return r.ok?r.json():{value:[]}}).then(function(d){showSuggest(d.value||[])}).catch(function(){showSuggest([])})}var searchTimer=null;contactSearch.oninput=function(){clearTimeout(searchTimer);var q=this.value.trim();searchTimer=setTimeout(function(){graphSearch(q)},250)};manualAssign.onclick=function(){addAssign(contactSearch.value)};rowList(replyList,r.value?r.value.split(\"|\"):[],0);rowList(labelList,labels,6);renderAssign(assigns);addReply.onclick=function(){addRow(replyList,0)};function payload(a){return{mode:\"teams\",action:a,tenantId:t.value.trim()||\"organizations\",clientId:c.value.trim(),scopes:s.value.trim(),mockMode:m.checked?\"1\":\"\",cannedReplies:vals(replyList).join(\"|\"),plannerLabels:vals(labelList).join(\"|\"),assignContacts:assignVals().join(\"\\n\")}}function close(a){location=\"pebblejs://close#\"+encodeURIComponent(JSON.stringify(payload(a)))}signin.onclick=function(){close(\"connect\")};if(hasCode&&window.openlogin){openlogin.onclick=function(){location=loginlink.href}}mock.onclick=function(){m.checked=true;close(\"mock\")};signout.onclick=function(){close(\"signout\")};save.onclick=function(){close(\"save\")}</script></body></html>';
  return 'data:text/html;charset=utf-8,' + encodeURIComponent(html);
}

function create(options) {
  options = options || {};
  var status = options.status || function() {};
  pendingDeviceCodeCallback = options.onDeviceCode || null;

  return {
    settingsPageUrl: function() {
      return inlineSettingsUrl(options, '');
    },
    settingsPageUrlAsync: function() {
      if (!useGraph()) {
        return Promise.resolve(inlineSettingsUrl(options, ''));
      }
      return ensureToken(status, false).then(function(token) {
        return inlineSettingsUrl(options, token);
      }, function() {
        return inlineSettingsUrl(options, '');
      });
    },
    applySettings: function(data) {
      if (data && data.mode === 'teams') {
        var nextTenant = data.tenantId || DEFAULT_TENANT;
        var nextClient = data.clientId || BUILT_IN_CLIENT_ID;
        var nextScopes = !data.scopes || data.scopes === OLD_DEFAULT_SCOPES ? DEFAULT_SCOPES : data.scopes;
        var changed = nextTenant !== authTenantId() ||
          nextClient !== authClientId() ||
          nextScopes !== authScopes();

        saveSetting('tenantId', nextTenant);
        saveSetting('clientId', nextClient === BUILT_IN_CLIENT_ID ? '' : nextClient);
        saveSetting('scopes', nextScopes);
        saveSetting('plannerLabels', data.plannerLabels || '');
        saveSetting('assignContacts', data.assignContacts || '');

        if (data.action === 'mock' || data.mockMode === '1') {
          saveSetting('mockMode', '1');
        } else {
          removeSetting('mockMode');
        }

        if (changed || data.action === 'signout') {
          clearAuthState();
        }
        if (data.action === 'signout') {
          saveSetting('mockMode', '1');
        }
      }
      return data && data.action;
    },
    plannerLabels: function() {
      return plannerLabelsPacked();
    },
    connect: function(forceNewCode) {
      if (!graphConfigured()) {
        return Promise.reject(new Error('Microsoft app ID missing'));
      }
      removeSetting('mockMode');
      if (forceNewCode) {
        clearAuthState();
        tokenPromise = startDeviceCode(status);
        return tokenPromise.then(function() {
          tokenPromise = null;
          return {addEventHandler: function() {}};
        }, function(err) {
          tokenPromise = null;
          throw err;
        });
      }
      if (tokenPromise && deviceCodeActive()) {
        showActiveDeviceCode(status);
        return tokenPromise.then(function() {
          return {addEventHandler: function() {}};
        });
      }
      return ensureToken(status, true).then(function() {
        return {addEventHandler: function() {}};
      });
    },
    ready: function() {
      if (!useGraph()) {
        return Promise.resolve({addEventHandler: function() {}});
      }
      return ensureToken(status, false).then(function() {
        return {addEventHandler: function() {}};
      });
    },
    chats: function(limit) {
      return useGraph() ? graphChats(limit, status) : Promise.resolve(mockChatRows(limit));
    },
    groupChats: function(limit) {
      if (useGraph()) {
        return graphGroupChats(limit, status);
      }
      if (!graphGroupChatRows || !graphGroupChatRows.length) {
        mockChatRows(50);
      }
      return Promise.resolve(clone(graphGroupChatRows || []).slice(0, limit || 20));
    },
    messages: function(chatToken, limit, beforeId) {
      return useGraph() ? graphMessages(chatToken, limit, beforeId, status) :
        Promise.resolve(clone(mockChatRowsForId(chatToken)));
    },
    olderMessages: function(chatToken, limit, beforeId) {
      return useGraph() ? graphMessagesPage(chatToken, limit, beforeId, 'older', status) :
        this.messages(chatToken, limit, beforeId);
    },
    newerMessages: function(chatToken, limit, afterId) {
      return useGraph() ? graphMessagesPage(chatToken, limit, afterId, 'newer', status) :
        Promise.resolve([]);
    },
    keepalive: function() {
      return useGraph() ? ensureToken(status, false) : Promise.resolve();
    },
    sendMessage: function(chatToken, text) {
      if (useGraph()) {
        return graph('/chats/' + encodeURIComponent(realId('chat', chatToken)) + '/messages', {
          method: 'POST',
          status: status,
          body: {body: {contentType: 'text', content: String(text || '')}}
        });
      }
      var id = realId('chat', chatToken);
      var rows = mockMessages[id] || (mockMessages[id] = []);
      rows.push({id: String(Date.now()), sender: 'You', text: String(text || ''), outgoing: true, meta: 'Now'});
      return Promise.resolve();
    },
    editMessage: function(chatToken, messageToken, text) {
      if (useGraph()) {
        return graph('/chats/' + encodeURIComponent(realId('chat', chatToken)) + '/messages/' +
          encodeURIComponent(realMessageId(chatToken, messageToken)), {
          method: 'PATCH',
          status: status,
          body: {
            body: {
              contentType: 'text',
              content: String(text || '')
            }
          }
        });
      }
      var rows = mockChatRowsForId(chatToken);
      rows.forEach(function(row) {
        if (String(row.id) === String(messageToken)) {
          row.text = String(text || '');
          row.meta = 'Edited';
        }
      });
      return Promise.resolve();
    },
    sendReaction: function(chatToken, messageToken, token) {
      if (useGraph()) {
        var reactionType = graphReactionType(token);
        if (token !== 'remove' && !reactionType) {
          return Promise.reject(new Error('Unsupported reaction'));
        }
        if (token === 'remove') {
          return graphUnsetReaction(chatToken, messageToken, status);
        }
        return graph('/chats/' + encodeURIComponent(realId('chat', chatToken)) + '/messages/' +
          encodeURIComponent(realMessageId(chatToken, messageToken)) + '/setReaction', {
          method: 'POST',
          status: status,
          body: {reactionType: reactionType}
        });
      }
      return Promise.resolve();
    },
    message: function(chatToken, messageToken) {
      if (useGraph()) {
        return graph('/chats/' + encodeURIComponent(realId('chat', chatToken)) + '/messages/' +
          encodeURIComponent(realMessageId(chatToken, messageToken)), {
          status: status
        }).then(function(message) {
          return graphMessageRow(message, chatToken);
        });
      }
      var rows = mockChatRowsForId(chatToken);
      var found = null;
      rows.forEach(function(row) {
        if (String(row.id) === String(messageToken)) {
          found = row;
        }
      });
      return Promise.resolve(clone(found || {}));
    },
    deleteMessage: function(chatToken, messageToken) {
      if (useGraph()) {
        return graph('/me/chats/' + encodeURIComponent(realId('chat', chatToken)) + '/messages/' +
          encodeURIComponent(realMessageId(chatToken, messageToken)) + '/softDelete', {
          method: 'POST',
          status: status
        });
      }
      var id = realId('chat', chatToken);
      mockMessages[id] = (mockMessages[id] || []).filter(function(row) {
        return String(row.id) !== String(messageToken);
      });
      return Promise.resolve();
    },
    markRead: function(chatToken) {
      return useGraph() ? graphChatUserAction(chatToken, 'markChatReadForUser', {}, status) : Promise.resolve();
    },
    archiveChat: function(chatToken) {
      return useGraph() ? graphChatUserAction(chatToken, 'hideForUser', {}, status) : Promise.resolve();
    },
    markUnread: function(chatToken) {
      return useGraph() ? graphChatUserAction(chatToken, 'markChatUnreadForUser', {}, status) : Promise.resolve();
    },
    plans: function() {
      return useGraph() ? graphPlans(status) : Promise.resolve(applyPinnedPlanOrder(mockRows(mockPlans, 'plan')));
    },
    togglePlanPin: function(planToken) {
      return togglePlanPin(planToken);
    },
    buckets: function(planToken) {
      return useGraph() ? graphBuckets(planToken, status) : Promise.resolve(mockRows(mockBuckets[realId('plan', planToken)], 'bucket'));
    },
    tasks: function(bucketToken) {
      if (useGraph()) {
        return graphTasks(bucketToken, status).then(function(rows) {
          return rows.filter(function(row) {
            var task = taskCache[row.id] || {};
            return (task.percentComplete || 0) < 100;
          });
        });
      }
      var rows = mockRows((mockTasks[realId('bucket', bucketToken)] || []).filter(function(task) {
        return (task.percentComplete || 0) < 100;
      }), 'task');
      bucketTaskCache[bucketToken] = rows;
      return Promise.resolve(rows);
    },
    completedTasks: function(bucketToken) {
      if (useGraph()) {
        return graphTasks(bucketToken, status).then(function(rows) {
          return rows.filter(function(row) {
            var task = taskCache[row.id] || {};
            return (task.percentComplete || 0) >= 100;
          });
        });
      }
      return Promise.resolve(mockRows((mockTasks[realId('bucket', bucketToken)] || []).filter(function(task) {
        return (task.percentComplete || 0) >= 100;
      }), 'task'));
    },
    taskDetail: function(taskToken) {
      return useGraph() ? graphTaskDetail(taskToken, status) : Promise.resolve(mockTaskDetail(taskToken));
    },
    createTask: function(bucketToken, title) {
      return useGraph() ? graphCreateTask(bucketToken, title, status) : mockCreateTask(bucketToken, title);
    },
    createBucket: function(planToken, name) {
      return useGraph() ? graphCreateBucket(planToken, name, status) : mockCreateBucket(planToken, name);
    },
    addTaskNote: function(taskToken, text) {
      if (useGraph()) {
        return graphAddTaskNote(taskToken, text, status);
      }
      return mockUpdateDetailRow(taskToken, 'notes', function(row) {
        row.text = row.text && row.text !== 'No notes yet' ? row.text + '\n' + String(text || '') : String(text || '');
      });
    },
    addChecklistItem: function(taskToken, text) {
      if (useGraph()) {
        return graphAddChecklistItem(taskToken, text, status);
      }
      return mockUpdateDetailRow(taskToken, 'checklist', function(row) {
        var lines = checklistLines(row.text);
        lines.push(makeChecklistLine(text));
        row.text = lines.join('\n');
      });
    },
    setTaskDue: function(taskToken, text) {
      if (useGraph()) {
        return graphSetTaskDue(taskToken, text, status);
      }
      return mockUpdateDetailRow(taskToken, 'due', function(row) {
        row.text = String(text || '').trim() || 'No due date';
      });
    },
    completeTask: function(taskToken) {
      if (useGraph()) {
        return graphCompleteTask(taskToken, status);
      }
      var task = findMockTask(taskToken);
      if (task) {
        task.percentComplete = 100;
        task.preview = 'Complete';
      }
      return mockUpdateDetailRow(taskToken, 'status', function(row) {
        row.text = 'Complete';
      });
    },
    uncompleteTask: function(taskToken) {
      if (useGraph()) {
        return graphUncompleteTask(taskToken, status);
      }
      var task = findMockTask(taskToken);
      if (task) {
        task.percentComplete = 0;
        task.preview = '0% complete';
      }
      return mockUpdateDetailRow(taskToken, 'status', function(row) {
        row.text = '0% complete';
      });
    },
    deleteTask: function(taskToken) {
      return useGraph() ? graphDeleteTask(taskToken, status) : removeMockTask(taskToken);
    },
    plannerWrite: function(taskToken, command, text, messageId) {
      return useGraph() ? graphPlannerWrite(taskToken, command, text, messageId, status) :
                          mockPlannerWrite(taskToken, command, text, messageId);
    },
    assignTeams: function() {
      if (useGraph()) {
        return graphAssignTeams(status);
      }
      return Promise.resolve([
        {id: tokenFor('assignTeam', 'mock-team-it'), title: 'IT Team', preview: 'Mock team', kind: 'assignTeam'},
        {id: tokenFor('assignTeam', 'mock-team-sec'), title: 'Security Team', preview: 'Mock team', kind: 'assignTeam'}
      ]);
    },
    assignConfiguredContacts: function() {
      return Promise.resolve(configuredAssignRows());
    },
    assignTeamMembers: function(teamToken) {
      if (useGraph()) {
        return graphAssignTeamMembers(teamToken, status);
      }
      return Promise.resolve([
        {id: tokenFor('assignUser', 'mock-user-jordan'), title: 'Jordan Lee', preview: 'Mock member', kind: 'assignMember'},
        {id: tokenFor('assignUser', 'mock-user-maya'), title: 'Maya Patel', preview: 'Mock member', kind: 'assignMember'},
        {id: tokenFor('assignUser', 'mock-user-sam'), title: 'Sam Rivera', preview: 'Mock member', kind: 'assignMember'}
      ]);
    },
    toggleChecklist: function(taskToken, messageId, itemIndex) {
      if (useGraph()) {
        return graphToggleChecklist(taskToken, itemIndex, status);
      }
      return toggleMockChecklist(taskToken, itemIndex);
    },
    imageBytes: function(chatToken, messageToken, width, height, colors, maxBytes, maxPixels, retryLevel, maxCost, forceTall) {
      if (!useGraph()) {
        return Promise.reject(new Error('Images unavailable in mock mode'));
      }
      return graphImage.graphImageBytes(chatToken, messageToken, function() {
        return graphMessageImageBytes(chatToken, messageToken, status);
      }, width, height, colors, maxBytes, maxPixels, retryLevel, maxCost, forceTall, status);
    },
    avatarBytes: function(chatToken, width, height, colors, maxBytes) {
      if (!useGraph()) {
        return mockAvatarBytes(chatToken, maxBytes);
      }
      var peer = chatPeerCache[chatToken];
      if (!peer || !peer.id) {
        return Promise.reject(new Error('No 1:1 contact'));
      }
      return graphImage.graphAvatarBytes(realId('chat', chatToken) || chatToken, function() {
        return peerPhotoBytes(chatToken, peer, status);
      }, width, height, colors, maxBytes);
    },
    cancelImageRequests: graphImage.cancelImageRequests
  };
}

module.exports = {
  create: create
};
