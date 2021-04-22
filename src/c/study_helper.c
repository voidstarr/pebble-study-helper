#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static const char* lock_text = "500 Internal Server Error\n{'error': '500', 'message': 'Internal Server Error', 'long_message': 'server encountered unexpected return values'}";

static int idx = 0;
static int state = 0;
static int sstate = 0;
static int dnstate = 0;
static bool lock_on = true;

// every line is 23 characters
// there are 12 lines on the display
// 253 characters per page since first line is index

static const int content_length = 10;
static char* content[10] = {
  "contrapositive\n~q->~p\nconverse\nq->p\ninverse\n~p->~q\nnegate implication\n~(p->q)=(p^~q)\nsufficient\np->q\nnecessary\n~p->~q\n",
  "Modus Ponens\np->q\np\ntf q\nModus Tollens\np->q\n~q\ntf ~p\nMake truth table with\nhypothesis and concl\nrows to test argument\n",
  "generalization\np        q\ntf pVq   tf pVq\nspecialization\np^q      p^q\ntf p     tf q\nelimination\npVq\n~q\ntf p\n",
  "~(fAx in D, Q(x))\n=tEx in D s/t ~Q(x)\n~(fAx,P(x)->Q(x))\n=tEx, s/t P(x)^~Q(x)\nonly if\nfAx, if ~s(x)->~r(x)\nrational\ntEa,b s/t r=a/b,b!=0\ndivisible\nd|n <=> tEk in Z s/t\n              n = dk\n\n",
  "A sub B <> fAx x in A\n          -> x not in B\nx in XUY <> x in X or Y\nx in X-Y <> x in X and \n            x not in Y\nequivalence relation\nreflx xRx\nsymme xRy, yRx\ntrans xRy, yRz -> xRz\nequiv class [a]<>xRa\n\n",
  "m=n(mod d)<>d|(m-n)\npigeonhole\ndefine f:A->B\nshow f not 1-1\ntf \n\n\n\n\n\n\n",
  "Euclidian gcd(888,54)=6\n888=54(16)+24\n|---^ |-----^\nv     v\n54=  24(2)+6\n|----^ |---^\nv      v\n24=    6(4)+0\n       ^--gcd\n\n         \n",
  "ordered\nr-perm P(n,r)=n!/(n-r)!\ncomb n^k\n\nunordered\nr-comb nCr=P(n,r)/r!\nno rep (r+n-1)Cr\nBayes' Thm\n          P(A|Bk)P(Bk)\nP(Bk|A)=---------------\n        P(A|Bk)P(Bk)+..\n",
  "Isomorphic\nsame n vert, edge. has\nm vert of deg n,\ncircuit of len k, conne\nEuler Circuit\nonly if every vert is\neven degree\n\n\n\n \n",
  "Hamiltonian Circuit\ncreate subgraph H remov\nedges and make H discon\nremove vert, vert deg!=\n2\n\n\n\n\n\n\n\n"
};

static void log_state() {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "sstate: %d, dnstate: %d, lock_state: %d, idx: %d, content_length: %d"
  //          , sstate, dnstate, state, idx, content_length);
}

static void reset_state_lock(){
  //log_state();
  if(sstate != 0 || dnstate != 0){
    sstate = 0;
    dnstate = 0;
  }
}

static void swap_text(){
  log_state();
  if(state == 0)
    return;
  text_layer_set_text(text_layer, content[idx]);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  reset_state_lock();
  if(state == 0)
    return;
  idx = 0;
  swap_text();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  reset_state_lock();
  if(state == 0)
    return;
  idx++;
  if (idx > content_length-1)
    idx=0;
  //text_layer_set_text(text_layer, content[idx]);
  swap_text();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  reset_state_lock();
  if(state == 0)
    return;
  if(idx == 0){
    idx=(content_length-1);
  }else{
    idx--;
  }
  //text_layer_set_text(text_layer, content[idx]);
  swap_text();
}

static void long_select_down_handler(ClickRecognizerRef recognizer, void *context) {
  sstate++;
  log_state();
}

static void long_down_down_handler(ClickRecognizerRef recognizer, void *context) {
  if(sstate > 2)
    dnstate++;
  if(sstate > 2 && dnstate > 0){
    state++;
    text_layer_set_text_alignment(text_layer, GTextAlignmentLeft);
  }
  log_state();
  swap_text();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, long_select_down_handler, NULL);
  window_long_click_subscribe(BUTTON_ID_DOWN, 500, long_down_down_handler, NULL);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  
  if (lock_on) {
    //TODO: reset this
    text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
    text_layer_set_text(text_layer, lock_text);
  } else {
    state = 1;
    text_layer_set_text_alignment(text_layer, GTextAlignmentLeft);
    swap_text();
  }
  
  text_layer_set_overflow_mode(text_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void init(void) {
  if(persist_exists(4092)) {
    idx = persist_read_int(4092);
  }

  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  persist_write_int(4093, idx);
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
