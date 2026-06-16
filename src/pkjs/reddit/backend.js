var image = require('./image');

var SETTINGS_PREFIX = 'pebbit.';
var REDDIT_AUTH = 'https://www.reddit.com/api/v1/authorize';
var REDDIT_TOKEN = 'https://www.reddit.com/api/v1/access_token';
var REDDIT_API = 'https://oauth.reddit.com';
var DEFAULT_SCOPES = 'identity read mysubreddits vote save submit edit';
var LOCAL_REDIRECT_URI = 'http://localhost:65010/pebbit';
var DEFAULT_REDIRECT_URI = 'https://tombolger.github.io/Pebbit/reddit-callback.html';
var DEFAULT_SUBREDDITS = 'pebble|pebblewatch|programming|technology|AskReddit';
var DEFAULT_USER_AGENT = 'Pebbit/0.1 PebbleOS Reddit client';
var MAX_CACHE_MS = 5 * 60 * 1000;
var MAX_VISIBLE_REPLIES_PER_COMMENT = 2;
var MAX_COMMENT_ROWS = 40;
var MAX_LISTING_ROWS = 12;
var PINNED_MENU_ID = '__pin';
var SUBSCRIPTIONS_MENU_ID = '__subs';

var tokenMaps = {
  post: {},
  comment: {},
  subreddit: {},
  sort: {},
  url: {},
  media: {}
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
var hiddenRepliesByPost = {};
var fullTextById = {};
var savedByToken = {};
var recentReplyQuotes = {};
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

function watchBubbleText(value, maxChars) {
  var text = plainText(value).replace(/\s+/g, ' ').trim();
  maxChars = maxChars || 120;
  if (text.length <= maxChars) {
    return text;
  }
  return text.substring(0, Math.max(0, maxChars - 4)).replace(/\s+\S*$/, '') + ' ...';
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

function countValue(value) {
  var n = Number(value || 0);
  return isFinite(n) && n > 0 ? n : 0;
}

function scoreText(thing) {
  thing = thing || {};
  var ups = countValue(thing.ups);
  var score = Number(thing.score || 0);
  if (!ups && score > 0) {
    ups = score;
  }
  if (ups) {
    return '👍 ' + compactNumber(ups);
  }
  if (score) {
    return '👍 ' + compactNumber(score);
  }
  return '';
}

function awardText(thing) {
  thing = thing || {};
  var awards = thing.all_awardings || [];
  var total = countValue(thing.total_awards_received);
  if (!total && awards.length) {
    awards.forEach(function(award) {
      total += countValue(award && award.count) || 1;
    });
  }
  if (!total) {
    return '';
  }
  return '🎉 ' + compactNumber(total);
}

function postMeta(post) {
  var parts = [];
  var score = scoreText(post);
  if (score) {
    parts.push(score);
  }
  if (post && post.num_comments !== undefined) {
    parts.push(compactNumber(post.num_comments || 0) + 'c');
  }
  if (post && post.saved) {
    parts.push('saved');
  }
  return parts.join(' ');
}

function sideMeta(thing) {
  var parts = [];
  var awards = awardText(thing);
  if (awards) {
    parts.push(awards);
  }
  parts.push(ageText(thing && thing.created_utc));
  return parts.join(' ');
}

function commentMeta(comment) {
  var parts = [];
  var score = scoreText(comment);
  if (score) {
    parts.push(score);
  }
  if (comment && comment.edited && comment.edited !== false) {
    parts.push('edited');
  }
  if (comment && comment.saved) {
    parts.push('saved');
  }
  return parts.join(' ');
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

function userAgent() {
  return setting('userAgent', DEFAULT_USER_AGENT);
}

function authHeaders() {
  var clientId = setting('clientId', '');
  return {
    Authorization: 'Basic ' + btoa(clientId + ':'),
    'Content-Type': 'application/x-www-form-urlencoded',
    'User-Agent': userAgent()
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
  return ensureRealToken();
}

function ensureRealToken() {
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

function redditRequest(path, options, tokenLoader) {
  options = options || {};
  return tokenLoader().then(function(token) {
    var headers = {
      Authorization: 'bearer ' + token,
      'User-Agent': userAgent()
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

function reddit(path, options) {
  return redditRequest(path, options, ensureToken);
}

function redditReal(path, options) {
  return redditRequest(path, options, ensureRealToken);
}

function imageBytesFromUrl(url, label) {
  if (typeof fetch === 'function') {
    return fetch(url).then(function(response) {
      if (!response.ok) {
        throw new Error(label + ' download failed');
      }
      return response.arrayBuffer();
    }).then(function(buffer) {
      return new Uint8Array(buffer);
    });
  }
  return new Promise(function(resolve, reject) {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', url, true);
    xhr.responseType = 'arraybuffer';
    xhr.onload = function() {
      if ((xhr.status >= 200 && xhr.status < 300) || xhr.status === 0) {
        resolve(new Uint8Array(xhr.response));
      } else {
        reject(new Error(label + ' download failed'));
      }
    };
    xhr.onerror = function() {
      reject(new Error(label + ' download failed'));
    };
    xhr.send();
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
      url: 'https://picsum.photos/480/320.jpg',
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
  var parts = ['r/' + post.subreddit, 'u/' + post.author, postMeta(post)];
  if (post.domain && post.domain.indexOf('self.') !== 0) {
    parts.push(post.domain);
  }
  return parts.join(' - ');
}

function normalizePost(data) {
  var post = data && data.data ? data.data : data;
  var id = post.name || post.id || '';
  var token = tokenFor('post', id);
  var media = bestMedia(post);
  var row = {
    id: token,
    title: plainText(post.title || 'Untitled'),
    preview: postPreview(post),
    kind: 'post',
    section: media ? 'media' : 'post',
    unread: post.likes !== null && post.likes !== undefined,
    unread_count: post.saved ? 1 : 0
  };
  postCache[token] = clone(post);
  savedByToken[token] = !!post.saved;
  fullTextById[token] = plainText(post.selftext || post.title || '');
  if (media) {
    tokenMaps.url[token] = media.url;
    tokenMaps.media[token] = media;
  }
  return row;
}

function validImageUrl(url) {
  return !!url && /^https?:\/\//i.test(url) && /\.(png|jpe?g|webp|gif)(\?|$)/i.test(url);
}

function hostName(url) {
  var match = String(url || '').match(/^https?:\/\/([^\/?#]+)/i);
  return match ? match[1].toLowerCase() : '';
}

function isRedditMediaHost(url) {
  var host = hostName(url);
  return /(^|\.)redd\.it$/.test(host) || /(^|\.)redditmedia\.com$/.test(host);
}

function imgurThumbnailUrl(url) {
  var value = decodeHtml(url || '');
  var match;
  if (!/^https?:\/\//i.test(value)) {
    return '';
  }
  match = value.match(/^https?:\/\/i\.imgur\.com\/([A-Za-z0-9]+)([a-z])?\.(?:gifv|mp4|png|jpe?g|webp|gif)(?:[?#].*)?$/i);
  if (match) {
    return 'https://i.imgur.com/' + match[1] + 'h.jpg';
  }
  match = value.match(/^https?:\/\/(?:www\.)?imgur\.com\/(?:r\/[^\/?#]+\/)?([A-Za-z0-9]+)(?:[?#].*)?$/i);
  if (match) {
    return 'https://i.imgur.com/' + match[1] + 'm.jpg';
  }
  return '';
}

function mediaCandidate(url, width, height) {
  url = decodeHtml(url || '');
  return url ? {
    url: url,
    width: countValue(width) || 320,
    height: countValue(height) || 220
  } : null;
}

function pushMediaCandidate(candidates, candidate) {
  if (!candidate || !candidate.url) {
    return;
  }
  for (var i = 0; i < candidates.length; i += 1) {
    if (candidates[i].url === candidate.url) {
      return;
    }
  }
  candidates.push(candidate);
}

function watchPreviewCandidate(items, source) {
  var all = (items || []).slice();
  if (source) {
    all.push(source);
  }
  var best = null;
  var bestScore = 2147483647;
  for (var i = 0; i < all.length; i += 1) {
    var item = all[i];
    var url = item && (item.url || item.u);
    var width = countValue(item && (item.width || item.x));
    var height = countValue(item && (item.height || item.y));
    var score;
    if (!url || !width || !height) {
      continue;
    }
    score = width >= 176 ? (width * height) : ((176 - width) * 1000000) + (width * height);
    if (!best || score < bestScore) {
      best = item;
      bestScore = score;
    }
  }
  return best || source || (items && items.length ? items[items.length - 1] : null);
}

function redditPreviewMedia(post) {
  var images = post && post.preview && post.preview.images;
  if (!images || !images.length) {
    return null;
  }
  var image = images[0];
  var resolutions = image.resolutions || [];
  var candidate = watchPreviewCandidate(resolutions, image.source);
  return mediaCandidate((candidate && candidate.url) || (image.source && image.source.url) || '',
                        (candidate && candidate.width) || (image.source && image.source.width),
                        (candidate && candidate.height) || (image.source && image.source.height));
}

function redditGalleryMedia(post) {
  var metadata = post && post.media_metadata;
  var gallery = post && post.gallery_data && post.gallery_data.items;
  var item = gallery && gallery.length ? gallery[0] : null;
  var media = item && metadata ? metadata[item.media_id] : null;
  var previews = media && media.p;
  var candidate = watchPreviewCandidate(previews, media && media.s);
  return mediaCandidate((candidate && (candidate.u || candidate.url)) || '',
                        (candidate && (candidate.x || candidate.width)),
                        (candidate && (candidate.y || candidate.height)));
}

function mediaEmbedThumbnail(media) {
  if (media && media.oembed && validImageUrl(media.oembed.thumbnail_url)) {
    return decodeHtml(media.oembed.thumbnail_url);
  }
  return '';
}

function bestMedia(post) {
  if (!post) {
    return null;
  }
  var url = decodeHtml(post.url_overridden_by_dest || post.url || '');
  var preview = redditPreviewMedia(post);
  var galleryPreview = redditGalleryMedia(post);
  var thumbnail = decodeHtml(post.thumbnail || '');
  var candidates = [];
  pushMediaCandidate(candidates, preview);
  pushMediaCandidate(candidates, galleryPreview);
  pushMediaCandidate(candidates, mediaCandidate(thumbnail, post.thumbnail_width, post.thumbnail_height));
  pushMediaCandidate(candidates, mediaCandidate(mediaEmbedThumbnail(post.secure_media), 320, 220));
  pushMediaCandidate(candidates, mediaCandidate(mediaEmbedThumbnail(post.media), 320, 220));
  pushMediaCandidate(candidates, mediaCandidate(imgurThumbnailUrl(url), 320, 320));
  pushMediaCandidate(candidates, mediaCandidate(url, 320, 220));
  for (var i = 0; i < candidates.length; i += 1) {
    if (validImageUrl(candidates[i].url) &&
        (isRedditMediaHost(candidates[i].url) || hostName(candidates[i].url).indexOf('imgur.com') >= 0 ||
         candidates[i].url === url || candidates[i].url === thumbnail ||
         (preview && candidates[i].url === preview.url))) {
      return candidates[i];
    }
  }
  return null;
}

function normalizeComment(data, parentById) {
  var comment = data && data.data ? data.data : data;
  var id = comment.name || comment.id || '';
  var token = tokenFor(comment.kind === 'more' || id.indexOf('more_') === 0 ? 'comment' : 'comment', id);
  var depth = parseInt(comment.depth || 0, 10);
  if (!isFinite(depth) || depth < 0) {
    depth = 0;
  }
  depth = Math.min(9, depth);
  var moreCount = comment.more_children ? comment.more_children.length : countValue(comment.count);
  var text = comment.kind === 'more' ?
    ('View more' + (moreCount ? ' (' + compactNumber(moreCount) + ')' : '')) :
    plainText(comment.body || comment.body_html || '[deleted]');
  var visibleText = comment.kind === 'more' || comment.kind === 'summary' ?
    watchBubbleText(text, 42) :
    watchBubbleText(text, 170);
  var meta = comment.kind === 'summary' ? '' : commentMeta(comment);
  var displayDepth = Math.min(2, depth);
  var row = {
    id: token,
    sender: comment.kind === 'more' || comment.kind === 'summary' ? 'More' : ('u/' + (comment.author || '[deleted]')),
    text: visibleText,
    meta: meta,
    reactions: comment.kind === 'summary' ? '' : sideMeta(comment),
    outgoing: !!comment.outgoing,
    section: comment.kind === 'summary' ? 'collapsed' :
      (comment.kind === 'more' ? 'more' : ('d' + displayDepth))
  };
  if (comment.kind === 'summary' && parentById.__postToken && comment.hidden_children && comment.hidden_children.length) {
    hiddenRepliesByPost[parentById.__postToken] = hiddenRepliesByPost[parentById.__postToken] || {};
    hiddenRepliesByPost[parentById.__postToken][token] = {
      depth: depth,
      children: comment.hidden_children
    };
  }
  if (comment.kind === 'more' && parentById.__postToken && comment.more_children && comment.more_children.length) {
    hiddenRepliesByPost[parentById.__postToken] = hiddenRepliesByPost[parentById.__postToken] || {};
    hiddenRepliesByPost[parentById.__postToken][token] = {
      depth: depth,
      moreChildren: comment.more_children,
      parentId: comment.parent_id || ''
    };
  }
  savedByToken[token] = !!comment.saved;
  fullTextById[token] = text;
  return row;
}

function countCommentTree(children) {
  var count = 0;
  (children || []).forEach(function(child) {
    if (!child || !child.data) {
      return;
    }
    count += 1;
    if (child.kind !== 'more' && child.data.replies && child.data.replies.data) {
      count += countCommentTree(child.data.replies.data.children);
    }
  });
  return count;
}

function summaryComment(parentId, depth, hiddenCount, hiddenChildren) {
  return {
    kind: 'summary',
    id: 'summary_' + parentId + '_' + hiddenCount,
    name: 'summary_' + parentId + '_' + hiddenCount,
    parent_id: parentId,
    author: 'More',
    body: 'View more (' + compactNumber(hiddenCount) + ')',
    hidden_children: hiddenChildren || [],
    depth: depth,
    created_utc: nowSeconds()
  };
}

function flattenComments(children, depth, rows, parentById) {
  (children || []).forEach(function(child) {
    if (!child || !child.data) {
      return;
    }
    if (child.kind === 'more') {
      var moreChildren = child.data.children || [];
      rows.push({
        kind: 'more',
        id: 'more_' + (child.data.id || rows.length),
        name: 'more_' + (child.data.id || rows.length),
        parent_id: child.data.parent_id,
        author: 'More',
        body: 'View more' + (moreChildren.length ? ' (' + compactNumber(moreChildren.length) + ')' : ''),
        count: child.data.count || moreChildren.length,
        more_children: moreChildren,
        depth: depth,
        created_utc: nowSeconds()
      });
      return;
    }
    child.data.depth = depth;
    parentById[child.data.name] = child.data;
    parentById[child.data.id] = child.data;
    rows.push(child.data);
    if (child.data.replies && child.data.replies.data) {
      var replies = child.data.replies.data.children || [];
      var visible = replies;
      var hidden = 0;
      var hiddenChildren = [];
      var limit = depth === 0 ? MAX_VISIBLE_REPLIES_PER_COMMENT : 1;
      if (depth >= 2) {
        hidden = countCommentTree(replies);
        hiddenChildren = replies;
        visible = [];
      } else if (replies.length > limit) {
        visible = replies.slice(0, limit);
        hiddenChildren = replies.slice(limit);
        hidden = countCommentTree(hiddenChildren);
      }
      flattenComments(visible, depth + 1, rows, parentById);
      if (hidden > 0) {
        rows.push(summaryComment(child.data.name || child.data.id, depth + 1, hidden, hiddenChildren));
      }
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
  return path + '?limit=' + MAX_LISTING_ROWS + (after ? '&after=' + encodeURIComponent(after) : '');
}

function loadListing(feed) {
  if (isMockMode()) {
    var rows = mockPosts(feed).slice(0, MAX_LISTING_ROWS).map(normalizePost);
    postRowsByFeed[feed] = rows;
    return Promise.resolve(rows);
  }
  return reddit(listingPath(feed)).then(function(data) {
    var rows = (((data || {}).data || {}).children || [])
      .filter(function(child) {
        return child && child.kind === 't3' && child.data;
      })
      .slice(0, MAX_LISTING_ROWS)
      .map(normalizePost);
    postRowsByFeed[feed] = rows;
    return rows;
  });
}

function loadComments(postToken) {
  var post = postCache[postToken] || {};
  var real = realId('post', postToken);
  if (!postCache[postToken] || real.indexOf('t3_') !== 0) {
    return Promise.reject(new Error('Post expired. Refresh posts.'));
  }
  if (isMockMode()) {
    var comments = (mockComments[real] || mockComments[post.id] || []).slice();
    var parentById = {};
    parentById[real] = post;
    parentById.__postToken = postToken;
    parentById.__recentReplyQuotes = recentReplyQuotes[postToken] || {};
    comments.forEach(function(row) {
      parentById[row.id] = row;
      parentById[row.name] = row;
    });
    var rows = [postMessage(postToken)].concat(comments.map(function(row) {
      return normalizeComment(row, parentById);
    }));
    rows.forEach(function(row, index) { row.sort_ts = index; });
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
    parentById.__postToken = postToken;
    parentById.__recentReplyQuotes = recentReplyQuotes[postToken] || {};
    hiddenRepliesByPost[postToken] = {};
    flattenComments(listing, 0, flat, parentById);
    var rows = [postMessage(postToken)].concat(flat.slice(0, MAX_COMMENT_ROWS).map(function(row) {
      return normalizeComment(row, parentById);
    }));
    rows.forEach(function(row, index) { row.sort_ts = index; });
    commentsByPost[postToken] = rows;
    return rows;
  });
}

function postMessage(postToken) {
  var post = postCache[postToken] || {};
  var media = tokenMaps.media[postToken] || null;
  var text = plainText(post.selftext || post.title || '');
  if (!text && post.url) {
    text = post.url;
  }
  fullTextById[postToken] = text || '[link post]';
  return {
    id: postToken,
    sender: 'r/' + (post.subreddit || 'reddit'),
    text: watchBubbleText(text || '[link post]', 170),
    meta: postMeta(post),
    reactions: sideMeta(post),
    outgoing: false,
    section: 'post',
    image_token: tokenMaps.url[postToken] ? postToken : '',
    image_width: media ? media.width : 0,
    image_height: media ? media.height : 0
  };
}

function expandReplies(postToken, markerToken) {
  var rows = commentsByPost[postToken] || [];
  var hidden = hiddenRepliesByPost[postToken] && hiddenRepliesByPost[postToken][markerToken];
  var index = -1;
  for (var i = 0; i < rows.length; i += 1) {
    if (String(rows[i].id) === String(markerToken || '')) {
      index = i;
      break;
    }
  }
  if (index < 0 || !hidden) {
    return Promise.resolve({
      rows: rows,
      anchorId: markerToken,
      changed: false
    });
  }
  if (hidden.moreChildren && hidden.moreChildren.length && !hidden.children) {
    return loadMoreChildren(postToken, markerToken, hidden, index);
  }
  if (!hidden.children || !hidden.children.length) {
    return Promise.resolve({
      rows: rows,
      anchorId: markerToken,
      changed: false
    });
  }
  var flat = [];
  var parentById = {};
  var post = postCache[postToken] || {};
  parentById[realId('post', postToken)] = post;
  parentById.__postToken = postToken;
  parentById.__recentReplyQuotes = recentReplyQuotes[postToken] || {};
  flattenComments(hidden.children, hidden.depth, flat, parentById);
  var insert = flat.slice(0, 6).map(function(row) {
    return normalizeComment(row, parentById);
  });
  if (!insert.length) {
    return Promise.resolve({
      rows: rows,
      anchorId: markerToken,
      changed: false
    });
  }
  delete hiddenRepliesByPost[postToken][markerToken];
  rows = rows.slice(0, index).concat(insert).concat(rows.slice(index + 1));
  rows.forEach(function(row, rowIndex) { row.sort_ts = rowIndex; });
  commentsByPost[postToken] = rows;
  return Promise.resolve({
    rows: rows,
    anchorId: insert[0].id,
    changed: true
  });
}

function loadMoreChildren(postToken, markerToken, hidden, index) {
  if (isMockMode()) {
    return Promise.resolve({
      rows: commentsByPost[postToken] || [],
      anchorId: markerToken,
      changed: false
    });
  }
  var post = postCache[postToken] || {};
  var postId = realId('post', postToken);
  var children = (hidden.moreChildren || []).slice(0, 20).join(',');
  if (!postId || !children) {
    return Promise.resolve({
      rows: commentsByPost[postToken] || [],
      anchorId: markerToken,
      changed: false
    });
  }
  return reddit('/api/morechildren', {
    method: 'POST',
    body: {
      api_type: 'json',
      link_id: postId,
      children: children,
      sort: setting('commentSort', 'confidence')
    }
  }).then(function(data) {
    var things = (((data || {}).json || {}).data || {}).things || [];
    var flat = [];
    var parentById = {};
    var rows = commentsByPost[postToken] || [];
    parentById[postId] = post;
    parentById.__postToken = postToken;
    parentById.__recentReplyQuotes = recentReplyQuotes[postToken] || {};
    flattenComments(things, hidden.depth, flat, parentById);
    var insert = flat.slice(0, 12).map(function(row) {
      return normalizeComment(row, parentById);
    });
    if (!insert.length) {
      return {
        rows: rows,
        anchorId: markerToken,
        changed: false
      };
    }
    delete hiddenRepliesByPost[postToken][markerToken];
    rows = rows.slice(0, index).concat(insert).concat(rows.slice(index + 1));
    rows.forEach(function(row, rowIndex) { row.sort_ts = rowIndex; });
    commentsByPost[postToken] = rows;
    return {
      rows: rows,
      anchorId: insert[0].id,
      changed: true
    };
  });
}

function navRows() {
  return [
    {id: '__feed_front', title: 'Front Page', preview: 'Best posts from your Reddit home', kind: 'feed'},
    {id: '__feed_hot', title: 'Hot', preview: 'Posts rising across Reddit', kind: 'feed'},
    {id: '__feed_trending', title: 'Trending', preview: 'Rising posts', kind: 'feed'},
    {id: '__feed_saved', title: 'Saved', preview: 'Saved posts and comments', kind: 'feed'},
    {id: PINNED_MENU_ID, title: 'Pinned', preview: 'Pinned communities', kind: 'feed'},
    {id: SUBSCRIPTIONS_MENU_ID, title: 'Subscriptions', preview: 'Communities from your Reddit account', kind: 'feed'}
  ];
}

function pinnedMenuId(value) {
  return value === PINNED_MENU_ID ||
    value === '__submenu_subreddits' ||
    value === '__submenu_subreddit';
}

function subscriptionsMenuId(value) {
  return value === SUBSCRIPTIONS_MENU_ID ||
    value === '__submenu_subscriptions' ||
    value === '__submenu_subscript';
}

function communityMenuId(value) {
  return pinnedMenuId(value) || subscriptionsMenuId(value);
}

function hiddenSubredditMap() {
  var hidden = {};
  splitList(setting('hiddenSubreddits', '')).forEach(function(name) {
    hidden[name.toLowerCase()] = true;
  });
  return hidden;
}

function pinnedCommunityFeedRows() {
  var hidden = hiddenSubredditMap();
  return splitList(setting('subreddits', DEFAULT_SUBREDDITS)).filter(function(name) {
    return !hidden[name.toLowerCase()];
  }).map(function(name) {
    return {
      id: '__feed_r/' + name,
      title: 'r/' + name,
      preview: 'Hot posts from r/' + name,
      kind: 'feed',
      section: 'feed'
    };
  });
}

function subscriptionRows() {
  if (!authConfigured()) {
    return Promise.reject(new Error('Sign in to load subscriptions'));
  }
  return redditReal('/subreddits/mine/subscriber?limit=100').then(function(data) {
    var hidden = hiddenSubredditMap();
    return ((((data || {}).data || {}).children || [])).map(function(child) {
      var sub = child && child.data ? child.data : {};
      var name = sub.display_name_prefixed || (sub.display_name ? 'r/' + sub.display_name : '');
      var plain = name.replace(/^r\//i, '');
      if (!plain || hidden[plain.toLowerCase()]) {
        return null;
      }
      return {
        id: '__feed_r/' + plain,
        title: 'r/' + plain,
        preview: sub.public_description || sub.title || ('Hot posts from r/' + plain),
        kind: 'feed',
        section: 'feed'
      };
    }).filter(function(row) {
      return !!row;
    }).slice(0, MAX_LISTING_ROWS);
  });
}

function subredditMenuRows(kind) {
  if (subscriptionsMenuId(kind)) {
    return subscriptionRows();
  }
  return Promise.resolve(pinnedCommunityFeedRows());
}

function pinnedCommunityRows() {
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
  var userAgentText = escapeHtml(setting('userAgent', DEFAULT_USER_AGENT));
  var signedIn = !!setting('refreshToken', '');
  var mockChecked = mockModeEnabled() ? ' checked' : '';
  var nsfwChecked = setting('showNsfw', '') === '1' ? ' checked' : '';
  var media = escapeHtml(setting('mediaMode', 'selected'));
  var auth = authConfigured() ? authUrl() : '';
  var html = '<!doctype html><html><head><meta charset=utf-8><meta name=viewport content="width=device-width,initial-scale=1">' +
    '<title>Pebbit Settings</title><style>body{margin:0;font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif;background:#f4f6f8;color:#17202a}main{max-width:620px;margin:auto;padding:18px}h1{margin:0 0 6px;font-size:26px}.lede,.h{color:#607080;font-size:14px;line-height:1.4}label{display:block;margin-top:15px;font-weight:700}input,textarea,select{box-sizing:border-box;width:100%;border:1px solid #c9d2dc;border-radius:6px;padding:11px;font:inherit;background:white}textarea{min-height:84px}.panel{margin:14px 0;padding:12px;border:1px solid #d6dde5;border-radius:6px;background:white}.row{display:flex;gap:10px}button,.btn{display:block;box-sizing:border-box;text-align:center;border:0;border-radius:6px;background:#ff4500;color:white;padding:12px;margin-top:12px;font:inherit;font-weight:800;text-decoration:none}.secondary{background:#e5edf3;color:#17202a}.danger{background:#334155;color:white}.toggle{display:flex;gap:8px;align-items:center}.toggle input{width:auto}.savebar{position:sticky;bottom:0;background:#f4f6f8;padding:10px 0}</style></head><body><main>' +
    '<h1>Pebbit</h1><p class=lede>Reddit client settings. Each user should use their own Reddit installed-app client ID. No post creation is exposed in this app.</p>' +
    '<div class=panel><b>Status:</b> ' + (signedIn ? 'Reddit refresh token saved.' : 'Not signed in.') +
    '<p class=h>Create an installed app at Reddit, copy its client ID here, and set its redirect URI to the exact value below. Installed apps do not use a client secret.</p>' +
    '<p class=h>Save, reopen this page, sign in, then paste either the code or the full redirected URL. Pebbit ignores trailing #_ automatically.</p>' +
    '<p class=h>Hosted callback: ' + escapeHtml(DEFAULT_REDIRECT_URI) + '<br>Local fallback: ' + escapeHtml(LOCAL_REDIRECT_URI) + '</p></div>' +
    '<label>Reddit app client ID</label><input id=clientId value="' + clientId + '" placeholder="Installed app client ID">' +
    '<label>Redirect URI</label><input id=redirectUri value="' + redirectUri + '">' +
    '<label>Authorization code</label><input id=code placeholder="Paste code or full redirected URL">' +
    '<div class=h>Paste the code value from the redirect URL. If the copied text ends in #_, leave it in or remove it; Pebbit strips it before signing in.</div>' +
    (auth ? '<a class=btn href="' + escapeHtml(auth) + '">Sign in with Reddit</a>' : '') +
    '<label>Default feed</label><select id=defaultFeed><option value=front>Front Page</option><option value=hot>Hot</option><option value=trending>Trending</option><option value=saved>Saved</option></select>' +
    '<label>Comment sort</label><select id=commentSort><option value=confidence>Best</option><option value=top>Top</option><option value=new>New</option><option value=controversial>Controversial</option><option value=old>Old</option></select>' +
    '<label>Pinned Communities</label><textarea id=subreddits>' + subreddits + '</textarea>' +
    '<label>Hidden subreddits</label><textarea id=hiddenSubreddits>' + hidden + '</textarea>' +
    '<label>Media loading</label><select id=mediaMode><option value=off>Off</option><option value=selected>Selected only</option><option value=nearby>Near viewport</option></select>' +
    '<label>Canned replies</label><textarea id=cannedReplies>' + replies.replace(/\|/g, '\n') + '</textarea>' +
    '<label class=toggle><input id=showNsfw type=checkbox' + nsfwChecked + '> Show NSFW rows</label>' +
    '<label class=toggle><input id=mockMode type=checkbox' + mockChecked + '> Use mock data</label>' +
    '<details><summary>Advanced</summary><label>OAuth scopes</label><input id=scopes value="' + scopes + '"><label>User-Agent</label><input id=userAgent value="' + userAgentText + '"><div class=h>Keep this descriptive if you publish a fork or custom build.</div></details>' +
    '<div class=savebar><button id=save>Save</button><button id=signout class=secondary>Sign out</button></div>' +
    '<script>function v(id){return document.getElementById(id)}function lines(id){return v(id).value.split(/\\n+/).map(function(x){return x.trim()}).filter(Boolean).join("|")}function cleanCode(x){x=String(x||"").trim();var m=x.match(/[?&]code=([^&#]*)/);if(m)x=m[1];else if(x.indexOf("code=")===0)x=x.substring(5);return x.split("&")[0].split("#")[0].trim()}v("defaultFeed").value="' + escapeHtml(setting('defaultFeed', 'front')) + '";v("commentSort").value="' + escapeHtml(setting('commentSort', 'confidence')) + '";v("mediaMode").value="' + media + '";function close(a){var d={mode:"reddit",action:a,clientId:v("clientId").value.trim(),redirectUri:v("redirectUri").value.trim(),code:cleanCode(v("code").value),defaultFeed:v("defaultFeed").value,commentSort:v("commentSort").value,subreddits:lines("subreddits"),hiddenSubreddits:lines("hiddenSubreddits"),mediaMode:v("mediaMode").value,cannedReplies:lines("cannedReplies"),showNsfw:v("showNsfw").checked?"1":"",mockMode:v("mockMode").checked?"1":"0",scopes:v("scopes").value.trim(),userAgent:v("userAgent").value.trim()};location="pebblejs://close#"+encodeURIComponent(JSON.stringify(d))}v("save").onclick=function(){close("save")};v("signout").onclick=function(){close("signout")}</script>' +
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
  return setSaveThing(token, null);
}

function setSaveThing(token, shouldSave) {
  var id = realId('post', token) || realId('comment', token);
  if (isMockMode()) {
    savedByToken[token] = shouldSave === null || shouldSave === undefined ? !savedByToken[token] : !!shouldSave;
    if (postCache[token]) postCache[token].saved = savedByToken[token];
    return Promise.resolve();
  }
  var post = postCache[token];
  var saved = savedByToken[token] !== undefined ? savedByToken[token] : (post && !!post.saved);
  var targetSaved = shouldSave === null || shouldSave === undefined ? !saved : !!shouldSave;
  if (targetSaved === saved) {
    return Promise.resolve();
  }
  return reddit(targetSaved ? '/api/save' : '/api/unsave', {method: 'POST', body: {id: id}}).then(function(result) {
    savedByToken[token] = targetSaved;
    if (post) {
      post.saved = savedByToken[token];
    }
    return result;
  });
}

function replyThing(postToken, parentToken, text) {
  var parent = parentToken ?
    (realId('comment', parentToken) || realId('post', parentToken)) :
    realId('post', postToken);
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
      outgoing: true,
      quote_parent: true
    });
    return Promise.resolve();
  }
  recentReplyQuotes[postToken] = recentReplyQuotes[postToken] || {};
  recentReplyQuotes[postToken][parent] = nowSeconds();
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
        saveSetting('userAgent', data.userAgent || DEFAULT_USER_AGENT);
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
      feedId = String(feedId || '');
      if (communityMenuId(feedId)) {
        status(subscriptionsMenuId(feedId) ? 'Fetching subscriptions...' : 'Fetching pinned...');
        return subredditMenuRows(feedId);
      }
      feedId = feedId.replace('__feed_', '') || setting('defaultFeed', 'front');
      status('Fetching posts...');
      return loadListing(feedId);
    },
    plans: pinnedCommunityRows,
    togglePlanPin: function() {
      return Promise.resolve('Community saved');
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
      return Promise.reject(new Error('Creating communities is not supported'));
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
    newerMessages: function(postToken, limit, afterId) {
      var rows = commentsByPost[postToken] || [];
      var found = -1;
      limit = Math.max(1, Math.min(Number(limit || 6), 12));
      for (var i = 0; i < rows.length; i += 1) {
        if (String(rows[i].id) === String(afterId || '')) {
          found = i;
          break;
        }
      }
      return Promise.resolve(found < 0 ? [] : rows.slice(found + 1, found + 1 + limit));
    },
    message: function(postToken, messageToken) {
      var rows = commentsByPost[postToken] || [];
      var found = rows.filter(function(row) { return row.id === messageToken; })[0];
      return Promise.resolve(clone(found || {}));
    },
    fullText: function(postToken, messageToken) {
      var token = messageToken || postToken;
      return Promise.resolve(fullTextById[token] || fullTextById[postToken] || '');
    },
    expandReplies: expandReplies,
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
        if (token === 'like' || token === 'upvote') return voteThing(target, 1);
        if (token === 'angry' || token === 'dislike' || token === 'downvote') return voteThing(target, -1);
        if (token === 'remove' || token === 'clear_vote') return voteThing(target, 0);
        if (token === 'heart' || token === 'save') return setSaveThing(target, true);
        if (token === 'unsave') return setSaveThing(target, false);
        return Promise.resolve();
      });
    },
    thingAction: function(postToken, messageToken, action) {
      return actionGuard(function() {
        var target = messageToken || postToken;
        if (action === 'upvote') return voteThing(target, 1);
        if (action === 'downvote') return voteThing(target, -1);
        if (action === 'clear_vote') return voteThing(target, 0);
        if (action === 'save') return setSaveThing(target, true);
        if (action === 'unsave') return setSaveThing(target, false);
        if (action === 'toggle_save') return saveThing(target);
        return Promise.reject(new Error('Action unavailable'));
      });
    },
    markRead: function() { return Promise.resolve(); },
    archiveChat: function(chatToken) {
      return setSaveThing(chatToken, true);
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
        return imageBytesFromUrl(url, 'image');
      }, width, height, colors, maxBytes, maxPixels, retryLevel, maxCost, forceTall, statusCb);
    },
    avatarBytes: function(chatToken, width, height, colors, maxBytes) {
      var url = tokenMaps.url[chatToken];
      if (!url || setting('mediaMode', 'selected') === 'off') {
        return Promise.reject(new Error('No thumbnail'));
      }
      return image.graphAvatarBytes(chatToken, function() {
        return imageBytesFromUrl(url, 'thumbnail');
      }, width, height, colors, maxBytes);
    },
    cancelImageRequests: image.cancelImageRequests
  };
}

module.exports = {
  create: create
};
