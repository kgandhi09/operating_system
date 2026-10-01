/* A Bison parser, made by GNU Bison 3.4.1.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 43 "f-exp.y"


#include "expression.h"
#include "value.h"
#include "parser-defs.h"
#include "language.h"
#include "f-lang.h"
#include "block.h"
#include <algorithm>
#include "type-stack.h"
#include "f-exp.h"

#define parse_type(ps) builtin_type (ps->gdbarch ())
#define parse_f_type(ps) builtin_f_type (ps->gdbarch ())

/* Remap normal yacc parser interface names (yyparse, yylex, yyerror,
   etc).  */
#define GDB_YY_REMAP_PREFIX f_
#include "yy-remap.h"

/* The state of the parser, used internally when we are parsing the
   expression.  */

static struct parser_state *pstate = NULL;

/* Depth of parentheses.  */
static int paren_depth;

/* The current type stack.  */
static struct type_stack *type_stack;

int yyparse (void);

static int yylex (void);

static void yyerror (const char *);

static void growbuf_by_size (int);

static int match_string_literal (void);

static void push_kind_type (LONGEST val, struct type *type);

static struct type *convert_to_kind_type (struct type *basetype, int kind);

static void wrap_unop_intrinsic (exp_opcode opcode);

static void wrap_binop_intrinsic (exp_opcode opcode);

static void wrap_ternop_intrinsic (exp_opcode opcode);

template<typename T>
static void fortran_wrap2_kind (type *base_type);

template<typename T>
static void fortran_wrap3_kind (type *base_type);

using namespace expr;

#line 130 "f-exp.c.tmp"

# ifndef YY_NULLPTRPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTRPTR nullptr
#   else
#    define YY_NULLPTRPTR 0
#   endif
#  else
#   define YY_NULLPTRPTR ((void*)0)
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif


/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    INT = 258,
    FLOAT = 259,
    STRING_LITERAL = 260,
    BOOLEAN_LITERAL = 261,
    NAME = 262,
    TYPENAME = 263,
    COMPLETE = 264,
    NAME_OR_INT = 265,
    SIZEOF = 266,
    KIND = 267,
    ERROR = 268,
    INT_S1_KEYWORD = 269,
    INT_S2_KEYWORD = 270,
    INT_KEYWORD = 271,
    INT_S4_KEYWORD = 272,
    INT_S8_KEYWORD = 273,
    LOGICAL_S1_KEYWORD = 274,
    LOGICAL_S2_KEYWORD = 275,
    LOGICAL_KEYWORD = 276,
    LOGICAL_S4_KEYWORD = 277,
    LOGICAL_S8_KEYWORD = 278,
    REAL_KEYWORD = 279,
    REAL_S4_KEYWORD = 280,
    REAL_S8_KEYWORD = 281,
    REAL_S16_KEYWORD = 282,
    COMPLEX_KEYWORD = 283,
    COMPLEX_S4_KEYWORD = 284,
    COMPLEX_S8_KEYWORD = 285,
    COMPLEX_S16_KEYWORD = 286,
    BOOL_AND = 287,
    BOOL_OR = 288,
    BOOL_NOT = 289,
    SINGLE = 290,
    DOUBLE = 291,
    PRECISION = 292,
    CHARACTER = 293,
    DOLLAR_VARIABLE = 294,
    ASSIGN_MODIFY = 295,
    UNOP_INTRINSIC = 296,
    BINOP_INTRINSIC = 297,
    UNOP_OR_BINOP_INTRINSIC = 298,
    UNOP_OR_BINOP_OR_TERNOP_INTRINSIC = 299,
    ABOVE_COMMA = 300,
    EQUAL = 301,
    NOTEQUAL = 302,
    LESSTHAN = 303,
    GREATERTHAN = 304,
    LEQ = 305,
    GEQ = 306,
    LSH = 307,
    RSH = 308,
    STARSTAR = 309,
    UNARY = 310
  };
#endif
/* Tokens.  */
#define INT 258
#define FLOAT 259
#define STRING_LITERAL 260
#define BOOLEAN_LITERAL 261
#define NAME 262
#define TYPENAME 263
#define COMPLETE 264
#define NAME_OR_INT 265
#define SIZEOF 266
#define KIND 267
#define ERROR 268
#define INT_S1_KEYWORD 269
#define INT_S2_KEYWORD 270
#define INT_KEYWORD 271
#define INT_S4_KEYWORD 272
#define INT_S8_KEYWORD 273
#define LOGICAL_S1_KEYWORD 274
#define LOGICAL_S2_KEYWORD 275
#define LOGICAL_KEYWORD 276
#define LOGICAL_S4_KEYWORD 277
#define LOGICAL_S8_KEYWORD 278
#define REAL_KEYWORD 279
#define REAL_S4_KEYWORD 280
#define REAL_S8_KEYWORD 281
#define REAL_S16_KEYWORD 282
#define COMPLEX_KEYWORD 283
#define COMPLEX_S4_KEYWORD 284
#define COMPLEX_S8_KEYWORD 285
#define COMPLEX_S16_KEYWORD 286
#define BOOL_AND 287
#define BOOL_OR 288
#define BOOL_NOT 289
#define SINGLE 290
#define DOUBLE 291
#define PRECISION 292
#define CHARACTER 293
#define DOLLAR_VARIABLE 294
#define ASSIGN_MODIFY 295
#define UNOP_INTRINSIC 296
#define BINOP_INTRINSIC 297
#define UNOP_OR_BINOP_INTRINSIC 298
#define UNOP_OR_BINOP_OR_TERNOP_INTRINSIC 299
#define ABOVE_COMMA 300
#define EQUAL 301
#define NOTEQUAL 302
#define LESSTHAN 303
#define GREATERTHAN 304
#define LEQ 305
#define GEQ 306
#define LSH 307
#define RSH 308
#define STARSTAR 309
#define UNARY 310

/* Value type.  */
#if ! defined f_exp_YYSTYPE && ! defined f_exp_YYSTYPE_IS_DECLARED
union f_exp_YYSTYPE
{
#line 108 "f-exp.y"

    LONGEST lval;
    struct {
      LONGEST val;
      struct type *type;
    } typed_val;
    struct {
      gdb_byte val[16];
      struct type *type;
    } typed_val_float;
    struct symbol *sym;
    struct type *tval;
    struct stoken sval;
    struct ttype tsym;
    struct symtoken ssym;
    int voidval;
    enum exp_opcode opcode;
    struct internalvar *ivar;

    struct type **tvec;
    int *ivec;
  

#line 304 "f-exp.c.tmp"

};
typedef union f_exp_YYSTYPE f_exp_YYSTYPE;
# define f_exp_YYSTYPE_IS_TRIVIAL 1
# define f_exp_YYSTYPE_IS_DECLARED 1
#endif


extern f_exp_YYSTYPE yylval;

int yyparse (void);



/* Second part of user prologue.  */
#line 131 "f-exp.y"

/* f_exp_YYSTYPE gets defined by %union */
static int parse_number (struct parser_state *, const char *, int,
			 int, f_exp_YYSTYPE *);

#line 326 "f-exp.c.tmp"


#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or xmalloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined xmalloc) \
             && (defined YYFREE || defined xfree)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC xmalloc
#   if ! defined xmalloc && ! defined EXIT_SUCCESS
void *xmalloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE xfree
#   if ! defined xfree && ! defined EXIT_SUCCESS
void xfree (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined f_exp_YYSTYPE_IS_TRIVIAL && f_exp_YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union f_exp_yyalloc
{
  yytype_int16 yyss_alloc;
  f_exp_YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union f_exp_yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (f_exp_YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  69
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   924

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  72
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  20
/* YYNRULES -- Number of rules.  */
#define YYNRULES  113
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  184

#define YYUNDEFTOK  2
#define YYMAXUTOK   310

/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  ((unsigned) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    66,    51,     2,
      68,    69,    63,    61,    45,    62,     2,    64,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    71,     2,
       2,    47,     2,    48,    60,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    50,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    49,     2,    70,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      46,    52,    53,    54,    55,    56,    57,    58,    59,    65,
      67
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   217,   217,   218,   221,   225,   230,   234,   238,   242,
     246,   250,   254,   264,   263,   274,   280,   287,   286,   307,
     306,   330,   333,   337,   341,   345,   351,   361,   370,   379,
     390,   402,   414,   426,   438,   442,   452,   459,   466,   476,
     488,   492,   496,   500,   504,   508,   512,   516,   520,   524,
     528,   532,   536,   540,   544,   548,   552,   556,   561,   565,
     569,   578,   585,   595,   604,   607,   611,   620,   624,   631,
     639,   642,   643,   695,   697,   699,   701,   703,   706,   708,
     710,   712,   714,   718,   720,   725,   727,   729,   731,   733,
     735,   737,   739,   741,   743,   745,   747,   749,   751,   753,
     755,   761,   763,   765,   767,   773,   775,   777,   779,   784,
     789,   797,   799,   803
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "INT", "FLOAT", "STRING_LITERAL",
  "BOOLEAN_LITERAL", "NAME", "TYPENAME", "COMPLETE", "NAME_OR_INT",
  "SIZEOF", "KIND", "ERROR", "INT_S1_KEYWORD", "INT_S2_KEYWORD",
  "INT_KEYWORD", "INT_S4_KEYWORD", "INT_S8_KEYWORD", "LOGICAL_S1_KEYWORD",
  "LOGICAL_S2_KEYWORD", "LOGICAL_KEYWORD", "LOGICAL_S4_KEYWORD",
  "LOGICAL_S8_KEYWORD", "REAL_KEYWORD", "REAL_S4_KEYWORD",
  "REAL_S8_KEYWORD", "REAL_S16_KEYWORD", "COMPLEX_KEYWORD",
  "COMPLEX_S4_KEYWORD", "COMPLEX_S8_KEYWORD", "COMPLEX_S16_KEYWORD",
  "BOOL_AND", "BOOL_OR", "BOOL_NOT", "SINGLE", "DOUBLE", "PRECISION",
  "CHARACTER", "DOLLAR_VARIABLE", "ASSIGN_MODIFY", "UNOP_INTRINSIC",
  "BINOP_INTRINSIC", "UNOP_OR_BINOP_INTRINSIC",
  "UNOP_OR_BINOP_OR_TERNOP_INTRINSIC", "','", "ABOVE_COMMA", "'='", "'?'",
  "'|'", "'^'", "'&'", "EQUAL", "NOTEQUAL", "LESSTHAN", "GREATERTHAN",
  "LEQ", "GEQ", "LSH", "RSH", "'@'", "'+'", "'-'", "'*'", "'/'",
  "STARSTAR", "'%'", "UNARY", "'('", "')'", "'~'", "':'", "$accept",
  "start", "type_exp", "exp", "$@1", "$@2", "$@3", "arglist", "subrange",
  "complexnum", "variable", "type", "ptype", "abs_decl", "direct_abs_decl",
  "func_mod", "typebase", "nonempty_typelist", "name", "name_not_typename", YY_NULLPTRPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,    44,   300,    61,    63,   124,
      94,    38,   301,   302,   303,   304,   305,   306,   307,   308,
      64,    43,    45,    42,    47,   309,    37,   310,    40,    41,
     126,    58
};
# endif

#define YYPACT_NINF -103

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-103)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     249,  -103,  -103,  -103,  -103,  -103,  -103,  -103,   291,   -62,
    -103,  -103,  -103,  -103,  -103,  -103,  -103,  -103,  -103,  -103,
    -103,  -103,  -103,  -103,  -103,  -103,  -103,  -103,   333,     6,
      18,  -103,  -103,   -61,   -33,   -26,   -21,   333,   333,   333,
     249,   333,    56,  -103,   718,  -103,  -103,  -103,    -6,  -103,
     249,   -20,   333,   -20,  -103,  -103,  -103,  -103,   333,   333,
    -103,  -103,   -20,   -20,   -20,   567,   -16,    -4,   -20,  -103,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,    60,  -103,    -6,    -2,   432,  -103,    29,  -103,    40,
     373,   605,   681,    88,    88,   333,  -103,  -103,   333,   802,
     753,   718,   718,   821,   839,   856,   733,   733,    53,    53,
      53,    53,   133,   133,    72,    38,    38,    58,    58,    58,
    -103,  -103,  -103,    54,    88,  -103,  -103,  -103,    63,  -103,
    -103,    59,   -40,    -5,  -103,   375,  -103,  -103,   333,   166,
     456,   -18,  -103,    -9,   718,   -20,  -103,    51,   104,  -103,
     812,  -103,   643,   333,   493,   180,    88,  -103,  -103,  -103,
      75,  -103,  -103,   718,   333,   333,   530,   456,  -103,  -103,
     718,   718,   333,   718
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    61,    63,    68,    67,   113,    85,    62,     0,     0,
      86,    87,    88,    89,    90,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,     0,     0,
       0,    91,    65,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     3,     2,    64,     4,    70,    71,    69,
       0,    11,     0,     9,   107,   105,   108,   106,     0,     0,
      17,    19,     7,     8,     6,     0,     0,     0,    10,     1,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    75,    73,     0,    72,    77,    82,     0,
       0,     0,     0,    21,    21,     0,     5,    35,     0,    57,
      58,    60,    59,    56,    55,    54,    48,    49,    52,    53,
      50,    51,    46,    47,    40,    44,    45,    42,    43,    41,
     111,   112,    39,    37,    21,    76,    80,    74,     0,    83,
     109,     0,     0,     0,    81,    66,    12,    15,     0,    29,
      22,     0,    23,     0,    34,    36,    38,     0,     0,    78,
       0,    84,     0,     0,    28,    27,     0,    18,    20,    14,
       0,   110,    16,    33,     0,     0,    26,    24,    25,    79,
      32,    31,     0,    30
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
    -103,  -103,  -103,     0,  -103,  -103,  -103,  -102,   -41,  -103,
    -103,     4,  -103,    48,  -103,    49,  -103,  -103,  -103,  -103
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    42,    43,   150,   134,   103,   104,   151,   152,    66,
      45,   140,    47,    96,    97,    98,    48,   142,   133,    49
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      44,   136,   153,     6,    46,   160,    52,    58,    51,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,   166,    53,   161,
      29,    30,   157,    31,    54,    59,   166,    62,    63,    64,
      65,    68,    60,    55,    67,    93,    56,    61,    92,    93,
      65,   167,   100,   107,    99,    57,    69,    94,   101,   102,
     168,    94,    95,   156,   139,   108,    95,   130,   131,   132,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,     1,     2,     3,     4,     5,   166,   143,     7,     8,
       9,    88,    89,    90,    91,   154,    92,   170,   155,   145,
     158,    83,    84,    85,    86,    87,    88,    89,    90,    91,
     169,    92,    28,    90,    91,   178,    92,    32,   159,    33,
      34,    35,    36,    86,    87,    88,    89,    90,    91,    37,
      92,   135,   137,   141,   179,   155,   144,     0,   162,   164,
      38,    39,     0,     0,     0,     0,    40,     0,    41,   149,
       0,     0,     0,   173,   171,   176,   177,     0,     0,     1,
       2,     3,     4,     5,   180,   181,     7,     8,     9,     0,
       0,     0,   183,     1,     2,     3,     4,     5,     0,     0,
       7,     8,     9,    85,    86,    87,    88,    89,    90,    91,
      28,    92,     0,     0,     0,    32,     0,    33,    34,    35,
      36,     0,     0,     0,    28,     0,     0,    37,     0,    32,
       0,    33,    34,    35,    36,     0,     0,     0,    38,    39,
       0,    37,     0,     0,    40,     0,    41,   163,     0,     0,
       0,     0,    38,    39,     0,     0,     0,     0,    40,     0,
      41,   175,     1,     2,     3,     4,     5,     6,     0,     7,
       8,     9,     0,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,     0,    28,    29,    30,     0,    31,    32,     0,
      33,    34,    35,    36,     1,     2,     3,     4,     5,     0,
      37,     7,     8,     9,     0,     0,     0,     0,     0,     0,
       0,    38,    39,     0,     0,     0,     0,    40,     0,    41,
       0,     0,     0,     0,     0,    28,     0,     0,     0,     0,
      32,     0,    33,    34,    35,    36,     1,     2,     3,     4,
       5,     0,    37,     7,     8,     9,     0,     0,     0,     0,
       0,     0,     0,    38,    39,     0,     0,     0,     0,    50,
       0,    41,     0,     0,     0,     0,     0,    28,     0,     0,
       0,     0,    32,     0,    33,    34,    35,    36,     1,     2,
       3,     4,     5,     0,    37,     7,     8,     9,     0,     0,
       0,     0,     0,     0,     0,    38,    39,     0,     0,     0,
       0,    40,     0,    41,     0,    70,    71,     0,     0,    28,
       0,     0,     0,    72,    32,     0,    33,    34,    35,    36,
      73,     0,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
       6,    92,   146,    40,   138,    41,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,     0,     0,    29,    30,     0,
      31,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    93,     0,     0,     0,     0,    70,    71,
       0,     0,     0,     0,     0,    94,    72,     0,     0,     0,
      95,   139,     0,    73,     0,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,     0,    92,    70,    71,   165,     0,     0,
       0,     0,     0,    72,     0,     0,     0,     0,     0,     0,
      73,     0,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
       0,    92,    70,    71,   174,     0,     0,     0,     0,     0,
      72,     0,     0,     0,     0,     0,     0,    73,     0,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,     0,    92,    70,
      71,   182,     0,     0,     0,     0,     0,    72,     0,     0,
       0,     0,   105,     0,    73,     0,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,     0,    92,   106,    70,    71,     0,
       0,     0,     0,     0,     0,    72,     0,     0,     0,     0,
       0,     0,    73,     0,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,     0,    92,   147,    70,    71,     0,     0,     0,
       0,     0,     0,    72,     0,     0,     0,     0,     0,     0,
      73,     0,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
       0,    92,   172,    70,    71,     0,     0,     0,     0,     0,
       0,    72,     0,     0,     0,     0,   148,     0,    73,     0,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,     0,    92,
      70,    71,     0,     0,     0,     0,     0,     0,    72,     0,
       0,     0,     0,     0,     0,    73,     0,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    70,    92,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
       0,    92,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
       6,    92,     0,     0,     0,     0,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,     0,     0,    29,    30,     0,
      31,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,     0,
      92,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,     0,    92,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,     0,    92,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,     0,    92
};

static const yytype_int16 yycheck[] =
{
       0,     3,   104,     8,     0,    45,    68,    68,     8,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    45,    28,    69,
      35,    36,   134,    38,    28,    68,    45,    37,    38,    39,
      40,    41,    68,    37,    40,    51,    28,    68,    68,    51,
      50,    69,    52,    69,    50,    37,     0,    63,    58,    59,
      69,    63,    68,     9,    69,    69,    68,     7,     8,     9,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,     3,     4,     5,     6,     7,    45,    68,    10,    11,
      12,    63,    64,    65,    66,   105,    68,     3,   108,    69,
      47,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      69,    68,    34,    65,    66,   166,    68,    39,    69,    41,
      42,    43,    44,    61,    62,    63,    64,    65,    66,    51,
      68,    93,    94,    95,    69,   145,    97,    -1,   148,   149,
      62,    63,    -1,    -1,    -1,    -1,    68,    -1,    70,    71,
      -1,    -1,    -1,   163,   160,   165,   166,    -1,    -1,     3,
       4,     5,     6,     7,   174,   175,    10,    11,    12,    -1,
      -1,    -1,   182,     3,     4,     5,     6,     7,    -1,    -1,
      10,    11,    12,    60,    61,    62,    63,    64,    65,    66,
      34,    68,    -1,    -1,    -1,    39,    -1,    41,    42,    43,
      44,    -1,    -1,    -1,    34,    -1,    -1,    51,    -1,    39,
      -1,    41,    42,    43,    44,    -1,    -1,    -1,    62,    63,
      -1,    51,    -1,    -1,    68,    -1,    70,    71,    -1,    -1,
      -1,    -1,    62,    63,    -1,    -1,    -1,    -1,    68,    -1,
      70,    71,     3,     4,     5,     6,     7,     8,    -1,    10,
      11,    12,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    -1,    -1,    34,    35,    36,    -1,    38,    39,    -1,
      41,    42,    43,    44,     3,     4,     5,     6,     7,    -1,
      51,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    62,    63,    -1,    -1,    -1,    -1,    68,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    34,    -1,    -1,    -1,    -1,
      39,    -1,    41,    42,    43,    44,     3,     4,     5,     6,
       7,    -1,    51,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    62,    63,    -1,    -1,    -1,    -1,    68,
      -1,    70,    -1,    -1,    -1,    -1,    -1,    34,    -1,    -1,
      -1,    -1,    39,    -1,    41,    42,    43,    44,     3,     4,
       5,     6,     7,    -1,    51,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    62,    63,    -1,    -1,    -1,
      -1,    68,    -1,    70,    -1,    32,    33,    -1,    -1,    34,
      -1,    -1,    -1,    40,    39,    -1,    41,    42,    43,    44,
      47,    -1,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
       8,    68,    69,    68,    12,    70,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    -1,    -1,    -1,    35,    36,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    32,    33,
      -1,    -1,    -1,    -1,    -1,    63,    40,    -1,    -1,    -1,
      68,    69,    -1,    47,    -1,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    68,    32,    33,    71,    -1,    -1,
      -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    -1,
      47,    -1,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    68,    32,    33,    71,    -1,    -1,    -1,    -1,    -1,
      40,    -1,    -1,    -1,    -1,    -1,    -1,    47,    -1,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    68,    32,
      33,    71,    -1,    -1,    -1,    -1,    -1,    40,    -1,    -1,
      -1,    -1,    45,    -1,    47,    -1,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    68,    69,    32,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,
      -1,    -1,    47,    -1,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    68,    69,    32,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    -1,
      47,    -1,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    68,    69,    32,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    40,    -1,    -1,    -1,    -1,    45,    -1,    47,    -1,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    68,
      32,    33,    -1,    -1,    -1,    -1,    -1,    -1,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    -1,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    32,    68,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    68,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
       8,    68,    -1,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    -1,    -1,    -1,    35,    36,    -1,
      38,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      68,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    68,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    68,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    68
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,    10,    11,    12,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    34,    35,
      36,    38,    39,    41,    42,    43,    44,    51,    62,    63,
      68,    70,    73,    74,    75,    82,    83,    84,    88,    91,
      68,    75,    68,    75,    28,    37,    28,    37,    68,    68,
      68,    68,    75,    75,    75,    75,    81,    83,    75,     0,
      32,    33,    40,    47,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    68,    51,    63,    68,    85,    86,    87,    83,
      75,    75,    75,    77,    78,    45,    69,    69,    69,    75,
      75,    75,    75,    75,    75,    75,    75,    75,    75,    75,
      75,    75,    75,    75,    75,    75,    75,    75,    75,    75,
       7,     8,     9,    90,    76,    85,     3,    85,    12,    69,
      83,    85,    89,    68,    87,    69,    69,    69,    45,    71,
      75,    79,    80,    79,    75,    75,     9,    79,    47,    69,
      45,    69,    75,    71,    75,    71,    45,    69,    69,    69,
       3,    83,    69,    75,    71,    71,    75,    75,    80,    69,
      75,    75,    71,    75
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    72,    73,    73,    74,    75,    75,    75,    75,    75,
      75,    75,    75,    76,    75,    75,    75,    77,    75,    78,
      75,    79,    79,    79,    79,    79,    80,    80,    80,    80,
      80,    80,    80,    80,    81,    75,    75,    75,    75,    75,
      75,    75,    75,    75,    75,    75,    75,    75,    75,    75,
      75,    75,    75,    75,    75,    75,    75,    75,    75,    75,
      75,    75,    75,    75,    75,    75,    75,    75,    75,    82,
      83,    84,    84,    85,    85,    85,    85,    85,    86,    86,
      86,    86,    86,    87,    87,    88,    88,    88,    88,    88,
      88,    88,    88,    88,    88,    88,    88,    88,    88,    88,
      88,    88,    88,    88,    88,    88,    88,    88,    88,    89,
      89,    90,    90,    91
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     3,     2,     2,     2,     2,
       2,     2,     4,     0,     5,     4,     6,     0,     5,     0,
       5,     0,     1,     1,     3,     3,     3,     2,     2,     1,
       5,     4,     4,     3,     3,     3,     4,     3,     4,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     1,     1,     1,     1,     1,     4,     1,     1,     1,
       1,     1,     2,     1,     2,     1,     2,     1,     3,     5,
       2,     2,     1,     2,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     2,     2,     2,     1,
       3,     1,     1,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, f_exp_YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, f_exp_YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyo, yytype, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, f_exp_YYSTYPE *yyvsp, int yyrule)
{
  unsigned long yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &yyvsp[(yyi + 1) - (yynrhs)]
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return (YYSIZE_T) (yystpcpy (yyres, yystr) - yyres);
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTRPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTRPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTRPTR, yytname[yyx]);
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, f_exp_YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
f_exp_YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to xreallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    f_exp_YYSTYPE yyvsa[YYINITDEPTH];
    f_exp_YYSTYPE *yyvs;
    f_exp_YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  f_exp_YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yynewstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  *yyssp = (yytype_int16) yystate;

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = (YYSIZE_T) (yyssp - yyss + 1);

# if defined yyoverflow
      {
        /* Give user a chance to xreallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        f_exp_YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union f_exp_yyalloc *yyptr =
          (union f_exp_yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 4:
#line 222 "f-exp.y"
    { pstate->push_new<type_operation> ((yyvsp[0].tval)); }
#line 1695 "f-exp.c.tmp"
    break;

  case 5:
#line 226 "f-exp.y"
    { }
#line 1701 "f-exp.c.tmp"
    break;

  case 6:
#line 231 "f-exp.y"
    { pstate->wrap<unop_ind_operation> (); }
#line 1707 "f-exp.c.tmp"
    break;

  case 7:
#line 235 "f-exp.y"
    { pstate->wrap<unop_addr_operation> (); }
#line 1713 "f-exp.c.tmp"
    break;

  case 8:
#line 239 "f-exp.y"
    { pstate->wrap<unary_neg_operation> (); }
#line 1719 "f-exp.c.tmp"
    break;

  case 9:
#line 243 "f-exp.y"
    { pstate->wrap<unary_logical_not_operation> (); }
#line 1725 "f-exp.c.tmp"
    break;

  case 10:
#line 247 "f-exp.y"
    { pstate->wrap<unary_complement_operation> (); }
#line 1731 "f-exp.c.tmp"
    break;

  case 11:
#line 251 "f-exp.y"
    { pstate->wrap<unop_sizeof_operation> (); }
#line 1737 "f-exp.c.tmp"
    break;

  case 12:
#line 255 "f-exp.y"
    { pstate->wrap<fortran_kind_operation> (); }
#line 1743 "f-exp.c.tmp"
    break;

  case 13:
#line 264 "f-exp.y"
    { pstate->start_arglist (); }
#line 1749 "f-exp.c.tmp"
    break;

  case 14:
#line 266 "f-exp.y"
    {
			  std::vector<operation_up> args
			    = pstate->pop_vector (pstate->end_arglist ());
			  pstate->push_new<fortran_undetermined>
			    (pstate->pop (), std::move (args));
			}
#line 1760 "f-exp.c.tmp"
    break;

  case 15:
#line 275 "f-exp.y"
    {
			  wrap_unop_intrinsic ((yyvsp[-3].opcode));
			}
#line 1768 "f-exp.c.tmp"
    break;

  case 16:
#line 281 "f-exp.y"
    {
			  wrap_binop_intrinsic ((yyvsp[-5].opcode));
			}
#line 1776 "f-exp.c.tmp"
    break;

  case 17:
#line 287 "f-exp.y"
    { pstate->start_arglist (); }
#line 1782 "f-exp.c.tmp"
    break;

  case 18:
#line 289 "f-exp.y"
    {
			  const int n = pstate->end_arglist ();

			  switch (n)
			    {
			    case 1:
			      wrap_unop_intrinsic ((yyvsp[-4].opcode));
			      break;
			    case 2:
			      wrap_binop_intrinsic ((yyvsp[-4].opcode));
			      break;
			    default:
			      gdb_assert_not_reached
				("wrong number of arguments for intrinsics");
			    }
			}
#line 1803 "f-exp.c.tmp"
    break;

  case 19:
#line 307 "f-exp.y"
    { pstate->start_arglist (); }
#line 1809 "f-exp.c.tmp"
    break;

  case 20:
#line 309 "f-exp.y"
    {
			  const int n = pstate->end_arglist ();

			  switch (n)
			    {
			    case 1:
			      wrap_unop_intrinsic ((yyvsp[-4].opcode));
			      break;
			    case 2:
			      wrap_binop_intrinsic ((yyvsp[-4].opcode));
			      break;
			    case 3:
			      wrap_ternop_intrinsic ((yyvsp[-4].opcode));
			      break;
			    default:
			      gdb_assert_not_reached
				("wrong number of arguments for intrinsics");
			    }
			}
#line 1833 "f-exp.c.tmp"
    break;

  case 22:
#line 334 "f-exp.y"
    { pstate->arglist_len = 1; }
#line 1839 "f-exp.c.tmp"
    break;

  case 23:
#line 338 "f-exp.y"
    { pstate->arglist_len = 1; }
#line 1845 "f-exp.c.tmp"
    break;

  case 24:
#line 342 "f-exp.y"
    { pstate->arglist_len++; }
#line 1851 "f-exp.c.tmp"
    break;

  case 25:
#line 346 "f-exp.y"
    { pstate->arglist_len++; }
#line 1857 "f-exp.c.tmp"
    break;

  case 26:
#line 352 "f-exp.y"
    {
			  operation_up high = pstate->pop ();
			  operation_up low = pstate->pop ();
			  pstate->push_new<fortran_range_operation>
			    (RANGE_STANDARD, std::move (low),
			     std::move (high), operation_up ());
			}
#line 1869 "f-exp.c.tmp"
    break;

  case 27:
#line 362 "f-exp.y"
    {
			  operation_up low = pstate->pop ();
			  pstate->push_new<fortran_range_operation>
			    (RANGE_HIGH_BOUND_DEFAULT, std::move (low),
			     operation_up (), operation_up ());
			}
#line 1880 "f-exp.c.tmp"
    break;

  case 28:
#line 371 "f-exp.y"
    {
			  operation_up high = pstate->pop ();
			  pstate->push_new<fortran_range_operation>
			    (RANGE_LOW_BOUND_DEFAULT, operation_up (),
			     std::move (high), operation_up ());
			}
#line 1891 "f-exp.c.tmp"
    break;

  case 29:
#line 380 "f-exp.y"
    {
			  pstate->push_new<fortran_range_operation>
			    (RANGE_LOW_BOUND_DEFAULT
			     | RANGE_HIGH_BOUND_DEFAULT,
			     operation_up (), operation_up (),
			     operation_up ());
			}
#line 1903 "f-exp.c.tmp"
    break;

  case 30:
#line 391 "f-exp.y"
    {
			  operation_up stride = pstate->pop ();
			  operation_up high = pstate->pop ();
			  operation_up low = pstate->pop ();
			  pstate->push_new<fortran_range_operation>
			    (RANGE_STANDARD | RANGE_HAS_STRIDE,
			     std::move (low), std::move (high),
			     std::move (stride));
			}
#line 1917 "f-exp.c.tmp"
    break;

  case 31:
#line 403 "f-exp.y"
    {
			  operation_up stride = pstate->pop ();
			  operation_up low = pstate->pop ();
			  pstate->push_new<fortran_range_operation>
			    (RANGE_HIGH_BOUND_DEFAULT
			     | RANGE_HAS_STRIDE,
			     std::move (low), operation_up (),
			     std::move (stride));
			}
#line 1931 "f-exp.c.tmp"
    break;

  case 32:
#line 415 "f-exp.y"
    {
			  operation_up stride = pstate->pop ();
			  operation_up high = pstate->pop ();
			  pstate->push_new<fortran_range_operation>
			    (RANGE_LOW_BOUND_DEFAULT
			     | RANGE_HAS_STRIDE,
			     operation_up (), std::move (high),
			     std::move (stride));
			}
#line 1945 "f-exp.c.tmp"
    break;

  case 33:
#line 427 "f-exp.y"
    {
			  operation_up stride = pstate->pop ();
			  pstate->push_new<fortran_range_operation>
			    (RANGE_LOW_BOUND_DEFAULT
			     | RANGE_HIGH_BOUND_DEFAULT
			     | RANGE_HAS_STRIDE,
			     operation_up (), operation_up (),
			     std::move (stride));
			}
#line 1959 "f-exp.c.tmp"
    break;

  case 34:
#line 439 "f-exp.y"
    { }
#line 1965 "f-exp.c.tmp"
    break;

  case 35:
#line 443 "f-exp.y"
    {
			  operation_up rhs = pstate->pop ();
			  operation_up lhs = pstate->pop ();
			  pstate->push_new<complex_operation>
			    (std::move (lhs), std::move (rhs),
			     parse_f_type (pstate)->builtin_complex_s16);
			}
#line 1977 "f-exp.c.tmp"
    break;

  case 36:
#line 453 "f-exp.y"
    {
			  pstate->push_new<unop_cast_operation>
			    (pstate->pop (), (yyvsp[-2].tval));
			}
#line 1986 "f-exp.c.tmp"
    break;

  case 37:
#line 460 "f-exp.y"
    {
			  pstate->push_new<fortran_structop_operation>
			    (pstate->pop (), copy_name ((yyvsp[0].sval)));
			}
#line 1995 "f-exp.c.tmp"
    break;

  case 38:
#line 467 "f-exp.y"
    {
			  structop_base_operation *op
			    = new fortran_structop_operation (pstate->pop (),
							      copy_name ((yyvsp[-1].sval)));
			  pstate->mark_struct_expression (op);
			  pstate->push (operation_up (op));
			}
#line 2007 "f-exp.c.tmp"
    break;

  case 39:
#line 477 "f-exp.y"
    {
			  structop_base_operation *op
			    = new fortran_structop_operation (pstate->pop (),
							      "");
			  pstate->mark_struct_expression (op);
			  pstate->push (operation_up (op));
			}
#line 2019 "f-exp.c.tmp"
    break;

  case 40:
#line 489 "f-exp.y"
    { pstate->wrap2<repeat_operation> (); }
#line 2025 "f-exp.c.tmp"
    break;

  case 41:
#line 493 "f-exp.y"
    { pstate->wrap2<exp_operation> (); }
#line 2031 "f-exp.c.tmp"
    break;

  case 42:
#line 497 "f-exp.y"
    { pstate->wrap2<mul_operation> (); }
#line 2037 "f-exp.c.tmp"
    break;

  case 43:
#line 501 "f-exp.y"
    { pstate->wrap2<div_operation> (); }
#line 2043 "f-exp.c.tmp"
    break;

  case 44:
#line 505 "f-exp.y"
    { pstate->wrap2<add_operation> (); }
#line 2049 "f-exp.c.tmp"
    break;

  case 45:
#line 509 "f-exp.y"
    { pstate->wrap2<sub_operation> (); }
#line 2055 "f-exp.c.tmp"
    break;

  case 46:
#line 513 "f-exp.y"
    { pstate->wrap2<lsh_operation> (); }
#line 2061 "f-exp.c.tmp"
    break;

  case 47:
#line 517 "f-exp.y"
    { pstate->wrap2<rsh_operation> (); }
#line 2067 "f-exp.c.tmp"
    break;

  case 48:
#line 521 "f-exp.y"
    { pstate->wrap2<equal_operation> (); }
#line 2073 "f-exp.c.tmp"
    break;

  case 49:
#line 525 "f-exp.y"
    { pstate->wrap2<notequal_operation> (); }
#line 2079 "f-exp.c.tmp"
    break;

  case 50:
#line 529 "f-exp.y"
    { pstate->wrap2<leq_operation> (); }
#line 2085 "f-exp.c.tmp"
    break;

  case 51:
#line 533 "f-exp.y"
    { pstate->wrap2<geq_operation> (); }
#line 2091 "f-exp.c.tmp"
    break;

  case 52:
#line 537 "f-exp.y"
    { pstate->wrap2<less_operation> (); }
#line 2097 "f-exp.c.tmp"
    break;

  case 53:
#line 541 "f-exp.y"
    { pstate->wrap2<gtr_operation> (); }
#line 2103 "f-exp.c.tmp"
    break;

  case 54:
#line 545 "f-exp.y"
    { pstate->wrap2<bitwise_and_operation> (); }
#line 2109 "f-exp.c.tmp"
    break;

  case 55:
#line 549 "f-exp.y"
    { pstate->wrap2<bitwise_xor_operation> (); }
#line 2115 "f-exp.c.tmp"
    break;

  case 56:
#line 553 "f-exp.y"
    { pstate->wrap2<bitwise_ior_operation> (); }
#line 2121 "f-exp.c.tmp"
    break;

  case 57:
#line 557 "f-exp.y"
    { pstate->wrap2<logical_and_operation> (); }
#line 2127 "f-exp.c.tmp"
    break;

  case 58:
#line 562 "f-exp.y"
    { pstate->wrap2<logical_or_operation> (); }
#line 2133 "f-exp.c.tmp"
    break;

  case 59:
#line 566 "f-exp.y"
    { pstate->wrap2<assign_operation> (); }
#line 2139 "f-exp.c.tmp"
    break;

  case 60:
#line 570 "f-exp.y"
    {
			  operation_up rhs = pstate->pop ();
			  operation_up lhs = pstate->pop ();
			  pstate->push_new<assign_modify_operation>
			    ((yyvsp[-1].opcode), std::move (lhs), std::move (rhs));
			}
#line 2150 "f-exp.c.tmp"
    break;

  case 61:
#line 579 "f-exp.y"
    {
			  pstate->push_new<long_const_operation>
			    ((yyvsp[0].typed_val).type, (yyvsp[0].typed_val).val);
			}
#line 2159 "f-exp.c.tmp"
    break;

  case 62:
#line 586 "f-exp.y"
    { f_exp_YYSTYPE val;
			  parse_number (pstate, (yyvsp[0].ssym).stoken.ptr,
					(yyvsp[0].ssym).stoken.length, 0, &val);
			  pstate->push_new<long_const_operation>
			    (val.typed_val.type,
			     val.typed_val.val);
			}
#line 2171 "f-exp.c.tmp"
    break;

  case 63:
#line 596 "f-exp.y"
    {
			  float_data data;
			  std::copy (std::begin ((yyvsp[0].typed_val_float).val), std::end ((yyvsp[0].typed_val_float).val),
				     std::begin (data));
			  pstate->push_new<float_const_operation> ((yyvsp[0].typed_val_float).type, data);
			}
#line 2182 "f-exp.c.tmp"
    break;

  case 65:
#line 608 "f-exp.y"
    { pstate->push_dollar ((yyvsp[0].sval)); }
#line 2188 "f-exp.c.tmp"
    break;

  case 66:
#line 612 "f-exp.y"
    {
			  (yyvsp[-1].tval) = check_typedef ((yyvsp[-1].tval));
			  pstate->push_new<long_const_operation>
			    (parse_f_type (pstate)->builtin_integer,
			     (yyvsp[-1].tval)->length ());
			}
#line 2199 "f-exp.c.tmp"
    break;

  case 67:
#line 621 "f-exp.y"
    { pstate->push_new<bool_operation> ((yyvsp[0].lval)); }
#line 2205 "f-exp.c.tmp"
    break;

  case 68:
#line 625 "f-exp.y"
    {
			  pstate->push_new<string_operation>
			    (copy_name ((yyvsp[0].sval)));
			}
#line 2214 "f-exp.c.tmp"
    break;

  case 69:
#line 632 "f-exp.y"
    { struct block_symbol sym = (yyvsp[0].ssym).sym;
			  std::string name = copy_name ((yyvsp[0].ssym).stoken);
			  pstate->push_symbol (name.c_str (), sym);
			}
#line 2223 "f-exp.c.tmp"
    break;

  case 72:
#line 644 "f-exp.y"
    {
		  /* This is where the interesting stuff happens.  */
		  int done = 0;
		  int array_size;
		  struct type *follow_type = (yyvsp[-1].tval);
		  struct type *range_type;

		  while (!done)
		    switch (type_stack->pop ())
		      {
		      case tp_end:
			done = 1;
			break;
		      case tp_pointer:
			follow_type = lookup_pointer_type (follow_type);
			break;
		      case tp_reference:
			follow_type = lookup_lvalue_reference_type (follow_type);
			break;
		      case tp_array:
			array_size = type_stack->pop_int ();
			if (array_size != -1)
			  {
			    struct type *idx_type
			      = parse_f_type (pstate)->builtin_integer;
			    type_allocator alloc (idx_type);
			    range_type =
			      create_static_range_type (alloc, idx_type,
							0, array_size - 1);
			    follow_type = create_array_type (alloc,
							     follow_type,
							     range_type);
			  }
			else
			  follow_type = lookup_pointer_type (follow_type);
			break;
		      case tp_function:
			follow_type = lookup_function_type (follow_type);
			break;
		      case tp_kind:
			{
			  int kind_val = type_stack->pop_int ();
			  follow_type
			    = convert_to_kind_type (follow_type, kind_val);
			}
			break;
		      }
		  (yyval.tval) = follow_type;
		}
#line 2277 "f-exp.c.tmp"
    break;

  case 73:
#line 696 "f-exp.y"
    { type_stack->push (tp_pointer); (yyval.voidval) = 0; }
#line 2283 "f-exp.c.tmp"
    break;

  case 74:
#line 698 "f-exp.y"
    { type_stack->push (tp_pointer); (yyval.voidval) = (yyvsp[0].voidval); }
#line 2289 "f-exp.c.tmp"
    break;

  case 75:
#line 700 "f-exp.y"
    { type_stack->push (tp_reference); (yyval.voidval) = 0; }
#line 2295 "f-exp.c.tmp"
    break;

  case 76:
#line 702 "f-exp.y"
    { type_stack->push (tp_reference); (yyval.voidval) = (yyvsp[0].voidval); }
#line 2301 "f-exp.c.tmp"
    break;

  case 78:
#line 707 "f-exp.y"
    { (yyval.voidval) = (yyvsp[-1].voidval); }
#line 2307 "f-exp.c.tmp"
    break;

  case 79:
#line 709 "f-exp.y"
    { push_kind_type ((yyvsp[-1].typed_val).val, (yyvsp[-1].typed_val).type); }
#line 2313 "f-exp.c.tmp"
    break;

  case 80:
#line 711 "f-exp.y"
    { push_kind_type ((yyvsp[0].typed_val).val, (yyvsp[0].typed_val).type); }
#line 2319 "f-exp.c.tmp"
    break;

  case 81:
#line 713 "f-exp.y"
    { type_stack->push (tp_function); }
#line 2325 "f-exp.c.tmp"
    break;

  case 82:
#line 715 "f-exp.y"
    { type_stack->push (tp_function); }
#line 2331 "f-exp.c.tmp"
    break;

  case 83:
#line 719 "f-exp.y"
    { (yyval.voidval) = 0; }
#line 2337 "f-exp.c.tmp"
    break;

  case 84:
#line 721 "f-exp.y"
    { xfree ((yyvsp[-1].tvec)); (yyval.voidval) = 0; }
#line 2343 "f-exp.c.tmp"
    break;

  case 85:
#line 726 "f-exp.y"
    { (yyval.tval) = (yyvsp[0].tsym).type; }
#line 2349 "f-exp.c.tmp"
    break;

  case 86:
#line 728 "f-exp.y"
    { (yyval.tval) = parse_f_type (pstate)->builtin_integer_s1; }
#line 2355 "f-exp.c.tmp"
    break;

  case 87:
#line 730 "f-exp.y"
    { (yyval.tval) = parse_f_type (pstate)->builtin_integer_s2; }
#line 2361 "f-exp.c.tmp"
    break;

  case 88:
#line 732 "f-exp.y"
    { (yyval.tval) = parse_f_type (pstate)->builtin_integer; }
#line 2367 "f-exp.c.tmp"
    break;

  case 89:
#line 734 "f-exp.y"
    { (yyval.tval) = parse_f_type (pstate)->builtin_integer; }
#line 2373 "f-exp.c.tmp"
    break;

  case 90:
#line 736 "f-exp.y"
    { (yyval.tval) = parse_f_type (pstate)->builtin_integer_s8; }
#line 2379 "f-exp.c.tmp"
    break;

  case 91:
#line 738 "f-exp.y"
    { (yyval.tval) = parse_f_type (pstate)->builtin_character; }
#line 2385 "f-exp.c.tmp"
    break;

  case 92:
#line 740 "f-exp.y"
    { (yyval.tval) = parse_f_type (pstate)->builtin_logical_s1; }
#line 2391 "f-exp.c.tmp"
    break;

  case 93:
#line 742 "f-exp.y"
    { (yyval.tval) = parse_f_type (pstate)->builtin_logical_s2; }
#line 2397 "f-exp.c.tmp"
    break;

  case 94:
#line 744 "f-exp.y"
    { (yyval.tval) = parse_f_type (pstate)->builtin_logical; }
#line 2403 "f-exp.c.tmp"
    break;

  case 95:
#line 746 "f-exp.y"
    { (yyval.tval) = parse_f_type (pstate)->builtin_logical; }
#line 2409 "f-exp.c.tmp"
    break;

  case 96:
#line 748 "f-exp.y"
    { (yyval.tval) = parse_f_type (pstate)->builtin_logical_s8; }
#line 2415 "f-exp.c.tmp"
    break;

  case 97:
#line 750 "f-exp.y"
    { (yyval.tval) = parse_f_type (pstate)->builtin_real; }
#line 2421 "f-exp.c.tmp"
    break;

  case 98:
#line 752 "f-exp.y"
    { (yyval.tval) = parse_f_type (pstate)->builtin_real; }
#line 2427 "f-exp.c.tmp"
    break;

  case 99:
#line 754 "f-exp.y"
    { (yyval.tval) = parse_f_type (pstate)->builtin_real_s8; }
#line 2433 "f-exp.c.tmp"
    break;

  case 100:
#line 756 "f-exp.y"
    { (yyval.tval) = parse_f_type (pstate)->builtin_real_s16;
			  if ((yyval.tval)->code () == TYPE_CODE_ERROR)
			    error (_("unsupported type %s"),
				   (yyval.tval)->safe_name ());
			}
#line 2443 "f-exp.c.tmp"
    break;

  case 101:
#line 762 "f-exp.y"
    { (yyval.tval) = parse_f_type (pstate)->builtin_complex; }
#line 2449 "f-exp.c.tmp"
    break;

  case 102:
#line 764 "f-exp.y"
    { (yyval.tval) = parse_f_type (pstate)->builtin_complex; }
#line 2455 "f-exp.c.tmp"
    break;

  case 103:
#line 766 "f-exp.y"
    { (yyval.tval) = parse_f_type (pstate)->builtin_complex_s8; }
#line 2461 "f-exp.c.tmp"
    break;

  case 104:
#line 768 "f-exp.y"
    { (yyval.tval) = parse_f_type (pstate)->builtin_complex_s16;
			  if ((yyval.tval)->code () == TYPE_CODE_ERROR)
			    error (_("unsupported type %s"),
				   (yyval.tval)->safe_name ());
			}
#line 2471 "f-exp.c.tmp"
    break;

  case 105:
#line 774 "f-exp.y"
    { (yyval.tval) = parse_f_type (pstate)->builtin_real;}
#line 2477 "f-exp.c.tmp"
    break;

  case 106:
#line 776 "f-exp.y"
    { (yyval.tval) = parse_f_type (pstate)->builtin_real_s8;}
#line 2483 "f-exp.c.tmp"
    break;

  case 107:
#line 778 "f-exp.y"
    { (yyval.tval) = parse_f_type (pstate)->builtin_complex;}
#line 2489 "f-exp.c.tmp"
    break;

  case 108:
#line 780 "f-exp.y"
    { (yyval.tval) = parse_f_type (pstate)->builtin_complex_s8;}
#line 2495 "f-exp.c.tmp"
    break;

  case 109:
#line 785 "f-exp.y"
    { (yyval.tvec) = (struct type **) xmalloc (sizeof (struct type *) * 2);
		  (yyval.ivec)[0] = 1;	/* Number of types in vector */
		  (yyval.tvec)[1] = (yyvsp[0].tval);
		}
#line 2504 "f-exp.c.tmp"
    break;

  case 110:
#line 790 "f-exp.y"
    { int len = sizeof (struct type *) * (++((yyvsp[-2].ivec)[0]) + 1);
		  (yyval.tvec) = (struct type **) xrealloc ((char *) (yyvsp[-2].tvec), len);
		  (yyval.tvec)[(yyval.ivec)[0]] = (yyvsp[0].tval);
		}
#line 2513 "f-exp.c.tmp"
    break;

  case 111:
#line 798 "f-exp.y"
    { (yyval.sval) = (yyvsp[0].ssym).stoken; }
#line 2519 "f-exp.c.tmp"
    break;

  case 112:
#line 800 "f-exp.y"
    { (yyval.sval) = (yyvsp[0].tsym).stoken; }
#line 2525 "f-exp.c.tmp"
    break;


#line 2529 "f-exp.c.tmp"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 813 "f-exp.y"


/* Called to match intrinsic function calls with one argument to their
   respective implementation and push the operation.  */

static void
wrap_unop_intrinsic (exp_opcode code)
{
  switch (code)
    {
    case UNOP_ABS:
      pstate->wrap<fortran_abs_operation> ();
      break;
    case FORTRAN_FLOOR:
      pstate->wrap<fortran_floor_operation_1arg> ();
      break;
    case FORTRAN_CEILING:
      pstate->wrap<fortran_ceil_operation_1arg> ();
      break;
    case UNOP_FORTRAN_ALLOCATED:
      pstate->wrap<fortran_allocated_operation> ();
      break;
    case UNOP_FORTRAN_RANK:
      pstate->wrap<fortran_rank_operation> ();
      break;
    case UNOP_FORTRAN_SHAPE:
      pstate->wrap<fortran_array_shape_operation> ();
      break;
    case UNOP_FORTRAN_LOC:
      pstate->wrap<fortran_loc_operation> ();
      break;
    case FORTRAN_ASSOCIATED:
      pstate->wrap<fortran_associated_1arg> ();
      break;
    case FORTRAN_ARRAY_SIZE:
      pstate->wrap<fortran_array_size_1arg> ();
      break;
    case FORTRAN_CMPLX:
      pstate->wrap<fortran_cmplx_operation_1arg> ();
      break;
    case FORTRAN_LBOUND:
    case FORTRAN_UBOUND:
      pstate->push_new<fortran_bound_1arg> (code, pstate->pop ());
      break;
    default:
      gdb_assert_not_reached ("unhandled intrinsic");
    }
}

/* Called to match intrinsic function calls with two arguments to their
   respective implementation and push the operation.  */

static void
wrap_binop_intrinsic (exp_opcode code)
{
  switch (code)
    {
    case FORTRAN_FLOOR:
      fortran_wrap2_kind<fortran_floor_operation_2arg>
	(parse_f_type (pstate)->builtin_integer);
      break;
    case FORTRAN_CEILING:
      fortran_wrap2_kind<fortran_ceil_operation_2arg>
	(parse_f_type (pstate)->builtin_integer);
      break;
    case BINOP_MOD:
      pstate->wrap2<fortran_mod_operation> ();
      break;
    case BINOP_FORTRAN_MODULO:
      pstate->wrap2<fortran_modulo_operation> ();
      break;
    case FORTRAN_CMPLX:
      pstate->wrap2<fortran_cmplx_operation_2arg> ();
      break;
    case FORTRAN_ASSOCIATED:
      pstate->wrap2<fortran_associated_2arg> ();
      break;
    case FORTRAN_ARRAY_SIZE:
      pstate->wrap2<fortran_array_size_2arg> ();
      break;
    case FORTRAN_LBOUND:
    case FORTRAN_UBOUND:
      {
	operation_up arg2 = pstate->pop ();
	operation_up arg1 = pstate->pop ();
	pstate->push_new<fortran_bound_2arg> (code, std::move (arg1),
					      std::move (arg2));
      }
      break;
    default:
      gdb_assert_not_reached ("unhandled intrinsic");
    }
}

/* Called to match intrinsic function calls with three arguments to their
   respective implementation and push the operation.  */

static void
wrap_ternop_intrinsic (exp_opcode code)
{
  switch (code)
    {
    case FORTRAN_LBOUND:
    case FORTRAN_UBOUND:
      {
	operation_up kind_arg = pstate->pop ();
	operation_up arg2 = pstate->pop ();
	operation_up arg1 = pstate->pop ();

	value *val = kind_arg->evaluate (nullptr, pstate->expout.get (),
					 EVAL_AVOID_SIDE_EFFECTS);
	gdb_assert (val != nullptr);

	type *follow_type
	  = convert_to_kind_type (parse_f_type (pstate)->builtin_integer,
				  value_as_long (val));

	pstate->push_new<fortran_bound_3arg> (code, std::move (arg1),
					      std::move (arg2), follow_type);
      }
      break;
    case FORTRAN_ARRAY_SIZE:
      fortran_wrap3_kind<fortran_array_size_3arg>
	(parse_f_type (pstate)->builtin_integer);
      break;
    case FORTRAN_CMPLX:
      fortran_wrap3_kind<fortran_cmplx_operation_3arg>
	(parse_f_type (pstate)->builtin_complex);
      break;
    default:
      gdb_assert_not_reached ("unhandled intrinsic");
    }
}

/* A helper that pops two operations (similar to wrap2), evaluates the last one
   assuming it is a kind parameter, and wraps them in some other operation
   pushing it to the stack.  */

template<typename T>
static void
fortran_wrap2_kind (type *base_type)
{
  operation_up kind_arg = pstate->pop ();
  operation_up arg = pstate->pop ();

  value *val = kind_arg->evaluate (nullptr, pstate->expout.get (),
				   EVAL_AVOID_SIDE_EFFECTS);
  gdb_assert (val != nullptr);

  type *follow_type = convert_to_kind_type (base_type, value_as_long (val));

  pstate->push_new<T> (std::move (arg), follow_type);
}

/* A helper that pops three operations, evaluates the last one assuming it is a
   kind parameter, and wraps them in some other operation pushing it to the
   stack.  */

template<typename T>
static void
fortran_wrap3_kind (type *base_type)
{
  operation_up kind_arg = pstate->pop ();
  operation_up arg2 = pstate->pop ();
  operation_up arg1 = pstate->pop ();

  value *val = kind_arg->evaluate (nullptr, pstate->expout.get (),
				   EVAL_AVOID_SIDE_EFFECTS);
  gdb_assert (val != nullptr);

  type *follow_type = convert_to_kind_type (base_type, value_as_long (val));

  pstate->push_new<T> (std::move (arg1), std::move (arg2), follow_type);
}

/* Take care of parsing a number (anything that starts with a digit).
   Set yylval and return the token type; update lexptr.
   LEN is the number of characters in it.  */

/*** Needs some error checking for the float case ***/

static int
parse_number (struct parser_state *par_state,
	      const char *p, int len, int parsed_float, f_exp_YYSTYPE *putithere)
{
  ULONGEST n = 0;
  ULONGEST prevn = 0;
  int c;
  int base = input_radix;
  int unsigned_p = 0;
  int long_p = 0;
  ULONGEST high_bit;
  struct type *signed_type;
  struct type *unsigned_type;

  if (parsed_float)
    {
      /* It's a float since it contains a point or an exponent.  */
      /* [dD] is not understood as an exponent by parse_float,
	 change it to 'e'.  */
      char *tmp, *tmp2;

      tmp = xstrdup (p);
      for (tmp2 = tmp; *tmp2; ++tmp2)
	if (*tmp2 == 'd' || *tmp2 == 'D')
	  *tmp2 = 'e';

      /* FIXME: Should this use different types?  */
      putithere->typed_val_float.type = parse_f_type (pstate)->builtin_real_s8;
      bool parsed = parse_float (tmp, len,
				 putithere->typed_val_float.type,
				 putithere->typed_val_float.val);
      xfree (tmp);
      return parsed? FLOAT : ERROR;
    }

  /* Handle base-switching prefixes 0x, 0t, 0d, 0 */
  if (p[0] == '0' && len > 1)
    switch (p[1])
      {
      case 'x':
      case 'X':
	if (len >= 3)
	  {
	    p += 2;
	    base = 16;
	    len -= 2;
	  }
	break;

      case 't':
      case 'T':
      case 'd':
      case 'D':
	if (len >= 3)
	  {
	    p += 2;
	    base = 10;
	    len -= 2;
	  }
	break;

      default:
	base = 8;
	break;
      }

  while (len-- > 0)
    {
      c = *p++;
      if (c_isupper (c))
	c = c_tolower (c);
      if (len == 0 && c == 'l')
	long_p = 1;
      else if (len == 0 && c == 'u')
	unsigned_p = 1;
      else
	{
	  int i;
	  if (c >= '0' && c <= '9')
	    i = c - '0';
	  else if (c >= 'a' && c <= 'f')
	    i = c - 'a' + 10;
	  else
	    return ERROR;	/* Char not a digit */
	  if (i >= base)
	    return ERROR;		/* Invalid digit in this base */
	  n *= base;
	  n += i;
	}
      /* Test for overflow.  */
      if (prevn == 0 && n == 0)
	;
      else if (RANGE_CHECK && prevn >= n)
	range_error (_("Overflow on numeric constant."));
      prevn = n;
    }

  /* If the number is too big to be an int, or it's got an l suffix
     then it's a long.  Work out if this has to be a long by
     shifting right and seeing if anything remains, and the
     target int size is different to the target long size.

     In the expression below, we could have tested
     (n >> gdbarch_int_bit (parse_gdbarch))
     to see if it was zero,
     but too many compilers warn about that, when ints and longs
     are the same size.  So we shift it twice, with fewer bits
     each time, for the same result.  */

  int bits_available;
  if ((gdbarch_int_bit (par_state->gdbarch ())
       != gdbarch_long_bit (par_state->gdbarch ())
       && ((n >> 2)
	   >> (gdbarch_int_bit (par_state->gdbarch ())-2))) /* Avoid
							    shift warning */
      || long_p)
    {
      bits_available = gdbarch_long_bit (par_state->gdbarch ());
      unsigned_type = parse_type (par_state)->builtin_unsigned_long;
      signed_type = parse_type (par_state)->builtin_long;
  }
  else
    {
      bits_available = gdbarch_int_bit (par_state->gdbarch ());
      unsigned_type = parse_type (par_state)->builtin_unsigned_int;
      signed_type = parse_type (par_state)->builtin_int;
    }
  high_bit = ((ULONGEST)1) << (bits_available - 1);

  if (RANGE_CHECK
      && ((n >> 2) >> (bits_available - 2)))
    range_error (_("Overflow on numeric constant."));

  putithere->typed_val.val = n;

  /* If the high bit of the worked out type is set then this number
     has to be unsigned.  */

  if (unsigned_p || (n & high_bit))
    putithere->typed_val.type = unsigned_type;
  else
    putithere->typed_val.type = signed_type;

  return INT;
}

/* Called to setup the type stack when we encounter a '(kind=N)' type
   modifier, performs some bounds checking on 'N' and then pushes this to
   the type stack followed by the 'tp_kind' marker.  */
static void
push_kind_type (LONGEST val, struct type *type)
{
  int ival;

  if (type->is_unsigned ())
    {
      ULONGEST uval = static_cast <ULONGEST> (val);
      if (uval > INT_MAX)
	error (_("kind value out of range"));
      ival = static_cast <int> (uval);
    }
  else
    {
      if (val > INT_MAX || val < 0)
	error (_("kind value out of range"));
      ival = static_cast <int> (val);
    }

  type_stack->push (tp_kind, ival);
}

/* Helper function for convert_to_kind_type.  */
static struct type *
convert_to_kind_type_1 (struct type *basetype, int kind)
{
  if (basetype == parse_f_type (pstate)->builtin_character)
    {
      /* Character of kind 1 is a special case, this is the same as the
	 base character type.  */
      if (kind == 1)
	return parse_f_type (pstate)->builtin_character;
    }
  else if (basetype == parse_f_type (pstate)->builtin_complex)
    {
      if (kind == 4)
	return parse_f_type (pstate)->builtin_complex;
      else if (kind == 8)
	return parse_f_type (pstate)->builtin_complex_s8;
      else if (kind == 16)
	return parse_f_type (pstate)->builtin_complex_s16;
    }
  else if (basetype == parse_f_type (pstate)->builtin_real)
    {
      if (kind == 4)
	return parse_f_type (pstate)->builtin_real;
      else if (kind == 8)
	return parse_f_type (pstate)->builtin_real_s8;
      else if (kind == 16)
	return parse_f_type (pstate)->builtin_real_s16;
    }
  else if (basetype == parse_f_type (pstate)->builtin_logical)
    {
      if (kind == 1)
	return parse_f_type (pstate)->builtin_logical_s1;
      else if (kind == 2)
	return parse_f_type (pstate)->builtin_logical_s2;
      else if (kind == 4)
	return parse_f_type (pstate)->builtin_logical;
      else if (kind == 8)
	return parse_f_type (pstate)->builtin_logical_s8;
    }
  else if (basetype == parse_f_type (pstate)->builtin_integer)
    {
      if (kind == 1)
	return parse_f_type (pstate)->builtin_integer_s1;
      else if (kind == 2)
	return parse_f_type (pstate)->builtin_integer_s2;
      else if (kind == 4)
	return parse_f_type (pstate)->builtin_integer;
      else if (kind == 8)
	return parse_f_type (pstate)->builtin_integer_s8;
    }

  return nullptr;
}

/* Called when a type has a '(kind=N)' modifier after it, for example
   'character(kind=1)'.  The BASETYPE is the type described by 'character'
   in our example, and KIND is the integer '1'.  This function returns a
   new type that represents the basetype of a specific kind.  */
static struct type *
convert_to_kind_type (struct type *basetype, int kind)
{
  struct type *res = convert_to_kind_type_1 (basetype, kind);

  if (res == nullptr || res->code () == TYPE_CODE_ERROR)
    error (_("unsupported kind %d for type %s"),
	   kind, basetype->safe_name ());

  return res;
}

struct f_token
{
  /* The string to match against.  */
  const char *oper;

  /* The lexer token to return.  */
  int token;

  /* The expression opcode to embed within the token.  */
  enum exp_opcode opcode;

  /* When this is true the string in OPER is matched exactly including
     case, when this is false OPER is matched case insensitively.  */
  bool case_sensitive;
};

/* List of Fortran operators.  */

static const struct f_token fortran_operators[] =
{
  { ".and.", BOOL_AND, OP_NULL, false },
  { ".or.", BOOL_OR, OP_NULL, false },
  { ".not.", BOOL_NOT, OP_NULL, false },
  { ".eq.", EQUAL, OP_NULL, false },
  { ".eqv.", EQUAL, OP_NULL, false },
  { ".neqv.", NOTEQUAL, OP_NULL, false },
  { ".xor.", NOTEQUAL, OP_NULL, false },
  { "==", EQUAL, OP_NULL, false },
  { ".ne.", NOTEQUAL, OP_NULL, false },
  { "/=", NOTEQUAL, OP_NULL, false },
  { ".le.", LEQ, OP_NULL, false },
  { "<=", LEQ, OP_NULL, false },
  { ".ge.", GEQ, OP_NULL, false },
  { ">=", GEQ, OP_NULL, false },
  { ".gt.", GREATERTHAN, OP_NULL, false },
  { ">", GREATERTHAN, OP_NULL, false },
  { ".lt.", LESSTHAN, OP_NULL, false },
  { "<", LESSTHAN, OP_NULL, false },
  { "**", STARSTAR, BINOP_EXP, false },
};

/* Holds the Fortran representation of a boolean, and the integer value we
   substitute in when one of the matching strings is parsed.  */
struct f77_boolean_val
{
  /* The string representing a Fortran boolean.  */
  const char *name;

  /* The integer value to replace it with.  */
  int value;
};

/* The set of Fortran booleans.  These are matched case insensitively.  */
static const struct f77_boolean_val boolean_values[]  =
{
  { ".true.", 1 },
  { ".false.", 0 }
};

static const struct f_token f_intrinsics[] =
{
  /* The following correspond to actual functions in Fortran and are case
     insensitive.  */
  { "kind", KIND, OP_NULL, false },
  { "abs", UNOP_INTRINSIC, UNOP_ABS, false },
  { "mod", BINOP_INTRINSIC, BINOP_MOD, false },
  { "floor", UNOP_OR_BINOP_INTRINSIC, FORTRAN_FLOOR, false },
  { "ceiling", UNOP_OR_BINOP_INTRINSIC, FORTRAN_CEILING, false },
  { "modulo", BINOP_INTRINSIC, BINOP_FORTRAN_MODULO, false },
  { "cmplx", UNOP_OR_BINOP_OR_TERNOP_INTRINSIC, FORTRAN_CMPLX, false },
  { "lbound", UNOP_OR_BINOP_OR_TERNOP_INTRINSIC, FORTRAN_LBOUND, false },
  { "ubound", UNOP_OR_BINOP_OR_TERNOP_INTRINSIC, FORTRAN_UBOUND, false },
  { "allocated", UNOP_INTRINSIC, UNOP_FORTRAN_ALLOCATED, false },
  { "associated", UNOP_OR_BINOP_INTRINSIC, FORTRAN_ASSOCIATED, false },
  { "rank", UNOP_INTRINSIC, UNOP_FORTRAN_RANK, false },
  { "size", UNOP_OR_BINOP_OR_TERNOP_INTRINSIC, FORTRAN_ARRAY_SIZE, false },
  { "shape", UNOP_INTRINSIC, UNOP_FORTRAN_SHAPE, false },
  { "loc", UNOP_INTRINSIC, UNOP_FORTRAN_LOC, false },
  { "sizeof", SIZEOF, OP_NULL, false },
};

static const f_token f_keywords[] =
{
  /* Historically these have always been lowercase only in GDB.  */
  { "character", CHARACTER, OP_NULL, true },
  { "complex", COMPLEX_KEYWORD, OP_NULL, true },
  { "complex_4", COMPLEX_S4_KEYWORD, OP_NULL, true },
  { "complex_8", COMPLEX_S8_KEYWORD, OP_NULL, true },
  { "complex_16", COMPLEX_S16_KEYWORD, OP_NULL, true },
  { "integer_1", INT_S1_KEYWORD, OP_NULL, true },
  { "integer_2", INT_S2_KEYWORD, OP_NULL, true },
  { "integer_4", INT_S4_KEYWORD, OP_NULL, true },
  { "integer", INT_KEYWORD, OP_NULL, true },
  { "integer_8", INT_S8_KEYWORD, OP_NULL, true },
  { "logical_1", LOGICAL_S1_KEYWORD, OP_NULL, true },
  { "logical_2", LOGICAL_S2_KEYWORD, OP_NULL, true },
  { "logical", LOGICAL_KEYWORD, OP_NULL, true },
  { "logical_4", LOGICAL_S4_KEYWORD, OP_NULL, true },
  { "logical_8", LOGICAL_S8_KEYWORD, OP_NULL, true },
  { "real", REAL_KEYWORD, OP_NULL, true },
  { "real_4", REAL_S4_KEYWORD, OP_NULL, true },
  { "real_8", REAL_S8_KEYWORD, OP_NULL, true },
  { "real_16", REAL_S16_KEYWORD, OP_NULL, true },
  { "single", SINGLE, OP_NULL, true },
  { "double", DOUBLE, OP_NULL, true },
  { "precision", PRECISION, OP_NULL, true },
};

/* Implementation of a dynamically expandable buffer for processing input
   characters acquired through lexptr and building a value to return in
   yylval.  Ripped off from ch-exp.y */

static char *tempbuf;		/* Current buffer contents */
static int tempbufsize;		/* Size of allocated buffer */
static int tempbufindex;	/* Current index into buffer */

#define GROWBY_MIN_SIZE 64	/* Minimum amount to grow buffer by */

#define CHECKBUF(size) \
  do { \
    if (tempbufindex + (size) >= tempbufsize) \
      { \
	growbuf_by_size (size); \
      } \
  } while (0);


/* Grow the static temp buffer if necessary, including allocating the
   first one on demand.  */

static void
growbuf_by_size (int count)
{
  int growby;

  growby = std::max (count, GROWBY_MIN_SIZE);
  tempbufsize += growby;
  if (tempbuf == NULL)
    tempbuf = (char *) xmalloc (tempbufsize);
  else
    tempbuf = (char *) xrealloc (tempbuf, tempbufsize);
}

/* Blatantly ripped off from ch-exp.y. This routine recognizes F77
   string-literals.

   Recognize a string literal.  A string literal is a nonzero sequence
   of characters enclosed in matching single quotes, except that
   a single character inside single quotes is a character literal, which
   we reject as a string literal.  To embed the terminator character inside
   a string, it is simply doubled (I.E. 'this''is''one''string') */

static int
match_string_literal (void)
{
  const char *tokptr = pstate->lexptr;

  for (tempbufindex = 0, tokptr++; *tokptr != '\0'; tokptr++)
    {
      CHECKBUF (1);
      if (*tokptr == *pstate->lexptr)
	{
	  if (*(tokptr + 1) == *pstate->lexptr)
	    tokptr++;
	  else
	    break;
	}
      tempbuf[tempbufindex++] = *tokptr;
    }
  if (*tokptr == '\0'					/* no terminator */
      || tempbufindex == 0)				/* no string */
    return 0;
  else
    {
      tempbuf[tempbufindex] = '\0';
      yylval.sval.ptr = tempbuf;
      yylval.sval.length = tempbufindex;
      pstate->lexptr = ++tokptr;
      return STRING_LITERAL;
    }
}

/* This is set if a NAME token appeared at the very end of the input
   string, with no whitespace separating the name from the EOF.  This
   is used only when parsing to do field name completion.  */
static bool saw_name_at_eof;

/* This is set if the previously-returned token was a structure
   operator '%'.  */
static bool last_was_structop;

/* Read one token, getting characters through lexptr.  */

static int
yylex (void)
{
  int c;
  int namelen;
  unsigned int token;
  const char *tokstart;
  bool saw_structop = last_was_structop;

  last_was_structop = false;

 retry:

  pstate->prev_lexptr = pstate->lexptr;

  tokstart = pstate->lexptr;

  /* First of all, let us make sure we are not dealing with the
     special tokens .true. and .false. which evaluate to 1 and 0.  */

  if (*pstate->lexptr == '.')
    {
      for (const auto &candidate : boolean_values)
	{
	  if (strncasecmp (tokstart, candidate.name,
			   strlen (candidate.name)) == 0)
	    {
	      pstate->lexptr += strlen (candidate.name);
	      yylval.lval = candidate.value;
	      return BOOLEAN_LITERAL;
	    }
	}
    }

  /* See if it is a Fortran operator.  */
  for (const auto &candidate : fortran_operators)
    if (strncasecmp (tokstart, candidate.oper,
		     strlen (candidate.oper)) == 0)
      {
	gdb_assert (!candidate.case_sensitive);
	pstate->lexptr += strlen (candidate.oper);
	yylval.opcode = candidate.opcode;
	return candidate.token;
      }

  switch (c = *tokstart)
    {
    case 0:
      if (saw_name_at_eof)
	{
	  saw_name_at_eof = false;
	  return COMPLETE;
	}
      else if (pstate->parse_completion && saw_structop)
	return COMPLETE;
      return 0;

    case ' ':
    case '\t':
    case '\n':
      pstate->lexptr++;
      goto retry;

    case '\'':
      token = match_string_literal ();
      if (token != 0)
	return (token);
      break;

    case '(':
      paren_depth++;
      pstate->lexptr++;
      return c;

    case ')':
      if (paren_depth == 0)
	return 0;
      paren_depth--;
      pstate->lexptr++;
      return c;

    case ',':
      if (pstate->comma_terminates && paren_depth == 0)
	return 0;
      pstate->lexptr++;
      return c;

    case '.':
      /* Might be a floating point number.  */
      if (pstate->lexptr[1] < '0' || pstate->lexptr[1] > '9')
	goto symbol;		/* Nope, must be a symbol.  */
      [[fallthrough]];

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      {
	/* It's a number.  */
	int got_dot = 0, got_e = 0, got_d = 0, toktype;
	const char *p = tokstart;
	int hex = input_radix > 10;

	if (c == '0' && (p[1] == 'x' || p[1] == 'X'))
	  {
	    p += 2;
	    hex = 1;
	  }
	else if (c == '0' && (p[1]=='t' || p[1]=='T'
			      || p[1]=='d' || p[1]=='D'))
	  {
	    p += 2;
	    hex = 0;
	  }

	for (;; ++p)
	  {
	    if (!hex && !got_e && (*p == 'e' || *p == 'E'))
	      got_dot = got_e = 1;
	    else if (!hex && !got_d && (*p == 'd' || *p == 'D'))
	      got_dot = got_d = 1;
	    else if (!hex && !got_dot && *p == '.')
	      got_dot = 1;
	    else if (((got_e && (p[-1] == 'e' || p[-1] == 'E'))
		     || (got_d && (p[-1] == 'd' || p[-1] == 'D')))
		     && (*p == '-' || *p == '+'))
	      /* This is the sign of the exponent, not the end of the
		 number.  */
	      continue;
	    /* We will take any letters or digits.  parse_number will
	       complain if past the radix, or if L or U are not final.  */
	    else if ((*p < '0' || *p > '9')
		     && ((*p < 'a' || *p > 'z')
			 && (*p < 'A' || *p > 'Z')))
	      break;
	  }
	toktype = parse_number (pstate, tokstart, p - tokstart,
				got_dot|got_e|got_d,
				&yylval);
	if (toktype == ERROR)
	  error (_("Invalid number \"%.*s\"."), (int) (p - tokstart),
		 tokstart);
	pstate->lexptr = p;
	return toktype;
      }

    case '%':
      last_was_structop = true;
      [[fallthrough]];
    case '+':
    case '-':
    case '*':
    case '/':
    case '|':
    case '&':
    case '^':
    case '~':
    case '!':
    case '@':
    case '<':
    case '>':
    case '[':
    case ']':
    case '?':
    case ':':
    case '=':
    case '{':
    case '}':
    symbol:
      pstate->lexptr++;
      return c;
    }

  if (!(c == '_' || c == '$' || c ==':'
	|| (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
    /* We must have come across a bad character (e.g. ';').  */
    error (_("Invalid character '%c' in expression."), c);

  namelen = 0;
  for (c = tokstart[namelen];
       (c == '_' || c == '$' || c == ':' || (c >= '0' && c <= '9')
	|| (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
       c = tokstart[++namelen]);

  /* The token "if" terminates the expression and is NOT
     removed from the input stream.  */

  if (namelen == 2 && tokstart[0] == 'i' && tokstart[1] == 'f')
    return 0;

  pstate->lexptr += namelen;

  /* Catch specific keywords.  */

  for (const auto &keyword : f_keywords)
    if (strlen (keyword.oper) == namelen
	&& ((!keyword.case_sensitive
	     && strncasecmp (tokstart, keyword.oper, namelen) == 0)
	    || (keyword.case_sensitive
		&& strncmp (tokstart, keyword.oper, namelen) == 0)))
      {
	yylval.opcode = keyword.opcode;
	return keyword.token;
      }

  yylval.sval.ptr = tokstart;
  yylval.sval.length = namelen;

  if (*tokstart == '$')
    return DOLLAR_VARIABLE;

  /* Use token-type TYPENAME for symbols that happen to be defined
     currently as names of types; NAME for other symbols.
     The caller is not constrained to care about the distinction.  */
  {
    std::string tmp = copy_name (yylval.sval);
    struct block_symbol result;
    const domain_search_flags lookup_domains[] =
    {
      SEARCH_VFT,
      SEARCH_STRUCT_DOMAIN,
      SEARCH_MODULE_DOMAIN
    };
    int hextype;

    for (const auto &domain : lookup_domains)
      {
	result = lookup_symbol (tmp.c_str (), pstate->expression_context_block,
				domain, NULL);
	if (result.symbol && result.symbol->loc_class () == LOC_TYPEDEF)
	  {
	    yylval.tsym.type = result.symbol->type ();
	    return TYPENAME;
	  }

	if (result.symbol)
	  break;
      }

    yylval.tsym.type
      = language_lookup_primitive_type (pstate->language (),
					pstate->gdbarch (), tmp.c_str ());
    if (yylval.tsym.type != NULL)
      return TYPENAME;

    /* This is post the symbol search as symbols can hide intrinsics.  Also,
       give Fortran intrinsics priority over C symbols.  This prevents
       non-Fortran symbols from hiding intrinsics, for example abs.  */
    if (!result.symbol || result.symbol->language () != language_fortran)
      for (const auto &intrinsic : f_intrinsics)
	{
	  gdb_assert (!intrinsic.case_sensitive);
	  if (strlen (intrinsic.oper) == namelen
	      && strncasecmp (tokstart, intrinsic.oper, namelen) == 0)
	    {
	      yylval.opcode = intrinsic.opcode;
	      return intrinsic.token;
	    }
	}

    /* Input names that aren't symbols but ARE valid hex numbers,
       when the input radix permits them, can be names or numbers
       depending on the parse.  Note we support radixes > 16 here.  */
    if (!result.symbol
	&& ((tokstart[0] >= 'a' && tokstart[0] < 'a' + input_radix - 10)
	    || (tokstart[0] >= 'A' && tokstart[0] < 'A' + input_radix - 10)))
      {
	f_exp_YYSTYPE newlval;	/* Its value is ignored.  */
	hextype = parse_number (pstate, tokstart, namelen, 0, &newlval);
	if (hextype == INT)
	  {
	    yylval.ssym.sym = result;
	    yylval.ssym.is_a_field_of_this = false;
	    return NAME_OR_INT;
	  }
      }

    if (pstate->parse_completion && *pstate->lexptr == '\0')
      saw_name_at_eof = true;

    /* Any other kind of symbol */
    yylval.ssym.sym = result;
    yylval.ssym.is_a_field_of_this = false;
    return NAME;
  }
}

int
f_language::parser (struct parser_state *par_state) const
{
  /* Setting up the parser state.  */
  scoped_restore pstate_restore = make_scoped_restore (&pstate);
  scoped_restore restore_yydebug = make_scoped_restore (&yydebug,
							par_state->debug);
  gdb_assert (par_state != NULL);
  pstate = par_state;
  last_was_structop = false;
  saw_name_at_eof = false;
  paren_depth = 0;

  struct type_stack stack;
  scoped_restore restore_type_stack = make_scoped_restore (&type_stack,
							   &stack);

  int result = yyparse ();
  if (!result)
    pstate->set_operation (pstate->pop ());
  return result;
}

static void
yyerror (const char *msg)
{
  pstate->parse_error (msg);
}
