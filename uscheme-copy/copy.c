#include "all.h"
/* copy.c 331a */
/* private declarations for copying collection 331b */
static Value *fromspace, *tospace;    /* used only at GC time */
static int semispacesize;
                                     /* # of objects in fromspace and tospace */
/* private declarations for copying collection 331c */
static Value *hp, *heaplimit;                /* used for every allocation */
/* private declarations for copying collection 332b */
static void scanenv      (Env env);
static void scanexp      (Exp exp);
static void scanexplist  (Explist es);
static void scanframe    (Frame *fr);
static void scantest     (UnitTest t);
static void scantests    (UnitTestlist ts);
static void scanloc      (Value *vp);
/* private declarations for copying collection 334a */
#define isinspace(LOC, SPACE) ((SPACE) <= (LOC) && (LOC) < (SPACE) +\
                                                                  semispacesize)
static Value *forward(Value *p);
/* private declarations for copying collection 1094e */
static void collect(void);
/* copy.c 331d */
/* representation of [[struct Stack]] 1069a */
struct Stack {
    int size;
    Frame *frames;  // memory for 'size' frames
    Frame *sp;      // points to first unused frame
};
/* copy.c 332a */
int nalloc;   /* OMIT */
Value* allocloc(void) {
    if (hp == heaplimit){
        collect();
        if(hp == heaplimit){
          //increase size using malloc & free
          //malloc value * (semispacesize+2)
        }
      }
    assert(hp < heaplimit);
    assert(isinspace(hp, fromspace)); /*runs after spaces are swapped*/ /*OMIT*/
    nalloc++;   /* OMIT */
    /* tell the debugging interface that [[hp]] is about to be allocated 338c */
    gc_debug_pre_allocate(hp);
    return hp++;
}
/* copy.c 332g */
static void scanenv(Env env) {
    for (; env; env = env->tl)
      { /*OMIT*/
        env->loc = forward(env->loc);
        assert(isinspace(env->loc, tospace)); /*OMIT*/
      } /*OMIT*/
}
/* copy.c 333a */
static void scanloc(Value *vp) {
    switch (vp->alt) {
    case NIL:
    case BOOLV:
    case NUM:
    case SYM:
        return;
    case PAIR:
        vp->u.pair.car = forward(vp->u.pair.car);
        vp->u.pair.cdr = forward(vp->u.pair.cdr);
        return;
    case CLOSURE:
        scanexp(vp->u.closure.lambda.body);
        scanenv(vp->u.closure.env);
        return;
    case PRIMITIVE:
        return;
    default:
        assert(0);
        return;
    }
}
/* copy.c 333b */
static Value* forward(Value *p) {
    if (isinspace(p, tospace)) {
                          /* already in to space; must belong to scanned root */
        return p;
    } else {
        assert(isinspace(p, fromspace));
        /* forward pointer [[p]] and return the result 327b */
        if (p->alt == FORWARD) {            /* forwarding pointer */
            assert(isinspace(p->u.forward, tospace));   /* OMIT */
            return p->u.forward;
        } else {
            assert(isinspace(hp, tospace)); /* there is room */   /* OMIT */

    /* tell the debugging interface that [[hp]] is about to be allocated 338c */
            gc_debug_pre_allocate(hp);
            *hp = *p;
            *p  = mkForward(hp);
                                /* overwrite *p with a new forwarding pointer */
            assert(isinspace(p->u.forward, tospace)); /*extra*/   /* OMIT */
            return hp++;
        }
    }
    return NULL; /* appease a stupid compiler */  /*OMIT*/
}
/* copy.c 1084c */
static void scanexp(Exp e) {
    switch (e->alt) {
    /* cases for [[scanexp]] 1085a */
    case LITERAL:
        scanloc(&e->u.literal);
        return;
    case VAR:
        return;
    case IFX:
        scanexp(e->u.ifx.cond);
        scanexp(e->u.ifx.truex);
        scanexp(e->u.ifx.falsex);
        return;
    case WHILEX:
        scanexp(e->u.whilex.cond);
        scanexp(e->u.whilex.body);
        return;
    case BEGIN:
        scanexplist(e->u.begin);
        return;
    case SET:
        scanexp(e->u.set.exp);
        return;
    case LETX:
        scanexplist(e->u.letx.es);
        scanexp(e->u.letx.body);
        return;
    case LAMBDAX:
        scanexp(e->u.lambdax.body);
        return;
    case APPLY:
        scanexp(e->u.apply.fn);
        scanexplist(e->u.apply.actuals);
        return;
    /* cases for [[scanexp]] 1085b */
    case BREAKX:
        return;
    case CONTINUEX:
        return;
    case RETURNX:
        scanexp(e->u.returnx);
        return;
    case THROW:
        scanexp(e->u.throw);
        return;
    case TRY_CATCH:
        scanexp(e->u.try_catch.handler);
        scanexp(e->u.try_catch.body);
        return;
    /* cases for [[scanexp]] 1086a */
    case HOLE:
    case WHILE_RUNNING_BODY:
        return;
    case LETXENV:
        scanenv(e->u.letxenv);
        return;
    case CALLENV:
        scanenv(e->u.callenv);
        return;
    }
    assert(0);
}
/* copy.c 1086b */
static void scanframe(Frame *fr) {
    scanexp(&fr->context);
        if (fr->syntax != NULL)
            scanexp(fr->syntax);
}
/* copy.c 1086c */
static void scanexplist(Explist es) {
    for (; es; es = es->tl)
        scanexp(es->hd);
}
/* copy.c 1086d */
static void scantests(UnitTestlist tests) {
    for (; tests; tests = tests->tl)
        scantest(tests->hd);
}
/* copy.c 1086e */
static void scantest(UnitTest t) {
    switch (t->alt) {
    case CHECK_EXPECT:
        scanexp(t->u.check_expect.check);
        scanexp(t->u.check_expect.expect);
        return;
    case CHECK_ASSERT:
        scanexp(t->u.check_assert);
        return;
    case CHECK_ERROR:
        scanexp(t->u.check_error);
        return;
    }
    assert(0);
}
/* copy.c ((prototype)) 1095a */
/* you need to redefine these functions */
static void collect(void) {
  if(hp == NULL){
    semispacesize = 50;//value;
    *hp = malloc(semispacesize*sizeof(Value));
    *heaplimit = hp+ (semispacesize/2);
    }
  Value fp = hp;
  Value *scanp = hp = tospace;
  scanenv(*roots.global.user);
  for(Frame *fr = roots.stack->frames; fr<roots.stack->sp; fr++){
    forawrd(*fr);
  }
  
  for(UnitTestlistlist tests = roots.globals.internal.pending_tests; tests; tests=tests->tl){
    
  }
  for(; scanp < hp; scanp++){
    scanloc(scanp) //Catch up pointer, it's mostly in the book
    
  }
  

  //Swap
  (void)scanframe; (void)scantests; assert(0); }
void printfinalstats(void) { assert(0); }
/* you need to initialize this variable */
int gc_uses_mark_bits;
