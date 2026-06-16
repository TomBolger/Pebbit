# Pebbit Plan

Pebbit is a personal PebbleOS client for Reddit. The first build should be a practical reader and comment/reply tool, seeded heavily from Pebblegram and the Planner surfaces in Pebble Teams. It should not support creating new posts yet.

### Plan execution

 The initial plan has been drafted. Now, think to optimize its execution.
 1. Analyze Dependencies: Critically review the list of tasks. 
 2. Group for Parallelism: Identify any tasks that are independent and can be executed 
 concurrently. Group them into a parallel stage. 
 3. Format for Parallel Execution: Place multiple <invoke name="Task"> calls inside a single 
 <function_calls> block in your response, using the optimal agent for each task. 
 4. Delegate Every Step: Even stages that have just one step should be delegated to an agent, 
 unless it is a trivial step. This avoids clogging your context window. And remember, no more than one task per agent.

 <example> 
 Assistant: I will now run [list of tasks] in parallel.
 <function_calls> 
 <invoke name="Task"> 
 <parameter name="description">First parallel task...</parameter> 
 <parameter name="prompt">Details for the first task...</parameter> 
 </invoke> 
 <invoke name="Task"> 
 <parameter name="description">Second parallel task...</parameter> 
 <parameter name="prompt">Details for the second task...</parameter> 
 </invoke> 
 </function_calls>
 I will now run [list of tasks] in parallel.
 ...more parallel tasks
 </example>

Please present your analysis of parallel stages and then proceed with the first stage. Remember, you MUST delegate steps to agents. Update the plan with progress after each phase of implementation. 

## Feasibility Estimate

Overall feasibility: high for a personal MVP, medium for a polished daily driver.

Estimated effort:
- Working mock UI seeded from existing code: 1 to 2 focused days.
- Authenticated Reddit listing/comment/reply MVP: 3 to 5 focused days after the seed build compiles.
- Comfortable personal-use build with media, subreddit filtering, comment paging, and action reliability: 1 to 2 weeks.

Main confidence boosters:
- Pebblegram already has the watch-side chat list, message bubbles, AppMessage protocol, action menus, dictation, Emery keyboard path, reply/context panels, metadata row, and lazy image transfer.
- Pebble Teams already proves that the same UI shell can be repurposed into planner/card lists and card-like detail rows.
- Reddit's listing/comment model maps cleanly onto the existing "chat list opens message thread" architecture.

Main risks:
- Reddit OAuth inside a Pebble settings webview needs a careful callback flow and token storage.
- Reddit comment trees can be deeper and larger than Pebble memory wants; Pebbit needs a flattened, windowed comment model from the start.
- Reddit media is varied: hosted images, galleries, thumbnails, external images, GIF/video previews, and self posts all need defensive handling.
- The local Pebble Teams plan records real instability around media pressure plus action/dictation lifecycle. Pebbit should keep media opt-in and serialize all menu/dictation/backend actions.
- Reddit API terms and limits can change. As of this planning pass, Reddit documents free Data API rate limits of 100 queries per minute per OAuth client ID for eligible free access, requires OAuth/login credentials, and blocks commercial use without permission.

## Sources Examined

Local:
- `/mnt/c/Users/tombo/Documents/Pebblegram/main-release`
- `/mnt/c/Users/tombo/Documents/Pebble Teams`

Web:
- https://github.com/TomBolger/Pebblegram
- https://www.reddit.com/dev/api/
- https://support.reddithelp.com/hc/en-us/articles/16160319875092-Reddit-Data-API-Wiki
- https://support.reddithelp.com/hc/en-us/articles/14945211791892-Developer-Platform-Accessing-Reddit-Data
- https://redditinc.com/policies/data-api-terms
- https://github.com/reddit-archive/reddit/wiki/oauth2

## Donor Code Map

Pebblegram:
- `src/c/Pebblegram.c`: base watch UI, chat list, comment/message bubbles, scrolling, action menu, dictation, keyboard, image transfer and decode.
- `src/pkjs/index.js`: AppMessage queue, payload streaming, message windowing, action router, settings lifecycle.
- `src/pkjs/pgjs/image.js`: image decode, resize, palette reduction, PNG/PBI preparation, retry budgets.
- `src/pkjs/config.html`: settings flow pattern to adapt or replace with Clay.
- `package.json`: Pebble app shape and message keys.

Pebble Teams:
- `src/c/Pebblegram.c`: Planner list modes, card-style rows, card detail bubbles, checklist row selection, safer custom action sheet patterns, interaction locking.
- `src/pkjs/index.js`: normalized list/detail payloads, mock backend pattern, list loaders, media transfer gates, long-running action routing.
- `plan.md`: hard-won notes about media pressure, dictation/menu lifecycle, and action serialization.

## Product Shape

Top-level watch navigation:
- Front Page
- Hot
- Rising or Popular as "Trending" unless a better Reddit endpoint is chosen
- Saved
- Subreddits

Feed views use the chat list:
- Row title: post title.
- Preview: subreddit, author, score, comment count, age, domain/flair.
- Unread dot can mean new/unseen or saved/upvoted state once local history exists.
- Long press opens post actions.

Subreddit browser uses Planner card styling:
- Cards for pinned subreddits.
- Subreddit rows can show display name, subscribers/active count if fetched, and a pinned/hidden marker.
- Clay settings controls which subreddits are pinned or hidden.

Post detail uses chat bubble/comment code:
- First bubble is the post body/media summary.
- Comments are flattened depth-first into selectable bubbles.
- Sender name is Reddit username.
- Metadata row shows score, age, edited marker, saved/vote state.
- Reply context can quote the parent comment or selected parent.
- Depth is shown with small horizontal indents and a vertical colored line by depth.

Actions:
- Post/comment: upvote, downvote, clear vote, save/unsave.
- Comment: reply with dictation/canned/keyboard, edit own comment, delete own comment if desired later.
- Post: open comments, load media, save/vote. No create-post action in v1.
- Thread: change sort, load more comments, refresh.

## Reddit API Plan

Auth:
- Use a user-provided Reddit app/client ID in settings for personal use.
- Prefer OAuth authorization-code flow through the Pebble settings page; request a permanent token and store the refresh token in phone-side local storage.
- Do not store a client secret in the PBW for installed-app style auth.
- Use `https://oauth.reddit.com` for bearer-token API calls.
- Use a descriptive User-Agent with app name, version, platform, and contact/user identifier.

Likely scopes:
- `identity` for account verification.
- `read` for listings and comments.
- `mysubreddits` for subscribed subreddit browsing, if used.
- `vote` for up/down/clear vote.
- `save` for save/unsave.
- `submit` for replying to posts/comments. Do not expose post creation even though the same scope can create comments.
- `edit` if editing own comments is included in v1.

Endpoints:
- Listings: `/best`, `/hot`, `/rising`, `/top`, `/new`, `/r/{subreddit}/{sort}`.
- Comments: `/r/{subreddit}/comments/{article}` with depth/limit/sort.
- More comments: `/api/morechildren`.
- Reply: `/api/comment`.
- Vote: `/api/vote`.
- Save/unsave: `/api/save`, `/api/unsave`.
- Edit comment: `/api/editusertext`.
- User/subreddit lists: `/subreddits/mine/subscriber`, `/subreddits/popular`, `/r/{subreddit}/about`.

Policy guardrails:
- Personal/non-commercial use is plausible under Reddit's free access model, but still requires Data API eligibility/sign-up and compliance.
- Respect rate-limit headers and back off.
- Avoid bulk export, long-term content hoarding, AI/model use, ads, paywalls, or redistribution.
- Keep cache short-lived; routinely expire stored Reddit content and handle deleted content.
- Do not use Reddit trademarks as the app name or imply endorsement. Pebbit is fine; use "for Reddit" attribution only where needed.

## Architecture

Initial project seed:
- Copy Pebblegram main-release into Pebbit as the base once coding starts.
- Rename app metadata, UUID, icons, text, storage keys, and source comments.
- Remove Telegram-specific code and dependencies.
- Keep the watch C filename only temporarily if renaming causes churn.

Phone-side modules:
- `src/pkjs/index.js`: Pebble event/router, AppMessage queue, current view state.
- `src/pkjs/reddit/auth.js`: OAuth URL builder, token exchange, refresh, logout.
- `src/pkjs/reddit/client.js`: authenticated fetch wrapper, rate-limit tracking, errors.
- `src/pkjs/reddit/listings.js`: front page, hot, rising/popular, subreddit listings.
- `src/pkjs/reddit/comments.js`: comment tree flattening, context, paging.
- `src/pkjs/reddit/actions.js`: vote, save, reply, edit.
- `src/pkjs/reddit/media.js`: URL detection plus Pebblegram image pipeline adapter.
- `src/pkjs/reddit/mock.js`: mock data for emulator and UI iteration.

Watch-side model:
- Reuse `Chat` for feed/subreddit rows.
- Reuse `Message` for posts/comments.
- Add minimal Reddit-specific fields only when existing keys cannot carry the data safely: depth, thing kind, vote state, saved state, locked/archived flags, and maybe subreddit/post fullname.
- Prefer sending normalized, watch-ready text from JS to C.

Message protocol:
- Keep AppMessage serialized; no parallel row streams, image chunks, avatars, and status spam.
- Reuse `chat`, `chats_done`, `messages_start`, `message`, `message_prepend`, `message_append`, `messages_done`, `image_start`, `image`, `image_done`, `image_error`.
- Add commands: `get_feed`, `get_subreddits`, `open_subreddit`, `get_comments`, `get_more_comments`, `reply_comment`, `edit_comment`, `vote_thing`, `save_thing`, `load_post_media`, `set_comment_sort`, `refresh`.

## Clay Settings

Settings should cover:
- Reddit client ID.
- OAuth sign in/out.
- Default feed at launch.
- Comment sort default.
- Front page mode: home/subscribed, best, popular, all.
- Subreddit browser visible list.
- Hide/mute subreddit list.
- NSFW visibility toggle, default off unless explicitly enabled.
- Media loading: off, selected only, near viewport.
- Canned replies.
- Debug/mock mode.

Implementation options:
- Use Clay if the project can add the dependency cleanly.
- If Clay becomes brittle in modern Pebble/Rebble tooling, preserve a Clay-shaped custom `config.html` and document the compromise.

## Phased Build Plan

Phase 0: Planning and seed hygiene
- Keep this plan and `agent.md` current.
- Decide whether to copy from Pebblegram local dirty state, GitHub main, or a clean local commit.
- Create a fresh UUID and app identity.

Phase 1: Mock Pebbit shell
- Copy/rename Pebblegram.
- Replace Telegram backend with mock Reddit backend.
- Show top-level feed rows.
- Open mock post detail and render nested comments with usernames, metadata, quotes, colors, indents, and vertical lines.
- Disable media by default but keep placeholders.

Phase 2: Reddit auth and read-only API
- Build settings login.
- Implement token refresh and rate-limit-aware fetch.
- Load front page/hot/rising/popular.
- Load subreddit list from settings and/or subscriptions.
- Open post comments with sort and depth/limit controls.

Phase 3: Actions
- Implement upvote/downvote/clear vote.
- Implement save/unsave.
- Implement dictation/canned/keyboard comment replies.
- Implement edit own comment if auth and endpoint behavior are stable.
- Reconcile changed comment/post state after action.

Phase 4: Media
- Reuse Pebblegram image preparation.
- Support selected post image only at first.
- Add galleries/thumbnails/still previews later.
- Cap loaded images to one or two and unload on navigation/action menus.

Phase 5: Daily-driver polish
- Comment paging with `/api/morechildren`.
- Better subreddit browser sorting and show/hide controls.
- Saved listing.
- Robust error states for deleted/locked/archived/removed content.
- Build PBW and test on emulator plus real watch.

## Acceptance Criteria for MVP

- Buildable Pebble app with Pebbit name/UUID.
- Settings can sign into Reddit and save subreddit visibility settings.
- Watch can browse front page/hot/rising or popular.
- Watch can browse configured subreddits.
- Opening a post shows post text plus comments as bubbles.
- User can upvote, downvote, clear vote, save, unsave, and reply to comments.
- No create-post path exists in watch UI or backend router.
- Media loads only after explicit selection and does not destabilize navigation/actions.

## Implementation Progress

2026-06-09:
- Delegated the required parallel review work from Plan Execution: one agent mapped reusable Pebblegram code, one mapped Pebble Teams Planner/card code, one checked Reddit backend/settings constraints, one reviewed watch-side stale Planner affordances, and one refreshed the fallback settings page.
- Seeded Pebbit from the local Pebble Teams codebase because it already contains the Pebblegram shell plus the Planner/card surfaces needed for subreddit browsing.
- Renamed the app identity to Pebbit with UUID `f54ed9a8-12db-4282-92c5-536f1190e73f`; kept `src/c/Pebblegram.c` temporarily to avoid unnecessary source churn.
- Added `src/pkjs/reddit/backend.js` with mock Reddit listings/comments, OAuth settings/token exchange scaffolding, Reddit listing/comment normalization, vote/save/reply/edit actions, selected-media loading, and explicit rejection for post creation.
- Updated top-level navigation to Front Page, Hot, Trending, Saved, and Subreddits. Feed rows use the chat-list surface; subreddit/sort/post browsing uses the reused card-list surface.
- Updated post opening to load Reddit-shaped post/comment bubbles. Select and long press in comment detail now route to normal comment actions instead of Planner field editing.
- Added Pebbit Reddit settings coverage in both the backend inline settings page and `src/pkjs/config.html`: client ID, redirect URI/code, default feed, comment sort, pinned/hidden subreddits, media mode, canned replies, NSFW, mock mode, and scopes.
- Removed visible create-post affordances from active watch paths; stale `create_task` backend compatibility paths remain only as fail-closed rejects.
- Verified `src/pkjs/index.js` and `src/pkjs/reddit/backend.js` with Node syntax checks.
- Ran `pebble build` successfully. Output: `build/Pebbit.pbw`. Build warnings observed: existing Waf lock warning, linker RWX segment warnings, and minifier warnings from bundled JS/image code.

2026-06-10:
- Fixed mock mode persistence by saving unchecked mock mode as `0`; mock data is now forced only when no Reddit client ID is configured or the mock checkbox is explicitly on.
- Improved Reddit OAuth code handling so Pebbit accepts a bare code, `code=...`, a full redirected URL, and copied URLs with trailing `#_`.
- Added a static hosted-callback helper at `docs/reddit-callback.html`; default redirect URI is now `https://tombolger.github.io/Pebbit/reddit-callback.html`, with `http://localhost:65010/pebbit` documented as a fallback.
- Added Reddit media URL extraction for direct images, Reddit preview images, thumbnails, and oEmbed thumbnails, including video preview thumbnails where Reddit exposes them.
- Wired media post rows to the existing small avatar/thumbnail transfer path and kept opened post media on the selected message image loader path.
- Updated save/unsave state tracking so saved post rows can clear the checkmark after toggling.
- Replaced the Pebble menu icon/source art with Reddit artwork from the Pebble iconography source.
- Updated watch colors/layout for Reddit: orange/yellow palette, top post as a card, left-aligned comments with small indents and orange rails.
- Verified with Node syntax checks and `pebble build`; output remains `build/Pebbit.pbw` and is copied to `I:\My drive`.
- Delegated the required implementation review work from Plan Execution: one worker focused on watch UI/action changes and one worker focused on backend/media/comment normalization. Integrated the backend worker results and closed the still-running watch worker after local implementation and verification.
- Changed opened Reddit threads so the initial detail view selects the OP/top card and scrolls downward into comments instead of opening at the bottom like a chat.
- Updated post/comment actions to remove visible React menus from Reddit detail cards and expose Reply, Upvote, Downvote, and Save through direct Reddit action commands. Reply targets the selected bubble/comment.
- Changed comment normalization so nested comments no longer get fake quote panels. Quote context is now reserved for explicit/recent Pebbit replies.
- Added simple hidden-reply summary rows for large reply branches so long Reddit conversations are partially collapsed on the watch.
- Expanded media extraction for post rows across all post list views, including front page, hot/trending/saved, subreddit posts, Reddit previews/galleries/video thumbnails, and direct Imgur image/page thumbnails. Text-only posts do not reserve thumbnail space.
- Changed list thumbnails and opened media frames to square/rectangular rendering instead of circular masks.
- Updated metadata strings to compact Reddit-style arrows and awards: up/down arrows in metadata and award emoji/count on the opposite side when available.
- Trimmed donor Planner/static UI buffers and unused marker drawing to keep all target platforms under Pebble's 16-bit app virtual-size limit.
- Verified with `node --check src/pkjs/index.js`, `node --check src/pkjs/reddit/backend.js`, `git diff --check`, and `pebble build`. Output: `build/Pebbit.pbw`.

2026-06-11:
- Delegated the required crash investigation from Plan Execution: one explorer audited phone-side Reddit/list/comment streaming and one explorer audited watch-side thread-opening crash risks.
- Capped phone-side list rows to the watch capacity of 12 so `chats_done` no longer reports rows the watch cannot store.
- Filtered Reddit listing rows to posts for feed/subreddit overviews and fail clearly on stale/expired post tokens instead of opening malformed `/comments/p1` URLs.
- Fixed Reddit thread ordering so initial detail windows preserve the OP/top card plus first comments instead of sorting or tail-selecting later comments.
- Routed feed post rows through Reddit detail mode so opened threads start at the OP card and scroll downward into comments.
- Added low-memory guards for thread layer creation and action window/layer creation to avoid NULL dereferences under heap pressure.
- Disabled automatic media fetch on Reddit thread open and tightened bitmap decode headroom checks; media placeholders remain, but native decode is no longer kicked off simply by opening a thread.
- Updated Reddit detail metadata rendering to display the compact arrow/comment/time string as sent by the backend instead of parsing it as legacy `up|down` counters.
- Verified with Node syntax checks, `git diff --check`, `pebble build`, and Basalt emulator open-thread testing. The feed loads, opening a thread lands on the OP card, and the emulator stays alive. Output copied to `I:\My drive\Pebbit.pbw`.
- Narrowed active target platforms to Emery for development so crash/media work can focus on the actual test watch.
- Fixed post-list media previews by starting the existing thumbnail transfer queue from every post/list row batch, not only the initial top-level navigation load.
- Re-enabled selected image loading in Reddit thread detail and cleared stale image/avatar transfer buffers before opening a thread to reduce Emery heap pressure.
- Fixed phone-side media downloads in Pebble JS runtimes by adding an XMLHttpRequest binary fallback when `fetch` is unavailable.
- Made Reddit detail state explicit so feed posts use Reddit comment actions/layout without corrupting Back navigation state.
- Changed phone-to-watch message transfer counts to report rows actually sent after trimming, preventing the watch from waiting for nonexistent comment rows.
- Reduced first thread/comment packets and real comment flattening to a smaller Emery-safe window while crash testing continues.
- Verified on the Emery emulator: feed opens, the media post row renders an actual square thumbnail, opening the media thread starts at the OP card, the OP image loads, and the emulator remains alive. Output copied to `I:\My drive\Pebbit.pbw`.

2026-06-12:
- Delegated the required Plan Execution crash follow-up: one explorer audited watch-side comment rendering/image cancellation, and one explorer audited phone-side image transfer cancellation/Reddit row shaping.
- Fixed the likely media/comment collision by having the watch send `cancel_image` before clearing an active image request and by ignoring stale image start/done/error packets without kicking off new image work.
- Changed phone-side image delivery from one burst of queued chunks to a paced, cancellable transfer that checks the current chat/request before each chunk.
- Reduced Reddit detail scroll pressure: Reddit detail row changes now use immediate alignment instead of the donor chat animation loop, and the 2ms scroll timer no longer prods image loading during Reddit detail animations.
- Made Reddit comment rows cheaper for Emery by removing visible fetched-comment quote panels, sending shorter watch-visible comment/OP text, and keeping full text available through the phone-side full-text cache.
- Trimmed donor-era watch buffers to keep the Emery build under Pebble's app size limit: smaller transient text buffers, fewer canned reply slots, and shorter title/status buffers.
- Reduced Emery media payload settings and increased phone-side image preparation timeout so selected media has more time to finish without flooding the watch.
- Verified `node --check src/pkjs/index.js`, `node --check src/pkjs/reddit/backend.js`, `git diff --check`, and `pebble build`.
- Wiped stale SDK emulator app state after install transport timeouts, then installed on the Emery emulator successfully. Text-only thread with first comment visible stayed alive; media thread scrolled from the OP/media card into the first comment and hidden-reply row without emulator OS crash.
- Final PBW for this pass was copied to `I:\My drive\Pebbit.pbw`. Remaining observation: the mock `picsum.photos` media path timed out once in the emulator after the stability fix, so real-device media should be rechecked with Reddit-hosted images.

2026-06-12 follow-up bug batch:
- [done] Fix opened-thread media sizing so Reddit media uses the full watch/card width, does not tile, stays loaded while still visible, and uses Pebblegram-style tall-photo safety to avoid high-resolution crashes.
- [done] Prevent double image loads/retry loops by having the phone choose a safe image size before sending instead of relying on watch-side resize retries.
- [done] Load at least five comments under the OP card and add bottom-scroll loading for more cached comments.
- [done] Shape comment threads like the June 11 My Drive screenshot: shallow indents, vertical rails, top-level/second-level readability, and collapsed deeper reply stacks with “view more” rows.
- [done] Show `Unsave` in post/comment action menus when the selected Reddit thing is already saved.
- [done] Move comment timestamps to the right side of the bottom metadata row and show awards separately; use U+1F44D for upvotes and U+1F389 for awards where the watch/font supports them.
- [done] Verified with JS syntax checks, `git diff --check`, and an Emery `pebble build`; output copied to `I:\My drive\Pebbit.pbw`. Explicit PBW install to the Emery emulator succeeded, but the follow-up screenshot/ping transport disconnected after install.
- [future] UI consistency overhaul: front page, subreddit views, and menus currently mix Pebblegram chat/list patterns with Planner card patterns and should be unified in a later design pass.

2026-06-12 deterministic media/text follow-up:
- [done] Removed watch-driven image resize negotiation. The watch now requests one fixed platform-safe image budget and failed media no longer re-enters a smaller retry loop.
- [done] Changed phone media generation to use one Emery-safe message-image target (`176x352`, `18000` bytes, `36000` pixels) and compact PBI output for post media before any chunks are sent.
- [done] Shrank unloaded media placeholders to loading-bar height until `image_start` reports the actual safe incoming dimensions, then relayouts the media card to match the incoming image.
- [done] Increased watch comment text capacity and Reddit preview text so normal rows carry substantially more body text before needing Full Message.
- [done] Fixed Full Message to load from the Reddit backend full-text cache instead of the already-truncated watch row, and unload the rest of the thread while the long scrolling text view is open.
- [done] Made collapsed/thread marker rows expose a single `Load More` action instead of comment actions.
- [done] Made Reddit Reply launch dictation directly instead of opening a Reply submenu with canned responses.
- [done] Verified with JS syntax checks, `git diff --check`, Emery `pebble build`, PBW copy to `I:\My drive\Pebbit.pbw`, and explicit Emery emulator install of `build/Pebbit.pbw`.

2026-06-12 regression repair:
- [done] Restored Pebblegram's hardened media retry/PBI safety branches after the deterministic media pass caused routine `Photo too large` failures.
- [done] Bumped the media cache version so stale oversized encodes are not reused after the image pipeline fix.
- [done] Restored `View Full Message` for Reddit detail rows and kept it backed by the full-text cache instead of the truncated watch row.
- [done] Fixed Reddit detail bottom scrolling so reaching the bottom requests the next cached comment window instead of doing nothing.
- [done] Fixed Reddit detail top scrolling after paging down so reaching the top requests the previous cached comment window instead of trapping the reader in newer comments.
- [done] Made `View More` selectable directly and wired it to expand the selected reply branch, including Reddit `/api/morechildren` markers when only child IDs are available.
- [done] Removed the old watch-side canned reply parser/menu entry so Reddit replies stay direct-to-dictation and the Emery app image remains under Pebble's size limit.
- [done] Verified with JS syntax checks, `git diff --check`, Emery `pebble build`, and PBW copy to `I:\My drive\Pebbit.pbw`.

2026-06-13 media/full-text/UI repair:
- [done] Delegated image/emoji comparison against Pebblegram and duplicate-UI mapping per Plan Execution.
- [done] Restored Pebblegram's PNG decode fallback after freeing other images so borderline images are not mislabeled `Photo too large`.
- [done] Fixed wide-image tiling by using the decoded bitmap aspect height for loaded media instead of forcing a 4:3 minimum rectangle.
- [done] Changed Reddit preview/gallery selection to choose the smallest watch-appropriate preview instead of the largest available source, and switched Imgur page thumbnails to medium thumbnails.
- [done] Fixed Full Message back navigation so it restores the unloaded thread window instead of returning to `No messages loaded`.
- [done] Replaced generic unsupported emoji text with known shortcode names such as `:eggplant:` and `:clown:`, falling back to `:emoji:` only for unknown emoji.
- [done] Removed the visible Planner-style Subreddits ladder from active navigation; configured subreddits now appear as normal front-page-style feed rows and open through the same post-list UI.
- [todo] Pebblegram handoff: port the unsupported emoji shortcode map from Pebbit's `src/pkjs/index.js` into Pebblegram's matching normalization functions; Pebblegram still uses generic `:emoji:` today.
- [done] Verified with JS syntax checks, `git diff --check`, Emery `pebble build`, and PBW copy to `I:\My drive\Pebbit.pbw`.

2026-06-13 publishing/navigation pass:
- [done] Delegated publishing/submenu review per Plan Execution.
- [done] Put pinned subreddits back behind a top-level `Subreddits` submenu while keeping subreddit post lists on the same front-page card UI.
- [done] Added a top-level `Subscriptions` submenu backed by Reddit's subscribed-subreddits API, filtered by the existing hidden-subreddits setting.
- [done] Updated Reddit settings copy so public users are directed to enter their own Reddit installed-app client ID/API key, use the exact redirect URI, and avoid client secrets.
- [done] Added a configurable User-Agent advanced setting and removed the Tom-specific default User-Agent from the backend.
- [done] Verified settings/navigation changes with JS syntax checks, `git diff --check`, an Emery build, and an Emery emulator install; output copied to `I:\My drive\Pebbit.pbw`.

2026-06-13 subscriptions/pinned wording fix:
- [done] Delegated subscription fallback review per Plan Execution.
- [done] Renamed the settings-managed subreddit list from `Visible subreddits` to `Pinned subreddits` in both settings pages and docs.
- [done] Changed the Subscriptions submenu so it no longer falls back to pinned subreddit rows when mock mode is enabled; it now uses the saved Reddit OAuth token path and fails clearly if sign-in is missing.
- [done] Verified with JS syntax checks, `git diff --check`, Emery build, and emulator install; output copied to `I:\My drive\Pebbit.pbw`.

2026-06-14 Emery size rescue:
- [done] Delegated a read-only native size review per Plan Execution.
- [done] Preserved the short pinned/subscription menu IDs while trimming watch-side legacy submenu comparisons and unused `Pin Community` action text.
- [done] Shortened a few native status strings to bring the Emery image back under Pebble's 16-bit virtual-size limit.
- [done] Verified with an Emery `pebble build`; output copied to `I:\My drive\Pebbit.pbw`.

2026-06-15 image tone/release pass:
- [done] Delegated a Pebblegram image-pipeline comparison per Plan Execution.
- [done] Ported Pebblegram's adaptive dark/light image tone mapping into Pebbit's Reddit image encoder.
- [done] Bumped the Reddit image cache version so washed-out cached media is regenerated.
- [done] Bumped the app package metadata to 0.2.0 for the release build.
- [done] Verified with JS syntax checks, `git diff --check`, and an Emery `pebble build`; output copied to `I:\My drive\Pebbit.pbw`.
