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
#line 38 "m2-exp.y"


#include "expression.h"
#include "language.h"
#include "value.h"
#include "parser-defs.h"
#include "m2-lang.h"
#include "block.h"
#include "m2-exp.h"

#define parse_type(ps) builtin_type (ps->gdbarch ())
#define parse_m2_type(ps) builtin_m2_type (ps->gdbarch ())

/* Remap normal yacc parser interface names (yyparse, yylex, yyerror,
   etc).  */
#define GDB_YY_REMAP_PREFIX m2_
#include "yy-remap.h"

/* The state of the parser, used internally when we are parsing the
   expression.  */

static struct parser_state *pstate = NULL;

int yyparse (void);

static int yylex (void);

static void yyerror (const char *);

static int parse_number (int);

/* The sign of the number being parsed.  */
static int number_sign = 1;

using namespace expr;

#line 107 "m2-exp.c.tmp"

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
    HEX = 259,
    ERROR = 260,
    UINT = 261,
    M2_TRUE = 262,
    M2_FALSE = 263,
    CHAR = 264,
    FLOAT = 265,
    STRING = 266,
    NAME = 267,
    BLOCKNAME = 268,
    IDENT = 269,
    VARNAME = 270,
    TYPENAME = 271,
    SIZE = 272,
    CAP = 273,
    ORD = 274,
    HIGH = 275,
    ABS = 276,
    MIN_FUNC = 277,
    MAX_FUNC = 278,
    FLOAT_FUNC = 279,
    VAL = 280,
    CHR = 281,
    ODD = 282,
    TRUNC = 283,
    TSIZE = 284,
    ADR = 285,
    INC = 286,
    DEC = 287,
    INCL = 288,
    EXCL = 289,
    COLONCOLON = 290,
    DOLLAR_VARIABLE = 291,
    ABOVE_COMMA = 292,
    ASSIGN = 293,
    LEQ = 294,
    GEQ = 295,
    NOTEQUAL = 296,
    IN = 297,
    OROR = 298,
    LOGICAL_AND = 299,
    DIV = 300,
    MOD = 301,
    UNARY = 302,
    DOT = 303,
    NOT = 304,
    QID = 305
  };
#endif
/* Tokens.  */
#define INT 258
#define HEX 259
#define ERROR 260
#define UINT 261
#define M2_TRUE 262
#define M2_FALSE 263
#define CHAR 264
#define FLOAT 265
#define STRING 266
#define NAME 267
#define BLOCKNAME 268
#define IDENT 269
#define VARNAME 270
#define TYPENAME 271
#define SIZE 272
#define CAP 273
#define ORD 274
#define HIGH 275
#define ABS 276
#define MIN_FUNC 277
#define MAX_FUNC 278
#define FLOAT_FUNC 279
#define VAL 280
#define CHR 281
#define ODD 282
#define TRUNC 283
#define TSIZE 284
#define ADR 285
#define INC 286
#define DEC 287
#define INCL 288
#define EXCL 289
#define COLONCOLON 290
#define DOLLAR_VARIABLE 291
#define ABOVE_COMMA 292
#define ASSIGN 293
#define LEQ 294
#define GEQ 295
#define NOTEQUAL 296
#define IN 297
#define OROR 298
#define LOGICAL_AND 299
#define DIV 300
#define MOD 301
#define UNARY 302
#define DOT 303
#define NOT 304
#define QID 305

/* Value type.  */
#if ! defined m2_exp_YYSTYPE && ! defined m2_exp_YYSTYPE_IS_DECLARED
union m2_exp_YYSTYPE
{
#line 80 "m2-exp.y"

    LONGEST lval;
    ULONGEST ulval;
    gdb_byte val[16];
    struct symbol *sym;
    struct type *tval;
    struct stoken sval;
    int voidval;
    const struct block *bval;
    enum exp_opcode opcode;
    struct internalvar *ivar;

    struct type **tvec;
    int *ivec;
  

#line 264 "m2-exp.c.tmp"

};
typedef union m2_exp_YYSTYPE m2_exp_YYSTYPE;
# define m2_exp_YYSTYPE_IS_TRIVIAL 1
# define m2_exp_YYSTYPE_IS_DECLARED 1
#endif


extern m2_exp_YYSTYPE yylval;

int yyparse (void);





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
         || (defined m2_exp_YYSTYPE_IS_TRIVIAL && m2_exp_YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union m2_exp_yyalloc
{
  yytype_int16 yyss_alloc;
  m2_exp_YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union m2_exp_yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (m2_exp_YYSTYPE)) \
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
#define YYFINAL  71
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   924

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  70
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  15
/* YYNRULES -- Number of rules.  */
#define YYNRULES  82
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  189

#define YYUNDEFTOK  2
#define YYMAXUTOK   305

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
       2,     2,     2,     2,     2,    46,     2,     2,    50,     2,
      62,    66,    54,    52,    37,    53,     2,    55,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      40,    44,    41,     2,    51,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    61,     2,    69,    59,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    67,     2,    68,    64,     2,     2,     2,
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
      35,    36,    38,    39,    42,    43,    45,    47,    48,    49,
      56,    57,    58,    60,    63,    65
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   149,   149,   150,   153,   159,   164,   163,   170,   174,
     178,   179,   182,   186,   190,   194,   198,   202,   206,   210,
     214,   218,   222,   226,   230,   234,   239,   243,   252,   256,
     265,   272,   275,   279,   283,   287,   289,   299,   295,   313,
     310,   323,   326,   330,   335,   340,   345,   352,   359,   367,
     371,   375,   379,   383,   387,   391,   395,   399,   401,   405,
     409,   413,   417,   421,   425,   429,   436,   440,   444,   451,
     458,   466,   476,   479,   487,   492,   496,   506,   518,   526,
     531,   547,   563
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "INT", "HEX", "ERROR", "UINT", "M2_TRUE",
  "M2_FALSE", "CHAR", "FLOAT", "STRING", "NAME", "BLOCKNAME", "IDENT",
  "VARNAME", "TYPENAME", "SIZE", "CAP", "ORD", "HIGH", "ABS", "MIN_FUNC",
  "MAX_FUNC", "FLOAT_FUNC", "VAL", "CHR", "ODD", "TRUNC", "TSIZE", "ADR",
  "INC", "DEC", "INCL", "EXCL", "COLONCOLON", "DOLLAR_VARIABLE", "','",
  "ABOVE_COMMA", "ASSIGN", "'<'", "'>'", "LEQ", "GEQ", "'='", "NOTEQUAL",
  "'#'", "IN", "OROR", "LOGICAL_AND", "'&'", "'@'", "'+'", "'-'", "'*'",
  "'/'", "DIV", "MOD", "UNARY", "'^'", "DOT", "'['", "'('", "NOT", "'~'",
  "QID", "')'", "'{'", "'}'", "']'", "$accept", "start", "type_exp", "exp",
  "$@1", "not_exp", "set", "$@2", "$@3", "arglist", "non_empty_arglist",
  "block", "fblock", "variable", "type", YY_NULLPTRPTR
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
     285,   286,   287,   288,   289,   290,   291,    44,   292,   293,
      60,    62,   294,   295,    61,   296,    35,   297,   298,   299,
      38,    64,    43,    45,    42,    47,   300,   301,   302,    94,
     303,    91,    40,   304,   126,   305,    41,   123,   125,    93
};
# endif

#define YYPACT_NINF -91

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-91)))

#define YYTABLE_NINF -76

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-76)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     165,   -91,   -91,   -91,   -91,   -91,   -91,   -91,   -91,   -91,
     -91,   227,   -59,   -56,   -22,   -21,   -18,   -16,     3,     9,
      18,    29,    30,    31,    32,    33,    35,    36,    37,   -91,
     165,   -91,   165,   -91,   -91,   165,    47,   -91,   838,   165,
     -91,    -6,    -2,   -91,   -36,   165,     8,   -36,   165,   165,
     165,   165,    85,    85,   165,    85,   165,   165,   165,   165,
     165,   165,   165,   165,   165,     8,   165,   312,   838,   -33,
     -43,   -91,   165,   165,   165,   165,   165,   165,   165,   165,
     -15,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     -91,    90,   -91,   -91,     8,    15,   165,   165,   -24,   340,
     368,   396,   424,    38,    39,   452,    66,   480,   508,   536,
     564,   592,   256,   284,   788,   814,     8,   -91,   165,   -91,
     165,   862,   -39,   -39,   -39,   -39,   -39,   -39,   -39,   165,
     -91,    42,    70,    92,   150,   210,   210,     8,     8,     8,
       8,   -91,   165,   165,   -91,   -91,   620,   -32,   -91,   -91,
     -91,   -91,   -91,   -91,   -91,   -91,   165,   -91,   -91,   -91,
     -91,   -91,   165,   -91,   165,   -91,   165,   165,   838,     8,
     838,   -35,   -29,   -91,   -91,   648,   676,   704,   732,   760,
     165,   -91,   -91,   -91,   -91,   -91,   -91,   -91,   838
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    68,    69,    66,    67,    70,    71,    74,    81,    76,
      82,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    79,
       0,     6,     0,    10,    11,    41,     0,     3,     2,     0,
      31,     0,    78,    72,     4,     0,    25,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     8,     0,     0,    42,     0,
       0,     1,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       5,     0,    37,    39,     9,     0,     0,    41,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     7,    48,     0,    35,
       0,    65,    61,    62,    59,    60,    56,    57,    58,    41,
      32,     0,    64,    63,    49,    54,    55,    50,    51,    52,
      53,    30,     0,    41,    80,    77,     0,     0,    73,    12,
      13,    16,    14,    17,    18,    19,     0,    21,    22,    23,
      24,    15,     0,    26,     0,    28,     0,     0,    43,    46,
      44,     0,     0,    47,    36,     0,     0,     0,     0,     0,
       0,    38,    40,    20,    27,    29,    33,    34,    45
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -91,   -91,   -91,     0,   -91,   -91,    26,   -91,   -91,   -90,
     -91,   -91,   -91,   -91,    55
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    36,    37,    68,    66,    39,    40,   142,   143,    69,
     171,    41,    42,    43,    47
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      38,    10,   180,    48,   118,   118,    49,   147,   118,    81,
      82,    46,    83,    84,    85,    86,    87,    88,    89,    96,
      90,    91,    92,    93,    97,   120,    96,   144,   145,    95,
      65,    97,    67,   -75,   181,   119,   174,   182,    96,    94,
      50,    51,   148,    97,    52,    67,    53,    71,    99,   100,
     101,   102,   129,   172,   105,    44,   107,   108,   109,   110,
     111,   112,   113,   114,   115,    54,   116,    90,    91,    92,
      93,    55,   121,   122,   123,   124,   125,   126,   127,   128,
      56,   132,   133,   134,   135,   136,   137,   138,   139,   140,
      70,    57,    58,    59,    60,    61,   146,    62,    63,    64,
      98,    10,   141,   156,   153,   154,   130,   103,   104,    97,
     106,     0,     0,     0,     0,     0,     0,     0,   168,    82,
     169,    83,    84,    85,    86,    87,    88,    89,     0,    90,
      91,    92,    93,     0,     0,   131,     0,     0,     0,     0,
       0,     0,   170,    83,    84,    85,    86,    87,    88,    89,
       0,    90,    91,    92,    93,     0,   175,     0,     0,     0,
       0,     0,   176,     0,   177,     0,   178,   179,     1,     0,
       0,     2,     3,     4,     5,     6,     7,     8,     9,     0,
     188,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    84,    85,    86,    87,    88,    89,     0,    90,
      91,    92,    93,     0,     0,     0,     0,    30,    31,     0,
       0,     0,     0,     0,     0,     0,     0,    32,    33,    34,
       1,     0,    35,     2,     3,     4,     5,     6,     7,     8,
       9,     0,     0,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    86,    87,    88,    89,     0,    90,
      91,    92,    93,     0,     0,     0,     0,     0,     0,    30,
      31,     0,     0,     0,     0,     0,     0,     0,     0,    45,
      33,    34,     0,   162,    35,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,     0,    83,    84,    85,
      86,    87,    88,    89,     0,    90,    91,    92,    93,     0,
       0,   164,   163,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,     0,    83,    84,    85,    86,    87,
      88,    89,     0,    90,    91,    92,    93,     0,     0,     0,
     165,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,     0,    83,    84,    85,    86,    87,    88,    89,
       0,    90,    91,    92,    93,     0,     0,     0,   117,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
       0,    83,    84,    85,    86,    87,    88,    89,     0,    90,
      91,    92,    93,     0,     0,     0,   149,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,     0,    83,
      84,    85,    86,    87,    88,    89,     0,    90,    91,    92,
      93,     0,     0,     0,   150,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,     0,    83,    84,    85,
      86,    87,    88,    89,     0,    90,    91,    92,    93,     0,
       0,     0,   151,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,     0,    83,    84,    85,    86,    87,
      88,    89,     0,    90,    91,    92,    93,     0,     0,     0,
     152,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,     0,    83,    84,    85,    86,    87,    88,    89,
       0,    90,    91,    92,    93,     0,     0,     0,   155,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
       0,    83,    84,    85,    86,    87,    88,    89,     0,    90,
      91,    92,    93,     0,     0,     0,   157,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,     0,    83,
      84,    85,    86,    87,    88,    89,     0,    90,    91,    92,
      93,     0,     0,     0,   158,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,     0,    83,    84,    85,
      86,    87,    88,    89,     0,    90,    91,    92,    93,     0,
       0,     0,   159,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,     0,    83,    84,    85,    86,    87,
      88,    89,     0,    90,    91,    92,    93,     0,     0,     0,
     160,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,     0,    83,    84,    85,    86,    87,    88,    89,
       0,    90,    91,    92,    93,     0,     0,     0,   161,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
       0,    83,    84,    85,    86,    87,    88,    89,     0,    90,
      91,    92,    93,     0,     0,     0,   173,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,     0,    83,
      84,    85,    86,    87,    88,    89,     0,    90,    91,    92,
      93,     0,     0,     0,   183,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,     0,    83,    84,    85,
      86,    87,    88,    89,     0,    90,    91,    92,    93,     0,
       0,     0,   184,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,     0,    83,    84,    85,    86,    87,
      88,    89,     0,    90,    91,    92,    93,     0,     0,     0,
     185,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,     0,    83,    84,    85,    86,    87,    88,    89,
       0,    90,    91,    92,    93,     0,     0,     0,   186,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
       0,    83,    84,    85,    86,    87,    88,    89,     0,    90,
      91,    92,    93,     0,     0,   166,   187,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,     0,    83,
      84,    85,    86,    87,    88,    89,     0,    90,    91,    92,
      93,   167,     0,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,     0,    83,    84,    85,    86,    87,
      88,    89,     0,    90,    91,    92,    93,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,     0,    83,
      84,    85,    86,    87,    88,    89,     0,    90,    91,    92,
      93,   -76,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,     0,    83,    84,    85,    86,    87,    88,    89,
       0,    90,    91,    92,    93
};

static const yytype_int16 yycheck[] =
{
       0,    16,    37,    62,    37,    37,    62,    97,    37,    48,
      49,    11,    51,    52,    53,    54,    55,    56,    57,    62,
      59,    60,    61,    62,    67,    68,    62,    12,    13,    35,
      30,    67,    32,    35,    69,    68,    68,    66,    62,    39,
      62,    62,    66,    67,    62,    45,    62,     0,    48,    49,
      50,    51,    67,   143,    54,     0,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    62,    66,    59,    60,    61,
      62,    62,    72,    73,    74,    75,    76,    77,    78,    79,
      62,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      35,    62,    62,    62,    62,    62,    96,    62,    62,    62,
      45,    16,    12,    37,    66,    66,    80,    52,    53,    67,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   118,    49,
     120,    51,    52,    53,    54,    55,    56,    57,    -1,    59,
      60,    61,    62,    -1,    -1,    80,    -1,    -1,    -1,    -1,
      -1,    -1,   142,    51,    52,    53,    54,    55,    56,    57,
      -1,    59,    60,    61,    62,    -1,   156,    -1,    -1,    -1,
      -1,    -1,   162,    -1,   164,    -1,   166,   167,     3,    -1,
      -1,     6,     7,     8,     9,    10,    11,    12,    13,    -1,
     180,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      -1,    36,    52,    53,    54,    55,    56,    57,    -1,    59,
      60,    61,    62,    -1,    -1,    -1,    -1,    52,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    63,    64,
       3,    -1,    67,     6,     7,     8,     9,    10,    11,    12,
      13,    -1,    -1,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    -1,    36,    54,    55,    56,    57,    -1,    59,
      60,    61,    62,    -1,    -1,    -1,    -1,    -1,    -1,    52,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,
      63,    64,    -1,    37,    67,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    -1,    59,    60,    61,    62,    -1,
      -1,    37,    66,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    -1,    59,    60,    61,    62,    -1,    -1,    -1,
      66,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      -1,    59,    60,    61,    62,    -1,    -1,    -1,    66,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    -1,    59,
      60,    61,    62,    -1,    -1,    -1,    66,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    -1,    59,    60,    61,
      62,    -1,    -1,    -1,    66,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    -1,    59,    60,    61,    62,    -1,
      -1,    -1,    66,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    -1,    59,    60,    61,    62,    -1,    -1,    -1,
      66,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      -1,    59,    60,    61,    62,    -1,    -1,    -1,    66,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    -1,    59,
      60,    61,    62,    -1,    -1,    -1,    66,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    -1,    59,    60,    61,
      62,    -1,    -1,    -1,    66,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    -1,    59,    60,    61,    62,    -1,
      -1,    -1,    66,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    -1,    59,    60,    61,    62,    -1,    -1,    -1,
      66,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      -1,    59,    60,    61,    62,    -1,    -1,    -1,    66,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    -1,    59,
      60,    61,    62,    -1,    -1,    -1,    66,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    -1,    59,    60,    61,
      62,    -1,    -1,    -1,    66,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    -1,    59,    60,    61,    62,    -1,
      -1,    -1,    66,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    -1,    59,    60,    61,    62,    -1,    -1,    -1,
      66,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      -1,    59,    60,    61,    62,    -1,    -1,    -1,    66,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    -1,    59,
      60,    61,    62,    -1,    -1,    37,    66,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    -1,    59,    60,    61,
      62,    37,    -1,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    -1,    59,    60,    61,    62,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    -1,    59,    60,    61,
      62,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      -1,    59,    60,    61,    62
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     6,     7,     8,     9,    10,    11,    12,    13,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    36,
      52,    53,    62,    63,    64,    67,    71,    72,    73,    75,
      76,    81,    82,    83,    84,    62,    73,    84,    62,    62,
      62,    62,    62,    62,    62,    62,    62,    62,    62,    62,
      62,    62,    62,    62,    62,    73,    74,    73,    73,    79,
      84,     0,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    51,    52,    53,    54,    55,    56,    57,
      59,    60,    61,    62,    73,    35,    62,    67,    84,    73,
      73,    73,    73,    84,    84,    73,    84,    73,    73,    73,
      73,    73,    73,    73,    73,    73,    73,    66,    37,    68,
      68,    73,    73,    73,    73,    73,    73,    73,    73,    67,
      76,    84,    73,    73,    73,    73,    73,    73,    73,    73,
      73,    12,    77,    78,    12,    13,    73,    79,    66,    66,
      66,    66,    66,    66,    66,    66,    37,    66,    66,    66,
      66,    66,    37,    66,    37,    66,    37,    37,    73,    73,
      73,    80,    79,    66,    68,    73,    73,    73,    73,    73,
      37,    69,    66,    66,    66,    66,    66,    66,    73
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    70,    71,    71,    72,    73,    74,    73,    73,    73,
      75,    75,    73,    73,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    73,    76,    76,    77,    73,    78,
      73,    79,    79,    79,    80,    80,    73,    73,    73,    73,
      73,    73,    73,    73,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    73,    81,    82,    82,    83,    83,
      83,    83,    84
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     2,     0,     3,     2,     2,
       1,     1,     4,     4,     4,     4,     4,     4,     4,     4,
       6,     4,     4,     4,     4,     2,     4,     6,     4,     6,
       3,     1,     3,     6,     6,     3,     4,     0,     5,     0,
       5,     0,     1,     3,     1,     3,     4,     4,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     1,     1,     1,     1,
       1,     1,     1,     4,     1,     1,     1,     3,     1,     1,
       3,     1,     1
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
yy_symbol_value_print (FILE *yyo, int yytype, m2_exp_YYSTYPE const * const yyvaluep)
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
yy_symbol_print (FILE *yyo, int yytype, m2_exp_YYSTYPE const * const yyvaluep)
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
yy_reduce_print (yytype_int16 *yyssp, m2_exp_YYSTYPE *yyvsp, int yyrule)
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
yydestruct (const char *yymsg, int yytype, m2_exp_YYSTYPE *yyvaluep)
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
m2_exp_YYSTYPE yylval;
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
    m2_exp_YYSTYPE yyvsa[YYINITDEPTH];
    m2_exp_YYSTYPE *yyvs;
    m2_exp_YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  m2_exp_YYSTYPE yyval;

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
        m2_exp_YYSTYPE *yyvs1 = yyvs;
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
        union m2_exp_yyalloc *yyptr =
          (union m2_exp_yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
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
#line 154 "m2-exp.y"
    { pstate->push_new<type_operation> ((yyvsp[0].tval)); }
#line 1629 "m2-exp.c.tmp"
    break;

  case 5:
#line 160 "m2-exp.y"
    { pstate->wrap<unop_ind_operation> (); }
#line 1635 "m2-exp.c.tmp"
    break;

  case 6:
#line 164 "m2-exp.y"
    { number_sign = -1; }
#line 1641 "m2-exp.c.tmp"
    break;

  case 7:
#line 166 "m2-exp.y"
    { number_sign = 1;
			  pstate->wrap<unary_neg_operation> (); }
#line 1648 "m2-exp.c.tmp"
    break;

  case 8:
#line 171 "m2-exp.y"
    { pstate->wrap<unary_plus_operation> (); }
#line 1654 "m2-exp.c.tmp"
    break;

  case 9:
#line 175 "m2-exp.y"
    { pstate->wrap<unary_logical_not_operation> (); }
#line 1660 "m2-exp.c.tmp"
    break;

  case 12:
#line 183 "m2-exp.y"
    { error (_("CAP function is not implemented")); }
#line 1666 "m2-exp.c.tmp"
    break;

  case 13:
#line 187 "m2-exp.y"
    { error (_("ORD function is not implemented")); }
#line 1672 "m2-exp.c.tmp"
    break;

  case 14:
#line 191 "m2-exp.y"
    { error (_("ABS function is not implemented")); }
#line 1678 "m2-exp.c.tmp"
    break;

  case 15:
#line 195 "m2-exp.y"
    { pstate->wrap<unop_addr_operation> (); }
#line 1684 "m2-exp.c.tmp"
    break;

  case 16:
#line 199 "m2-exp.y"
    { pstate->wrap<m2_unop_high_operation> (); }
#line 1690 "m2-exp.c.tmp"
    break;

  case 17:
#line 203 "m2-exp.y"
    { error (_("MIN function is not implemented")); }
#line 1696 "m2-exp.c.tmp"
    break;

  case 18:
#line 207 "m2-exp.y"
    { error (_("MAX function is not implemented")); }
#line 1702 "m2-exp.c.tmp"
    break;

  case 19:
#line 211 "m2-exp.y"
    { error (_("FLOAT function is not implemented")); }
#line 1708 "m2-exp.c.tmp"
    break;

  case 20:
#line 215 "m2-exp.y"
    { error (_("VAL function is not implemented")); }
#line 1714 "m2-exp.c.tmp"
    break;

  case 21:
#line 219 "m2-exp.y"
    { error (_("CHR function is not implemented")); }
#line 1720 "m2-exp.c.tmp"
    break;

  case 22:
#line 223 "m2-exp.y"
    { error (_("ODD function is not implemented")); }
#line 1726 "m2-exp.c.tmp"
    break;

  case 23:
#line 227 "m2-exp.y"
    { error (_("TRUNC function is not implemented")); }
#line 1732 "m2-exp.c.tmp"
    break;

  case 24:
#line 231 "m2-exp.y"
    { pstate->wrap<unop_sizeof_operation> (); }
#line 1738 "m2-exp.c.tmp"
    break;

  case 25:
#line 235 "m2-exp.y"
    { pstate->wrap<unop_sizeof_operation> (); }
#line 1744 "m2-exp.c.tmp"
    break;

  case 26:
#line 240 "m2-exp.y"
    { pstate->wrap<preinc_operation> (); }
#line 1750 "m2-exp.c.tmp"
    break;

  case 27:
#line 244 "m2-exp.y"
    {
			  operation_up rhs = pstate->pop ();
			  operation_up lhs = pstate->pop ();
			  pstate->push_new<assign_modify_operation>
			    (BINOP_ADD, std::move (lhs), std::move (rhs));
			}
#line 1761 "m2-exp.c.tmp"
    break;

  case 28:
#line 253 "m2-exp.y"
    { pstate->wrap<predec_operation> (); }
#line 1767 "m2-exp.c.tmp"
    break;

  case 29:
#line 257 "m2-exp.y"
    {
			  operation_up rhs = pstate->pop ();
			  operation_up lhs = pstate->pop ();
			  pstate->push_new<assign_modify_operation>
			    (BINOP_SUB, std::move (lhs), std::move (rhs));
			}
#line 1778 "m2-exp.c.tmp"
    break;

  case 30:
#line 266 "m2-exp.y"
    {
			  pstate->push_new<structop_operation>
			    (pstate->pop (), copy_name ((yyvsp[0].sval)));
			}
#line 1787 "m2-exp.c.tmp"
    break;

  case 32:
#line 276 "m2-exp.y"
    { error (_("Sets are not implemented."));}
#line 1793 "m2-exp.c.tmp"
    break;

  case 33:
#line 280 "m2-exp.y"
    { error (_("Sets are not implemented."));}
#line 1799 "m2-exp.c.tmp"
    break;

  case 34:
#line 284 "m2-exp.y"
    { error (_("Sets are not implemented."));}
#line 1805 "m2-exp.c.tmp"
    break;

  case 35:
#line 288 "m2-exp.y"
    { error (_("Sets are not implemented."));}
#line 1811 "m2-exp.c.tmp"
    break;

  case 36:
#line 290 "m2-exp.y"
    { error (_("Sets are not implemented."));}
#line 1817 "m2-exp.c.tmp"
    break;

  case 37:
#line 299 "m2-exp.y"
    { pstate->start_arglist(); }
#line 1823 "m2-exp.c.tmp"
    break;

  case 38:
#line 301 "m2-exp.y"
    {
			  gdb_assert (pstate->arglist_len > 0);
			  std::vector<operation_up> args
			    = pstate->pop_vector (pstate->end_arglist ());
			  pstate->push_new<multi_subscript_operation>
			    (pstate->pop (), std::move (args));
			}
#line 1835 "m2-exp.c.tmp"
    break;

  case 39:
#line 313 "m2-exp.y"
    { pstate->start_arglist (); }
#line 1841 "m2-exp.c.tmp"
    break;

  case 40:
#line 315 "m2-exp.y"
    {
			  std::vector<operation_up> args
			    = pstate->pop_vector (pstate->end_arglist ());
			  pstate->push_new<funcall_operation>
			    (pstate->pop (), std::move (args));
			}
#line 1852 "m2-exp.c.tmp"
    break;

  case 42:
#line 327 "m2-exp.y"
    { pstate->arglist_len = 1; }
#line 1858 "m2-exp.c.tmp"
    break;

  case 43:
#line 331 "m2-exp.y"
    { pstate->arglist_len++; }
#line 1864 "m2-exp.c.tmp"
    break;

  case 44:
#line 336 "m2-exp.y"
    { pstate->arglist_len = 1; }
#line 1870 "m2-exp.c.tmp"
    break;

  case 45:
#line 341 "m2-exp.y"
    { pstate->arglist_len++; }
#line 1876 "m2-exp.c.tmp"
    break;

  case 46:
#line 346 "m2-exp.y"
    {
			  pstate->push_new<unop_memval_operation>
			    (pstate->pop (), (yyvsp[-2].tval));
			}
#line 1885 "m2-exp.c.tmp"
    break;

  case 47:
#line 353 "m2-exp.y"
    {
			  pstate->push_new<unop_cast_operation>
			    (pstate->pop (), (yyvsp[-3].tval));
			}
#line 1894 "m2-exp.c.tmp"
    break;

  case 48:
#line 360 "m2-exp.y"
    { }
#line 1900 "m2-exp.c.tmp"
    break;

  case 49:
#line 368 "m2-exp.y"
    { pstate->wrap2<repeat_operation> (); }
#line 1906 "m2-exp.c.tmp"
    break;

  case 50:
#line 372 "m2-exp.y"
    { pstate->wrap2<mul_operation> (); }
#line 1912 "m2-exp.c.tmp"
    break;

  case 51:
#line 376 "m2-exp.y"
    { pstate->wrap2<div_operation> (); }
#line 1918 "m2-exp.c.tmp"
    break;

  case 52:
#line 380 "m2-exp.y"
    { pstate->wrap2<intdiv_operation> (); }
#line 1924 "m2-exp.c.tmp"
    break;

  case 53:
#line 384 "m2-exp.y"
    { pstate->wrap2<rem_operation> (); }
#line 1930 "m2-exp.c.tmp"
    break;

  case 54:
#line 388 "m2-exp.y"
    { pstate->wrap2<add_operation> (); }
#line 1936 "m2-exp.c.tmp"
    break;

  case 55:
#line 392 "m2-exp.y"
    { pstate->wrap2<sub_operation> (); }
#line 1942 "m2-exp.c.tmp"
    break;

  case 56:
#line 396 "m2-exp.y"
    { pstate->wrap2<equal_operation> (); }
#line 1948 "m2-exp.c.tmp"
    break;

  case 57:
#line 400 "m2-exp.y"
    { pstate->wrap2<notequal_operation> (); }
#line 1954 "m2-exp.c.tmp"
    break;

  case 58:
#line 402 "m2-exp.y"
    { pstate->wrap2<notequal_operation> (); }
#line 1960 "m2-exp.c.tmp"
    break;

  case 59:
#line 406 "m2-exp.y"
    { pstate->wrap2<leq_operation> (); }
#line 1966 "m2-exp.c.tmp"
    break;

  case 60:
#line 410 "m2-exp.y"
    { pstate->wrap2<geq_operation> (); }
#line 1972 "m2-exp.c.tmp"
    break;

  case 61:
#line 414 "m2-exp.y"
    { pstate->wrap2<less_operation> (); }
#line 1978 "m2-exp.c.tmp"
    break;

  case 62:
#line 418 "m2-exp.y"
    { pstate->wrap2<gtr_operation> (); }
#line 1984 "m2-exp.c.tmp"
    break;

  case 63:
#line 422 "m2-exp.y"
    { pstate->wrap2<logical_and_operation> (); }
#line 1990 "m2-exp.c.tmp"
    break;

  case 64:
#line 426 "m2-exp.y"
    { pstate->wrap2<logical_or_operation> (); }
#line 1996 "m2-exp.c.tmp"
    break;

  case 65:
#line 430 "m2-exp.y"
    { pstate->wrap2<assign_operation> (); }
#line 2002 "m2-exp.c.tmp"
    break;

  case 66:
#line 437 "m2-exp.y"
    { pstate->push_new<bool_operation> ((yyvsp[0].ulval)); }
#line 2008 "m2-exp.c.tmp"
    break;

  case 67:
#line 441 "m2-exp.y"
    { pstate->push_new<bool_operation> ((yyvsp[0].ulval)); }
#line 2014 "m2-exp.c.tmp"
    break;

  case 68:
#line 445 "m2-exp.y"
    {
			  pstate->push_new<long_const_operation>
			    (parse_m2_type (pstate)->builtin_int, (yyvsp[0].lval));
			}
#line 2023 "m2-exp.c.tmp"
    break;

  case 69:
#line 452 "m2-exp.y"
    {
			  pstate->push_new<long_const_operation>
			    (parse_m2_type (pstate)->builtin_card, (yyvsp[0].ulval));
			}
#line 2032 "m2-exp.c.tmp"
    break;

  case 70:
#line 459 "m2-exp.y"
    {
			  pstate->push_new<long_const_operation>
			    (parse_m2_type (pstate)->builtin_char, (yyvsp[0].ulval));
			}
#line 2041 "m2-exp.c.tmp"
    break;

  case 71:
#line 467 "m2-exp.y"
    {
			  float_data data;
			  std::copy (std::begin ((yyvsp[0].val)), std::end ((yyvsp[0].val)),
				     std::begin (data));
			  pstate->push_new<float_const_operation>
			    (parse_m2_type (pstate)->builtin_real, data);
			}
#line 2053 "m2-exp.c.tmp"
    break;

  case 73:
#line 480 "m2-exp.y"
    {
			  pstate->push_new<long_const_operation>
			    (parse_m2_type (pstate)->builtin_int,
			     (yyvsp[-1].tval)->length ());
			}
#line 2063 "m2-exp.c.tmp"
    break;

  case 74:
#line 488 "m2-exp.y"
    { error (_("strings are not implemented")); }
#line 2069 "m2-exp.c.tmp"
    break;

  case 75:
#line 493 "m2-exp.y"
    { (yyval.bval) = (yyvsp[0].sym)->value_block (); }
#line 2075 "m2-exp.c.tmp"
    break;

  case 76:
#line 497 "m2-exp.y"
    { struct symbol *sym
			    = lookup_symbol (copy_name ((yyvsp[0].sval)).c_str (),
					     pstate->expression_context_block,
					     SEARCH_VFT, 0).symbol;
			  (yyval.sym) = sym;}
#line 2085 "m2-exp.c.tmp"
    break;

  case 77:
#line 507 "m2-exp.y"
    { struct symbol *tem
			    = lookup_symbol (copy_name ((yyvsp[0].sval)).c_str (), (yyvsp[-2].bval),
					     SEARCH_VFT, 0).symbol;
			  if (!tem || tem->loc_class () != LOC_BLOCK)
			    error (_("No function \"%s\" in specified context."),
				   copy_name ((yyvsp[0].sval)).c_str ());
			  (yyval.sym) = tem;
			}
#line 2098 "m2-exp.c.tmp"
    break;

  case 78:
#line 519 "m2-exp.y"
    {
			  block_symbol sym { (yyvsp[0].sym), nullptr };
			  pstate->push_new<var_value_operation> (sym);
			}
#line 2107 "m2-exp.c.tmp"
    break;

  case 79:
#line 527 "m2-exp.y"
    { pstate->push_dollar ((yyvsp[0].sval)); }
#line 2113 "m2-exp.c.tmp"
    break;

  case 80:
#line 532 "m2-exp.y"
    { struct block_symbol sym
			    = lookup_symbol (copy_name ((yyvsp[0].sval)).c_str (), (yyvsp[-2].bval),
					     SEARCH_VFT, 0);

			  if (sym.symbol == 0)
			    error (_("No symbol \"%s\" in specified context."),
				   copy_name ((yyvsp[0].sval)).c_str ());
			  if (symbol_read_needs_frame (sym.symbol))
			    pstate->block_tracker->update (sym);

			  pstate->push_new<var_value_operation> (sym);
			}
#line 2130 "m2-exp.c.tmp"
    break;

  case 81:
#line 548 "m2-exp.y"
    { struct block_symbol sym;
			  struct field_of_this_result is_a_field_of_this;

			  std::string name = copy_name ((yyvsp[0].sval));
			  sym
			    = lookup_symbol (name.c_str (),
					     pstate->expression_context_block,
					     SEARCH_VFT,
					     &is_a_field_of_this);

			  pstate->push_symbol (name.c_str (), sym);
			}
#line 2147 "m2-exp.c.tmp"
    break;

  case 82:
#line 564 "m2-exp.y"
    { (yyval.tval)
			    = lookup_typename (pstate->language (),
					       copy_name ((yyvsp[0].sval)).c_str (),
					       pstate->expression_context_block,
					       0);
			}
#line 2158 "m2-exp.c.tmp"
    break;


#line 2162 "m2-exp.c.tmp"

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
#line 573 "m2-exp.y"


/* Take care of parsing a number (anything that starts with a digit).
   Set yylval and return the token type; update lexptr.
   LEN is the number of characters in it.  */

/*** Needs some error checking for the float case ***/

static int
parse_number (int olen)
{
  const char *p = pstate->lexptr;
  ULONGEST n = 0;
  ULONGEST prevn = 0;
  int c,i,ischar=0;
  int base = input_radix;
  int len = olen;

  if(p[len-1] == 'H')
  {
     base = 16;
     len--;
  }
  else if(p[len-1] == 'C' || p[len-1] == 'B')
  {
     base = 8;
     ischar = p[len-1] == 'C';
     len--;
  }

  /* Scan the number */
  for (c = 0; c < len; c++)
  {
    if (p[c] == '.' && base == 10)
      {
	/* It's a float since it contains a point.  */
	if (!parse_float (p, len,
			  parse_m2_type (pstate)->builtin_real,
			  yylval.val))
	  return ERROR;

	pstate->lexptr += len;
	return FLOAT;
      }
    if (p[c] == '.' && base != 10)
       error (_("Floating point numbers must be base 10."));
    if (base == 10 && (p[c] < '0' || p[c] > '9'))
       error (_("Invalid digit \'%c\' in number."),p[c]);
 }

  while (len-- > 0)
    {
      c = *p++;
      n *= base;
      if( base == 8 && (c == '8' || c == '9'))
	 error (_("Invalid digit \'%c\' in octal number."),c);
      if (c >= '0' && c <= '9')
	i = c - '0';
      else
	{
	  if (base == 16 && c >= 'A' && c <= 'F')
	    i = c - 'A' + 10;
	  else
	     return ERROR;
	}
      n+=i;
      if(i >= base)
	 return ERROR;
      if (n == 0 && prevn == 0)
	;
      else if (RANGE_CHECK && prevn >= n)
	range_error (_("Overflow on numeric constant."));

	 prevn=n;
    }

  pstate->lexptr = p;
  if(*p == 'B' || *p == 'C' || *p == 'H')
     pstate->lexptr++;			/* Advance past B,C or H */

  if (ischar)
  {
     yylval.ulval = n;
     return CHAR;
  }

  int int_bits = gdbarch_int_bit (pstate->gdbarch ());
  bool have_signed = number_sign == -1;
  bool have_unsigned = number_sign == 1;
  if (have_signed && fits_in_type (number_sign, n, int_bits, true))
    {
      yylval.lval = n;
      return INT;
    }
  else if (have_unsigned && fits_in_type (number_sign, n, int_bits, false))
    {
      yylval.ulval = n;
      return UINT;
    }
  else
    error (_("Overflow on numeric constant."));
}


/* Some tokens */

static struct
{
   char name[2];
   int token;
} tokentab2[] =
{
    { {'<', '>'},    NOTEQUAL 	},
    { {':', '='},    ASSIGN	},
    { {'<', '='},    LEQ	},
    { {'>', '='},    GEQ	},
    { {':', ':'},    COLONCOLON },

};

/* Some specific keywords */

struct keyword {
   char keyw[10];
   int token;
};

static struct keyword keytab[] =
{
    {"OR" ,   OROR	 },
    {"IN",    IN         },/* Note space after IN */
    {"AND",   LOGICAL_AND},
    {"ABS",   ABS	 },
    {"ADR",   ADR	 },
    {"CHR",   CHR	 },
    {"DEC",   DEC	 },
    {"NOT",   NOT	 },
    {"DIV",   DIV    	 },
    {"INC",   INC	 },
    {"MAX",   MAX_FUNC	 },
    {"MIN",   MIN_FUNC	 },
    {"MOD",   MOD	 },
    {"ODD",   ODD	 },
    {"CAP",   CAP	 },
    {"ORD",   ORD	 },
    {"VAL",   VAL	 },
    {"EXCL",  EXCL	 },
    {"HIGH",  HIGH       },
    {"INCL",  INCL	 },
    {"SIZE",  SIZE       },
    {"FLOAT", FLOAT_FUNC },
    {"TRUNC", TRUNC	 },
    {"TSIZE", SIZE       },
};


/* Depth of parentheses.  */
static int paren_depth;

/* Read one token, getting characters through lexptr.  */

/* This is where we will check to make sure that the language and the
   operators used are compatible  */

static int
yylex (void)
{
  int c;
  int namelen;
  int i;
  const char *tokstart;
  char quote;

 retry:

  pstate->prev_lexptr = pstate->lexptr;

  tokstart = pstate->lexptr;


  /* See if it is a special token of length 2 */
  for( i = 0 ; i < (int) (sizeof tokentab2 / sizeof tokentab2[0]) ; i++)
     if (strncmp (tokentab2[i].name, tokstart, 2) == 0)
     {
	pstate->lexptr += 2;
	return tokentab2[i].token;
     }

  switch (c = *tokstart)
    {
    case 0:
      return 0;

    case ' ':
    case '\t':
    case '\n':
      pstate->lexptr++;
      goto retry;

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
      if (pstate->lexptr[1] >= '0' && pstate->lexptr[1] <= '9')
	break;			/* Falls into number code.  */
      else
      {
	 pstate->lexptr++;
	 return DOT;
      }

/* These are character tokens that appear as-is in the YACC grammar */
    case '+':
    case '-':
    case '*':
    case '/':
    case '^':
    case '<':
    case '>':
    case '[':
    case ']':
    case '=':
    case '{':
    case '}':
    case '#':
    case '@':
    case '~':
    case '&':
      pstate->lexptr++;
      return c;

    case '\'' :
    case '"':
      quote = c;
      for (namelen = 1; (c = tokstart[namelen]) != quote && c != '\0'; namelen++)
	if (c == '\\')
	  {
	    c = tokstart[++namelen];
	    if (c >= '0' && c <= '9')
	      {
		c = tokstart[++namelen];
		if (c >= '0' && c <= '9')
		  c = tokstart[++namelen];
	      }
	  }
      if(c != quote)
	 error (_("Unterminated string or character constant."));
      yylval.sval.ptr = tokstart + 1;
      yylval.sval.length = namelen - 1;
      pstate->lexptr += namelen + 1;

      if(namelen == 2)  	/* Single character */
      {
	   yylval.ulval = tokstart[1];
	   return CHAR;
      }
      else
	 return STRING;
    }

  /* Is it a number?  */
  /* Note:  We have already dealt with the case of the token '.'.
     See case '.' above.  */
  if ((c >= '0' && c <= '9'))
    {
      /* It's a number.  */
      int got_dot = 0, got_e = 0;
      const char *p = tokstart;
      int toktype;

      for (++p ;; ++p)
	{
	  if (!got_e && (*p == 'e' || *p == 'E'))
	    got_dot = got_e = 1;
	  else if (!got_dot && *p == '.')
	    got_dot = 1;
	  else if (got_e && (p[-1] == 'e' || p[-1] == 'E')
		   && (*p == '-' || *p == '+'))
	    /* This is the sign of the exponent, not the end of the
	       number.  */
	    continue;
	  else if ((*p < '0' || *p > '9') &&
		   (*p < 'A' || *p > 'F') &&
		   (*p != 'H'))  /* Modula-2 hexadecimal number */
	    break;
	}
	toktype = parse_number (p - tokstart);
	if (toktype == ERROR)
	  error (_("Invalid number \"%.*s\"."), (int) (p - tokstart),
		 tokstart);
	pstate->lexptr = p;
	return toktype;
    }

  if (!(c == '_' || c == '$'
	|| (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
    /* We must have come across a bad character (e.g. ';').  */
    error (_("Invalid character '%c' in expression."), c);

  /* It's a name.  See how long it is.  */
  namelen = 0;
  for (c = tokstart[namelen];
       (c == '_' || c == '$' || (c >= '0' && c <= '9')
	|| (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
       c = tokstart[++namelen])
    ;

  /* The token "if" terminates the expression and is NOT
     removed from the input stream.  */
  if (namelen == 2 && tokstart[0] == 'i' && tokstart[1] == 'f')
    {
      return 0;
    }

  pstate->lexptr += namelen;

  /*  Lookup special keywords */
  for(i = 0 ; i < (int) (sizeof(keytab) / sizeof(keytab[0])) ; i++)
     if (namelen == strlen (keytab[i].keyw)
	 && strncmp (tokstart, keytab[i].keyw, namelen) == 0)
	   return keytab[i].token;

  yylval.sval.ptr = tokstart;
  yylval.sval.length = namelen;

  if (*tokstart == '$')
    return DOLLAR_VARIABLE;

  /* Use token-type BLOCKNAME for symbols that happen to be defined as
     functions.  If this is not so, then ...
     Use token-type TYPENAME for symbols that happen to be defined
     currently as names of types; NAME for other symbols.
     The caller is not constrained to care about the distinction.  */
 {
    std::string tmp = copy_name (yylval.sval);
    struct symbol *sym;

    if (lookup_symtab (current_program_space, tmp.c_str ()) != nullptr)
      return BLOCKNAME;

    sym = lookup_symbol (tmp.c_str (), pstate->expression_context_block,
			 SEARCH_VFT, 0).symbol;
    if (sym && sym->loc_class () == LOC_BLOCK)
      return BLOCKNAME;
    if (lookup_typename (pstate->language (),
			 tmp.c_str (), pstate->expression_context_block, 1))
      return TYPENAME;

    if(sym)
    {
      switch(sym->loc_class ())
       {
       case LOC_STATIC:
       case LOC_REGISTER:
       case LOC_ARG:
       case LOC_REF_ARG:
       case LOC_REGPARM_ADDR:
       case LOC_LOCAL:
       case LOC_CONST:
       case LOC_CONST_BYTES:
       case LOC_OPTIMIZED_OUT:
       case LOC_COMPUTED:
	  return NAME;

       case LOC_TYPEDEF:
	  return TYPENAME;

       case LOC_BLOCK:
	  return BLOCKNAME;

       case LOC_UNDEF:
	  error (_("internal:  Undefined class in m2lex()"));

       case LOC_LABEL:
       case LOC_UNRESOLVED:
	  error (_("internal:  Unforeseen case in m2lex()"));

       default:
	  error (_("unhandled token in m2lex()"));
	  break;
       }
    }
    else
    {
       /* Built-in BOOLEAN type.  This is sort of a hack.  */
       if (startswith (tokstart, "TRUE"))
       {
	  yylval.ulval = 1;
	  return M2_TRUE;
       }
       else if (startswith (tokstart, "FALSE"))
       {
	  yylval.ulval = 0;
	  return M2_FALSE;
       }
    }

    /* Must be another type of name...  */
    return NAME;
 }
}

int
m2_language::parser (struct parser_state *par_state) const
{
  /* Setting up the parser state.  */
  scoped_restore pstate_restore = make_scoped_restore (&pstate);
  gdb_assert (par_state != NULL);
  pstate = par_state;
  paren_depth = 0;

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
