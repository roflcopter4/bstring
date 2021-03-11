#ifndef TOP_BSTRING_H
#  error Never include this file manually. Include "bstring.h".
#endif
#ifndef BSTRLIB_ADDITIONS_H
#define BSTRLIB_ADDITIONS_H

#include "bstring.h"

#ifdef BSTR_USE_P99
#  include "p99/p99_id.h"
#endif

/*======================================================================================*/
/* MY ADDITIONS */

#ifndef BSTR_PUBLIC
#  define BSTR_PUBLIC
# endif
#ifndef INLINE
#  define INLINE extern __inline
#endif

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Simple macro which returns the data of a bstring cast to char *
 */
#if 0 && __STDC_VERSION__ >= 201112LL
#  if defined __GNUC__ && 0
#    define BS(BSTR)                                                         \
        __extension__({                                                  \
                _Pragma("GCC diagnostic push");                          \
                _Pragma("GCC diagnostic ignored \"-Waddress\"");         \
                __auto_type mbstr_ = (BSTR);                             \
                char *mbstr_cstr_ = (mbstr_)                             \
                        ? _Generic((mbstr_),                             \
                            const bstring *: ((char *)((mbstr_)->data)), \
                                  bstring *: ((char *)((mbstr_)->data))) \
                        : "(null)";                                      \
                _Pragma("GCC diagnostic pop");                           \
                mbstr_cstr_;                                             \
        })
#  else
#    define BS(BSTR)                                                     \
        ((BSTR) ? _Generic((BSTR),                                     \
                            const bstring *: ((char *)((BSTR)->data)), \
                                  bstring *: ((char *)((BSTR)->data))) \
                : "(null)")
#  endif

#  define BTS(BSTR) _Generic((BSTR), bstring: ((char *)((BSTR).data)))
#else
#  define BS(BSTR)  ((BSTR) ? (char *)((BSTR)->data) : "(null)")
#  define BTS(BSTR) ((char *)((BSTR).data))
#endif


/**
 * Initialize a bstring without any casting. seful when a constant expression is
 * required, such as in global or static variable initializers. The argument
 * MUST be a literal string, so double evaluation shouldn't be a problem..
 */
#define BT bt_init
#define bt_init(CSTR)                             \
        {                                         \
                .slen  = (sizeof(CSTR) - 1),      \
                .mlen  = 0,                       \
                .data  = (uchar *)("" CSTR ""),   \
                .flags = 0x00U                    \
        }
#define bt_fromlit bt_init

/**
 * This useful macro creates a valid pointer to a static bstring object by
 * casting bt_init() to (bstring[]). Used most often to supply literal string
 * arguments to functions that expect a bstring pointer. Like bt_init, the
 * argument must be a literal string.
 *
 * All bstring functions will refuse to modify the return from this macro,
 * including b_free(). The object must not otheriwise be free'd.
 */
#define b_tmp(CSTR) ((bstring[]){bt_init(CSTR)}) 
#define B(CSTR)     ((bstring[]){bt_init(CSTR)}) 

/**
 * Creates a static bstring reference to existing memory without copying it.
 * Unlike the return from b_tmp, this will accept non-literal strings, and the
 * data is modifyable by default. However, b_free will refuse to free the data,
 * and the object itself is stack memory and therefore also not freeable.
 */
/**
 * Return a static bstring derived from a cstring. Identical to bt_fromblk
 * except that the length field is derived through a call to strlen(). Beware
 * that this macro evaluates its argument twice!
 */

#define bt_fromblk(BLK, LEN) \
        ((bstring){ .data = ((uchar *)(BLK)), .slen = (LEN), .mlen = 0, .flags = 0 })

#define bt_fromcstr(CSTR) \
        ((bstring){ .data = ((uchar *)(CSTR)), .slen = strlen(CSTR), .mlen = 0, .flags = 0 })

#define bt_fromarray(CSTR) \
        ((bstring){ .data = (uchar *)(CSTR), .slen = (sizeof(CSTR) - 1), .mlen = 0, .flags = 0 })


#define btp_fromblk(BLK, LEN) \
        ((bstring[]){{ .data = ((uchar *)(BLK)), .slen = (LEN), .mlen = 0, .flags = 0}})

#define btp_fromcstr(STR_) \
        ((bstring[]){{ .data = ((uchar *)(STR_)), .slen = strlen(STR_), .mlen = 0, .flags = 0}})

#define btp_fromarray(CARRAY_) \
        ((bstring[]){{ .data  = ((uchar *)(CARRAY_)), .slen  = (sizeof(CARRAY_) - 1), .mlen  = 0, .flags = 0}})


#define b_litsiz                   b_staticBlkParms
#define b_lit2bstr(LIT_STR)        b_fromblk(b_staticBlkParms(LIT_STR))
#define b_assignlit(BSTR, LIT_STR) b_assign_blk((BSTR), b_staticBlkParms(LIT_STR))
#define b_catlit(BSTR, LIT_STR)    b_catblk((BSTR), b_staticBlkParms(LIT_STR))
#define b_fromlit(LIT_STR)         b_lit2bstr(LIT_STR)
#define b_iseq_lit(BSTR, LIT_STR)  b_iseq((BSTR), B(LIT_STR))

#if __STDC_VERSION__ >= 201112LL
#  define B_ISEQ(a, b) _Generic(b,                                               \
                bstring *          : b_iseq     (((void *)(a)), ((void *) (b))), \
                const bstring *    : b_iseq     (((void *)(a)), ((void *) (b))), \
                volatile bstring * : b_iseq     (((void *)(a)), ((void *) (b))), \
                char *             : b_iseq_cstr(((void *)(a)), ((void *) (b))), \
                const char *       : b_iseq     (((void *)(a)), ((void *)B(b))), \
                volatile char *    : b_iseq_cstr(((void *)(a)), ((void *) (b))))
#endif


/**
 * Allocates a reference to the data of an existing bstring without copying the
 * data. The bstring itself must be free'd, however b_free will not free the
 * data. The clone is write protected, but the original is not, so caution must
 * be taken when modifying it lest the clone's length fields become invalid.
 *
 * This is rarely useful.
 */
BSTR_PUBLIC bstring *b_clone(const bstring *src);

/**
 * Similar to b_clone() except the original bstring is designated as the clone
 * and is write protected. Useful in situations where some routine will destroy
 * the original, but you want to keep its data.
 */
BSTR_PUBLIC bstring *b_clone_swap(bstring *src);

/**
 * Allocates a bstring object that references the supplied memory. The memory is
 * not copied, so the user must ensure that it will not go out of scope. The
 * bstring object itself must be freed, but the memory it references will not be
 * freed by b_destroy or b_free by default. The user must either manually set
 * the BSTR_DATA_FREEABLE flag or ensure that the memory is freed independently.
 */
BSTR_PUBLIC bstring *b_refblk(void *blk, unsigned len);
BSTR_PUBLIC bstring *b_steal (void *blk, unsigned len);

/**
 * The same as b_refblk with the exception that the size is derived by strlen().
 * Required to be an inline function to avoid evaluating the arguments twice via
 * the necessary strlen() call.
 */
INLINE bstring *b_refcstr(char *str)
{
        return b_refblk(str, strlen(str));
}


/*--------------------------------------------------------------------------------------*/
/* Read wrappers */

/**
 * Simple wrapper for fgetc that casts the void * paramater to a FILE * object
 * to avoid compiler warnings.
 */
INLINE int
b_fgetc(void *param)
{
        return fgetc((FILE *)param);
}

/**
 * Simple wrapper for fread that casts the void * paramater to a FILE * object
 * to avoid compiler warnings.
 */
INLINE size_t
b_fread(void *buf, const size_t size, const size_t nelem, void *param)
{
        return fread(buf, size, nelem, (FILE *)param);
}

#define B_GETS(PARAM, TERM, END_) b_gets(&b_fgetc, (PARAM), (TERM), (END_))
#define B_READ(PARAM, END_)       b_fread(&b_fread, (PARAM), (END_))

__attribute__((__format__(__printf__, 1, 2)))
BSTR_PUBLIC bstring *b_quickread(const char *__restrict fmt, ...);
BSTR_PUBLIC bstring *b_read_fd(const int fd);
BSTR_PUBLIC bstring *b_read_stdin(void);


/*--------------------------------------------------------------------------------------*/
/* Some additional list operations. */

/**
 * Signifies the end of a list of bstring varargs.
 */
#define BSTR_ARGLIST_VIGIL \
        ((bstring[]){{.data = NULL, .slen = 0, .mlen = 0, .flags = BSTR_LIST_END}})


/**
 * Concatenate a series of bstrings.
 */
BSTR_PUBLIC bstring *_b_concat_all(const bstring *join, int join_end, ...);
BSTR_PUBLIC int      _b_append_all(bstring *dest, const bstring *join, int join_end, ...);
#define b_concat_all(...) \
        _b_concat_all(NULL, 0, __VA_ARGS__, BSTR_ARGLIST_VIGIL)
#define b_append_all(BDEST, ...) \
        _b_append_all((BDEST), NULL, 0, __VA_ARGS__, BSTR_ARGLIST_VIGIL)
#define b_join_all(JOIN, END, ...) \
        _b_concat_all((JOIN), (END), __VA_ARGS__, BSTR_ARGLIST_VIGIL)
#define b_join_append_all(BDEST, JOIN, END, ...) \
        _b_append_all((BDEST), (JOIN), (END), __VA_ARGS__, BSTR_ARGLIST_VIGIL)

/*--------------------------------------------------------------------------------------*/

/**
 * Safely free several bstrings.
 */
BSTR_PUBLIC void _b_free_all(bstring **bstr, ...);

/**
 * Write bstrings to files/stdout/stderr without the calls to strlen that the
 * standard c library would make. These call fwrite on the supplied stream,
 * passing the slen of the bstring as the length.
 */
BSTR_PUBLIC void _b_fwrite(FILE *fp, bstring *bstr, ...);

/**
 * Same as _b_fputs but writes to a file descriptor using the write(2) function
 * rather than a FILE * object and fwrite(3);
 */
BSTR_PUBLIC int  _b_write(int fd, bstring *bstr, ...);
BSTR_PUBLIC void _b_list_dump(FILE *fp, const b_list *list, const char *listname);
BSTR_PUBLIC void _b_list_dump_fd(int fd, const b_list *list, const char *listname);

#define b_free_all(...)         _b_free_all(__VA_ARGS__, BSTR_ARGLIST_VIGIL)
#define b_puts(...)             _b_fputs(stdout, __VA_ARGS__, BSTR_ARGLIST_VIGIL)
#define b_warn(...)             _b_fputs(stderr, __VA_ARGS__, BSTR_ARGLIST_VIGIL)
#define b_fwrite(FP, ...)       _b_fwrite(FP, __VA_ARGS__,   BSTR_ARGLIST_VIGIL)
#define b_write(FD, ...)        _b_write(FD, __VA_ARGS__,   BSTR_ARGLIST_VIGIL)
#define b_list_dump(FP, LST)    _b_list_dump((FP), (LST), #LST)
#define b_list_dump_fd(FD, LST) _b_list_dump_fd((FD), (LST), #LST)

#ifdef BSTR_USE_P99
#  define B_LIST_FOREACH(LIST, VAR, ...)                                                 \
        B_LIST_FOREACH_EXPLICIT_(LIST, VAR,                                              \
                                  P99_IF_EMPTY(__VA_ARGS__)(P99_UNIQ(cnt))(__VA_ARGS__), \
                                  P99_UNIQ(blist_b))
#  define B_LIST_FOREACH_EXPLICIT_(BLIST, VAR, CTR, BL)         \
        for (unsigned CTR, (BL) = true;                         \
             (BLIST) != NULL && (BL);                           \
             (BL) = false)                                      \
                for (bstring *VAR = ((BLIST)->lst[((CTR) = 0)]);                   \
                     (CTR) < (BLIST)->qty && (((VAR) = (BLIST)->lst[(CTR)]) || 1); \
                     ++(CTR))
#else
#if 0
#  define B_LIST_FOREACH(BLIST, VAR, CTR)                                  \
        for (bstring *VAR = ((BLIST)->lst[((CTR) = 0)]);                   \
             (CTR) < (BLIST)->qty && (((VAR) = (BLIST)->lst[(CTR)]) || 1); \
             ++(CTR))
#endif
#  define B_LIST_FOREACH(BLIST, VAR, CTR)                                          \
        for (unsigned CTR, __LINE__##VAR##_##CTR##_b = true;                       \
             (BLIST) != NULL && __LINE__##VAR##_##CTR##_b;                         \
             __LINE__##VAR##_##CTR##_b = false)                                    \
                for (bstring *VAR = ((BLIST)->lst[((CTR) = 0)]);                   \
                     (CTR) < (BLIST)->qty && (((VAR) = (BLIST)->lst[(CTR)]) || 1); \
                     ++(CTR))
#endif

#define B_LIST_SORT(BLIST) \
        qsort((BLIST)->lst, (BLIST)->qty, sizeof((BLIST)->lst[0]), &b_strcmp_wrap)
#define B_LIST_SORT_FAST(BLIST) \
        qsort((BLIST)->lst, (BLIST)->qty, sizeof((BLIST)->lst[0]), &b_strcmp_fast_wrap)

#define B_LIST_BSEARCH(BLIST, ITEM_) \
        bsearch(&(ITEM_), (BLIST)->lst, (BLIST)->qty, sizeof(bstring *), &b_strcmp_wrap)
#define B_LIST_BSEARCH_FAST(BLIST, ITEM_) \
        _Generic(ITEM_, bstring *: bsearch, const bstring *: bsearch) \
        (&(ITEM_), (BLIST)->lst, (BLIST)->qty, sizeof(bstring *), &b_strcmp_fast_wrap)

/*--------------------------------------------------------------------------------------*/

#define BSTR_M_DEL_SRC   0x01
#define BSTR_M_SORT      0x02
#define BSTR_M_SORT_FAST 0x04
#define BSTR_M_DEL_DUPS  0x08

#if 0
#define _bstr_helper_b_list_steal(lst, bstr, VL, VB, VR) \
        __extension__({                                  \
                b_list * VL = (lst);                     \
                bstring *VB = (bstr);                    \
                int      VR = b_list_append(VL, VB);     \
                VB          = NULL;                      \
                VR;                                      \
        })
#define b_list_steal(lst, bstr) \
        _bstr_helper_b_list_steal(lst, bstr, P99_UNIQ(L), P99_UNIQ(B), P99_UNIQ(R))
#endif

BSTR_PUBLIC int       b_list_append(b_list *list, bstring *bstr);
BSTR_PUBLIC int       b_list_merge(b_list **dest, b_list *src, int flags);
BSTR_PUBLIC int       b_list_remove_dups(b_list **listp);
BSTR_PUBLIC b_list   *b_list_copy(const b_list *list);
BSTR_PUBLIC b_list   *b_list_clone(const b_list *list);
BSTR_PUBLIC b_list   *b_list_clone_swap(b_list *list);
BSTR_PUBLIC bstring  *b_list_join(const b_list *list, const bstring *sep);

BSTR_PUBLIC int b_list_writeprotect(b_list *list);
BSTR_PUBLIC int b_list_writeallow(b_list *list);

BSTR_PUBLIC bstring  *b_join_quote(const b_list *bl, const bstring *sep, int ch);

BSTR_PUBLIC int     b_memsep(bstring *dest, bstring *stringp, char delim);
BSTR_PUBLIC b_list *b_strsep(bstring *ostr, const char *delim, int refonly);
BSTR_PUBLIC b_list *b_split_char(bstring *split, int delim, bool destroy);
BSTR_PUBLIC b_list *b_split_lines(bstring *split, bool destroy);

BSTR_PUBLIC int b_advance(bstring *bstr, unsigned n);

/*--------------------------------------------------------------------------------------*/

__attribute__((pure))
BSTR_PUBLIC int64_t b_strstr(const bstring *haystack, const bstring *needle, unsigned pos);
__attribute__((pure))
BSTR_PUBLIC int64_t b_strpbrk_pos(const bstring *bstr, unsigned pos, const bstring *delim);
__attribute__((pure))
BSTR_PUBLIC int64_t b_strrpbrk_pos(const bstring *bstr, unsigned pos, const bstring *delim);
__attribute__((pure))
BSTR_PUBLIC _Bool   b_starts_with(const bstring *b0, const bstring *b1);

#define b_strpbrk(BSTR_, DELIM_) b_strpbrk_pos((BSTR_), 0, (DELIM_))
#define b_strrpbrk(BSTR_, DELIM_) b_strrpbrk_pos((BSTR_), ((BSTR_)->slen), (DELIM_))

BSTR_PUBLIC int        b_regularize_path(bstring *path) __attribute__((pure));
BSTR_PUBLIC bstring   *b_dirname(const bstring *path);
BSTR_PUBLIC bstring   *b_basename(const bstring *path);
BSTR_PUBLIC int        b_chomp(bstring *bstr);
BSTR_PUBLIC int        b_strip_leading_ws(bstring *bstr);
BSTR_PUBLIC int        b_strip_trailing_ws(bstring *bstr);
BSTR_PUBLIC int        b_replace_ch(bstring *bstr, int find, int replacement);
BSTR_PUBLIC int        b_catblk_nonul(bstring *bstr, void *blk, unsigned len);
BSTR_PUBLIC int        b_insert_char(bstring *str, unsigned location, int ch);

BSTR_PUBLIC bstring   *_b_sprintf  (const bstring *fmt, ...);
BSTR_PUBLIC bstring   *_b_vsprintf (const bstring *fmt, va_list args);
BSTR_PUBLIC int        _b_fprintf  (FILE *out_fp, const bstring *fmt, ...);
BSTR_PUBLIC int        _b_vfprintf (FILE *out_fp, const bstring *fmt, va_list args);
BSTR_PUBLIC int        _b_dprintf  (int out_fd, const bstring *fmt, ...);
BSTR_PUBLIC int        _b_vdprintf (int out_fd, const bstring *fmt, va_list args);
BSTR_PUBLIC int        _b_sprintfa (bstring *dest, const bstring *fmt, ...);
BSTR_PUBLIC int        _b_vsprintfa(bstring *dest, const bstring *fmt, va_list args);

#define b_sprintf(FMT, ...)         _b_sprintf(B(FMT),          ##__VA_ARGS__)
#define b_vsprintf(FMT, ...)        _b_vsprintf(B(FMT),         ##__VA_ARGS__)
#define b_fprintf(FP, FMT, ...)     _b_fprintf((FP), B(FMT),    ##__VA_ARGS__)
#define b_vfprintf(FP, FMT, ...)    _b_vfprintf((FP), B(FMT),   ##__VA_ARGS__)
#define b_dprintf(FD, FMT, ...)     _b_dprintf((FD), B(FMT),    ##__VA_ARGS__)
#define b_vdprintf(FD, FMT, ...)    _b_vdprintf((FD), B(FMT),   ##__VA_ARGS__)
#define b_sprintfa(DST, FMT, ...)   _b_sprintfa((DST), B(FMT),  ##__VA_ARGS__)
#define b_vsprintfa(DST, FMT, ...)  _b_vsprintfa((DST), B(FMT), ##__VA_ARGS__)
#define b_vsdprintfa(DST, FMT, ...) _b_vsprintfa((DST), B(FMT), ##__VA_ARGS__)

#define b_printf(...)  b_fprintf(stdout, __VA_ARGS__)
#define b_eprintf(...) b_fprintf(stderr, __VA_ARGS__)

BSTR_PUBLIC bstring *b_ll2str(const long long value);
BSTR_PUBLIC int      b_strcmp_fast(const bstring *a, const bstring *b) __attribute__((pure));
BSTR_PUBLIC int      b_strcmp_fast_wrap(const void *vA, const void *vB) __attribute__((pure));
BSTR_PUBLIC int      b_strcmp_wrap(const void *vA, const void *vB) __attribute__((pure));

/*--------------------------------------------------------------------------------------*/

#define b_conchar b_catchar

INLINE int
b_catchar(bstring *bstr, const char ch)
{
        if (!bstr || !bstr->data || ((bstr->flags & BSTR_WRITE_ALLOWED) == 0))
                abort();

        if (bstr->mlen < (bstr->slen + 2))
                if (b_alloc(bstr, bstr->slen + 2) != BSTR_OK)
                        abort();

        bstr->data[bstr->slen++] = (uchar)ch;
        bstr->data[bstr->slen]   = (uchar)'\0';

        return BSTR_OK;
}


#undef BSTR_PRIVATE
#undef BSTR_PUBLIC
#undef INLINE
#ifdef _MSC_VER
#  undef __attribute__
#endif

#ifdef __cplusplus
}
#endif

#endif /* additions.h */
