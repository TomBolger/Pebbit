#include <pebble.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "message_keys.auto.h"

#define PT_DEBUG_BRANCH 0
#define PT_DEBUG_OPEN_PLANNER_CARD 0
#define PT_DEBUG_FORCE_DUE_SELECTION 0
#if defined(PBL_PLATFORM_BASALT)
#define MEDIA_ENABLED 0
#define MEDIA_ONLY_UNUSED __attribute__((unused))
#else
#define MEDIA_ENABLED 1
#define MEDIA_ONLY_UNUSED
#endif

#define MAX_CHATS 12
#define MAX_ASSIGNEE_OPTIONS PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 3, 3, 3, 3, 5, 5, 5)
#define MAX_MESSAGES 6
#define MAX_BUCKET_OPTIONS 10
#define DATE_CHOICE_COUNT 5
#define MAX_TEXT 180
#define MAX_FULL_TEXT 1900
#define MESSAGE_PREVIEW_TEXT PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 120, 120, 120, 120, 220, 220, 220)
#define MAX_SENDER 36
#define MAX_REACTIONS 17
#define MAX_META 24
#define MAX_CONTEXT_TEXT PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 44, 38, 38, 38, 62, 58, 54)
#define MAX_CONTEXT_SENDER_TEXT PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 16, 14, 14, 14, 26, 24, 22)
#define MAX_CONTEXT_BODY_TEXT PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 24, 20, 20, 20, 46, 40, 36)
#define MAX_ID 20
#define MAX_IMAGE_ERROR 20
#define MAX_IMAGE_BYTES PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 10000, 6500, 6500, 6000, 30000, 15000, 15000)
#define MAX_AVATAR_BYTES PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 5000, 5000, 5000, 3000, 5000, 5000, 5000)
#define MAX_LOADED_IMAGES 1
#define IMAGE_THUMB_SIZE PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 120, 96, 96, 96, 176, 156, 118)
#define IMAGE_FRAME_EXTRA_W PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 10, 8, 8, 6, 14, 14, 10)
#if defined(PBL_PLATFORM_BASALT)
#define APP_INBOX_SIZE 768
#define APP_OUTBOX_SIZE 384
#else
#define APP_INBOX_SIZE PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 1024, 1024, 1024, 1024, 2048, 2048, 2048)
#define APP_OUTBOX_SIZE PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 512, 512, 512, 512, 1024, 1024, 1024)
#endif
#define BW_UI PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 0, 0, 0, 1, 0, 0, 0)
#define ROUND_UI PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 0, 0, 0, 0, 0, 0, 1)
#define STATUS_H PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 24, 24, 24, 24, 24, 24, 22)
#define PLANNER_LABEL_COUNT 6
#define PLANNER_LABEL_TEXT_LEN 24
#define PG_MIN(a, b) ((a) < (b) ? (a) : (b))
#define PG_MAX(a, b) ((a) > (b) ? (a) : (b))
#define APP_COLOR GColorFromHEX(0xFF4500)
#define APP_COLOR_LIGHT GColorFromHEX(0xD93A00)
#define UNREAD_COLOR GColorFromHEX(0xFFB000)
#define CHAT_BG GColorWhite
#define PLANNER_BG GColorFromHEX(0xFFF7E6)
#define PLANNER_CARD GColorFromHEX(0xFFF0CC)
#define PLANNER_CARD_ALT GColorFromHEX(0xFFB000)
#define PLANNER_ITEM_BG GColorFromHEX(0xFFF8E1)
#define PLANNER_ITEM_NAME_COLOR GColorFromHEX(0xC63D00)
#define IN_BUBBLE GColorFromHEX(0xFFF4CC)
#define OUT_BUBBLE GColorFromHEX(0xFFE1B2)
#define IN_CONTEXT_BUBBLE GColorFromHEX(0xFFF2A8)
#define OUT_CONTEXT_BUBBLE GColorFromHEX(0xFFD199)
#define CHAT_SENDER_COLOR GColorFromHEX(0xC63D00)
#define SELF_CONTEXT_SENDER_COLOR GColorFromHEX(0xA83200)
#define SELECTOR_COLOR GColorFromHEX(0xFF4500)
#define SELECTED_IN_BUBBLE GColorFromHEX(0xDADADA)
#define SELECTED_OUT_BUBBLE APP_COLOR
#define ACTION_BG GColorBlack
#define ACTION_TEXT GColorDarkGray
#define ACTION_TEXT_SELECTED GColorWhite
#define CHAT_SCROLL_STEPS 4
#define CHAT_SCROLL_FRAME_MS 2
#define CHAT_SCROLL_DELTA 30
#define REPEAT_SCROLL_MS 140
#define MESSAGE_MODE_INITIAL 0
#define MESSAGE_MODE_OLDER 1
#define MESSAGE_MODE_NEWER 2
#define LONG_MESSAGE_SCROLL_DELTA PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 42, 42, 42, 42, 56, 56, 48)
#define COMPOSE_BUBBLE_H 30
#define COMPOSE_BUBBLE_GAP 8
#define MESSAGE_COMMAND_RETRY_MS 3000
#define MESSAGE_COMMAND_WAKE_RETRY_MS 650
#define MESSAGE_COMMAND_MAX_ATTEMPTS 3
#define MESSAGE_TRANSFER_TIMEOUT_MS 20000
#define CHAT_FIRST_PAINT_ROWS PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 3, 3, 3, 3, 5, 5, 5)
#define IMAGE_COMMAND_RETRY_MS 350
#define IMAGE_PREPARE_STALL_MS 30000
#define IMAGE_TRANSFER_STALL_MS 12000
#define CHAT_COMMAND_WAKE_RETRY_MS 700
#define CHAT_COMMAND_MAX_ATTEMPTS 4
#define PHONE_WAKE_DELAY_MS 180
#define IMAGE_KEEP_SCREEN_MARGIN 48
#define IMAGE_LOAD_SCREEN_MARGIN 24
#define IMAGE_TALL_MAX_MULTIPLIER 2
#define IMAGE_DECODE_HEADROOM_BYTES 12000
#define IMAGE_DECODE_HEADROOM_PIXELS 16000
#define IMAGE_DECODE_FINAL_HEADROOM_BYTES PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 8000, 7000, 7000, 6000, 10000, 9000, 8000)
#define IMAGE_DECODE_MAX_DIMENSION 512
#define IMAGE_RETRY_MAX_LEVEL 3
#define IMAGE_DIAG_LOGS 0
#define STATUS_CLEAR_MS 1000
#define VIEW_TRANSITION_MS 120
#define DICTATION_START_DELAY_MS 900
#define DICTATION_CONFIRM_DELAY_MS 800
#define TOUCH_KEYBOARD_ENABLED 0
#define TOUCH_KEYBOARD_MAX_TEXT 96
#define TOUCH_KEYBOARD_INPUT_H 30
#define TOUCH_KEYBOARD_ROW_H 21
#define TOUCH_KEYBOARD_ROWS 4
#if TOUCH_KEYBOARD_ENABLED && defined(_PBL_API_EXISTS_touch_service_subscribe)
#define TOUCH_KEYBOARD_AVAILABLE 1
#else
#define TOUCH_KEYBOARD_AVAILABLE 0
#endif

#if IMAGE_DIAG_LOGS
#define IMAGE_DIAG(...) APP_LOG(APP_LOG_LEVEL_INFO, __VA_ARGS__)
#else
#define IMAGE_DIAG(...)
#endif

#if PT_DEBUG_BRANCH
#define DBG(...)
#define CDBG(...)
#else
#define DBG(...)
#define CDBG(...)
#endif

// Platform constants are centralized here. Basalt/Diorite stay conservative on
// heap use; Emery/Gabbro can afford longer text and larger image payloads.
typedef enum {
  ViewStateChatList,
  ViewStateChat
} ViewState;

typedef enum {
  ListModeChats,
  ListModeGroupChats,
  ListModeSubreddits,
  ListModeSorts,
  ListModeTasks
} ListMode;

typedef enum {
  ActionMenuMain,
  ActionMenuChat,
  ActionMenuCanned,
  ActionMenuConfirm,
  ActionMenuReply,
  ActionMenuReactionGrid,
  ActionMenuEmojiReplyGrid,
  ActionMenuPost,
  ActionMenuChecklistList,
  ActionMenuChecklistItem,
  ActionMenuSubredditsElement,
  ActionMenuSubredditsAddItem,
  ActionMenuNotes,
  ActionMenuPriority,
  ActionMenuProgress,
  ActionMenuLabels,
  ActionMenuSorts,
  ActionMenuDate,
  ActionMenuAssign,
  ActionMenuAssignRemove,
  ActionMenuAssignTeams,
  ActionMenuAssignMembers,
  ActionMenuFullText
} ActionMenuMode;

typedef enum {
  ActionItemCompose,
  ActionItemCanned,
  ActionItemEmoji,
  ActionItemReply,
  ActionItemReplyDictate,
  ActionItemReplyCanned,
  ActionItemReplyEmoji,
  ActionItemReact,
  ActionItemEdit,
  ActionItemDelete,
  ActionItemFullText,
  ActionItemFullContext,
  ActionItemGoToBottom,
  ActionItemArchiveChat,
  ActionItemMarkUnread,
  ActionItemSubredditsAddNote,
  ActionItemSubredditsAddChecklist,
  ActionItemSubredditsSetDue,
  ActionItemSubredditsComplete,
  ActionItemSubredditsDownvote,
  ActionItemOpenPost,
  ActionItemCompletedPosts,
  ActionItemDeletePost,
  ActionItemChecklistEditList,
  ActionItemChecklistDeleteList,
  ActionItemChecklistAddItem,
  ActionItemChecklistEditItem,
  ActionItemChecklistDeleteItem,
  ActionItemChecklistToggleItem,
  ActionItemSubredditsElementEdit,
  ActionItemSubredditsElementDelete,
  ActionItemSubredditsAddAssign,
  ActionItemSubredditsAddStartDate,
  ActionItemSubredditsAddPriority,
  ActionItemSubredditsAddProgress,
  ActionItemSubredditsAddBucket,
  ActionItemSubredditsAddLabels,
  ActionItemSubredditsMarkActive,
  ActionItemNotesView,
  ActionItemNotesEdit,
  ActionItemNotesAdd,
  ActionItemNotesDelete,
  ActionItemPriorityChoice,
  ActionItemProgressChoice,
  ActionItemLabelChoice,
  ActionItemBucketChoice,
  ActionItemBucketNew,
  ActionItemDateChoice,
  ActionItemAssignMe,
  ActionItemAssignConfigured,
  ActionItemAssignRemove,
  ActionItemAssignRemoveMenu,
  ActionItemAssignAdd,
  ActionItemAssignTeam,
  ActionItemAssignMember,
  ActionItemConfirmSend,
  ActionItemConfirmCancel,
  ActionItemGoBack
} ActionItem;

typedef struct {
  char id[MAX_ID];
  char *string_storage;
  char *title;
  char *preview;
  char section[16];
  bool unread;
  int unread_count;
  GBitmap *avatar_bitmap;
} Chat;

typedef struct {
  char id[MAX_ID];
  char title[36];
} BucketOption;

typedef struct {
  char id[MAX_ID];
  char title[32];
} AssigneeOption;

typedef struct {
  bool active;
  ActionItem item;
  int index;
  uint32_t generation;
  char chat_id[MAX_ID];
  char message_id[MAX_ID];
  char title[36];
  int checklist_item;
} SubredditsIntent;

typedef struct {
  bool active;
  char command[24];
  char target_id[MAX_ID];
  char text[MAX_TEXT];
  char message_id[MAX_ID];
} SubredditsWireCommand;

typedef struct {
  char id[MAX_ID];
  char *string_storage;
  char *sender;
  char *text;
  char *reactions;
  char *meta;
  char *context;
  char section[16];
  char image_token[MAX_ID];
  char image_error[MAX_IMAGE_ERROR];
  bool outgoing;
  bool text_truncated;
  bool image_placeholder;
  bool image_requested;
  bool image_failed;
  uint8_t image_progress;
  uint8_t image_retry_level;
  uint16_t image_width;
  uint16_t image_height;
  GBitmap *image_bitmap;
  uint8_t *image_data;
} Message;

static const int DATE_CHOICE_OFFSETS[DATE_CHOICE_COUNT] = {
  0, 1, 2, 3, 7
};

static const char *const DATE_CHOICE_LABELS[DATE_CHOICE_COUNT] = {
  "Today",
  "Tomorrow",
  "In 2 days",
  "In 3 days",
  "In 1 week"
};

static Window *s_main_window;
static MenuLayer *s_chat_menu;
static TextLayer *s_status_layer;
static Layer *s_messages_root;
static PropertyAnimation *s_chat_menu_animation;
static PropertyAnimation *s_messages_animation;

static Window *s_action_window;
static Window *s_action_window_pending_destroy;
static Layer *s_action_layer;
static ActionMenuMode s_action_mode;
static int s_action_selected;
static int s_full_text_scroll_offset;
static int s_full_text_height;
static bool s_full_text_context;
static bool s_full_text_unloaded_messages;
static char *s_full_text_body;

static DictationSession *s_dictation_session;
static bool s_dictation_in_progress;

typedef struct {
  char full_text_title[MAX_SENDER + 10];
  char full_text_message_id[MAX_ID];
  char message_restore_id[MAX_ID];
  char action_label_edit[36];
  char action_label_delete[36];
  char image_message_id[MAX_ID];
  char selected_image_focus_id[MAX_ID];
  char avatar_chat_id[MAX_ID];
  char pending_text[MAX_TEXT];
  char pending_edit_message_id[MAX_ID];
  char pending_chat_command[24];
  char pending_subreddit_source_id[MAX_ID];
  char current_chat_id[MAX_ID];
  char current_chat_title[36];
  char status_text[40];
  char chat_refresh_selected_id[MAX_ID];
  char chat_list_selected_id[MAX_ID];
  char current_plan_id[MAX_ID];
  char current_plan_title[36];
  char current_bucket_id[MAX_ID];
  char current_bucket_title[36];
  char assign_selected_team_id[MAX_ID];
  char date_command[24];
  char date_label[20];
  char date_target_id[MAX_ID];
  char subreddit_refresh_anchor_id[MAX_ID];
  char older_anchor_id[MAX_ID];
  char newer_anchor_id[MAX_ID];
  SubredditsIntent subreddit_intent;
  SubredditsWireCommand subreddit_wire_command;
} PebbitHeapState;

static PebbitHeapState *s_heap_state;
#define s_full_text_title (s_heap_state->full_text_title)
#define s_full_text_message_id (s_heap_state->full_text_message_id)
#define s_message_restore_id (s_heap_state->message_restore_id)
#define s_action_label_edit (s_heap_state->action_label_edit)
#define s_action_label_delete (s_heap_state->action_label_delete)
#define s_image_message_id (s_heap_state->image_message_id)
#define s_selected_image_focus_id (s_heap_state->selected_image_focus_id)
#define s_avatar_chat_id (s_heap_state->avatar_chat_id)
#define s_pending_text (s_heap_state->pending_text)
#define s_pending_edit_message_id (s_heap_state->pending_edit_message_id)
#define s_pending_chat_command (s_heap_state->pending_chat_command)
#define s_pending_subreddit_source_id (s_heap_state->pending_subreddit_source_id)
#define s_current_chat_id (s_heap_state->current_chat_id)
#define s_current_chat_title (s_heap_state->current_chat_title)
#define s_status_text (s_heap_state->status_text)
#define s_chat_refresh_selected_id (s_heap_state->chat_refresh_selected_id)
#define s_chat_list_selected_id (s_heap_state->chat_list_selected_id)
#define s_current_plan_id (s_heap_state->current_plan_id)
#define s_current_plan_title (s_heap_state->current_plan_title)
#define s_current_bucket_id (s_heap_state->current_bucket_id)
#define s_current_bucket_title (s_heap_state->current_bucket_title)
#define s_assign_selected_team_id (s_heap_state->assign_selected_team_id)
#define s_date_command (s_heap_state->date_command)
#define s_date_label (s_heap_state->date_label)
#define s_date_target_id (s_heap_state->date_target_id)
#define s_subreddit_refresh_anchor_id (s_heap_state->subreddit_refresh_anchor_id)
#define s_older_anchor_id (s_heap_state->older_anchor_id)
#define s_newer_anchor_id (s_heap_state->newer_anchor_id)
#define s_subreddit_intent (s_heap_state->subreddit_intent)
#define s_subreddit_wire_command (s_heap_state->subreddit_wire_command)

static Chat *s_chats;
static Message *s_messages;
static Message *s_message_stage;
static int *s_message_y;
static int *s_message_h;
static int s_compose_bubble_y;
static uint8_t *s_image_buffer;
static uint8_t *s_avatar_buffer;
static uint16_t s_image_buffer_capacity;
static uint16_t s_avatar_buffer_capacity;
static int s_image_size;
static int s_image_received;
static int s_image_expected_offset;
static int s_image_transfer_id;
static bool s_image_is_pbi;
static int s_avatar_size;
static int s_avatar_received;
static int s_avatar_expected_offset;
static int s_avatar_transfer_id;
static int s_loaded_image_count;
static char *s_subreddit_labels_packed;
static bool s_pending_send_as_reply;
static bool s_touch_keyboard_open;
#if TOUCH_KEYBOARD_AVAILABLE
static bool s_touch_keyboard_symbols;
static bool s_touch_keyboard_shift;
static char s_touch_keyboard_sent_text[TOUCH_KEYBOARD_MAX_TEXT];
#endif
static char s_loading_text[20] = "Loading...";

static int s_chat_count;
static int s_message_count;
static int s_selected_chat;
static int s_selected_message;
static int s_expected_rows;
static int s_message_transfer_id;
static int s_message_stream_mode;
static int s_message_stage_count;
static int s_chat_scroll_offset;
static int s_chat_scroll_start;
static int s_chat_scroll_target;
static int s_chat_scroll_step;
static int s_chat_content_height;
static int s_message_scroll_direction;
static ViewState s_view_state;
static ListMode s_list_mode;
static int s_selected_checklist_item;
static bool s_viewing_completed_tasks;
static bool s_checklist_edit_mode;
static bool s_reddit_detail_request;
static uint32_t s_ui_generation;
static uint32_t s_pending_subreddit_generation;
static uint32_t s_date_generation;
static BucketOption *s_bucket_options;
static int s_bucket_option_count;
static AssigneeOption *s_assign_team_options;
static int s_assign_team_option_count;
static AssigneeOption *s_assign_member_options;
static int s_assign_member_option_count;
static int s_subreddit_refresh_anchor_checklist_item;
static bool s_subreddit_refresh_anchor_valid;
static bool s_bridge_ready;
static bool s_interaction_locked;
static bool s_chats_loading;
static bool s_loading_error;
static bool s_chat_first_paint_reported;
static bool s_chat_view_pending;
static bool s_loading_messages;
static bool s_loading_older_messages;
static bool s_loading_newer_messages;
static bool s_user_scrolled_messages;
static bool s_at_newest;
static bool s_at_oldest;
static bool s_message_stream_silent;
static int s_chat_loading_progress;
static int s_older_anchor_y;
static int s_newer_anchor_y;
static AppTimer *s_chat_scroll_timer;
static AppTimer *s_message_timeout_timer;
static AppTimer *s_message_retry_timer;
static AppTimer *s_image_retry_timer;
static AppTimer *s_status_clear_timer;
static AppTimer *s_action_window_destroy_timer;
static AppTimer *s_interaction_unlock_timer;
static AppTimer *s_chat_retry_timer;
static AppTimer *s_startup_wake_timer;
static AppTimer *s_dictation_start_timer;
static AppTimer *s_dictation_confirm_timer;
static AppTimer *s_dictation_failure_timer;
static AppTimer *s_date_send_timer;
static AppTimer *s_subreddit_intent_timer;
static AppTimer *s_subreddit_command_timer;
#if PT_DEBUG_FORCE_DUE_SELECTION
static AppTimer *s_debug_due_open_timer;
static bool s_debug_due_opened;
#endif
static int s_message_request_attempts;
static int s_chat_request_attempts;

static void request_chats(void);
static void request_subreddits(void);
static void request_group_chats(const char *feed_id, const char *feed_title);
static void request_sorts(const char *plan_id, const char *plan_title);
static void request_tasks(const char *bucket_id, const char *bucket_title);
static void request_completed_tasks(const char *bucket_id, const char *bucket_title);
static void request_task_detail(const char *task_id, const char *task_title);
static void request_messages(const char *chat_id);
static void request_next_image(void);
static void startup_wake_timer_callback(void *data);
static void chat_retry_timer_callback(void *data);
static void clear_active_image_request(void);
static bool selected_message_needs_image(void);
static void image_retry_timer_callback(void *data);
static void request_older_messages(bool silent);
static void request_newer_messages(bool silent);
static void refresh_loaded_image_count(void);
static void destroy_message_images(void);
static void destroy_offscreen_message_images(void);
static bool destroy_unselected_loaded_image(void);
static void main_back_click_handler(ClickRecognizerRef recognizer, void *context);
static void send_text_message(const char *text, bool as_reply);
static void edit_selected_message(const char *text);
static void send_selected_chat_action(const char *command);
static void show_action_window(ActionMenuMode mode);
static void action_window_destroy_timer_callback(void *data);
static void open_date_window(const char *command, const char *label);
static void send_pending_subreddit_action(void);
static bool pending_command_is_subreddit_action(void);
static void set_pending_subreddit_voice_action(const char *command, const char *message_id);
static void commit_pending_text_action(void);
static bool subreddit_detail_row_is_top_post(int index);
static int subreddit_detail_bubble_width(GRect safe, int index);
static int subreddit_detail_indent_for_index(int index);
static void subreddit_intent_timer_callback(void *data);
static void capture_subreddit_intent(ActionItem item, int index);
static void schedule_captured_subreddit_intent(uint32_t delay_ms);
static void clear_subreddit_intent(void);
static void queue_subreddit_command(const char *command, const char *target_id,
                                  const char *text, const char *message_id);
static void select_chat_row(int row, bool animated);
static void remove_chat_at(int row);
static void render_messages(void);
static void preserve_stream_anchor(const char *anchor_id, int anchor_y, bool dirty);
static void show_chat_view_timer(void *data);
static void message_timeout_timer_callback(void *data);
static int subreddit_message_index_for(const char *id, const char *kind);
#if PT_DEBUG_FORCE_DUE_SELECTION
static void debug_focus_due_row(void);
#endif
static void cancel_message_timeout(void);
static void cancel_message_retry(void);
static void schedule_message_timeout(void);
static void schedule_message_send_retry(void);
static void show_status(const char *message);
static void lock_interactions(uint32_t delay_ms);
static void unlock_interactions_now(void);
static bool interaction_busy(const char *message);
static void status_clear_timer_callback(void *data);
static const char *default_status_text(void);
static bool send_command_with_status(const char *command, const char *chat_id, const char *text,
                                     const char *reply_to, const char *message_id, bool show_failures);
static bool send_command(const char *command, const char *chat_id, const char *text,
                         const char *reply_to, const char *message_id);
static void show_loading_text(const char *message, bool is_error);
static void click_config_provider(void *context);
static void copy_cstr(char *dest, size_t dest_size, const char *src);
static void action_click_config_provider(void *context);
static void action_window_unload(Window *window);
static void start_dictation(void);
static void schedule_dictation_start(void);
#if TOUCH_KEYBOARD_AVAILABLE
static int touch_keyboard_height(void);
static void touch_handler(const TouchEvent *event, void *context);
#endif
static bool selected_message_is_truncated(void);
static bool selected_message_is_thread_marker(void);
static bool selected_message_has_context(void);
static bool has_selected_message(void);
static bool cstr_equal_ignore_case(const char *a, const char *b);
static bool message_is_checklist(Message *message);
static int checklist_item_count(const char *text);
static void checklist_line_at(const char *text, int target, char *dest, size_t dest_size);
static bool compose_target_is_selected(void);
static void recalc_message_layout(void);
static void scroll_to_bottom(bool animated);
static void go_to_bottom(void);
static void set_chat_scroll_offset(int target, bool animated);
static void set_chat_scroll_offset_quiet(int target);
static void select_message_with_alignment(int index, bool align_top, bool animated);
static void ensure_selected_checklist_item_visible(bool animated);
static int subreddit_message_index_for(const char *id, const char *kind);
static void chat_scroll_timer_callback(void *data);
static void messages_root_update_proc(Layer *layer, GContext *ctx);

static void update_subreddit_labels(const char *packed) {
  if (s_subreddit_labels_packed) {
    free(s_subreddit_labels_packed);
    s_subreddit_labels_packed = NULL;
  }
  if (packed && packed[0]) {
    s_subreddit_labels_packed = malloc(strlen(packed) + 1);
    if (s_subreddit_labels_packed) {
      copy_cstr(s_subreddit_labels_packed, strlen(packed) + 1, packed);
    }
  }
}

static void subreddit_label_text(int index, char *dest, size_t dest_size) {
  if (!dest || dest_size == 0) {
    return;
  }
  snprintf(dest, dest_size, "Category %d", index + 1);
  if (index < 0 || index >= PLANNER_LABEL_COUNT || !s_subreddit_labels_packed) {
    return;
  }

  const char *start = s_subreddit_labels_packed;
  for (int i = 0; i < index; i++) {
    start = strchr(start, '|');
    if (!start) {
      return;
    }
    start++;
  }

  const char *end = strchr(start, '|');
  size_t len = end ? (size_t)(end - start) : strlen(start);
  if (len == 0) {
    return;
  }
  len = PG_MIN(len, dest_size - 1);
  memcpy(dest, start, len);
  dest[len] = '\0';
}

static int utf8_expected_bytes(unsigned char lead) {
  if (lead < 0x80) {
    return 1;
  }
  if ((lead & 0xe0) == 0xc0) {
    return 2;
  }
  if ((lead & 0xf0) == 0xe0) {
    return 3;
  }
  if ((lead & 0xf8) == 0xf0) {
    return 4;
  }
  return 0;
}

static void trim_incomplete_utf8(char *text) {
  size_t len;
  size_t lead;
  int expected;
  if (!text || !text[0]) {
    return;
  }
  len = strlen(text);
  lead = len;
  while (lead > 0 && (((unsigned char)text[lead - 1] & 0xc0) == 0x80)) {
    lead--;
  }
  if (lead == len) {
    expected = utf8_expected_bytes((unsigned char)text[len - 1]);
    if (expected == 1) {
      return;
    }
    if (expected == 0 || len - 1 + (size_t)expected > len) {
      text[len - 1] = '\0';
    }
    return;
  }
  if (lead == 0) {
    text[0] = '\0';
    return;
  }
  expected = utf8_expected_bytes((unsigned char)text[lead - 1]);
  if (expected == 0 || lead - 1 + (size_t)expected > len) {
    text[lead - 1] = '\0';
  }
}

// Pebble's string helpers do not consistently protect callers from NULL input.
static void copy_cstr(char *dest, size_t dest_size, const char *src) {
  if (!dest || dest_size == 0) {
    return;
  }
  if (!src) {
    dest[0] = '\0';
    return;
  }
  strncpy(dest, src, dest_size - 1);
  dest[dest_size - 1] = '\0';
  trim_incomplete_utf8(dest);
}

static char s_empty_message_string[] = "";

static void release_message_strings(Message *message);

static size_t bounded_string_len(const char *src, size_t max_size) {
  size_t len;
  if (!src || !src[0] || max_size == 0) {
    return 0;
  }
  len = strlen(src);
  if (len >= max_size) {
    len = max_size - 1;
  }
  return len;
}

static char *pack_message_string(char **cursor, const char *src, size_t len) {
  char *dest;
  if (!cursor || !*cursor || !src || !src[0] || len == 0) {
    return s_empty_message_string;
  }
  dest = *cursor;
  memcpy(dest, src, len);
  dest[len] = '\0';
  trim_incomplete_utf8(dest);
  *cursor += len + 1;
  return dest[0] ? dest : s_empty_message_string;
}

static void init_chat_strings(Chat *chat) {
  if (!chat) {
    return;
  }
  chat->string_storage = NULL;
  chat->title = s_empty_message_string;
  chat->preview = s_empty_message_string;
}

static void release_chat_strings(Chat *chat) {
  if (!chat) {
    return;
  }
  if (chat->string_storage) {
    free(chat->string_storage);
    chat->string_storage = NULL;
  }
  chat->title = s_empty_message_string;
  chat->preview = s_empty_message_string;
}

static bool set_chat_strings(Chat *chat, const char *title, const char *preview) {
  size_t title_len = bounded_string_len(title, 48);
  size_t preview_len = bounded_string_len(preview, MESSAGE_PREVIEW_TEXT);
  size_t total = (title_len ? title_len + 1 : 0) +
                 (preview_len ? preview_len + 1 : 0);
  char *storage = total ? malloc(total) : NULL;
  char *cursor = storage;
  if (!chat) {
    if (storage) {
      free(storage);
    }
    return false;
  }
  release_chat_strings(chat);
  if (total && !storage) {
    return false;
  }
  chat->string_storage = storage;
  chat->title = pack_message_string(&cursor, title, title_len);
  chat->preview = pack_message_string(&cursor, preview, preview_len);
  return true;
}

static void init_message_strings(Message *message) {
  if (!message) {
    return;
  }
  message->string_storage = NULL;
  message->sender = s_empty_message_string;
  message->text = s_empty_message_string;
  message->reactions = s_empty_message_string;
  message->meta = s_empty_message_string;
  message->context = s_empty_message_string;
}

static bool set_message_strings(Message *message, const char *sender, const char *text,
                                const char *reactions, const char *meta, const char *context) {
  size_t sender_len = bounded_string_len(sender, MAX_SENDER);
  size_t text_len = bounded_string_len(text, MAX_TEXT);
  size_t reactions_len = bounded_string_len(reactions, MAX_REACTIONS);
  size_t meta_len = bounded_string_len(meta, MAX_META);
  size_t context_len = bounded_string_len(context, MAX_CONTEXT_TEXT);
  size_t total = (sender_len ? sender_len + 1 : 0) +
                 (text_len ? text_len + 1 : 0) +
                 (reactions_len ? reactions_len + 1 : 0) +
                 (meta_len ? meta_len + 1 : 0) +
                 (context_len ? context_len + 1 : 0);
  char *storage = total ? malloc(total) : NULL;
  char *cursor = storage;
  if (!message) {
    if (storage) {
      free(storage);
    }
    return false;
  }
  release_message_strings(message);
  if (total && !storage) {
    return false;
  }
  message->string_storage = storage;
  message->sender = pack_message_string(&cursor, sender, sender_len);
  message->text = pack_message_string(&cursor, text, text_len);
  message->reactions = pack_message_string(&cursor, reactions, reactions_len);
  message->meta = pack_message_string(&cursor, meta, meta_len);
  message->context = pack_message_string(&cursor, context, context_len);
  return true;
}

static void release_message_strings(Message *message) {
  if (!message) {
    return;
  }
  if (message->string_storage) {
    free(message->string_storage);
    message->string_storage = NULL;
  }
  message->sender = s_empty_message_string;
  message->text = s_empty_message_string;
  message->reactions = s_empty_message_string;
  message->meta = s_empty_message_string;
  message->context = s_empty_message_string;
}

static void set_message_text(Message *message, const char *text) {
  if (!message) {
    return;
  }
  char sender[MAX_SENDER];
  char reactions[MAX_REACTIONS];
  char meta[MAX_META];
  char context[MAX_CONTEXT_TEXT];
  copy_cstr(sender, sizeof(sender), message->sender);
  copy_cstr(reactions, sizeof(reactions), message->reactions);
  copy_cstr(meta, sizeof(meta), message->meta);
  copy_cstr(context, sizeof(context), message->context);
  set_message_strings(message, sender, text, reactions, meta, context);
}

static void set_message_sender_text(Message *message, const char *sender, const char *text) {
  if (!message) {
    return;
  }
  char reactions[MAX_REACTIONS];
  char meta[MAX_META];
  char context[MAX_CONTEXT_TEXT];
  copy_cstr(reactions, sizeof(reactions), message->reactions);
  copy_cstr(meta, sizeof(meta), message->meta);
  copy_cstr(context, sizeof(context), message->context);
  set_message_strings(message, sender, text, reactions, meta, context);
}

static void set_message_context_text(Message *message, const char *context) {
  if (!message) {
    return;
  }
  char sender[MAX_SENDER];
  char text[MAX_TEXT];
  char reactions[MAX_REACTIONS];
  char meta[MAX_META];
  copy_cstr(sender, sizeof(sender), message->sender);
  copy_cstr(text, sizeof(text), message->text);
  copy_cstr(reactions, sizeof(reactions), message->reactions);
  copy_cstr(meta, sizeof(meta), message->meta);
  set_message_strings(message, sender, text, reactions, meta, context);
}

static void truncate_cstr_bytes(char *text, size_t size, size_t max_bytes, const char *suffix) {
  size_t suffix_len = suffix ? strlen(suffix) : 0;
  size_t cut;
  if (!text || size == 0 || strlen(text) <= max_bytes || max_bytes + 1 > size) {
    return;
  }
  cut = max_bytes > suffix_len ? max_bytes - suffix_len : 0;
  text[cut] = '\0';
  trim_incomplete_utf8(text);
  if (suffix_len && strlen(text) + suffix_len < size) {
    strncat(text, suffix, size - strlen(text) - 1);
  }
}

static char *tuple_cstring(DictionaryIterator *iter, uint32_t key) {
  Tuple *tuple = dict_find(iter, key);
  return tuple ? tuple->value->cstring : NULL;
}

static int tuple_int(DictionaryIterator *iter, uint32_t key, int fallback) {
  Tuple *tuple = dict_find(iter, key);
  return tuple ? (int)tuple->value->int32 : fallback;
}

// Round screens need a narrower central column so rows stay clear of corners.
static GRect round_safe_rect(GRect bounds) {
  if (!ROUND_UI) {
    return bounds;
  }
  int inset = bounds.size.w >= 180 ? 28 : 22;
  return GRect(inset, bounds.origin.y, bounds.size.w - (inset * 2), bounds.size.h);
}

static int message_side_inset(GRect bounds) {
  return ROUND_UI ? PG_MAX(28, bounds.size.w / 7) : 3;
}

static int message_bubble_width(GRect bounds) {
  if (ROUND_UI) {
    return PG_MAX(112, bounds.size.w - (message_side_inset(bounds) * 2));
  }
  return bounds.size.w - 14;
}

static bool subreddit_list_active(void) {
  return s_list_mode == ListModeSubreddits || s_list_mode == ListModeSorts || s_list_mode == ListModeTasks;
}

static bool subreddit_detail_active(void) {
  return s_view_state == ViewStateChat && (s_reddit_detail_request || s_list_mode == ListModeTasks);
}

static bool checklist_edit_active(void) {
  return subreddit_detail_active() && s_checklist_edit_mode;
}

static void bump_ui_generation(void) {
  s_ui_generation++;
  if (s_ui_generation == 0) {
    s_ui_generation = 1;
  }
}

static bool chat_row_has_avatar(Chat *chat) {
#if !MEDIA_ENABLED
  return false;
#endif
  return chat && strcmp(chat->section, "media") == 0;
}

static int message_image_frame_width(int bubble_w) {
  return PG_MIN(IMAGE_THUMB_SIZE + IMAGE_FRAME_EXTRA_W, bubble_w - 10);
}

static int message_image_frame_width_for_index(int index, GRect bounds) {
  int bubble_w = message_bubble_width(bounds);
  if (subreddit_detail_active() && index >= 0) {
    GRect safe = round_safe_rect(bounds);
    bubble_w = subreddit_detail_bubble_width(safe, index);
    int text_w = bubble_w - 10;
    if (!subreddit_detail_row_is_top_post(index)) {
      text_w = PG_MAX(1, text_w - 5);
    }
    return PG_MAX(32, text_w);
  }
  return message_image_frame_width(bubble_w);
}

static int chat_status_y(void) {
  return ROUND_UI ? 6 : 0;
}

static int chat_content_y(void) {
  return ROUND_UI ? 48 : STATUS_H;
}

static int chat_bottom_pad(void) {
  return ROUND_UI ? 24 : 0;
}

static void chat_initials(const char *title, char *initials, size_t initials_size) {
  initials[0] = '\0';
  if (!title || !title[0] || initials_size < 2) {
    return;
  }
  initials[0] = toupper((unsigned char)title[0]);
  initials[1] = '\0';
  for (int i = 1; title[i] && initials_size > 2; i++) {
    if (title[i - 1] == ' ' && title[i] != ' ') {
      initials[1] = toupper((unsigned char)title[i]);
      initials[2] = '\0';
      return;
    }
  }
}

static Message *find_message_by_id(const char *message_id) {
  if (!message_id || !message_id[0]) {
    return NULL;
  }
  for (int i = 0; i < s_message_count; i++) {
    if (strcmp(s_messages[i].id, message_id) == 0) {
      return &s_messages[i];
    }
  }
  return NULL;
}

static Message *find_message_by_image_token(const char *image_token) {
  if (!image_token || !image_token[0]) {
    return NULL;
  }
  for (int i = 0; i < s_message_count; i++) {
    if (strcmp(s_messages[i].image_token, image_token) == 0) {
      return &s_messages[i];
    }
  }
  return find_message_by_id(image_token);
}

static int find_message_index_by_id(const char *message_id) {
  if (!message_id || !message_id[0]) {
    return -1;
  }
  for (int i = 0; i < s_message_count; i++) {
    if (strcmp(s_messages[i].id, message_id) == 0) {
      return i;
    }
  }
  return -1;
}

static bool message_stage_contains_id(const char *message_id) {
  if (!s_message_stage || !message_id || !message_id[0]) {
    return false;
  }
  for (int i = 0; i < s_message_stage_count; i++) {
    if (strcmp(s_message_stage[i].id, message_id) == 0) {
      return true;
    }
  }
  return false;
}

static int find_chat_index_by_id(const char *chat_id) {
  if (!s_chats || !chat_id || !chat_id[0]) {
    return -1;
  }
  for (int i = 0; i < s_chat_count; i++) {
    if (strcmp(s_chats[i].id, chat_id) == 0) {
      return i;
    }
  }
  return -1;
}

static void free_image_transfer_buffer(void) {
  if (s_image_buffer) {
    free(s_image_buffer);
    s_image_buffer = NULL;
  }
  s_image_buffer_capacity = 0;
}

static void free_avatar_transfer_buffer(void) {
  if (s_avatar_buffer) {
    free(s_avatar_buffer);
    s_avatar_buffer = NULL;
  }
  s_avatar_buffer_capacity = 0;
}

static bool ensure_transfer_buffer(uint8_t **buffer, uint16_t *capacity,
                                   int size, int max_size) {
  if (size <= 0 || size > max_size) {
    return false;
  }
  if (*buffer && *capacity >= size) {
    return true;
  }
  if (*buffer) {
    free(*buffer);
    *buffer = NULL;
    *capacity = 0;
  }
  *buffer = malloc(size);
  if (!*buffer) {
    return false;
  }
  *capacity = (uint16_t)size;
  return true;
}

static MEDIA_ONLY_UNUSED bool ensure_image_transfer_buffer(int image_size) {
  return ensure_transfer_buffer(&s_image_buffer, &s_image_buffer_capacity,
                                image_size, MAX_IMAGE_BYTES);
}

static MEDIA_ONLY_UNUSED bool ensure_avatar_transfer_buffer(int image_size) {
  return ensure_transfer_buffer(&s_avatar_buffer, &s_avatar_buffer_capacity,
                                image_size, MAX_AVATAR_BYTES);
}

static void reset_image_transfer_state(void) {
  free_image_transfer_buffer();
  s_image_message_id[0] = '\0';
  s_image_size = 0;
  s_image_received = 0;
  s_image_expected_offset = 0;
  s_image_transfer_id = 0;
  s_image_is_pbi = false;
}

static void reset_avatar_transfer_state(void) {
  free_avatar_transfer_buffer();
  s_avatar_chat_id[0] = '\0';
  s_avatar_size = 0;
  s_avatar_received = 0;
  s_avatar_expected_offset = 0;
  s_avatar_transfer_id = 0;
}

static MEDIA_ONLY_UNUSED bool transfer_chunk_fits(int offset, int length, int expected_offset,
                                                  int total_size, int capacity) {
  return offset == expected_offset &&
         offset >= 0 &&
         length > 0 &&
         total_size > 0 &&
         capacity >= total_size &&
         offset <= total_size &&
         length <= total_size - offset &&
         length <= capacity - offset;
}

static bool ensure_full_text_body(void) {
  if (s_full_text_body) {
    return true;
  }
  s_full_text_body = malloc(MAX_FULL_TEXT);
  if (!s_full_text_body) {
    return false;
  }
  s_full_text_body[0] = '\0';
  return true;
}

static void free_full_text_body(void) {
  if (s_full_text_body) {
    free(s_full_text_body);
    s_full_text_body = NULL;
  }
  s_full_text_context = false;
  s_full_text_unloaded_messages = false;
  s_full_text_message_id[0] = '\0';
  s_full_text_title[0] = '\0';
  s_full_text_scroll_offset = 0;
  s_full_text_height = 0;
}

static void destroy_chat_avatar(Chat *chat) {
  if (chat && chat->avatar_bitmap) {
    gbitmap_destroy(chat->avatar_bitmap);
    chat->avatar_bitmap = NULL;
  }
}

static void clear_chat_slot(Chat *chat) {
  if (!chat) {
    return;
  }
  destroy_chat_avatar(chat);
  release_chat_strings(chat);
  memset(chat, 0, sizeof(Chat));
  init_chat_strings(chat);
}

static void preserve_chat_avatar(Chat *chat, const char *incoming_id) {
  if (!s_chats) {
    clear_chat_slot(chat);
    return;
  }
  int existing_index = find_chat_index_by_id(incoming_id);
  if (existing_index < 0 || &s_chats[existing_index] == chat) {
    clear_chat_slot(chat);
    return;
  }
  Chat moved = s_chats[existing_index];
  s_chats[existing_index] = *chat;
  *chat = moved;
}

static void clear_chat_rows(void) {
  if (!s_chats) {
    reset_avatar_transfer_state();
    return;
  }
  for (int i = 0; i < MAX_CHATS; i++) {
    clear_chat_slot(&s_chats[i]);
  }
  s_chat_count = 0;
  reset_avatar_transfer_state();
}

static void destroy_message_bitmap(Message *message) {
#if !MEDIA_ENABLED
  return;
#else
  if (message && message->image_bitmap) {
    gbitmap_destroy(message->image_bitmap);
    message->image_bitmap = NULL;
    message->image_requested = false;
    if (s_loaded_image_count > 0) {
      s_loaded_image_count--;
    }
  }
  if (message && message->image_data) {
    free(message->image_data);
    message->image_data = NULL;
  }
#endif
}

static void refresh_loaded_image_count(void) {
#if !MEDIA_ENABLED
  s_loaded_image_count = 0;
  return;
#else
  s_loaded_image_count = 0;
  for (int i = 0; i < MAX_MESSAGES; i++) {
    if (s_messages[i].image_bitmap) {
      s_loaded_image_count++;
    }
  }
#endif
}

static void destroy_other_message_images(Message *keep) {
#if !MEDIA_ENABLED
  return;
#else
  for (int i = 0; i < MAX_MESSAGES; i++) {
    if (&s_messages[i] != keep) {
      destroy_message_bitmap(&s_messages[i]);
    }
  }
  refresh_loaded_image_count();
#endif
}

static void destroy_message_images(void) {
#if !MEDIA_ENABLED
  s_loaded_image_count = 0;
  reset_image_transfer_state();
  s_selected_image_focus_id[0] = '\0';
  return;
#else
  if (!s_messages) {
    s_loaded_image_count = 0;
    reset_image_transfer_state();
    return;
  }
  if (s_image_retry_timer) {
    app_timer_cancel(s_image_retry_timer);
    s_image_retry_timer = NULL;
  }
  for (int i = 0; i < MAX_MESSAGES; i++) {
    destroy_message_bitmap(&s_messages[i]);
    s_messages[i].image_requested = false;
    s_messages[i].image_failed = false;
    s_messages[i].image_error[0] = '\0';
    s_messages[i].image_progress = 0;
  }
  s_loaded_image_count = 0;
  reset_image_transfer_state();
  s_selected_image_focus_id[0] = '\0';
#endif
}

static void schedule_image_retry(void) {
#if !MEDIA_ENABLED
  return;
#endif
  if (!s_image_retry_timer && s_view_state == ViewStateChat && s_messages_root) {
    s_image_retry_timer = app_timer_register(IMAGE_COMMAND_RETRY_MS, image_retry_timer_callback, NULL);
  }
}

static void schedule_image_timeout(uint32_t timeout_ms) {
#if !MEDIA_ENABLED
  return;
#endif
  if (s_image_retry_timer) {
    app_timer_cancel(s_image_retry_timer);
  }
  if (s_view_state == ViewStateChat && s_messages_root) {
    s_image_retry_timer = app_timer_register(timeout_ms, image_retry_timer_callback, NULL);
  } else {
    s_image_retry_timer = NULL;
  }
}

static void schedule_image_prepare_timeout(void) {
  schedule_image_timeout(IMAGE_PREPARE_STALL_MS);
}

static MEDIA_ONLY_UNUSED void schedule_image_transfer_timeout(void) {
  schedule_image_timeout(IMAGE_TRANSFER_STALL_MS);
}

static bool message_needs_image(Message *message) {
#if !MEDIA_ENABLED
  return false;
#endif
  return message && message->image_placeholder && message->image_token[0] &&
         !message->image_requested && !message->image_failed && !message->image_bitmap;
}

static int message_image_display_width(const Message *message, int max_w) {
  int frame_w = PG_MAX(32, max_w);
  if (!message || message->image_width <= 0 || message->image_height <= 0) {
    return frame_w;
  }
  if (message->image_bitmap && message->image_width < frame_w) {
    return PG_MAX(32, (int)message->image_width);
  }
  return frame_w;
}

static int message_image_display_height(const Message *message, int max_w) {
  int w = message_image_display_width(message, max_w);
  if (message && !message->image_bitmap && message->image_requested && message->image_progress == 0) {
    return 52;
  }
  if (!message || message->image_width <= 0 || message->image_height <= 0) {
    return w;
  }
  int h = ((int)message->image_height * w) / (int)message->image_width;
  if (message->image_bitmap) {
    return PG_MAX(1, h);
  }
  int min_h = PG_MAX(32, (w * 3) / 4);
  int max_h = PG_MAX(min_h, w * IMAGE_TALL_MAX_MULTIPLIER);
  return PG_MAX(min_h, PG_MIN(max_h, h));
}

static int message_index_from_ptr(Message *message) {
  if (!message || !s_messages || message < s_messages || message >= s_messages + MAX_MESSAGES) {
    return -1;
  }
  return (int)(message - s_messages);
}

static MEDIA_ONLY_UNUSED void set_message_image_error(Message *message, const char *error) {
  if (!message) {
    return;
  }
  copy_cstr(message->image_error, sizeof(message->image_error), error && error[0] ? error : "Photo failed");
  message->image_progress = 0;
}

static void set_message_image_progress(Message *message, int percent) {
  if (!message) {
    return;
  }
  percent = PG_MAX(0, PG_MIN(100, percent));
  if (percent > message->image_progress) {
    message->image_progress = (uint8_t)percent;
  }
}

static bool message_image_near_viewport(int index, int margin) {
#if !MEDIA_ENABLED
  return false;
#else
  if (index < 0 || index >= s_message_count || !s_messages[index].image_placeholder) {
    return false;
  }
  if (!s_messages_root) {
    return false;
  }
  GRect bounds = layer_get_bounds(s_messages_root);
  int image_w = message_image_frame_width_for_index(index, bounds);
  int image_h = message_image_display_height(&s_messages[index], image_w);
  int image_top = s_message_y[index] + s_message_h[index] - image_h - 4;
  int image_bottom = image_top + image_h;
  return image_bottom >= s_chat_scroll_offset - margin &&
         image_top <= s_chat_scroll_offset + bounds.size.h + margin;
#endif
}

static bool message_image_visible(int index) {
  return message_image_near_viewport(index, 0);
}

static MEDIA_ONLY_UNUSED bool message_image_should_keep(int index) {
  if (index < 0 || index >= s_message_count) {
    return false;
  }
  if (index == s_selected_message) {
    return true;
  }
  return message_image_near_viewport(index, IMAGE_KEEP_SCREEN_MARGIN);
}

static int message_image_focus_distance(int index) {
#if !MEDIA_ENABLED
  return 0;
#else
  if (s_selected_message >= 0 && s_selected_message < s_message_count) {
    return abs(index - s_selected_message) * 1000;
  }
  if (!s_messages_root) {
    return index * 1000;
  }
  GRect bounds = layer_get_bounds(s_messages_root);
  int focus_y = s_chat_scroll_offset + (bounds.size.h / 2);
  int image_w = message_image_frame_width_for_index(index, bounds);
  int image_h = message_image_display_height(&s_messages[index], image_w);
  int image_y = s_message_y[index] + s_message_h[index] - (image_h / 2) - 4;
  return abs(image_y - focus_y);
#endif
}

static MEDIA_ONLY_UNUSED bool message_needs_decode_headroom(const Message *message, int image_size) {
  if (!message) {
    return image_size >= IMAGE_DECODE_HEADROOM_BYTES;
  }
  if (image_size >= IMAGE_DECODE_HEADROOM_BYTES) {
    return true;
  }
  if (!s_messages_root) {
    return false;
  }
  GRect bounds = layer_get_bounds(s_messages_root);
  int image_max_w = message_image_frame_width_for_index(message_index_from_ptr((Message *)message), bounds);
  int image_w = message_image_display_width(message, image_max_w);
  int image_h = message_image_display_height(message, image_max_w);
  return image_w * image_h >= IMAGE_DECODE_HEADROOM_PIXELS;
}

static size_t message_image_bitmap_heap_estimate(const Message *message) {
  if (!message || message->image_width == 0 || message->image_height == 0) {
    return 0;
  }
  size_t row_bytes = (size_t)((message->image_width + 3) & ~3);
  return row_bytes * message->image_height;
}

static MEDIA_ONLY_UNUSED bool message_image_decode_has_headroom(const Message *message) {
#ifdef _PBL_API_EXISTS_heap_bytes_free
  size_t estimate = message_image_bitmap_heap_estimate(message);
  if (estimate > 0) {
    size_t needed = estimate + IMAGE_DECODE_FINAL_HEADROOM_BYTES;
    size_t available = heap_bytes_free();
    if (available < needed) {
      return false;
    }
  }
#endif
  return true;
}

static unsigned image_request_decode_cost_budget(void) {
#ifdef _PBL_API_EXISTS_heap_bytes_free
  size_t available = heap_bytes_free();
  size_t reserve = IMAGE_DECODE_FINAL_HEADROOM_BYTES + 4096;
  if (available <= reserve) {
    return 0;
  }
  size_t budget = available - reserve;
  return (unsigned)PG_MIN(budget, 65000);
#else
  return 0;
#endif
}

#if IMAGE_DIAG_LOGS
static unsigned image_diag_heap_free(void) {
#ifdef _PBL_API_EXISTS_heap_bytes_free
  return (unsigned)heap_bytes_free();
#else
  return 0;
#endif
}
#endif

static MEDIA_ONLY_UNUSED bool message_is_gif(const Message *message) {
  return message && strncmp(message->text, "[GIF", 4) == 0;
}

static bool destroy_farthest_loaded_image(void) {
  int farthest_index = -1;
  int farthest_distance = -1;
  for (int i = 0; i < s_message_count; i++) {
    if (!s_messages[i].image_bitmap || message_image_visible(i)) {
      continue;
    }
    int distance = message_image_focus_distance(i);
    if (distance > farthest_distance) {
      farthest_distance = distance;
      farthest_index = i;
    }
  }
  if (farthest_index < 0) {
    return false;
  }
  destroy_message_bitmap(&s_messages[farthest_index]);
  return true;
}

static void prepare_selected_image_request(void) {
  if (s_selected_message < 0 || s_selected_message >= s_message_count ||
      !s_messages[s_selected_message].image_placeholder ||
      !s_messages[s_selected_message].image_token[0]) {
    s_selected_image_focus_id[0] = '\0';
    return;
  }
  Message *selected = &s_messages[s_selected_message];
  if (strcmp(s_selected_image_focus_id, selected->image_token) != 0) {
    copy_cstr(s_selected_image_focus_id, sizeof(s_selected_image_focus_id), selected->image_token);
    selected->image_failed = false;
    selected->image_error[0] = '\0';
    selected->image_progress = 0;
    selected->image_retry_level = 0;
  }

  if (s_image_message_id[0] && strcmp(s_image_message_id, selected->image_token) != 0) {
    clear_active_image_request();
  }
}

static void clear_active_image_request(void) {
  Message *message = find_message_by_image_token(s_image_message_id);
  if (s_image_retry_timer) {
    app_timer_cancel(s_image_retry_timer);
    s_image_retry_timer = NULL;
  }
  if (message) {
    message->image_requested = false;
  }
  if (s_image_message_id[0] && s_current_chat_id[0]) {
    send_command_with_status("cancel_image", s_current_chat_id, NULL, NULL, s_image_message_id, false);
  }
  reset_image_transfer_state();
}

static bool click_is_repeating(ClickRecognizerRef recognizer) {
  return recognizer && click_number_of_clicks_counted(recognizer) > 1;
}

static MEDIA_ONLY_UNUSED void sync_message_images(void) {
#if !MEDIA_ENABLED
  return;
#else
  for (int i = 0; i < MAX_MESSAGES; i++) {
    if (!message_image_should_keep(i)) {
      destroy_message_bitmap(&s_messages[i]);
      s_messages[i].image_failed = false;
      s_messages[i].image_error[0] = '\0';
      s_messages[i].image_progress = 0;
    }
  }

  if (s_image_message_id[0]) {
    Message *message = find_message_by_image_token(s_image_message_id);
    int image_index = message_index_from_ptr(message);
    if (!message_image_should_keep(image_index)) {
      clear_active_image_request();
    }
  }

  refresh_loaded_image_count();
  while (s_loaded_image_count > MAX_LOADED_IMAGES && destroy_farthest_loaded_image()) {
    refresh_loaded_image_count();
  }
  while (s_loaded_image_count > MAX_LOADED_IMAGES && destroy_unselected_loaded_image()) {
    refresh_loaded_image_count();
  }
#endif
}

static void destroy_offscreen_message_images(void) {
#if MEDIA_ENABLED
  sync_message_images();
#endif
}

static bool message_is_at_or_below_selection(int index) {
  return s_selected_message < 0 || s_selected_message >= s_message_count || index >= s_selected_message;
}

static bool selected_message_needs_image(void) {
  return s_selected_message >= 0 && s_selected_message < s_message_count &&
         s_messages[s_selected_message].image_placeholder &&
         s_messages[s_selected_message].image_token[0] &&
         !s_messages[s_selected_message].image_failed &&
         !s_messages[s_selected_message].image_bitmap;
}

static bool destroy_unselected_loaded_image(void) {
  int farthest_index = -1;
  int farthest_distance = -1;
  for (int i = 0; i < s_message_count; i++) {
    if (i == s_selected_message || !s_messages[i].image_bitmap) {
      continue;
    }
    int distance = message_image_focus_distance(i);
    if (distance > farthest_distance) {
      farthest_distance = distance;
      farthest_index = i;
    }
  }
  if (farthest_index < 0) {
    return false;
  }
  destroy_message_bitmap(&s_messages[farthest_index]);
  return true;
}

static int find_best_image_candidate(bool visible_only, bool prefer_below_selection) {
  int best_index = -1;
  int best_distance = 2147483647;
  for (int i = 0; i < s_message_count; i++) {
    if (!message_needs_image(&s_messages[i])) {
      continue;
    }
    if (visible_only) {
      if (!message_image_visible(i)) {
        continue;
      }
    } else if (!message_image_near_viewport(i, IMAGE_LOAD_SCREEN_MARGIN)) {
      continue;
    }
    if (prefer_below_selection && !message_is_at_or_below_selection(i)) {
      continue;
    }
    int distance = message_image_focus_distance(i);
    if (distance < best_distance) {
      best_distance = distance;
      best_index = i;
    }
  }
  return best_index;
}

static bool send_command_with_status(const char *command, const char *chat_id, const char *text,
                                     const char *reply_to, const char *message_id, bool show_failures) {
  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);
  DBG("PTDBG send_command begin cmd=%s chat=%s text=%s msg=%s gen=%lu heap=%u",
      command ? command : "", chat_id ? chat_id : "", text ? text : "",
      message_id ? message_id : "", (unsigned long)s_ui_generation, (unsigned)heap_bytes_free());
  if (result != APP_MSG_OK || !iter) {
    DBG("PTDBG send_command outbox_begin failed result=%d", (int)result);
    if (show_failures) {
      show_status("Bridge busy");
    }
    return false;
  }

  DictionaryResult dict_result = dict_write_cstring(iter, MESSAGE_KEY_Command, command);
  if (dict_result != DICT_OK) {
    DBG("PTDBG send_command command write failed dict=%d", (int)dict_result);
    if (show_failures) {
      show_status("Write fail");
    }
    return false;
  }
  if (chat_id) {
    dict_result = dict_write_cstring(iter, MESSAGE_KEY_ChatId, chat_id);
    if (dict_result != DICT_OK) {
      DBG("PTDBG send_command chat write failed dict=%d", (int)dict_result);
      if (show_failures) {
      show_status("ID fail");
      }
      return false;
    }
  }
  if (text) {
    dict_result = dict_write_cstring(iter, MESSAGE_KEY_Text, text);
    if (dict_result != DICT_OK) {
      DBG("PTDBG send_command text write failed dict=%d", (int)dict_result);
      if (show_failures) {
      show_status("Text fail");
      }
      return false;
    }
  }
  if (reply_to) {
    dict_result = dict_write_cstring(iter, MESSAGE_KEY_ReplyTo, reply_to);
    if (dict_result != DICT_OK) {
      DBG("PTDBG send_command reply write failed dict=%d", (int)dict_result);
      if (show_failures) {
      show_status("Reply fail");
      }
      return false;
    }
  }
  if (text && message_id && strcmp(command, "edit_message") == 0) {
    dict_result = dict_write_cstring(iter, MESSAGE_KEY_EditMessageId, message_id);
    if (dict_result != DICT_OK) {
      DBG("PTDBG send_command edit id write failed dict=%d", (int)dict_result);
      if (show_failures) {
      show_status("Edit fail");
      }
      return false;
    }
  }
  if (message_id) {
    dict_result = dict_write_cstring(iter, MESSAGE_KEY_MessageId, message_id);
    if (dict_result != DICT_OK) {
      DBG("PTDBG send_command msg id write failed dict=%d", (int)dict_result);
      if (show_failures) {
      show_status("Msg fail");
      }
      return false;
    }
  }
  dict_write_end(iter);
  result = app_message_outbox_send();
  DBG("PTDBG send_command sent cmd=%s result=%d heap=%u",
      command ? command : "", (int)result, (unsigned)heap_bytes_free());
  if (result != APP_MSG_OK) {
    if (show_failures) {
    show_status("Send fail");
    }
    return false;
  }
  return true;
}

static bool send_command(const char *command, const char *chat_id, const char *text,
                         const char *reply_to, const char *message_id) {
  return send_command_with_status(command, chat_id, text, reply_to, message_id, true);
}

static const char *default_status_text(void) {
  if (s_view_state == ViewStateChat && s_current_chat_title[0]) {
    return s_current_chat_title;
  }
  if (s_view_state == ViewStateChatList) {
    switch (s_list_mode) {
      case ListModeSubreddits:
        return "Pinned";
      case ListModeSorts:
        return "Sorts";
      case ListModeTasks:
        return s_viewing_completed_tasks ? "Saved" : "Posts";
      case ListModeGroupChats:
        return "Posts";
      case ListModeChats:
      default:
        break;
    }
  }
  return "Pebbit";
}

static bool status_message_should_persist(const char *message) {
  if (!message || !message[0] || strcmp(message, default_status_text()) == 0) {
    return true;
  }
  return strncmp(message, "Loading", 7) == 0 ||
         strncmp(message, "Connecting", 10) == 0 ||
         strncmp(message, "Fetching", 8) == 0 ||
         strncmp(message, "Requesting", 10) == 0;
}

static void cancel_status_clear(void) {
  if (s_status_clear_timer) {
    app_timer_cancel(s_status_clear_timer);
    s_status_clear_timer = NULL;
  }
}

static void schedule_status_clear(const char *message) {
  cancel_status_clear();
  if (!s_chats_loading && !status_message_should_persist(message)) {
    s_status_clear_timer = app_timer_register(STATUS_CLEAR_MS, status_clear_timer_callback, NULL);
  }
}

static void status_clear_timer_callback(void *data) {
  s_status_clear_timer = NULL;
  if (!s_chats_loading) {
    show_status(default_status_text());
  }
}

static void show_status(const char *message) {
  if (s_status_layer) {
    const char *shown = s_chats_loading ? "Pebbit" :
                        (message && message[0] ? message : default_status_text());
    copy_cstr(s_status_text, sizeof(s_status_text), shown);
    text_layer_set_text(s_status_layer, s_status_text);
    text_layer_set_text_color(s_status_layer, GColorWhite);
    text_layer_set_background_color(s_status_layer, APP_COLOR);
    schedule_status_clear(shown);
  }
}

static void interaction_unlock_timer_callback(void *data) {
  (void)data;
  s_interaction_unlock_timer = NULL;
  s_interaction_locked = false;
}

static void unlock_interactions_now(void) {
  if (s_interaction_unlock_timer) {
    app_timer_cancel(s_interaction_unlock_timer);
    s_interaction_unlock_timer = NULL;
  }
  s_interaction_locked = false;
}

static void lock_interactions(uint32_t delay_ms) {
  if (s_interaction_unlock_timer) {
    app_timer_cancel(s_interaction_unlock_timer);
    s_interaction_unlock_timer = NULL;
  }
  s_interaction_locked = true;
  s_interaction_unlock_timer = app_timer_register(delay_ms, interaction_unlock_timer_callback, NULL);
}

static bool interaction_busy(const char *message) {
  if (s_interaction_locked || s_action_window_pending_destroy ||
      s_chats_loading || s_loading_messages || s_chat_view_pending ||
      s_subreddit_intent_timer || s_subreddit_command_timer ||
      s_dictation_in_progress || s_dictation_confirm_timer ||
      s_dictation_failure_timer || s_dictation_start_timer || s_date_send_timer) {
    show_status(message && message[0] ? message : "One moment...");
    return true;
  }
  return false;
}

static void show_loading_text(const char *message, bool is_error) {
  copy_cstr(s_loading_text, sizeof(s_loading_text), message && message[0] ? message : "Loading...");
  s_loading_error = is_error;
  if (s_chat_menu) {
    menu_layer_reload_data(s_chat_menu);
  }
}

static bool status_is_microsoft_login(const char *status) {
  return status && (strncmp(status, "Code ", 5) == 0 ||
                    strcmp(status, "Open Pebbit settings") == 0 ||
                    strcmp(status, "Starting Reddit sign-in...") == 0);
}

static void show_microsoft_login_status(const char *status) {
  s_view_state = ViewStateChatList;
  s_list_mode = ListModeChats;
  s_chats_loading = true;
  s_loading_error = false;
  s_chat_loading_progress = 0;
  clear_chat_rows();
  s_selected_chat = 0;
  show_loading_text(status && status[0] ? status : "Starting Reddit sign-in...", false);
  if (s_chat_menu) {
    menu_layer_reload_data(s_chat_menu);
  }
}

static MEDIA_ONLY_UNUSED int progress_percent(int current, int total) {
  if (total <= 0) {
    return current > 0 ? 100 : 0;
  }
  return PG_MAX(0, PG_MIN(100, (current * 100) / total));
}

static MEDIA_ONLY_UNUSED int image_loading_phase_percent(const char *detail) {
  if (!detail || !detail[0]) {
    return 5;
  }
  if (strcmp(detail, "Waiting phone") == 0) {
    return 5;
  }
  if (strcmp(detail, "Preparing") == 0) {
    return 10;
  }
  if (strcmp(detail, "Downloading") == 0) {
    return 15;
  }
  if (strcmp(detail, "Decoding") == 0) {
    return 20;
  }
  if (strcmp(detail, "Sending") == 0) {
    return 25;
  }
  if (strcmp(detail, "Receiving") == 0) {
    return 25;
  }
  return 10;
}

static int chat_loading_percent(void) {
  int percent;
  if (s_expected_rows > 0) {
    percent = 90 + ((PG_MAX(0, PG_MIN(s_chat_count, s_expected_rows)) * 10) / s_expected_rows);
  } else if (strcmp(s_loading_text, "Connecting...") == 0) {
    percent = 15;
  } else if (strcmp(s_loading_text, "Fetching chats...") == 0) {
    percent = 45;
  } else if (strcmp(s_loading_text, "Sending chats...") == 0) {
    percent = 90;
  } else {
    percent = 8;
  }
  if (percent > s_chat_loading_progress) {
    s_chat_loading_progress = percent;
  }
  return s_chat_loading_progress;
}

static int subreddit_visible_row_count(void) {
  if (!subreddit_list_active() || s_chat_count <= 0 || !s_chat_menu) {
    return s_chat_count;
  }
  Layer *layer = menu_layer_get_layer(s_chat_menu);
  int row_h = ROUND_UI ? 48 : 52;
  int min_rows = (layer_get_bounds(layer).size.h + row_h - 1) / row_h;
  return PG_MAX(s_chat_count, min_rows);
}

static void draw_loading_bar(GContext *ctx, GRect rect, int percent) {
  percent = PG_MAX(0, PG_MIN(100, percent));
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_round_rect(ctx, rect, 2);
  if (percent > 0) {
    int pad = 3;
    int fill_w = ((rect.size.w - (pad * 2)) * percent) / 100;
    GRect fill = GRect(rect.origin.x + pad, rect.origin.y + pad,
                      PG_MAX(1, fill_w), rect.size.h - (pad * 2));
    graphics_context_set_fill_color(ctx, APP_COLOR);
    graphics_fill_rect(ctx, fill, 1, GCornersAll);
  }
}

static uint16_t chat_menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return 1;
}

static uint16_t chat_menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  if (s_chat_count > 0 && subreddit_list_active()) {
    return subreddit_visible_row_count();
  }
  return s_chat_count > 0 ? s_chat_count : 1;
}

static int16_t chat_menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return 0;
}

static void chat_menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  GRect bounds = layer_get_bounds(cell_layer);
  GRect safe = round_safe_rect(bounds);
  bool selected = menu_layer_is_index_selected(s_chat_menu, cell_index);
  bool subreddit = subreddit_list_active();
  bool subreddit_style = subreddit;
  bool microsoft_code = s_chats_loading && strncmp(s_loading_text, "Code ", 5) == 0;

  graphics_context_set_fill_color(ctx, subreddit ? PLANNER_BG : CHAT_BG);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  if (s_chat_count == 0) {
    const char *empty_title = microsoft_code ? "Reddit sign-in" :
                              (s_loading_error ? "Login needs attention" :
                               (s_bridge_ready ? ((s_list_mode == ListModeChats ||
                                                   s_list_mode == ListModeGroupChats) ?
                                                  "No chats yet" : "No items") : "Loading..."));
    graphics_context_set_text_color(ctx, GColorBlack);
    graphics_draw_text(ctx, empty_title,
                       fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
                       GRect(safe.origin.x, s_chats_loading ? (bounds.size.h / 2) - 34 : (bounds.size.h - 40) / 2,
                             safe.size.w, 40),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    if (microsoft_code) {
      graphics_context_set_text_color(ctx, GColorDarkGray);
      graphics_draw_text(ctx, "Open", fonts_get_system_font(FONT_KEY_GOTHIC_18),
                         GRect(safe.origin.x, (bounds.size.h / 2) - 4, safe.size.w, 22),
                         GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
      graphics_draw_text(ctx, "Pebbit settings", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
                         GRect(safe.origin.x, (bounds.size.h / 2) + 16, safe.size.w, 24),
                         GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
      graphics_context_set_text_color(ctx, APP_COLOR);
      graphics_draw_text(ctx, s_loading_text + 5, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD),
                         GRect(safe.origin.x, (bounds.size.h / 2) + 40, safe.size.w, 34),
                         GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    } else
    if (s_loading_error) {
      graphics_context_set_text_color(ctx, GColorDarkGray);
      graphics_draw_text(ctx, s_loading_text, fonts_get_system_font(FONT_KEY_GOTHIC_18),
                         GRect(safe.origin.x + 4, (bounds.size.h / 2), safe.size.w - 8, 44),
                         GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    } else if (s_chats_loading) {
      int bar_w = PG_MIN(safe.size.w - 24, 112);
      GRect bar = GRect(safe.origin.x + ((safe.size.w - bar_w) / 2), (bounds.size.h / 2) + 2,
                        bar_w, 14);
      draw_loading_bar(ctx, bar, chat_loading_percent());
      graphics_context_set_text_color(ctx, GColorDarkGray);
      graphics_draw_text(ctx, s_loading_text, fonts_get_system_font(FONT_KEY_GOTHIC_18),
                         GRect(safe.origin.x, bar.origin.y + 15, safe.size.w, 22),
                         GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    }
    return;
  }

  if (cell_index->row >= s_chat_count) {
    graphics_context_set_fill_color(ctx, subreddit ? PLANNER_BG : CHAT_BG);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
    return;
  }

  Chat *chat = &s_chats[cell_index->row];
  if (strcmp(chat->id, "__pin_divider") == 0) {
    graphics_context_set_fill_color(ctx, PLANNER_BG);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
    graphics_context_set_stroke_color(ctx, APP_COLOR);
    graphics_draw_line(ctx, GPoint(safe.origin.x + 8, bounds.size.h / 2),
                       GPoint(safe.origin.x + safe.size.w - 8, bounds.size.h / 2));
    return;
  }
  subreddit_style = subreddit || (s_list_mode == ListModeChats && strcmp(chat->id, "__subreddit") == 0);

  if (selected) {
    graphics_context_set_fill_color(ctx, subreddit_style ? APP_COLOR : APP_COLOR_LIGHT);
    graphics_fill_rect(ctx, GRect(safe.origin.x - 4, subreddit_style ? 3 : 1, safe.size.w + 8,
                                  bounds.size.h - (subreddit_style ? 6 : 3)),
                       subreddit_style || ROUND_UI ? 5 : 0, GCornersAll);
  } else if (subreddit_style) {
    graphics_context_set_fill_color(ctx, PLANNER_CARD);
    graphics_fill_rect(ctx, GRect(safe.origin.x + 2, 4, safe.size.w - 4, bounds.size.h - 8),
                       5, GCornersAll);
  }
  if (!subreddit_style) {
    graphics_context_set_stroke_color(ctx, GColorLightGray);
    graphics_draw_line(ctx, GPoint(safe.origin.x, bounds.size.h - 1),
                       GPoint(safe.origin.x + safe.size.w, bounds.size.h - 1));
  } else if (!selected) {
    graphics_context_set_stroke_color(ctx, PLANNER_CARD_ALT);
    graphics_draw_round_rect(ctx, GRect(safe.origin.x + 2, 4, safe.size.w - 4, bounds.size.h - 8), 5);
  }

  graphics_context_set_text_color(ctx, selected ? GColorWhite : GColorBlack);

  GFont title_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  int unread_w = chat->unread ? 24 : 0;
  int avatar_r = ROUND_UI ? 12 : 14;
  int avatar_cx = safe.origin.x + avatar_r + 1;
  int avatar_cy = bounds.size.h / 2;
  bool has_avatar = chat_row_has_avatar(chat);
  int text_x = has_avatar ? safe.origin.x + (avatar_r * 2) + 8 : safe.origin.x + (subreddit_style ? 10 : 4);
  int text_w = safe.size.w - (text_x - safe.origin.x) - unread_w;
  char initials[3];

  if (has_avatar) {
    GRect avatar_rect = GRect(avatar_cx - avatar_r, avatar_cy - avatar_r,
                              avatar_r * 2, avatar_r * 2);
    graphics_context_set_fill_color(ctx, GColorLightGray);
    graphics_fill_rect(ctx, avatar_rect, 0, GCornerNone);
    if (chat->avatar_bitmap) {
      graphics_draw_bitmap_in_rect(ctx, chat->avatar_bitmap, avatar_rect);
    }
    graphics_context_set_stroke_color(ctx, selected ? GColorWhite : APP_COLOR);
    graphics_draw_rect(ctx, avatar_rect);
    if (!chat->avatar_bitmap) {
      chat_initials(chat->title, initials, sizeof(initials));
      graphics_context_set_text_color(ctx, APP_COLOR);
      graphics_draw_text(ctx, initials, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                         GRect(avatar_cx - avatar_r, avatar_cy - 9, avatar_r * 2, 18),
                         GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    }
  } else if (subreddit_style && !selected) {
    graphics_context_set_fill_color(ctx, APP_COLOR);
    graphics_fill_rect(ctx, GRect(safe.origin.x + 2, 9, 3, bounds.size.h - 18), 1, GCornersAll);
  }

  graphics_context_set_text_color(ctx, selected ? GColorWhite : GColorBlack);
  graphics_draw_text(ctx, chat->title, title_font, GRect(text_x, subreddit_style ? 1 : -4, text_w, 25),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  graphics_context_set_text_color(ctx, selected ? GColorWhite : (subreddit_style ? GColorBlack : GColorDarkGray));
  graphics_draw_text(ctx, chat->preview, fonts_get_system_font(FONT_KEY_GOTHIC_18),
                     GRect(text_x, subreddit_style ? 25 : 20, text_w, 23), GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentLeft, NULL);
  if (chat->unread) {
    int cx = safe.origin.x + safe.size.w - 12;
    int cy = bounds.size.h / 2;
    graphics_context_set_fill_color(ctx, UNREAD_COLOR);
    if (chat->unread_count > 0) {
      graphics_fill_circle(ctx, GPoint(cx, cy), 10);
      char unread_text[12];
      if (chat->unread_count > 99) {
        copy_cstr(unread_text, sizeof(unread_text), "99+");
      } else {
        snprintf(unread_text, sizeof(unread_text), "%d", chat->unread_count);
      }
      graphics_context_set_text_color(ctx, GColorBlack);
      graphics_draw_text(ctx, unread_text, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                         GRect(cx - 10, cy - 10, 20, 18),
                         GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    } else {
      graphics_fill_circle(ctx, GPoint(cx, cy), 4);
    }
  }
}

static int16_t chat_menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  if (s_chat_count == 0) {
    Layer *layer = menu_layer_get_layer(menu_layer);
    return layer_get_bounds(layer).size.h;
  }
  if (subreddit_list_active()) {
    if (cell_index && cell_index->row < s_chat_count &&
        strcmp(s_chats[cell_index->row].id, "__pin_divider") == 0) {
      return 16;
    }
    return ROUND_UI ? 48 : 52;
  }
  if (cell_index && cell_index->row < s_chat_count &&
      strcmp(s_chats[cell_index->row].id, "__subreddit") == 0) {
    return ROUND_UI ? 48 : 52;
  }
  return ROUND_UI ? 42 : 46;
}

static void chat_menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  if (s_chats_loading || s_loading_messages || s_interaction_locked || s_action_window_pending_destroy) {
    show_status(s_loading_messages ? "Loading messages..." : "Loading...");
    return;
  }
  if (s_chat_count == 0) {
    lock_interactions(450);
    request_chats();
    return;
  }
  if (cell_index->row >= s_chat_count) {
    select_chat_row(s_chat_count - 1, true);
    return;
  }
  s_selected_chat = cell_index->row;
  if (strcmp(s_chats[s_selected_chat].id, "__pin_divider") == 0) {
    select_chat_row(PG_MIN(s_selected_chat + 1, s_chat_count - 1), true);
    return;
  }
  if (s_list_mode == ListModeChats &&
      strcmp(s_chats[s_selected_chat].id, "__subreddit") == 0) {
    lock_interactions(700);
    request_subreddits();
    return;
  }
  if ((s_list_mode == ListModeChats || s_list_mode == ListModeGroupChats) &&
      (strncmp(s_chats[s_selected_chat].id, "__feed_", 7) == 0 ||
       strcmp(s_chats[s_selected_chat].id, "__pin") == 0 ||
       strcmp(s_chats[s_selected_chat].id, "__subs") == 0)) {
    lock_interactions(700);
    request_group_chats(s_chats[s_selected_chat].id, s_chats[s_selected_chat].title);
    return;
  }
  if (s_list_mode == ListModeChats &&
      strcmp(s_chats[s_selected_chat].id, "__group_chats") == 0) {
    lock_interactions(700);
    request_group_chats(NULL, NULL);
    return;
  }
  if (s_list_mode == ListModeSubreddits) {
    lock_interactions(700);
    request_sorts(s_chats[s_selected_chat].id, s_chats[s_selected_chat].title);
    return;
  }
  if (s_list_mode == ListModeSorts) {
    lock_interactions(700);
    request_tasks(s_chats[s_selected_chat].id, s_chats[s_selected_chat].title);
    return;
  }
  if (s_list_mode == ListModeTasks) {
    if (strcmp(s_chats[s_selected_chat].id, "__new_task") == 0) {
      lock_interactions(500);
      show_status("Posting disabled");
      return;
    }
    if (strcmp(s_chats[s_selected_chat].id, "__completed_tasks") == 0) {
      lock_interactions(700);
      request_completed_tasks(s_current_bucket_id, s_current_bucket_title);
      return;
    }
    lock_interactions(700);
    request_task_detail(s_chats[s_selected_chat].id, s_chats[s_selected_chat].title);
    return;
  }
  lock_interactions(700);
  if (s_list_mode == ListModeGroupChats) {
    request_task_detail(s_chats[s_selected_chat].id, s_chats[s_selected_chat].title);
    return;
  }
  copy_cstr(s_current_chat_id, sizeof(s_current_chat_id), s_chats[s_selected_chat].id);
  copy_cstr(s_current_chat_title, sizeof(s_current_chat_title), s_chats[s_selected_chat].title);
  request_messages(s_current_chat_id);
}

static int clamp_scroll_offset(int offset) {
  if (!s_messages_root) {
    return 0;
  }
  GRect bounds = layer_get_bounds(s_messages_root);
  int visible_h = bounds.size.h;
#if TOUCH_KEYBOARD_AVAILABLE
  if (s_touch_keyboard_open) {
    visible_h = PG_MAX(1, visible_h - touch_keyboard_height());
  }
#endif
  int max_offset = PG_MAX(0, s_chat_content_height - visible_h);
  return PG_MAX(0, PG_MIN(offset, max_offset));
}


static bool message_has_context(Message *message) {
  return message && message->context[0];
}

static void copy_context_part(char *dest, size_t dest_size, const char *start, const char *end) {
  size_t len;
  if (!dest || dest_size == 0) {
    return;
  }
  if (!start) {
    dest[0] = '\0';
    return;
  }
  len = end && end > start ? (size_t)(end - start) : strlen(start);
  if (len >= dest_size) {
    len = dest_size - 1;
  }
  memcpy(dest, start, len);
  dest[len] = '\0';
  trim_incomplete_utf8(dest);
}

static int message_context_height(Message *message) {
  return message_has_context(message) ? PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 34, 32, 36, 32, 54, 50, 48) : 0;
}

static void message_context_strings(Message *message, char *title, size_t title_size,
                                    char *body, size_t body_size) {
  char *separator = strchr(message->context, '\n');
  copy_context_part(title, title_size, message->context, separator);
  copy_context_part(body, body_size, separator ? separator + 1 : "Message", NULL);
}

static void set_context_parts(Message *message, const char *sender, const char *text) {
  size_t used;
  char context[MAX_CONTEXT_TEXT];
  if (!message) {
    return;
  }
  context[0] = '\0';
  copy_cstr(context, sizeof(context), sender && sender[0] ? sender : "Reply");
  used = strlen(context);
  if (used + 1 >= sizeof(context)) {
    set_message_context_text(message, context);
    return;
  }
  context[used++] = '\n';
  context[used] = '\0';
  copy_cstr(context + used, sizeof(context) - used,
            text && text[0] ? text : "Message");
  set_message_context_text(message, context);
}

static void set_message_context(Message *message, const char *reply_sender, const char *reply_text,
                                const char *forward_sender, const char *forward_text) {
  const char *sender;
  const char *text;
  char sender_buf[MAX_CONTEXT_SENDER_TEXT + 4];
  char text_buf[MAX_CONTEXT_BODY_TEXT + 4];
  char fwd_sender_buf[MAX_CONTEXT_SENDER_TEXT + 4];
  if (reply_text && reply_text[0]) {
    sender = reply_sender && reply_sender[0] ? reply_sender : "Reply";
    text = reply_text;
    copy_cstr(sender_buf, sizeof(sender_buf), sender);
    truncate_cstr_bytes(sender_buf, sizeof(sender_buf), MAX_CONTEXT_SENDER_TEXT, "...");
    copy_cstr(text_buf, sizeof(text_buf), text);
    truncate_cstr_bytes(text_buf, sizeof(text_buf), MAX_CONTEXT_BODY_TEXT, "...");
    set_context_parts(message, sender_buf, text_buf);
    return;
  }
  if (forward_text && forward_text[0]) {
    sender = forward_sender && forward_sender[0] ? forward_sender : "Forwarded";
    text = forward_text;
    copy_cstr(sender_buf, sizeof(sender_buf), sender);
    copy_cstr(fwd_sender_buf, sizeof(fwd_sender_buf), "Fwd ");
    copy_cstr(fwd_sender_buf + strlen(fwd_sender_buf),
              sizeof(fwd_sender_buf) - strlen(fwd_sender_buf), sender_buf);
    truncate_cstr_bytes(fwd_sender_buf, sizeof(fwd_sender_buf), MAX_CONTEXT_SENDER_TEXT, "...");
    copy_cstr(text_buf, sizeof(text_buf), text);
    truncate_cstr_bytes(text_buf, sizeof(text_buf), MAX_CONTEXT_BODY_TEXT, "...");
    set_context_parts(message, fwd_sender_buf, text_buf);
    return;
  }
  set_message_context_text(message, "");
}


static void clear_message_slot(Message *message) {
  if (!message) {
    return;
  }
#if MEDIA_ENABLED
  if (s_image_message_id[0] && strcmp(s_image_message_id, message->image_token) == 0) {
    clear_active_image_request();
  }
  destroy_message_bitmap(message);
#endif
  release_message_strings(message);
  memset(message, 0, sizeof(Message));
  init_message_strings(message);
}

static void clear_message_rows(void) {
  if (!s_messages) {
    s_message_count = 0;
    s_selected_message = -1;
    reset_image_transfer_state();
    return;
  }
  for (int i = 0; i < MAX_MESSAGES; i++) {
    clear_message_slot(&s_messages[i]);
  }
  s_message_count = 0;
  s_selected_message = -1;
  s_selected_checklist_item = -1;
  s_expected_rows = 0;
  s_chat_scroll_offset = 0;
  s_chat_content_height = 0;
}

static void clear_pending_text_action(void) {
  s_pending_text[0] = '\0';
  s_pending_edit_message_id[0] = '\0';
  s_pending_chat_command[0] = '\0';
  s_pending_subreddit_source_id[0] = '\0';
  s_pending_subreddit_generation = 0;
  s_pending_send_as_reply = false;
}

static void clear_subreddit_refresh_anchor(void) {
  s_subreddit_refresh_anchor_id[0] = '\0';
  s_subreddit_refresh_anchor_checklist_item = -1;
  s_subreddit_refresh_anchor_valid = false;
}

static void capture_subreddit_refresh_anchor(void) {
  if (!subreddit_detail_active()) {
    clear_subreddit_refresh_anchor();
    return;
  }
  if (has_selected_message()) {
    copy_cstr(s_subreddit_refresh_anchor_id, sizeof(s_subreddit_refresh_anchor_id),
              s_messages[s_selected_message].id);
    s_subreddit_refresh_anchor_checklist_item = s_selected_checklist_item;
    s_subreddit_refresh_anchor_valid = true;
    return;
  }
  clear_subreddit_refresh_anchor();
}

static void clear_subreddit_intent(void) {
  if (s_subreddit_intent_timer) {
    app_timer_cancel(s_subreddit_intent_timer);
    s_subreddit_intent_timer = NULL;
  }
  memset(&s_subreddit_intent, 0, sizeof(s_subreddit_intent));
}

static void capture_subreddit_intent(ActionItem item, int index) {
  if (s_subreddit_intent_timer) {
    app_timer_cancel(s_subreddit_intent_timer);
    s_subreddit_intent_timer = NULL;
  }
  memset(&s_subreddit_intent, 0, sizeof(s_subreddit_intent));
  s_subreddit_intent.active = true;
  s_subreddit_intent.item = item;
  s_subreddit_intent.index = index;
  s_subreddit_intent.generation = s_ui_generation;
  s_subreddit_intent.checklist_item = s_selected_checklist_item;
  if (s_view_state == ViewStateChatList && s_selected_chat >= 0 && s_selected_chat < s_chat_count) {
    copy_cstr(s_subreddit_intent.chat_id, sizeof(s_subreddit_intent.chat_id), s_chats[s_selected_chat].id);
    copy_cstr(s_subreddit_intent.title, sizeof(s_subreddit_intent.title), s_chats[s_selected_chat].title);
  } else {
    copy_cstr(s_subreddit_intent.chat_id, sizeof(s_subreddit_intent.chat_id), s_current_chat_id);
    copy_cstr(s_subreddit_intent.title, sizeof(s_subreddit_intent.title), s_current_chat_title);
  }
  if (has_selected_message()) {
    copy_cstr(s_subreddit_intent.message_id, sizeof(s_subreddit_intent.message_id),
              s_messages[s_selected_message].id);
  }
}

static void schedule_captured_subreddit_intent(uint32_t delay_ms) {
  if (!s_subreddit_intent.active) {
    return;
  }
  if (s_subreddit_intent_timer) {
    app_timer_cancel(s_subreddit_intent_timer);
    s_subreddit_intent_timer = NULL;
  }
  s_subreddit_intent_timer = app_timer_register(delay_ms, subreddit_intent_timer_callback, NULL);
}

static void subreddit_command_timer_callback(void *data) {
  s_subreddit_command_timer = NULL;
  SubredditsWireCommand command = s_subreddit_wire_command;
  memset(&s_subreddit_wire_command, 0, sizeof(s_subreddit_wire_command));
  if (!command.active || !command.command[0] || !command.target_id[0]) {
    return;
  }
  send_command(command.command, command.target_id,
               command.text[0] ? command.text : NULL, NULL,
               command.message_id[0] ? command.message_id : NULL);
}

static void queue_subreddit_command(const char *command, const char *target_id,
                                  const char *text, const char *message_id) {
  if (!command || !command[0] || !target_id || !target_id[0]) {
    show_status("Action canceled");
    return;
  }
  if (s_subreddit_command_timer) {
    app_timer_cancel(s_subreddit_command_timer);
    s_subreddit_command_timer = NULL;
  }
  memset(&s_subreddit_wire_command, 0, sizeof(s_subreddit_wire_command));
  s_subreddit_wire_command.active = true;
  copy_cstr(s_subreddit_wire_command.command, sizeof(s_subreddit_wire_command.command), command);
  copy_cstr(s_subreddit_wire_command.target_id, sizeof(s_subreddit_wire_command.target_id), target_id);
  copy_cstr(s_subreddit_wire_command.text, sizeof(s_subreddit_wire_command.text), text);
  copy_cstr(s_subreddit_wire_command.message_id, sizeof(s_subreddit_wire_command.message_id), message_id);
  s_subreddit_command_timer = app_timer_register(650, subreddit_command_timer_callback, NULL);
}

static void cancel_deferred_subreddit_ui(void) {
  if (s_dictation_start_timer) {
    app_timer_cancel(s_dictation_start_timer);
    s_dictation_start_timer = NULL;
  }
  if (s_dictation_confirm_timer) {
    app_timer_cancel(s_dictation_confirm_timer);
    s_dictation_confirm_timer = NULL;
  }
  if (s_dictation_failure_timer) {
    app_timer_cancel(s_dictation_failure_timer);
    s_dictation_failure_timer = NULL;
  }
  if (s_date_send_timer) {
    app_timer_cancel(s_date_send_timer);
    s_date_send_timer = NULL;
  }
  if (s_subreddit_command_timer) {
    app_timer_cancel(s_subreddit_command_timer);
    s_subreddit_command_timer = NULL;
  }
  memset(&s_subreddit_wire_command, 0, sizeof(s_subreddit_wire_command));
  clear_subreddit_intent();
  s_date_command[0] = '\0';
  s_date_label[0] = '\0';
  s_date_target_id[0] = '\0';
  s_date_generation = 0;
  if (pending_command_is_subreddit_action()) {
    clear_pending_text_action();
  }
}

static void reset_message_stream_state(void) {
  s_message_stream_silent = false;
  s_message_stream_mode = MESSAGE_MODE_INITIAL;
}

static void clear_message_stage(void) {
  if (!s_message_stage) {
    s_message_stage_count = 0;
    return;
  }
  for (int i = 0; i < MAX_MESSAGES; i++) {
    clear_message_slot(&s_message_stage[i]);
  }
  free(s_message_stage);
  s_message_stage = NULL;
  s_message_stage_count = 0;
}

static bool prepare_message_stage(void) {
  clear_message_stage();
  s_message_stage = malloc(sizeof(Message) * MAX_MESSAGES);
  if (!s_message_stage) {
    destroy_message_images();
    s_message_stage = malloc(sizeof(Message) * MAX_MESSAGES);
  }
  if (!s_message_stage) {
    return false;
  }
  memset(s_message_stage, 0, sizeof(Message) * MAX_MESSAGES);
  for (int i = 0; i < MAX_MESSAGES; i++) {
    init_message_strings(&s_message_stage[i]);
  }
  s_message_stage_count = 0;
  return true;
}

static MEDIA_ONLY_UNUSED bool messages_match_image(const Message *a, const Message *b) {
#if !MEDIA_ENABLED
  return false;
#else
  return a && b && a->id[0] && b->id[0] && a->image_token[0] &&
         strcmp(a->id, b->id) == 0 && strcmp(a->image_token, b->image_token) == 0;
#endif
}

static void preserve_stage_image_state(void) {
#if !MEDIA_ENABLED
  return;
#else
  if (!s_message_stage) {
    return;
  }
  int rows = PG_MIN(s_message_stage_count, MAX_MESSAGES);
  for (int i = 0; i < rows; i++) {
    Message *stage = &s_message_stage[i];
    for (int j = 0; j < s_message_count; j++) {
      Message *current = &s_messages[j];
      if (!messages_match_image(stage, current)) {
        continue;
      }
      if (!stage->image_bitmap && current->image_bitmap) {
        stage->image_bitmap = current->image_bitmap;
        stage->image_data = current->image_data;
        current->image_bitmap = NULL;
        current->image_data = NULL;
      }
      stage->image_requested = current->image_requested;
      stage->image_failed = current->image_failed;
      stage->image_retry_level = current->image_retry_level;
      copy_cstr(stage->image_error, sizeof(stage->image_error), current->image_error);
      current->image_token[0] = '\0';
      break;
    }
  }
#endif
}

static void commit_message_stage(int count) {
  if (!s_message_stage) {
    return;
  }
  preserve_stage_image_state();
  clear_message_rows();
  int rows = PG_MIN(PG_MIN(count, s_message_stage_count), MAX_MESSAGES);
  for (int i = 0; i < rows; i++) {
    s_messages[i] = s_message_stage[i];
    memset(&s_message_stage[i], 0, sizeof(Message));
    init_message_strings(&s_message_stage[i]);
  }
  s_message_count = rows;
  free(s_message_stage);
  s_message_stage = NULL;
  s_message_stage_count = 0;
#if MEDIA_ENABLED
  refresh_loaded_image_count();
#endif
}

static bool message_transfer_matches(DictionaryIterator *iter) {
  int transfer_id = tuple_int(iter, MESSAGE_KEY_ImageTransferId, 0);
  return transfer_id == 0 || (s_message_transfer_id != 0 && transfer_id == s_message_transfer_id);
}

static void populate_message_from_tuple(Message *message, DictionaryIterator *iter) {
  char *incoming_message_id = tuple_cstring(iter, MESSAGE_KEY_MessageId);
  char *incoming_text = tuple_cstring(iter, MESSAGE_KEY_Text);
#if MEDIA_ENABLED
  char *incoming_image_token = tuple_cstring(iter, MESSAGE_KEY_ImageToken);
  bool preserve_image_state = incoming_message_id && incoming_image_token &&
                              strcmp(message->id, incoming_message_id) == 0 &&
                              strcmp(message->image_token, incoming_image_token) == 0;
  if (!preserve_image_state) {
    destroy_message_bitmap(message);
  }
#else
#endif
  copy_cstr(message->id, sizeof(message->id), incoming_message_id);
  set_message_strings(message,
                      tuple_cstring(iter, MESSAGE_KEY_Sender),
                      incoming_text,
                      tuple_cstring(iter, MESSAGE_KEY_Reactions),
                      tuple_cstring(iter, MESSAGE_KEY_MessageMeta),
                      NULL);
  message->text_truncated = incoming_text && strlen(incoming_text) >= MAX_TEXT;
  copy_cstr(message->section, sizeof(message->section), tuple_cstring(iter, MESSAGE_KEY_Section));
  set_message_context(message,
                      tuple_cstring(iter, MESSAGE_KEY_ReplySender),
                      tuple_cstring(iter, MESSAGE_KEY_ReplyText),
                      tuple_cstring(iter, MESSAGE_KEY_ForwardSender),
                      tuple_cstring(iter, MESSAGE_KEY_ForwardText));
  message->outgoing = tuple_int(iter, MESSAGE_KEY_IsOutgoing, 0) != 0;
#if MEDIA_ENABLED
  copy_cstr(message->image_token, sizeof(message->image_token), incoming_image_token);
  message->image_placeholder = message->image_token[0] != '\0';
  message->image_width = tuple_int(iter, MESSAGE_KEY_ImageWidth, message->image_placeholder ? IMAGE_THUMB_SIZE : 0);
  message->image_height = tuple_int(iter, MESSAGE_KEY_ImageHeight, message->image_placeholder ? IMAGE_THUMB_SIZE : 0);
  if (!preserve_image_state) {
    message->image_requested = false;
    message->image_failed = false;
    message->image_error[0] = '\0';
    message->image_progress = 0;
    message->image_retry_level = 0;
    message->image_bitmap = NULL;
  }
#else
  message->image_token[0] = '\0';
  message->image_placeholder = false;
  message->image_width = 0;
  message->image_height = 0;
#endif
}

static Message *prepend_message_slot(void) {
  if (s_message_count >= MAX_MESSAGES) {
    clear_message_slot(&s_messages[MAX_MESSAGES - 1]);
    s_message_count = MAX_MESSAGES - 1;
  }
  for (int i = s_message_count; i > 0; i--) {
    s_messages[i] = s_messages[i - 1];
  }
  memset(&s_messages[0], 0, sizeof(Message));
  init_message_strings(&s_messages[0]);
  s_message_count++;
  return &s_messages[0];
}

static Message *append_message_slot(void) {
  if (s_message_count >= MAX_MESSAGES) {
    clear_message_slot(&s_messages[0]);
    for (int i = 0; i < MAX_MESSAGES - 1; i++) {
      s_messages[i] = s_messages[i + 1];
    }
    s_message_count = MAX_MESSAGES - 1;
    if (s_selected_message > 0) {
      s_selected_message--;
    }
  }
  memset(&s_messages[s_message_count], 0, sizeof(Message));
  init_message_strings(&s_messages[s_message_count]);
  s_message_count++;
  return &s_messages[s_message_count - 1];
}

static void remove_message_at(int index) {
  char anchor_id[MAX_ID];
  int anchor_y = 0;
  anchor_id[0] = '\0';
  if (index < 0 || index >= s_message_count) {
    return;
  }
  bump_ui_generation();
  if (s_messages_root) {
    recalc_message_layout();
    int anchor_index = index + 1 < s_message_count ? index + 1 : index - 1;
    if (anchor_index >= 0 && anchor_index < s_message_count) {
      copy_cstr(anchor_id, sizeof(anchor_id), s_messages[anchor_index].id);
      anchor_y = s_message_y[anchor_index];
    }
  }
  clear_message_slot(&s_messages[index]);
  for (int i = index; i < s_message_count - 1; i++) {
    s_messages[i] = s_messages[i + 1];
  }
  memset(&s_messages[s_message_count - 1], 0, sizeof(Message));
  init_message_strings(&s_messages[s_message_count - 1]);
  s_message_count--;
  if (s_message_count <= 0) {
    s_selected_message = s_at_newest ? s_message_count : -1;
  } else if (index < s_message_count) {
    s_selected_message = index;
  } else {
    s_selected_message = s_message_count - 1;
  }
  if (s_messages_root) {
    if (anchor_id[0]) {
      preserve_stream_anchor(anchor_id, anchor_y, true);
    } else {
      recalc_message_layout();
      set_chat_scroll_offset(s_chat_scroll_offset, false);
      layer_mark_dirty(s_messages_root);
      request_next_image();
    }
  }
}

static void preserve_stream_anchor(const char *anchor_id, int anchor_y, bool dirty) {
  recalc_message_layout();
  if (anchor_id && anchor_id[0]) {
    int anchor_index = find_message_index_by_id(anchor_id);
    if (anchor_index >= 0) {
      s_selected_message = anchor_index;
      int target = s_chat_scroll_offset + (s_message_y[anchor_index] - anchor_y);
      if (dirty) {
        set_chat_scroll_offset(target, false);
      } else {
        set_chat_scroll_offset_quiet(target);
      }
    }
  }
  if (dirty && s_messages_root) {
    layer_mark_dirty(s_messages_root);
  }
  if (dirty) {
    request_next_image();
  }
}

static void render_after_stream_append(const char *anchor_id, int anchor_y) {
  if (s_message_stream_silent) {
    preserve_stream_anchor(anchor_id, anchor_y, false);
    return;
  }
  preserve_stream_anchor(anchor_id, anchor_y, true);
}

static void render_after_stream_prepend(const char *anchor_id, int anchor_y) {
  if (s_message_stream_silent) {
    preserve_stream_anchor(anchor_id, anchor_y, false);
    return;
  }
  if (subreddit_detail_active() && (!anchor_id || !anchor_id[0])) {
    recalc_message_layout();
    s_selected_message = s_message_count > 0 ? 0 : -1;
    s_selected_checklist_item = -1;
    set_chat_scroll_offset(0, false);
    return;
  }
  if (!s_user_scrolled_messages && (!anchor_id || !anchor_id[0])) {
    recalc_message_layout();
    scroll_to_bottom(false);
    return;
  }
  preserve_stream_anchor(anchor_id, anchor_y, true);
}


static uint8_t message_meta_receipts(const char *meta) {
  char *separator = meta ? strchr(meta, '|') : NULL;
  if (!separator || !separator[1]) {
    return 0;
  }
  return separator[1] == '2' ? 2 : 1;
}

static void message_meta_time(const char *meta, char *dest, size_t dest_size) {
  char *separator = meta ? strchr(meta, '|') : NULL;
  if (!dest || dest_size == 0) {
    return;
  }
  if (!meta) {
    dest[0] = '\0';
    return;
  }
  if (separator) {
    copy_context_part(dest, dest_size, meta, separator);
  } else {
    copy_cstr(dest, dest_size, meta);
  }
}

static void draw_receipt_tick(GContext *ctx, int x, int y) {
  graphics_draw_line(ctx, GPoint(x, y + 4), GPoint(x + 2, y + 6));
  graphics_draw_line(ctx, GPoint(x + 2, y + 6), GPoint(x + 6, y + 1));
}

static void draw_message_meta(GContext *ctx, const char *meta, GFont font, GRect rect) {
  char time_text[8];
  uint8_t receipts = message_meta_receipts(meta);
  int ticks_w = receipts ? (receipts == 2 ? 13 : 7) : 0;
  message_meta_time(meta, time_text, sizeof(time_text));
  graphics_context_set_text_color(ctx, BW_UI ? GColorBlack : GColorDarkGray);
  graphics_context_set_stroke_color(ctx, BW_UI ? GColorBlack : GColorDarkGray);
  if (time_text[0] && rect.size.w > ticks_w + 2) {
    graphics_draw_text(ctx, time_text, font,
                       GRect(rect.origin.x, rect.origin.y, rect.size.w - ticks_w - 2, rect.size.h),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);
  }
  if (receipts) {
    int tick_x = rect.origin.x + rect.size.w - ticks_w;
    int tick_y = rect.origin.y + 5;
    draw_receipt_tick(ctx, tick_x, tick_y);
    if (receipts > 1) {
      draw_receipt_tick(ctx, tick_x + 6, tick_y);
    }
  }
}

static void draw_message_context(GContext *ctx, Message *message, GRect rect) {
  char title[MAX_SENDER + 10];
  char body[MAX_CONTEXT_TEXT];
  if (!message_has_context(message) || rect.size.h <= 0) {
    return;
  }
  message_context_strings(message, title, sizeof(title), body, sizeof(body));
  bool quoted_self = cstr_equal_ignore_case(title, "You");
  GColor fill = BW_UI ? GColorWhite : (quoted_self ? OUT_CONTEXT_BUBBLE : IN_CONTEXT_BUBBLE);
  GColor accent = BW_UI ? GColorBlack : (quoted_self ? UNREAD_COLOR : GColorDarkGray);
  GColor title_color = BW_UI ? GColorBlack : (quoted_self ? SELF_CONTEXT_SENDER_COLOR : CHAT_SENDER_COLOR);
  GColor body_color = BW_UI ? GColorBlack : GColorBlack;
  GRect title_rect = GRect(rect.origin.x + 5, rect.origin.y, PG_MAX(1, rect.size.w - 8), 15);
  GRect body_rect = GRect(rect.origin.x + 5, rect.origin.y + 14,
                         PG_MAX(1, rect.size.w - 8), PG_MAX(1, rect.size.h - 15));
  truncate_cstr_bytes(title, sizeof(title), MAX_CONTEXT_SENDER_TEXT, "...");
  truncate_cstr_bytes(body, sizeof(body), MAX_CONTEXT_BODY_TEXT, "...");
  graphics_context_set_fill_color(ctx, fill);
  graphics_fill_rect(ctx, rect, 3, GCornersAll);
  graphics_context_set_fill_color(ctx, accent);
  if (rect.size.h > 4) {
    graphics_fill_rect(ctx, GRect(rect.origin.x, rect.origin.y + 2, 3, rect.size.h - 4), 1, GCornersAll);
  }
  graphics_context_set_text_color(ctx, title_color);
  graphics_draw_text(ctx, title, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                     title_rect,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  graphics_context_set_text_color(ctx, body_color);
  graphics_draw_text(ctx, body, fonts_get_system_font(FONT_KEY_GOTHIC_14),
                     body_rect,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
}

static bool message_is_checklist(Message *message) {
  return message && (strcmp(message->section, "checklist") == 0 ||
                     strcmp(message->sender, "Checklist") == 0);
}

static bool cstr_equal_ignore_case(const char *a, const char *b) {
  if (!a || !b) {
    return false;
  }
  while (*a && *b) {
    if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
      return false;
    }
    a++;
    b++;
  }
  return *a == '\0' && *b == '\0';
}

static bool message_kind_matches(Message *message, const char *kind) {
  return message && kind &&
         (strcmp(message->section, kind) == 0 ||
          strcmp(message->id, kind) == 0 ||
          cstr_equal_ignore_case(message->sender, kind));
}

static bool message_is_notes(Message *message) {
  return message_kind_matches(message, "notes") ||
         (message && strcmp(message->sender, "Notes") == 0);
}

static bool message_text_is_empty_field(Message *message) {
  if (!message || !message->text[0]) {
    return true;
  }
  return strncmp(message->text, "No ", 3) == 0;
}

static bool subreddit_post_has_field(const char *kind) {
  if (!kind) {
    return false;
  }
  for (int i = 0; i < s_message_count; i++) {
    Message *message = &s_messages[i];
    if (message->id[0] == '_' || !message_kind_matches(message, kind)) {
      continue;
    }
    if (message_is_checklist(message)) {
      return checklist_item_count(message->text) > 0;
    }
    return !message_text_is_empty_field(message);
  }
  return false;
}

static int checklist_item_count(const char *text) {
  if (!text || !text[0] || strcmp(text, "No checklist items") == 0) {
    return 0;
  }
  int count = 0;
  bool in_line = false;
  for (const char *p = text; p && *p; p++) {
    if (!in_line && *p != '\n' && *p != '\r' && *p != '|') {
      count++;
      in_line = true;
    } else if (*p == '\n' || *p == '\r' || *p == '|') {
      in_line = false;
    }
  }
  return count;
}

static int assigned_message_index(void) {
  return subreddit_message_index_for("assigned", "assigned");
}

static Message *assigned_message(void) {
  int index = assigned_message_index();
  return index >= 0 ? &s_messages[index] : NULL;
}

static int selected_assignee_count(void) {
  Message *message = assigned_message();
  if (!message) {
    return 0;
  }
  return checklist_item_count(message->text);
}

static AssigneeOption *assign_picker_options(bool teams) {
  AssigneeOption **options = teams ? &s_assign_team_options : &s_assign_member_options;
  if (!*options) {
    *options = malloc(sizeof(AssigneeOption) * MAX_ASSIGNEE_OPTIONS);
    if (*options) {
      memset(*options, 0, sizeof(AssigneeOption) * MAX_ASSIGNEE_OPTIONS);
    }
  }
  return *options;
}

static void clear_assign_picker_options(bool teams) {
  AssigneeOption *options = teams ? s_assign_team_options : s_assign_member_options;
  if (teams) {
    if (options) {
      memset(options, 0, sizeof(AssigneeOption) * MAX_ASSIGNEE_OPTIONS);
    }
    s_assign_team_option_count = 0;
  } else {
    if (options) {
      memset(options, 0, sizeof(AssigneeOption) * MAX_ASSIGNEE_OPTIONS);
    }
    s_assign_member_option_count = 0;
  }
}

static void remember_assign_picker_option(bool team, int index, const char *id, const char *title) {
  AssigneeOption *options = assign_picker_options(team);
  int *count = team ? &s_assign_team_option_count : &s_assign_member_option_count;
  if (index == 0) {
    clear_assign_picker_options(team);
  }
  if (!options || !id || !id[0] || !title || !title[0] || index < 0 || index >= MAX_ASSIGNEE_OPTIONS) {
    return;
  }
  copy_cstr(options[index].id, sizeof(options[index].id), id);
  copy_cstr(options[index].title, sizeof(options[index].title), title);
  if (index + 1 > *count) {
    *count = index + 1;
  }
}

static void clamp_subreddit_selection(void) {
  if (!subreddit_detail_active()) {
    s_checklist_edit_mode = false;
    s_selected_checklist_item = -1;
    return;
  }
  if (s_message_count <= 0) {
    s_selected_message = -1;
    s_selected_checklist_item = -1;
    s_checklist_edit_mode = false;
    return;
  }
  if (s_selected_message < 0 || s_selected_message >= s_message_count) {
    s_selected_message = PG_MAX(0, PG_MIN(s_selected_message, s_message_count - 1));
  }
  if (!has_selected_message() || !message_is_checklist(&s_messages[s_selected_message])) {
    s_selected_checklist_item = -1;
    s_checklist_edit_mode = false;
    return;
  }
  int items = checklist_item_count(s_messages[s_selected_message].text);
  if (s_checklist_edit_mode) {
    s_selected_checklist_item = PG_MAX(0, PG_MIN(s_selected_checklist_item, items));
  } else {
    s_selected_checklist_item = PG_MAX(-1, PG_MIN(s_selected_checklist_item, items));
  }
}

static void checklist_line_at(const char *text, int target, char *dest, size_t dest_size) {
  int index = 0;
  const char *start = text;
  const char *p = text;
  if (!dest || dest_size == 0) {
    return;
  }
  dest[0] = '\0';
  while (p && *p) {
    if (*p == '\n' || *p == '\r' || *p == '|') {
      if (index == target) {
        copy_context_part(dest, dest_size, start, p);
        return;
      }
      index++;
      start = p + 1;
    }
    p++;
  }
  if (index == target && start && *start) {
    copy_context_part(dest, dest_size, start, NULL);
  }
}

static void toggle_checklist_line(int target) {
  int index = 0;
  char *start = s_messages[s_selected_message].text;
  char *p = start;
  while (p && *p) {
    if (index == target) {
      if (strncmp(p, "[ ]", 3) == 0) {
        p[1] = 'x';
      } else if (strncmp(p, "[x]", 3) == 0 || strncmp(p, "[X]", 3) == 0) {
        p[1] = ' ';
      }
      return;
    }
    if (*p == '\n' || *p == '\r' || *p == '|') {
      index++;
    }
    p++;
  }
}

static bool subreddit_detail_row_is_top_post(int index) {
  return index == 0;
}

static int subreddit_detail_depth_for_index(int index) {
  if (index < 0 || index >= s_message_count || !s_messages[index].section[0] ||
      s_messages[index].section[0] != 'd') {
    return 0;
  }
  int depth = s_messages[index].section[1] - '0';
  return PG_MAX(0, PG_MIN(2, depth));
}

static int subreddit_detail_indent_for_index(int index) {
  if (subreddit_detail_row_is_top_post(index)) {
    return 4;
  }
  return (ROUND_UI ? 12 : 10) + subreddit_detail_depth_for_index(index) * 8;
}

static int subreddit_detail_bubble_width(GRect safe, int index) {
  int right_pad = subreddit_detail_row_is_top_post(index) ? 4 : 6;
  int width = safe.size.w - subreddit_detail_indent_for_index(index) - right_pad;
  return PG_MAX(24, width);
}

static bool message_is_thread_marker(Message *message) {
  return message && (strcmp(message->section, "more") == 0 ||
                     strcmp(message->section, "collapsed") == 0);
}

static bool metadata_has_saved(const char *text) {
  return text && strstr(text, "saved") != NULL;
}

static bool subreddit_target_saved(const char *chat_id, const char *message_id) {
  if (message_id && message_id[0]) {
    Message *message = find_message_by_id(message_id);
    return message && (metadata_has_saved(message->meta) || metadata_has_saved(message->reactions));
  }
  int chat_index = find_chat_index_by_id(chat_id && chat_id[0] ? chat_id : s_current_chat_id);
  return chat_index >= 0 && s_chats && s_chats[chat_index].unread_count > 0;
}

static bool selected_reddit_saved(void) {
  if (subreddit_detail_active() && has_selected_message()) {
    Message *message = &s_messages[s_selected_message];
    return metadata_has_saved(message->meta) || metadata_has_saved(message->reactions);
  }
  if (s_selected_chat >= 0 && s_selected_chat < s_chat_count && s_chats) {
    return s_chats[s_selected_chat].unread_count > 0;
  }
  return false;
}

#define draw_text_safe(ctx, text, font, rect, overflow, alignment) \
  graphics_draw_text(ctx, text, font, rect, overflow, alignment, NULL)

static int subreddit_post_height(Message *message, int text_w) {
  int header_h = message->sender[0] ? 18 : 0;
  int image_h = 0;
  int meta_h = (message->reactions[0] || message->meta[0]) ? 17 : 0;
  if (message_is_thread_marker(message)) {
    return 22;
  }
  if (message_is_checklist(message)) {
    return PG_MAX(64, header_h + (checklist_item_count(message->text) + 1) * 22 + 10);
  }
#if MEDIA_ENABLED
  if (message->image_placeholder) {
    image_h = message_image_display_height(message, text_w) + 8;
  }
#endif
  GSize size = GSize(0, 0);
  if (message->text[0] && text_w > 4) {
    size = graphics_text_layout_get_content_size(
      message->text,
      fonts_get_system_font(FONT_KEY_GOTHIC_18),
      GRect(0, 0, text_w, 2000),
      GTextOverflowModeWordWrap,
      GTextAlignmentLeft
    );
  }
  return PG_MAX(40, header_h + PG_MIN(size.h, MAX_TEXT * 2) + image_h + meta_h + 10);
}

static bool message_display_is_truncated(Message *message) {
  return message && (message->text_truncated || (int)strlen(message->text) > MESSAGE_PREVIEW_TEXT);
}

static void message_preview_text(Message *message, char *dest, size_t dest_size) {
  bool truncated = message_display_is_truncated(message);
  copy_cstr(dest, dest_size, message ? message->text : "");
  if (!truncated) {
    return;
  }
  if ((int)strlen(dest) > MESSAGE_PREVIEW_TEXT) {
    truncate_cstr_bytes(dest, dest_size, MESSAGE_PREVIEW_TEXT, " ...");
    return;
  }
  if (dest_size > 5 && strlen(dest) + 4 < dest_size) {
    strncat(dest, " ...", dest_size - strlen(dest) - 1);
  }
}

static int message_bubble_height(Message *message, int text_w, int bubble_w) {
  if (subreddit_detail_active()) {
    return subreddit_post_height(message, text_w);
  }
  char display_text[MESSAGE_PREVIEW_TEXT + 8];
  GFont text_font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  int name_h = (!message->outgoing && message->sender[0]) ? 16 : 0;
  int reaction_h = (message->reactions[0] || message->meta[0]) ? 17 : 0;
  int context_h = message_context_height(message);
  int image_h = 0;
#if MEDIA_ENABLED
  image_h = message->image_placeholder ?
            message_image_display_height(message, message_image_frame_width(bubble_w)) + 8 : 0;
#endif
  message_preview_text(message, display_text, sizeof(display_text));
  GSize size = GSize(0, 0);
  if (display_text[0] && text_w > 4) {
    size = graphics_text_layout_get_content_size(
      display_text,
      text_font,
      GRect(0, 0, text_w, 2000),
      GTextOverflowModeWordWrap,
      GTextAlignmentLeft
    );
  }
  int text_h = display_text[0] ? size.h : 0;
  text_h = PG_MAX(0, PG_MIN(text_h, MAX_TEXT * 2));
  return PG_MAX(28, text_h + name_h + context_h + image_h + reaction_h + 7);
}

static void recalc_message_layout(void) {
  if (!s_messages_root) {
    return;
  }

  GRect bounds = layer_get_bounds(s_messages_root);
#if TOUCH_KEYBOARD_AVAILABLE
  int visible_h = s_touch_keyboard_open ? PG_MAX(1, bounds.size.h - touch_keyboard_height()) :
                                          bounds.size.h;
#else
  int visible_h = bounds.size.h;
#endif
  int bubble_w = message_bubble_width(bounds);
  GRect safe = round_safe_rect(bounds);
  bool subreddit = subreddit_detail_active();
  int y = ROUND_UI ? 8 : 3;

  for (int i = 0; i < s_message_count; i++) {
    int row_bubble_w = subreddit ? subreddit_detail_bubble_width(safe, i) : bubble_w;
    int row_text_w = row_bubble_w - 10;
    s_message_y[i] = y;
    s_message_h[i] = message_bubble_height(&s_messages[i], row_text_w, row_bubble_w);
    y += s_message_h[i] + (subreddit ? (subreddit_detail_row_is_top_post(i) ? 7 : 5) :
                           (ROUND_UI ? 6 : 5));
  }
  int bottom_pad = ROUND_UI ? 12 : 5;
  int compose_min_y = visible_h - COMPOSE_BUBBLE_H - (ROUND_UI ? 8 : 6);
  bool reserve_compose_bubble = s_list_mode == ListModeChats && s_at_newest && !s_touch_keyboard_open;
  if (reserve_compose_bubble && s_message_count > 0 && y + COMPOSE_BUBBLE_GAP < compose_min_y) {
    int shift = compose_min_y - COMPOSE_BUBBLE_GAP - y;
    for (int i = 0; i < s_message_count; i++) {
      s_message_y[i] += shift;
    }
    y += shift;
  }
  if (reserve_compose_bubble) {
    s_compose_bubble_y = PG_MAX(y + COMPOSE_BUBBLE_GAP, compose_min_y);
    s_chat_content_height = s_compose_bubble_y + COMPOSE_BUBBLE_H + bottom_pad;
  } else {
    s_compose_bubble_y = y + COMPOSE_BUBBLE_GAP;
    s_chat_content_height = y + bottom_pad;
  }
  s_chat_scroll_offset = clamp_scroll_offset(s_chat_scroll_offset);
}

static void scroll_to_bottom(bool animated) {
  recalc_message_layout();
  s_selected_message = (s_list_mode == ListModeChats && s_at_newest) ?
                       s_message_count :
                       (s_message_count > 0 ? s_message_count - 1 : -1);
  s_selected_checklist_item = -1;
  set_chat_scroll_offset(s_chat_content_height, animated);
  destroy_offscreen_message_images();
  request_next_image();
}

static void go_to_bottom(void) {
  cancel_message_timeout();
  cancel_message_retry();
  s_loading_older_messages = false;
  s_loading_newer_messages = false;
  s_older_anchor_id[0] = '\0';
  s_newer_anchor_id[0] = '\0';
  s_older_anchor_y = 0;
  s_newer_anchor_y = 0;
  clear_message_stage();
  reset_message_stream_state();
  if (s_at_newest) {
    scroll_to_bottom(true);
  } else {
    s_user_scrolled_messages = false;
    s_message_scroll_direction = 1;
    request_newer_messages(false);
  }
}

static void chat_scroll_timer_callback(void *data) {
  s_chat_scroll_timer = NULL;
  s_chat_scroll_step++;

  if (s_chat_scroll_step >= CHAT_SCROLL_STEPS) {
    s_chat_scroll_offset = s_chat_scroll_target;
  } else {
    int delta = s_chat_scroll_target - s_chat_scroll_start;
    int progress = s_chat_scroll_step * CHAT_SCROLL_STEPS;
    int eased = (progress * 2) - (s_chat_scroll_step * s_chat_scroll_step);
    s_chat_scroll_offset = s_chat_scroll_start + ((delta * eased) / (CHAT_SCROLL_STEPS * CHAT_SCROLL_STEPS));
    s_chat_scroll_timer = app_timer_register(CHAT_SCROLL_FRAME_MS, chat_scroll_timer_callback, NULL);
  }

  if (s_messages_root) {
    layer_mark_dirty(s_messages_root);
  }
  if (!subreddit_detail_active()) {
    request_next_image();
  }
}

static void set_chat_scroll_offset_quiet(int target) {
  if (s_chat_scroll_timer) {
    app_timer_cancel(s_chat_scroll_timer);
    s_chat_scroll_timer = NULL;
  }
  s_chat_scroll_offset = clamp_scroll_offset(target);
}

static void set_chat_scroll_offset(int target, bool animated) {
  target = clamp_scroll_offset(target);
  if (s_chat_scroll_timer) {
    app_timer_cancel(s_chat_scroll_timer);
    s_chat_scroll_timer = NULL;
  }

  if (!animated) {
    s_chat_scroll_offset = target;
    if (s_messages_root) {
      layer_mark_dirty(s_messages_root);
    }
    request_next_image();
    return;
  }

  s_chat_scroll_start = s_chat_scroll_offset;
  s_chat_scroll_target = target;
  if (s_chat_scroll_start == s_chat_scroll_target) {
    if (s_messages_root) {
      layer_mark_dirty(s_messages_root);
    }
    request_next_image();
    return;
  }
  s_chat_scroll_step = 0;
  s_chat_scroll_timer = app_timer_register(CHAT_SCROLL_FRAME_MS, chat_scroll_timer_callback, NULL);
}

static void select_message_with_alignment(int index, bool align_top, bool animated) {
  if (!s_messages_root || index < 0 || index >= s_message_count) {
    return;
  }
  if (subreddit_detail_active()) {
    animated = false;
  }

  s_selected_message = index;
  s_selected_checklist_item = -1;
  recalc_message_layout();
  prepare_selected_image_request();
  GRect bounds = layer_get_bounds(s_messages_root);
  int margin = 6;
  int top = s_message_y[s_selected_message] - margin;
  int bottom = s_message_y[s_selected_message] + s_message_h[s_selected_message] + margin;
  int target = s_chat_scroll_offset;

  if (s_message_h[s_selected_message] > bounds.size.h - (margin * 2)) {
    target = align_top ? top : bottom - bounds.size.h;
    set_chat_scroll_offset(target, animated);
    request_next_image();
    return;
  }

  if (align_top && top < s_chat_scroll_offset) {
    target = top;
    set_chat_scroll_offset(target, animated);
    request_next_image();
    return;
  }

  if (!align_top && bottom > s_chat_scroll_offset + bounds.size.h) {
    target = bottom - bounds.size.h;
    set_chat_scroll_offset(target, animated);
    request_next_image();
    return;
  }

  if (s_messages_root) {
    layer_mark_dirty(s_messages_root);
  }
  request_next_image();
}

static void ensure_selected_checklist_item_visible(bool animated) {
  clamp_subreddit_selection();
  if (!subreddit_detail_active() || !has_selected_message() ||
      !message_is_checklist(&s_messages[s_selected_message]) || s_selected_checklist_item < -1 ||
      !s_messages_root) {
    return;
  }
  GRect bounds = layer_get_bounds(s_messages_root);
  int header_h = s_messages[s_selected_message].sender[0] ? 18 : 0;
  int row_top = s_selected_checklist_item < 0 ?
                s_message_y[s_selected_message] + 2 :
                s_message_y[s_selected_message] + 2 + header_h + (s_selected_checklist_item * 22);
  int row_bottom = row_top + (s_selected_checklist_item < 0 ? PG_MAX(18, header_h) : 22);
  int margin = 6;
  if (row_top - margin < s_chat_scroll_offset) {
    set_chat_scroll_offset(row_top - margin, animated);
  } else if (row_bottom + margin > s_chat_scroll_offset + bounds.size.h) {
    set_chat_scroll_offset(row_bottom + margin - bounds.size.h, animated);
  } else if (s_messages_root) {
    layer_mark_dirty(s_messages_root);
  }
}

static void render_messages(void) {
  if (!s_messages_root) {
    return;
  }
  recalc_message_layout();
  clamp_subreddit_selection();
  if (!has_selected_message() && s_at_newest && s_message_count > 0) {
    if (subreddit_detail_active()) {
      s_selected_message = 0;
      s_selected_checklist_item = -1;
      set_chat_scroll_offset(0, false);
      return;
    }
    s_selected_message = s_message_count;
    s_selected_checklist_item = -1;
    set_chat_scroll_offset(s_chat_content_height, false);
    return;
  }
  layer_mark_dirty(s_messages_root);
  request_next_image();
}

static GRect compose_rect_for_bounds(GRect bounds) {
  int compose_w = PG_MIN(bounds.size.w - 24, ROUND_UI ? 120 : 132);
  int compose_x = (bounds.size.w - compose_w) / 2;
  int compose_y = s_compose_bubble_y - s_chat_scroll_offset;
  return GRect(compose_x, compose_y, compose_w, COMPOSE_BUBBLE_H);
}

#if TOUCH_KEYBOARD_AVAILABLE
static int touch_keyboard_height(void) {
  return TOUCH_KEYBOARD_INPUT_H + (TOUCH_KEYBOARD_ROW_H * TOUCH_KEYBOARD_ROWS);
}

static GRect touch_keyboard_rect_for_bounds(GRect bounds) {
  int keyboard_h = touch_keyboard_height();
  return GRect(0, bounds.size.h - keyboard_h, bounds.size.w, keyboard_h);
}

static void close_touch_keyboard(void) {
  if (!s_touch_keyboard_open) {
    return;
  }
  s_touch_keyboard_open = false;
  s_touch_keyboard_symbols = false;
  s_touch_keyboard_shift = false;
  s_pending_text[0] = '\0';
  if (s_messages_root) {
    layer_mark_dirty(s_messages_root);
  }
}

static void open_touch_keyboard(void) {
  if (!TOUCH_KEYBOARD_ENABLED) {
    return;
  }
  s_touch_keyboard_open = true;
  s_touch_keyboard_symbols = false;
  s_touch_keyboard_shift = false;
  s_pending_text[0] = '\0';
  show_status("Type message");
  if (s_messages_root) {
    recalc_message_layout();
    set_chat_scroll_offset(s_chat_content_height, true);
    layer_mark_dirty(s_messages_root);
  }
}

static const char *touch_keyboard_chars_for_row(int row) {
  static const char *alpha[] = {"qwertyuiop", "asdfghjkl", "zxcvbnm"};
  static const char *symbols[] = {"1234567890", "-/:;()$&@", ".,!?'\"+"};
  return s_touch_keyboard_symbols ? symbols[row] : alpha[row];
}

static GRect touch_keyboard_key_rect(GRect keyboard_rect, int row, int start_unit,
                                     int unit_count, int total_units) {
  int row_y = keyboard_rect.origin.y + TOUCH_KEYBOARD_INPUT_H + (row * TOUCH_KEYBOARD_ROW_H);
  int left = (keyboard_rect.size.w * start_unit) / total_units;
  int right = (keyboard_rect.size.w * (start_unit + unit_count)) / total_units;
  return GRect(keyboard_rect.origin.x + left + 1, row_y + 1,
               PG_MAX(1, right - left - 2), TOUCH_KEYBOARD_ROW_H - 2);
}

static bool touch_keyboard_point_in_key(GRect keyboard_rect, GPoint point, int row,
                                        int start_unit, int unit_count, int total_units) {
  GRect rect = touch_keyboard_key_rect(keyboard_rect, row, start_unit, unit_count, total_units);
  return grect_contains_point(&rect, &point);
}

static char touch_keyboard_char_at(GRect keyboard_rect, GPoint point, char *action) {
  if (action) {
    *action = '\0';
  }
  if (!grect_contains_point(&keyboard_rect, &point) ||
      point.y < keyboard_rect.origin.y + TOUCH_KEYBOARD_INPUT_H) {
    return '\0';
  }

  for (int row = 0; row < 2; row++) {
    const char *chars = touch_keyboard_chars_for_row(row);
    int len = strlen(chars);
    for (int i = 0; i < len; i++) {
      if (touch_keyboard_point_in_key(keyboard_rect, point, row, i, 1, len)) {
        return chars[i];
      }
    }
  }

  const char *third_row = touch_keyboard_chars_for_row(2);
  if (touch_keyboard_point_in_key(keyboard_rect, point, 2, 0, 2, 11)) {
    if (action) *action = '^';
    return '\0';
  }
  for (int i = 0; i < 7; i++) {
    if (touch_keyboard_point_in_key(keyboard_rect, point, 2, i + 2, 1, 11)) {
      return third_row[i];
    }
  }
  if (touch_keyboard_point_in_key(keyboard_rect, point, 2, 9, 2, 11)) {
    if (action) *action = 'b';
    return '\0';
  }

  if (touch_keyboard_point_in_key(keyboard_rect, point, 3, 0, 2, 10)) {
    if (action) *action = 'm';
  } else if (touch_keyboard_point_in_key(keyboard_rect, point, 3, 2, 5, 10)) {
    if (action) *action = ' ';
  } else if (touch_keyboard_point_in_key(keyboard_rect, point, 3, 7, 3, 10)) {
    if (action) *action = '>';
  }
  return '\0';
}

static void append_touch_keyboard_char(char ch) {
  size_t current = strlen(s_pending_text);
  if (current + 1 >= TOUCH_KEYBOARD_MAX_TEXT) {
    show_status("Message full");
    return;
  }
  if (!s_touch_keyboard_symbols && s_touch_keyboard_shift && ch >= 'a' && ch <= 'z') {
    ch = (char)(ch - 'a' + 'A');
  }
  s_pending_text[current] = ch;
  s_pending_text[current + 1] = '\0';
  s_touch_keyboard_shift = false;
}

static void backspace_touch_keyboard_text(void) {
  size_t len = strlen(s_pending_text);
  if (len > 0) {
    s_pending_text[len - 1] = '\0';
  }
}

static void send_touch_keyboard_text(void) {
  if (!s_pending_text[0]) {
    show_status("Type message");
    return;
  }
  char text[TOUCH_KEYBOARD_MAX_TEXT];
  copy_cstr(text, sizeof(text), s_pending_text);
  copy_cstr(s_touch_keyboard_sent_text, sizeof(s_touch_keyboard_sent_text), text);
  s_touch_keyboard_open = false;
  s_touch_keyboard_symbols = false;
  s_touch_keyboard_shift = false;
  s_pending_text[0] = '\0';
  Message *slot = append_message_slot();
  copy_cstr(slot->id, sizeof(slot->id), "pending");
  set_message_strings(slot, "You", text, NULL, "...", NULL);
  slot->outgoing = true;
  s_at_newest = true;
  s_user_scrolled_messages = false;
  s_selected_message = s_message_count - 1;
  if (s_messages_root) {
    recalc_message_layout();
    set_chat_scroll_offset(s_chat_content_height, true);
  }
  send_text_message(text, false);
  if (s_messages_root) {
    layer_mark_dirty(s_messages_root);
  }
}

static void handle_touch_keyboard_key(char ch, char action) {
  if (ch) {
    append_touch_keyboard_char(ch);
  } else if (action == ' ') {
    append_touch_keyboard_char(' ');
  } else if (action == 'b') {
    backspace_touch_keyboard_text();
  } else if (action == '^') {
    s_touch_keyboard_shift = !s_touch_keyboard_shift;
  } else if (action == 'm') {
    s_touch_keyboard_symbols = !s_touch_keyboard_symbols;
    s_touch_keyboard_shift = false;
  } else if (action == '>') {
    send_touch_keyboard_text();
    return;
  }
  if (s_messages_root) {
    layer_mark_dirty(s_messages_root);
  }
}

static void draw_touch_keyboard_key(GContext *ctx, GRect rect, const char *label) {
  graphics_context_set_fill_color(ctx, GColorLightGray);
  graphics_fill_rect(ctx, rect, 0, GCornerNone);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_rect(ctx, rect);
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx, label, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                     GRect(rect.origin.x, rect.origin.y + 1, rect.size.w, rect.size.h - 1),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

static void draw_touch_keyboard_char_row(GContext *ctx, GRect keyboard_rect, int row) {
  const char *chars = touch_keyboard_chars_for_row(row);
  int len = strlen(chars);
  char label[2] = {'\0', '\0'};
  for (int i = 0; i < len; i++) {
    label[0] = chars[i];
    if (!s_touch_keyboard_symbols && s_touch_keyboard_shift && label[0] >= 'a' && label[0] <= 'z') {
      label[0] = (char)(label[0] - 'a' + 'A');
    }
    draw_touch_keyboard_key(ctx, touch_keyboard_key_rect(keyboard_rect, row, i, 1, len), label);
  }
}

static void draw_touch_keyboard(GContext *ctx, GRect bounds) {
  GRect keyboard_rect = touch_keyboard_rect_for_bounds(bounds);
  GRect input_rect = GRect(keyboard_rect.origin.x + 7, keyboard_rect.origin.y + 3,
                          keyboard_rect.size.w - 14, TOUCH_KEYBOARD_INPUT_H - 5);
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_fill_rect(ctx, keyboard_rect, 0, GCornerNone);
  graphics_context_set_fill_color(ctx, BW_UI ? GColorWhite : OUT_BUBBLE);
  graphics_fill_rect(ctx, input_rect, 6, GCornersAll);
  graphics_context_set_stroke_color(ctx, BW_UI ? GColorBlack : APP_COLOR);
  graphics_draw_round_rect(ctx, input_rect, 6);
  graphics_context_set_text_color(ctx, s_pending_text[0] ? GColorBlack : GColorDarkGray);
  graphics_draw_text(ctx, s_pending_text[0] ? s_pending_text : "Type...",
                     fonts_get_system_font(FONT_KEY_GOTHIC_18),
                     GRect(input_rect.origin.x + 5, input_rect.origin.y + 1,
                           input_rect.size.w - 10, input_rect.size.h - 2),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);

  draw_touch_keyboard_char_row(ctx, keyboard_rect, 0);
  draw_touch_keyboard_char_row(ctx, keyboard_rect, 1);
  draw_touch_keyboard_key(ctx, touch_keyboard_key_rect(keyboard_rect, 2, 0, 2, 11), "^");
  const char *third_row = touch_keyboard_chars_for_row(2);
  char label[2] = {'\0', '\0'};
  for (int i = 0; i < 7; i++) {
    label[0] = third_row[i];
    if (!s_touch_keyboard_symbols && s_touch_keyboard_shift && label[0] >= 'a' && label[0] <= 'z') {
      label[0] = (char)(label[0] - 'a' + 'A');
    }
    draw_touch_keyboard_key(ctx, touch_keyboard_key_rect(keyboard_rect, 2, i + 2, 1, 11), label);
  }
  draw_touch_keyboard_key(ctx, touch_keyboard_key_rect(keyboard_rect, 2, 9, 2, 11), "<");
  draw_touch_keyboard_key(ctx, touch_keyboard_key_rect(keyboard_rect, 3, 0, 2, 10),
                          s_touch_keyboard_symbols ? "ABC" : "#?");
  draw_touch_keyboard_key(ctx, touch_keyboard_key_rect(keyboard_rect, 3, 2, 5, 10), "space");
  draw_touch_keyboard_key(ctx, touch_keyboard_key_rect(keyboard_rect, 3, 7, 3, 10), "send");
}
#else
static void close_touch_keyboard(void) {
  s_touch_keyboard_open = false;
}
#endif

static void messages_root_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GRect safe = round_safe_rect(bounds);
  bool subreddit = subreddit_detail_active();
  graphics_context_set_fill_color(ctx, subreddit ? PLANNER_BG : CHAT_BG);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  if (s_message_count == 0) {
    graphics_context_set_text_color(ctx, GColorDarkGray);
    graphics_draw_text(ctx, s_loading_messages ? "Loading messages..." : "No messages loaded",
                       fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
                       GRect(8, 40, bounds.size.w - 16, 80), GTextOverflowModeWordWrap,
                       GTextAlignmentCenter, NULL);
  }

  GFont text_font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  GFont sender_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  GFont reaction_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  int first = 0;
  while (first < s_message_count - 1 &&
         s_message_y[first] + s_message_h[first] < s_chat_scroll_offset - 12) {
    first++;
  }
  if (checklist_edit_active()) {
    first = s_selected_message;
  }

  for (int i = first; i < s_message_count; i++) {
    if (checklist_edit_active() && i != s_selected_message) {
      break;
    }
    Message *message = &s_messages[i];
    char display_text[MESSAGE_PREVIEW_TEXT + 8];
    bool selected = i == s_selected_message;
    bool top_post = subreddit && subreddit_detail_row_is_top_post(i);
    int bubble_w = subreddit ? subreddit_detail_bubble_width(safe, i) : message_bubble_width(bounds);
    int text_w = bubble_w - 10;
    int inset = message_side_inset(bounds);
    int offset = ROUND_UI ? 6 : 0;
    int x = subreddit ? safe.origin.x + subreddit_detail_indent_for_index(i) :
            (message->outgoing ? bounds.size.w - bubble_w - inset + offset : inset - offset);
    x = PG_MAX(2, PG_MIN(x, bounds.size.w - bubble_w - 2));
    int name_h = (subreddit || !message->outgoing) && message->sender[0] ? (subreddit ? 18 : 16) : 0;
    int reaction_h = (message->reactions[0] || message->meta[0]) ? 17 : 0;
    int context_h = subreddit ? 0 : message_context_height(message);
    int y = s_message_y[i] - s_chat_scroll_offset;
    int bubble_h = s_message_h[i];

    if (y > bounds.size.h) {
      break;
    }
    if (y + bubble_h < 0) {
      continue;
    }

    message_preview_text(message, display_text, sizeof(display_text));

    if (subreddit && message_is_thread_marker(message)) {
      if (selected) {
        graphics_context_set_fill_color(ctx, BW_UI ? GColorWhite : SELECTOR_COLOR);
        graphics_fill_rect(ctx, GRect(x + 2, y, bubble_w - 4, bubble_h), 4, GCornersAll);
      }
      graphics_context_set_text_color(ctx, selected ? GColorWhite : (BW_UI ? GColorBlack : GColorDarkGray));
      draw_text_safe(ctx, display_text,
                     fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                     GRect(x + 4, y + 1, bubble_w - 8, bubble_h),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);
      continue;
    }

    GColor fill = BW_UI ? GColorWhite :
                  (subreddit ? (top_post ? GColorWhite : PLANNER_CARD) :
                   (message->outgoing ? OUT_BUBBLE : IN_BUBBLE));
    GRect bubble = GRect(x, y, bubble_w, bubble_h);
    GRect visible_bubble = bubble;
    if (subreddit && visible_bubble.origin.y < 0) {
      visible_bubble.size.h += visible_bubble.origin.y;
      visible_bubble.origin.y = 0;
    }
    if (visible_bubble.size.h <= 0) {
      continue;
    }

    graphics_context_set_fill_color(ctx, fill);
    graphics_fill_rect(ctx, subreddit ? visible_bubble : bubble,
                       subreddit && y < 0 ? 0 : 6, GCornersAll);

    GColor stroke = BW_UI ? GColorBlack :
                    (selected ? SELECTOR_COLOR :
                     (subreddit ? (top_post ? APP_COLOR : PLANNER_CARD_ALT) :
                      UNREAD_COLOR));
    graphics_context_set_stroke_color(ctx, stroke);
    if (subreddit && y < 0) {
      graphics_draw_rect(ctx, visible_bubble);
    } else {
      graphics_draw_round_rect(ctx, bubble, 6);
    }
    if (selected && (!subreddit || y >= 0)) {
      graphics_draw_round_rect(ctx, GRect(bubble.origin.x + 1, bubble.origin.y + 1,
                                          bubble.size.w - 2, bubble.size.h - 2), 5);
      if (BW_UI) {
        graphics_draw_round_rect(ctx, GRect(bubble.origin.x + 2, bubble.origin.y + 2,
                                            bubble.size.w - 4, bubble.size.h - 4), 4);
      }
    }
    if (subreddit && !top_post && visible_bubble.size.h > 8) {
      graphics_context_set_fill_color(ctx, selected ? SELECTOR_COLOR : APP_COLOR);
      graphics_fill_rect(ctx, GRect(x + 2, visible_bubble.origin.y + 4, 3,
                                    visible_bubble.size.h - 8), 1, GCornersAll);
    }

    int content_x = (subreddit && !top_post) ? x + 5 : x;
    int content_text_w = (subreddit && !top_post) ? PG_MAX(1, text_w - 5) : text_w;
    int text_y = y + 2;
    if (name_h) {
      graphics_context_set_text_color(ctx, BW_UI ? GColorBlack :
                                      (subreddit ? (top_post ? APP_COLOR : PLANNER_ITEM_NAME_COLOR) :
                                       CHAT_SENDER_COLOR));
      draw_text_safe(ctx, message->sender, sender_font,
                     GRect(content_x + 5, text_y, content_text_w, name_h),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft);
      text_y += name_h;
    }
    if (subreddit && message_is_checklist(message)) {
        int items = checklist_item_count(message->text);
        for (int item = 0; item < items; item++) {
          char line[96];
          checklist_line_at(message->text, item, line, sizeof(line));
          bool checked = strncmp(line, "[x]", 3) == 0 || strncmp(line, "[X]", 3) == 0;
          char *label = line;
          if (strncmp(line, "[ ]", 3) == 0 || checked) {
            label = line + 3;
            while (*label == ' ') {
              label++;
            }
          }
          GRect row = GRect(content_x + 4, text_y + (item * 22), content_text_w + 2, 21);
          bool item_selected = selected && item == s_selected_checklist_item;
          if (row.origin.y > bounds.size.h || row.origin.y + row.size.h < 0) {
            continue;
          }
          graphics_context_set_fill_color(ctx, BW_UI ? GColorWhite : PLANNER_ITEM_BG);
          graphics_fill_rect(ctx, row, 3, GCornersAll);
          graphics_context_set_stroke_color(ctx, BW_UI ? GColorBlack :
                                            (item_selected ? SELECTOR_COLOR : PLANNER_CARD_ALT));
          graphics_draw_round_rect(ctx, row, 3);
          if (item_selected) {
            graphics_draw_round_rect(ctx, GRect(row.origin.x + 1, row.origin.y + 1,
                                                row.size.w - 2, row.size.h - 2), 2);
          }
          GRect box = GRect(row.origin.x + 3, row.origin.y + 4, 12, 12);
          graphics_context_set_stroke_color(ctx, APP_COLOR);
          graphics_draw_round_rect(ctx, box, 2);
          if (checked) {
            graphics_draw_line(ctx, GPoint(box.origin.x + 2, box.origin.y + 6),
                               GPoint(box.origin.x + 5, box.origin.y + 9));
            graphics_draw_line(ctx, GPoint(box.origin.x + 5, box.origin.y + 9),
                               GPoint(box.origin.x + 10, box.origin.y + 3));
          }
          graphics_context_set_text_color(ctx, GColorBlack);
          draw_text_safe(ctx, label, fonts_get_system_font(FONT_KEY_GOTHIC_18),
                         GRect(row.origin.x + 20, row.origin.y - 1, row.size.w - 22, row.size.h + 2),
                         GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft);
        }
        GRect add_row = GRect(content_x + 4, text_y + (items * 22), content_text_w + 2, 21);
        bool add_selected = selected && s_selected_checklist_item == items;
        if (!(add_row.origin.y > bounds.size.h || add_row.origin.y + add_row.size.h < 0)) {
          graphics_context_set_fill_color(ctx, BW_UI ? GColorWhite : PLANNER_ITEM_BG);
          graphics_fill_rect(ctx, add_row, 3, GCornersAll);
          graphics_context_set_stroke_color(ctx, BW_UI ? GColorBlack :
                                            (add_selected ? SELECTOR_COLOR : PLANNER_CARD_ALT));
          graphics_draw_round_rect(ctx, add_row, 3);
          if (add_selected) {
            graphics_draw_round_rect(ctx, GRect(add_row.origin.x + 1, add_row.origin.y + 1,
                                                add_row.size.w - 2, add_row.size.h - 2), 2);
          }
          graphics_context_set_text_color(ctx, add_selected ? GColorWhite : GColorBlack);
          draw_text_safe(ctx, "+ Add an item", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
                         GRect(add_row.origin.x + 4, add_row.origin.y - 1,
                               add_row.size.w - 8, add_row.size.h + 2),
                         GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft);
        }
      continue;
    }
    if (context_h) {
      draw_message_context(ctx, message, GRect(x + 5, text_y + 1, text_w, context_h - 3));
      text_y += context_h;
    }
    graphics_context_set_text_color(ctx, GColorBlack);
    int image_h = 0;
#if MEDIA_ENABLED
    image_h = message->image_placeholder ?
              message_image_display_height(message, message_image_frame_width_for_index(i, bounds)) + 8 : 0;
#endif
    int text_rect_h = bubble_h - name_h - context_h - image_h - reaction_h - 6;
    if (display_text[0] && text_rect_h > 0 && content_text_w > 4) {
      draw_text_safe(ctx, display_text, text_font,
                     GRect(content_x + 5, text_y, content_text_w, text_rect_h),
                     GTextOverflowModeWordWrap, GTextAlignmentLeft);
    }

#if MEDIA_ENABLED
    if (message->image_placeholder) {
      int max_image_w = PG_MIN(message_image_frame_width_for_index(i, bounds), content_text_w);
      int image_w = message_image_display_width(message, max_image_w);
      int image_h = message_image_display_height(message, max_image_w);
      GRect image_rect = GRect(content_x + 5,
                              y + bubble_h - reaction_h - image_h - 4,
                              image_w, image_h);
      bool image_visible = image_rect.size.w > 0 && image_rect.size.h > 0 &&
                           image_rect.origin.y < bounds.size.h &&
                           image_rect.origin.y + image_rect.size.h > 0;
      if (image_visible && message->image_bitmap) {
        graphics_draw_bitmap_in_rect(ctx, message->image_bitmap, image_rect);
      } else if (image_visible) {
		        bool gif = message_is_gif(message);
		        const char *media_name = gif ? "GIF" : "Photo";
		        const char *label = message->image_failed ?
		                            (message->image_error[0] ? message->image_error : (gif ? "GIF failed" : "Photo failed")) :
		                            (message->image_requested ? "Loading..." : media_name);
		        const char *loading_detail = (!message->image_failed && message->image_requested && message->image_error[0]) ?
		                                     message->image_error : "";
        int image_percent = message->image_requested ? message->image_progress : 0;
	        graphics_context_set_stroke_color(ctx, BW_UI ? GColorBlack : GColorLightGray);
	        graphics_draw_rect(ctx, image_rect);
	        graphics_context_set_text_color(ctx, GColorBlack);
		        int requested_h = loading_detail[0] && image_rect.size.h >= 64 ? 58 : 42;
		        int label_h = message->image_failed ? PG_MIN(image_rect.size.h - 4, 46) : 24;
		        int label_y = image_rect.origin.y + PG_MAX(2, (image_rect.size.h - (message->image_requested ? requested_h : label_h)) / 2);
		        draw_text_safe(ctx, label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
		                       GRect(image_rect.origin.x + 4, label_y, image_rect.size.w - 8, label_h),
		                       message->image_failed ? GTextOverflowModeWordWrap : GTextOverflowModeTrailingEllipsis,
		                       GTextAlignmentCenter);
	        if (message->image_requested && image_rect.size.h >= 48) {
	          int bar_w = PG_MIN(image_rect.size.w - 20, 112);
	          GRect bar = GRect(image_rect.origin.x + ((image_rect.size.w - bar_w) / 2),
	                            label_y + 28, bar_w, 10);
	          draw_loading_bar(ctx, bar, image_percent);
	          if (loading_detail[0] && image_rect.size.h >= 64) {
	            graphics_context_set_text_color(ctx, GColorDarkGray);
	            draw_text_safe(ctx, loading_detail, fonts_get_system_font(FONT_KEY_GOTHIC_14),
	                           GRect(image_rect.origin.x + 4, label_y + 40, image_rect.size.w - 8, 18),
	                           GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);
	          }
	        }
	      }
	    }
#endif

    if (reaction_h > 0) {
      int meta_w = message->meta[0] ? (message->reactions[0] ? PG_MIN(74, text_w / 2) : text_w) : 0;
      graphics_context_set_text_color(ctx, BW_UI ? GColorBlack : GColorDarkGray);
      if (subreddit) {
        int side_w = message->reactions[0] ? PG_MIN(64, content_text_w / 2) : 0;
        if (message->meta[0]) {
          draw_text_safe(ctx, message->meta, reaction_font,
                         GRect(content_x + 7, y + bubble_h - reaction_h - 1,
                               PG_MAX(1, content_text_w - side_w - 4), reaction_h),
                         GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft);
        }
        if (message->reactions[0]) {
          draw_text_safe(ctx, message->reactions, reaction_font,
                         GRect(content_x + content_text_w - side_w, y + bubble_h - reaction_h - 1,
                               side_w, reaction_h),
                         GTextOverflowModeTrailingEllipsis, GTextAlignmentRight);
        }
      } else if (message->reactions[0]) {
        draw_text_safe(ctx, message->reactions, reaction_font,
                       GRect(x + 7, y + bubble_h - reaction_h - 1,
                             text_w - meta_w - 6, reaction_h),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft);
      }
      if (!subreddit && message->meta[0]) {
        draw_message_meta(ctx, message->meta, reaction_font,
                          GRect(x + bubble_w - meta_w - 7, y + bubble_h - reaction_h - 1,
                                meta_w, reaction_h));
      }
    }
  }

  GRect compose_rect = compose_rect_for_bounds(bounds);
  int compose_y = compose_rect.origin.y;
  bool compose_selected = compose_target_is_selected();
  if (s_list_mode == ListModeChats && s_at_newest && !s_touch_keyboard_open &&
      compose_y < bounds.size.h && compose_y + COMPOSE_BUBBLE_H > 0) {
    graphics_context_set_fill_color(ctx, BW_UI ? GColorWhite : GColorLightGray);
    graphics_fill_rect(ctx, compose_rect, COMPOSE_BUBBLE_H / 2, GCornersAll);
    graphics_context_set_stroke_color(ctx, BW_UI ? GColorBlack : (compose_selected ? SELECTOR_COLOR : GColorDarkGray));
    graphics_draw_round_rect(ctx, compose_rect, COMPOSE_BUBBLE_H / 2);
    if (compose_selected) {
      graphics_draw_round_rect(ctx, GRect(compose_rect.origin.x + 1, compose_rect.origin.y + 1,
                                          compose_rect.size.w - 2, compose_rect.size.h - 2),
                               (COMPOSE_BUBBLE_H / 2) - 1);
    }
    graphics_context_set_text_color(ctx, GColorBlack);
    graphics_draw_text(ctx, "New message", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
                       GRect(compose_rect.origin.x + 8, compose_rect.origin.y + 3,
                             compose_rect.size.w - 16, compose_rect.size.h - 5),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  }

#if TOUCH_KEYBOARD_AVAILABLE
  if (s_touch_keyboard_open) {
    draw_touch_keyboard(ctx, bounds);
  }
#endif
}

static void destroy_chat_view(void) {
  if (s_chat_menu_animation) {
    animation_unschedule((Animation *)s_chat_menu_animation);
    property_animation_destroy(s_chat_menu_animation);
    s_chat_menu_animation = NULL;
  }
  if (s_messages_animation) {
    animation_unschedule((Animation *)s_messages_animation);
    property_animation_destroy(s_messages_animation);
    s_messages_animation = NULL;
  }
  if (s_chat_scroll_timer) {
    app_timer_cancel(s_chat_scroll_timer);
    s_chat_scroll_timer = NULL;
  }
  if (s_messages_root) {
    layer_destroy(s_messages_root);
    s_messages_root = NULL;
  }
}

static void clear_layer_animation(PropertyAnimation **animation_ref) {
  if (*animation_ref) {
    animation_unschedule((Animation *)*animation_ref);
    property_animation_destroy(*animation_ref);
    *animation_ref = NULL;
  }
}

static void generic_layer_animation_stopped(Animation *animation, bool finished, void *context) {
  PropertyAnimation **animation_ref = (PropertyAnimation **)context;
  if (animation_ref && *animation_ref) {
    property_animation_destroy(*animation_ref);
    *animation_ref = NULL;
  }
}

static void chat_menu_slide_out_stopped(Animation *animation, bool finished, void *context) {
  generic_layer_animation_stopped(animation, finished, &s_chat_menu_animation);
  if (finished && s_view_state == ViewStateChat && s_chat_menu) {
    layer_set_hidden(menu_layer_get_layer(s_chat_menu), true);
  }
}

static void messages_slide_back_stopped(Animation *animation, bool finished, void *context) {
  generic_layer_animation_stopped(animation, finished, &s_messages_animation);
  if (finished && s_view_state == ViewStateChatList && s_messages_root) {
    layer_destroy(s_messages_root);
    s_messages_root = NULL;
  }
}

static void animate_layer_frame(PropertyAnimation **animation_ref, Layer *layer,
                                GRect from_frame, GRect to_frame,
                                AnimationStoppedHandler stopped_handler) {
  clear_layer_animation(animation_ref);
  layer_set_frame(layer, from_frame);
  *animation_ref = property_animation_create_layer_frame(layer, &from_frame, &to_frame);
  if (!*animation_ref) {
    layer_set_frame(layer, to_frame);
    return;
  }
  Animation *animation = (Animation *)*animation_ref;
  animation_set_duration(animation, VIEW_TRANSITION_MS);
  animation_set_curve(animation, AnimationCurveEaseOut);
  animation_set_handlers(animation, (AnimationHandlers) {
    .stopped = stopped_handler ? stopped_handler : generic_layer_animation_stopped
  }, animation_ref);
  animation_schedule(animation);
}

static void show_chat_view(void) {
  s_chat_view_pending = false;
  unlock_interactions_now();
  s_view_state = ViewStateChat;
  window_set_click_config_provider(s_main_window, click_config_provider);
  destroy_chat_view();
  show_status(s_current_chat_title);

  Layer *window_layer = window_get_root_layer(s_main_window);
  GRect bounds = layer_get_bounds(window_layer);
  int content_y = chat_content_y();
  int bottom_pad = chat_bottom_pad();
  GRect messages_to = GRect(0, content_y, bounds.size.w, bounds.size.h - content_y - bottom_pad);
  GRect messages_from = messages_to;
  messages_from.origin.x = bounds.size.w;
  s_messages_root = layer_create(messages_from);
  if (!s_messages_root) {
    s_view_state = ViewStateChatList;
    s_loading_messages = false;
    s_chat_view_pending = false;
    show_status("Memory low");
    if (s_chat_menu) {
      menu_layer_reload_data(s_chat_menu);
    }
    return;
  }
  layer_set_update_proc(s_messages_root, messages_root_update_proc);
  layer_add_child(window_layer, s_messages_root);
  s_chat_scroll_offset = 0;
  s_chat_content_height = 0;
  recalc_message_layout();
  if (subreddit_detail_active() && s_message_count > 0) {
    s_selected_message = 0;
    s_selected_checklist_item = -1;
    s_chat_scroll_offset = 0;
  } else if (s_message_count > 0) {
    s_chat_scroll_offset = clamp_scroll_offset(s_chat_content_height);
  }
  render_messages();
#if PT_DEBUG_FORCE_DUE_SELECTION
  if (subreddit_detail_active() && s_message_count > 0) {
    debug_focus_due_row();
  }
#endif
  animate_layer_frame(&s_messages_animation, s_messages_root, messages_from, messages_to, NULL);
  if (s_chat_menu) {
    Layer *menu_layer = menu_layer_get_layer(s_chat_menu);
    GRect menu_from = layer_get_frame(menu_layer);
    GRect menu_to = menu_from;
    menu_from.origin.x = 0;
    menu_to.origin.x = -bounds.size.w;
    layer_set_hidden(menu_layer, false);
    animate_layer_frame(&s_chat_menu_animation, menu_layer, menu_from, menu_to,
                        chat_menu_slide_out_stopped);
  }
}

static void show_chat_view_timer(void *data) {
  show_chat_view();
}

#if PT_DEBUG_FORCE_DUE_SELECTION
static void debug_auto_open_due_timer_callback(void *data) {
  (void)data;
  s_debug_due_open_timer = NULL;
  if (s_debug_due_opened || !subreddit_detail_active()) {
    return;
  }
  s_debug_due_opened = true;
  open_date_window("set_task_due", "Due in days");
}

static void debug_focus_due_row(void) {
  if (!subreddit_detail_active() || s_message_count <= 0) {
    return;
  }
  int due_index = subreddit_message_index_for("due", "due");
  if (due_index >= 0) {
    s_selected_message = due_index;
    s_selected_checklist_item = -1;
    recalc_message_layout();
    select_message_with_alignment(due_index, true, false);
    DBG("PT dfr i=%d r=%d g=%lu h=%u",
         due_index, s_message_count, (unsigned long)s_ui_generation, (unsigned)heap_bytes_free());
  } else {
    DBG("PT dfr nf r=%d g=%lu",
         s_message_count, (unsigned long)s_ui_generation);
  }
  if (!s_debug_due_opened && !s_debug_due_open_timer) {
    s_debug_due_open_timer = app_timer_register(500, debug_auto_open_due_timer_callback, NULL);
  }
}
#endif

static void render_chat_list_with_transition(void) {
  int row = s_selected_chat;
  s_view_state = ViewStateChatList;
  s_reddit_detail_request = false;
  s_chats_loading = false;
  s_loading_error = false;
  window_set_click_config_provider(s_main_window, click_config_provider);
  if (s_chat_scroll_timer) {
    app_timer_cancel(s_chat_scroll_timer);
    s_chat_scroll_timer = NULL;
  }
  Layer *window_layer = window_get_root_layer(s_main_window);
  GRect bounds = layer_get_bounds(window_layer);
  if (s_chat_menu) {
    int saved_row = find_chat_index_by_id(s_chat_list_selected_id);
    if (saved_row >= 0) {
      row = saved_row;
    }
    Layer *menu_layer = menu_layer_get_layer(s_chat_menu);
    GRect menu_to = layer_get_frame(menu_layer);
    menu_to.origin.x = 0;
    GRect menu_from = menu_to;
    menu_from.origin.x = -bounds.size.w;
    layer_set_hidden(menu_layer, false);
    menu_layer_reload_data(s_chat_menu);
    select_chat_row(row, false);
    animate_layer_frame(&s_chat_menu_animation, menu_layer, menu_from, menu_to, NULL);
  }
  if (s_messages_root) {
    GRect messages_from = layer_get_frame(s_messages_root);
    messages_from.origin.x = 0;
    GRect messages_to = messages_from;
    messages_to.origin.x = bounds.size.w;
    animate_layer_frame(&s_messages_animation, s_messages_root, messages_from, messages_to,
                        messages_slide_back_stopped);
  }
  show_status(default_status_text());
}

static bool chat_row_is_pin_divider(int row) {
  return row >= 0 && row < s_chat_count && strcmp(s_chats[row].id, "__pin_divider") == 0;
}

static int nearest_selectable_chat_row(int row, int direction) {
  if (s_chat_count <= 0) {
    return 0;
  }
  row = PG_MAX(0, PG_MIN(row, s_chat_count - 1));
  if (!chat_row_is_pin_divider(row)) {
    return row;
  }
  int step = direction < 0 ? -1 : 1;
  for (int candidate = row + step; candidate >= 0 && candidate < s_chat_count; candidate += step) {
    if (!chat_row_is_pin_divider(candidate)) {
      return candidate;
    }
  }
  for (int candidate = row - step; candidate >= 0 && candidate < s_chat_count; candidate -= step) {
    if (!chat_row_is_pin_divider(candidate)) {
      return candidate;
    }
  }
  return row;
}

static void select_chat_row(int row, bool animated) {
  if (!s_chat_menu || s_chats_loading || s_loading_messages || s_chat_count <= 0) {
    return;
  }
  s_selected_chat = nearest_selectable_chat_row(row, row < s_selected_chat ? -1 : 1);
  copy_cstr(s_chat_list_selected_id, sizeof(s_chat_list_selected_id), s_chats[s_selected_chat].id);
  menu_layer_set_selected_index(s_chat_menu, MenuIndex(0, s_selected_chat), MenuRowAlignCenter, animated);
}

static void reveal_available_chat_rows(void) {
  if (s_view_state == ViewStateChat || s_chat_count <= 0) {
    return;
  }
  s_chats_loading = false;
  s_loading_error = false;
  s_selected_chat = nearest_selectable_chat_row(PG_MAX(0, PG_MIN(s_selected_chat, s_chat_count - 1)), 1);
  if (s_chat_menu) {
    menu_layer_reload_data(s_chat_menu);
    select_chat_row(s_selected_chat, false);
  }
  if (!s_chat_first_paint_reported && s_list_mode == ListModeChats &&
      s_chat_count >= PG_MIN(CHAT_FIRST_PAINT_ROWS,
                             s_expected_rows > 0 ? s_expected_rows : CHAT_FIRST_PAINT_ROWS)) {
    s_chat_first_paint_reported = true;
    send_command_with_status("chat_first_paint", NULL, NULL, NULL, NULL, false);
  }
}

static void remove_chat_at(int row) {
  if (row < 0 || row >= s_chat_count) {
    return;
  }
  bump_ui_generation();
  clear_chat_slot(&s_chats[row]);
  for (int i = row; i < s_chat_count - 1; i++) {
    s_chats[i] = s_chats[i + 1];
  }
  memset(&s_chats[s_chat_count - 1], 0, sizeof(Chat));
  init_chat_strings(&s_chats[s_chat_count - 1]);
  s_chat_count--;
  if (s_selected_chat > row) {
    s_selected_chat--;
  }
  if (s_chat_count <= 0) {
    s_selected_chat = 0;
  } else if (s_selected_chat >= s_chat_count) {
    s_selected_chat = s_chat_count - 1;
  }
  if (s_chat_menu) {
    menu_layer_reload_data(s_chat_menu);
    select_chat_row(s_selected_chat, false);
  }
}

static void request_chats(void) {
  bump_ui_generation();
  DBG("PTDBG request_chats gen=%lu", (unsigned long)s_ui_generation);
  cancel_deferred_subreddit_ui();
  s_view_state = ViewStateChatList;
  s_list_mode = ListModeChats;
  s_current_plan_id[0] = '\0';
  s_current_plan_title[0] = '\0';
  s_current_bucket_id[0] = '\0';
  s_current_bucket_title[0] = '\0';
  s_viewing_completed_tasks = false;
  s_reddit_detail_request = false;
  cancel_message_timeout();
  if (s_chat_retry_timer) {
    app_timer_cancel(s_chat_retry_timer);
    s_chat_retry_timer = NULL;
  }
  if (s_chat_count == 0) {
    destroy_message_images();
    clear_chat_rows();
    s_chat_loading_progress = 0;
  }
  s_expected_rows = 0;
  s_bridge_ready = false;
  s_chats_loading = true;
  s_chat_first_paint_reported = false;
  s_chat_request_attempts = 1;
  show_loading_text("Loading...", false);
  show_status("Pebbit");
  if (s_chat_menu) {
    menu_layer_reload_data(s_chat_menu);
  }
  send_command_with_status("get_chats", NULL, NULL, NULL, NULL, false);
  if (s_chat_request_attempts < CHAT_COMMAND_MAX_ATTEMPTS) {
    s_chat_retry_timer = app_timer_register(CHAT_COMMAND_WAKE_RETRY_MS, chat_retry_timer_callback, NULL);
  }
  schedule_message_timeout();
}

static void request_list_rows(const char *command, const char *id, const char *title,
                              ListMode mode, const char *loading) {
  char id_copy[MAX_ID];
  char title_copy[48];
  copy_cstr(id_copy, sizeof(id_copy), id);
  copy_cstr(title_copy, sizeof(title_copy), title);
  bump_ui_generation();
  DBG("PTDBG request_list_rows cmd=%s id=%s mode=%d gen=%lu",
      command ? command : "", id_copy, mode, (unsigned long)s_ui_generation);
  cancel_deferred_subreddit_ui();
  s_view_state = ViewStateChatList;
  s_list_mode = mode;
  s_reddit_detail_request = false;
  cancel_message_timeout();
  cancel_message_retry();
  if (s_chat_retry_timer) {
    app_timer_cancel(s_chat_retry_timer);
    s_chat_retry_timer = NULL;
  }
  destroy_message_images();
  clear_chat_rows();
  s_selected_chat = 0;
  s_chat_list_selected_id[0] = '\0';
  s_chat_refresh_selected_id[0] = '\0';
  s_expected_rows = 0;
  s_bridge_ready = false;
  s_chats_loading = true;
  s_chat_request_attempts = CHAT_COMMAND_MAX_ATTEMPTS;
  show_loading_text(loading && loading[0] ? loading : "Loading...", false);
  show_status(default_status_text());
  if (s_chat_menu) {
    menu_layer_reload_data(s_chat_menu);
  }
  send_command_with_status(command, id_copy[0] ? id_copy : NULL,
                           title_copy[0] ? title_copy : NULL,
                           NULL, NULL, false);
  schedule_message_timeout();
}

static void request_subreddits(void) {
  request_list_rows("get_subreddits", NULL, NULL, ListModeSubreddits, "Loading pinned");
}

static void request_group_chats(const char *feed_id, const char *feed_title) {
  const char *loading = "Loading posts...";
  if (feed_id && strcmp(feed_id, "__subs") == 0) {
    loading = "Loading subs...";
  } else if (feed_id && strcmp(feed_id, "__pin") == 0) {
    loading = "Loading pinned";
  }
  request_list_rows("get_group_chats", feed_id, feed_title, ListModeGroupChats, loading);
}

static void request_sorts(const char *plan_id, const char *plan_title) {
  copy_cstr(s_current_plan_id, sizeof(s_current_plan_id), plan_id);
  copy_cstr(s_current_plan_title, sizeof(s_current_plan_title), plan_title);
  s_viewing_completed_tasks = false;
  s_bucket_option_count = 0;
  request_list_rows("get_sorts", plan_id, NULL, ListModeSorts, "Loading sorts...");
}

static void cache_visible_sorts(void) {
  if (s_list_mode != ListModeSorts) {
    return;
  }
  if (!s_bucket_options) {
    s_bucket_options = malloc(sizeof(BucketOption) * MAX_BUCKET_OPTIONS);
  }
  if (!s_bucket_options) {
    s_bucket_option_count = 0;
    return;
  }
  s_bucket_option_count = PG_MIN(s_chat_count, MAX_BUCKET_OPTIONS);
  for (int i = 0; i < s_bucket_option_count; i++) {
    copy_cstr(s_bucket_options[i].id, sizeof(s_bucket_options[i].id), s_chats[i].id);
    copy_cstr(s_bucket_options[i].title, sizeof(s_bucket_options[i].title), s_chats[i].title);
  }
}

static void request_tasks(const char *bucket_id, const char *bucket_title) {
  cache_visible_sorts();
  copy_cstr(s_current_bucket_id, sizeof(s_current_bucket_id), bucket_id);
  copy_cstr(s_current_bucket_title, sizeof(s_current_bucket_title), bucket_title);
  s_viewing_completed_tasks = false;
  s_checklist_edit_mode = false;
  request_list_rows("get_tasks", bucket_id, NULL, ListModeTasks, "Loading posts...");
}

static void request_completed_tasks(const char *bucket_id, const char *bucket_title) {
  copy_cstr(s_current_bucket_id, sizeof(s_current_bucket_id), bucket_id);
  copy_cstr(s_current_bucket_title, sizeof(s_current_bucket_title), bucket_title);
  s_viewing_completed_tasks = true;
  s_checklist_edit_mode = false;
  request_list_rows("get_completed_tasks", bucket_id, NULL, ListModeTasks, "Loading saved...");
}

static void request_task_detail(const char *task_id, const char *task_title) {
  bump_ui_generation();
  DBG("PT detail g=%lu h=%u", (unsigned long)s_ui_generation, (unsigned)heap_bytes_free());
  cancel_deferred_subreddit_ui();
  copy_cstr(s_current_chat_id, sizeof(s_current_chat_id), task_id);
  copy_cstr(s_current_chat_title, sizeof(s_current_chat_title), task_title);
  cancel_message_timeout();
  cancel_message_retry();
  close_touch_keyboard();
  s_checklist_edit_mode = false;
  s_reddit_detail_request = true;
  destroy_message_images();
  reset_image_transfer_state();
  reset_avatar_transfer_state();
  clear_message_stage();
  s_loading_older_messages = false;
  s_loading_newer_messages = false;
  s_older_anchor_id[0] = '\0';
  s_newer_anchor_id[0] = '\0';
  s_older_anchor_y = 0;
  s_newer_anchor_y = 0;
  s_at_newest = true;
  s_at_oldest = true;
  s_message_scroll_direction = 0;
  s_message_count = 0;
  s_expected_rows = 0;
  s_message_transfer_id = 0;
  reset_message_stream_state();
  s_selected_message = -1;
  s_selected_checklist_item = -1;
  s_user_scrolled_messages = false;
  s_chat_view_pending = false;
  s_loading_messages = true;
  s_message_request_attempts = 1;
  show_status("Loading post...");
  if (send_command_with_status("get_task_detail", task_id, NULL, NULL, NULL, false)) {
    schedule_message_timeout();
  } else {
    schedule_message_send_retry();
  }
}

static void chat_retry_timer_callback(void *data) {
  s_chat_retry_timer = NULL;
  if (!s_chats_loading || s_view_state == ViewStateChat || s_bridge_ready ||
      s_chat_request_attempts >= CHAT_COMMAND_MAX_ATTEMPTS) {
    return;
  }

  s_chat_request_attempts++;
  send_command_with_status("get_chats", NULL, NULL, NULL, NULL, false);
  if (s_chat_request_attempts < CHAT_COMMAND_MAX_ATTEMPTS) {
    s_chat_retry_timer = app_timer_register(CHAT_COMMAND_WAKE_RETRY_MS, chat_retry_timer_callback, NULL);
  }
}

static void startup_wake_timer_callback(void *data) {
  s_startup_wake_timer = NULL;
#if PT_DEBUG_OPEN_PLANNER_CARD
    DBG("PT startup h=%u", (unsigned)heap_bytes_free());
    s_list_mode = ListModeTasks;
    s_viewing_completed_tasks = false;
    s_debug_due_opened = false;
    request_task_detail("mock-task-intune", "Approve Intune rollout");
    return;
#endif
  if (s_view_state != ViewStateChat && s_chats_loading) {
    request_chats();
  }
}

static void cancel_message_timeout(void) {
  if (s_message_timeout_timer) {
    app_timer_cancel(s_message_timeout_timer);
    s_message_timeout_timer = NULL;
  }
}

static void schedule_message_timeout(void) {
  cancel_message_timeout();
  s_message_timeout_timer = app_timer_register(MESSAGE_TRANSFER_TIMEOUT_MS, message_timeout_timer_callback, NULL);
}

static void cancel_message_retry(void) {
  if (s_message_retry_timer) {
    app_timer_cancel(s_message_retry_timer);
    s_message_retry_timer = NULL;
  }
}

static bool send_pending_message_request(void) {
  if (!s_current_chat_id[0]) {
    return false;
  }
  if (s_loading_messages) {
    if (s_reddit_detail_request || s_list_mode == ListModeTasks) {
      return send_command_with_status("get_task_detail", s_current_chat_id, NULL, NULL, NULL, false);
    }
    return send_command_with_status("get_messages", s_current_chat_id, NULL, NULL, NULL, false);
  }
  if (s_loading_older_messages && s_message_count > 0 && s_messages[0].id[0]) {
    const char *anchor_id = s_older_anchor_id[0] ? s_older_anchor_id : s_messages[0].id;
    return send_command_with_status("get_older_messages", s_current_chat_id, NULL,
                                    s_messages[0].id, anchor_id, false);
  }
  if (s_loading_newer_messages && s_message_count > 0 && s_messages[s_message_count - 1].id[0]) {
    const char *anchor_id = s_newer_anchor_id[0] ? s_newer_anchor_id :
                            s_messages[s_message_count - 1].id;
    return send_command_with_status("get_newer_messages", s_current_chat_id, NULL,
                                    s_messages[s_message_count - 1].id, anchor_id, false);
  }
  return false;
}

static void message_retry_timer_callback(void *data) {
  s_message_retry_timer = NULL;
  if (!s_loading_messages && !s_loading_older_messages && !s_loading_newer_messages) {
    return;
  }
  if (s_message_request_attempts >= MESSAGE_COMMAND_MAX_ATTEMPTS) {
    bool had_rows = s_message_count > 0;
    s_loading_messages = false;
    s_loading_older_messages = false;
    s_loading_newer_messages = false;
    unlock_interactions_now();
    s_message_transfer_id = 0;
    s_older_anchor_id[0] = '\0';
    s_newer_anchor_id[0] = '\0';
    clear_message_stage();
    reset_message_stream_state();
    show_status(had_rows ? s_current_chat_title : "Messages failed");
    if (s_messages_root) {
      layer_mark_dirty(s_messages_root);
    }
    return;
  }
  s_message_request_attempts++;
  if (send_pending_message_request()) {
    schedule_message_timeout();
  } else {
    schedule_message_send_retry();
  }
}

static void schedule_message_send_retry(void) {
  cancel_message_retry();
  s_message_retry_timer = app_timer_register(MESSAGE_COMMAND_WAKE_RETRY_MS,
                                             message_retry_timer_callback, NULL);
}

static void message_timeout_timer_callback(void *data) {
  s_message_timeout_timer = NULL;
  if (s_chats_loading && s_view_state != ViewStateChat) {
    s_chats_loading = false;
    s_loading_error = true;
    unlock_interactions_now();
    show_loading_text((s_list_mode == ListModeChats || s_list_mode == ListModeGroupChats) ?
                      "Chats failed" : "Pinned failed", true);
    show_status(default_status_text());
    if (s_chat_menu) {
      menu_layer_reload_data(s_chat_menu);
    }
    return;
  }
  if (!s_loading_messages && !s_loading_older_messages && !s_loading_newer_messages) {
    return;
  }
  bool had_rows = s_message_count > 0;
  s_loading_messages = false;
  s_loading_older_messages = false;
  s_loading_newer_messages = false;
  unlock_interactions_now();
  s_message_transfer_id = 0;
  s_older_anchor_id[0] = '\0';
  s_newer_anchor_id[0] = '\0';
  clear_message_stage();
  reset_message_stream_state();
  show_status(had_rows ? s_current_chat_title : "Messages failed");
  if (s_messages_root) {
    layer_mark_dirty(s_messages_root);
  }
}

// Load the first photo that is visible or just about to enter view. This keeps
// heap use predictable while still prefetching as the user scrolls.
static bool send_active_image_request(void) {
#if !MEDIA_ENABLED
  return false;
#endif
  if (!s_image_message_id[0]) {
    return false;
  }
  Message *message = find_message_by_image_token(s_image_message_id);
  if (!message || message->image_bitmap) {
    clear_active_image_request();
    return false;
  }
  int image_index = message_index_from_ptr(message);
  if (image_index != s_selected_message &&
      !message_image_near_viewport(image_index, IMAGE_KEEP_SCREEN_MARGIN)) {
    IMAGE_DIAG("PGIMG watch request cancel offscreen msg=%s index=%d selected=%d",
               message->image_token, image_index, s_selected_message);
    clear_active_image_request();
    return false;
  }
  unsigned budget = image_request_decode_cost_budget();
  char request_text[24];
  snprintf(request_text, sizeof(request_text), "%u:%u:%u",
           (unsigned)message->image_retry_level,
           budget,
           (unsigned)MAX_IMAGE_BYTES);
  IMAGE_DIAG("PGIMG watch request msg=%s attempt=%u index=%d selected=%d budget=%u heap=%u",
             message->image_token, (unsigned)message->image_retry_level,
             image_index, s_selected_message, budget, image_diag_heap_free());
  if (send_command_with_status("get_image", s_current_chat_id, request_text, NULL, message->image_token, false)) {
    schedule_image_prepare_timeout();
    return true;
  }
  schedule_image_retry();
  return false;
}

static MEDIA_ONLY_UNUSED bool retry_active_image_request(Message *message, const char *detail) {
#if !MEDIA_ENABLED
  return false;
#endif
  int image_index = message_index_from_ptr(message);
  if (!message || !message->image_token[0] ||
      image_index < 0 ||
      (image_index != s_selected_message &&
       !message_image_near_viewport(image_index, IMAGE_KEEP_SCREEN_MARGIN)) ||
      message->image_retry_level >= IMAGE_RETRY_MAX_LEVEL) {
    IMAGE_DIAG("PGIMG watch retry blocked msg=%s index=%d selected=%d attempt=%u detail=%s",
               message ? message->image_token : "(null)", image_index, s_selected_message,
               message ? (unsigned)message->image_retry_level : 0,
               detail && detail[0] ? detail : "");
    return false;
  }

  IMAGE_DIAG("PGIMG watch retry msg=%s fromAttempt=%u detail=%s heap=%u",
             message->image_token, (unsigned)message->image_retry_level,
             detail && detail[0] ? detail : "", image_diag_heap_free());
  if (s_image_retry_timer) {
    app_timer_cancel(s_image_retry_timer);
    s_image_retry_timer = NULL;
  }
  reset_image_transfer_state();
  copy_cstr(s_image_message_id, sizeof(s_image_message_id), message->image_token);
  message->image_retry_level++;
  message->image_requested = true;
  message->image_failed = false;
  message->image_progress = 0;
  copy_cstr(message->image_error, sizeof(message->image_error),
            detail && detail[0] ? detail : "Resizing");
  set_message_image_progress(message, 12);
  if (!send_active_image_request()) {
    schedule_image_retry();
  }
  if (s_messages_root) {
    layer_mark_dirty(s_messages_root);
  }
  return true;
}

static void request_next_image(void) {
#if !MEDIA_ENABLED
  return;
#endif
  if (!s_messages_root || s_message_count == 0) {
    return;
  }
  refresh_loaded_image_count();
  if (subreddit_detail_active() && !selected_message_needs_image()) {
    if (s_image_message_id[0]) {
      clear_active_image_request();
    }
    refresh_loaded_image_count();
    return;
  }

  if (s_image_message_id[0]) {
    Message *active_message = find_message_by_image_token(s_image_message_id);
    int active_index = message_index_from_ptr(active_message);
    if ((active_index != s_selected_message &&
         !message_image_near_viewport(active_index, IMAGE_KEEP_SCREEN_MARGIN)) ||
        (selected_message_needs_image() && active_index != s_selected_message)) {
      IMAGE_DIAG("PGIMG watch active cleared msg=%s active=%d selected=%d selectedNeeds=%d",
                 s_image_message_id, active_index, s_selected_message,
                 selected_message_needs_image() ? 1 : 0);
      clear_active_image_request();
    } else {
      return;
    }
  }

  if (s_image_message_id[0]) {
    return;
  }

  int image_index = selected_message_needs_image() ? s_selected_message : -1;
  if (subreddit_detail_active() && image_index < 0) {
    return;
  }
  if (image_index < 0) {
    image_index = find_best_image_candidate(true, true);
  }
  if (image_index < 0) {
    image_index = find_best_image_candidate(true, false);
  }

  if (image_index < 0) {
    return;
  }

  Message *message = &s_messages[image_index];
  IMAGE_DIAG("PGIMG watch candidate msg=%s index=%d selected=%d loaded=%d failed=%d heap=%u",
             message->image_token, image_index, s_selected_message, s_loaded_image_count,
             message->image_failed ? 1 : 0, image_diag_heap_free());
  if (image_index == s_selected_message && s_loaded_image_count > 0) {
    destroy_other_message_images(message);
  }
  refresh_loaded_image_count();

  if (s_loaded_image_count >= MAX_LOADED_IMAGES) {
    if (destroy_farthest_loaded_image()) {
      refresh_loaded_image_count();
    }
    if (s_loaded_image_count >= MAX_LOADED_IMAGES && image_index == s_selected_message &&
        destroy_unselected_loaded_image()) {
      refresh_loaded_image_count();
    }
    if (s_loaded_image_count >= MAX_LOADED_IMAGES) {
      return;
    }
  }

  message->image_failed = false;
  copy_cstr(message->image_error, sizeof(message->image_error), "Waiting phone");
  message->image_requested = true;
  set_message_image_progress(message, 5);
  copy_cstr(s_image_message_id, sizeof(s_image_message_id), message->image_token);
  free_image_transfer_buffer();
  s_image_size = 0;
  s_image_received = 0;
  s_image_expected_offset = 0;
  s_image_transfer_id = 0;
  if (!send_active_image_request()) {
    schedule_image_retry();
  }
  if (s_messages_root) {
    layer_mark_dirty(s_messages_root);
  }
}

static void image_retry_timer_callback(void *data) {
  s_image_retry_timer = NULL;
#if !MEDIA_ENABLED
  return;
#endif
  if (!send_active_image_request()) {
    request_next_image();
  }
}

static void request_older_messages(bool silent) {
  if (s_at_oldest) {
    if (!silent) {
      show_status("No older messages");
    }
    return;
  }
  if (s_message_count <= 0 || !s_messages[0].id[0]) {
    if (!silent) {
      show_status("No older messages");
    }
    return;
  }
  if (!silent) {
    destroy_message_images();
  }
  if (s_loading_older_messages) {
    if (silent) {
      return;
    }
    cancel_message_timeout();
    cancel_message_retry();
    clear_message_stage();
    reset_message_stream_state();
    s_loading_older_messages = false;
    s_older_anchor_id[0] = '\0';
    s_older_anchor_y = 0;
  }
  recalc_message_layout();
  if (s_selected_message >= 0 && s_selected_message < s_message_count) {
    copy_cstr(s_older_anchor_id, sizeof(s_older_anchor_id), s_messages[s_selected_message].id);
    s_older_anchor_y = s_message_y[s_selected_message];
  } else {
    s_older_anchor_id[0] = '\0';
    s_older_anchor_y = 0;
  }
  s_loading_older_messages = true;
  s_message_request_attempts = 1;
  if (!silent) {
    show_status("Loading older...");
  }
  const char *anchor_id = (s_selected_message >= 0 && s_selected_message < s_message_count) ?
                          s_messages[s_selected_message].id : s_messages[0].id;
  if (!send_command_with_status("get_older_messages", s_current_chat_id, silent ? "silent" : NULL,
                                s_messages[0].id, anchor_id, !silent)) {
    if (silent) {
      s_loading_older_messages = false;
      s_older_anchor_id[0] = '\0';
      s_older_anchor_y = 0;
    } else {
      schedule_message_send_retry();
    }
  } else {
    schedule_message_timeout();
  }
}

static void request_newer_messages(bool silent) {
  if (s_message_count <= 0 || !s_messages[s_message_count - 1].id[0]) {
    if (!silent) {
      scroll_to_bottom(true);
    }
    return;
  }
  if (s_at_newest && !subreddit_detail_active()) {
    if (!silent) {
      scroll_to_bottom(true);
    }
    return;
  }
  if (s_loading_newer_messages) {
    return;
  }
  if (!silent) {
    destroy_message_images();
  }
  recalc_message_layout();
  if (s_selected_message >= 0 && s_selected_message < s_message_count) {
    copy_cstr(s_newer_anchor_id, sizeof(s_newer_anchor_id), s_messages[s_selected_message].id);
    s_newer_anchor_y = s_message_y[s_selected_message];
  } else {
    s_newer_anchor_id[0] = '\0';
    s_newer_anchor_y = 0;
  }
  s_loading_newer_messages = true;
  s_message_request_attempts = 1;
  if (!silent) {
    show_status("Loading newer...");
  }
  const char *anchor_id = (s_selected_message >= 0 && s_selected_message < s_message_count) ?
                          s_messages[s_selected_message].id : s_messages[s_message_count - 1].id;
  bool sent = send_command_with_status("get_newer_messages", s_current_chat_id, silent ? "silent" : NULL,
                                       s_messages[s_message_count - 1].id, anchor_id, !silent);
  if (!sent) {
    if (silent) {
      s_loading_newer_messages = false;
      s_newer_anchor_id[0] = '\0';
      s_newer_anchor_y = 0;
    } else {
      schedule_message_send_retry();
    }
  } else {
    schedule_message_timeout();
  }
}

static void request_messages(const char *chat_id) {
  bump_ui_generation();
  cancel_message_timeout();
  cancel_message_retry();
  close_touch_keyboard();
  s_reddit_detail_request = false;
  destroy_message_images();
  reset_image_transfer_state();
  reset_avatar_transfer_state();
  clear_message_stage();
  s_loading_older_messages = false;
  s_loading_newer_messages = false;
  s_older_anchor_id[0] = '\0';
  s_newer_anchor_id[0] = '\0';
  s_older_anchor_y = 0;
  s_newer_anchor_y = 0;
  s_at_newest = true;
  s_at_oldest = false;
  s_message_scroll_direction = 0;
  s_message_count = 0;
  s_expected_rows = 0;
  s_message_transfer_id = 0;
  reset_message_stream_state();
  s_selected_message = -1;
  s_user_scrolled_messages = false;
  s_chat_view_pending = false;
  s_loading_messages = true;
  s_message_request_attempts = 1;
  show_status("Loading messages...");
  if (s_view_state != ViewStateChat || !s_messages_root) {
    s_chat_view_pending = false;
  } else {
    render_messages();
    if (s_messages_root) {
      layer_mark_dirty(s_messages_root);
    }
  }
  if (send_command_with_status("get_messages", chat_id, NULL, NULL, NULL, false)) {
    schedule_message_timeout();
  } else {
    schedule_message_send_retry();
  }
}

static void maybe_prefetch_older_messages(void) {
  if (!s_loading_older_messages && !s_at_oldest && s_message_count > 0 &&
      s_selected_message >= 0) {
    const char *anchor_id = s_messages[s_selected_message].id;
    send_command_with_status("prefetch_older_messages", s_current_chat_id, "silent",
                             s_messages[0].id, anchor_id, false);
  }
}

static void maybe_prefetch_newer_messages(void) {
  if (!s_loading_newer_messages && !s_at_newest && s_message_count > 0 &&
      s_selected_message >= 0) {
    const char *anchor_id = s_messages[s_selected_message].id;
    send_command_with_status("prefetch_newer_messages", s_current_chat_id, "silent",
                             s_messages[s_message_count - 1].id, anchor_id, false);
  }
}

static void send_text_message(const char *text, bool as_reply) {
  const char *reply_to = NULL;
  if (as_reply && s_selected_message >= 0 && s_selected_message < s_message_count) {
    reply_to = s_messages[s_selected_message].id;
  }
  show_status("Sending...");
  send_command("send_message", s_current_chat_id, text, reply_to, NULL);
}

static void edit_selected_message(const char *text) {
  if (!s_pending_edit_message_id[0]) {
    show_status("No edit target");
    return;
  }
  show_status("Editing...");
  send_command("edit_message", s_current_chat_id, text, NULL, s_pending_edit_message_id);
  s_pending_edit_message_id[0] = '\0';
}

static bool has_selected_message(void) {
  return s_selected_message >= 0 && s_selected_message < s_message_count;
}

static bool compose_target_is_selected(void) {
  return s_list_mode == ListModeChats && s_at_newest && s_selected_message == s_message_count;
}

static bool latest_message_target_is_visible(void) {
  if (s_list_mode != ListModeChats || !s_at_newest || !s_messages_root) {
    return false;
  }
  GRect bounds = layer_get_bounds(s_messages_root);
  int visible_top = s_chat_scroll_offset;
  int visible_bottom = visible_top + bounds.size.h;
  if (!s_touch_keyboard_open) {
    GRect compose = compose_rect_for_bounds(bounds);
    int compose_top = s_chat_scroll_offset + compose.origin.y;
    int compose_bottom = compose_top + compose.size.h;
    if (compose_bottom > visible_top && compose_top < visible_bottom) {
      return true;
    }
  }
  if (s_message_count <= 0) {
    return false;
  }
  int last = s_message_count - 1;
  int message_top = s_message_y[last];
  int message_bottom = message_top + s_message_h[last];
  return message_bottom > visible_top && message_top < visible_bottom;
}

static bool selected_message_is_truncated(void) {
  if (subreddit_detail_active() && has_selected_message() && !selected_message_is_thread_marker()) {
    return true;
  }
  return has_selected_message() && message_display_is_truncated(&s_messages[s_selected_message]);
}

static bool selected_message_is_thread_marker(void) {
  return has_selected_message() && message_is_thread_marker(&s_messages[s_selected_message]);
}

static bool selected_message_has_context(void) {
  return has_selected_message() && message_has_context(&s_messages[s_selected_message]);
}

static bool selected_message_context_is_forward(void) {
  return selected_message_has_context() &&
         strncmp(s_messages[s_selected_message].context, "Fwd from ", 9) == 0;
}

static void send_selected_chat_action(const char *command) {
  if (s_selected_chat < 0 || s_selected_chat >= s_chat_count) {
    return;
  }
  show_status("Updating...");
  send_command(command, s_chats[s_selected_chat].id, NULL, NULL, NULL);
}

static void selected_subreddit_element_name(char *dest, size_t dest_size) {
  if (!dest || dest_size == 0) {
    return;
  }
  copy_cstr(dest, dest_size, "Item");
  if (!subreddit_detail_active() || !has_selected_message()) {
    return;
  }
  Message *message = &s_messages[s_selected_message];
  if (message_is_checklist(message)) {
    if (s_selected_checklist_item < 0) {
      copy_cstr(dest, dest_size, "Checklist");
    } else if (s_selected_checklist_item >= checklist_item_count(message->text)) {
      copy_cstr(dest, dest_size, "Checklist item");
    } else {
      copy_cstr(dest, dest_size, "Checklist item");
    }
    return;
  }
  if (message->sender[0]) {
    copy_cstr(dest, dest_size, message->sender);
  }
}

static void set_pending_subreddit_voice_action(const char *command, const char *message_id) {
  copy_cstr(s_pending_chat_command, sizeof(s_pending_chat_command), command);
  copy_cstr(s_pending_edit_message_id, sizeof(s_pending_edit_message_id), message_id);
  s_pending_subreddit_generation = s_ui_generation;
  if (command && strcmp(command, "create_task") == 0) {
    copy_cstr(s_pending_subreddit_source_id, sizeof(s_pending_subreddit_source_id), s_current_bucket_id);
  } else if (command && strcmp(command, "create_bucket") == 0) {
    copy_cstr(s_pending_subreddit_source_id, sizeof(s_pending_subreddit_source_id), s_current_plan_id);
  } else {
    copy_cstr(s_pending_subreddit_source_id, sizeof(s_pending_subreddit_source_id), s_current_chat_id);
  }
  s_pending_send_as_reply = false;
}

static bool pending_subreddit_action_is_current(void) {
  if (!pending_command_is_subreddit_action() || !s_pending_subreddit_source_id[0]) {
    return true;
  }
  if (s_pending_subreddit_generation != s_ui_generation) {
    return false;
  }
  if (strcmp(s_pending_chat_command, "create_task") == 0) {
    return s_view_state == ViewStateChatList && s_list_mode == ListModeTasks &&
           strcmp(s_current_bucket_id, s_pending_subreddit_source_id) == 0;
  }
  if (strcmp(s_pending_chat_command, "create_bucket") == 0) {
    return s_view_state == ViewStateChatList &&
           (s_list_mode == ListModeSorts || s_list_mode == ListModeTasks) &&
           strcmp(s_current_plan_id, s_pending_subreddit_source_id) == 0;
  }
  return subreddit_detail_active() && s_messages_root &&
         strcmp(s_current_chat_id, s_pending_subreddit_source_id) == 0;
}

static bool date_target_is_current(void) {
  return s_date_generation == s_ui_generation &&
         subreddit_detail_active() && s_messages_root && s_date_target_id[0] &&
         strcmp(s_current_chat_id, s_date_target_id) == 0;
}

static void send_selected_checklist_toggle(void) {
  if (!subreddit_detail_active() || !has_selected_message() ||
      !message_is_checklist(&s_messages[s_selected_message]) || s_selected_checklist_item < 0 ||
      s_selected_checklist_item >= checklist_item_count(s_messages[s_selected_message].text)) {
    return;
  }
  char item_text[12];
  char task_id[MAX_ID];
  char checklist_id[MAX_ID];
  snprintf(item_text, sizeof(item_text), "%d", s_selected_checklist_item);
  copy_cstr(task_id, sizeof(task_id), s_current_chat_id);
  copy_cstr(checklist_id, sizeof(checklist_id), s_messages[s_selected_message].id);
  toggle_checklist_line(s_selected_checklist_item);
  show_status("Checklist updated");
  queue_subreddit_command("toggle_checklist", task_id, item_text, checklist_id);
  if (s_messages_root) {
    layer_mark_dirty(s_messages_root);
  }
}

static void enter_checklist_edit_mode(void) {
  if (!subreddit_detail_active() || !s_messages_root || !has_selected_message() ||
      !message_is_checklist(&s_messages[s_selected_message])) {
    return;
  }
  bump_ui_generation();
  s_checklist_edit_mode = true;
  s_selected_checklist_item = 0;
  recalc_message_layout();
  ensure_selected_checklist_item_visible(false);
  if (s_messages_root) {
    layer_mark_dirty(s_messages_root);
  }
  show_status("Item actions");
}

static void append_text_part(char *dest, size_t dest_size, const char *part) {
  size_t len;
  size_t remaining;
  if (!dest || dest_size == 0 || !part || !part[0]) {
    return;
  }
  len = strlen(dest);
  if (len >= dest_size - 1) {
    return;
  }
  remaining = dest_size - len - 1;
  strncat(dest, part, remaining);
}

static int subreddit_message_index_for(const char *id, const char *kind) {
  for (int i = 0; i < s_message_count; i++) {
    Message *message = &s_messages[i];
    if (id && id[0] && strcmp(message->id, id) == 0) {
      return i;
    }
    if (kind && kind[0] && message_kind_matches(message, kind)) {
      return i;
    }
  }
  return -1;
}

static Message *insert_subreddit_field_message(const char *id, const char *sender, const char *kind) {
  if (!subreddit_detail_active() || s_message_count >= MAX_MESSAGES) {
    return NULL;
  }
  int insert = s_message_count;
  for (int i = 0; i < s_message_count; i++) {
    if (strcmp(s_messages[i].id, "__add_item_bottom") == 0) {
      insert = i;
      break;
    }
  }
  for (int i = s_message_count; i > insert; i--) {
    s_messages[i] = s_messages[i - 1];
  }
  memset(&s_messages[insert], 0, sizeof(Message));
  init_message_strings(&s_messages[insert]);
  s_message_count++;
  if (s_selected_message >= insert) {
    s_selected_message++;
  }
  copy_cstr(s_messages[insert].id, sizeof(s_messages[insert].id), id);
  copy_cstr(s_messages[insert].section, sizeof(s_messages[insert].section), kind);
  set_message_sender_text(&s_messages[insert], sender, "");
  s_messages[insert].outgoing = false;
  return &s_messages[insert];
}

static Message *subreddit_field_message(const char *id, const char *sender, const char *kind) {
  int index = subreddit_message_index_for(id, kind);
  if (index >= 0) {
    return &s_messages[index];
  }
  return insert_subreddit_field_message(id, sender, kind);
}

static void redraw_subreddit_detail_locally(void) {
  recalc_message_layout();
  clamp_subreddit_selection();
  if (s_messages_root) {
    layer_mark_dirty(s_messages_root);
  }
}

static bool set_or_append_subreddit_field_locally(const char *id, const char *sender,
                                                const char *kind, const char *text,
                                                bool append) {
  if (!subreddit_detail_active() || !text || !text[0]) {
    return false;
  }
  Message *message = subreddit_field_message(id, sender, kind);
  if (!message) {
    return false;
  }
  copy_cstr(message->id, sizeof(message->id), id);
  copy_cstr(message->section, sizeof(message->section), kind);
  if (append && !message_text_is_empty_field(message)) {
    char next[MAX_TEXT];
    copy_cstr(next, sizeof(next), message->text);
    append_text_part(next, sizeof(next), "\n");
    append_text_part(next, sizeof(next), text);
    set_message_sender_text(message, sender, next);
  } else {
    set_message_sender_text(message, sender, text);
  }
  int index = subreddit_message_index_for(id, kind);
  if (index >= 0 && strcmp(kind, "notes") == 0) {
    s_selected_message = index;
    s_selected_checklist_item = -1;
  }
  redraw_subreddit_detail_locally();
  return true;
}

static bool append_assignee_locally(const char *name) {
  const char *label = name && name[0] ? name : "Me";
  Message *message = assigned_message();
  if (message) {
    int count = checklist_item_count(message->text);
    for (int i = 0; i < count; i++) {
      char line[MAX_TEXT];
      checklist_line_at(message->text, i, line, sizeof(line));
      if (strcmp(line, label) == 0) {
        return true;
      }
    }
  }
  return set_or_append_subreddit_field_locally("assigned", "Assigned", "assigned",
                                             label, true);
}

static bool remove_assignee_locally(int item_index) {
  Message *message = assigned_message();
  if (!message || item_index < 0 || item_index >= checklist_item_count(message->text)) {
    return false;
  }

  char next[MAX_TEXT];
  next[0] = '\0';
  int count = checklist_item_count(message->text);
  for (int i = 0; i < count; i++) {
    if (i == item_index) {
      continue;
    }
    char line[MAX_TEXT];
    checklist_line_at(message->text, i, line, sizeof(line));
    if (!line[0]) {
      continue;
    }
    if (next[0]) {
      append_text_part(next, sizeof(next), "\n");
    }
    append_text_part(next, sizeof(next), line);
  }

  if (next[0]) {
    set_message_text(message, next);
  } else {
    int index = assigned_message_index();
    if (index >= 0) {
      remove_message_at(index);
      return true;
    }
    set_message_text(message, "");
  }
  redraw_subreddit_detail_locally();
  return true;
}

static bool append_checklist_item_locally(const char *text) {
  if (!subreddit_detail_active() || !text || !text[0]) {
    return false;
  }
  Message *message = subreddit_field_message("checklist", "Checklist", "checklist");
  if (!message) {
    return false;
  }
  int index = subreddit_message_index_for("checklist", "checklist");
  int new_item = checklist_item_count(message->text);
  char line[MAX_TEXT];
  copy_cstr(line, sizeof(line), "[ ] ");
  append_text_part(line, sizeof(line), text);
  if (message_text_is_empty_field(message) || checklist_item_count(message->text) == 0) {
    set_message_sender_text(message, "Checklist", line);
  } else {
    char next[MAX_TEXT];
    copy_cstr(next, sizeof(next), message->text);
    append_text_part(next, sizeof(next), "\n");
    append_text_part(next, sizeof(next), line);
    set_message_text(message, next);
  }
  if (index >= 0) {
    s_selected_message = index;
    s_selected_checklist_item = new_item;
  }
  recalc_message_layout();
  clamp_subreddit_selection();
  ensure_selected_checklist_item_visible(false);
  return true;
}

static bool rewrite_checklist_item_locally(int target, const char *replacement, bool delete_item) {
  int index = has_selected_message() && message_is_checklist(&s_messages[s_selected_message]) ?
              s_selected_message : subreddit_message_index_for("checklist", "checklist");
  if (!subreddit_detail_active() || index < 0) {
    return false;
  }
  Message *message = &s_messages[index];
  int items = checklist_item_count(message->text);
  if (target < 0 || target >= items) {
    return false;
  }
  char next[MAX_TEXT];
  next[0] = '\0';
  for (int item = 0; item < items; item++) {
    char line[96];
    checklist_line_at(message->text, item, line, sizeof(line));
    if (item == target && delete_item) {
      continue;
    }
    if (item == target && replacement && replacement[0]) {
      bool checked = strncmp(line, "[x]", 3) == 0 || strncmp(line, "[X]", 3) == 0;
      snprintf(line, sizeof(line), "%s %s", checked ? "[x]" : "[ ]", replacement);
    }
    if (next[0]) {
      append_text_part(next, sizeof(next), "\n");
    }
    append_text_part(next, sizeof(next), line);
  }
  set_message_text(message, next[0] ? next : "No checklist items");
  s_selected_message = index;
  if (delete_item) {
    int remaining = checklist_item_count(message->text);
    s_selected_checklist_item = remaining > 0 ? PG_MIN(target, remaining - 1) : -1;
  } else {
    s_selected_checklist_item = target;
  }
  recalc_message_layout();
  clamp_subreddit_selection();
  ensure_selected_checklist_item_visible(false);
  return true;
}

static bool set_subreddit_message_text_locally(const char *message_id, const char *text) {
  int index;
  if (!subreddit_detail_active() || !text || !text[0]) {
    return false;
  }
  index = subreddit_message_index_for(message_id, NULL);
  if (index < 0 && has_selected_message() && s_messages[s_selected_message].id[0] != '_') {
    index = s_selected_message;
  }
  if (index < 0) {
    return false;
  }
  set_message_text(&s_messages[index], text);
  s_selected_message = index;
  s_selected_checklist_item = -1;
  redraw_subreddit_detail_locally();
  return true;
}

static void format_date_offset(int days, char *dest, size_t dest_size) {
  time_t selected = time(NULL) + ((time_t)days * 24 * 60 * 60);
  struct tm *local = localtime(&selected);
  if (!dest || dest_size == 0) {
    return;
  }
  if (!local) {
    copy_cstr(dest, dest_size, "");
    return;
  }
  strftime(dest, dest_size, "%Y-%m-%d", local);
}

static void open_date_window(const char *command, const char *label) {
  DBG("PT date open g=%lu h=%u", (unsigned long)s_ui_generation, (unsigned)heap_bytes_free());
  copy_cstr(s_date_command, sizeof(s_date_command), command);
  copy_cstr(s_date_label, sizeof(s_date_label), label);
  copy_cstr(s_date_target_id, sizeof(s_date_target_id), s_current_chat_id);
  s_date_generation = s_ui_generation;
  show_action_window(ActionMenuDate);
}

static void date_send_timer_callback(void *data) {
  s_date_send_timer = NULL;
  CDBG("df %d", date_target_is_current() ? 1 : 0);
  if (s_date_command[0] && s_date_target_id[0] && s_date_label[0] &&
      date_target_is_current()) {
    show_status("Setting date...");
    capture_subreddit_refresh_anchor();
    if (strcmp(s_date_command, "set_task_due") == 0) {
      set_or_append_subreddit_field_locally("due", "Due", "due", s_date_label, false);
    } else if (strcmp(s_date_command, "set_task_start") == 0) {
      set_or_append_subreddit_field_locally("start", "Start", "start", s_date_label, false);
    }
    queue_subreddit_command(s_date_command, s_date_target_id, s_date_label, NULL);
  } else {
    show_status("Date canceled");
  }
  s_date_command[0] = '\0';
  s_date_label[0] = '\0';
  s_date_target_id[0] = '\0';
  s_date_generation = 0;
  DBG("PT date done h=%u", (unsigned)heap_bytes_free());
}

static void schedule_date_send(const char *command, const char *target_id, const char *value) {
  DBG("PT date sched g=%lu h=%u", (unsigned long)s_ui_generation, (unsigned)heap_bytes_free());
  if (s_date_send_timer) {
    app_timer_cancel(s_date_send_timer);
    s_date_send_timer = NULL;
  }
  copy_cstr(s_date_command, sizeof(s_date_command), command);
  copy_cstr(s_date_target_id, sizeof(s_date_target_id), target_id);
  copy_cstr(s_date_label, sizeof(s_date_label), value);
  s_date_generation = s_ui_generation;
  s_date_send_timer = app_timer_register(300, date_send_timer_callback, NULL);
  DBG("PT date timer p=%p dg=%lu", s_date_send_timer, (unsigned long)s_date_generation);
}

static bool pending_command_is_subreddit_action(void) {
  return strcmp(s_pending_chat_command, "create_task") == 0 ||
         strcmp(s_pending_chat_command, "create_bucket") == 0 ||
         strcmp(s_pending_chat_command, "add_task_note") == 0 ||
         strcmp(s_pending_chat_command, "add_checklist_item") == 0 ||
         strcmp(s_pending_chat_command, "set_task_due") == 0 ||
         strcmp(s_pending_chat_command, "edit_task_element") == 0 ||
         strcmp(s_pending_chat_command, "edit_checklist") == 0 ||
         strcmp(s_pending_chat_command, "edit_checklist_item") == 0 ||
         strcmp(s_pending_chat_command, "assign_task") == 0 ||
         strcmp(s_pending_chat_command, "unassign_task") == 0 ||
         strcmp(s_pending_chat_command, "set_task_start") == 0 ||
         strcmp(s_pending_chat_command, "set_task_priority") == 0 ||
         strcmp(s_pending_chat_command, "set_task_progress") == 0 ||
         strcmp(s_pending_chat_command, "move_task_bucket") == 0 ||
         strcmp(s_pending_chat_command, "add_task_label") == 0;
}

static void send_pending_subreddit_action(void) {
  char command[sizeof(s_pending_chat_command)];
  char target_id[MAX_ID];
  char text[MAX_TEXT];
  char message_id[MAX_ID];
  copy_cstr(command, sizeof(command), s_pending_chat_command);
  copy_cstr(target_id, sizeof(target_id), s_current_chat_id);
  copy_cstr(text, sizeof(text), s_pending_text);
  copy_cstr(message_id, sizeof(message_id), s_pending_edit_message_id);
  if (strcmp(s_pending_chat_command, "create_task") == 0) {
    copy_cstr(target_id, sizeof(target_id), s_current_bucket_id);
  } else if (strcmp(s_pending_chat_command, "create_bucket") == 0) {
    copy_cstr(target_id, sizeof(target_id), s_current_plan_id);
  }
  if (!pending_subreddit_action_is_current()) {
    show_status("Action canceled");
    clear_pending_text_action();
    return;
  }
  if (!target_id[0] || !text[0]) {
    show_status("Nothing to add");
    clear_pending_text_action();
    return;
  }
  capture_subreddit_refresh_anchor();
  show_status(strcmp(command, "create_task") == 0 ? "Creating post..." :
              (strcmp(command, "create_bucket") == 0 ? "Creating bucket..." : "Updating post..."));
  if (strcmp(command, "add_task_note") == 0) {
    set_or_append_subreddit_field_locally("notes", "Notes", "notes", text, true);
  } else if (strcmp(command, "add_checklist_item") == 0) {
    append_checklist_item_locally(text);
  } else if (strcmp(command, "edit_task_element") == 0) {
    set_subreddit_message_text_locally(message_id, text);
  } else if (strcmp(command, "edit_checklist_item") == 0) {
    rewrite_checklist_item_locally(atoi(message_id), text, false);
  }
  queue_subreddit_command(command, target_id, text, message_id[0] ? message_id : NULL);
}

static void commit_pending_text_action(void) {
  if (s_pending_chat_command[0]) {
    if (pending_command_is_subreddit_action()) {
      send_pending_subreddit_action();
    } else {
      send_selected_chat_action(s_pending_chat_command);
    }
    s_pending_chat_command[0] = '\0';
    s_pending_edit_message_id[0] = '\0';
  } else if (s_pending_edit_message_id[0]) {
    edit_selected_message(s_pending_text);
  } else if (s_pending_text[0]) {
    send_text_message(s_pending_text, s_pending_send_as_reply);
  } else {
    show_status("No text");
  }
  s_pending_send_as_reply = false;
}

static bool subreddit_intent_generation_current(const SubredditsIntent *intent) {
  return intent && intent->active && intent->generation == s_ui_generation;
}

static bool subreddit_intent_detail_current(const SubredditsIntent *intent) {
  return subreddit_intent_generation_current(intent) &&
         subreddit_detail_active() && intent->chat_id[0] &&
         strcmp(s_current_chat_id, intent->chat_id) == 0;
}

static bool select_subreddit_intent_message(const SubredditsIntent *intent, bool require_checklist) {
  if (!subreddit_intent_detail_current(intent) || !intent->message_id[0]) {
    return false;
  }
  int message_index = find_message_index_by_id(intent->message_id);
  if (message_index < 0) {
    return false;
  }
  if (require_checklist && !message_is_checklist(&s_messages[message_index])) {
    return false;
  }
  s_selected_message = message_index;
  s_selected_checklist_item = intent->checklist_item;
  clamp_subreddit_selection();
  return has_selected_message();
}

static void cancel_stale_subreddit_intent(void) {
  show_status("Action canceled");
  clear_pending_text_action();
}

static void run_subreddit_intent(const SubredditsIntent *intent) {
  static const char *priorities[] = {"Urgent", "Important", "Medium", "Low"};
  static const char *progress[] = {"Not Started", "In Progress", "Complete"};

  if (!subreddit_intent_generation_current(intent)) {
    cancel_stale_subreddit_intent();
    return;
  }

  switch (intent->item) {
    case ActionItemSubredditsComplete:
    case ActionItemSubredditsMarkActive:
    case ActionItemSubredditsDownvote:
    case ActionItemDeletePost: {
      const char *command = "save";
      const char *status = "Saving...";
      if (!intent->chat_id[0]) {
        cancel_stale_subreddit_intent();
        return;
      }
      if (intent->item == ActionItemSubredditsComplete) {
        command = intent->message_id[0] ? "upvote" : "complete_task";
        status = "Upvoting...";
      } else if (intent->item == ActionItemSubredditsDownvote) {
        command = "downvote";
        status = "Downvoting...";
      } else if (intent->item == ActionItemSubredditsMarkActive) {
        command = "uncomplete_task";
        status = "Clearing vote...";
      } else if (intent->item == ActionItemDeletePost &&
                 subreddit_target_saved(intent->chat_id, intent->message_id)) {
        command = "unsave";
        status = "Unsaving...";
      }
      show_status(status);
      queue_subreddit_command(command, intent->chat_id, NULL,
                              intent->message_id[0] ? intent->message_id : NULL);
      break;
    }
    case ActionItemOpenPost:
      if (s_view_state == ViewStateChatList && intent->chat_id[0]) {
        request_task_detail(intent->chat_id, intent->title);
      } else {
        cancel_stale_subreddit_intent();
      }
      break;
    case ActionItemChecklistEditList:
      if (select_subreddit_intent_message(intent, true)) {
        enter_checklist_edit_mode();
      } else {
        cancel_stale_subreddit_intent();
      }
      break;
    case ActionItemChecklistDeleteList:
      if (select_subreddit_intent_message(intent, true)) {
        show_status("Deleting list...");
        capture_subreddit_refresh_anchor();
        queue_subreddit_command("delete_checklist", s_current_chat_id, NULL,
                              s_messages[s_selected_message].id);
      } else {
        cancel_stale_subreddit_intent();
      }
      break;
    case ActionItemChecklistAddItem:
      if (subreddit_intent_detail_current(intent)) {
        set_pending_subreddit_voice_action("add_checklist_item", NULL);
        schedule_dictation_start();
      } else {
        cancel_stale_subreddit_intent();
      }
      break;
    case ActionItemChecklistDeleteItem:
      if (select_subreddit_intent_message(intent, true) && s_selected_checklist_item >= 0 &&
          s_selected_checklist_item < checklist_item_count(s_messages[s_selected_message].text)) {
        char item_id[12];
        snprintf(item_id, sizeof(item_id), "%d", s_selected_checklist_item);
        show_status("Deleting item...");
        capture_subreddit_refresh_anchor();
        rewrite_checklist_item_locally(s_selected_checklist_item, NULL, true);
        queue_subreddit_command("delete_checklist_item", s_current_chat_id, NULL, item_id);
      } else {
        cancel_stale_subreddit_intent();
      }
      break;
    case ActionItemChecklistToggleItem:
      if (select_subreddit_intent_message(intent, true)) {
        capture_subreddit_refresh_anchor();
        send_selected_checklist_toggle();
      } else {
        cancel_stale_subreddit_intent();
      }
      break;
    case ActionItemSubredditsElementDelete:
    case ActionItemNotesDelete:
      if (select_subreddit_intent_message(intent, false)) {
        char message_id[MAX_ID];
        copy_cstr(message_id, sizeof(message_id), s_messages[s_selected_message].id);
        show_status(intent->item == ActionItemNotesDelete ? "Deleting notes..." : "Deleting item...");
        capture_subreddit_refresh_anchor();
        queue_subreddit_command("delete_task_element", s_current_chat_id, NULL, message_id);
      } else {
        cancel_stale_subreddit_intent();
      }
      break;
    case ActionItemSubredditsAddAssign:
      if (subreddit_intent_detail_current(intent)) {
        show_action_window(ActionMenuAssign);
      } else {
        cancel_stale_subreddit_intent();
      }
      break;
    case ActionItemPriorityChoice:
      if (subreddit_intent_detail_current(intent) && intent->index >= 0 && intent->index < 4) {
        show_status("Setting priority...");
        capture_subreddit_refresh_anchor();
        set_or_append_subreddit_field_locally("priority", "Priority", "priority",
                                            priorities[intent->index], false);
        queue_subreddit_command("set_task_priority", s_current_chat_id, priorities[intent->index], NULL);
      } else {
        cancel_stale_subreddit_intent();
      }
      break;
    case ActionItemProgressChoice:
      if (subreddit_intent_detail_current(intent) && intent->index >= 0 && intent->index < 3) {
        show_status("Setting progress...");
        capture_subreddit_refresh_anchor();
        set_or_append_subreddit_field_locally("status", "Status", "status",
                                            progress[intent->index], false);
        queue_subreddit_command("set_task_progress", s_current_chat_id, progress[intent->index], NULL);
      } else {
        cancel_stale_subreddit_intent();
      }
      break;
    case ActionItemLabelChoice:
      if (subreddit_intent_detail_current(intent) && intent->index >= 0 && intent->index < PLANNER_LABEL_COUNT) {
        char label_id[12];
        char label_text[PLANNER_LABEL_TEXT_LEN];
        snprintf(label_id, sizeof(label_id), "category%d", intent->index + 1);
        subreddit_label_text(intent->index, label_text, sizeof(label_text));
        show_status("Adding label...");
        capture_subreddit_refresh_anchor();
        set_or_append_subreddit_field_locally("labels", "Labels", "labels",
                                            label_text, true);
        queue_subreddit_command("add_task_label", s_current_chat_id, label_text, label_id);
      } else {
        cancel_stale_subreddit_intent();
      }
      break;
    case ActionItemBucketChoice:
      if (subreddit_intent_detail_current(intent)) {
        if (intent->index >= 0 && intent->index < s_bucket_option_count) {
          show_status("Moving post...");
          capture_subreddit_refresh_anchor();
          set_or_append_subreddit_field_locally("bucket", "Bucket", "bucket",
                                              s_bucket_options[intent->index].title, false);
          queue_subreddit_command("move_task_bucket", s_current_chat_id,
                                s_bucket_options[intent->index].title,
                                s_bucket_options[intent->index].id);
        } else if (s_current_bucket_id[0]) {
          show_status("Moving post...");
          capture_subreddit_refresh_anchor();
          set_or_append_subreddit_field_locally("bucket", "Bucket", "bucket",
                                              s_current_bucket_title, false);
          queue_subreddit_command("move_task_bucket", s_current_chat_id,
                                s_current_bucket_title, s_current_bucket_id);
        } else {
          cancel_stale_subreddit_intent();
        }
      } else {
        cancel_stale_subreddit_intent();
      }
      break;
    default:
      cancel_stale_subreddit_intent();
      break;
  }
}

static void subreddit_intent_timer_callback(void *data) {
  s_subreddit_intent_timer = NULL;
  SubredditsIntent intent = s_subreddit_intent;
  memset(&s_subreddit_intent, 0, sizeof(s_subreddit_intent));
  if (!intent.active) {
    return;
  }
  run_subreddit_intent(&intent);
}

static void inbox_received_callback(DictionaryIterator *iter, void *context) {
  char *type = tuple_cstring(iter, MESSAGE_KEY_Type);
  if (!type) {
    return;
  }

#if !MEDIA_ENABLED
  if (strcmp(type, "avatar_start") == 0 ||
      strcmp(type, "avatar") == 0 ||
      strcmp(type, "avatar_done") == 0 ||
      strcmp(type, "image_start") == 0 ||
      strcmp(type, "image_status") == 0 ||
      strcmp(type, "image") == 0 ||
      strcmp(type, "image_done") == 0 ||
      strcmp(type, "image_error") == 0) {
    reset_avatar_transfer_state();
    reset_image_transfer_state();
    return;
  }
#endif

  if (strcmp(type, "status") == 0) {
    char *status = tuple_cstring(iter, MESSAGE_KEY_Status);
    if (status) {
      if (status_is_microsoft_login(status)) {
        show_microsoft_login_status(status);
      }
      if (s_chats_loading && s_view_state != ViewStateChat) {
        s_chats_loading = true;
        show_loading_text(status, false);
      }
      if (strcmp(status, "Loading messages...") == 0) {
        cancel_message_retry();
      }
      show_status(status);
    }
    return;
  }

  if (strcmp(type, "settings") == 0) {
    update_subreddit_labels(tuple_cstring(iter, MESSAGE_KEY_PlannerLabels));
    return;
  }

  if (strcmp(type, "error") == 0) {
    char *error = tuple_cstring(iter, MESSAGE_KEY_Error);
    cancel_message_timeout();
    cancel_message_retry();
    unlock_interactions_now();
    if (s_chat_retry_timer) {
      app_timer_cancel(s_chat_retry_timer);
      s_chat_retry_timer = NULL;
    }
    if (s_view_state != ViewStateChat) {
      s_bridge_ready = false;
      s_chats_loading = false;
      show_loading_text(error ? error : "Login failed", true);
      show_status(default_status_text());
      if (s_chat_menu) {
        menu_layer_reload_data(s_chat_menu);
      }
    } else {
      s_loading_messages = false;
      s_loading_older_messages = false;
      s_loading_newer_messages = false;
      s_message_transfer_id = 0;
      show_status(error ? error : "Error");
      if (s_messages_root) {
        layer_mark_dirty(s_messages_root);
      }
    }
    return;
  }

  int index = tuple_int(iter, MESSAGE_KEY_Index, 0);
  int count = tuple_int(iter, MESSAGE_KEY_Count, 0);
  s_expected_rows = count;

  if (strcmp(type, "chats_done") == 0) {
    cancel_message_timeout();
    if (s_chat_retry_timer) {
      app_timer_cancel(s_chat_retry_timer);
      s_chat_retry_timer = NULL;
    }
    char selected_id[MAX_ID];
    copy_cstr(selected_id, sizeof(selected_id), s_chat_list_selected_id);
    if (!selected_id[0]) {
      copy_cstr(selected_id, sizeof(selected_id), s_chat_refresh_selected_id);
    }
    s_chat_refresh_selected_id[0] = '\0';
    if (!selected_id[0] && s_selected_chat >= 0 && s_selected_chat < s_chat_count) {
      copy_cstr(selected_id, sizeof(selected_id), s_chats[s_selected_chat].id);
    }
    s_bridge_ready = true;
    s_chats_loading = false;
    s_loading_error = false;
    unlock_interactions_now();
    if (s_chat_count > count) {
      s_chat_count = count;
    }
    int preserved_index = find_chat_index_by_id(selected_id);
    if (preserved_index >= 0) {
      s_selected_chat = preserved_index;
      copy_cstr(s_chat_list_selected_id, sizeof(s_chat_list_selected_id), s_chats[s_selected_chat].id);
    } else if (s_selected_chat >= s_chat_count) {
      s_selected_chat = s_chat_count > 0 ? s_chat_count - 1 : 0;
    }
    if (s_chat_menu) {
      menu_layer_reload_data(s_chat_menu);
      select_chat_row(s_selected_chat, false);
    }
    show_status(default_status_text());
    return;
  }

  if (strcmp(type, "assign_options_done") == 0) {
    char *section = tuple_cstring(iter, MESSAGE_KEY_Text);
    bool teams = section && strcmp(section, "team") == 0;
    unlock_interactions_now();
    if (count <= 0) {
      clear_assign_picker_options(teams);
      show_status(teams ? "No teams found" : "No members found");
    } else {
      show_status(teams ? "Teams loaded" : "Members loaded");
    }
    if (s_action_window && (s_action_mode == ActionMenuAssign ||
                            s_action_mode == ActionMenuAssignTeams ||
                            s_action_mode == ActionMenuAssignMembers) && s_action_layer) {
      s_action_selected = 0;
      layer_mark_dirty(s_action_layer);
    }
    return;
  }

  if (strcmp(type, "assign_option") == 0 && index >= 0 && index < MAX_ASSIGNEE_OPTIONS) {
    char *section = tuple_cstring(iter, MESSAGE_KEY_Section);
    bool teams = section && strcmp(section, "team") == 0;
    remember_assign_picker_option(teams, index,
                                  tuple_cstring(iter, MESSAGE_KEY_ChatId),
                                  tuple_cstring(iter, MESSAGE_KEY_Sender));
    if (s_action_window && (s_action_mode == ActionMenuAssign ||
                            s_action_mode == ActionMenuAssignTeams ||
                            s_action_mode == ActionMenuAssignMembers) && s_action_layer) {
      layer_mark_dirty(s_action_layer);
    }
    return;
  }

  if (strcmp(type, "messages_start") == 0) {
    int transfer_id = tuple_int(iter, MESSAGE_KEY_ImageTransferId, 0);
    int mode = tuple_int(iter, MESSAGE_KEY_Index, MESSAGE_MODE_INITIAL);
    bool initial = mode == MESSAGE_MODE_INITIAL;
    bool requested_page = s_loading_older_messages || s_loading_newer_messages;
    if (initial && s_view_state != ViewStateChat && !s_loading_messages) {
      return;
    }
    s_message_transfer_id = transfer_id;
    s_expected_rows = count;
    s_message_stream_mode = mode;
    char *stream_flag = tuple_cstring(iter, MESSAGE_KEY_Text);
    s_message_stream_silent = stream_flag && strcmp(stream_flag, "silent") == 0;
    IMAGE_DIAG("PGIMG watch messages_start mode=%d count=%d transfer=%d initial=%d silent=%d",
               mode, count, transfer_id, initial ? 1 : 0, s_message_stream_silent ? 1 : 0);
    if (initial) {
      if (subreddit_detail_active()) {
        bump_ui_generation();
      }
      clear_message_stage();
      clear_message_rows();
      s_loading_older_messages = false;
      s_loading_newer_messages = false;
      s_older_anchor_id[0] = '\0';
      s_newer_anchor_id[0] = '\0';
      s_at_newest = !subreddit_detail_active();
      s_at_oldest = false;
    } else if (requested_page && !prepare_message_stage()) {
      reset_message_stream_state();
      s_loading_older_messages = false;
      s_loading_newer_messages = false;
      show_status("Memory low");
      return;
    }
    if (initial && !s_message_stream_silent && s_messages_root) {
      layer_mark_dirty(s_messages_root);
    }
    return;
  }

  if (strcmp(type, "message_prepend") == 0) {
    char anchor_id[MAX_ID];
    int anchor_y = 0;
    Message *slot;
    if (!message_transfer_matches(iter)) {
      return;
    }
    if (s_view_state != ViewStateChat || !s_messages_root) {
      return;
    }
    anchor_id[0] = '\0';
    recalc_message_layout();
    if (!s_message_stream_silent && s_loading_older_messages && s_older_anchor_id[0]) {
      copy_cstr(anchor_id, sizeof(anchor_id), s_older_anchor_id);
      int anchor_index = find_message_index_by_id(s_older_anchor_id);
      if (anchor_index >= 0) {
        anchor_y = s_message_y[anchor_index];
      }
    } else if (has_selected_message()) {
      copy_cstr(anchor_id, sizeof(anchor_id), s_messages[s_selected_message].id);
      anchor_y = s_message_y[s_selected_message];
    }
    slot = prepend_message_slot();
    populate_message_from_tuple(slot, iter);
    s_expected_rows = count;
    render_after_stream_prepend(anchor_id, anchor_y);
    return;
  }

  if (strcmp(type, "message_append") == 0) {
    char anchor_id[MAX_ID];
    int anchor_y = 0;
    Message *slot;
    bool follow_bottom;
    if (!message_transfer_matches(iter)) {
      return;
    }
    if (s_view_state != ViewStateChat || !s_messages_root) {
      return;
    }
    anchor_id[0] = '\0';
    recalc_message_layout();
    follow_bottom = !subreddit_detail_active() && latest_message_target_is_visible();
    if (!s_message_stream_silent && s_loading_newer_messages && s_newer_anchor_id[0]) {
      copy_cstr(anchor_id, sizeof(anchor_id), s_newer_anchor_id);
      int anchor_index = find_message_index_by_id(s_newer_anchor_id);
      if (anchor_index >= 0) {
        anchor_y = s_message_y[anchor_index];
      }
    } else if (has_selected_message()) {
      copy_cstr(anchor_id, sizeof(anchor_id), s_messages[s_selected_message].id);
      anchor_y = s_message_y[s_selected_message];
    }
    bool replaces_pending = false;
#if TOUCH_KEYBOARD_AVAILABLE
    char *incoming_text = tuple_cstring(iter, MESSAGE_KEY_Text);
    replaces_pending = s_touch_keyboard_sent_text[0] &&
                       s_message_count > 0 &&
                       strcmp(s_messages[s_message_count - 1].id, "pending") == 0 &&
                       tuple_int(iter, MESSAGE_KEY_IsOutgoing, 0) != 0 &&
                       incoming_text && strcmp(incoming_text, s_touch_keyboard_sent_text) == 0;
#endif
    slot = replaces_pending ? &s_messages[s_message_count - 1] : append_message_slot();
    populate_message_from_tuple(slot, iter);
#if TOUCH_KEYBOARD_AVAILABLE
    if (replaces_pending) {
      s_touch_keyboard_sent_text[0] = '\0';
    }
#endif
    s_expected_rows = count;
    if (!s_message_stream_silent && follow_bottom) {
      scroll_to_bottom(false);
      return;
    }
    render_after_stream_append(anchor_id, anchor_y);
    return;
  }

  if (strcmp(type, "message_update") == 0) {
    char selected_id[MAX_ID];
    char *incoming_id = tuple_cstring(iter, MESSAGE_KEY_MessageId);
    int update_index;
    int selected_y = 0;
    int updated_selected_y = 0;
    if (!incoming_id || s_view_state != ViewStateChat || !s_messages_root) {
      return;
    }
    update_index = find_message_index_by_id(incoming_id);
    if (update_index < 0) {
      return;
    }
    selected_id[0] = '\0';
    if (has_selected_message()) {
      recalc_message_layout();
      copy_cstr(selected_id, sizeof(selected_id), s_messages[s_selected_message].id);
      selected_y = s_message_y[s_selected_message];
    }
    populate_message_from_tuple(&s_messages[update_index], iter);
    if (selected_id[0]) {
      int selected_index = find_message_index_by_id(selected_id);
      if (selected_index >= 0) {
        s_selected_message = selected_index;
        recalc_message_layout();
        updated_selected_y = s_message_y[selected_index];
        set_chat_scroll_offset(s_chat_scroll_offset + (updated_selected_y - selected_y), false);
      } else {
        recalc_message_layout();
      }
    } else {
      recalc_message_layout();
    }
    layer_mark_dirty(s_messages_root);
    request_next_image();
    return;
  }

  if (strcmp(type, "messages_done") == 0) {
    if (!message_transfer_matches(iter)) {
      return;
    }
    cancel_message_timeout();
    cancel_message_retry();
    char selected_id[MAX_ID];
    bool loading_initial = s_loading_messages;
    bool loading_older = s_loading_older_messages;
    bool loading_newer = s_loading_newer_messages;
    char *done_flag = tuple_cstring(iter, MESSAGE_KEY_Text);
    bool loading_silent = s_message_stream_silent || (done_flag && strcmp(done_flag, "silent") == 0);
    bool show_pending = s_chat_view_pending;
    bool chat_visible = s_view_state == ViewStateChat && s_messages_root;
    s_loading_messages = false;
    s_message_transfer_id = 0;
    if (loading_initial) {
      unlock_interactions_now();
    }
    selected_id[0] = '\0';

    if (loading_older && count == 0) {
      s_loading_older_messages = false;
      s_older_anchor_id[0] = '\0';
      s_older_anchor_y = 0;
      s_at_oldest = true;
      clear_message_stage();
      reset_message_stream_state();
      if (!loading_silent) {
        show_status("No older messages");
        if (s_messages_root) {
          layer_mark_dirty(s_messages_root);
        }
      }
      return;
    }

    if (loading_newer && count == 0) {
      s_loading_newer_messages = false;
      s_newer_anchor_id[0] = '\0';
      s_newer_anchor_y = 0;
      s_at_newest = true;
      clear_message_stage();
      reset_message_stream_state();
      if (!loading_silent) {
        show_status(s_current_chat_title);
        if (s_messages_root) {
          layer_mark_dirty(s_messages_root);
        }
      }
      return;
    }

    char fallback_id[MAX_ID];
    bool staged_load = loading_older || loading_newer;
    bool live_anchor = false;
    int live_anchor_y = 0;
    int live_anchor_scroll_offset = s_chat_scroll_offset;
    fallback_id[0] = '\0';

    if (chat_visible && staged_load && has_selected_message()) {
      recalc_message_layout();
      copy_cstr(selected_id, sizeof(selected_id), s_messages[s_selected_message].id);
      live_anchor_y = s_message_y[s_selected_message];
      live_anchor_scroll_offset = s_chat_scroll_offset;
      live_anchor = true;
    }

    if (loading_older) {
      s_at_newest = false;
      s_at_oldest = false;
      copy_cstr(fallback_id, sizeof(fallback_id), s_older_anchor_id);
    } else if (loading_newer) {
      s_at_oldest = false;
      copy_cstr(fallback_id, sizeof(fallback_id), s_newer_anchor_id);
    } else if (s_user_scrolled_messages && s_selected_message >= 0 && s_selected_message < s_message_count) {
      copy_cstr(selected_id, sizeof(selected_id), s_messages[s_selected_message].id);
    }

    bool reversed_load = staged_load &&
                         ((loading_older && s_message_scroll_direction > 0) ||
                          (loading_newer && s_message_scroll_direction < 0));
    if (reversed_load) {
      int current_direction = s_message_scroll_direction;
      s_loading_older_messages = false;
      s_loading_newer_messages = false;
      s_older_anchor_id[0] = '\0';
      s_newer_anchor_id[0] = '\0';
      clear_message_stage();
      reset_message_stream_state();
      if (chat_visible) {
        show_status(s_current_chat_title);
        layer_mark_dirty(s_messages_root);
      }
      if (current_direction > 0 && !s_at_newest) {
        request_newer_messages(true);
      } else if (current_direction < 0 && !s_at_oldest) {
        request_older_messages(true);
      }
      return;
    }

    if (staged_load && live_anchor && !message_stage_contains_id(selected_id)) {
      s_loading_older_messages = false;
      s_loading_newer_messages = false;
      s_older_anchor_id[0] = '\0';
      s_newer_anchor_id[0] = '\0';
      clear_message_stage();
      reset_message_stream_state();
      if (chat_visible) {
        show_status(s_current_chat_title);
        layer_mark_dirty(s_messages_root);
      }
      if (loading_older && !s_at_oldest) {
        request_older_messages(true);
      } else if (loading_newer && !s_at_newest) {
        request_newer_messages(true);
      }
      return;
    }

    if (staged_load && !live_anchor && fallback_id[0]) {
      copy_cstr(selected_id, sizeof(selected_id), fallback_id);
    }

    if (staged_load && s_message_stage) {
      commit_message_stage(count);
    } else if (s_message_count > count) {
      s_message_count = count;
    }
    if (!staged_load && subreddit_detail_active() && s_subreddit_refresh_anchor_valid &&
        s_subreddit_refresh_anchor_id[0]) {
      copy_cstr(selected_id, sizeof(selected_id), s_subreddit_refresh_anchor_id);
    }
    if (!staged_load && s_message_restore_id[0]) {
      copy_cstr(selected_id, sizeof(selected_id), s_message_restore_id);
    }
    IMAGE_DIAG("PGIMG watch messages_done count=%d staged=%d visible=%d selected=%d rows=%d",
               count, staged_load ? 1 : 0, chat_visible ? 1 : 0,
               s_selected_message, s_message_count);
    if (!staged_load) {
      clear_message_stage();
    }
    int preserved_index = find_message_index_by_id(selected_id);
    if (preserved_index < 0 && staged_load && fallback_id[0]) {
      copy_cstr(selected_id, sizeof(selected_id), fallback_id);
      preserved_index = find_message_index_by_id(selected_id);
    }
    bool restored_subreddit_anchor = !staged_load && subreddit_detail_active() &&
                                   s_subreddit_refresh_anchor_valid &&
                                   preserved_index >= 0;
    bool restored_message_anchor = !staged_load && s_message_restore_id[0] &&
                                   preserved_index >= 0;
    if (preserved_index >= 0) {
      s_selected_message = preserved_index;
    } else if (!staged_load && subreddit_detail_active() && s_message_count > 0) {
      s_selected_message = 0;
    } else if (!s_user_scrolled_messages && !staged_load) {
      s_selected_message = s_at_newest ? s_message_count : (s_message_count > 0 ? s_message_count - 1 : -1);
    } else if (s_message_count > 0 && s_selected_message >= s_message_count) {
      s_selected_message = s_message_count - 1;
    } else if (s_message_count <= 0) {
      s_selected_message = -1;
    }
    if (restored_subreddit_anchor && message_is_checklist(&s_messages[preserved_index])) {
      s_selected_checklist_item = s_subreddit_refresh_anchor_checklist_item;
      clamp_subreddit_selection();
    } else {
      s_selected_checklist_item = -1;
    }
    if (!staged_load) {
      clear_subreddit_refresh_anchor();
      if (!restored_message_anchor) {
        s_message_restore_id[0] = '\0';
      }
    }
    s_loading_older_messages = false;
    s_loading_newer_messages = false;
    s_older_anchor_id[0] = '\0';
    s_newer_anchor_id[0] = '\0';
    reset_message_stream_state();
    s_expected_rows = count;
    if (!loading_older && !loading_newer && s_list_mode != ListModeTasks &&
        s_selected_chat >= 0 && s_selected_chat < s_chat_count) {
      s_chats[s_selected_chat].unread = false;
      s_chats[s_selected_chat].unread_count = 0;
    }

    if (chat_visible) {
      recalc_message_layout();
#if PT_DEBUG_FORCE_DUE_SELECTION
      if (subreddit_detail_active() && !staged_load) {
        debug_focus_due_row();
      }
#endif
      if (restored_subreddit_anchor && has_selected_message()) {
        if (message_is_checklist(&s_messages[s_selected_message]) && checklist_edit_active()) {
          ensure_selected_checklist_item_visible(false);
        } else {
          select_message_with_alignment(s_selected_message, true, false);
        }
      }
      if (restored_message_anchor && has_selected_message()) {
        s_message_restore_id[0] = '\0';
        s_user_scrolled_messages = true;
        select_message_with_alignment(s_selected_message, false, false);
        show_status(s_current_chat_title);
        return;
      }
      if (!staged_load && subreddit_detail_active()) {
        s_selected_message = s_message_count > 0 ? 0 : -1;
        s_selected_checklist_item = -1;
        set_chat_scroll_offset(0, false);
        show_status(s_current_chat_title);
        layer_mark_dirty(s_messages_root);
        s_older_anchor_y = 0;
        s_newer_anchor_y = 0;
        request_next_image();
        return;
      }
      if (staged_load && preserved_index >= 0) {
        GRect bounds = layer_get_bounds(s_messages_root);
        int margin = 6;
        if (loading_older && !loading_silent && preserved_index > 0) {
          s_selected_message = preserved_index - 1;
          set_chat_scroll_offset(s_message_y[s_selected_message] - margin, false);
          show_status(s_current_chat_title);
          layer_mark_dirty(s_messages_root);
          s_older_anchor_y = 0;
          s_newer_anchor_y = 0;
          request_next_image();
          return;
        }
        bool selected_is_tall = s_message_h[preserved_index] > bounds.size.h - (margin * 2);
        bool anchor_top = s_message_scroll_direction < 0 ||
                          (s_message_scroll_direction == 0 && loading_older);
        int target = (selected_is_tall && live_anchor) ?
                     live_anchor_scroll_offset + (s_message_y[preserved_index] - live_anchor_y) :
                     (anchor_top ?
                      s_message_y[preserved_index] - margin :
                      s_message_y[preserved_index] + s_message_h[preserved_index] + margin - bounds.size.h);
        set_chat_scroll_offset(target, false);
      }
      if (!has_selected_message() && !loading_silent) {
        scroll_to_bottom(false);
      }
      show_status(s_current_chat_title);
      layer_mark_dirty(s_messages_root);
      s_older_anchor_y = 0;
      s_newer_anchor_y = 0;
      request_next_image();
      return;
    }

    s_older_anchor_y = 0;
    s_newer_anchor_y = 0;
    if (!show_pending && loading_initial) {
      s_chat_view_pending = true;
      app_timer_register(1, show_chat_view_timer, NULL);
    }
    return;
  }

  if (strcmp(type, "chat") == 0 && index >= 0 && index < MAX_CHATS) {
    if (!s_chats) {
      s_chats_loading = false;
      show_status("Memory low");
      return;
    }
    if (index == 0) {
      s_chat_refresh_selected_id[0] = '\0';
      if (s_chat_menu && s_view_state == ViewStateChatList) {
        MenuIndex selected = menu_layer_get_selected_index(s_chat_menu);
        s_selected_chat = selected.row;
        if (s_selected_chat >= 0 && s_selected_chat < s_chat_count) {
          copy_cstr(s_chat_list_selected_id, sizeof(s_chat_list_selected_id),
                    s_chats[s_selected_chat].id);
        }
      }
      if (s_chat_list_selected_id[0]) {
        copy_cstr(s_chat_refresh_selected_id, sizeof(s_chat_refresh_selected_id),
                  s_chat_list_selected_id);
      } else if (s_selected_chat >= 0 && s_selected_chat < s_chat_count) {
        copy_cstr(s_chat_refresh_selected_id, sizeof(s_chat_refresh_selected_id),
                  s_chats[s_selected_chat].id);
      }
    }
    Chat *chat = &s_chats[index];
    char *incoming_id = tuple_cstring(iter, MESSAGE_KEY_ChatId);
    if (incoming_id && strcmp(chat->id, incoming_id) != 0) {
      preserve_chat_avatar(chat, incoming_id);
    }
    copy_cstr(chat->id, sizeof(chat->id), incoming_id);
    set_chat_strings(chat,
                     tuple_cstring(iter, MESSAGE_KEY_Sender),
                     tuple_cstring(iter, MESSAGE_KEY_Text));
    copy_cstr(chat->section, sizeof(chat->section), tuple_cstring(iter, MESSAGE_KEY_Section));
    chat->unread = tuple_int(iter, MESSAGE_KEY_IsUnread, 0) != 0;
    chat->unread_count = tuple_int(iter, MESSAGE_KEY_UnreadCount, chat->unread ? 1 : 0);
    bool list_grew = index + 1 > s_chat_count;
    if (list_grew) {
      s_chat_count = index + 1;
    }
    s_bridge_ready = true;
    s_loading_error = false;
    if (list_grew) {
      reveal_available_chat_rows();
    }
    if (s_chat_count >= s_expected_rows) {
      show_status(default_status_text());
    }
    return;
  }

  if (strcmp(type, "message") == 0 && index >= 0 && index < MAX_MESSAGES) {
    cancel_message_timeout();
    cancel_message_retry();
    bool stream_initial = s_message_stream_mode == MESSAGE_MODE_INITIAL;
    Message *message = stream_initial || !s_message_stage ? &s_messages[index] : &s_message_stage[index];
    populate_message_from_tuple(message, iter);
    if (index + 1 > s_message_stage_count && !stream_initial && s_message_stage) {
      s_message_stage_count = index + 1;
    }
    if (index + 1 > s_message_count && stream_initial) {
      s_message_count = index + 1;
    }
    if (stream_initial && !s_message_stream_silent && s_view_state == ViewStateChat && s_messages_root) {
      render_messages();
      layer_mark_dirty(s_messages_root);
    }
    int loaded_count = stream_initial ? s_message_count : s_message_stage_count;
    if (loaded_count < s_expected_rows &&
        (s_loading_messages || s_loading_older_messages || s_loading_newer_messages)) {
      schedule_message_timeout();
    }
    if (loaded_count >= s_expected_rows) {
      if (stream_initial) {
        if (!s_user_scrolled_messages && subreddit_detail_active() && s_message_count > 0) {
          s_selected_message = 0;
          s_selected_checklist_item = -1;
          set_chat_scroll_offset_quiet(0);
        } else if (!s_user_scrolled_messages) {
          s_selected_message = s_at_newest ? s_message_count : (s_message_count > 0 ? s_message_count - 1 : -1);
        } else if (s_selected_message < 0 || s_selected_message >= s_message_count) {
          s_selected_message = s_message_count > 0 ? s_message_count - 1 : s_message_count;
        }
        request_next_image();
      }
      if (stream_initial && s_loading_messages && s_view_state != ViewStateChat && !s_chat_view_pending) {
        s_chat_view_pending = true;
        app_timer_register(1, show_chat_view_timer, NULL);
      }
    }
    return;
  }

#if MEDIA_ENABLED
  if (strcmp(type, "avatar_start") == 0) {
    char *chat_id = tuple_cstring(iter, MESSAGE_KEY_ChatId);
    int image_size = tuple_int(iter, MESSAGE_KEY_ImageSize, 0);
    int transfer_id = tuple_int(iter, MESSAGE_KEY_ImageTransferId, 0);
    if (!chat_id || find_chat_index_by_id(chat_id) < 0 || image_size <= 0 || image_size > MAX_AVATAR_BYTES) {
      reset_avatar_transfer_state();
      return;
    }
    if (!ensure_avatar_transfer_buffer(image_size)) {
      reset_avatar_transfer_state();
      return;
    }
    copy_cstr(s_avatar_chat_id, sizeof(s_avatar_chat_id), chat_id);
    s_avatar_size = image_size;
    s_avatar_received = 0;
    s_avatar_expected_offset = 0;
    s_avatar_transfer_id = transfer_id;
    return;
  }

  if (strcmp(type, "avatar") == 0) {
    char *chat_id = tuple_cstring(iter, MESSAGE_KEY_ChatId);
    int offset = tuple_int(iter, MESSAGE_KEY_Index, -1);
    int transfer_id = tuple_int(iter, MESSAGE_KEY_ImageTransferId, 0);
    Tuple *data = dict_find(iter, MESSAGE_KEY_ImageData);
    int data_len = data ? data->length : 0;
    if (!chat_id || strcmp(chat_id, s_avatar_chat_id) != 0 || transfer_id != s_avatar_transfer_id || !data ||
        !s_avatar_buffer ||
        !transfer_chunk_fits(offset, data_len, s_avatar_expected_offset,
                             s_avatar_size, s_avatar_buffer_capacity)) {
      reset_avatar_transfer_state();
      return;
    }
    memcpy(s_avatar_buffer + offset, data->value->data, data_len);
    s_avatar_received = offset + data_len;
    s_avatar_expected_offset = s_avatar_received;
    return;
  }

  if (strcmp(type, "avatar_done") == 0) {
    char *chat_id = tuple_cstring(iter, MESSAGE_KEY_ChatId);
    int transfer_id = tuple_int(iter, MESSAGE_KEY_ImageTransferId, 0);
    int chat_index = find_chat_index_by_id(chat_id);
    if (chat_index >= 0 && chat_id && strcmp(chat_id, s_avatar_chat_id) == 0 &&
        transfer_id == s_avatar_transfer_id && s_avatar_received == s_avatar_size &&
        s_avatar_buffer) {
      Chat *chat = &s_chats[chat_index];
      destroy_chat_avatar(chat);
      chat->avatar_bitmap = gbitmap_create_from_png_data(s_avatar_buffer, s_avatar_size);
      if (s_chat_menu) {
        layer_mark_dirty(menu_layer_get_layer(s_chat_menu));
      }
    }
    if (chat_id && strcmp(chat_id, s_avatar_chat_id) == 0) {
      reset_avatar_transfer_state();
    }
    return;
  }

	  if (strcmp(type, "image_start") == 0) {
	    char *message_id = tuple_cstring(iter, MESSAGE_KEY_MessageId);
	    int image_size = tuple_int(iter, MESSAGE_KEY_ImageSize, 0);
	    int transfer_id = tuple_int(iter, MESSAGE_KEY_ImageTransferId, 0);
    int image_width = tuple_int(iter, MESSAGE_KEY_ImageWidth, 0);
    int image_height = tuple_int(iter, MESSAGE_KEY_ImageHeight, 0);
    char *image_format = tuple_cstring(iter, MESSAGE_KEY_Text);
    Message *message = find_message_by_image_token(message_id);
    bool is_active_image = message_id && strcmp(message_id, s_image_message_id) == 0;
    IMAGE_DIAG("PGIMG watch image_start msg=%s active=%d transfer=%d bytes=%d dims=%dx%d fmt=%s heap=%u",
               message_id ? message_id : "(null)", is_active_image ? 1 : 0,
               transfer_id, image_size, image_width, image_height,
               image_format && image_format[0] ? image_format : "png",
               image_diag_heap_free());
    if (!message || !is_active_image) {
      return;
    }
    int image_index = message_index_from_ptr(message);
    if (!message_id || transfer_id <= 0 || image_size <= 0 || image_size > MAX_IMAGE_BYTES) {
      bool retrying_image = image_size > MAX_IMAGE_BYTES &&
                             retry_active_image_request(message, "Resizing");
      IMAGE_DIAG("PGIMG watch image_start rejected msg=%s size=%d retry=%d",
                 message_id ? message_id : "(null)", image_size, retrying_image ? 1 : 0);
      if (!retrying_image) {
        message->image_requested = false;
        message->image_failed = true;
        set_message_image_error(message, image_size > MAX_IMAGE_BYTES ? "Photo too large" : "Photo start failed");
      }
      if (s_messages_root) {
        layer_mark_dirty(s_messages_root);
      }
      if (!retrying_image && message_id && strcmp(message_id, s_image_message_id) == 0) {
        reset_image_transfer_state();
      }
      if (!retrying_image) {
        request_next_image();
      }
      return;
    }
    if (image_width > 0 && image_height > 0 &&
        image_width <= IMAGE_DECODE_MAX_DIMENSION && image_height <= IMAGE_DECODE_MAX_DIMENSION) {
      bool dimensions_changed = message->image_width != image_width || message->image_height != image_height;
      message->image_width = (uint16_t)image_width;
      message->image_height = (uint16_t)image_height;
      set_message_image_progress(message, 1);
      if (dimensions_changed) {
        recalc_message_layout();
      }
    }
	    if (image_index == s_selected_message || message_needs_decode_headroom(message, image_size)) {
	      destroy_other_message_images(message);
	    }
    reset_avatar_transfer_state();
    free_full_text_body();
    if (!ensure_image_transfer_buffer(image_size)) {
      destroy_other_message_images(message);
    }
    if (!ensure_image_transfer_buffer(image_size)) {
      bool retrying_image = retry_active_image_request(message, "Resizing");
      IMAGE_DIAG("PGIMG watch image_start buffer_fail msg=%s size=%d retry=%d heap=%u",
                 message_id ? message_id : "(null)", image_size, retrying_image ? 1 : 0,
                 image_diag_heap_free());
      if (!retrying_image) {
        message->image_requested = false;
        message->image_failed = true;
        set_message_image_error(message, "Photo too large");
        reset_image_transfer_state();
      }
      if (s_messages_root) {
        layer_mark_dirty(s_messages_root);
      }
      if (!retrying_image) {
        request_next_image();
      }
      return;
    }
    copy_cstr(s_image_message_id, sizeof(s_image_message_id), message_id);
    s_image_size = image_size;
	    s_image_received = 0;
	    s_image_expected_offset = 0;
	    s_image_transfer_id = transfer_id;
    s_image_is_pbi = image_format && strcmp(image_format, "pbi") == 0;
	    copy_cstr(message->image_error, sizeof(message->image_error), "Receiving");
	    set_message_image_progress(message, 25);
	    schedule_image_transfer_timeout();
	    if (s_messages_root) {
	      layer_mark_dirty(s_messages_root);
	    }
	    return;
	  }

	  if (strcmp(type, "image_status") == 0) {
	    char *message_id = tuple_cstring(iter, MESSAGE_KEY_MessageId);
	    char *detail = tuple_cstring(iter, MESSAGE_KEY_Error);
	    Message *message = find_message_by_image_token(message_id);
	    bool is_active_image = message_id && strcmp(message_id, s_image_message_id) == 0;
    IMAGE_DIAG("PGIMG watch image_status msg=%s active=%d detail=%s",
               message_id ? message_id : "(null)", is_active_image ? 1 : 0,
               detail && detail[0] ? detail : "");
	    if (message && is_active_image && message->image_requested && !message->image_failed) {
	      copy_cstr(message->image_error, sizeof(message->image_error), detail && detail[0] ? detail : "Preparing");
	      set_message_image_progress(message, image_loading_phase_percent(message->image_error));
      if (!s_image_buffer) {
        schedule_image_prepare_timeout();
      }
	      if (s_messages_root) {
	        layer_mark_dirty(s_messages_root);
	      }
	    }
	    return;
	  }

	  if (strcmp(type, "image") == 0) {
    char *message_id = tuple_cstring(iter, MESSAGE_KEY_MessageId);
    int offset = tuple_int(iter, MESSAGE_KEY_Index, -1);
    int transfer_id = tuple_int(iter, MESSAGE_KEY_ImageTransferId, 0);
    Tuple *data = dict_find(iter, MESSAGE_KEY_ImageData);
    int data_len = data ? data->length : 0;
    if (!message_id || strcmp(message_id, s_image_message_id) != 0 ||
        transfer_id != s_image_transfer_id || !data) {
      return;
    }
    if (!s_image_buffer ||
        !transfer_chunk_fits(offset, data_len, s_image_expected_offset,
                             s_image_size, s_image_buffer_capacity)) {
      Message *message = find_message_by_image_token(message_id);
      if (message) {
        message->image_requested = false;
        message->image_failed = true;
        set_message_image_error(message, "Transfer gap");
      }
      clear_active_image_request();
      if (s_messages_root) {
        layer_mark_dirty(s_messages_root);
      }
      request_next_image();
      return;
    }
    memcpy(s_image_buffer + offset, data->value->data, data_len);
    s_image_received = offset + data_len;
    s_image_expected_offset = s_image_received;
    Message *message = find_message_by_image_token(message_id);
    if (message) {
      set_message_image_progress(message, 25 + ((progress_percent(s_image_received, s_image_size) * 75) / 100));
    }
    if (offset == 0 || s_image_received == s_image_size) {
      IMAGE_DIAG("PGIMG watch image_chunk msg=%s transfer=%d offset=%d len=%d received=%d/%d",
                 message_id ? message_id : "(null)", transfer_id, offset, data_len,
                 s_image_received, s_image_size);
    }
    schedule_image_transfer_timeout();
    if (s_messages_root) {
      layer_mark_dirty(s_messages_root);
    }
    return;
  }

  if (strcmp(type, "image_done") == 0) {
    char *message_id = tuple_cstring(iter, MESSAGE_KEY_MessageId);
    int transfer_id = tuple_int(iter, MESSAGE_KEY_ImageTransferId, 0);
    Message *message = find_message_by_image_token(message_id);
    int image_index = message_index_from_ptr(message);
    bool transfer_complete = s_image_received == s_image_size && s_image_buffer;
    bool should_keep_image = message_image_near_viewport(image_index, IMAGE_KEEP_SCREEN_MARGIN);
    bool is_active_image = message_id && strcmp(message_id, s_image_message_id) == 0 &&
                           transfer_id == s_image_transfer_id;
    bool retrying_image = false;
    IMAGE_DIAG("PGIMG watch image_done msg=%s active=%d transfer=%d complete=%d received=%d/%d pbi=%d keep=%d heap=%u",
               message_id ? message_id : "(null)", is_active_image ? 1 : 0, transfer_id,
               transfer_complete ? 1 : 0, s_image_received, s_image_size,
               s_image_is_pbi ? 1 : 0, should_keep_image ? 1 : 0,
               image_diag_heap_free());
    if (message && is_active_image) {
      if (transfer_complete) {
        bool attempted_decode = false;
        destroy_message_bitmap(message);
        destroy_other_message_images(message);
        reset_avatar_transfer_state();
        free_full_text_body();
        if (s_image_is_pbi) {
          attempted_decode = true;
          message->image_bitmap = gbitmap_create_with_data(s_image_buffer);
          if (message->image_bitmap) {
            message->image_data = s_image_buffer;
            s_image_buffer = NULL;
            s_image_buffer_capacity = 0;
          }
        } else if (message_image_decode_has_headroom(message)) {
          attempted_decode = true;
          message->image_bitmap = gbitmap_create_from_png_data(s_image_buffer, s_image_size);
        }
        if (!s_image_is_pbi && !message->image_bitmap && s_loaded_image_count > 0) {
          destroy_other_message_images(message);
          message->image_bitmap = gbitmap_create_from_png_data(s_image_buffer, s_image_size);
        }
        if (message->image_bitmap) {
          message->image_failed = false;
          message->image_error[0] = '\0';
          s_loaded_image_count++;
          sync_message_images();
          IMAGE_DIAG("PGIMG watch decode_success msg=%s pbi=%d loaded=%d heap=%u",
                     message_id ? message_id : "(null)", s_image_is_pbi ? 1 : 0,
                     s_loaded_image_count, image_diag_heap_free());
        } else {
          retrying_image = retry_active_image_request(message, "Resizing");
          IMAGE_DIAG("PGIMG watch decode_fail msg=%s attempted=%d retry=%d heap=%u",
                     message_id ? message_id : "(null)", attempted_decode ? 1 : 0,
                     retrying_image ? 1 : 0, image_diag_heap_free());
          if (!retrying_image) {
            message->image_failed = true;
            set_message_image_error(message, attempted_decode ? "Photo decode failed" : "Photo too large");
          }
        }
      } else if (should_keep_image) {
        retrying_image = retry_active_image_request(message, "Retrying");
        IMAGE_DIAG("PGIMG watch incomplete msg=%s retry=%d",
                   message_id ? message_id : "(null)", retrying_image ? 1 : 0);
        if (!retrying_image) {
          message->image_failed = true;
          set_message_image_error(message, "Photo transfer incomplete");
        }
      } else {
        message->image_failed = false;
        message->image_error[0] = '\0';
        message->image_progress = 0;
      }
      if (!retrying_image) {
        message->image_requested = false;
      }
      if (s_messages_root) {
        layer_mark_dirty(s_messages_root);
      }
    }
    if (is_active_image && !retrying_image) {
      if (s_image_retry_timer) {
        app_timer_cancel(s_image_retry_timer);
        s_image_retry_timer = NULL;
      }
      reset_image_transfer_state();
    }
    if (is_active_image && !retrying_image) {
      request_next_image();
    } else if (!retrying_image) {
      request_next_image();
    }
    return;
  }

  if (strcmp(type, "image_error") == 0) {
    char *message_id = tuple_cstring(iter, MESSAGE_KEY_MessageId);
    char *detail = tuple_cstring(iter, MESSAGE_KEY_Error);
    int transfer_id = tuple_int(iter, MESSAGE_KEY_ImageTransferId, s_image_transfer_id);
    Message *message = find_message_by_image_token(message_id);
    int image_index = message_index_from_ptr(message);
    bool is_active_image = message_id && strcmp(message_id, s_image_message_id) == 0 &&
                           transfer_id == s_image_transfer_id;
    IMAGE_DIAG("PGIMG watch image_error msg=%s active=%d transfer=%d detail=%s",
               message_id ? message_id : "(null)", is_active_image ? 1 : 0,
               transfer_id, detail && detail[0] ? detail : "");
    if (message && is_active_image) {
      message->image_requested = false;
      message->image_failed = message_image_near_viewport(image_index, IMAGE_KEEP_SCREEN_MARGIN);
      if (message->image_failed) {
        set_message_image_error(message, detail && detail[0] ? detail : "Photo prepare failed");
      } else {
        message->image_error[0] = '\0';
        message->image_progress = 0;
      }
    }
    if (is_active_image) {
      if (s_image_retry_timer) {
        app_timer_cancel(s_image_retry_timer);
        s_image_retry_timer = NULL;
      }
      reset_image_transfer_state();
    }
    if (s_messages_root) {
      layer_mark_dirty(s_messages_root);
    }
    if (is_active_image) {
      request_next_image();
    } else {
      request_next_image();
    }
    return;
  }
#endif

  if (strcmp(type, "reacted") == 0) {
    show_status("Reacted");
  }

  if (strcmp(type, "sent") == 0) {
    show_status("Sent");
    return;
  }

  if (strcmp(type, "edited") == 0) {
    show_status("Edited");
    return;
  }

  if (strcmp(type, "deleted") == 0) {
    char *deleted_id = tuple_cstring(iter, MESSAGE_KEY_MessageId);
    int deleted_index = find_message_index_by_id(deleted_id);
    if (deleted_index >= 0) {
      remove_message_at(deleted_index);
    }
    show_status("Deleted");
    return;
  }

  if (strcmp(type, "message_context") == 0) {
    char *incoming_message_id = tuple_cstring(iter, MESSAGE_KEY_MessageId);
    bool matches_selected = has_selected_message() && incoming_message_id &&
                            strcmp(s_messages[s_selected_message].id, incoming_message_id) == 0;
    bool matches_full_text = s_action_mode == ActionMenuFullText && incoming_message_id &&
                             strcmp(s_full_text_message_id, incoming_message_id) == 0;
    if (!matches_selected && !matches_full_text) {
      return;
    }
    copy_cstr(s_full_text_title, sizeof(s_full_text_title), tuple_cstring(iter, MESSAGE_KEY_Sender));
    if (s_full_text_body) {
      copy_cstr(s_full_text_body, MAX_FULL_TEXT, tuple_cstring(iter, MESSAGE_KEY_Text));
    }
    if (s_action_mode == ActionMenuFullText && s_action_layer) {
      s_full_text_scroll_offset = 0;
      layer_mark_dirty(s_action_layer);
    }
    return;
  }

  if (strcmp(type, "chat_action_done") == 0) {
    char *action = tuple_cstring(iter, MESSAGE_KEY_Text);
    char *chat_id = tuple_cstring(iter, MESSAGE_KEY_ChatId);
    int chat_index = find_chat_index_by_id(chat_id);
    if (action && strcmp(action, "archiveChat") == 0) {
      if (s_list_mode == ListModeTasks) {
        int row = chat_index >= 0 ? chat_index : s_selected_chat;
        if (row >= 0 && row < s_chat_count) {
          s_chats[row].unread_count = s_chats[row].unread_count > 0 ? 0 : 1;
          if (s_chat_menu) {
            menu_layer_reload_data(s_chat_menu);
            select_chat_row(row, false);
          }
          show_status(s_chats[row].unread_count > 0 ? "Saved" : "Unsaved");
        } else {
          show_status("Saved");
        }
      } else {
        remove_chat_at(chat_index >= 0 ? chat_index : s_selected_chat);
        show_status("Archived");
      }
    } else if (action && strcmp(action, "markUnread") == 0) {
      if (chat_index >= 0) {
        s_chats[chat_index].unread = true;
        s_chats[chat_index].unread_count = 0;
        if (s_chat_menu) {
          menu_layer_reload_data(s_chat_menu);
          select_chat_row(s_selected_chat, false);
        }
      }
      show_status("Marked unread");
    } else {
      show_status("Done");
    }
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  unlock_interactions_now();
  show_status("Message dropped");
}

static void outbox_failed_callback(DictionaryIterator *iter, AppMessageResult reason, void *context) {
#if MEDIA_ENABLED
  char *command = tuple_cstring(iter, MESSAGE_KEY_Command);
  if (command && strcmp(command, "get_image") == 0) {
    char *message_id = tuple_cstring(iter, MESSAGE_KEY_MessageId);
    Message *message = find_message_by_image_token(tuple_cstring(iter, MESSAGE_KEY_MessageId));
    if (message) {
      message->image_requested = true;
    }
    if (message_id && strcmp(message_id, s_image_message_id) == 0) {
      free_image_transfer_buffer();
      s_image_size = 0;
      s_image_received = 0;
      s_image_expected_offset = 0;
      s_image_transfer_id = 0;
    }
    schedule_image_retry();
    if (s_messages_root) {
      layer_mark_dirty(s_messages_root);
    }
    return;
  }
#endif
  unlock_interactions_now();
  show_status("Send failed");
}

static void action_window_destroy_timer_callback(void *data) {
  s_action_window_destroy_timer = NULL;
  if (s_action_window_pending_destroy) {
    Window *window = s_action_window_pending_destroy;
    s_action_window_pending_destroy = NULL;
    window_destroy(window);
  }
}

static void close_action_window(void) {
  if (s_action_window) {
    Window *window = s_action_window;
    s_action_window = NULL;
    window_stack_remove(window, false);
    if (s_action_window_destroy_timer) {
      app_timer_cancel(s_action_window_destroy_timer);
      s_action_window_destroy_timer = NULL;
    }
    if (s_action_window_pending_destroy) {
      window_destroy(s_action_window_pending_destroy);
    }
    s_action_window_pending_destroy = window;
    s_action_window_destroy_timer = app_timer_register(350, action_window_destroy_timer_callback, NULL);
  }
  DBG("PT close done h=%u", (unsigned)heap_bytes_free());
}

static ActionItem subreddit_add_item_at(int target_index) {
  int index = 0;
  if (index++ == target_index) {
    return ActionItemSubredditsComplete;
  }
  if (!subreddit_post_has_field("assigned") && index++ == target_index) {
    return ActionItemSubredditsAddAssign;
  }
  if (!subreddit_post_has_field("start") && index++ == target_index) {
    return ActionItemSubredditsAddStartDate;
  }
  if (!subreddit_post_has_field("due") && index++ == target_index) {
    return ActionItemSubredditsSetDue;
  }
  if (!subreddit_post_has_field("priority") && index++ == target_index) {
    return ActionItemSubredditsAddPriority;
  }
  if (!subreddit_post_has_field("status") && index++ == target_index) {
    return ActionItemSubredditsAddProgress;
  }
  if (!subreddit_post_has_field("bucket") && index++ == target_index) {
    return ActionItemSubredditsAddBucket;
  }
  if (!subreddit_post_has_field("notes") && index++ == target_index) {
    return ActionItemSubredditsAddNote;
  }
  if (!subreddit_post_has_field("checklist") && index++ == target_index) {
    return ActionItemSubredditsAddChecklist;
  }
  if (!subreddit_post_has_field("labels") && index++ == target_index) {
    return ActionItemSubredditsAddLabels;
  }
  if (index++ == target_index) {
    return ActionItemDeletePost;
  }
  return ActionItemConfirmCancel;
}

static int subreddit_add_item_count(void) {
  int count = 2;
  if (!subreddit_post_has_field("assigned")) {
    count++;
  }
  if (!subreddit_post_has_field("start")) {
    count++;
  }
  if (!subreddit_post_has_field("due")) {
    count++;
  }
  if (!subreddit_post_has_field("priority")) {
    count++;
  }
  if (!subreddit_post_has_field("status")) {
    count++;
  }
  if (!subreddit_post_has_field("bucket")) {
    count++;
  }
  if (!subreddit_post_has_field("notes")) {
    count++;
  }
  if (!subreddit_post_has_field("checklist")) {
    count++;
  }
  if (!subreddit_post_has_field("labels")) {
    count++;
  }
  return PG_MAX(1, count);
}

static int action_item_count(void) {
  switch (s_action_mode) {
    case ActionMenuMain:
      if (subreddit_detail_active()) {
        if (selected_message_is_thread_marker()) {
          return 1;
        }
        int count = has_selected_message() ? 4 : 3;
        if (selected_message_is_truncated()) {
          count++;
        }
        return count;
      }
      if (!has_selected_message()) {
        return 2;
      }
      return 3 +
             (s_messages[s_selected_message].outgoing ? 1 : 0) +
             (selected_message_has_context() ? 1 : 0) +
             (selected_message_is_truncated() ? 1 : 0);
    case ActionMenuChat:
      return 3;
    case ActionMenuCanned:
      return 0;
    case ActionMenuConfirm:
      return 2;
    case ActionMenuReply:
      return 1;
    case ActionMenuReactionGrid:
    case ActionMenuEmojiReplyGrid:
      return 0;
    case ActionMenuPost:
      return 4;
    case ActionMenuChecklistList:
      return 3;
    case ActionMenuChecklistItem:
      return 3;
    case ActionMenuSubredditsElement:
      return 3;
    case ActionMenuSubredditsAddItem:
      return subreddit_add_item_count();
    case ActionMenuNotes:
      return (!has_selected_message() || message_text_is_empty_field(&s_messages[s_selected_message])) ? 2 : 3;
    case ActionMenuPriority:
      return 4;
    case ActionMenuProgress:
      return 3;
    case ActionMenuLabels:
      return PLANNER_LABEL_COUNT;
    case ActionMenuSorts:
      return PG_MAX(1, s_bucket_option_count) + 1;
    case ActionMenuDate:
      return DATE_CHOICE_COUNT;
    case ActionMenuAssign:
      return s_assign_member_option_count + 3;
    case ActionMenuAssignRemove:
      return PG_MAX(1, selected_assignee_count());
    case ActionMenuAssignTeams:
      return PG_MAX(1, s_assign_team_option_count);
    case ActionMenuAssignMembers:
      return PG_MAX(1, s_assign_member_option_count);
    case ActionMenuFullText:
      return 0;
  }
  return 0;
}

static ActionItem action_item_at(int index) {
  switch (s_action_mode) {
    case ActionMenuPost:
      if (index == 0) {
        return ActionItemOpenPost;
      }
      if (index == 1) {
        return s_viewing_completed_tasks ? ActionItemSubredditsMarkActive : ActionItemSubredditsComplete;
      }
      if (index == 2) {
        return ActionItemSubredditsDownvote;
      }
      return ActionItemDeletePost;
    case ActionMenuChecklistList: {
      static const ActionItem items[] = {
        ActionItemChecklistEditList,
        ActionItemChecklistDeleteList,
        ActionItemChecklistAddItem
      };
      return items[index];
    }
    case ActionMenuChecklistItem: {
      static const ActionItem items[] = {
        ActionItemChecklistEditItem,
        ActionItemChecklistDeleteItem,
        ActionItemChecklistToggleItem
      };
      return items[index];
    }
    case ActionMenuSubredditsElement:
      if (index == 0) {
        return ActionItemSubredditsElementEdit;
      }
      if (index == 1) {
        return ActionItemSubredditsElementDelete;
      }
      return ActionItemSubredditsAddNote;
    case ActionMenuSubredditsAddItem:
      return subreddit_add_item_at(index);
    case ActionMenuNotes: {
      bool empty = !has_selected_message() || message_text_is_empty_field(&s_messages[s_selected_message]);
      if (index == 0) {
        return ActionItemNotesView;
      }
      if (index == 1) {
        return empty ? ActionItemNotesAdd : ActionItemNotesEdit;
      }
      return ActionItemNotesDelete;
    }
    case ActionMenuPriority:
      return ActionItemPriorityChoice;
    case ActionMenuProgress:
      return ActionItemProgressChoice;
    case ActionMenuLabels:
      return ActionItemLabelChoice;
    case ActionMenuSorts:
      return index == action_item_count() - 1 ? ActionItemBucketNew : ActionItemBucketChoice;
    case ActionMenuDate:
      return ActionItemDateChoice;
    case ActionMenuAssign:
      if (index == 0) {
        return ActionItemAssignMe;
      }
      if (index <= s_assign_member_option_count) {
        return ActionItemAssignConfigured;
      }
      return index == s_assign_member_option_count + 1 ?
        ActionItemAssignAdd : ActionItemAssignRemoveMenu;
    case ActionMenuAssignRemove:
      return ActionItemAssignRemove;
    case ActionMenuAssignTeams:
      return ActionItemAssignTeam;
    case ActionMenuAssignMembers:
      return ActionItemAssignMember;
    case ActionMenuReply:
      return ActionItemReplyDictate;
    case ActionMenuChat:
      if (index == 0) {
        return ActionItemArchiveChat;
      }
      if (index == 1) {
        return ActionItemMarkUnread;
      }
      return ActionItemGoBack;
    case ActionMenuCanned:
      return ActionItemGoBack;
    case ActionMenuConfirm:
      return index == 0 ? ActionItemConfirmSend : ActionItemConfirmCancel;
    case ActionMenuReactionGrid:
    case ActionMenuEmojiReplyGrid:
      return ActionItemGoBack;
    case ActionMenuMain:
      break;
    case ActionMenuFullText:
      return ActionItemGoBack;
  }

  if (subreddit_detail_active()) {
    if (selected_message_is_thread_marker()) {
      return ActionItemOpenPost;
    }
    int target = 0;
    if (has_selected_message() && index == target++) {
      return ActionItemReply;
    }
    if (index == target++) {
      return ActionItemSubredditsComplete;
    }
    if (index == target++) {
      return ActionItemSubredditsDownvote;
    }
    if (index == target++) {
      return ActionItemDeletePost;
    }
    if (selected_message_is_truncated() && index == target++) {
      return ActionItemFullText;
    }
    return ActionItemGoBack;
  }

  if (!has_selected_message()) {
    if (index == 0) {
      return ActionItemCompose;
    }
    return ActionItemGoToBottom;
  }

  int target = 0;
  if (index == target++) {
    return ActionItemReply;
  }
  if (selected_message_has_context() && index == target++) {
    return ActionItemFullContext;
  }
  if (s_messages[s_selected_message].outgoing) {
    if (index == target) {
      return ActionItemEdit;
    }
    target++;
  }
  if (selected_message_is_truncated() && index == target++) {
    return ActionItemFullText;
  }
  if (index == target++) {
    return ActionItemDelete;
  }
  return ActionItemGoToBottom;
}

static bool action_item_has_chevron(int index) {
  if (s_action_mode == ActionMenuMain) {
    ActionItem item = action_item_at(index);
    return subreddit_detail_active() && item == ActionItemReply;
  }
  if (s_action_mode == ActionMenuReply) {
    return false;
  }
  if (s_action_mode == ActionMenuAssign) {
    ActionItem item = action_item_at(index);
    return item == ActionItemAssignAdd || item == ActionItemAssignRemoveMenu;
  }
  if (s_action_mode == ActionMenuAssignRemove) {
    return false;
  }
  if (s_action_mode == ActionMenuAssignTeams) {
    return index < s_assign_team_option_count;
  }
  return false;
}

static bool action_item_has_separator_before(int index) {
  if (s_action_mode == ActionMenuMain) {
    ActionItem item = action_item_at(index);
    return item == ActionItemDelete || (!has_selected_message() && item == ActionItemGoToBottom);
  }
  return false;
}

static const char *action_item_title(int index) {
  static const char *confirm_items[] = {
    "Send",
    "Cancel"
  };
  static const char *delete_confirm_items[] = {
    "Delete",
    "Cancel"
  };

  if (s_action_mode == ActionMenuChat) {
    static const char *chat_items[] = {
      "Archive Chat",
      "Mark as Unread",
      "Go Back"
    };
    return chat_items[index];
  }
  if (s_action_mode == ActionMenuReply) {
    return "Dictate Reply";
  }
  if (s_action_mode == ActionMenuMain) {
    ActionItem item = action_item_at(index);
    switch (item) {
      case ActionItemCompose:
        return "Voice";
      case ActionItemCanned:
      case ActionItemReact:
      case ActionItemReplyEmoji:
        return "";
      case ActionItemReply:
        return "Reply";
      case ActionItemSubredditsDownvote:
        return "Downvote";
      case ActionItemEdit:
        return subreddit_detail_active() ? "Edit Comment" : "Edit Message";
      case ActionItemDelete:
        return subreddit_detail_active() ? "Delete Comment" : "Delete Message";
      case ActionItemFullText:
#if defined(PBL_PLATFORM_BASALT)
        return "Full Message";
#else
        return "View Full Message";
#endif
      case ActionItemFullContext:
        return selected_message_context_is_forward() ? "View Forward" : "View Quote";
      case ActionItemGoToBottom:
        return "Go to Bottom";
      case ActionItemSubredditsAddNote:
        return "Add Note";
      case ActionItemSubredditsAddChecklist:
        return "Add Checklist";
      case ActionItemSubredditsSetDue:
        return "Set Due";
      case ActionItemSubredditsComplete:
        return "Upvote";
      case ActionItemOpenPost:
        return selected_message_is_thread_marker() ? "Load More" : "Open Comments";
      case ActionItemCompletedPosts:
        return "Saved Posts";
      case ActionItemDeletePost:
        return selected_reddit_saved() ? "Unsave" : "Save";
      case ActionItemChecklistEditList:
        return "Edit List";
      case ActionItemChecklistDeleteList:
        return "Delete List";
      case ActionItemChecklistAddItem:
        return "Add an Item";
      case ActionItemChecklistEditItem:
        return "Edit Item";
      case ActionItemChecklistDeleteItem:
        return "Delete Item";
      case ActionItemChecklistToggleItem:
        return "Mark Complete";
      case ActionItemSubredditsElementEdit:
        return "Edit Item";
      case ActionItemSubredditsElementDelete:
        return "Delete Item";
      case ActionItemSubredditsAddAssign:
        return "Assign";
      case ActionItemSubredditsAddStartDate:
        return "Start Date";
      case ActionItemSubredditsAddPriority:
        return "Priority";
      case ActionItemSubredditsAddProgress:
        return "Progress";
      case ActionItemSubredditsAddBucket:
        return "Bucket";
      case ActionItemSubredditsAddLabels:
        return "Labels";
      case ActionItemSubredditsMarkActive:
        return "Mark Active";
      case ActionItemNotesView:
        return "View Notes";
      case ActionItemNotesEdit:
        return "Edit Note";
      case ActionItemNotesAdd:
        return "Add Note";
      case ActionItemNotesDelete:
        return "Delete Notes";
      case ActionItemPriorityChoice:
        return "Priority";
      case ActionItemProgressChoice:
        return "Progress";
      case ActionItemLabelChoice:
        return "Label";
      case ActionItemBucketChoice:
        return "Bucket";
      case ActionItemBucketNew:
        return "New Bucket";
      case ActionItemDateChoice:
        return "Date";
      case ActionItemAssignMe:
        return "Me";
      case ActionItemAssignConfigured:
        return "Assignee";
      case ActionItemAssignRemove:
        return "Assignee";
      case ActionItemAssignRemoveMenu:
        return "Remove";
      case ActionItemAssignAdd:
        return "Add from Teams";
      case ActionItemAssignTeam:
        return "Team";
      case ActionItemAssignMember:
        return "Member";
      case ActionItemEmoji:
      case ActionItemConfirmSend:
      case ActionItemConfirmCancel:
        return "";
      case ActionItemReplyDictate:
      case ActionItemReplyCanned:
      case ActionItemArchiveChat:
      case ActionItemMarkUnread:
      case ActionItemGoBack:
        return "";
    }
  }
  if (s_action_mode == ActionMenuPost) {
    static const char *post_items[] = {"Open Comments", "Upvote", "Downvote", "Save"};
    if (s_viewing_completed_tasks && index == 1) {
      return "Clear Vote";
    }
    if (index == 3) {
      return selected_reddit_saved() ? "Unsave" : "Save";
    }
    return (index >= 0 && index < 4) ? post_items[index] : "Post";
  }
  if (s_action_mode == ActionMenuCanned) {
    return "";
  }
  if (s_action_mode == ActionMenuChecklistList) {
    static const char *checklist_items[] = {
      "Edit List",
      "Delete List",
      "Add an Item"
    };
    return (index >= 0 && index < 3) ? checklist_items[index] : "Checklist";
  }
  if (s_action_mode == ActionMenuChecklistItem) {
    static const char *checklist_item_items[] = {
      "Edit Item",
      "Delete Item",
      "Mark Complete"
    };
    return (index >= 0 && index < 3) ? checklist_item_items[index] : "Checklist Item";
  }
  if (s_action_mode == ActionMenuSubredditsElement) {
    if (index == 0 || index == 1) {
      char name[24];
      selected_subreddit_element_name(name, sizeof(name));
      snprintf(index == 0 ? s_action_label_edit : s_action_label_delete,
               sizeof(s_action_label_edit),
               index == 0 ? "Edit %s" : "Delete %s",
               name);
      return index == 0 ? s_action_label_edit : s_action_label_delete;
    }
    return "Add Item";
  }
  if (s_action_mode == ActionMenuSubredditsAddItem) {
    switch (subreddit_add_item_at(index)) {
      case ActionItemSubredditsAddAssign:
        return "Assign";
      case ActionItemSubredditsComplete:
        return "Upvote";
      case ActionItemDeletePost:
        return "Save";
      case ActionItemSubredditsAddStartDate:
        return "Start Date";
      case ActionItemSubredditsSetDue:
        return "Due Date";
      case ActionItemSubredditsAddPriority:
        return "Priority";
      case ActionItemSubredditsAddProgress:
        return "Progress";
      case ActionItemSubredditsAddBucket:
        return "Bucket";
      case ActionItemSubredditsAddNote:
        return "Notes";
      case ActionItemSubredditsAddChecklist:
        return "Checklist";
      case ActionItemSubredditsAddLabels:
        return "Labels";
      default:
        return "All fields set";
    }
  }
  if (s_action_mode == ActionMenuAssign) {
    if (index == 0) {
      return "Me";
    }
    if (s_assign_member_options && index > 0 && index <= s_assign_member_option_count) {
      return s_assign_member_options[index - 1].title;
    }
    if (index == s_assign_member_option_count + 1) {
      return "Add from Teams";
    }
    return "Remove";
  }
  if (s_action_mode == ActionMenuAssignRemove) {
    if (selected_assignee_count() <= 0) {
      return "No Assignees";
    }
    if (index < selected_assignee_count()) {
      checklist_line_at(has_selected_message() ? s_messages[s_selected_message].text : "",
                        index, s_action_label_edit, sizeof(s_action_label_edit));
      return s_action_label_edit[0] ? s_action_label_edit : "Assignee";
    }
    return "Assignee";
  }
  if (s_action_mode == ActionMenuAssignTeams) {
    if (s_assign_team_options && index >= 0 && index < s_assign_team_option_count) {
      return s_assign_team_options[index].title;
    }
    return "Loading Teams";
  }
  if (s_action_mode == ActionMenuAssignMembers) {
    if (s_assign_member_options && index >= 0 && index < s_assign_member_option_count) {
      return s_assign_member_options[index].title;
    }
    return "Loading Members";
  }
  if (s_action_mode == ActionMenuNotes) {
    bool empty = !has_selected_message() || message_text_is_empty_field(&s_messages[s_selected_message]);
    if (index == 0) {
      return "View Notes";
    }
    if (index == 1) {
      return empty ? "Add Note" : "Edit Note";
    }
    return "Delete Notes";
  }
  if (s_action_mode == ActionMenuPriority) {
    static const char *priorities[] = {"Urgent", "Important", "Medium", "Low"};
    return (index >= 0 && index < 4) ? priorities[index] : "Priority";
  }
  if (s_action_mode == ActionMenuProgress) {
    static const char *progress[] = {"Not Started", "In Progress", "Complete"};
    return (index >= 0 && index < 3) ? progress[index] : "Progress";
  }
  if (s_action_mode == ActionMenuLabels) {
    subreddit_label_text(index, s_action_label_edit, sizeof(s_action_label_edit));
    return s_action_label_edit;
  }
  if (s_action_mode == ActionMenuSorts) {
    if (index == action_item_count() - 1) {
      return "New Bucket";
    }
    if (s_bucket_option_count <= 0) {
      return s_current_bucket_title[0] ? s_current_bucket_title : "Current Bucket";
    }
    return (index >= 0 && index < s_bucket_option_count) ? s_bucket_options[index].title : "Bucket";
  }
  if (s_action_mode == ActionMenuDate) {
    return (index >= 0 && index < DATE_CHOICE_COUNT) ? DATE_CHOICE_LABELS[index] : "Date";
  }
  if (s_action_mode == ActionMenuConfirm && s_pending_chat_command[0] &&
      !pending_command_is_subreddit_action()) {
    return delete_confirm_items[index];
  }
  return confirm_items[index];
}

// Custom action sheet instead of ActionMenu: it keeps behavior identical across
// the SDK targets this app supports.
static void action_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, ACTION_BG);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  int rail_w = ROUND_UI ? 0 : 18;
  int content_x = ROUND_UI ? 28 : 24;
  int content_w = bounds.size.w - content_x - (ROUND_UI ? 24 : 0);
  graphics_context_set_fill_color(ctx, APP_COLOR);
  if (ROUND_UI) {
    graphics_fill_rect(ctx, GRect(0, 0, 12, bounds.size.h), 0, GCornerNone);
  } else {
    graphics_fill_rect(ctx, GRect(0, 0, rail_w, bounds.size.h), 0, GCornerNone);
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_circle(ctx, GPoint(rail_w / 2, 10), 2);
  }

  int count = action_item_count();
  int row_h = ROUND_UI ? 32 : 48;
  int top = ROUND_UI ? PG_MAX(0, (bounds.size.h - (count * row_h)) / 2) : 0;

  if (s_action_mode == ActionMenuConfirm) {
    graphics_context_set_text_color(ctx, ACTION_TEXT);
    graphics_draw_text(ctx, s_pending_text, fonts_get_system_font(FONT_KEY_GOTHIC_18),
                       GRect(content_x + 6, 10, content_w - 12, 70),
                       GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
    top = bounds.size.h - (count * row_h) - 8;
  }

  if (s_action_mode == ActionMenuFullText) {
    char title[MAX_SENDER + 10];
    const char *text = "";
    const char *heading = NULL;
    GFont full_font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
    GFont heading_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
    int text_w = content_w - 12;
    int heading_h = 0;
    GSize text_size;

    title[0] = '\0';
    if (s_full_text_context) {
      copy_cstr(title, sizeof(title), s_full_text_title);
      heading = title;
    }
    text = s_full_text_body ? s_full_text_body : "";

    if (heading && heading[0]) {
      GSize heading_size = graphics_text_layout_get_content_size(
        heading, heading_font, GRect(0, 0, text_w, 2000),
        GTextOverflowModeWordWrap, GTextAlignmentLeft
      );
      heading_h = heading_size.h + 4;
    }
    text_size = graphics_text_layout_get_content_size(
      text, full_font, GRect(0, 0, text_w, 2000),
      GTextOverflowModeWordWrap, GTextAlignmentLeft
    );
    s_full_text_height = heading_h + text_size.h + 20;
    int max_scroll = PG_MAX(0, s_full_text_height - bounds.size.h + 8);
    s_full_text_scroll_offset = PG_MIN(s_full_text_scroll_offset, max_scroll);

    graphics_context_set_text_color(ctx, ACTION_TEXT_SELECTED);
    if (heading && heading[0]) {
      graphics_draw_text(ctx, heading, heading_font,
                         GRect(content_x + 6, 8 - s_full_text_scroll_offset, text_w, heading_h),
                         GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
    }
    graphics_draw_text(ctx, text, full_font,
                       GRect(content_x + 6, 8 + heading_h - s_full_text_scroll_offset,
                             text_w, text_size.h + 16),
                       GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
    return;
  }

  if (s_action_mode != ActionMenuConfirm) {
    int list_h = count * row_h;
    if (list_h > bounds.size.h) {
      int visible_rows = PG_MAX(1, bounds.size.h / row_h);
      int first_row = s_action_selected - (visible_rows / 2);
      int max_first_row = PG_MAX(0, count - visible_rows);
      first_row = PG_MAX(0, PG_MIN(first_row, max_first_row));
      top = -(first_row * row_h);
    }
  }

  for (int i = 0; i < count; i++) {
    GRect row = GRect(content_x, top + (i * row_h), content_w, row_h);
    bool selected = i == s_action_selected;

    if (action_item_has_separator_before(i)) {
      int line_y = row.origin.y - 3;
      graphics_context_set_stroke_color(ctx, GColorLightGray);
      graphics_draw_line(ctx, GPoint(row.origin.x + 8, line_y),
                         GPoint(row.origin.x + row.size.w - 1, line_y));
    }

    graphics_context_set_text_color(ctx, selected ? ACTION_TEXT_SELECTED : ACTION_TEXT);
    graphics_draw_text(ctx, action_item_title(i), fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
                       GRect(row.origin.x + 1, row.origin.y + 1,
                             row.size.w - (action_item_has_chevron(i) ? 34 : 4), row.size.h - 3),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
    if (action_item_has_chevron(i)) {
      graphics_draw_text(ctx, ">>", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
                         GRect(row.origin.x + row.size.w - 32, row.origin.y + 1, 30, row.size.h - 3),
                         GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);
    }
  }
}

static void show_action_window(ActionMenuMode mode) {
  if (s_action_window) {
    close_action_window();
  }
  close_touch_keyboard();
  if (s_view_state == ViewStateChat) {
    show_status(s_current_chat_title);
  } else {
    show_status(default_status_text());
  }
  s_action_mode = mode;
  s_action_selected = 0;
  s_action_window = window_create();
  if (!s_action_window) {
    unlock_interactions_now();
    show_status("Memory low");
    return;
  }
  window_set_background_color(s_action_window, ACTION_BG);
  window_set_click_config_provider(s_action_window, action_click_config_provider);
  window_set_window_handlers(s_action_window, (WindowHandlers) {
    .unload = action_window_unload
  });

  Layer *window_layer = window_get_root_layer(s_action_window);
  GRect bounds = layer_get_bounds(window_layer);
  s_action_layer = layer_create(bounds);
  if (!s_action_layer) {
    window_destroy(s_action_window);
    s_action_window = NULL;
    unlock_interactions_now();
    show_status("Memory low");
    return;
  }
  layer_set_update_proc(s_action_layer, action_layer_update_proc);
  layer_add_child(window_layer, s_action_layer);

  window_stack_push(s_action_window, true);

  if (mode == ActionMenuAssign) {
    clear_assign_picker_options(false);
    show_status("Loading assignees...");
    send_command_with_status("get_assign_configured", NULL, NULL, NULL, NULL, false);
    unlock_interactions_now();
  }
}

static void dictation_confirm_timer_callback(void *data) {
  s_dictation_confirm_timer = NULL;
  if (pending_command_is_subreddit_action() && !pending_subreddit_action_is_current()) {
    clear_pending_text_action();
    show_status("Action canceled");
    return;
  }
  commit_pending_text_action();
}

static void schedule_dictation_confirm(void) {
  if (s_dictation_confirm_timer) {
    app_timer_cancel(s_dictation_confirm_timer);
    s_dictation_confirm_timer = NULL;
  }
  s_dictation_confirm_timer = app_timer_register(DICTATION_CONFIRM_DELAY_MS,
                                                 dictation_confirm_timer_callback, NULL);
}

static void dictation_failure_timer_callback(void *data) {
  s_dictation_failure_timer = NULL;
  clear_pending_text_action();
  s_dictation_in_progress = false;
  unlock_interactions_now();
  show_status("Dictation canceled");
}

static void schedule_dictation_failure(void) {
  if (s_dictation_failure_timer) {
    app_timer_cancel(s_dictation_failure_timer);
    s_dictation_failure_timer = NULL;
  }
  s_dictation_failure_timer = app_timer_register(500, dictation_failure_timer_callback, NULL);
}

static void dictation_callback(DictationSession *session, DictationSessionStatus status,
                               char *transcription, void *context) {
  if (session != s_dictation_session) {
    return;
  }
  s_dictation_in_progress = false;
  if (status == DictationSessionStatusSuccess && transcription) {
    if (s_dictation_failure_timer) {
      app_timer_cancel(s_dictation_failure_timer);
      s_dictation_failure_timer = NULL;
    }
    copy_cstr(s_pending_text, sizeof(s_pending_text), transcription);
    schedule_dictation_confirm();
  } else {
    schedule_dictation_failure();
  }
}

static void start_dictation(void) {
  if (s_dictation_in_progress || s_dictation_confirm_timer || s_dictation_failure_timer) {
    show_status("Voice busy");
    return;
  }
  if (!s_dictation_session) {
    s_dictation_session = dictation_session_create(MAX_TEXT - 1, dictation_callback, NULL);
    if (!s_dictation_session) {
      show_status("Voice unavailable");
      return;
    }
#if defined(_PBL_API_EXISTS_dictation_session_enable_confirmation)
    dictation_session_enable_confirmation(s_dictation_session, false);
#endif
  }
  s_dictation_in_progress = true;
  DictationSessionStatus status = dictation_session_start(s_dictation_session);
  if (status != DictationSessionStatusSuccess) {
    s_dictation_in_progress = false;
    schedule_dictation_failure();
  }
}

static void dictation_start_timer_callback(void *data) {
  s_dictation_start_timer = NULL;
  if (pending_command_is_subreddit_action() && !pending_subreddit_action_is_current()) {
    clear_pending_text_action();
    show_status("Action canceled");
    return;
  }
  start_dictation();
}

static void schedule_dictation_start(void) {
  if (s_dictation_start_timer) {
    app_timer_cancel(s_dictation_start_timer);
    s_dictation_start_timer = NULL;
  }
  s_dictation_start_timer = app_timer_register(DICTATION_START_DELAY_MS,
                                               dictation_start_timer_callback, NULL);
}

static void action_window_unload(Window *window) {
  CDBG("ub %p %p", window, s_action_window);
  if (s_action_layer) {
    layer_destroy(s_action_layer);
    s_action_layer = NULL;
  }
  free_full_text_body();
  if (s_action_window == window) {
    s_action_window = NULL;
  }
}

static void action_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_interaction_locked) {
    show_status("One moment...");
    return;
  }
  if (s_action_mode == ActionMenuFullText) {
    lock_interactions(300);
    close_action_window();
    return;
  }

  ActionMenuMode mode = s_action_mode;
  int selected = s_action_selected;
  ActionItem item = action_item_at(selected);

  if (mode == ActionMenuPost &&
      (s_view_state != ViewStateChatList || s_list_mode != ListModeTasks ||
       s_selected_chat < 0 || s_selected_chat >= s_chat_count)) {
    close_action_window();
    show_status("Action canceled");
    return;
  }
  if ((mode == ActionMenuChecklistList || mode == ActionMenuChecklistItem) &&
      (!subreddit_detail_active() || !has_selected_message() ||
       !message_is_checklist(&s_messages[s_selected_message]))) {
    close_action_window();
    show_status("Action canceled");
    return;
  }
  if (mode == ActionMenuAssign &&
      !subreddit_detail_active()) {
    close_action_window();
    show_status("Action canceled");
    return;
  }
  if ((mode == ActionMenuAssignRemove || mode == ActionMenuAssignTeams || mode == ActionMenuAssignMembers) &&
      !subreddit_detail_active()) {
    close_action_window();
    show_status("Action canceled");
    return;
  }

  lock_interactions(350);
  if (mode == ActionMenuSubredditsElement && selected == 2) {
    close_action_window();
    show_action_window(ActionMenuSubredditsAddItem);
    return;
  }

  switch (item) {
    case ActionItemCompose:
      s_pending_edit_message_id[0] = '\0';
      s_pending_chat_command[0] = '\0';
      s_pending_send_as_reply = false;
      close_action_window();
      schedule_dictation_start();
      return;
    case ActionItemCanned:
    case ActionItemReplyCanned:
    case ActionItemEmoji:
    case ActionItemReplyEmoji:
      close_action_window();
      show_status("Action unavailable");
      return;
    case ActionItemReply:
    case ActionItemReplyDictate:
      s_pending_edit_message_id[0] = '\0';
      s_pending_chat_command[0] = '\0';
      s_pending_send_as_reply = true;
      close_action_window();
      schedule_dictation_start();
      return;
    case ActionItemReact:
      close_action_window();
      show_status("Action unavailable");
      return;
    case ActionItemEdit:
      if (has_selected_message() && s_messages[s_selected_message].outgoing) {
        copy_cstr(s_pending_edit_message_id, sizeof(s_pending_edit_message_id),
                  s_messages[s_selected_message].id);
        s_pending_chat_command[0] = '\0';
        s_pending_send_as_reply = false;
        close_action_window();
        schedule_dictation_start();
      } else {
        close_action_window();
        show_status("Action canceled");
      }
      return;
    case ActionItemDelete:
      if (has_selected_message()) {
        char message_id[MAX_ID];
        copy_cstr(message_id, sizeof(message_id), s_messages[s_selected_message].id);
        show_status("Deleting...");
        send_command("delete_message", s_current_chat_id, NULL, NULL, message_id);
      }
      close_action_window();
      return;
    case ActionItemFullText:
      close_action_window();
      s_full_text_context = false;
      if (!has_selected_message() || !ensure_full_text_body()) {
        show_status("Memory low");
        return;
      }
      copy_cstr(s_full_text_message_id, sizeof(s_full_text_message_id),
                s_messages[s_selected_message].id);
      s_full_text_title[0] = '\0';
      copy_cstr(s_full_text_body, MAX_FULL_TEXT, s_messages[s_selected_message].text);
      send_command_with_status("get_message_text", s_current_chat_id, NULL, NULL,
                               s_full_text_message_id, false);
      s_full_text_unloaded_messages = true;
      clear_message_rows();
      set_chat_scroll_offset_quiet(0);
      if (s_messages_root) {
        layer_mark_dirty(s_messages_root);
      }
      s_full_text_scroll_offset = 0;
      show_action_window(ActionMenuFullText);
      return;
    case ActionItemFullContext: {
      close_action_window();
      s_full_text_context = true;
      s_full_text_unloaded_messages = false;
      if (!has_selected_message() || !ensure_full_text_body()) {
        show_status("Memory low");
        return;
      }
      copy_cstr(s_full_text_message_id, sizeof(s_full_text_message_id),
                s_messages[s_selected_message].id);
      char body[MAX_CONTEXT_TEXT];
      message_context_strings(&s_messages[s_selected_message], s_full_text_title,
                              sizeof(s_full_text_title), body, sizeof(body));
      copy_cstr(s_full_text_body, MAX_FULL_TEXT, body);
      send_command_with_status("get_context", s_current_chat_id, NULL, NULL,
                               s_full_text_message_id, false);
      s_full_text_scroll_offset = 0;
      show_action_window(ActionMenuFullText);
      return;
    }
    case ActionItemArchiveChat:
      send_selected_chat_action("archive_chat");
      close_action_window();
      return;
    case ActionItemMarkUnread:
      if (s_selected_chat >= 0 && s_selected_chat < s_chat_count) {
        s_chats[s_selected_chat].unread = true;
        s_chats[s_selected_chat].unread_count = 0;
        if (s_chat_menu) {
          menu_layer_reload_data(s_chat_menu);
          select_chat_row(s_selected_chat, false);
        }
      }
      send_selected_chat_action("mark_unread");
      close_action_window();
      return;
    case ActionItemSubredditsSetDue:
      close_action_window();
      open_date_window("set_task_due", "Due in days");
      return;
    case ActionItemSubredditsAddStartDate:
      close_action_window();
      open_date_window("set_task_start", "Start in days");
      return;
    case ActionItemSubredditsAddPriority:
      close_action_window();
      show_action_window(ActionMenuPriority);
      return;
    case ActionItemSubredditsAddProgress:
      close_action_window();
      show_action_window(ActionMenuProgress);
      return;
    case ActionItemSubredditsAddBucket:
      close_action_window();
      show_action_window(ActionMenuSorts);
      return;
    case ActionItemSubredditsAddLabels:
      close_action_window();
      show_action_window(ActionMenuLabels);
      return;
    case ActionItemSubredditsAddNote:
      set_pending_subreddit_voice_action("add_task_note", NULL);
      close_action_window();
      schedule_dictation_start();
      return;
    case ActionItemSubredditsAddChecklist:
    case ActionItemChecklistAddItem:
      set_pending_subreddit_voice_action("add_checklist_item", NULL);
      close_action_window();
      schedule_dictation_start();
      return;
    case ActionItemSubredditsAddAssign:
      close_action_window();
      show_action_window(ActionMenuAssign);
      return;
    case ActionItemAssignMe:
      show_status("Assigning...");
      capture_subreddit_refresh_anchor();
      append_assignee_locally("Me");
      queue_subreddit_command("assign_task", s_current_chat_id, "Me", "me");
      close_action_window();
      return;
    case ActionItemAssignConfigured:
      if (s_assign_member_options && selected > 0 && selected <= s_assign_member_option_count) {
        show_status("Assigning...");
        capture_subreddit_refresh_anchor();
        append_assignee_locally(s_assign_member_options[selected - 1].title);
        queue_subreddit_command("assign_task", s_current_chat_id,
                              s_assign_member_options[selected - 1].title,
                              s_assign_member_options[selected - 1].id);
        close_action_window();
      } else {
        unlock_interactions_now();
        show_status("Still loading");
      }
      return;
    case ActionItemAssignRemoveMenu:
      s_action_mode = ActionMenuAssignRemove;
      s_action_selected = 0;
      if (s_action_layer) {
        layer_mark_dirty(s_action_layer);
      }
      unlock_interactions_now();
      return;
    case ActionItemSubredditsComplete:
    case ActionItemSubredditsMarkActive:
    case ActionItemSubredditsDownvote:
    case ActionItemDeletePost:
    case ActionItemChecklistEditList:
    case ActionItemChecklistDeleteList:
    case ActionItemChecklistDeleteItem:
    case ActionItemChecklistToggleItem:
    case ActionItemSubredditsElementDelete:
    case ActionItemNotesDelete:
    case ActionItemPriorityChoice:
    case ActionItemProgressChoice:
    case ActionItemLabelChoice:
    case ActionItemBucketChoice:
      capture_subreddit_intent(item, selected);
      close_action_window();
      schedule_captured_subreddit_intent(200);
      return;
    case ActionItemOpenPost:
      if (subreddit_detail_active() && selected_message_is_thread_marker()) {
        char message_id[MAX_ID];
        copy_cstr(message_id, sizeof(message_id), s_messages[s_selected_message].id);
        close_action_window();
        show_status("Loading replies...");
        send_command_with_status("expand_replies", s_current_chat_id, NULL, NULL, message_id, true);
        return;
      }
      capture_subreddit_intent(item, selected);
      close_action_window();
      schedule_captured_subreddit_intent(200);
      return;
    case ActionItemChecklistEditItem:
      if (s_selected_checklist_item >= 0) {
        char item_id[12];
        snprintf(item_id, sizeof(item_id), "%d", s_selected_checklist_item);
        set_pending_subreddit_voice_action("edit_checklist_item", item_id);
        close_action_window();
        schedule_dictation_start();
      } else {
        close_action_window();
        show_status("Action canceled");
      }
      return;
    case ActionItemSubredditsElementEdit:
      if (has_selected_message()) {
        char message_id[MAX_ID];
        copy_cstr(message_id, sizeof(message_id), s_messages[s_selected_message].id);
        set_pending_subreddit_voice_action("edit_task_element", message_id);
        close_action_window();
        schedule_dictation_start();
      } else {
        close_action_window();
        show_status("Action canceled");
      }
      return;
    case ActionItemNotesView:
      close_action_window();
      if (!ensure_full_text_body()) {
        show_status("Memory low");
        return;
      }
      copy_cstr(s_full_text_title, sizeof(s_full_text_title), "Notes");
      copy_cstr(s_full_text_body, MAX_FULL_TEXT,
                has_selected_message() && !message_text_is_empty_field(&s_messages[s_selected_message]) ?
                s_messages[s_selected_message].text : "No notes yet");
      s_full_text_context = true;
      s_full_text_scroll_offset = 0;
      show_action_window(ActionMenuFullText);
      return;
    case ActionItemNotesEdit:
      if (has_selected_message()) {
        char message_id[MAX_ID];
        copy_cstr(message_id, sizeof(message_id), s_messages[s_selected_message].id);
        set_pending_subreddit_voice_action("edit_task_element", message_id);
        close_action_window();
        schedule_dictation_start();
      } else {
        close_action_window();
        show_status("Action canceled");
      }
      return;
    case ActionItemNotesAdd:
      set_pending_subreddit_voice_action("add_task_note", NULL);
      close_action_window();
      schedule_dictation_start();
      return;
    case ActionItemBucketNew:
      set_pending_subreddit_voice_action("create_bucket", NULL);
      close_action_window();
      schedule_dictation_start();
      return;
    case ActionItemAssignRemove: {
      if (selected_assignee_count() <= 0) {
        unlock_interactions_now();
        show_status("No assignees");
        return;
      }
      char item_id[12];
      snprintf(item_id, sizeof(item_id), "%d", selected);
      show_status("Removing assignee...");
      capture_subreddit_refresh_anchor();
      remove_assignee_locally(selected);
      queue_subreddit_command("unassign_task", s_current_chat_id, NULL, item_id);
      close_action_window();
      return;
    }
    case ActionItemAssignAdd: {
      clear_assign_picker_options(true);
      clear_assign_picker_options(false);
      s_assign_selected_team_id[0] = '\0';
      s_action_mode = ActionMenuAssignTeams;
      s_action_selected = 0;
      show_status("Loading teams...");
      if (s_action_layer) {
        layer_mark_dirty(s_action_layer);
      }
      send_command_with_status("get_assign_teams", NULL, NULL, NULL, NULL, false);
      unlock_interactions_now();
      return;
    }
    case ActionItemAssignTeam:
      if (s_assign_team_options && selected >= 0 && selected < s_assign_team_option_count) {
        copy_cstr(s_assign_selected_team_id, sizeof(s_assign_selected_team_id),
                  s_assign_team_options[selected].id);
        clear_assign_picker_options(false);
        s_action_mode = ActionMenuAssignMembers;
        s_action_selected = 0;
        show_status("Loading members...");
        if (s_action_layer) {
          layer_mark_dirty(s_action_layer);
        }
        send_command_with_status("get_assign_members", s_assign_selected_team_id, NULL, NULL, NULL, false);
        unlock_interactions_now();
      } else {
        unlock_interactions_now();
        show_status("Still loading");
      }
      return;
    case ActionItemAssignMember:
      if (s_assign_member_options && selected >= 0 && selected < s_assign_member_option_count) {
        show_status("Assigning...");
        capture_subreddit_refresh_anchor();
        append_assignee_locally(s_assign_member_options[selected].title);
        queue_subreddit_command("assign_task", s_current_chat_id,
                              s_assign_member_options[selected].title,
                              s_assign_member_options[selected].id);
        close_action_window();
      } else {
        unlock_interactions_now();
        show_status("Still loading");
      }
      return;
    case ActionItemDateChoice:
      if (selected >= 0 && selected < DATE_CHOICE_COUNT) {
        char command[sizeof(s_date_command)];
        char target_id[MAX_ID];
        char date[16];
        copy_cstr(command, sizeof(command), s_date_command);
        copy_cstr(target_id, sizeof(target_id), s_date_target_id);
        format_date_offset(DATE_CHOICE_OFFSETS[selected], date, sizeof(date));
        close_action_window();
        if (date_target_is_current() && command[0] && target_id[0] && date[0]) {
          schedule_date_send(command, target_id, date);
        } else {
          s_date_command[0] = '\0';
          s_date_label[0] = '\0';
          s_date_target_id[0] = '\0';
          show_status("Date canceled");
        }
      }
      return;
    case ActionItemGoToBottom:
      close_action_window();
      go_to_bottom();
      return;
    case ActionItemConfirmCancel:
      close_action_window();
      return;
    case ActionItemConfirmSend:
      close_action_window();
      commit_pending_text_action();
      return;
    default:
      close_action_window();
      show_status("Action canceled");
      return;
  }
}

static void action_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_action_mode == ActionMenuFullText) {
    s_full_text_scroll_offset = PG_MAX(0, s_full_text_scroll_offset - CHAT_SCROLL_DELTA);
    layer_mark_dirty(s_action_layer);
    return;
  }
  if (s_action_selected > 0) {
    s_action_selected--;
    layer_mark_dirty(s_action_layer);
  }
}

static void action_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_action_mode == ActionMenuFullText) {
    if (s_action_layer) {
      GRect bounds = layer_get_bounds(s_action_layer);
      int max_scroll = PG_MAX(0, s_full_text_height - bounds.size.h + 8);
      s_full_text_scroll_offset = PG_MIN(max_scroll, s_full_text_scroll_offset + CHAT_SCROLL_DELTA);
      layer_mark_dirty(s_action_layer);
    }
    return;
  }
  if (s_action_selected < action_item_count() - 1) {
    s_action_selected++;
    layer_mark_dirty(s_action_layer);
  }
}

static void action_back_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_interaction_locked) {
    return;
  }
  lock_interactions(300);
  if (s_action_mode == ActionMenuCanned || s_action_mode == ActionMenuConfirm ||
      s_action_mode == ActionMenuReply || s_action_mode == ActionMenuReactionGrid ||
      s_action_mode == ActionMenuEmojiReplyGrid || s_action_mode == ActionMenuFullText) {
    if (s_action_mode == ActionMenuFullText) {
      bool restore_messages = s_full_text_unloaded_messages && s_full_text_message_id[0];
      char restore_id[MAX_ID];
      restore_id[0] = '\0';
      if (restore_messages) {
        copy_cstr(restore_id, sizeof(restore_id), s_full_text_message_id);
      }
      free_full_text_body();
      if (restore_messages) {
        copy_cstr(s_message_restore_id, sizeof(s_message_restore_id), restore_id);
        s_loading_messages = true;
        s_user_scrolled_messages = true;
        show_status("Restoring messages...");
        close_action_window();
        send_command_with_status("restore_messages", s_current_chat_id, NULL, NULL, restore_id, false);
        return;
      }
    }
    s_action_mode = ActionMenuMain;
    s_action_selected = 0;
    layer_mark_dirty(s_action_layer);
  } else if (s_action_mode == ActionMenuAssignRemove) {
    s_action_mode = ActionMenuAssign;
    s_action_selected = 0;
    layer_mark_dirty(s_action_layer);
  } else if (s_action_mode == ActionMenuAssignMembers) {
    s_action_mode = ActionMenuAssignTeams;
    s_action_selected = 0;
    layer_mark_dirty(s_action_layer);
  } else if (s_action_mode == ActionMenuAssignTeams) {
    s_action_mode = ActionMenuAssign;
    s_action_selected = 0;
    layer_mark_dirty(s_action_layer);
  } else {
    close_action_window();
  }
}

static void action_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, action_select_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_UP, REPEAT_SCROLL_MS, action_up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, REPEAT_SCROLL_MS, action_down_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, action_back_click_handler);
}

static void main_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_view_state == ViewStateChatList && interaction_busy(s_loading_messages ? "Loading messages..." : "Loading...")) {
    return;
  }
  if (s_view_state == ViewStateChat && subreddit_detail_active() && interaction_busy("One moment...")) {
    return;
  }
  if (s_view_state == ViewStateChat) {
    if (subreddit_detail_active() && selected_message_is_thread_marker()) {
      char message_id[MAX_ID];
      copy_cstr(message_id, sizeof(message_id), s_messages[s_selected_message].id);
      lock_interactions(350);
      show_status("Loading replies...");
      send_command_with_status("expand_replies", s_current_chat_id, NULL, NULL, message_id, true);
      return;
    }
    if (s_list_mode == ListModeTasks) {
      lock_interactions(350);
      show_action_window(ActionMenuMain);
      return;
    }
    lock_interactions(350);
    show_action_window(ActionMenuMain);
  } else if (s_view_state == ViewStateChatList && s_chat_menu) {
    MenuIndex index = menu_layer_get_selected_index(s_chat_menu);
    chat_menu_select_callback(s_chat_menu, &index, NULL);
  }
}

static void main_select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_view_state == ViewStateChatList && interaction_busy("Loading...")) {
    return;
  }
  if (s_view_state == ViewStateChat && subreddit_detail_active() && interaction_busy("One moment...")) {
    return;
  }
  if (s_view_state == ViewStateChat && subreddit_detail_active()) {
    if (has_selected_message()) {
      if (message_is_notes(&s_messages[s_selected_message])) {
        lock_interactions(350);
        show_action_window(ActionMenuNotes);
        return;
      }
      if (message_is_checklist(&s_messages[s_selected_message])) {
        if (s_checklist_edit_mode && s_selected_checklist_item >= 0 &&
            s_selected_checklist_item < checklist_item_count(s_messages[s_selected_message].text)) {
          lock_interactions(350);
          show_action_window(ActionMenuChecklistItem);
        } else if (!s_checklist_edit_mode && s_selected_checklist_item < 0) {
          lock_interactions(350);
          show_action_window(ActionMenuChecklistList);
        } else {
          lock_interactions(350);
          show_action_window(ActionMenuSubredditsAddItem);
        }
        return;
      }
    }
    lock_interactions(350);
    show_action_window(ActionMenuMain);
    return;
  }
  if (s_view_state == ViewStateChatList && !s_chats_loading && !s_loading_messages &&
      s_chat_count > 0 && s_chat_menu) {
    MenuIndex index = menu_layer_get_selected_index(s_chat_menu);
    s_selected_chat = index.row;
    if (s_list_mode == ListModeSubreddits && s_selected_chat >= 0 && s_selected_chat < s_chat_count &&
        strcmp(s_chats[s_selected_chat].id, "__pin_divider") != 0) {
      lock_interactions(700);
      send_command_with_status("toggle_plan_pin", s_chats[s_selected_chat].id, NULL, NULL, NULL, false);
      return;
    }
    if (s_list_mode == ListModeTasks && s_selected_chat >= 0 && s_selected_chat < s_chat_count) {
      if (strcmp(s_chats[s_selected_chat].id, "__new_task") == 0) {
        lock_interactions(500);
        show_status("Posting disabled");
        return;
      }
      if (strcmp(s_chats[s_selected_chat].id, "__completed_tasks") == 0) {
        lock_interactions(700);
        request_completed_tasks(s_current_bucket_id, s_current_bucket_title);
      } else {
        lock_interactions(350);
        show_action_window(ActionMenuPost);
      }
      return;
    }
    if (s_list_mode == ListModeChats || s_list_mode == ListModeGroupChats) {
      lock_interactions(350);
      show_action_window(ActionMenuChat);
    }
  }
}

static void main_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  bool repeating = click_is_repeating(recognizer);
  if (s_view_state == ViewStateChatList && s_chat_menu) {
    if (s_chats_loading || s_loading_messages || s_chat_view_pending) {
      show_status(s_loading_messages ? "Loading messages..." : "Loading...");
      return;
    }
    menu_layer_set_selected_next(s_chat_menu, true, MenuRowAlignCenter, !repeating);
    s_selected_chat = menu_layer_get_selected_index(s_chat_menu).row;
    if (chat_row_is_pin_divider(s_selected_chat)) {
      select_chat_row(nearest_selectable_chat_row(s_selected_chat, -1), !repeating);
      return;
    }
    if (subreddit_list_active() && s_selected_chat >= s_chat_count) {
      select_chat_row(s_chat_count - 1, !repeating);
      return;
    }
    if (s_selected_chat >= 0 && s_selected_chat < s_chat_count) {
      copy_cstr(s_chat_list_selected_id, sizeof(s_chat_list_selected_id), s_chats[s_selected_chat].id);
    }
    return;
  }
  if (s_view_state != ViewStateChat || !s_messages_root || s_message_count == 0) {
    return;
  }
  s_user_scrolled_messages = true;
  bool reversed_direction = s_message_scroll_direction == 1;
  s_message_scroll_direction = -1;
  recalc_message_layout();
  if (repeating) {
    clear_active_image_request();
  }

  if (subreddit_detail_active() && has_selected_message() &&
      message_is_checklist(&s_messages[s_selected_message])) {
    if (checklist_edit_active()) {
      if (s_selected_checklist_item > 0) {
        s_selected_checklist_item--;
        ensure_selected_checklist_item_visible(!repeating);
      } else {
        s_selected_checklist_item = 0;
        ensure_selected_checklist_item_visible(!repeating);
      }
      return;
    }
    if (s_selected_checklist_item > -1) {
      s_selected_checklist_item--;
      ensure_selected_checklist_item_visible(!repeating);
      return;
    }
  }

  if (compose_target_is_selected() || s_selected_message < 0) {
    select_message_with_alignment(s_message_count - 1, true, !repeating);
    if (!reversed_direction) {
      maybe_prefetch_older_messages();
    }
    return;
  }

  GRect bounds = layer_get_bounds(s_messages_root);
  int margin = 6;
  int top = s_message_y[s_selected_message] - margin;
  int visible_top = clamp_scroll_offset(top);
  if (s_selected_message == 0 && s_chat_scroll_offset <= visible_top + 2) {
    if (s_loading_older_messages) {
      show_status("Loading older...");
    } else {
      request_older_messages(false);
    }
    return;
  }
  if (!repeating && s_message_h[s_selected_message] > bounds.size.h - (margin * 2) &&
      s_chat_scroll_offset > visible_top) {
    set_chat_scroll_offset(PG_MAX(visible_top, s_chat_scroll_offset - LONG_MESSAGE_SCROLL_DELTA), true);
    if (!reversed_direction) {
      maybe_prefetch_older_messages();
    }
    return;
  }
  if (s_selected_message > 0) {
    int prev_index = s_selected_message - 1;
    bool prev_is_tall = s_message_h[prev_index] > bounds.size.h - (margin * 2);
    select_message_with_alignment(prev_index, !prev_is_tall, !repeating);
    if (!reversed_direction) {
      maybe_prefetch_older_messages();
    }
  } else {
    if (s_loading_older_messages) {
      show_status("Loading older...");
    } else {
      request_older_messages(false);
    }
  }
}

static void main_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  bool repeating = click_is_repeating(recognizer);
  if (s_view_state == ViewStateChatList && s_chat_menu) {
    if (s_chats_loading || s_loading_messages || s_chat_view_pending) {
      show_status(s_loading_messages ? "Loading messages..." : "Loading...");
      return;
    }
    menu_layer_set_selected_next(s_chat_menu, false, MenuRowAlignCenter, !repeating);
    s_selected_chat = menu_layer_get_selected_index(s_chat_menu).row;
    if (chat_row_is_pin_divider(s_selected_chat)) {
      select_chat_row(nearest_selectable_chat_row(s_selected_chat, 1), !repeating);
      return;
    }
    if (subreddit_list_active() && s_selected_chat >= s_chat_count) {
      select_chat_row(s_chat_count - 1, !repeating);
      return;
    }
    if (s_selected_chat >= 0 && s_selected_chat < s_chat_count) {
      copy_cstr(s_chat_list_selected_id, sizeof(s_chat_list_selected_id), s_chats[s_selected_chat].id);
    }
    return;
  }
  if (s_view_state != ViewStateChat || !s_messages_root || s_message_count == 0) {
    return;
  }
  s_user_scrolled_messages = true;
  bool reversed_direction = s_message_scroll_direction == -1;
  s_message_scroll_direction = 1;
  recalc_message_layout();
  if (repeating) {
    clear_active_image_request();
  }

  if (subreddit_detail_active() && has_selected_message() &&
      message_is_checklist(&s_messages[s_selected_message])) {
    int items = checklist_item_count(s_messages[s_selected_message].text);
    if (checklist_edit_active()) {
      if (s_selected_checklist_item < items) {
        s_selected_checklist_item++;
        ensure_selected_checklist_item_visible(!repeating);
      } else {
        s_selected_checklist_item = items;
        ensure_selected_checklist_item_visible(!repeating);
      }
      return;
    }
    if (s_selected_checklist_item < items) {
      s_selected_checklist_item++;
      ensure_selected_checklist_item_visible(!repeating);
      return;
    }
  }

  if (compose_target_is_selected() || s_selected_message < 0) {
    if (s_at_newest) {
      scroll_to_bottom(!repeating);
    } else if (s_message_count > 0) {
      select_message_with_alignment(s_message_count - 1, false, !repeating);
      if (!reversed_direction) {
        maybe_prefetch_newer_messages();
      }
    }
    return;
  }

  GRect bounds = layer_get_bounds(s_messages_root);
  int margin = 6;
  int bottom = s_message_y[s_selected_message] + s_message_h[s_selected_message] + margin;
  if (!repeating && s_message_h[s_selected_message] > bounds.size.h - (margin * 2) &&
      s_chat_scroll_offset + bounds.size.h < bottom) {
    set_chat_scroll_offset(PG_MIN(bottom - bounds.size.h, s_chat_scroll_offset + LONG_MESSAGE_SCROLL_DELTA), true);
    if (!reversed_direction) {
      maybe_prefetch_newer_messages();
    }
    return;
  }
  if (s_selected_message < s_message_count - 1) {
    int next_index = s_selected_message + 1;
    bool next_is_tall = s_message_h[next_index] > bounds.size.h - (margin * 2);
    select_message_with_alignment(next_index, next_is_tall, !repeating);
    if (!reversed_direction) {
      maybe_prefetch_newer_messages();
    }
  } else if (s_loading_newer_messages) {
    show_status("Loading newer...");
  } else if (s_at_newest) {
    if (subreddit_detail_active()) {
      request_newer_messages(false);
    } else {
      scroll_to_bottom(!repeating);
    }
  } else {
    request_newer_messages(false);
  }
}

static void main_back_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_interaction_locked || s_action_window_pending_destroy) {
    return;
  }
  if (s_touch_keyboard_open) {
    lock_interactions(300);
    close_touch_keyboard();
    show_status(s_current_chat_title);
    return;
  }
  if (s_chats_loading || s_loading_messages || s_chat_view_pending ||
      s_subreddit_intent_timer || s_subreddit_command_timer ||
      s_dictation_start_timer || s_date_send_timer) {
    show_status(s_loading_messages ? "Loading messages..." : "One moment...");
    return;
  }
  if (s_view_state == ViewStateChat && subreddit_detail_active() && s_checklist_edit_mode) {
    lock_interactions(300);
    bump_ui_generation();
    s_checklist_edit_mode = false;
    s_selected_checklist_item = -1;
    show_status(s_current_chat_title);
    render_messages();
    return;
  }
  if (s_view_state == ViewStateChat) {
    lock_interactions(700);
    bool was_subreddit_detail = subreddit_detail_active();
    if (was_subreddit_detail) {
      cancel_deferred_subreddit_ui();
    }
    cancel_message_timeout();
    cancel_message_retry();
    s_loading_messages = false;
    s_loading_older_messages = false;
    s_loading_newer_messages = false;
    s_message_transfer_id = 0;
    s_chat_view_pending = false;
    clear_message_stage();
    close_touch_keyboard();
    send_command_with_status("leave_chat", s_current_chat_id, NULL, NULL, NULL, false);
    s_reddit_detail_request = false;
    render_chat_list_with_transition();
  } else if (s_view_state == ViewStateChatList && s_list_mode == ListModeTasks) {
    lock_interactions(700);
    request_sorts(s_current_plan_id, s_current_plan_title);
  } else if (s_view_state == ViewStateChatList && s_list_mode == ListModeSorts) {
    lock_interactions(700);
    request_subreddits();
  } else if (s_view_state == ViewStateChatList && s_list_mode == ListModeSubreddits) {
    lock_interactions(700);
	    request_chats();
  } else if (s_view_state == ViewStateChatList && s_list_mode == ListModeGroupChats) {
    lock_interactions(700);
    request_chats();
  } else {
    lock_interactions(300);
    window_stack_pop(true);
  }
}

#if TOUCH_KEYBOARD_AVAILABLE
static void touch_handler(const TouchEvent *event, void *context) {
  if (!TOUCH_KEYBOARD_ENABLED || !event || event->type != TouchEvent_Liftoff ||
      s_view_state != ViewStateChat || !s_messages_root) {
    return;
  }

  GRect bounds = layer_get_bounds(s_messages_root);
  GRect frame = layer_get_frame(s_messages_root);
  GPoint point = GPoint(event->x - frame.origin.x, event->y - frame.origin.y);
  if (!grect_contains_point(&bounds, &point)) {
    return;
  }
  if (s_touch_keyboard_open) {
    GRect keyboard_rect = touch_keyboard_rect_for_bounds(bounds);
    char action;
    char ch = touch_keyboard_char_at(keyboard_rect, point, &action);
    handle_touch_keyboard_key(ch, action);
    return;
  }

  if (s_at_newest) {
    GRect compose_rect = compose_rect_for_bounds(bounds);
    if (grect_contains_point(&compose_rect, &point)) {
      open_touch_keyboard();
    }
  }
}
#endif

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, main_select_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, main_select_long_click_handler, NULL);
  window_single_repeating_click_subscribe(BUTTON_ID_UP, REPEAT_SCROLL_MS, main_up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, REPEAT_SCROLL_MS, main_down_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, main_back_click_handler);
}

static void main_window_load(Window *window) {
  window_set_background_color(window, CHAT_BG);
  window_set_click_config_provider(window, click_config_provider);
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  GRect status_rect = ROUND_UI ? GRect(24, chat_status_y(), bounds.size.w - 48, STATUS_H) :
                                 GRect(0, 0, bounds.size.w, STATUS_H);
  s_status_layer = text_layer_create(status_rect);
  if (!s_status_layer) {
    return;
  }
  text_layer_set_text(s_status_layer, "Pebbit");
  text_layer_set_font(s_status_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_status_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_status_layer, GColorWhite);
  text_layer_set_background_color(s_status_layer, APP_COLOR);
  layer_add_child(window_layer, text_layer_get_layer(s_status_layer));

  int content_y = chat_content_y();
  int bottom_pad = chat_bottom_pad();
  s_chat_menu = menu_layer_create(GRect(0, content_y, bounds.size.w, bounds.size.h - content_y - bottom_pad));
  if (!s_chat_menu) {
    return;
  }
  menu_layer_set_callbacks(s_chat_menu, NULL, (MenuLayerCallbacks) {
    .get_num_sections = chat_menu_get_num_sections_callback,
    .get_num_rows = chat_menu_get_num_rows_callback,
    .get_header_height = chat_menu_get_header_height_callback,
    .draw_row = chat_menu_draw_row_callback,
    .get_cell_height = chat_menu_get_cell_height_callback,
    .select_click = chat_menu_select_callback
  });
  if (ROUND_UI) {
    menu_layer_set_center_focused(s_chat_menu, false);
  }
  layer_add_child(window_layer, menu_layer_get_layer(s_chat_menu));
}

static void main_window_unload(Window *window) {
  light_enable(false);
  cancel_status_clear();
  destroy_chat_view();
  clear_chat_rows();
  destroy_message_images();
  if (s_chat_menu) {
    menu_layer_destroy(s_chat_menu);
    s_chat_menu = NULL;
  }
  if (s_status_layer) {
    text_layer_destroy(s_status_layer);
    s_status_layer = NULL;
  }
}

static void main_window_appear(Window *window) {
  light_enable(false);
}

static void init(void) {
  s_view_state = ViewStateChatList;
  s_selected_message = -1;
  s_chats_loading = true;
  s_heap_state = calloc(1, sizeof(PebbitHeapState));
  s_messages = calloc(MAX_MESSAGES, sizeof(Message));
  s_message_y = calloc(MAX_MESSAGES, sizeof(int));
  s_message_h = calloc(MAX_MESSAGES, sizeof(int));
  if (!s_heap_state || !s_messages || !s_message_y || !s_message_h) {
    s_chats_loading = false;
    return;
  }
  for (int i = 0; i < MAX_MESSAGES; i++) {
    init_message_strings(&s_messages[i]);
  }
  s_chats = calloc(MAX_CHATS, sizeof(Chat));
  light_enable(false);
  if (s_chats) {
    for (int i = 0; i < MAX_CHATS; i++) {
      init_chat_strings(&s_chats[i]);
    }
  }

  s_main_window = window_create();
  if (!s_main_window) {
    return;
  }
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .appear = main_window_appear,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);

  if (!s_chats) {
    s_chats_loading = false;
    show_status("Memory low");
    return;
  }

  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  AppMessageResult bridge_result = app_message_open(APP_INBOX_SIZE, APP_OUTBOX_SIZE);
  if (bridge_result != APP_MSG_OK) {
    s_chats_loading = false;
    show_status("Bridge memory low");
    return;
  }
#if TOUCH_KEYBOARD_AVAILABLE
  if (TOUCH_KEYBOARD_ENABLED) {
    touch_service_subscribe(touch_handler, NULL);
  }
#endif
  s_startup_wake_timer = app_timer_register(PHONE_WAKE_DELAY_MS, startup_wake_timer_callback, NULL);
}

static void deinit(void) {
  light_enable(false);
  if (s_startup_wake_timer) {
    app_timer_cancel(s_startup_wake_timer);
    s_startup_wake_timer = NULL;
  }
  if (s_chat_retry_timer) {
    app_timer_cancel(s_chat_retry_timer);
    s_chat_retry_timer = NULL;
  }
  if (s_action_window_destroy_timer) {
    app_timer_cancel(s_action_window_destroy_timer);
    s_action_window_destroy_timer = NULL;
  }
  if (s_interaction_unlock_timer) {
    app_timer_cancel(s_interaction_unlock_timer);
    s_interaction_unlock_timer = NULL;
  }
  if (s_dictation_start_timer) {
    app_timer_cancel(s_dictation_start_timer);
    s_dictation_start_timer = NULL;
  }
  if (s_dictation_confirm_timer) {
    app_timer_cancel(s_dictation_confirm_timer);
    s_dictation_confirm_timer = NULL;
  }
  if (s_dictation_failure_timer) {
    app_timer_cancel(s_dictation_failure_timer);
    s_dictation_failure_timer = NULL;
  }
  s_dictation_in_progress = false;
  if (s_date_send_timer) {
    app_timer_cancel(s_date_send_timer);
    s_date_send_timer = NULL;
  }
  if (s_subreddit_command_timer) {
    app_timer_cancel(s_subreddit_command_timer);
    s_subreddit_command_timer = NULL;
  }
  memset(&s_subreddit_wire_command, 0, sizeof(s_subreddit_wire_command));
  clear_subreddit_intent();
  clear_chat_rows();
  destroy_message_images();
  clear_message_rows();
  if (s_messages) {
    for (int i = 0; i < MAX_MESSAGES; i++) {
      release_message_strings(&s_messages[i]);
    }
    free(s_messages);
    s_messages = NULL;
  }
  if (s_message_y) {
    free(s_message_y);
    s_message_y = NULL;
  }
  if (s_message_h) {
    free(s_message_h);
    s_message_h = NULL;
  }
  if (s_chats) {
    free(s_chats);
    s_chats = NULL;
  }
  if (s_dictation_session) {
    dictation_session_destroy(s_dictation_session);
    s_dictation_session = NULL;
  }
  if (s_action_window) {
    window_destroy(s_action_window);
    s_action_window = NULL;
  }
  if (s_action_window_pending_destroy) {
    window_destroy(s_action_window_pending_destroy);
    s_action_window_pending_destroy = NULL;
  }
  if (s_bucket_options) {
    free(s_bucket_options);
    s_bucket_options = NULL;
  }
  if (s_assign_team_options) {
    free(s_assign_team_options);
    s_assign_team_options = NULL;
  }
  if (s_assign_member_options) {
    free(s_assign_member_options);
    s_assign_member_options = NULL;
  }
  if (s_subreddit_labels_packed) {
    free(s_subreddit_labels_packed);
    s_subreddit_labels_packed = NULL;
  }
  free_image_transfer_buffer();
  free_avatar_transfer_buffer();
  free_full_text_body();
#if TOUCH_KEYBOARD_AVAILABLE
  if (TOUCH_KEYBOARD_ENABLED) {
    touch_service_unsubscribe();
  }
#endif
  window_destroy(s_main_window);
  if (s_heap_state) {
    free(s_heap_state);
    s_heap_state = NULL;
  }
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
