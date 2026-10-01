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
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 38 "cp-name-parser.y"



#include <unistd.h>
#include "demangle.h"
#include "cp-support.h"
#include "c-support.h"
#include "parser-defs.h"
#include "gdbsupport/selftest.h"

#define GDB_YY_REMAP_PREFIX cpname
#include "yy-remap.h"


#line 85 "cp-name-parser.c.tmp"

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
    NAME = 260,
    STRUCT = 261,
    CLASS = 262,
    UNION = 263,
    ENUM = 264,
    SIZEOF = 265,
    UNSIGNED = 266,
    COLONCOLON = 267,
    TEMPLATE = 268,
    ERROR = 269,
    NEW = 270,
    DELETE = 271,
    OPERATOR = 272,
    STATIC_CAST = 273,
    REINTERPRET_CAST = 274,
    DYNAMIC_CAST = 275,
    SIGNED_KEYWORD = 276,
    LONG = 277,
    SHORT = 278,
    INT_KEYWORD = 279,
    CONST_KEYWORD = 280,
    VOLATILE_KEYWORD = 281,
    DOUBLE_KEYWORD = 282,
    BOOL = 283,
    ELLIPSIS = 284,
    RESTRICT = 285,
    VOID = 286,
    FLOAT_KEYWORD = 287,
    CHAR = 288,
    WCHAR_T = 289,
    ASSIGN_MODIFY = 290,
    TRUEKEYWORD = 291,
    FALSEKEYWORD = 292,
    DEMANGLER_SPECIAL = 293,
    CONSTRUCTION_VTABLE = 294,
    CONSTRUCTION_IN = 295,
    OROR = 296,
    ANDAND = 297,
    EQUAL = 298,
    NOTEQUAL = 299,
    LEQ = 300,
    GEQ = 301,
    SPACESHIP = 302,
    LSH = 303,
    RSH = 304,
    UNARY = 305,
    INCREMENT = 306,
    DECREMENT = 307,
    ARROW = 308
  };
#endif
/* Tokens.  */
#define INT 258
#define FLOAT 259
#define NAME 260
#define STRUCT 261
#define CLASS 262
#define UNION 263
#define ENUM 264
#define SIZEOF 265
#define UNSIGNED 266
#define COLONCOLON 267
#define TEMPLATE 268
#define ERROR 269
#define NEW 270
#define DELETE 271
#define OPERATOR 272
#define STATIC_CAST 273
#define REINTERPRET_CAST 274
#define DYNAMIC_CAST 275
#define SIGNED_KEYWORD 276
#define LONG 277
#define SHORT 278
#define INT_KEYWORD 279
#define CONST_KEYWORD 280
#define VOLATILE_KEYWORD 281
#define DOUBLE_KEYWORD 282
#define BOOL 283
#define ELLIPSIS 284
#define RESTRICT 285
#define VOID 286
#define FLOAT_KEYWORD 287
#define CHAR 288
#define WCHAR_T 289
#define ASSIGN_MODIFY 290
#define TRUEKEYWORD 291
#define FALSEKEYWORD 292
#define DEMANGLER_SPECIAL 293
#define CONSTRUCTION_VTABLE 294
#define CONSTRUCTION_IN 295
#define OROR 296
#define ANDAND 297
#define EQUAL 298
#define NOTEQUAL 299
#define LEQ 300
#define GEQ 301
#define SPACESHIP 302
#define LSH 303
#define RSH 304
#define UNARY 305
#define INCREMENT 306
#define DECREMENT 307
#define ARROW 308

/* Value type.  */
#if ! defined cp_name_parser_YYSTYPE && ! defined cp_name_parser_YYSTYPE_IS_DECLARED
union cp_name_parser_YYSTYPE
{
#line 54 "cp-name-parser.y"

    struct demangle_component *comp;
    struct nested {
      struct demangle_component *comp;
      struct demangle_component **last;
    } nested;
    struct {
      struct demangle_component *comp, *last;
    } nested1;
    struct {
      struct demangle_component *comp, **last;
      struct nested fn;
      struct demangle_component *start;
      int fold_flag;
    } abstract;
    int lval;
    const char *opname;
  

#line 251 "cp-name-parser.c.tmp"

};
typedef union cp_name_parser_YYSTYPE cp_name_parser_YYSTYPE;
# define cp_name_parser_YYSTYPE_IS_TRIVIAL 1
# define cp_name_parser_YYSTYPE_IS_DECLARED 1
#endif



int yyparse (struct cpname_state *state);



/* Second part of user prologue.  */
#line 73 "cp-name-parser.y"


struct cpname_state
{
  cpname_state (const char *input, demangle_parse_info *info)
    : lexptr (input),
      prev_lexptr (input),
      demangle_info (info)
  { }

  /* Un-push a character into the lexer.  This can only un-push the
     previous character in the input string.  */
  void unpush (char c)
  {
    gdb_assert (lexptr[-1] == c);
    --lexptr;
  }

  /* LEXPTR is the current pointer into our lex buffer.  PREV_LEXPTR
     is the start of the last token lexed, only used for diagnostics.
     ERROR_LEXPTR is the first place an error occurred.  GLOBAL_ERRMSG
     is the first error message encountered.  */

  const char *lexptr, *prev_lexptr;
  const char *error_lexptr = nullptr;
  const char *global_errmsg = nullptr;

  demangle_parse_info *demangle_info;

  /* The parse tree created by the parser is stored here after a
     successful parse.  */

  struct demangle_component *global_result = nullptr;

  struct demangle_component *d_grab ();

  /* Helper functions.  These wrap the demangler tree interface,
     handle allocation from our global store, and return the allocated
     component.  */

  struct demangle_component *fill_comp (enum demangle_component_type d_type,
					struct demangle_component *lhs,
					struct demangle_component *rhs);

  struct demangle_component *make_operator (const char *name, int args);

  struct demangle_component *make_dtor (enum gnu_v3_dtor_kinds kind,
					struct demangle_component *name);

  struct demangle_component *make_builtin_type (const char *name);

  struct demangle_component *make_name (const char *name, int len);

  struct demangle_component *d_qualify (struct demangle_component *lhs,
					int qualifiers, int is_method);

  struct demangle_component *d_int_type (int flags);

  struct demangle_component *d_unary (const char *name,
				      struct demangle_component *lhs);

  struct demangle_component *d_binary (const char *name,
				       struct demangle_component *lhs,
				       struct demangle_component *rhs);

  int parse_number (const char *p, int len, int parsed_float, cp_name_parser_YYSTYPE *lvalp);
};

struct demangle_component *
cpname_state::d_grab ()
{
  return obstack_new<demangle_component> (&demangle_info->obstack);
}

/* Flags passed to d_qualify.  */

#define QUAL_CONST 1
#define QUAL_RESTRICT 2
#define QUAL_VOLATILE 4

/* Flags passed to d_int_type.  */

#define INT_CHAR	(1 << 0)
#define INT_SHORT	(1 << 1)
#define INT_LONG	(1 << 2)
#define INT_LLONG	(1 << 3)

#define INT_SIGNED	(1 << 4)
#define INT_UNSIGNED	(1 << 5)

/* Helper functions.  These wrap the demangler tree interface, handle
   allocation from our global store, and return the allocated component.  */

struct demangle_component *
cpname_state::fill_comp (enum demangle_component_type d_type,
			 struct demangle_component *lhs,
			 struct demangle_component *rhs)
{
  struct demangle_component *ret = d_grab ();
  int i;

  i = cplus_demangle_fill_component (ret, d_type, lhs, rhs);
  gdb_assert (i);

  return ret;
}

struct demangle_component *
cpname_state::make_operator (const char *name, int args)
{
  struct demangle_component *ret = d_grab ();
  int i;

  i = cplus_demangle_fill_operator (ret, name, args);
  gdb_assert (i);

  return ret;
}

struct demangle_component *
cpname_state::make_dtor (enum gnu_v3_dtor_kinds kind,
			 struct demangle_component *name)
{
  struct demangle_component *ret = d_grab ();
  int i;

  i = cplus_demangle_fill_dtor (ret, kind, name);
  gdb_assert (i);

  return ret;
}

struct demangle_component *
cpname_state::make_builtin_type (const char *name)
{
  struct demangle_component *ret = d_grab ();
  int i;

  i = cplus_demangle_fill_builtin_type (ret, name);
  gdb_assert (i);

  return ret;
}

struct demangle_component *
cpname_state::make_name (const char *name, int len)
{
  struct demangle_component *ret = d_grab ();
  int i;

  i = cplus_demangle_fill_name (ret, name, len);
  gdb_assert (i);

  return ret;
}

#define d_left(dc) (dc)->u.s_binary.left
#define d_right(dc) (dc)->u.s_binary.right

static int yylex (cp_name_parser_YYSTYPE *, cpname_state *);
static void yyerror (cpname_state *, const char *);

#line 429 "cp-name-parser.c.tmp"


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
         || (defined cp_name_parser_YYSTYPE_IS_TRIVIAL && cp_name_parser_YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union cp_name_parser_yyalloc
{
  yytype_int16 yyss_alloc;
  cp_name_parser_YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union cp_name_parser_yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (cp_name_parser_YYSTYPE)) \
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
#define YYFINAL  85
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1151

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  76
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  40
/* YYNRULES -- Number of rules.  */
#define YYNRULES  201
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  332

#define YYUNDEFTOK  2
#define YYMAXUTOK   308

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
       2,     2,     2,    73,     2,     2,     2,    64,    49,     2,
      74,    41,    62,    60,    42,    61,    69,    63,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    75,     2,
      52,    43,    53,    44,    59,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    70,     2,    71,    48,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    47,     2,    72,     2,     2,     2,
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
      35,    36,    37,    38,    39,    40,    45,    46,    50,    51,
      54,    55,    56,    57,    58,    65,    66,    67,    68
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   331,   331,   340,   342,   344,   349,   350,   357,   366,
     373,   376,   393,   396,   415,   417,   421,   427,   433,   439,
     445,   447,   449,   451,   453,   455,   457,   459,   461,   463,
     465,   467,   469,   471,   473,   475,   477,   479,   481,   483,
     485,   487,   489,   491,   493,   495,   497,   499,   501,   503,
     511,   516,   521,   525,   530,   538,   539,   541,   546,   558,
     559,   565,   567,   568,   570,   573,   574,   577,   578,   582,
     584,   587,   591,   596,   600,   609,   611,   618,   621,   632,
     633,   637,   639,   641,   642,   645,   649,   654,   659,   665,
     675,   679,   683,   691,   692,   695,   697,   699,   703,   704,
     711,   713,   715,   717,   719,   721,   725,   726,   730,   732,
     734,   736,   738,   740,   742,   746,   751,   754,   757,   763,
     771,   773,   787,   789,   790,   792,   795,   797,   798,   800,
     803,   805,   807,   809,   814,   817,   822,   829,   833,   844,
     850,   868,   871,   879,   881,   892,   899,   900,   906,   910,
     914,   916,   921,   926,   938,   942,   946,   954,   959,   968,
     972,   977,   982,   986,   992,   998,  1001,  1008,  1010,  1015,
    1019,  1023,  1030,  1046,  1053,  1060,  1079,  1083,  1087,  1091,
    1095,  1099,  1103,  1107,  1111,  1115,  1119,  1123,  1127,  1131,
    1135,  1139,  1143,  1147,  1152,  1156,  1160,  1167,  1171,  1174,
    1183,  1192
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "INT", "FLOAT", "NAME", "STRUCT",
  "CLASS", "UNION", "ENUM", "SIZEOF", "UNSIGNED", "COLONCOLON", "TEMPLATE",
  "ERROR", "NEW", "DELETE", "OPERATOR", "STATIC_CAST", "REINTERPRET_CAST",
  "DYNAMIC_CAST", "SIGNED_KEYWORD", "LONG", "SHORT", "INT_KEYWORD",
  "CONST_KEYWORD", "VOLATILE_KEYWORD", "DOUBLE_KEYWORD", "BOOL",
  "ELLIPSIS", "RESTRICT", "VOID", "FLOAT_KEYWORD", "CHAR", "WCHAR_T",
  "ASSIGN_MODIFY", "TRUEKEYWORD", "FALSEKEYWORD", "DEMANGLER_SPECIAL",
  "CONSTRUCTION_VTABLE", "CONSTRUCTION_IN", "')'", "','", "'='", "'?'",
  "OROR", "ANDAND", "'|'", "'^'", "'&'", "EQUAL", "NOTEQUAL", "'<'", "'>'",
  "LEQ", "GEQ", "SPACESHIP", "LSH", "RSH", "'@'", "'+'", "'-'", "'*'",
  "'/'", "'%'", "UNARY", "INCREMENT", "DECREMENT", "ARROW", "'.'", "'['",
  "']'", "'~'", "'!'", "'('", "':'", "$accept", "result", "start",
  "start_opt", "function", "demangler_special", "oper", "conversion_op",
  "conversion_op_name", "unqualified_name", "colon_name", "name",
  "colon_ext_name", "colon_ext_only", "ext_only_name", "nested_name",
  "templ", "template_params", "template_arg", "function_args",
  "function_arglist", "qualifiers_opt", "qualifier", "qualifiers",
  "int_part", "int_seq", "builtin_type", "ptr_operator", "array_indicator",
  "typespec_2", "abstract_declarator", "direct_abstract_declarator",
  "abstract_declarator_fn", "type", "declarator", "direct_declarator",
  "declarator_1", "direct_declarator_1", "exp", "exp1", YY_NULLPTRPTR
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
     295,    41,    44,    61,    63,   296,   297,   124,    94,    38,
     298,   299,    60,    62,   300,   301,   302,   303,   304,    64,
      43,    45,    42,    47,    37,   305,   306,   307,   308,    46,
      91,    93,   126,    33,    40,    58
};
# endif

#define YYPACT_NINF -218

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-218)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     295,    -2,  -218,    52,   558,  -218,   -11,  -218,  -218,  -218,
    -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,   295,   295,
      18,    38,  -218,  -218,  -218,     1,  -218,    13,  -218,   117,
     -10,  -218,    81,    43,   117,   937,  -218,   367,   117,   334,
    -218,  -218,   338,  -218,   117,  -218,    81,    58,     0,    39,
    -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,
    -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,
    -218,  -218,  -218,  -218,     4,    46,  -218,  -218,    71,    66,
    -218,  -218,  -218,    85,  -218,  -218,   338,    -2,   295,  -218,
    -218,   117,     5,   622,  -218,     6,    43,   122,   497,  -218,
      80,  -218,  -218,   825,   122,     9,  -218,  -218,   139,  -218,
    -218,    58,   117,   117,  -218,  -218,  -218,   121,   761,   622,
    -218,  -218,    80,  -218,    17,   122,   716,  -218,    80,  -218,
      80,  -218,  -218,    89,   125,   127,   131,  -218,  -218,   655,
     504,   460,   504,   420,  -218,     7,  -218,   334,   954,  -218,
    -218,    98,   102,  -218,  -218,  -218,   295,    99,  -218,    22,
    -218,  -218,   119,  -218,    58,   161,   117,   498,    10,   -12,
     498,   498,   162,     9,   117,   139,   295,  -218,   199,  -218,
     193,  -218,  -218,  -218,  -218,   117,  -218,  -218,  -218,    41,
     725,   197,  -218,  -218,   498,  -218,  -218,  -218,   200,  -218,
     913,   913,   913,   913,   295,  -218,   504,    91,    91,    91,
     686,   498,   170,   928,   172,   338,  -218,  -218,  -218,   504,
     504,   504,   504,   504,   504,   504,   504,   504,   504,   504,
     504,   504,   504,   504,   504,   504,   504,   504,   228,   229,
    -218,  -218,  -218,  -218,  -218,   117,  -218,    30,   117,  -218,
     117,   883,  -218,  -218,  -218,    35,   295,  -218,   725,  -218,
     725,   203,    80,   295,   295,   204,   194,   196,   205,   211,
     295,  -218,   504,   504,  -218,  -218,   823,   978,  1001,  1023,
    1044,  1064,  1082,  1082,   493,   493,   493,   493,   683,   683,
     218,   218,    91,    91,    91,  -218,  -218,  -218,  -218,  -218,
    -218,   498,  -218,   219,  -218,  -218,  -218,  -218,  -218,  -218,
    -218,   185,   195,   209,  -218,   232,    91,   954,   504,  -218,
    -218,   464,   464,   464,  -218,   954,   233,   243,   247,  -218,
    -218,  -218
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    62,   102,     0,     0,   101,   104,   105,   100,    97,
      96,   110,   112,    95,   114,   109,   103,   113,     0,     0,
       0,     0,     2,     5,     4,    55,    52,     6,    70,   127,
      11,    67,     0,    64,    98,     0,   106,   108,   123,   146,
       3,    71,     0,    54,   131,    68,     0,     0,    16,    17,
      33,    45,    30,    42,    41,    27,    25,    26,    36,    37,
      31,    32,    38,    39,    40,    34,    35,    20,    21,    22,
      23,    24,    43,    44,    47,     0,    28,    29,     0,     0,
      50,   111,    14,     0,    58,     1,     0,     0,     0,   117,
     116,    93,     0,     0,    12,     0,     0,     6,   141,   140,
     143,    13,   126,     0,     6,    61,    51,    69,    63,    73,
      99,     0,   129,   125,   104,   107,   122,     0,     0,     0,
      65,    59,   155,    66,     0,     6,   134,   147,   136,     8,
     156,   197,   198,     0,     0,     0,     0,   200,   201,     0,
       0,     0,     0,     0,    84,     0,    77,    79,    83,   130,
      53,     0,     0,    46,    49,    48,     0,     0,     7,     0,
     115,    94,     0,   120,     0,   114,    93,     0,     0,     0,
     134,    85,     0,     0,    93,     0,     0,   145,     0,   142,
     138,   139,    10,    72,    74,   133,   128,   124,    60,     0,
     134,   162,   163,     9,     0,   135,   154,   138,   160,   161,
       0,     0,     0,     0,     0,    81,     0,   169,   171,   170,
       0,   146,     0,   165,     0,     0,    75,    76,    80,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      18,    19,    15,    56,    57,    93,   121,     0,    93,    92,
      93,     0,    86,   137,   118,     0,     0,   132,     0,   153,
     134,     0,   149,     0,     0,     0,     0,     0,     0,     0,
       0,   167,     0,     0,   164,    78,     0,   193,   192,   191,
     190,   189,   183,   184,   188,   185,   186,   187,   181,   182,
     179,   180,   176,   177,   178,   194,   195,   119,    91,    90,
      89,    87,   144,     0,   148,   159,   151,   152,   157,   158,
     199,     0,     0,     0,    82,     0,   172,   166,     0,    88,
     150,     0,     0,     0,   168,   196,     0,     0,     0,   173,
     175,   174
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -218,  -218,    33,    14,   -39,  -218,  -218,    -1,  -218,    -4,
    -218,   145,  -178,   -19,     2,    -3,    83,   160,    75,  -218,
     -26,  -157,  -218,   241,   254,  -218,   258,   -20,   -74,    63,
     -25,   -21,   201,   -70,  -217,  -218,   169,  -218,    -5,  -127
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    21,   158,    94,    23,    24,    25,    26,    27,    28,
     120,    29,   122,    30,    31,    32,    33,   145,   146,   169,
      97,   160,    34,    35,    36,    37,    38,   170,    99,    39,
     172,   128,   101,    40,   261,   262,   129,   130,   213,   214
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint16 yytable[] =
{
      46,    79,    43,   144,   104,    45,   100,    98,   162,   249,
      41,   173,   259,   125,   127,   105,    81,   254,    87,   126,
     123,   183,   105,    84,    95,    88,   181,   105,   107,   250,
     251,   106,    79,    22,   118,   105,   124,   148,    85,     4,
     173,   303,   107,   304,    79,   150,   105,   144,   192,   215,
      42,    82,    83,    86,   181,   109,   199,     1,   118,    89,
     216,    42,    90,     1,   103,   217,   153,    80,   174,     4,
     151,   105,   174,   212,   180,    91,   163,   100,    98,   174,
     259,   148,   259,    92,   245,   159,   105,    93,   297,    20,
     168,   298,   245,   299,    20,    95,   191,   245,     4,   190,
      79,   195,   197,   245,   198,   147,   126,   123,    79,   152,
      96,   177,   155,    20,   189,   108,   168,   154,   182,    45,
     107,   125,   218,   124,    20,   156,     1,   126,   123,   108,
     265,   266,   267,   268,   176,   207,   208,   209,   118,   193,
      79,   215,     9,    10,   124,   195,   252,    13,    44,   147,
      92,   184,   243,    20,   103,   107,   171,   244,   106,   238,
     239,   247,   108,   200,    95,   195,   171,    95,    95,   240,
     260,   123,   205,   241,   190,   255,   144,   201,   175,   202,
     112,    96,   171,   203,   121,   107,   127,   124,   307,   242,
     246,    95,    44,    20,   326,   327,   328,    79,    79,    79,
      79,   208,   248,   253,    87,   256,   211,   108,    95,   263,
     148,   272,   264,   274,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   195,   306,   269,   260,   123,
     260,   123,   108,   271,   305,   310,   157,   311,    79,   312,
      96,   108,   314,    96,    96,   124,   185,   124,   313,   321,
     320,    96,   188,   211,   211,   211,   211,   316,   317,   322,
     102,   121,   108,   324,   329,   110,   319,    96,   147,   116,
     235,   236,   237,   323,   330,   149,   238,   239,   331,   302,
     275,   115,   121,   113,    96,   196,   308,   309,    95,   179,
       1,     0,     0,   315,     0,     0,     2,     3,     0,    44,
       0,     0,     4,   325,   301,     0,     5,     6,     7,     8,
       9,    10,    11,    12,     0,    13,    14,    15,    16,    17,
     108,     0,   161,    18,    19,   121,     0,     0,   175,     1,
       0,   131,   132,     1,     0,     0,   117,     0,   133,     2,
       3,   118,     0,   186,   187,     4,   134,   135,   136,     5,
       6,     7,     8,     9,    10,    11,    12,    20,    13,    14,
      15,    16,    17,     0,   137,   138,     0,     0,     2,     0,
      89,     0,     0,    90,    96,     0,     0,   139,     5,   114,
       7,     8,     0,     0,     0,     0,    91,     0,     0,   140,
      16,     0,     0,   121,    92,   121,    20,   161,   119,     0,
     141,   142,   143,     0,     0,   161,     0,     0,     0,     0,
       0,     0,     0,   131,   132,     1,   257,     0,     0,     0,
     133,     2,    47,     0,     0,     0,     0,     0,   134,   135,
     136,     5,     6,     7,     8,     9,    10,    11,    12,     0,
      13,    14,    15,    16,    17,     0,   137,   138,     0,     0,
       0,     0,     0,   131,   132,    84,     0,   131,   132,   210,
     133,     0,     0,     0,   133,     0,     0,     0,   134,   135,
     136,   140,   134,   135,   136,     0,   161,     0,     0,   161,
       0,   161,   206,   142,   143,     0,   137,   138,     0,     0,
     137,   138,    87,    87,     0,     0,     0,   131,   132,   178,
     178,     0,     0,   210,   133,     0,     0,     0,     0,     0,
       0,   140,   134,   135,   136,   140,     0,     0,     0,     0,
       0,     0,   206,   142,   143,     0,   206,   142,   143,     0,
     137,   138,     0,    89,    89,     0,    90,    90,     0,     0,
     231,   232,     0,   233,   234,   235,   236,   237,     0,    91,
      91,   238,   239,     1,     0,   140,     0,    92,    92,     2,
      47,    93,   167,    48,    49,     0,   206,   142,   143,     5,
       6,     7,     8,     9,    10,    11,    12,     0,    13,    14,
      15,    16,    17,    50,     0,     0,     0,     0,     0,     0,
      51,    52,     0,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,     0,    67,    68,
      69,    70,    71,     0,    72,    73,    74,     1,    75,     0,
      76,    77,    78,     2,   164,     0,     0,     0,     0,     0,
       0,     0,     0,     5,     6,     7,     8,     9,    10,    11,
      12,     0,    13,   165,    15,    16,    17,     0,     0,     0,
       1,     0,     0,   166,     0,     0,     2,     3,    89,     0,
       0,    90,     4,     0,     0,     0,     5,     6,     7,     8,
       9,    10,    11,    12,    91,    13,    14,    15,    16,    17,
       0,     1,    92,    18,    19,     0,   167,     2,     3,     0,
       0,     0,     0,     4,     0,     0,     0,     5,     6,     7,
       8,     9,    10,    11,    12,     0,    13,    14,    15,    16,
      17,     1,     0,     0,    18,    19,     0,    20,   117,   204,
       1,     0,     0,   118,     0,     0,     0,   117,     0,     0,
       0,     0,   118,   233,   234,   235,   236,   237,     0,     0,
       0,   238,   239,     0,     0,     0,     0,     0,    20,     0,
     270,     0,    89,     0,     0,    90,     0,     0,     0,     0,
       0,    89,     0,     0,    90,     0,    48,    49,    91,     0,
       0,     0,     0,     0,     0,     0,    92,    91,    20,     0,
     194,     0,     0,     0,     0,    92,    50,    20,     0,   258,
       0,     0,     0,    51,    52,     0,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
       0,    67,    68,    69,    70,    71,     0,    72,    73,    74,
       1,    75,     0,    76,    77,    78,     2,    47,     0,     0,
       0,     0,     0,     0,     0,     0,     5,     6,     7,     8,
       9,    10,    11,    12,     0,    13,   165,    15,    16,    17,
       0,     0,     0,     0,     0,     0,   166,   219,   220,   221,
     222,   223,   224,   225,   226,   227,     0,   228,   229,   230,
     231,   232,     0,   233,   234,   235,   236,   237,     1,     0,
       0,   238,   239,     0,     2,    47,     0,     0,   318,     0,
       0,     0,     0,     0,     5,     6,     7,     8,     9,    10,
      11,    12,   300,    13,    14,    15,    16,    17,     1,     0,
       0,     0,     0,     0,     2,    47,     0,     0,     0,     0,
       0,     0,     0,     0,     5,     6,     7,     8,     9,    10,
      11,    12,     1,    13,    14,    15,    16,    17,     2,   111,
       0,     0,     0,     0,     0,     0,     0,     0,     5,     6,
       7,     8,     0,     0,    11,    12,     0,     0,    14,    15,
      16,    17,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   273,   228,   229,   230,   231,   232,     0,   233,   234,
     235,   236,   237,     0,     0,     0,   238,   239,   219,   220,
     221,   222,   223,   224,   225,   226,   227,     0,   228,   229,
     230,   231,   232,     0,   233,   234,   235,   236,   237,     0,
       0,     0,   238,   239,   221,   222,   223,   224,   225,   226,
     227,     0,   228,   229,   230,   231,   232,     0,   233,   234,
     235,   236,   237,     0,     0,     0,   238,   239,   222,   223,
     224,   225,   226,   227,     0,   228,   229,   230,   231,   232,
       0,   233,   234,   235,   236,   237,     0,     0,     0,   238,
     239,   223,   224,   225,   226,   227,     0,   228,   229,   230,
     231,   232,     0,   233,   234,   235,   236,   237,     0,     0,
       0,   238,   239,   224,   225,   226,   227,     0,   228,   229,
     230,   231,   232,     0,   233,   234,   235,   236,   237,     0,
       0,     0,   238,   239,   225,   226,   227,     0,   228,   229,
     230,   231,   232,     0,   233,   234,   235,   236,   237,     0,
       0,     0,   238,   239,   227,     0,   228,   229,   230,   231,
     232,     0,   233,   234,   235,   236,   237,     0,     0,     0,
     238,   239
};

static const yytype_int16 yycheck[] =
{
       3,     4,     3,    42,    30,     3,    27,    27,     3,   166,
      12,     5,   190,    39,    39,     5,    27,   174,     5,    39,
      39,    12,     5,     5,    27,    12,   100,     5,    32,    41,
      42,    32,    35,     0,    17,     5,    39,    42,     0,    17,
       5,   258,    46,   260,    47,    46,     5,    86,   122,    42,
      52,    18,    19,    52,   128,    12,   130,     5,    17,    46,
      53,    52,    49,     5,    74,    58,    62,     4,    62,    17,
      70,     5,    62,   143,   100,    62,    71,    98,    98,    62,
     258,    86,   260,    70,    62,    88,     5,    74,   245,    72,
      93,   248,    62,   250,    72,    98,   122,    62,    17,   119,
     103,   126,   128,    62,   130,    42,   126,   126,   111,    70,
      27,    97,    41,    72,   117,    32,   119,    71,   104,   117,
     124,   147,   147,   126,    72,    40,     5,   147,   147,    46,
     200,   201,   202,   203,    12,   140,   141,   142,    17,   125,
     143,    42,    25,    26,   147,   170,   171,    30,     3,    86,
      70,    12,    53,    72,    74,   159,    93,    58,   159,    68,
      69,   164,    79,    74,   167,   190,   103,   170,   171,    71,
     190,   190,   139,    71,   194,   178,   215,    52,    95,    52,
      35,    98,   119,    52,    39,   189,   211,   190,   262,   156,
      71,   194,    47,    72,   321,   322,   323,   200,   201,   202,
     203,   206,    41,    41,     5,    12,   143,   124,   211,    12,
     215,    41,    12,    41,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,     5,     5,   260,   262,   204,   258,   258,
     260,   260,   159,   210,    41,    41,    86,    53,   251,    53,
     167,   168,    41,   170,   171,   258,   111,   260,    53,    74,
      41,   178,   117,   200,   201,   202,   203,   272,   273,    74,
      29,   126,   189,    41,    41,    34,   301,   194,   215,    38,
      62,    63,    64,    74,    41,    44,    68,    69,    41,   256,
     215,    37,   147,    35,   211,   126,   263,   264,   301,    98,
       5,    -1,    -1,   270,    -1,    -1,    11,    12,    -1,   164,
      -1,    -1,    17,   318,   251,    -1,    21,    22,    23,    24,
      25,    26,    27,    28,    -1,    30,    31,    32,    33,    34,
     247,    -1,    91,    38,    39,   190,    -1,    -1,   255,     5,
      -1,     3,     4,     5,    -1,    -1,    12,    -1,    10,    11,
      12,    17,    -1,   112,   113,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    72,    30,    31,
      32,    33,    34,    -1,    36,    37,    -1,    -1,    11,    -1,
      46,    -1,    -1,    49,   301,    -1,    -1,    49,    21,    22,
      23,    24,    -1,    -1,    -1,    -1,    62,    -1,    -1,    61,
      33,    -1,    -1,   258,    70,   260,    72,   166,    74,    -1,
      72,    73,    74,    -1,    -1,   174,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     3,     4,     5,   185,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    31,    32,    33,    34,    -1,    36,    37,    -1,    -1,
      -1,    -1,    -1,     3,     4,     5,    -1,     3,     4,    49,
      10,    -1,    -1,    -1,    10,    -1,    -1,    -1,    18,    19,
      20,    61,    18,    19,    20,    -1,   245,    -1,    -1,   248,
      -1,   250,    72,    73,    74,    -1,    36,    37,    -1,    -1,
      36,    37,     5,     5,    -1,    -1,    -1,     3,     4,    12,
      12,    -1,    -1,    49,    10,    -1,    -1,    -1,    -1,    -1,
      -1,    61,    18,    19,    20,    61,    -1,    -1,    -1,    -1,
      -1,    -1,    72,    73,    74,    -1,    72,    73,    74,    -1,
      36,    37,    -1,    46,    46,    -1,    49,    49,    -1,    -1,
      57,    58,    -1,    60,    61,    62,    63,    64,    -1,    62,
      62,    68,    69,     5,    -1,    61,    -1,    70,    70,    11,
      12,    74,    74,    15,    16,    -1,    72,    73,    74,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    31,
      32,    33,    34,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    -1,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    -1,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,     5,    70,    -1,
      72,    73,    74,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    31,    32,    33,    34,    -1,    -1,    -1,
       5,    -1,    -1,    41,    -1,    -1,    11,    12,    46,    -1,
      -1,    49,    17,    -1,    -1,    -1,    21,    22,    23,    24,
      25,    26,    27,    28,    62,    30,    31,    32,    33,    34,
      -1,     5,    70,    38,    39,    -1,    74,    11,    12,    -1,
      -1,    -1,    -1,    17,    -1,    -1,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    31,    32,    33,
      34,     5,    -1,    -1,    38,    39,    -1,    72,    12,    74,
       5,    -1,    -1,    17,    -1,    -1,    -1,    12,    -1,    -1,
      -1,    -1,    17,    60,    61,    62,    63,    64,    -1,    -1,
      -1,    68,    69,    -1,    -1,    -1,    -1,    -1,    72,    -1,
      74,    -1,    46,    -1,    -1,    49,    -1,    -1,    -1,    -1,
      -1,    46,    -1,    -1,    49,    -1,    15,    16,    62,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    70,    62,    72,    -1,
      74,    -1,    -1,    -1,    -1,    70,    35,    72,    -1,    74,
      -1,    -1,    -1,    42,    43,    -1,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      -1,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
       5,    70,    -1,    72,    73,    74,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    21,    22,    23,    24,
      25,    26,    27,    28,    -1,    30,    31,    32,    33,    34,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    54,    55,    56,
      57,    58,    -1,    60,    61,    62,    63,    64,     5,    -1,
      -1,    68,    69,    -1,    11,    12,    -1,    -1,    75,    -1,
      -1,    -1,    -1,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,     5,    -1,
      -1,    -1,    -1,    -1,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,     5,    30,    31,    32,    33,    34,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,    22,
      23,    24,    -1,    -1,    27,    28,    -1,    -1,    31,    32,
      33,    34,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    -1,    60,    61,
      62,    63,    64,    -1,    -1,    -1,    68,    69,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    55,
      56,    57,    58,    -1,    60,    61,    62,    63,    64,    -1,
      -1,    -1,    68,    69,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    55,    56,    57,    58,    -1,    60,    61,
      62,    63,    64,    -1,    -1,    -1,    68,    69,    47,    48,
      49,    50,    51,    52,    -1,    54,    55,    56,    57,    58,
      -1,    60,    61,    62,    63,    64,    -1,    -1,    -1,    68,
      69,    48,    49,    50,    51,    52,    -1,    54,    55,    56,
      57,    58,    -1,    60,    61,    62,    63,    64,    -1,    -1,
      -1,    68,    69,    49,    50,    51,    52,    -1,    54,    55,
      56,    57,    58,    -1,    60,    61,    62,    63,    64,    -1,
      -1,    -1,    68,    69,    50,    51,    52,    -1,    54,    55,
      56,    57,    58,    -1,    60,    61,    62,    63,    64,    -1,
      -1,    -1,    68,    69,    52,    -1,    54,    55,    56,    57,
      58,    -1,    60,    61,    62,    63,    64,    -1,    -1,    -1,
      68,    69
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     5,    11,    12,    17,    21,    22,    23,    24,    25,
      26,    27,    28,    30,    31,    32,    33,    34,    38,    39,
      72,    77,    78,    80,    81,    82,    83,    84,    85,    87,
      89,    90,    91,    92,    98,    99,   100,   101,   102,   105,
     109,    12,    52,    83,    87,    90,    91,    12,    15,    16,
      35,    42,    43,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    60,    61,    62,
      63,    64,    66,    67,    68,    70,    72,    73,    74,    91,
     105,    27,    78,    78,     5,     0,    52,     5,    12,    46,
      49,    62,    70,    74,    79,    91,    92,    96,   103,   104,
     107,   108,    99,    74,    96,     5,    83,    85,    92,    12,
      99,    12,    87,   102,    22,   100,    99,    12,    17,    74,
      86,    87,    88,    89,    91,    96,   103,   106,   107,   112,
     113,     3,     4,    10,    18,    19,    20,    36,    37,    49,
      61,    72,    73,    74,    80,    93,    94,   105,   114,    99,
      83,    70,    70,    62,    71,    41,    40,    93,    78,    91,
      97,    99,     3,    71,    12,    31,    41,    74,    91,    95,
     103,   105,   106,     5,    62,    92,    12,    79,    12,   108,
      96,   104,    79,    12,    12,    87,    99,    99,    87,    91,
     103,    96,   104,    79,    74,   106,   112,    96,    96,   104,
      74,    52,    52,    52,    74,    78,    72,   114,   114,   114,
      49,   105,   109,   114,   115,    42,    53,    58,   106,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    54,    55,
      56,    57,    58,    60,    61,    62,    63,    64,    68,    69,
      71,    71,    78,    53,    58,    62,    71,    91,    41,    97,
      41,    42,   106,    41,    97,    91,    12,    99,    74,    88,
     103,   110,   111,    12,    12,   109,   109,   109,   109,    78,
      74,    78,    41,    53,    41,    94,   114,   114,   114,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   114,   114,   114,   114,     5,     5,    97,    97,    97,
      29,   105,    78,   110,   110,    41,    96,   104,    78,    78,
      41,    53,    53,    53,    41,    78,   114,   114,    75,   106,
      41,    74,    74,    74,    41,   114,   115,   115,   115,    41,
      41,    41
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    76,    77,    78,    78,    78,    79,    79,    80,    80,
      80,    80,    80,    80,    81,    81,    82,    82,    82,    82,
      82,    82,    82,    82,    82,    82,    82,    82,    82,    82,
      82,    82,    82,    82,    82,    82,    82,    82,    82,    82,
      82,    82,    82,    82,    82,    82,    82,    82,    82,    82,
      83,    84,    84,    84,    84,    85,    85,    85,    85,    86,
      86,    87,    87,    87,    87,    88,    88,    89,    89,    90,
      90,    91,    91,    91,    91,    92,    92,    93,    93,    94,
      94,    94,    94,    94,    94,    95,    95,    95,    95,    95,
      96,    96,    96,    97,    97,    98,    98,    98,    99,    99,
     100,   100,   100,   100,   100,   100,   101,   101,   102,   102,
     102,   102,   102,   102,   102,   103,   103,   103,   103,   103,
     104,   104,   105,   105,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   106,   106,   106,   107,   107,   107,
     107,   108,   108,   108,   108,   108,   109,   109,   110,   110,
     111,   111,   111,   111,   112,   112,   112,   112,   112,   113,
     113,   113,   113,   113,   114,   115,   115,   115,   115,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   114
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     0,     2,     2,     3,
       3,     1,     2,     2,     2,     4,     2,     2,     4,     4,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     3,     2,     3,     3,
       2,     2,     1,     3,     2,     1,     4,     4,     2,     1,
       2,     2,     1,     2,     1,     1,     1,     1,     2,     2,
       1,     2,     3,     2,     3,     4,     4,     1,     3,     1,
       2,     2,     4,     1,     1,     1,     2,     3,     4,     3,
       4,     4,     3,     0,     1,     1,     1,     1,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     2,     1,     1,
       1,     2,     1,     1,     1,     2,     1,     1,     3,     4,
       2,     3,     2,     1,     3,     2,     2,     1,     3,     2,
       3,     2,     4,     3,     1,     2,     1,     3,     2,     2,
       1,     1,     2,     1,     4,     2,     1,     2,     2,     1,
       3,     2,     2,     1,     2,     1,     1,     4,     4,     4,
       2,     2,     2,     2,     3,     1,     3,     2,     4,     2,
       2,     2,     4,     7,     7,     7,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     5,     1,     1,     4,
       1,     1
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
        yyerror (state, YY_("syntax error: cannot back up")); \
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
                  Type, Value, state); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, cp_name_parser_YYSTYPE const * const yyvaluep, struct cpname_state *state)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  YYUSE (state);
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
yy_symbol_print (FILE *yyo, int yytype, cp_name_parser_YYSTYPE const * const yyvaluep, struct cpname_state *state)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyo, yytype, yyvaluep, state);
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
yy_reduce_print (yytype_int16 *yyssp, cp_name_parser_YYSTYPE *yyvsp, int yyrule, struct cpname_state *state)
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
                                              , state);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, state); \
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
yydestruct (const char *yymsg, int yytype, cp_name_parser_YYSTYPE *yyvaluep, struct cpname_state *state)
{
  YYUSE (yyvaluep);
  YYUSE (state);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (struct cpname_state *state)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static cp_name_parser_YYSTYPE yyval_default;)
cp_name_parser_YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

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
    cp_name_parser_YYSTYPE yyvsa[YYINITDEPTH];
    cp_name_parser_YYSTYPE *yyvs;
    cp_name_parser_YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  cp_name_parser_YYSTYPE yyval;

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
        cp_name_parser_YYSTYPE *yyvs1 = yyvs;
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
        union cp_name_parser_yyalloc *yyptr =
          (union cp_name_parser_yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
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
      yychar = yylex (&yylval, state);
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
  case 2:
#line 332 "cp-name-parser.y"
    {
			  state->global_result = (yyvsp[0].comp);

			  /* Avoid warning about "yynerrs" being unused.  */
			  (void) yynerrs;
			}
#line 1933 "cp-name-parser.c.tmp"
    break;

  case 6:
#line 349 "cp-name-parser.y"
    { (yyval.comp) = NULL; }
#line 1939 "cp-name-parser.c.tmp"
    break;

  case 7:
#line 351 "cp-name-parser.y"
    { (yyval.comp) = (yyvsp[0].comp); }
#line 1945 "cp-name-parser.c.tmp"
    break;

  case 8:
#line 358 "cp-name-parser.y"
    { (yyval.comp) = (yyvsp[0].nested).comp;
			  *(yyvsp[0].nested).last = (yyvsp[-1].comp);
			}
#line 1953 "cp-name-parser.c.tmp"
    break;

  case 9:
#line 367 "cp-name-parser.y"
    { (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_TYPED_NAME,
					  (yyvsp[-2].comp), (yyvsp[-1].nested).comp);
			  if ((yyvsp[0].comp))
			    (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_LOCAL_NAME,
						   (yyval.comp), (yyvsp[0].comp));
			}
#line 1964 "cp-name-parser.c.tmp"
    break;

  case 10:
#line 374 "cp-name-parser.y"
    { (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_TYPED_NAME, (yyvsp[-2].comp), (yyvsp[-1].nested).comp);
			  if ((yyvsp[0].comp)) (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_LOCAL_NAME, (yyval.comp), (yyvsp[0].comp)); }
#line 1971 "cp-name-parser.c.tmp"
    break;

  case 11:
#line 377 "cp-name-parser.y"
    {
			  /* This production is a hack to handle
			     something like "name::operator new[]" --
			     without arguments, this ordinarily would
			     not parse, but canonicalizing it is
			     important.  So we infer the "()" and then
			     remove it when converting back to string.
			     Note that this works because this
			     production is terminal.  */
			  demangle_component *comp
			    = state->fill_comp (DEMANGLE_COMPONENT_FUNCTION_TYPE,
						nullptr, nullptr);
			  (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_TYPED_NAME, (yyvsp[0].comp), comp);
			  state->demangle_info->added_parens = true;
			}
#line 1991 "cp-name-parser.c.tmp"
    break;

  case 12:
#line 394 "cp-name-parser.y"
    { (yyval.comp) = (yyvsp[-1].nested).comp;
			  if ((yyvsp[0].comp)) (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_LOCAL_NAME, (yyval.comp), (yyvsp[0].comp)); }
#line 1998 "cp-name-parser.c.tmp"
    break;

  case 13:
#line 397 "cp-name-parser.y"
    { if ((yyvsp[0].abstract).last)
			    {
			       /* First complete the abstract_declarator's type using
				  the typespec from the conversion_op_name.  */
			      *(yyvsp[0].abstract).last = *(yyvsp[-1].nested).last;
			      /* Then complete the conversion_op_name with the type.  */
			      *(yyvsp[-1].nested).last = (yyvsp[0].abstract).comp;
			    }
			  /* If we have an arglist, build a function type.  */
			  if ((yyvsp[0].abstract).fn.comp)
			    (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_TYPED_NAME, (yyvsp[-1].nested).comp, (yyvsp[0].abstract).fn.comp);
			  else
			    (yyval.comp) = (yyvsp[-1].nested).comp;
			  if ((yyvsp[0].abstract).start) (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_LOCAL_NAME, (yyval.comp), (yyvsp[0].abstract).start);
			}
#line 2018 "cp-name-parser.c.tmp"
    break;

  case 14:
#line 416 "cp-name-parser.y"
    { (yyval.comp) = state->fill_comp ((enum demangle_component_type) (yyvsp[-1].lval), (yyvsp[0].comp), NULL); }
#line 2024 "cp-name-parser.c.tmp"
    break;

  case 15:
#line 418 "cp-name-parser.y"
    { (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_CONSTRUCTION_VTABLE, (yyvsp[-2].comp), (yyvsp[0].comp)); }
#line 2030 "cp-name-parser.c.tmp"
    break;

  case 16:
#line 422 "cp-name-parser.y"
    {
			  /* Match the whitespacing of cplus_demangle_operators.
			     It would abort on unrecognized string otherwise.  */
			  (yyval.comp) = state->make_operator ("new", 3);
			}
#line 2040 "cp-name-parser.c.tmp"
    break;

  case 17:
#line 428 "cp-name-parser.y"
    {
			  /* Match the whitespacing of cplus_demangle_operators.
			     It would abort on unrecognized string otherwise.  */
			  (yyval.comp) = state->make_operator ("delete ", 1);
			}
#line 2050 "cp-name-parser.c.tmp"
    break;

  case 18:
#line 434 "cp-name-parser.y"
    {
			  /* Match the whitespacing of cplus_demangle_operators.
			     It would abort on unrecognized string otherwise.  */
			  (yyval.comp) = state->make_operator ("new[]", 3);
			}
#line 2060 "cp-name-parser.c.tmp"
    break;

  case 19:
#line 440 "cp-name-parser.y"
    {
			  /* Match the whitespacing of cplus_demangle_operators.
			     It would abort on unrecognized string otherwise.  */
			  (yyval.comp) = state->make_operator ("delete[] ", 1);
			}
#line 2070 "cp-name-parser.c.tmp"
    break;

  case 20:
#line 446 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("+", 2); }
#line 2076 "cp-name-parser.c.tmp"
    break;

  case 21:
#line 448 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("-", 2); }
#line 2082 "cp-name-parser.c.tmp"
    break;

  case 22:
#line 450 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("*", 2); }
#line 2088 "cp-name-parser.c.tmp"
    break;

  case 23:
#line 452 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("/", 2); }
#line 2094 "cp-name-parser.c.tmp"
    break;

  case 24:
#line 454 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("%", 2); }
#line 2100 "cp-name-parser.c.tmp"
    break;

  case 25:
#line 456 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("^", 2); }
#line 2106 "cp-name-parser.c.tmp"
    break;

  case 26:
#line 458 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("&", 2); }
#line 2112 "cp-name-parser.c.tmp"
    break;

  case 27:
#line 460 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("|", 2); }
#line 2118 "cp-name-parser.c.tmp"
    break;

  case 28:
#line 462 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("~", 1); }
#line 2124 "cp-name-parser.c.tmp"
    break;

  case 29:
#line 464 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("!", 1); }
#line 2130 "cp-name-parser.c.tmp"
    break;

  case 30:
#line 466 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("=", 2); }
#line 2136 "cp-name-parser.c.tmp"
    break;

  case 31:
#line 468 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("<", 2); }
#line 2142 "cp-name-parser.c.tmp"
    break;

  case 32:
#line 470 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator (">", 2); }
#line 2148 "cp-name-parser.c.tmp"
    break;

  case 33:
#line 472 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ((yyvsp[0].opname), 2); }
#line 2154 "cp-name-parser.c.tmp"
    break;

  case 34:
#line 474 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("<<", 2); }
#line 2160 "cp-name-parser.c.tmp"
    break;

  case 35:
#line 476 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator (">>", 2); }
#line 2166 "cp-name-parser.c.tmp"
    break;

  case 36:
#line 478 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("==", 2); }
#line 2172 "cp-name-parser.c.tmp"
    break;

  case 37:
#line 480 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("!=", 2); }
#line 2178 "cp-name-parser.c.tmp"
    break;

  case 38:
#line 482 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("<=", 2); }
#line 2184 "cp-name-parser.c.tmp"
    break;

  case 39:
#line 484 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator (">=", 2); }
#line 2190 "cp-name-parser.c.tmp"
    break;

  case 40:
#line 486 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("<=>", 2); }
#line 2196 "cp-name-parser.c.tmp"
    break;

  case 41:
#line 488 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("&&", 2); }
#line 2202 "cp-name-parser.c.tmp"
    break;

  case 42:
#line 490 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("||", 2); }
#line 2208 "cp-name-parser.c.tmp"
    break;

  case 43:
#line 492 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("++", 1); }
#line 2214 "cp-name-parser.c.tmp"
    break;

  case 44:
#line 494 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("--", 1); }
#line 2220 "cp-name-parser.c.tmp"
    break;

  case 45:
#line 496 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator (",", 2); }
#line 2226 "cp-name-parser.c.tmp"
    break;

  case 46:
#line 498 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("->*", 2); }
#line 2232 "cp-name-parser.c.tmp"
    break;

  case 47:
#line 500 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("->", 2); }
#line 2238 "cp-name-parser.c.tmp"
    break;

  case 48:
#line 502 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("()", 2); }
#line 2244 "cp-name-parser.c.tmp"
    break;

  case 49:
#line 504 "cp-name-parser.y"
    { (yyval.comp) = state->make_operator ("[]", 2); }
#line 2250 "cp-name-parser.c.tmp"
    break;

  case 50:
#line 512 "cp-name-parser.y"
    { (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_CONVERSION, (yyvsp[0].comp), NULL); }
#line 2256 "cp-name-parser.c.tmp"
    break;

  case 51:
#line 517 "cp-name-parser.y"
    { (yyval.nested).comp = (yyvsp[-1].nested1).comp;
			  d_right ((yyvsp[-1].nested1).last) = (yyvsp[0].comp);
			  (yyval.nested).last = &d_left ((yyvsp[0].comp));
			}
#line 2265 "cp-name-parser.c.tmp"
    break;

  case 52:
#line 522 "cp-name-parser.y"
    { (yyval.nested).comp = (yyvsp[0].comp);
			  (yyval.nested).last = &d_left ((yyvsp[0].comp));
			}
#line 2273 "cp-name-parser.c.tmp"
    break;

  case 53:
#line 526 "cp-name-parser.y"
    { (yyval.nested).comp = (yyvsp[-1].nested1).comp;
			  d_right ((yyvsp[-1].nested1).last) = (yyvsp[0].comp);
			  (yyval.nested).last = &d_left ((yyvsp[0].comp));
			}
#line 2282 "cp-name-parser.c.tmp"
    break;

  case 54:
#line 531 "cp-name-parser.y"
    { (yyval.nested).comp = (yyvsp[0].comp);
			  (yyval.nested).last = &d_left ((yyvsp[0].comp));
			}
#line 2290 "cp-name-parser.c.tmp"
    break;

  case 56:
#line 540 "cp-name-parser.y"
    { (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_TEMPLATE, (yyvsp[-3].comp), (yyvsp[-1].nested).comp); }
#line 2296 "cp-name-parser.c.tmp"
    break;

  case 57:
#line 542 "cp-name-parser.y"
    {
			  (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_TEMPLATE, (yyvsp[-3].comp), (yyvsp[-1].nested).comp);
			  state->unpush ('>');
			}
#line 2305 "cp-name-parser.c.tmp"
    break;

  case 58:
#line 547 "cp-name-parser.y"
    { (yyval.comp) = state->make_dtor (gnu_v3_complete_object_dtor, (yyvsp[0].comp)); }
#line 2311 "cp-name-parser.c.tmp"
    break;

  case 60:
#line 560 "cp-name-parser.y"
    { (yyval.comp) = (yyvsp[0].comp); }
#line 2317 "cp-name-parser.c.tmp"
    break;

  case 61:
#line 566 "cp-name-parser.y"
    { (yyval.comp) = (yyvsp[-1].nested1).comp; d_right ((yyvsp[-1].nested1).last) = (yyvsp[0].comp); }
#line 2323 "cp-name-parser.c.tmp"
    break;

  case 63:
#line 569 "cp-name-parser.y"
    { (yyval.comp) = (yyvsp[-1].nested1).comp; d_right ((yyvsp[-1].nested1).last) = (yyvsp[0].comp); }
#line 2329 "cp-name-parser.c.tmp"
    break;

  case 68:
#line 579 "cp-name-parser.y"
    { (yyval.comp) = (yyvsp[0].comp); }
#line 2335 "cp-name-parser.c.tmp"
    break;

  case 69:
#line 583 "cp-name-parser.y"
    { (yyval.comp) = (yyvsp[-1].nested1).comp; d_right ((yyvsp[-1].nested1).last) = (yyvsp[0].comp); }
#line 2341 "cp-name-parser.c.tmp"
    break;

  case 71:
#line 588 "cp-name-parser.y"
    { (yyval.nested1).comp = state->fill_comp (DEMANGLE_COMPONENT_QUAL_NAME, (yyvsp[-1].comp), NULL);
			  (yyval.nested1).last = (yyval.nested1).comp;
			}
#line 2349 "cp-name-parser.c.tmp"
    break;

  case 72:
#line 592 "cp-name-parser.y"
    { (yyval.nested1).comp = (yyvsp[-2].nested1).comp;
			  d_right ((yyvsp[-2].nested1).last) = state->fill_comp (DEMANGLE_COMPONENT_QUAL_NAME, (yyvsp[-1].comp), NULL);
			  (yyval.nested1).last = d_right ((yyvsp[-2].nested1).last);
			}
#line 2358 "cp-name-parser.c.tmp"
    break;

  case 73:
#line 597 "cp-name-parser.y"
    { (yyval.nested1).comp = state->fill_comp (DEMANGLE_COMPONENT_QUAL_NAME, (yyvsp[-1].comp), NULL);
			  (yyval.nested1).last = (yyval.nested1).comp;
			}
#line 2366 "cp-name-parser.c.tmp"
    break;

  case 74:
#line 601 "cp-name-parser.y"
    { (yyval.nested1).comp = (yyvsp[-2].nested1).comp;
			  d_right ((yyvsp[-2].nested1).last) = state->fill_comp (DEMANGLE_COMPONENT_QUAL_NAME, (yyvsp[-1].comp), NULL);
			  (yyval.nested1).last = d_right ((yyvsp[-2].nested1).last);
			}
#line 2375 "cp-name-parser.c.tmp"
    break;

  case 75:
#line 610 "cp-name-parser.y"
    { (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_TEMPLATE, (yyvsp[-3].comp), (yyvsp[-1].nested).comp); }
#line 2381 "cp-name-parser.c.tmp"
    break;

  case 76:
#line 612 "cp-name-parser.y"
    {
			  (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_TEMPLATE, (yyvsp[-3].comp), (yyvsp[-1].nested).comp);
			  state->unpush ('>');
			}
#line 2390 "cp-name-parser.c.tmp"
    break;

  case 77:
#line 619 "cp-name-parser.y"
    { (yyval.nested).comp = state->fill_comp (DEMANGLE_COMPONENT_TEMPLATE_ARGLIST, (yyvsp[0].comp), NULL);
			(yyval.nested).last = &d_right ((yyval.nested).comp); }
#line 2397 "cp-name-parser.c.tmp"
    break;

  case 78:
#line 622 "cp-name-parser.y"
    { (yyval.nested).comp = (yyvsp[-2].nested).comp;
			  *(yyvsp[-2].nested).last = state->fill_comp (DEMANGLE_COMPONENT_TEMPLATE_ARGLIST, (yyvsp[0].comp), NULL);
			  (yyval.nested).last = &d_right (*(yyvsp[-2].nested).last);
			}
#line 2406 "cp-name-parser.c.tmp"
    break;

  case 80:
#line 634 "cp-name-parser.y"
    { (yyval.comp) = (yyvsp[0].abstract).comp;
			  *(yyvsp[0].abstract).last = (yyvsp[-1].comp);
			}
#line 2414 "cp-name-parser.c.tmp"
    break;

  case 81:
#line 638 "cp-name-parser.y"
    { (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_UNARY, state->make_operator ("&", 1), (yyvsp[0].comp)); }
#line 2420 "cp-name-parser.c.tmp"
    break;

  case 82:
#line 640 "cp-name-parser.y"
    { (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_UNARY, state->make_operator ("&", 1), (yyvsp[-1].comp)); }
#line 2426 "cp-name-parser.c.tmp"
    break;

  case 85:
#line 646 "cp-name-parser.y"
    { (yyval.nested).comp = state->fill_comp (DEMANGLE_COMPONENT_ARGLIST, (yyvsp[0].comp), NULL);
			  (yyval.nested).last = &d_right ((yyval.nested).comp);
			}
#line 2434 "cp-name-parser.c.tmp"
    break;

  case 86:
#line 650 "cp-name-parser.y"
    { *(yyvsp[0].abstract).last = (yyvsp[-1].comp);
			  (yyval.nested).comp = state->fill_comp (DEMANGLE_COMPONENT_ARGLIST, (yyvsp[0].abstract).comp, NULL);
			  (yyval.nested).last = &d_right ((yyval.nested).comp);
			}
#line 2443 "cp-name-parser.c.tmp"
    break;

  case 87:
#line 655 "cp-name-parser.y"
    { *(yyvsp[-2].nested).last = state->fill_comp (DEMANGLE_COMPONENT_ARGLIST, (yyvsp[0].comp), NULL);
			  (yyval.nested).comp = (yyvsp[-2].nested).comp;
			  (yyval.nested).last = &d_right (*(yyvsp[-2].nested).last);
			}
#line 2452 "cp-name-parser.c.tmp"
    break;

  case 88:
#line 660 "cp-name-parser.y"
    { *(yyvsp[0].abstract).last = (yyvsp[-1].comp);
			  *(yyvsp[-3].nested).last = state->fill_comp (DEMANGLE_COMPONENT_ARGLIST, (yyvsp[0].abstract).comp, NULL);
			  (yyval.nested).comp = (yyvsp[-3].nested).comp;
			  (yyval.nested).last = &d_right (*(yyvsp[-3].nested).last);
			}
#line 2462 "cp-name-parser.c.tmp"
    break;

  case 89:
#line 666 "cp-name-parser.y"
    { *(yyvsp[-2].nested).last
			    = state->fill_comp (DEMANGLE_COMPONENT_ARGLIST,
					   state->make_builtin_type ("..."),
					   NULL);
			  (yyval.nested).comp = (yyvsp[-2].nested).comp;
			  (yyval.nested).last = &d_right (*(yyvsp[-2].nested).last);
			}
#line 2474 "cp-name-parser.c.tmp"
    break;

  case 90:
#line 676 "cp-name-parser.y"
    { (yyval.nested).comp = state->fill_comp (DEMANGLE_COMPONENT_FUNCTION_TYPE, NULL, (yyvsp[-2].nested).comp);
			  (yyval.nested).last = &d_left ((yyval.nested).comp);
			  (yyval.nested).comp = state->d_qualify ((yyval.nested).comp, (yyvsp[0].lval), 1); }
#line 2482 "cp-name-parser.c.tmp"
    break;

  case 91:
#line 680 "cp-name-parser.y"
    { (yyval.nested).comp = state->fill_comp (DEMANGLE_COMPONENT_FUNCTION_TYPE, NULL, NULL);
			  (yyval.nested).last = &d_left ((yyval.nested).comp);
			  (yyval.nested).comp = state->d_qualify ((yyval.nested).comp, (yyvsp[0].lval), 1); }
#line 2490 "cp-name-parser.c.tmp"
    break;

  case 92:
#line 684 "cp-name-parser.y"
    { (yyval.nested).comp = state->fill_comp (DEMANGLE_COMPONENT_FUNCTION_TYPE, NULL, NULL);
			  (yyval.nested).last = &d_left ((yyval.nested).comp);
			  (yyval.nested).comp = state->d_qualify ((yyval.nested).comp, (yyvsp[0].lval), 1); }
#line 2498 "cp-name-parser.c.tmp"
    break;

  case 93:
#line 691 "cp-name-parser.y"
    { (yyval.lval) = 0; }
#line 2504 "cp-name-parser.c.tmp"
    break;

  case 95:
#line 696 "cp-name-parser.y"
    { (yyval.lval) = QUAL_RESTRICT; }
#line 2510 "cp-name-parser.c.tmp"
    break;

  case 96:
#line 698 "cp-name-parser.y"
    { (yyval.lval) = QUAL_VOLATILE; }
#line 2516 "cp-name-parser.c.tmp"
    break;

  case 97:
#line 700 "cp-name-parser.y"
    { (yyval.lval) = QUAL_CONST; }
#line 2522 "cp-name-parser.c.tmp"
    break;

  case 99:
#line 705 "cp-name-parser.y"
    { (yyval.lval) = (yyvsp[-1].lval) | (yyvsp[0].lval); }
#line 2528 "cp-name-parser.c.tmp"
    break;

  case 100:
#line 712 "cp-name-parser.y"
    { (yyval.lval) = 0; }
#line 2534 "cp-name-parser.c.tmp"
    break;

  case 101:
#line 714 "cp-name-parser.y"
    { (yyval.lval) = INT_SIGNED; }
#line 2540 "cp-name-parser.c.tmp"
    break;

  case 102:
#line 716 "cp-name-parser.y"
    { (yyval.lval) = INT_UNSIGNED; }
#line 2546 "cp-name-parser.c.tmp"
    break;

  case 103:
#line 718 "cp-name-parser.y"
    { (yyval.lval) = INT_CHAR; }
#line 2552 "cp-name-parser.c.tmp"
    break;

  case 104:
#line 720 "cp-name-parser.y"
    { (yyval.lval) = INT_LONG; }
#line 2558 "cp-name-parser.c.tmp"
    break;

  case 105:
#line 722 "cp-name-parser.y"
    { (yyval.lval) = INT_SHORT; }
#line 2564 "cp-name-parser.c.tmp"
    break;

  case 107:
#line 727 "cp-name-parser.y"
    { (yyval.lval) = (yyvsp[-1].lval) | (yyvsp[0].lval); if ((yyvsp[-1].lval) & (yyvsp[0].lval) & INT_LONG) (yyval.lval) = (yyvsp[-1].lval) | INT_LLONG; }
#line 2570 "cp-name-parser.c.tmp"
    break;

  case 108:
#line 731 "cp-name-parser.y"
    { (yyval.comp) = state->d_int_type ((yyvsp[0].lval)); }
#line 2576 "cp-name-parser.c.tmp"
    break;

  case 109:
#line 733 "cp-name-parser.y"
    { (yyval.comp) = state->make_builtin_type ("float"); }
#line 2582 "cp-name-parser.c.tmp"
    break;

  case 110:
#line 735 "cp-name-parser.y"
    { (yyval.comp) = state->make_builtin_type ("double"); }
#line 2588 "cp-name-parser.c.tmp"
    break;

  case 111:
#line 737 "cp-name-parser.y"
    { (yyval.comp) = state->make_builtin_type ("long double"); }
#line 2594 "cp-name-parser.c.tmp"
    break;

  case 112:
#line 739 "cp-name-parser.y"
    { (yyval.comp) = state->make_builtin_type ("bool"); }
#line 2600 "cp-name-parser.c.tmp"
    break;

  case 113:
#line 741 "cp-name-parser.y"
    { (yyval.comp) = state->make_builtin_type ("wchar_t"); }
#line 2606 "cp-name-parser.c.tmp"
    break;

  case 114:
#line 743 "cp-name-parser.y"
    { (yyval.comp) = state->make_builtin_type ("void"); }
#line 2612 "cp-name-parser.c.tmp"
    break;

  case 115:
#line 747 "cp-name-parser.y"
    { (yyval.nested).comp = state->fill_comp (DEMANGLE_COMPONENT_POINTER, NULL, NULL);
			  (yyval.nested).last = &d_left ((yyval.nested).comp);
			  (yyval.nested).comp = state->d_qualify ((yyval.nested).comp, (yyvsp[0].lval), 0); }
#line 2620 "cp-name-parser.c.tmp"
    break;

  case 116:
#line 752 "cp-name-parser.y"
    { (yyval.nested).comp = state->fill_comp (DEMANGLE_COMPONENT_REFERENCE, NULL, NULL);
			  (yyval.nested).last = &d_left ((yyval.nested).comp); }
#line 2627 "cp-name-parser.c.tmp"
    break;

  case 117:
#line 755 "cp-name-parser.y"
    { (yyval.nested).comp = state->fill_comp (DEMANGLE_COMPONENT_RVALUE_REFERENCE, NULL, NULL);
			  (yyval.nested).last = &d_left ((yyval.nested).comp); }
#line 2634 "cp-name-parser.c.tmp"
    break;

  case 118:
#line 758 "cp-name-parser.y"
    { (yyval.nested).comp = state->fill_comp (DEMANGLE_COMPONENT_PTRMEM_TYPE, (yyvsp[-2].nested1).comp, NULL);
			  /* Convert the innermost DEMANGLE_COMPONENT_QUAL_NAME to a DEMANGLE_COMPONENT_NAME.  */
			  *(yyvsp[-2].nested1).last = *d_left ((yyvsp[-2].nested1).last);
			  (yyval.nested).last = &d_right ((yyval.nested).comp);
			  (yyval.nested).comp = state->d_qualify ((yyval.nested).comp, (yyvsp[0].lval), 0); }
#line 2644 "cp-name-parser.c.tmp"
    break;

  case 119:
#line 764 "cp-name-parser.y"
    { (yyval.nested).comp = state->fill_comp (DEMANGLE_COMPONENT_PTRMEM_TYPE, (yyvsp[-2].nested1).comp, NULL);
			  /* Convert the innermost DEMANGLE_COMPONENT_QUAL_NAME to a DEMANGLE_COMPONENT_NAME.  */
			  *(yyvsp[-2].nested1).last = *d_left ((yyvsp[-2].nested1).last);
			  (yyval.nested).last = &d_right ((yyval.nested).comp);
			  (yyval.nested).comp = state->d_qualify ((yyval.nested).comp, (yyvsp[0].lval), 0); }
#line 2654 "cp-name-parser.c.tmp"
    break;

  case 120:
#line 772 "cp-name-parser.y"
    { (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_ARRAY_TYPE, NULL, NULL); }
#line 2660 "cp-name-parser.c.tmp"
    break;

  case 121:
#line 774 "cp-name-parser.y"
    { (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_ARRAY_TYPE, (yyvsp[-1].comp), NULL); }
#line 2666 "cp-name-parser.c.tmp"
    break;

  case 122:
#line 788 "cp-name-parser.y"
    { (yyval.comp) = state->d_qualify ((yyvsp[-1].comp), (yyvsp[0].lval), 0); }
#line 2672 "cp-name-parser.c.tmp"
    break;

  case 124:
#line 791 "cp-name-parser.y"
    { (yyval.comp) = state->d_qualify ((yyvsp[-1].comp), (yyvsp[-2].lval) | (yyvsp[0].lval), 0); }
#line 2678 "cp-name-parser.c.tmp"
    break;

  case 125:
#line 793 "cp-name-parser.y"
    { (yyval.comp) = state->d_qualify ((yyvsp[0].comp), (yyvsp[-1].lval), 0); }
#line 2684 "cp-name-parser.c.tmp"
    break;

  case 126:
#line 796 "cp-name-parser.y"
    { (yyval.comp) = state->d_qualify ((yyvsp[-1].comp), (yyvsp[0].lval), 0); }
#line 2690 "cp-name-parser.c.tmp"
    break;

  case 128:
#line 799 "cp-name-parser.y"
    { (yyval.comp) = state->d_qualify ((yyvsp[-1].comp), (yyvsp[-2].lval) | (yyvsp[0].lval), 0); }
#line 2696 "cp-name-parser.c.tmp"
    break;

  case 129:
#line 801 "cp-name-parser.y"
    { (yyval.comp) = state->d_qualify ((yyvsp[0].comp), (yyvsp[-1].lval), 0); }
#line 2702 "cp-name-parser.c.tmp"
    break;

  case 130:
#line 804 "cp-name-parser.y"
    { (yyval.comp) = state->d_qualify ((yyvsp[-1].comp), (yyvsp[0].lval), 0); }
#line 2708 "cp-name-parser.c.tmp"
    break;

  case 131:
#line 806 "cp-name-parser.y"
    { (yyval.comp) = (yyvsp[0].comp); }
#line 2714 "cp-name-parser.c.tmp"
    break;

  case 132:
#line 808 "cp-name-parser.y"
    { (yyval.comp) = state->d_qualify ((yyvsp[-1].comp), (yyvsp[-3].lval) | (yyvsp[0].lval), 0); }
#line 2720 "cp-name-parser.c.tmp"
    break;

  case 133:
#line 810 "cp-name-parser.y"
    { (yyval.comp) = state->d_qualify ((yyvsp[0].comp), (yyvsp[-2].lval), 0); }
#line 2726 "cp-name-parser.c.tmp"
    break;

  case 134:
#line 815 "cp-name-parser.y"
    { (yyval.abstract).comp = (yyvsp[0].nested).comp; (yyval.abstract).last = (yyvsp[0].nested).last;
			  (yyval.abstract).fn.comp = NULL; (yyval.abstract).fn.last = NULL; }
#line 2733 "cp-name-parser.c.tmp"
    break;

  case 135:
#line 818 "cp-name-parser.y"
    { (yyval.abstract) = (yyvsp[0].abstract); (yyval.abstract).fn.comp = NULL; (yyval.abstract).fn.last = NULL;
			  if ((yyvsp[0].abstract).fn.comp) { (yyval.abstract).last = (yyvsp[0].abstract).fn.last; *(yyvsp[0].abstract).last = (yyvsp[0].abstract).fn.comp; }
			  *(yyval.abstract).last = (yyvsp[-1].nested).comp;
			  (yyval.abstract).last = (yyvsp[-1].nested).last; }
#line 2742 "cp-name-parser.c.tmp"
    break;

  case 136:
#line 823 "cp-name-parser.y"
    { (yyval.abstract).fn.comp = NULL; (yyval.abstract).fn.last = NULL;
			  if ((yyvsp[0].abstract).fn.comp) { (yyval.abstract).last = (yyvsp[0].abstract).fn.last; *(yyvsp[0].abstract).last = (yyvsp[0].abstract).fn.comp; }
			}
#line 2750 "cp-name-parser.c.tmp"
    break;

  case 137:
#line 830 "cp-name-parser.y"
    { (yyval.abstract) = (yyvsp[-1].abstract); (yyval.abstract).fn.comp = NULL; (yyval.abstract).fn.last = NULL; (yyval.abstract).fold_flag = 1;
			  if ((yyvsp[-1].abstract).fn.comp) { (yyval.abstract).last = (yyvsp[-1].abstract).fn.last; *(yyvsp[-1].abstract).last = (yyvsp[-1].abstract).fn.comp; }
			}
#line 2758 "cp-name-parser.c.tmp"
    break;

  case 138:
#line 834 "cp-name-parser.y"
    { (yyval.abstract).fold_flag = 0;
			  if ((yyvsp[-1].abstract).fn.comp) { (yyval.abstract).last = (yyvsp[-1].abstract).fn.last; *(yyvsp[-1].abstract).last = (yyvsp[-1].abstract).fn.comp; }
			  if ((yyvsp[-1].abstract).fold_flag)
			    {
			      *(yyval.abstract).last = (yyvsp[0].nested).comp;
			      (yyval.abstract).last = (yyvsp[0].nested).last;
			    }
			  else
			    (yyval.abstract).fn = (yyvsp[0].nested);
			}
#line 2773 "cp-name-parser.c.tmp"
    break;

  case 139:
#line 845 "cp-name-parser.y"
    { (yyval.abstract).fn.comp = NULL; (yyval.abstract).fn.last = NULL; (yyval.abstract).fold_flag = 0;
			  if ((yyvsp[-1].abstract).fn.comp) { (yyval.abstract).last = (yyvsp[-1].abstract).fn.last; *(yyvsp[-1].abstract).last = (yyvsp[-1].abstract).fn.comp; }
			  *(yyvsp[-1].abstract).last = (yyvsp[0].comp);
			  (yyval.abstract).last = &d_right ((yyvsp[0].comp));
			}
#line 2783 "cp-name-parser.c.tmp"
    break;

  case 140:
#line 851 "cp-name-parser.y"
    { (yyval.abstract).fn.comp = NULL; (yyval.abstract).fn.last = NULL; (yyval.abstract).fold_flag = 0;
			  (yyval.abstract).comp = (yyvsp[0].comp);
			  (yyval.abstract).last = &d_right ((yyvsp[0].comp));
			}
#line 2792 "cp-name-parser.c.tmp"
    break;

  case 141:
#line 869 "cp-name-parser.y"
    { (yyval.abstract).comp = (yyvsp[0].nested).comp; (yyval.abstract).last = (yyvsp[0].nested).last;
			  (yyval.abstract).fn.comp = NULL; (yyval.abstract).fn.last = NULL; (yyval.abstract).start = NULL; }
#line 2799 "cp-name-parser.c.tmp"
    break;

  case 142:
#line 872 "cp-name-parser.y"
    { (yyval.abstract) = (yyvsp[0].abstract);
			  if ((yyvsp[0].abstract).last)
			    *(yyval.abstract).last = (yyvsp[-1].nested).comp;
			  else
			    (yyval.abstract).comp = (yyvsp[-1].nested).comp;
			  (yyval.abstract).last = (yyvsp[-1].nested).last;
			}
#line 2811 "cp-name-parser.c.tmp"
    break;

  case 143:
#line 880 "cp-name-parser.y"
    { (yyval.abstract).comp = (yyvsp[0].abstract).comp; (yyval.abstract).last = (yyvsp[0].abstract).last; (yyval.abstract).fn = (yyvsp[0].abstract).fn; (yyval.abstract).start = NULL; }
#line 2817 "cp-name-parser.c.tmp"
    break;

  case 144:
#line 882 "cp-name-parser.y"
    { (yyval.abstract).start = (yyvsp[0].comp);
			  if ((yyvsp[-3].abstract).fn.comp) { (yyval.abstract).last = (yyvsp[-3].abstract).fn.last; *(yyvsp[-3].abstract).last = (yyvsp[-3].abstract).fn.comp; }
			  if ((yyvsp[-3].abstract).fold_flag)
			    {
			      *(yyval.abstract).last = (yyvsp[-2].nested).comp;
			      (yyval.abstract).last = (yyvsp[-2].nested).last;
			    }
			  else
			    (yyval.abstract).fn = (yyvsp[-2].nested);
			}
#line 2832 "cp-name-parser.c.tmp"
    break;

  case 145:
#line 893 "cp-name-parser.y"
    { (yyval.abstract).fn = (yyvsp[-1].nested);
			  (yyval.abstract).start = (yyvsp[0].comp);
			  (yyval.abstract).comp = NULL; (yyval.abstract).last = NULL;
			}
#line 2841 "cp-name-parser.c.tmp"
    break;

  case 147:
#line 901 "cp-name-parser.y"
    { (yyval.comp) = (yyvsp[0].abstract).comp;
			  *(yyvsp[0].abstract).last = (yyvsp[-1].comp);
			}
#line 2849 "cp-name-parser.c.tmp"
    break;

  case 148:
#line 907 "cp-name-parser.y"
    { (yyval.nested).comp = (yyvsp[0].nested).comp;
			  (yyval.nested).last = (yyvsp[-1].nested).last;
			  *(yyvsp[0].nested).last = (yyvsp[-1].nested).comp; }
#line 2857 "cp-name-parser.c.tmp"
    break;

  case 150:
#line 915 "cp-name-parser.y"
    { (yyval.nested) = (yyvsp[-1].nested); }
#line 2863 "cp-name-parser.c.tmp"
    break;

  case 151:
#line 917 "cp-name-parser.y"
    { (yyval.nested).comp = (yyvsp[-1].nested).comp;
			  *(yyvsp[-1].nested).last = (yyvsp[0].nested).comp;
			  (yyval.nested).last = (yyvsp[0].nested).last;
			}
#line 2872 "cp-name-parser.c.tmp"
    break;

  case 152:
#line 922 "cp-name-parser.y"
    { (yyval.nested).comp = (yyvsp[-1].nested).comp;
			  *(yyvsp[-1].nested).last = (yyvsp[0].comp);
			  (yyval.nested).last = &d_right ((yyvsp[0].comp));
			}
#line 2881 "cp-name-parser.c.tmp"
    break;

  case 153:
#line 927 "cp-name-parser.y"
    { (yyval.nested).comp = state->fill_comp (DEMANGLE_COMPONENT_TYPED_NAME, (yyvsp[0].comp), NULL);
			  (yyval.nested).last = &d_right ((yyval.nested).comp);
			}
#line 2889 "cp-name-parser.c.tmp"
    break;

  case 154:
#line 939 "cp-name-parser.y"
    { (yyval.nested).comp = (yyvsp[0].nested).comp;
			  (yyval.nested).last = (yyvsp[-1].nested).last;
			  *(yyvsp[0].nested).last = (yyvsp[-1].nested).comp; }
#line 2897 "cp-name-parser.c.tmp"
    break;

  case 155:
#line 943 "cp-name-parser.y"
    { (yyval.nested).comp = state->fill_comp (DEMANGLE_COMPONENT_TYPED_NAME, (yyvsp[0].comp), NULL);
			  (yyval.nested).last = &d_right ((yyval.nested).comp);
			}
#line 2905 "cp-name-parser.c.tmp"
    break;

  case 157:
#line 955 "cp-name-parser.y"
    { (yyval.nested).comp = state->fill_comp (DEMANGLE_COMPONENT_TYPED_NAME, (yyvsp[-3].comp), (yyvsp[-2].nested).comp);
			  (yyval.nested).last = (yyvsp[-2].nested).last;
			  (yyval.nested).comp = state->fill_comp (DEMANGLE_COMPONENT_LOCAL_NAME, (yyval.nested).comp, (yyvsp[0].comp));
			}
#line 2914 "cp-name-parser.c.tmp"
    break;

  case 158:
#line 960 "cp-name-parser.y"
    { (yyval.nested).comp = (yyvsp[-3].nested).comp;
			  *(yyvsp[-3].nested).last = (yyvsp[-2].nested).comp;
			  (yyval.nested).last = (yyvsp[-2].nested).last;
			  (yyval.nested).comp = state->fill_comp (DEMANGLE_COMPONENT_LOCAL_NAME, (yyval.nested).comp, (yyvsp[0].comp));
			}
#line 2924 "cp-name-parser.c.tmp"
    break;

  case 159:
#line 969 "cp-name-parser.y"
    { (yyval.nested).comp = (yyvsp[-1].nested).comp;
			  (yyval.nested).last = (yyvsp[-2].nested).last;
			  *(yyvsp[-1].nested).last = (yyvsp[-2].nested).comp; }
#line 2932 "cp-name-parser.c.tmp"
    break;

  case 160:
#line 973 "cp-name-parser.y"
    { (yyval.nested).comp = (yyvsp[-1].nested).comp;
			  *(yyvsp[-1].nested).last = (yyvsp[0].nested).comp;
			  (yyval.nested).last = (yyvsp[0].nested).last;
			}
#line 2941 "cp-name-parser.c.tmp"
    break;

  case 161:
#line 978 "cp-name-parser.y"
    { (yyval.nested).comp = (yyvsp[-1].nested).comp;
			  *(yyvsp[-1].nested).last = (yyvsp[0].comp);
			  (yyval.nested).last = &d_right ((yyvsp[0].comp));
			}
#line 2950 "cp-name-parser.c.tmp"
    break;

  case 162:
#line 983 "cp-name-parser.y"
    { (yyval.nested).comp = state->fill_comp (DEMANGLE_COMPONENT_TYPED_NAME, (yyvsp[-1].comp), (yyvsp[0].nested).comp);
			  (yyval.nested).last = (yyvsp[0].nested).last;
			}
#line 2958 "cp-name-parser.c.tmp"
    break;

  case 163:
#line 987 "cp-name-parser.y"
    { (yyval.nested).comp = state->fill_comp (DEMANGLE_COMPONENT_TYPED_NAME, (yyvsp[-1].comp), (yyvsp[0].comp));
			  (yyval.nested).last = &d_right ((yyvsp[0].comp));
			}
#line 2966 "cp-name-parser.c.tmp"
    break;

  case 164:
#line 993 "cp-name-parser.y"
    { (yyval.comp) = (yyvsp[-1].comp); }
#line 2972 "cp-name-parser.c.tmp"
    break;

  case 166:
#line 1002 "cp-name-parser.y"
    { (yyval.comp) = state->d_binary (">", (yyvsp[-2].comp), (yyvsp[0].comp)); }
#line 2978 "cp-name-parser.c.tmp"
    break;

  case 167:
#line 1009 "cp-name-parser.y"
    { (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_UNARY, state->make_operator ("&", 1), (yyvsp[0].comp)); }
#line 2984 "cp-name-parser.c.tmp"
    break;

  case 168:
#line 1011 "cp-name-parser.y"
    { (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_UNARY, state->make_operator ("&", 1), (yyvsp[-1].comp)); }
#line 2990 "cp-name-parser.c.tmp"
    break;

  case 169:
#line 1016 "cp-name-parser.y"
    { (yyval.comp) = state->d_unary ("-", (yyvsp[0].comp)); }
#line 2996 "cp-name-parser.c.tmp"
    break;

  case 170:
#line 1020 "cp-name-parser.y"
    { (yyval.comp) = state->d_unary ("!", (yyvsp[0].comp)); }
#line 3002 "cp-name-parser.c.tmp"
    break;

  case 171:
#line 1024 "cp-name-parser.y"
    { (yyval.comp) = state->d_unary ("~", (yyvsp[0].comp)); }
#line 3008 "cp-name-parser.c.tmp"
    break;

  case 172:
#line 1031 "cp-name-parser.y"
    { if ((yyvsp[0].comp)->type == DEMANGLE_COMPONENT_LITERAL
		      || (yyvsp[0].comp)->type == DEMANGLE_COMPONENT_LITERAL_NEG)
		    {
		      (yyval.comp) = (yyvsp[0].comp);
		      d_left ((yyvsp[0].comp)) = (yyvsp[-2].comp);
		    }
		  else
		    (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_UNARY,
				      state->fill_comp (DEMANGLE_COMPONENT_CAST, (yyvsp[-2].comp), NULL),
				      (yyvsp[0].comp));
		}
#line 3024 "cp-name-parser.c.tmp"
    break;

  case 173:
#line 1047 "cp-name-parser.y"
    { (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_UNARY,
				    state->fill_comp (DEMANGLE_COMPONENT_CAST, (yyvsp[-4].comp), NULL),
				    (yyvsp[-1].comp));
		}
#line 3033 "cp-name-parser.c.tmp"
    break;

  case 174:
#line 1054 "cp-name-parser.y"
    { (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_UNARY,
				    state->fill_comp (DEMANGLE_COMPONENT_CAST, (yyvsp[-4].comp), NULL),
				    (yyvsp[-1].comp));
		}
#line 3042 "cp-name-parser.c.tmp"
    break;

  case 175:
#line 1061 "cp-name-parser.y"
    { (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_UNARY,
				    state->fill_comp (DEMANGLE_COMPONENT_CAST, (yyvsp[-4].comp), NULL),
				    (yyvsp[-1].comp));
		}
#line 3051 "cp-name-parser.c.tmp"
    break;

  case 176:
#line 1080 "cp-name-parser.y"
    { (yyval.comp) = state->d_binary ("*", (yyvsp[-2].comp), (yyvsp[0].comp)); }
#line 3057 "cp-name-parser.c.tmp"
    break;

  case 177:
#line 1084 "cp-name-parser.y"
    { (yyval.comp) = state->d_binary ("/", (yyvsp[-2].comp), (yyvsp[0].comp)); }
#line 3063 "cp-name-parser.c.tmp"
    break;

  case 178:
#line 1088 "cp-name-parser.y"
    { (yyval.comp) = state->d_binary ("%", (yyvsp[-2].comp), (yyvsp[0].comp)); }
#line 3069 "cp-name-parser.c.tmp"
    break;

  case 179:
#line 1092 "cp-name-parser.y"
    { (yyval.comp) = state->d_binary ("+", (yyvsp[-2].comp), (yyvsp[0].comp)); }
#line 3075 "cp-name-parser.c.tmp"
    break;

  case 180:
#line 1096 "cp-name-parser.y"
    { (yyval.comp) = state->d_binary ("-", (yyvsp[-2].comp), (yyvsp[0].comp)); }
#line 3081 "cp-name-parser.c.tmp"
    break;

  case 181:
#line 1100 "cp-name-parser.y"
    { (yyval.comp) = state->d_binary ("<<", (yyvsp[-2].comp), (yyvsp[0].comp)); }
#line 3087 "cp-name-parser.c.tmp"
    break;

  case 182:
#line 1104 "cp-name-parser.y"
    { (yyval.comp) = state->d_binary (">>", (yyvsp[-2].comp), (yyvsp[0].comp)); }
#line 3093 "cp-name-parser.c.tmp"
    break;

  case 183:
#line 1108 "cp-name-parser.y"
    { (yyval.comp) = state->d_binary ("==", (yyvsp[-2].comp), (yyvsp[0].comp)); }
#line 3099 "cp-name-parser.c.tmp"
    break;

  case 184:
#line 1112 "cp-name-parser.y"
    { (yyval.comp) = state->d_binary ("!=", (yyvsp[-2].comp), (yyvsp[0].comp)); }
#line 3105 "cp-name-parser.c.tmp"
    break;

  case 185:
#line 1116 "cp-name-parser.y"
    { (yyval.comp) = state->d_binary ("<=", (yyvsp[-2].comp), (yyvsp[0].comp)); }
#line 3111 "cp-name-parser.c.tmp"
    break;

  case 186:
#line 1120 "cp-name-parser.y"
    { (yyval.comp) = state->d_binary (">=", (yyvsp[-2].comp), (yyvsp[0].comp)); }
#line 3117 "cp-name-parser.c.tmp"
    break;

  case 187:
#line 1124 "cp-name-parser.y"
    { (yyval.comp) = state->d_binary ("<=>", (yyvsp[-2].comp), (yyvsp[0].comp)); }
#line 3123 "cp-name-parser.c.tmp"
    break;

  case 188:
#line 1128 "cp-name-parser.y"
    { (yyval.comp) = state->d_binary ("<", (yyvsp[-2].comp), (yyvsp[0].comp)); }
#line 3129 "cp-name-parser.c.tmp"
    break;

  case 189:
#line 1132 "cp-name-parser.y"
    { (yyval.comp) = state->d_binary ("&", (yyvsp[-2].comp), (yyvsp[0].comp)); }
#line 3135 "cp-name-parser.c.tmp"
    break;

  case 190:
#line 1136 "cp-name-parser.y"
    { (yyval.comp) = state->d_binary ("^", (yyvsp[-2].comp), (yyvsp[0].comp)); }
#line 3141 "cp-name-parser.c.tmp"
    break;

  case 191:
#line 1140 "cp-name-parser.y"
    { (yyval.comp) = state->d_binary ("|", (yyvsp[-2].comp), (yyvsp[0].comp)); }
#line 3147 "cp-name-parser.c.tmp"
    break;

  case 192:
#line 1144 "cp-name-parser.y"
    { (yyval.comp) = state->d_binary ("&&", (yyvsp[-2].comp), (yyvsp[0].comp)); }
#line 3153 "cp-name-parser.c.tmp"
    break;

  case 193:
#line 1148 "cp-name-parser.y"
    { (yyval.comp) = state->d_binary ("||", (yyvsp[-2].comp), (yyvsp[0].comp)); }
#line 3159 "cp-name-parser.c.tmp"
    break;

  case 194:
#line 1153 "cp-name-parser.y"
    { (yyval.comp) = state->d_binary ("->", (yyvsp[-2].comp), (yyvsp[0].comp)); }
#line 3165 "cp-name-parser.c.tmp"
    break;

  case 195:
#line 1157 "cp-name-parser.y"
    { (yyval.comp) = state->d_binary (".", (yyvsp[-2].comp), (yyvsp[0].comp)); }
#line 3171 "cp-name-parser.c.tmp"
    break;

  case 196:
#line 1161 "cp-name-parser.y"
    { (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_TRINARY, state->make_operator ("?", 3),
				    state->fill_comp (DEMANGLE_COMPONENT_TRINARY_ARG1, (yyvsp[-4].comp),
						 state->fill_comp (DEMANGLE_COMPONENT_TRINARY_ARG2, (yyvsp[-2].comp), (yyvsp[0].comp))));
		}
#line 3180 "cp-name-parser.c.tmp"
    break;

  case 199:
#line 1175 "cp-name-parser.y"
    {
		  /* Match the whitespacing of cplus_demangle_operators.
		     It would abort on unrecognized string otherwise.  */
		  (yyval.comp) = state->d_unary ("sizeof ", (yyvsp[-1].comp));
		}
#line 3190 "cp-name-parser.c.tmp"
    break;

  case 200:
#line 1184 "cp-name-parser.y"
    { struct demangle_component *i;
		  i = state->make_name ("1", 1);
		  (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_LITERAL,
				    state->make_builtin_type ( "bool"),
				    i);
		}
#line 3201 "cp-name-parser.c.tmp"
    break;

  case 201:
#line 1193 "cp-name-parser.y"
    { struct demangle_component *i;
		  i = state->make_name ("0", 1);
		  (yyval.comp) = state->fill_comp (DEMANGLE_COMPONENT_LITERAL,
				    state->make_builtin_type ("bool"),
				    i);
		}
#line 3212 "cp-name-parser.c.tmp"
    break;


#line 3216 "cp-name-parser.c.tmp"

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
      yyerror (state, YY_("syntax error"));
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
        yyerror (state, yymsgp);
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
                      yytoken, &yylval, state);
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
                  yystos[yystate], yyvsp, state);
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
  yyerror (state, YY_("memory exhausted"));
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
                  yytoken, &yylval, state);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, state);
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
#line 1203 "cp-name-parser.y"


/* Apply QUALIFIERS to LHS and return a qualified component.  IS_METHOD
   is set if LHS is a method, in which case the qualifiers are logically
   applied to "this".  We apply qualifiers in a consistent order; LHS
   may already be qualified; duplicate qualifiers are not created.  */

struct demangle_component *
cpname_state::d_qualify (struct demangle_component *lhs, int qualifiers,
			 int is_method)
{
  struct demangle_component **inner_p;
  enum demangle_component_type type;

  /* For now the order is CONST (innermost), VOLATILE, RESTRICT.  */

#define HANDLE_QUAL(TYPE, MTYPE, QUAL)				\
  if ((qualifiers & QUAL) && (type != TYPE) && (type != MTYPE))	\
    {								\
      *inner_p = fill_comp (is_method ? MTYPE : TYPE,		\
			    *inner_p, NULL);			\
      inner_p = &d_left (*inner_p);				\
      type = (*inner_p)->type;					\
    }								\
  else if (type == TYPE || type == MTYPE)			\
    {								\
      inner_p = &d_left (*inner_p);				\
      type = (*inner_p)->type;					\
    }

  inner_p = &lhs;

  type = (*inner_p)->type;

  HANDLE_QUAL (DEMANGLE_COMPONENT_RESTRICT, DEMANGLE_COMPONENT_RESTRICT_THIS, QUAL_RESTRICT);
  HANDLE_QUAL (DEMANGLE_COMPONENT_VOLATILE, DEMANGLE_COMPONENT_VOLATILE_THIS, QUAL_VOLATILE);
  HANDLE_QUAL (DEMANGLE_COMPONENT_CONST, DEMANGLE_COMPONENT_CONST_THIS, QUAL_CONST);

  return lhs;
}

/* Return a builtin type corresponding to FLAGS.  */

struct demangle_component *
cpname_state::d_int_type (int flags)
{
  const char *name;

  switch (flags)
    {
    case INT_SIGNED | INT_CHAR:
      name = "signed char";
      break;
    case INT_CHAR:
      name = "char";
      break;
    case INT_UNSIGNED | INT_CHAR:
      name = "unsigned char";
      break;
    case 0:
    case INT_SIGNED:
      name = "int";
      break;
    case INT_UNSIGNED:
      name = "unsigned int";
      break;
    case INT_LONG:
    case INT_SIGNED | INT_LONG:
      name = "long";
      break;
    case INT_UNSIGNED | INT_LONG:
      name = "unsigned long";
      break;
    case INT_SHORT:
    case INT_SIGNED | INT_SHORT:
      name = "short";
      break;
    case INT_UNSIGNED | INT_SHORT:
      name = "unsigned short";
      break;
    case INT_LLONG | INT_LONG:
    case INT_SIGNED | INT_LLONG | INT_LONG:
      name = "long long";
      break;
    case INT_UNSIGNED | INT_LLONG | INT_LONG:
      name = "unsigned long long";
      break;
    default:
      return NULL;
    }

  return make_builtin_type (name);
}

/* Wrapper to create a unary operation.  */

struct demangle_component *
cpname_state::d_unary (const char *name, struct demangle_component *lhs)
{
  return fill_comp (DEMANGLE_COMPONENT_UNARY, make_operator (name, 1), lhs);
}

/* Wrapper to create a binary operation.  */

struct demangle_component *
cpname_state::d_binary (const char *name, struct demangle_component *lhs,
			struct demangle_component *rhs)
{
  return fill_comp (DEMANGLE_COMPONENT_BINARY, make_operator (name, 2),
		    fill_comp (DEMANGLE_COMPONENT_BINARY_ARGS, lhs, rhs));
}

/* Find the end of a symbol name starting at LEXPTR.  */

static const char *
symbol_end (const char *lexptr)
{
  const char *p = lexptr;

  while (*p && (c_ident_is_alnum (*p) || *p == '_' || *p == '$' || *p == '.'))
    p++;

  return p;
}

/* Take care of parsing a number (anything that starts with a digit).
   The number starts at P and contains LEN characters.  Store the result in
   YYLVAL.  */

int
cpname_state::parse_number (const char *p, int len, int parsed_float,
			    cp_name_parser_YYSTYPE *lvalp)
{
  int unsigned_p = 0;

  /* Number of "L" suffixes encountered.  */
  int long_p = 0;

  struct demangle_component *type, *name;
  enum demangle_component_type literal_type;

  if (p[0] == '-')
    {
      literal_type = DEMANGLE_COMPONENT_LITERAL_NEG;
      p++;
      len--;
    }
  else
    literal_type = DEMANGLE_COMPONENT_LITERAL;

  if (parsed_float)
    {
      /* It's a float since it contains a point or an exponent.  */
      char c;

      /* The GDB lexer checks the result of scanf at this point.  Not doing
	 this leaves our error checking slightly weaker but only for invalid
	 data.  */

      /* See if it has `f' or `l' suffix (float or long double).  */

      c = c_tolower (p[len - 1]);

      if (c == 'f')
	{
	  len--;
	  type = make_builtin_type ("float");
	}
      else if (c == 'l')
	{
	  len--;
	  type = make_builtin_type ("long double");
	}
      else if (c_isdigit (c) || c == '.')
	type = make_builtin_type ("double");
      else
	return ERROR;

      name = make_name (p, len);
      lvalp->comp = fill_comp (literal_type, type, name);

      return FLOAT;
    }

  /* Note that we do not automatically generate unsigned types.  This
     can't be done because we don't have access to the gdbarch
     here.  */

  int base = 10;
  if (len > 1 && p[0] == '0')
    {
      if (p[1] == 'x' || p[1] == 'X')
	{
	  base = 16;
	  p += 2;
	  len -= 2;
	}
      else if (p[1] == 'b' || p[1] == 'B')
	{
	  base = 2;
	  p += 2;
	  len -= 2;
	}
      else if (p[1] == 'd' || p[1] == 'D' || p[1] == 't' || p[1] == 'T')
	{
	  /* Apparently gdb extensions.  */
	  base = 10;
	  p += 2;
	  len -= 2;
	}
      else
	base = 8;
    }

  long_p = 0;
  unsigned_p = 0;
  while (len > 0)
    {
      if (p[len - 1] == 'l' || p[len - 1] == 'L')
	{
	  len--;
	  long_p++;
	  continue;
	}
      if (p[len - 1] == 'u' || p[len - 1] == 'U')
	{
	  len--;
	  unsigned_p++;
	  continue;
	}
      break;
    }

  /* Use gdb_mpz here in case a 128-bit value appears.  */
  gdb_mpz value (0);
  for (int off = 0; off < len; ++off)
    {
      int dig;
      if (c_isdigit (p[off]))
	dig = p[off] - '0';
      else
	dig = c_tolower (p[off]) - 'a' + 10;
      if (dig >= base)
	return ERROR;
      value *= base;
      value += dig;
    }

  std::string printed = value.str ();
  const char *copy = obstack_strdup (&demangle_info->obstack, printed);

  if (long_p == 0)
    {
      if (unsigned_p)
	type = make_builtin_type ("unsigned int");
      else
	type = make_builtin_type ("int");
    }
  else if (long_p == 1)
    {
      if (unsigned_p)
	type = make_builtin_type ("unsigned long");
      else
	type = make_builtin_type ("long");
    }
  else
    {
      if (unsigned_p)
	type = make_builtin_type ("unsigned long long");
      else
	type = make_builtin_type ("long long");
    }

  name = make_name (copy, strlen (copy));
  lvalp->comp = fill_comp (literal_type, type, name);

  return INT;
}

static const char backslashable[] = "abefnrtv";
static const char represented[] = "\a\b\e\f\n\r\t\v";

/* Translate the backslash the way we would in the host character set.  */
static int
c_parse_backslash (int host_char, int *target_char)
{
  const char *ix;
  ix = strchr (backslashable, host_char);
  if (! ix)
    return 0;
  else
    *target_char = represented[ix - backslashable];
  return 1;
}

/* Parse a C escape sequence.  STRING_PTR points to a variable
   containing a pointer to the string to parse.  That pointer
   should point to the character after the \.  That pointer
   is updated past the characters we use.  The value of the
   escape sequence is returned.

   A negative value means the sequence \ newline was seen,
   which is supposed to be equivalent to nothing at all.

   If \ is followed by a null character, we return a negative
   value and leave the string pointer pointing at the null character.

   If \ is followed by 000, we return 0 and leave the string pointer
   after the zeros.  A value of 0 does not mean end of string.  */

static int
cp_parse_escape (const char **string_ptr)
{
  int target_char;
  int c = *(*string_ptr)++;
  if (c_parse_backslash (c, &target_char))
    return target_char;
  else
    switch (c)
      {
      case '\n':
	return -2;
      case 0:
	(*string_ptr)--;
	return 0;
      case '^':
	{
	  c = *(*string_ptr)++;

	  if (c == '?')
	    return 0177;
	  else if (c == '\\')
	    target_char = cp_parse_escape (string_ptr);
	  else
	    target_char = c;

	  /* Now target_char is something like `c', and we want to find
	     its control-character equivalent.  */
	  target_char = target_char & 037;

	  return target_char;
	}

      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
	{
	  int i = c - '0';
	  int count = 0;
	  while (++count < 3)
	    {
	      c = (**string_ptr);
	      if (c >= '0' && c <= '7')
		{
		  (*string_ptr)++;
		  i *= 8;
		  i += c - '0';
		}
	      else
		{
		  break;
		}
	    }
	  return i;
	}
      default:
	return c;
      }
}

#define HANDLE_SPECIAL(string, comp)				\
  if (startswith (tokstart, string))				\
    {								\
      state->lexptr = tokstart + sizeof (string) - 1;			\
      lvalp->lval = comp;					\
      return DEMANGLER_SPECIAL;					\
    }

#define HANDLE_TOKEN2(string, token)			\
  if (state->lexptr[1] == string[1])				\
    {							\
      state->lexptr += 2;					\
      lvalp->opname = string;				\
      return token;					\
    }

#define HANDLE_TOKEN3(string, token)			\
  if (state->lexptr[1] == string[1] && state->lexptr[2] == string[2])	\
    {							\
      state->lexptr += 3;					\
      lvalp->opname = string;				\
      return token;					\
    }

/* Read one token, getting characters through LEXPTR.  */

static int
yylex (cp_name_parser_YYSTYPE *lvalp, cpname_state *state)
{
  int c;
  int namelen;
  const char *tokstart;
  char *copy;

 retry:
  state->prev_lexptr = state->lexptr;
  tokstart = state->lexptr;

  switch (c = *tokstart)
    {
    case 0:
      return 0;

    case ' ':
    case '\t':
    case '\n':
      state->lexptr++;
      goto retry;

    case '\'':
      /* We either have a character constant ('0' or '\177' for example)
	 or we have a quoted symbol reference ('foo(int,int)' in C++
	 for example). */
      state->lexptr++;
      c = *state->lexptr++;
      if (c == '\\')
	c = cp_parse_escape (&state->lexptr);
      else if (c == '\'')
	{
	  yyerror (state, _("empty character constant"));
	  return ERROR;
	}

      /* We over-allocate here, but it doesn't really matter . */
      copy = (char *) obstack_alloc (&state->demangle_info->obstack, 30);
      xsnprintf (copy, 30, "%d", c);

      c = *state->lexptr++;
      if (c != '\'')
	{
	  yyerror (state, _("invalid character constant"));
	  return ERROR;
	}

      lvalp->comp
	= state->fill_comp (DEMANGLE_COMPONENT_LITERAL,
			    state->make_builtin_type ("char"),
			    state->make_name (copy, strlen (copy)));

      return INT;

    case '(':
      if (startswith (tokstart, "(anonymous namespace)"))
	{
	  state->lexptr += 21;
	  lvalp->comp = state->make_name ("(anonymous namespace)",
					  sizeof "(anonymous namespace)" - 1);
	  return NAME;
	}
	[[fallthrough]];

    case ')':
    case ',':
      state->lexptr++;
      return c;

    case '.':
      if (state->lexptr[1] == '.' && state->lexptr[2] == '.')
	{
	  state->lexptr += 3;
	  return ELLIPSIS;
	}

      /* Might be a floating point number.  */
      if (state->lexptr[1] < '0' || state->lexptr[1] > '9')
	goto symbol;		/* Nope, must be a symbol. */

      goto try_number;

    case '-':
      HANDLE_TOKEN2 ("-=", ASSIGN_MODIFY);
      HANDLE_TOKEN2 ("--", DECREMENT);
      HANDLE_TOKEN2 ("->", ARROW);

      /* For construction vtables.  This is kind of hokey.  */
      if (startswith (tokstart, "-in-"))
	{
	  state->lexptr += 4;
	  return CONSTRUCTION_IN;
	}

      if (state->lexptr[1] < '0' || state->lexptr[1] > '9')
	{
	  state->lexptr++;
	  return '-';
	}

    try_number:
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
	int got_dot = 0, got_e = 0, toktype;
	const char *p = tokstart;
	int hex = 0;

	if (c == '-')
	  p++;

	if (c == '0' && (p[1] == 'x' || p[1] == 'X'))
	  {
	    p += 2;
	    hex = 1;
	  }
	else if (c == '0' && (p[1]=='t' || p[1]=='T' || p[1]=='d' || p[1]=='D'))
	  {
	    p += 2;
	    hex = 0;
	  }

	/* If the token includes the C++14 digits separator, we make a
	   copy so that we don't have to handle the separator in
	   parse_number.  */
	std::optional<std::string> no_tick;
	for (;; ++p)
	  {
	    /* This test includes !hex because 'e' is a valid hex digit
	       and thus does not indicate a floating point number when
	       the radix is hex.  */
	    if (!hex && !got_e && (*p == 'e' || *p == 'E'))
	      got_dot = got_e = 1;
	    /* This test does not include !hex, because a '.' always indicates
	       a decimal floating point number regardless of the radix.

	       NOTE drow/2005-03-09: This comment is not accurate in C99;
	       however, it's not clear that all the floating point support
	       in this file is doing any good here.  */
	    else if (!got_dot && *p == '.')
	      got_dot = 1;
	    else if (got_e && (p[-1] == 'e' || p[-1] == 'E')
		     && (*p == '-' || *p == '+'))
	      {
		/* This is the sign of the exponent, not the end of
		   the number.  */
	      }
	    /* C++14 allows a separator.  */
	    else if (*p == '\'')
	      {
		if (!no_tick.has_value ())
		  no_tick.emplace (tokstart, p);
		continue;
	      }
	    /* We will take any letters or digits.  parse_number will
	       complain if past the radix, or if L or U are not final.  */
	    else if (! c_isalnum (*p))
	      break;
	    if (no_tick.has_value ())
	      no_tick->push_back (*p);
	  }
	if (no_tick.has_value ())
	  toktype = state->parse_number (no_tick->c_str (),
					 no_tick->length (),
					 got_dot|got_e, lvalp);
	else
	  toktype = state->parse_number (tokstart, p - tokstart,
					 got_dot|got_e, lvalp);
	if (toktype == ERROR)
	  {
	    yyerror (state, _("invalid number"));
	    return ERROR;
	  }
	state->lexptr = p;
	return toktype;
      }

    case '+':
      HANDLE_TOKEN2 ("+=", ASSIGN_MODIFY);
      HANDLE_TOKEN2 ("++", INCREMENT);
      state->lexptr++;
      return c;
    case '*':
      HANDLE_TOKEN2 ("*=", ASSIGN_MODIFY);
      state->lexptr++;
      return c;
    case '/':
      HANDLE_TOKEN2 ("/=", ASSIGN_MODIFY);
      state->lexptr++;
      return c;
    case '%':
      HANDLE_TOKEN2 ("%=", ASSIGN_MODIFY);
      state->lexptr++;
      return c;
    case '|':
      HANDLE_TOKEN2 ("|=", ASSIGN_MODIFY);
      HANDLE_TOKEN2 ("||", OROR);
      state->lexptr++;
      return c;
    case '&':
      HANDLE_TOKEN2 ("&=", ASSIGN_MODIFY);
      HANDLE_TOKEN2 ("&&", ANDAND);
      state->lexptr++;
      return c;
    case '^':
      HANDLE_TOKEN2 ("^=", ASSIGN_MODIFY);
      state->lexptr++;
      return c;
    case '!':
      HANDLE_TOKEN2 ("!=", NOTEQUAL);
      state->lexptr++;
      return c;
    case '<':
      HANDLE_TOKEN3 ("<<=", ASSIGN_MODIFY);
      HANDLE_TOKEN3 ("<=>", SPACESHIP);
      HANDLE_TOKEN2 ("<=", LEQ);
      HANDLE_TOKEN2 ("<<", LSH);
      state->lexptr++;
      return c;
    case '>':
      HANDLE_TOKEN3 (">>=", ASSIGN_MODIFY);
      HANDLE_TOKEN2 (">=", GEQ);
      HANDLE_TOKEN2 (">>", RSH);
      state->lexptr++;
      return c;
    case '=':
      HANDLE_TOKEN2 ("==", EQUAL);
      state->lexptr++;
      return c;
    case ':':
      HANDLE_TOKEN2 ("::", COLONCOLON);
      state->lexptr++;
      return c;

    case '[':
    case ']':
    case '?':
    case '@':
    case '~':
    case '{':
    case '}':
    symbol:
      state->lexptr++;
      return c;

    case '"':
      /* These can't occur in C++ names.  */
      yyerror (state, _("unexpected string literal"));
      return ERROR;
    }

  if (!(c == '_' || c == '$' || c_ident_is_alpha (c)))
    {
      /* We must have come across a bad character (e.g. ';').  */
      yyerror (state, _("invalid character"));
      return ERROR;
    }

  /* It's a name.  See how long it is.  */
  namelen = 0;
  do
    c = tokstart[++namelen];
  while (c_ident_is_alnum (c) || c == '_' || c == '$');

  state->lexptr += namelen;

  /* Catch specific keywords.  Notice that some of the keywords contain
     spaces, and are sorted by the length of the first word.  They must
     all include a trailing space in the string comparison.  */
  switch (namelen)
    {
    case 16:
      if (startswith (tokstart, "reinterpret_cast"))
	return REINTERPRET_CAST;
      break;
    case 12:
      if (startswith (tokstart, "construction vtable for "))
	{
	  state->lexptr = tokstart + 24;
	  return CONSTRUCTION_VTABLE;
	}
      if (startswith (tokstart, "dynamic_cast"))
	return DYNAMIC_CAST;
      break;
    case 11:
      if (startswith (tokstart, "static_cast"))
	return STATIC_CAST;
      break;
    case 9:
      HANDLE_SPECIAL ("covariant return thunk to ", DEMANGLE_COMPONENT_COVARIANT_THUNK);
      HANDLE_SPECIAL ("reference temporary for ", DEMANGLE_COMPONENT_REFTEMP);
      break;
    case 8:
      HANDLE_SPECIAL ("typeinfo for ", DEMANGLE_COMPONENT_TYPEINFO);
      HANDLE_SPECIAL ("typeinfo fn for ", DEMANGLE_COMPONENT_TYPEINFO_FN);
      HANDLE_SPECIAL ("typeinfo name for ", DEMANGLE_COMPONENT_TYPEINFO_NAME);
      if (startswith (tokstart, "operator"))
	return OPERATOR;
      if (startswith (tokstart, "restrict"))
	return RESTRICT;
      if (startswith (tokstart, "unsigned"))
	return UNSIGNED;
      if (startswith (tokstart, "template"))
	return TEMPLATE;
      if (startswith (tokstart, "volatile"))
	return VOLATILE_KEYWORD;
      break;
    case 7:
      HANDLE_SPECIAL ("virtual thunk to ", DEMANGLE_COMPONENT_VIRTUAL_THUNK);
      if (startswith (tokstart, "wchar_t"))
	return WCHAR_T;
      break;
    case 6:
      if (startswith (tokstart, "global constructors keyed to "))
	{
	  const char *p;
	  state->lexptr = tokstart + 29;
	  lvalp->lval = DEMANGLE_COMPONENT_GLOBAL_CONSTRUCTORS;
	  /* Find the end of the symbol.  */
	  p = symbol_end (state->lexptr);
	  lvalp->comp = state->make_name (state->lexptr, p - state->lexptr);
	  state->lexptr = p;
	  return DEMANGLER_SPECIAL;
	}
      if (startswith (tokstart, "global destructors keyed to "))
	{
	  const char *p;
	  state->lexptr = tokstart + 28;
	  lvalp->lval = DEMANGLE_COMPONENT_GLOBAL_DESTRUCTORS;
	  /* Find the end of the symbol.  */
	  p = symbol_end (state->lexptr);
	  lvalp->comp = state->make_name (state->lexptr, p - state->lexptr);
	  state->lexptr = p;
	  return DEMANGLER_SPECIAL;
	}

      HANDLE_SPECIAL ("vtable for ", DEMANGLE_COMPONENT_VTABLE);
      if (startswith (tokstart, "delete"))
	return DELETE;
      if (startswith (tokstart, "struct"))
	return STRUCT;
      if (startswith (tokstart, "signed"))
	return SIGNED_KEYWORD;
      if (startswith (tokstart, "sizeof"))
	return SIZEOF;
      if (startswith (tokstart, "double"))
	return DOUBLE_KEYWORD;
      break;
    case 5:
      HANDLE_SPECIAL ("guard variable for ", DEMANGLE_COMPONENT_GUARD);
      if (startswith (tokstart, "false"))
	return FALSEKEYWORD;
      if (startswith (tokstart, "class"))
	return CLASS;
      if (startswith (tokstart, "union"))
	return UNION;
      if (startswith (tokstart, "float"))
	return FLOAT_KEYWORD;
      if (startswith (tokstart, "short"))
	return SHORT;
      if (startswith (tokstart, "const"))
	return CONST_KEYWORD;
      break;
    case 4:
      if (startswith (tokstart, "void"))
	return VOID;
      if (startswith (tokstart, "bool"))
	return BOOL;
      if (startswith (tokstart, "char"))
	return CHAR;
      if (startswith (tokstart, "enum"))
	return ENUM;
      if (startswith (tokstart, "long"))
	return LONG;
      if (startswith (tokstart, "true"))
	return TRUEKEYWORD;
      break;
    case 3:
      HANDLE_SPECIAL ("VTT for ", DEMANGLE_COMPONENT_VTT);
      HANDLE_SPECIAL ("non-virtual thunk to ", DEMANGLE_COMPONENT_THUNK);
      if (startswith (tokstart, "new"))
	return NEW;
      if (startswith (tokstart, "int"))
	return INT_KEYWORD;
      break;
    default:
      break;
    }

  lvalp->comp = state->make_name (tokstart, namelen);
  return NAME;
}

static void
yyerror (cpname_state *state, const char *msg)
{
  if (state->global_errmsg)
    return;

  state->error_lexptr = state->prev_lexptr;
  state->global_errmsg = msg ? msg : "parse error";
}

/* See cp-support.h.  */

gdb::unique_xmalloc_ptr<char>
cp_comp_to_string (struct demangle_component *result, int estimated_len)
{
  size_t err;

  char *res = gdb_cplus_demangle_print (DMGL_PARAMS | DMGL_ANSI,
					result, estimated_len, &err);
  return gdb::unique_xmalloc_ptr<char> (res);
}

/* Merge the two parse trees given by DEST and SRC.  The parse tree
   in SRC is attached to DEST at the node represented by TARGET.

   NOTE 1: Since there is no API to merge obstacks, this function does
   even attempt to try it.  Fortunately, we do not (yet?) need this ability.
   The code will assert if SRC->obstack is not empty.

   NOTE 2: The string from which SRC was parsed must not be freed, since
   this function will place pointers to that string into DEST.  */

void
cp_merge_demangle_parse_infos (struct demangle_parse_info *dest,
			       struct demangle_component *target,
			       std::unique_ptr<demangle_parse_info> src)

{
  /* Copy the SRC's parse data into DEST.  */
  *target = *src->tree;

  /* Make sure SRC is owned by DEST.  */
  dest->infos.push_back (std::move (src));
}

/* Convert a demangled name to a demangle_component tree.  On success,
   a structure containing the root of the new tree is returned.  On
   error, NULL is returned, and an error message will be set in
   *ERRMSG.  */

struct std::unique_ptr<demangle_parse_info>
cp_demangled_name_to_comp (const char *demangled_name,
			   std::string *errmsg)
{
  auto result = std::make_unique<demangle_parse_info> ();
  cpname_state state (demangled_name, result.get ());

  /* Note that we can't set yydebug here, as is done in the other
     parsers.  Bison implements yydebug as a global, even with a pure
     parser, and this parser is run from worker threads.  So, changing
     yydebug causes TSan reports.  If you need to debug this parser,
     debug gdb and set the global from the outer gdb.  */
  if (yyparse (&state))
    {
      if (state.global_errmsg && errmsg)
	*errmsg = state.global_errmsg;
      return NULL;
    }

  result->tree = state.global_result;

  return result;
}

#if GDB_SELF_TEST

static void
should_be_the_same (const char *one, const char *two)
{
  gdb::unique_xmalloc_ptr<char> cpone = cp_canonicalize_string (one);
  gdb::unique_xmalloc_ptr<char> cptwo = cp_canonicalize_string (two);

  if (cpone != nullptr)
    one = cpone.get ();
  if (cptwo != nullptr)
    two = cptwo.get ();

  SELF_CHECK (streq (one, two));
}

static void
should_parse (const char *name)
{
  std::string err;
  auto parsed = cp_demangled_name_to_comp (name, &err);
  SELF_CHECK (parsed != nullptr);
}

static void
canonicalize_tests ()
{
  should_be_the_same ("short int", "short");
  should_be_the_same ("int short", "short");

  should_be_the_same ("C<(char) 1>::m()", "C<(char) '\\001'>::m()");
  should_be_the_same ("x::y::z<1>", "x::y::z<0x01>");
  should_be_the_same ("x::y::z<1>", "x::y::z<01>");
  should_be_the_same ("x::y::z<(unsigned long long) 1>", "x::y::z<01ull>");
  should_be_the_same ("x::y::z<0b111>", "x::y::z<7>");
  should_be_the_same ("x::y::z<0b111>", "x::y::z<0t7>");
  should_be_the_same ("x::y::z<0b111>", "x::y::z<0D7>");

  should_be_the_same ("x::y::z<0xff'ff>", "x::y::z<65535>");

  should_be_the_same ("something<void ()>", "something<  void()  >");
  should_be_the_same ("something<void ()>", "something<void (void)>");

  should_parse ("void whatever::operator<=><int, int>");

  should_be_the_same ("Foozle<int>::fogey<Empty<int> > (Empty<int>)",
		      "Foozle<int>::fogey<Empty<int>> (Empty<int>)");

  should_be_the_same ("something :: operator new [ ]",
		      "something::operator new[]");
  should_be_the_same ("something :: operator   new",
		      "something::operator new");
  should_be_the_same ("operator()", "operator ()");
}

#endif

INIT_GDB_FILE (cp_name_parser)
{
#if GDB_SELF_TEST
  selftests::register_test ("canonicalize", canonicalize_tests);
#endif
}
