#include "all.h"
/* copy.c 331a */
/* private declarations for copying collection 331b */
static Value *fromspace, *tospace;    /* used only at GC time */
static int semispacesize = 12;/* # of objects in fromspace and tospace */

static int collectionCount = 0;
static int totalMemoryUsed = 0;
static int liveObjects  = 0;

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
static void copy(void);
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
    if (hp == heaplimit) {
        collect();
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
            *hp = *p;  /* this is a copy */
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
        collectionCount++;
    // If we have no hp, allocate everything
    if(hp == NULL) { 
        // Allocate the fromspace, tell GC we did
        fromspace = (Value*) malloc(semispacesize * sizeof(Value));
        gc_debug_post_acquire(fromspace, semispacesize);

        // Allocate the tospace, tell GC we did
        tospace = (Value*) malloc(semispacesize * sizeof(Value));
        gc_debug_post_acquire(tospace, semispacesize);

        // Set hp and heaplimit to fromspace 
        heaplimit = hp = fromspace;  
        heaplimit += semispacesize;  // increase the limit

        return; // We just allocated so we can't do anything else
    }
  
    // Copy everything over
    copy();

    // If hp is at the limit we need to allocate more space
    // We increment semispace size after copy() because copy uses the old space
    // size when performing `gc_debug_post_reclaim`
    if (hp == heaplimit){
        // Free the old tospace
        gc_debug_pre_release(tospace, semispacesize);
        free(tospace);

        // Allocate the new tospace
        tospace = (Value*) malloc(semispacesize * sizeof(Value) * 2);
        gc_debug_post_acquire(tospace, semispacesize * 2);

        // Copy over to the new tospace. This flips tospace and fromspace
        copy();

        // tospace is now the smaller space

        // Free tospace 
        gc_debug_pre_release(tospace, semispacesize);
        free(tospace);

        // Increment semispace size and set heaplimit
        semispacesize *= 2;
        heaplimit = fromspace + semispacesize;

        // Allocate new tospace
        tospace = (Value*) malloc(semispacesize * sizeof(Value));
        gc_debug_post_acquire(tospace, semispacesize);

    } 
    
    if(collectionCount % 10 == 0){
        printf("Total cells allocated: %d Heapsize: %d", liveObjects, (semispace*2));
        }
}

/* This function copies all the items from "fromspace" to "tospace" */
/* almost everything can be found on page 332 */
void copy(void){
    (void)scanframe; (void)scantests; // this was here but idk what it does

    Value *scanp = hp = tospace; 

    // Scan the environment
    scanenv(*roots.globals.user);

    // Scan frames and forward the pointers (c)
    Frame *fr = roots.stack->frames;
    for(; fr < roots.stack->sp; ++fr){
        scanframe(fr);
    }

    // Scan list of unit tests and forward the pointers (d)
    UnitTestlistlist tests = roots.globals.internal.pending_tests;
    for (; tests; tests=tests->tl) {
        scantests(tests->hd);
    }

    // Scan registers and forward the pointers (e)
    Registerlist regs = roots.registers;
    for(; regs != NULL; regs = regs->tl) {
        scanloc(regs->hd);
    }

    liveObjects = 0;
    // Scan the objects we've forwarded and forward pointers (f) 
    for(; scanp < hp; ++scanp){
        scanloc(scanp);  // Catch up pointer, it's mostly in the book
        liveObjects++;
        totalMemoryUsed++;
    }

    // Swap at the end
    Value *tmp = fromspace;
    heaplimit = fromspace = tospace;
    heaplimit += semispacesize;
    tospace = tmp; 
    
    printf("Heapsize: %d Live Objects: %d Live Objects/Heapsize: %f", semispace*2, liveObjects, (liveObjects/(semispace*2))
    // Mark all objects in tospace as invalid (page 337 b)
    gc_debug_post_reclaim_block(tospace, semispacesize);
}
void printfinalstats(void) {   
    printf("Heapsize: %d Live Objects: %d Live Objects/Heapsize: %f Total Collections: %d Total Objects Copied: %d ", semispace*2, liveObjects, (liveObjects/(semispace*2)), collectionCount, totalMemoryUsed); 
    }
/* you need to initialize this variable */
int gc_uses_mark_bits = 0;
