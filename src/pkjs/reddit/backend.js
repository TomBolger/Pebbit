var image = require('./image');

var SETTINGS_PREFIX = 'pebbit.';
var REDDIT_AUTH = 'https://www.reddit.com/api/v1/authorize';
var REDDIT_TOKEN = 'https://www.reddit.com/api/v1/access_token';
var REDDIT_API = 'https://oauth.reddit.com';
var DEFAULT_SCOPES = 'identity read mysubreddits vote save submit edit';
var LOCAL_REDIRECT_URI = 'http://localhost:65010/pebbit';
var DEFAULT_REDIRECT_URI = 'https://tombolger.github.io/Pebbit/reddit-callback.html';
var DEFAULT_SUBREDDITS = 'pebble|pebblewatch|programming|technology|AskReddit';
var USER_AGENT = 'Pebbit/0.1 personal PebbleOS client by u/tombolger';
var MAX_CACHE_MS = 5 * 60 * 1000;

var tokenMaps = {
  post: {},
  comment: {},
  subreddit: {},
  sort: {},
  url: {}
};
var reverseTokenMaps = {
  post: {},
  comment: {},
  subreddit: {},
  sort: {},
  url: {}
};
var tokenCounters = {
  post: 0,
  comment: 0,
  subreddit: 0,
  sort: 0,
  url: 0
};
var postCache = {};
var postRowsByFeed = {};
var commentsByPost = {};
var fullTextById = {};
var savedByToken = {};
var actionInFlight = false;
var tokenPromise = null;

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

function nowSeconds() {
  return Math.floor(Date.now() / 1000);
}

function clone(value) {
  return JSON.parse(JSON.stringify(value || null));
}

function tokenFor(kind, value) {
  value = String(value || '');
  if (!value) {
    return '';
  }
  if (!reverseTokenMaps[kind][value]) {
    tokenCounters[kind] += 1;
    reverseTokenMaps[kind][value] = kind.charAt(0) + tokenCounters[kind].toString(36);
    tokenMaps[kind][reverseTokenMaps[kind][value]] = value;
  }
  return reverseTokenMaps[kind][value];
}

function realId(kind, token) {
  return tokenMaps[kind][token] || token || '';
}

function splitList(value) {
  return String(value || '').split(/\r?\n|\||,/).join('|').split('|').map(function(item) {
    return item.trim().replace(/^r\//i, '');
  }).filter(function(item) {
    return !!item;
  });
}

function escapeHtml(value) {
  return String(value || '').replace(/[&<>"']/g, function(ch) {
    return {'&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;'}[ch];
  });
}

function decodeHtml(value) {
  return String(value || '')
    .replace(/&amp;/g, '&')
    .replace(/&lt;/g, '<')
    .replace(/&gt;/g, '>')
    .replace(/&quot;/g, '"')
    .replace(/&#39;/g, "'")
    .replace(/&apos;/g, "'");
}

function plainText(value) {
  return decodeHtml(String(value || '')
    .replace(/<br\s*\/?>/gi, '\n')
    .replace(/<\/p>/gi, '\n')
    .replace(/<[^>]+>/g, '')
    .replace(/\r/g, '')
    .replace(/\n{3,}/g, '\n\n')
    .trim());
}

function compactNumber(value) {
  var n = Number(value || 0);
  if (Math.abs(n) >= 1000000) {
    return (n / 1000000).toFixed(1).replace(/\.0$/, '') + 'm';
  }
  if (Math.abs(n) >= 1000) {
    return (n / 1000).toFixed(1).replace(/\.0$/, '') + 'k';
  }
  return String(n);
}

function ageText(createdUtc) {
  var seconds = Math.max(0, nowSeconds() - Number(createdUtc || 0));
  if (seconds < 90) return 'now';
  if (seconds < 3600) return Math.floor(seconds / 60) + 'm';
  if (seconds < 86400) return Math.floor(seconds / 3600) + 'h';
  if (seconds < 2592000) return Math.floor(seconds / 86400) + 'd';
  return Math.floor(seconds / 2592000) + 'mo';
}

function mockModeEnabled() {
  return setting('mockMode', '') === '1';
}

function isMockMode() {
  return mockModeEnabled() || !setting('clientId', '');
}

function authConfigured() {
  return !!setting('clientId', '');
}

function authHeaders() {
  var clientId = setting('clientId', '');
  return {
    Authorization: 'Basic ' + btoa(clientId + ':'),
    'Content-Type': 'application/x-www-form-urlencoded',
    'User-Agent': USER_AGENT
  };
}

function formBody(values) {
  var parts = [];
  Object.keys(values).forEach(function(key) {
    if (values[key] !== undefined && values[key] !== null) {
      parts.push(encodeURIComponent(key) + '=' + encodeURIComponent(values[key]));
    }
  });
  return parts.join('&');
}

function tokenExpired() {
  return Number(setting('accessTokenExpiresAt', '0')) < nowSeconds() + 60;
}

function saveTokenResponse(data) {
  if (data.access_token) {
    saveSetting('accessToken', data.access_token);
    saveSetting('accessTokenExpiresAt', String(nowSeconds() + Number(data.expires_in || 3600)));
  }
  if (data.refresh_token) {
    saveSetting('refreshToken', data.refresh_token);
  }
}

function cleanCode(code) {
  var value = String(code || '').trim();
  var match = value.match(/[?&]code=([^&#]*)/);
  if (match) {
    value = match[1];
  } else if (value.indexOf('code=') === 0) {
    value = value.substring(5);
  }
  value = value.split('&')[0].split('#')[0].trim();
  try {
    value = decodeURIComponent(value.replace(/\+/g, ' '));
  } catch (e) {
  }
  value = value.replace(/#_?$/, '').trim();
  return value;
}

function exchangeCode(code) {
  code = cleanCode(code);
  if (!code) {
    return Promise.reject(new Error('Reddit authorization code missing'));
  }
  return fetch(REDDIT_TOKEN, {
    method: 'POST',
    headers: authHeaders(),
    body: formBody({
      grant_type: 'authorization_code',
      code: code,
      redirect_uri: setting('redirectUri', DEFAULT_REDIRECT_URI)
    })
  }).then(readJson).then(function(data) {
    saveTokenResponse(data);
    saveSetting('mockMode', '0');
    return data;
  });
}

function refreshToken() {
  var refresh = setting('refreshToken', '');
  if (!refresh) {
    return Promise.reject(new Error('Reddit sign-in required'));
  }
  return fetch(REDDIT_TOKEN, {
    method: 'POST',
    headers: authHeaders(),
    body: formBody({
      grant_type: 'refresh_token',
      refresh_token: refresh
    })
  }).then(readJson).then(function(data) {
    saveTokenResponse(data);
    return data.access_token;
  });
}

function ensureToken() {
  if (isMockMode()) {
    return Promise.resolve('');
  }
  if (!authConfigured()) {
    return Promise.reject(new Error('Reddit client ID missing'));
  }
  if (setting('accessToken', '') && !tokenExpired()) {
    return Promise.resolve(setting('accessToken', ''));
  }
  if (!tokenPromise) {
    tokenPromise = refreshToken().then(function(token) {
      tokenPromise = null;
      return token;
    }, function(err) {
      tokenPromise = null;
      throw err;
    });
  }
  return tokenPromise;
}

function readJson(response) {
  return response.text().then(function(text) {
    var data = text ? JSON.parse(text) : {};
    if (!response.ok) {
      throw new Error((data && (data.message || data.error_description || data.error)) ||
                      ('HTTP ' + response.status));
    }
    return data;
  });
}

function reddit(path, options) {
  options = options || {};
  return ensureToken().then(function(token) {
    var headers = {
      Authorization: 'bearer ' + token,
      'User-Agent': USER_AGENT
    };
    var url = REDDIT_API + path + (path.indexOf('?') >= 0 ? '&' : '?') + 'raw_json=1';
    if (options.body) {
      headers['Content-Type'] = 'application/x-www-form-urlencoded';
    }
    return fetch(url, {
      method: options.method || 'GET',
      headers: headers,
      body: options.body ? formBody(options.body) : undefined
    }).then(readJson);
  });
}

function mockPosts(feed) {
  var base = [
    {
      id: 't3_mock1',
      title: 'PebbleOS still makes tiny internet clients feel possible',
      subreddit: 'pebble',
      author: 'smallwatchfan',
      selftext: 'A Reddit client on Pebble is a ridiculous little joy. This mock post exercises a self-text body, comments, scores, and a quote reply.',
      score: 128,
      num_comments: 24,
      created_utc: nowSeconds() - 3900,
      domain: 'self.pebble',
      url: '',
      saved: false,
      likes: null
    },
    {
      id: 't3_mock2',
      title: 'Show HN: A color e-paper inspired Pebble UI',
      subreddit: 'programming',
      author: 'threadweaver',
      selftext: 'External links should stay readable without loading media automatically.',
      score: 4200,
      num_comments: 312,
      created_utc: nowSeconds() - 7400,
      domain: 'example.com',
      url: 'https://picsum.photos/480/320',
      saved: true,
      likes: true
    },
    {
      id: 't3_mock3',
      title: 'What old device do you still use every day?',
      subreddit: 'AskReddit',
      author: 'curiousclock',
      selftext: '',
      score: 987,
      num_comments: 1800,
      created_utc: nowSeconds() - 21000,
      domain: 'self.AskReddit',
      url: '',
      saved: false,
      likes: false
    }
  ];
  if (feed === 'saved') {
    return base.filter(function(post) { return post.saved; });
  }
  return base;
}

var mockComments = {
  t3_mock1: [
    {id: 't1_c1', parent_id: 't3_mock1', author: 'tinytype', body: 'The existing chat bubble UI is a surprisingly good fit for comment threads.', score: 42, depth: 0, created_utc: nowSeconds() - 3000},
    {id: 't1_c2', parent_id: 't1_c1', author: 'watchkbd', body: 'Especially with sender names and quote panels already working.', score: 18, depth: 1, created_utc: nowSeconds() - 2500},
    {id: 't1_c3', parent_id: 't1_c2', author: 'you', body: 'Dictation replies are the whole trick.', score: 9, depth: 2, created_utc: nowSeconds() - 1600, outgoing: true},
    {id: 't1_c4', parent_id: 't3_mock1', author: 'ratelimitreader', body: 'Media should stay selected-only. Pebble heap is not a buffet.', score: 31, depth: 0, created_utc: nowSeconds() - 1200}
  ],
  t3_mock2: [
    {id: 't1_c5', parent_id: 't3_mock2', author: 'linkparser', body: 'Good preview text matters more than automatic images on a watch.', score: 77, depth: 0, created_utc: nowSeconds() - 6400},
    {id: 'more_mock2', kind: 'more', author: 'More', body: 'Load more comments', score: 0, depth: 0, created_utc: nowSeconds() - 6200}
  ],
  t3_mock3: [
    {id: 't1_c6', parent_id: 't3_mock3', author: 'palmtop', body: 'A Pebble Time. The battery is still better than my new watch.', score: 230, depth: 0, created_utc: nowSeconds() - 18000}
  ]
};

function postPreview(post) {
  var parts = ['r/' + post.subreddit, 'u/' + post.author, compactNumber(post.score) + ' pts',
    compactNumber(post.num_comments) + ' c', ageText(post.created_utc)];
  if (post.domain && post.domain.indexOf('self.') !== 0) {
    parts.push(post.domain);
  }
  if (post.saved) {
    parts.push('saved');
  }
  return parts.join(' - ');
}

function normalizePost(data) {
  var post = data && data.data ? data.data : data;
  var id = post.name || post.id || '';
  var token = tokenFor('post', id);
  var mediaUrl = bestMediaUrl(post);
  var row = {
    id: token,
    title: plainText(post.title || 'Untitled'),
    preview: postPreview(post),
    kind: 'post',
    section: mediaUrl ? 'media' : 'post',
    unread: post.likes !== null && post.likes !== undefined,
    unread_count: post.saved ? 1 : 0
  };
  postCache[token] = clone(post);
  savedByToken[token] = !!post.saved;
  fullTextById[token] = plainText(post.selftext || post.title || '');
  if (mediaUrl) {
    tokenMaps.url[token] = mediaUrl;
  }
  return row;
}

function validImageUrl(url) {
  return !!url && /^https?:\/\//i.test(url) && /\.(png|jpe?g|webp)(\?|$)/i.test(url);
}

function redditPreviewImage(post) {
  var images = post && post.preview && post.preview.images;
  if (!images || !images.length) {
    return '';
  }
  var image = images[0];
  var resolutions = image.resolutions || [];
  var candidate = resolutions.length ? resolutions[resolutions.length - 1] : image.source;
  return decodeHtml((candidate && candidate.url) || (image.source && image.source.url) || '');
}

function bestMediaUrl(post) {
  if (!post) {
    return '';
  }
  var url = decodeHtml(post.url_overridden_by_dest || post.url || '');
  var preview = redditPreviewImage(post);
  var thumbnail = decodeHtml(post.thumbnail || '');
  if (validImageUrl(url)) {
    return url;
  }
  if (validImageUrl(preview)) {
    return preview;
  }
  if (validImageUrl(thumbnail)) {
    return thumbnail;
  }
  if (post.media && post.media.oembed && validImageUrl(post.media.oembed.thumbnail_url)) {
    return decodeHtml(post.media.oembed.thumbnail_url);
  }
  if (post.secure_media && post.secure_media.oembed && validImageUrl(post.secure_media.oembed.thumbnail_url)) {
    return decodeHtml(post.secure_media.oembed.thumbnail_url);
  }
  return '';
}

function normalizeComment(data, parentById) {
  var comment = data && data.data ? data.data : data;
  var id = comment.name || comment.id || '';
  var token = tokenFor(comment.kind === 'more' || id.indexOf('more_') === 0 ? 'comment' : 'comment', id);
  var parent = parentById && parentById[comment.parent_id];
  var text = comment.kind === 'more' ? 'Load more comments' :
    plainText(comment.body || comment.body_html || '[deleted]');
  var meta = compactNumber(comment.score || 0) + ' pts ' + ageText(comment.created_utc);
  if (comment.edited && comment.edited !== false) {
    meta += ' edited';
  }
  var row = {
    id: token,
    sender: comment.kind === 'more' ? 'More' : ('u/' + (comment.author || '[deleted]')),
    text: text,
    meta: meta,
    reactions: comment.saved ? 'saved' : '',
    reply_sender: parent ? ('u/' + (parent.author || 'parent')) : '',
    reply_text: parent ? plainText(parent.body || parent.title || '') : '',
    outgoing: !!comment.outgoing,
    section: 'd' + Math.min(9, Number(comment.depth || 0))
  };
  savedByToken[token] = !!comment.saved;
  fullTextById[token] = text;
  return row;
}

function flattenComments(children, depth, rows, parentById) {
  (children || []).forEach(function(child) {
    if (!child || !child.data) {
      return;
    }
    if (child.kind === 'more') {
      rows.push({
        kind: 'more',
        id: 'more_' + (child.data.id || rows.length),
        name: 'more_' + (child.data.id || rows.length),
        parent_id: child.data.parent_id,
        author: 'More',
        body: 'Load more comments',
        depth: depth,
        created_utc: nowSeconds()
      });
      return;
    }
    child.data.depth = depth;
    parentById[child.data.name] = child.data;
    rows.push(child.data);
    if (child.data.replies && child.data.replies.data) {
      flattenComments(child.data.replies.data.children, depth + 1, rows, parentById);
    }
  });
}

function listingPath(feed, after) {
  var path;
  if (feed === 'front' || feed === 'best') {
    path = '/best';
  } else if (feed === 'hot') {
    path = '/hot';
  } else if (feed === 'trending' || feed === 'rising') {
    path = '/rising';
  } else if (feed === 'saved') {
    path = '/user/' + encodeURIComponent(setting('username', 'me')) + '/saved';
  } else if (feed.indexOf('r/') === 0) {
    path = '/' + feed;
  } else {
    path = '/best';
  }
  return path + '?limit=16' + (after ? '&after=' + encodeURIComponent(after) : '');
}

function loadListing(feed) {
  if (isMockMode()) {
    var rows = mockPosts(feed).map(normalizePost);
    postRowsByFeed[feed] = rows;
    return Promise.resolve(rows);
  }
  return reddit(listingPath(feed)).then(function(data) {
    var rows = (((data || {}).data || {}).children || []).map(normalizePost);
    postRowsByFeed[feed] = rows;
    return rows;
  });
}

function loadComments(postToken) {
  var post = postCache[postToken] || {};
  var real = realId('post', postToken);
  if (isMockMode()) {
    var comments = (mockComments[real] || mockComments[post.id] || []).slice();
    var parentById = {};
    parentById[real] = post;
    comments.forEach(function(row) { parentById[row.id] = row; });
    var rows = [postMessage(postToken)].concat(comments.map(function(row) {
      return normalizeComment(row, parentById);
    }));
    commentsByPost[postToken] = rows;
    return Promise.resolve(rows);
  }
  var articleId = real.replace(/^t3_/, '') || (post.id || '');
  var path = '/comments/' + encodeURIComponent(articleId) +
    '?limit=40&depth=6&sort=' + encodeURIComponent(setting('commentSort', 'confidence'));
  if (post.subreddit) {
    path = '/r/' + encodeURIComponent(post.subreddit) + path;
  }
  return reddit(path).then(function(data) {
    var listing = data && data[1] && data[1].data ? data[1].data.children : [];
    var flat = [];
    var parentById = {};
    parentById[real] = post;
    flattenComments(listing, 0, flat, parentById);
    var rows = [postMessage(postToken)].concat(flat.slice(0, 60).map(function(row) {
      return normalizeComment(row, parentById);
    }));
    commentsByPost[postToken] = rows;
    return rows;
  });
}

function postMessage(postToken) {
  var post = postCache[postToken] || {};
  var text = plainText(post.selftext || post.title || '');
  if (!text && post.url) {
    text = post.url;
  }
  return {
    id: postToken,
    sender: 'r/' + (post.subreddit || 'reddit'),
    text: text || '[link post]',
    meta: compactNumber(post.score || 0) + ' pts ' + compactNumber(post.num_comments || 0) + ' c ' +
      ageText(post.created_utc),
    reactions: post.saved ? 'saved' : '',
    outgoing: false,
    section: 'post',
    image_token: tokenMaps.url[postToken] ? postToken : '',
    image_width: tokenMaps.url[postToken] ? 320 : 0,
    image_height: tokenMaps.url[postToken] ? 220 : 0
  };
}

function navRows() {
  return [
    {id: '__feed_front', title: 'Front Page', preview: 'Best posts from your Reddit home', kind: 'feed'},
    {id: '__feed_hot', title: 'Hot', preview: 'Posts rising across Reddit', kind: 'feed'},
    {id: '__feed_trending', title: 'Trending', preview: 'Rising posts', kind: 'feed'},
    {id: '__feed_saved', title: 'Saved', preview: 'Saved posts and comments', kind: 'feed'},
    {id: '__subreddit', title: 'Subreddits', preview: 'Browse configured communities', kind: 'planner'}
  ];
}

function subredditRows() {
  var rows = splitList(setting('subreddits', DEFAULT_SUBREDDITS)).map(function(name) {
    return {
      id: tokenFor('subreddit', name),
      title: 'r/' + name,
      preview: 'Hot, new, top, and rising posts',
      kind: 'plan',
      section: 'subreddit'
    };
  });
  return Promise.resolve(rows);
}

function sortRows(subredditToken) {
  var subreddit = realId('subreddit', subredditToken);
  return Promise.resolve(['hot', 'new', 'top', 'rising'].map(function(sort) {
    return {
      id: tokenFor('sort', subreddit + ':' + sort),
      title: sort.charAt(0).toUpperCase() + sort.substring(1),
      preview: 'r/' + subreddit + ' ' + sort + ' posts',
      kind: 'bucket',
      section: 'sort'
    };
  }));
}

function subredditPosts(sortToken) {
  var value = realId('sort', sortToken);
  var parts = value.split(':');
  var subreddit = parts[0] || 'all';
  var sort = parts[1] || 'hot';
  return loadListing('r/' + subreddit + '/' + sort);
}

function authUrl() {
  var state = String(Date.now());
  saveSetting('oauthState', state);
  return REDDIT_AUTH + '?' + formBody({
    client_id: setting('clientId', ''),
    response_type: 'code',
    state: state,
    redirect_uri: setting('redirectUri', DEFAULT_REDIRECT_URI),
    duration: 'permanent',
    scope: setting('scopes', DEFAULT_SCOPES)
  });
}

function inlineSettingsUrl() {
  var clientId = escapeHtml(setting('clientId', ''));
  var redirectUri = escapeHtml(setting('redirectUri', DEFAULT_REDIRECT_URI));
  var scopes = escapeHtml(setting('scopes', DEFAULT_SCOPES));
  var subreddits = escapeHtml(setting('subreddits', DEFAULT_SUBREDDITS).replace(/\|/g, '\n'));
  var hidden = escapeHtml(setting('hiddenSubreddits', '').replace(/\|/g, '\n'));
  var replies = escapeHtml(localStorage.getItem('cannedReplies') || 'Yes|No|Thanks|I agree|Good point');
  var signedIn = !!setting('refreshToken', '');
  var mockChecked = mockModeEnabled() ? ' checked' : '';
  var nsfwChecked = setting('showNsfw', '') === '1' ? ' checked' : '';
  var media = escapeHtml(setting('mediaMode', 'selected'));
  var auth = authConfigured() ? authUrl() : '';
  var html = '<!doctype html><html><head><meta charset=utf-8><meta name=viewport content="width=device-width,initial-scale=1">' +
    '<title>Pebbit Settings</title><style>body{margin:0;font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif;background:#f4f6f8;color:#17202a}main{max-width:620px;margin:auto;padding:18px}h1{margin:0 0 6px;font-size:26px}.lede,.h{color:#607080;font-size:14px;line-height:1.4}label{display:block;margin-top:15px;font-weight:700}input,textarea,select{box-sizing:border-box;width:100%;border:1px solid #c9d2dc;border-radius:6px;padding:11px;font:inherit;background:white}textarea{min-height:84px}.panel{margin:14px 0;padding:12px;border:1px solid #d6dde5;border-radius:6px;background:white}.row{display:flex;gap:10px}button,.btn{display:block;box-sizing:border-box;text-align:center;border:0;border-radius:6px;background:#ff4500;color:white;padding:12px;margin-top:12px;font:inherit;font-weight:800;text-decoration:none}.secondary{background:#e5edf3;color:#17202a}.danger{background:#334155;color:white}.toggle{display:flex;gap:8px;align-items:center}.toggle input{width:auto}.savebar{position:sticky;bottom:0;background:#f4f6f8;padding:10px 0}</style></head><body><main>' +
    '<h1>Pebbit</h1><p class=lede>Personal Reddit client settings. No post creation is exposed in this app.</p>' +
    '<div class=panel><b>Status:</b> ' + (signedIn ? 'Reddit refresh token saved.' : 'Not signed in.') +
    '<p class=h>Register a Reddit installed app, set its redirect URI to the value below, save, open this page again, then sign in and paste either the code or the full redirected URL. Pebbit ignores trailing #_ automatically.</p>' +
    '<p class=h>Hosted callback: ' + escapeHtml(DEFAULT_REDIRECT_URI) + '<br>Local fallback: ' + escapeHtml(LOCAL_REDIRECT_URI) + '</p></div>' +
    '<label>Reddit client ID</label><input id=clientId value="' + clientId + '" placeholder="Installed app client ID">' +
    '<label>Redirect URI</label><input id=redirectUri value="' + redirectUri + '">' +
    '<label>Authorization code</label><input id=code placeholder="Paste code or full redirected URL">' +
    '<div class=h>Paste the code value from the redirect URL. If the copied text ends in #_, leave it in or remove it; Pebbit strips it before signing in.</div>' +
    (auth ? '<a class=btn href="' + escapeHtml(auth) + '">Sign in with Reddit</a>' : '') +
    '<label>Default feed</label><select id=defaultFeed><option value=front>Front Page</option><option value=hot>Hot</option><option value=trending>Trending</option><option value=saved>Saved</option></select>' +
    '<label>Comment sort</label><select id=commentSort><option value=confidence>Best</option><option value=top>Top</option><option value=new>New</option><option value=controversial>Controversial</option><option value=old>Old</option></select>' +
    '<label>Visible subreddits</label><textarea id=subreddits>' + subreddits + '</textarea>' +
    '<label>Hidden subreddits</label><textarea id=hiddenSubreddits>' + hidden + '</textarea>' +
    '<label>Media loading</label><select id=mediaMode><option value=off>Off</option><option value=selected>Selected only</option><option value=nearby>Near viewport</option></select>' +
    '<label>Canned replies</label><textarea id=cannedReplies>' + replies.replace(/\|/g, '\n') + '</textarea>' +
    '<label class=toggle><input id=showNsfw type=checkbox' + nsfwChecked + '> Show NSFW rows</label>' +
    '<label class=toggle><input id=mockMode type=checkbox' + mockChecked + '> Use mock data</label>' +
    '<details><summary>Advanced</summary><label>OAuth scopes</label><input id=scopes value="' + scopes + '"></details>' +
    '<div class=savebar><button id=save>Save</button><button id=signout class=secondary>Sign out</button></div>' +
    '<script>function v(id){return document.getElementById(id)}function lines(id){return v(id).value.split(/\\n+/).map(function(x){return x.trim()}).filter(Boolean).join("|")}function cleanCode(x){x=String(x||"").trim();var m=x.match(/[?&]code=([^&#]*)/);if(m)x=m[1];else if(x.indexOf("code=")===0)x=x.substring(5);return x.split("&")[0].split("#")[0].trim()}v("defaultFeed").value="' + escapeHtml(setting('defaultFeed', 'front')) + '";v("commentSort").value="' + escapeHtml(setting('commentSort', 'confidence')) + '";v("mediaMode").value="' + media + '";function close(a){var d={mode:"reddit",action:a,clientId:v("clientId").value.trim(),redirectUri:v("redirectUri").value.trim(),code:cleanCode(v("code").value),defaultFeed:v("defaultFeed").value,commentSort:v("commentSort").value,subreddits:lines("subreddits"),hiddenSubreddits:lines("hiddenSubreddits"),mediaMode:v("mediaMode").value,cannedReplies:lines("cannedReplies"),showNsfw:v("showNsfw").checked?"1":"",mockMode:v("mockMode").checked?"1":"0",scopes:v("scopes").value.trim()};location="pebblejs://close#"+encodeURIComponent(JSON.stringify(d))}v("save").onclick=function(){close("save")};v("signout").onclick=function(){close("signout")}</script>' +
    '</main></body></html>';
  return 'data:text/html;charset=utf-8,' + encodeURIComponent(html);
}

function actionGuard(fn) {
  if (actionInFlight) {
    return Promise.reject(new Error('Action already running'));
  }
  actionInFlight = true;
  return fn().then(function(value) {
    actionInFlight = false;
    return value;
  }, function(err) {
    actionInFlight = false;
    throw err;
  });
}

function voteThing(token, dir) {
  if (isMockMode()) {
    var post = postCache[token];
    if (post) post.likes = dir > 0 ? true : (dir < 0 ? false : null);
    return Promise.resolve();
  }
  return reddit('/api/vote', {method: 'POST', body: {id: realId('post', token) || realId('comment', token), dir: dir}});
}

function saveThing(token) {
  var id = realId('post', token) || realId('comment', token);
  if (isMockMode()) {
    savedByToken[token] = !savedByToken[token];
    if (postCache[token]) postCache[token].saved = savedByToken[token];
    return Promise.resolve();
  }
  var post = postCache[token];
  var saved = savedByToken[token] !== undefined ? savedByToken[token] : (post && !!post.saved);
  return reddit(saved ? '/api/unsave' : '/api/save', {method: 'POST', body: {id: id}}).then(function(result) {
    savedByToken[token] = !saved;
    if (post) {
      post.saved = savedByToken[token];
    }
    return result;
  });
}

function replyThing(postToken, parentToken, text) {
  var parent = parentToken ? realId('comment', parentToken) : realId('post', postToken);
  if (!parent || !text) {
    return Promise.reject(new Error('Reply needs text'));
  }
  if (isMockMode()) {
    var realPost = realId('post', postToken);
    mockComments[realPost] = mockComments[realPost] || [];
    mockComments[realPost].push({
      id: 't1_mock_reply_' + Date.now(),
      parent_id: parent,
      author: 'you',
      body: text,
      score: 1,
      depth: parent.indexOf('t1_') === 0 ? 1 : 0,
      created_utc: nowSeconds(),
      outgoing: true
    });
    return Promise.resolve();
  }
  return reddit('/api/comment', {method: 'POST', body: {thing_id: parent, text: text, api_type: 'json'}});
}

function editThing(commentToken, text) {
  if (!text) {
    return Promise.reject(new Error('Edit needs text'));
  }
  if (isMockMode()) {
    return Promise.resolve();
  }
  return reddit('/api/editusertext', {
    method: 'POST',
    body: {thing_id: realId('comment', commentToken), text: text, api_type: 'json'}
  });
}

function create(options) {
  options = options || {};
  var status = options.status || function() {};
  return {
    settingsPageUrl: inlineSettingsUrl,
    settingsPageUrlAsync: function() {
      return Promise.resolve(inlineSettingsUrl());
    },
    applySettings: function(data) {
      if (data && data.mode === 'reddit') {
        saveSetting('clientId', data.clientId || '');
        saveSetting('redirectUri', data.redirectUri || DEFAULT_REDIRECT_URI);
        saveSetting('scopes', data.scopes || DEFAULT_SCOPES);
        saveSetting('defaultFeed', data.defaultFeed || 'front');
        saveSetting('commentSort', data.commentSort || 'confidence');
        saveSetting('subreddits', data.subreddits || DEFAULT_SUBREDDITS);
        saveSetting('hiddenSubreddits', data.hiddenSubreddits || '');
        saveSetting('mediaMode', data.mediaMode || 'selected');
        saveSetting('showNsfw', data.showNsfw || '');
        if (data.cannedReplies !== undefined) {
          localStorage.setItem('cannedReplies', data.cannedReplies);
        }
        if (data.mockMode === '1') {
          saveSetting('mockMode', '1');
        } else {
          saveSetting('mockMode', '0');
        }
        if (data.action === 'signout') {
          ['accessToken', 'accessTokenExpiresAt', 'refreshToken', 'username'].forEach(removeSetting);
          saveSetting('mockMode', '1');
        }
        if (cleanCode(data.code)) {
          return exchangeCode(data.code).then(function() {
            return data.action || 'connect';
          });
        }
      }
      return data && data.action;
    },
    plannerLabels: function() {
      return '';
    },
    connect: function() {
      return ensureToken().then(function() {
        return {addEventHandler: function() {}};
      });
    },
    ready: function() {
      return Promise.resolve({addEventHandler: function() {}});
    },
    keepalive: function() {
      return Promise.resolve();
    },
    chats: function() {
      return Promise.resolve(navRows());
    },
    groupChats: function(maxRows, feedId) {
      feedId = String(feedId || '').replace('__feed_', '') || setting('defaultFeed', 'front');
      status('Fetching posts...');
      return loadListing(feedId);
    },
    plans: subredditRows,
    togglePlanPin: function() {
      return Promise.resolve('Subreddit saved');
    },
    buckets: sortRows,
    tasks: subredditPosts,
    completedTasks: function() {
      return loadListing('saved');
    },
    taskDetail: loadComments,
    createTask: function() {
      return Promise.reject(new Error('Creating posts is not supported'));
    },
    createBucket: function() {
      return Promise.reject(new Error('Creating subreddits is not supported'));
    },
    addTaskNote: function(postToken, text) {
      return replyThing(postToken, null, text);
    },
    addChecklistItem: function(postToken, text) {
      return replyThing(postToken, null, text);
    },
    setTaskDue: function() {
      return Promise.resolve();
    },
    completeTask: function(postToken) {
      return voteThing(postToken, 1);
    },
    uncompleteTask: function(postToken) {
      return voteThing(postToken, 0);
    },
    deleteTask: function() {
      return Promise.reject(new Error('Deleting posts is not supported'));
    },
    plannerWrite: function(postToken, command, text, messageId) {
      if (command === 'add_task_note' || command === 'add_checklist_item') {
        return replyThing(postToken, messageId, text);
      }
      return Promise.resolve();
    },
    assignTeams: function() { return Promise.resolve([]); },
    assignConfiguredContacts: function() { return Promise.resolve([]); },
    assignTeamMembers: function() { return Promise.resolve([]); },
    toggleChecklist: function() { return Promise.resolve(); },
    messages: loadComments,
    olderMessages: function() { return Promise.resolve([]); },
    newerMessages: function() { return Promise.resolve([]); },
    message: function(postToken, messageToken) {
      var rows = commentsByPost[postToken] || [];
      var found = rows.filter(function(row) { return row.id === messageToken; })[0];
      return Promise.resolve(clone(found || {}));
    },
    sendMessage: function(postToken, text, replyTo) {
      return actionGuard(function() {
        return replyThing(postToken, replyTo, text);
      });
    },
    editMessage: function(postToken, messageToken, text) {
      return actionGuard(function() {
        return editThing(messageToken, text).then(function() {
          return loadComments(postToken);
        });
      });
    },
    deleteMessage: function() {
      return Promise.reject(new Error('Deleting comments is not supported yet'));
    },
    sendReaction: function(postToken, messageToken, token) {
      return actionGuard(function() {
        var target = messageToken || postToken;
        if (token === 'like') return voteThing(target, 1);
        if (token === 'angry') return voteThing(target, -1);
        if (token === 'remove') return voteThing(target, 0);
        if (token === 'heart') return saveThing(target);
        return Promise.resolve();
      });
    },
    markRead: function() { return Promise.resolve(); },
    archiveChat: function(chatToken) {
      return saveThing(chatToken);
    },
    markUnread: function(chatToken) {
      return voteThing(chatToken, 0);
    },
    imageBytes: function(postToken, messageToken, width, height, colors, maxBytes, maxPixels, retryLevel, maxCost, forceTall, statusCb) {
      var token = messageToken || postToken;
      var url = tokenMaps.url[token] || tokenMaps.url[postToken];
      if (!url || setting('mediaMode', 'selected') === 'off') {
        return Promise.reject(new Error('No selected media'));
      }
      return image.graphImageBytes(postToken, token, function() {
        return fetch(url).then(function(response) {
          if (!response.ok) throw new Error('image download failed');
          return response.arrayBuffer();
        }).then(function(buffer) {
          return new Uint8Array(buffer);
        });
      }, width, height, colors, maxBytes, maxPixels, retryLevel, maxCost, forceTall, statusCb);
    },
    avatarBytes: function(chatToken, width, height, colors, maxBytes) {
      var url = tokenMaps.url[chatToken];
      if (!url || setting('mediaMode', 'selected') === 'off') {
        return Promise.reject(new Error('No thumbnail'));
      }
      return image.graphAvatarBytes(chatToken, function() {
        return fetch(url).then(function(response) {
          if (!response.ok) throw new Error('thumbnail download failed');
          return response.arrayBuffer();
        }).then(function(buffer) {
          return new Uint8Array(buffer);
        });
      }, width, height, colors, maxBytes);
    },
    cancelImageRequests: image.cancelImageRequests
  };
}

module.exports = {
  create: create
};
