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
#line 36 "c-exp.y"


#include "expression.h"
#include "value.h"
#include "parser-defs.h"
#include "language.h"
#include "c-lang.h"
#include "c-support.h"
#include "charset.h"
#include "block.h"
#include "cp-support.h"
#include "macroscope.h"
#include "objc-lang.h"
#include "typeprint.h"
#include "cp-abi.h"
#include "type-stack.h"
#include "target-float.h"
#include "c-exp.h"
#include "macroexp.h"
#include "cli/cli-style.h"

#define parse_type(ps) builtin_type (ps->gdbarch ())

/* Remap normal yacc parser interface names (yyparse, yylex, yyerror,
   etc).  */
#define GDB_YY_REMAP_PREFIX c_
#include "yy-remap.h"

/* The state of the parser, used internally when we are parsing the
   expression.  */

static struct parser_state *pstate = NULL;

/* Data that must be held for the duration of a parse.  */

struct c_parse_state
{
  /* These are used to hold type lists and type stacks that are
     allocated during the parse.  */
  std::vector<std::unique_ptr<std::vector<struct type *>>> type_lists;
  std::vector<std::unique_ptr<struct type_stack>> type_stacks;

  /* Storage for some strings allocated during the parse.  */
  std::vector<gdb::unique_xmalloc_ptr<char>> strings;

  /* When we find that lexptr (the global var defined in parse.c) is
     pointing at a macro invocation, we expand the invocation, and call
     scan_macro_expansion to save the old lexptr here and point lexptr
     into the expanded text.  When we reach the end of that, we call
     end_macro_expansion to pop back to the value we saved here.  The
     macro expansion code promises to return only fully-expanded text,
     so we don't need to "push" more than one level.

     This is disgusting, of course.  It would be cleaner to do all macro
     expansion beforehand, and then hand that to lexptr.  But we don't
     really know where the expression ends.  Remember, in a command like

     (gdb) break *ADDRESS if CONDITION

     we evaluate ADDRESS in the scope of the current frame, but we
     evaluate CONDITION in the scope of the breakpoint's location.  So
     it's simply wrong to try to macro-expand the whole thing at once.  */
  const char *macro_original_text = nullptr;

  /* We save all intermediate macro expansions on this obstack for the
     duration of a single parse.  The expansion text may sometimes have
     to live past the end of the expansion, due to yacc lookahead.
     Rather than try to be clever about saving the data for a single
     token, we simply keep it all and delete it after parsing has
     completed.  */
  auto_obstack expansion_obstack;

  /* The type stack.  */
  struct type_stack type_stack;

  /* When set, a name token is not looked up.  This can be useful when
     the search domain is known by context.  TYPE_CODE_UNDEF is used
     to mean "unset" here -- typically only types with tags (enum,
     struct, class, union) use this feature, but TYPE_CODE_VOID is
     also used to avoid the lookup for field names.  */
  type_code assume_classification = TYPE_CODE_UNDEF;
};

/* Used for field names, which skip name lookup.  */
struct qualified_name_token
{
  /* The prefix, if any.  This can be nullptr.  */
  const char *prefix;
  /* The field name itself.  */
  const char *name;
  /* True if the COMPLETE token was seen.  */
  bool complete;
};

/* A convenient overload of copy_name.  */
static std::string
copy_name (qualified_name_token token)
{
  if (token.prefix == nullptr)
    return token.name;
  return std::string (token.prefix) + "::" + token.name;
}

/* This is set and cleared in c_parse.  */

static struct c_parse_state *cpstate;

int yyparse (void);

static int yylex (void);

static void yyerror (const char *);

static int type_aggregate_p (struct type *);

static void handle_qualified_field_name (qualified_name_token token);

using namespace expr;

#line 190 "c-exp.c.tmp"

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
    COMPLEX_INT = 259,
    FLOAT = 260,
    COMPLEX_FLOAT = 261,
    STRING = 262,
    NSSTRING = 263,
    SELECTOR = 264,
    CHAR = 265,
    NAME = 266,
    UNKNOWN_CPP_NAME = 267,
    COMPLETE = 268,
    TYPENAME = 269,
    CLASSNAME = 270,
    OBJC_LBRAC = 271,
    NAME_OR_INT = 272,
    OPERATOR = 273,
    STRUCT = 274,
    CLASS = 275,
    UNION = 276,
    ENUM = 277,
    SIZEOF = 278,
    ALIGNOF = 279,
    UNSIGNED = 280,
    COLONCOLON = 281,
    TEMPLATE = 282,
    ERROR = 283,
    NEW = 284,
    DELETE = 285,
    REINTERPRET_CAST = 286,
    DYNAMIC_CAST = 287,
    STATIC_CAST = 288,
    CONST_CAST = 289,
    ENTRY = 290,
    TYPEOF = 291,
    DECLTYPE = 292,
    TYPEID = 293,
    SIGNED_KEYWORD = 294,
    LONG = 295,
    SHORT = 296,
    INT_KEYWORD = 297,
    CONST_KEYWORD = 298,
    VOLATILE_KEYWORD = 299,
    DOUBLE_KEYWORD = 300,
    RESTRICT = 301,
    ATOMIC = 302,
    FLOAT_KEYWORD = 303,
    COMPLEX = 304,
    DOLLAR_VARIABLE = 305,
    ASSIGN_MODIFY = 306,
    TRUEKEYWORD = 307,
    FALSEKEYWORD = 308,
    ABOVE_COMMA = 309,
    OROR = 310,
    ANDAND = 311,
    EQUAL = 312,
    NOTEQUAL = 313,
    LEQ = 314,
    GEQ = 315,
    LSH = 316,
    RSH = 317,
    UNARY = 318,
    INCREMENT = 319,
    DECREMENT = 320,
    ARROW = 321,
    ARROW_STAR = 322,
    DOT_STAR = 323,
    BLOCKNAME = 324,
    FILENAME = 325,
    DOTDOTDOT = 326
  };
#endif
/* Tokens.  */
#define INT 258
#define COMPLEX_INT 259
#define FLOAT 260
#define COMPLEX_FLOAT 261
#define STRING 262
#define NSSTRING 263
#define SELECTOR 264
#define CHAR 265
#define NAME 266
#define UNKNOWN_CPP_NAME 267
#define COMPLETE 268
#define TYPENAME 269
#define CLASSNAME 270
#define OBJC_LBRAC 271
#define NAME_OR_INT 272
#define OPERATOR 273
#define STRUCT 274
#define CLASS 275
#define UNION 276
#define ENUM 277
#define SIZEOF 278
#define ALIGNOF 279
#define UNSIGNED 280
#define COLONCOLON 281
#define TEMPLATE 282
#define ERROR 283
#define NEW 284
#define DELETE 285
#define REINTERPRET_CAST 286
#define DYNAMIC_CAST 287
#define STATIC_CAST 288
#define CONST_CAST 289
#define ENTRY 290
#define TYPEOF 291
#define DECLTYPE 292
#define TYPEID 293
#define SIGNED_KEYWORD 294
#define LONG 295
#define SHORT 296
#define INT_KEYWORD 297
#define CONST_KEYWORD 298
#define VOLATILE_KEYWORD 299
#define DOUBLE_KEYWORD 300
#define RESTRICT 301
#define ATOMIC 302
#define FLOAT_KEYWORD 303
#define COMPLEX 304
#define DOLLAR_VARIABLE 305
#define ASSIGN_MODIFY 306
#define TRUEKEYWORD 307
#define FALSEKEYWORD 308
#define ABOVE_COMMA 309
#define OROR 310
#define ANDAND 311
#define EQUAL 312
#define NOTEQUAL 313
#define LEQ 314
#define GEQ 315
#define LSH 316
#define RSH 317
#define UNARY 318
#define INCREMENT 319
#define DECREMENT 320
#define ARROW 321
#define ARROW_STAR 322
#define DOT_STAR 323
#define BLOCKNAME 324
#define FILENAME 325
#define DOTDOTDOT 326

/* Value type.  */
#if ! defined c_exp_YYSTYPE && ! defined c_exp_YYSTYPE_IS_DECLARED
union c_exp_YYSTYPE
{
#line 161 "c-exp.y"

    LONGEST lval;
    struct {
      LONGEST val;
      struct type *type;
    } typed_val_int;
    struct {
      gdb_byte val[16];
      struct type *type;
    } typed_val_float;
    struct type *tval;
    struct stoken sval;
    qualified_name_token qval;
    struct typed_stoken tsval;
    struct ttype tsym;
    struct symtoken ssym;
    int voidval;
    const struct block *bval;
    enum exp_opcode opcode;

    struct stoken_vector svec;
    std::vector<struct type *> *tvec;

    struct type_stack *type_stack;

    struct objc_class_str theclass;
  

#line 401 "c-exp.c.tmp"

};
typedef union c_exp_YYSTYPE c_exp_YYSTYPE;
# define c_exp_YYSTYPE_IS_TRIVIAL 1
# define c_exp_YYSTYPE_IS_DECLARED 1
#endif


extern c_exp_YYSTYPE yylval;

int yyparse (void);



/* Second part of user prologue.  */
#line 189 "c-exp.y"

/* c_exp_YYSTYPE gets defined by %union */
static int parse_number (struct parser_state *par_state,
			 const char *, int, int, c_exp_YYSTYPE *);
static struct stoken operator_stoken (const char *);
static qualified_name_token typename_stoken (const char *);
static void check_parameter_typelist (std::vector<struct type *> *);

#if defined(YYBISON) && YYBISON < 30800
static void c_print_token (FILE *file, int type, c_exp_YYSTYPE value);
#define YYPRINT(FILE, TYPE, VALUE) c_print_token (FILE, TYPE, VALUE)
#endif

#line 431 "c-exp.c.tmp"


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
         || (defined c_exp_YYSTYPE_IS_TRIVIAL && c_exp_YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union c_exp_yyalloc
{
  yytype_int16 yyss_alloc;
  c_exp_YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union c_exp_yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (c_exp_YYSTYPE)) \
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
#define YYFINAL  172
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1696

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  96
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  60
/* YYNRULES -- Number of rules.  */
#define YYNRULES  284
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  438

#define YYUNDEFTOK  2
#define YYMAXUTOK   326

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
       2,     2,     2,    90,     2,     2,     2,    76,    62,     2,
      85,    89,    74,    72,    54,    73,    82,    75,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    93,     2,
      65,    56,    66,    57,    71,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    84,     2,    92,    61,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    94,    60,    95,    91,     2,     2,     2,
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
      45,    46,    47,    48,    49,    50,    51,    52,    53,    55,
      58,    59,    63,    64,    67,    68,    69,    70,    77,    78,
      79,    80,    81,    83,    86,    87,    88
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   309,   309,   310,   313,   317,   321,   325,   332,   333,
     338,   342,   346,   350,   354,   364,   368,   372,   376,   380,
     384,   388,   392,   396,   401,   400,   429,   434,   433,   467,
     471,   475,   485,   484,   504,   503,   515,   514,   520,   522,
     525,   526,   529,   531,   533,   540,   537,   553,   562,   561,
     580,   584,   587,   591,   595,   610,   620,   627,   628,   631,
     638,   641,   650,   654,   664,   670,   674,   678,   682,   686,
     690,   694,   698,   702,   712,   722,   732,   742,   752,   762,
     766,   770,   774,   790,   806,   823,   833,   842,   849,   862,
     871,   882,   891,   914,   917,   923,   930,   949,   953,   957,
     961,   968,   985,  1003,  1035,  1045,  1051,  1059,  1067,  1073,
    1088,  1101,  1118,  1129,  1145,  1154,  1155,  1166,  1241,  1242,
    1246,  1248,  1250,  1252,  1254,  1259,  1267,  1268,  1272,  1273,
    1278,  1277,  1281,  1280,  1283,  1285,  1287,  1289,  1293,  1300,
    1302,  1303,  1306,  1308,  1315,  1322,  1329,  1337,  1339,  1341,
    1343,  1347,  1352,  1364,  1371,  1374,  1377,  1380,  1383,  1386,
    1389,  1392,  1395,  1398,  1401,  1404,  1407,  1410,  1413,  1416,
    1419,  1422,  1425,  1428,  1431,  1434,  1437,  1440,  1443,  1446,
    1449,  1454,  1459,  1464,  1467,  1470,  1473,  1489,  1491,  1493,
    1498,  1497,  1506,  1505,  1514,  1513,  1522,  1521,  1532,  1537,
    1539,  1543,  1544,  1551,  1558,  1568,  1570,  1579,  1588,  1595,
    1596,  1603,  1607,  1608,  1611,  1612,  1615,  1619,  1621,  1625,
    1627,  1629,  1631,  1633,  1635,  1637,  1639,  1641,  1643,  1645,
    1647,  1649,  1651,  1653,  1655,  1657,  1659,  1661,  1663,  1703,
    1705,  1707,  1709,  1711,  1713,  1715,  1717,  1719,  1721,  1723,
    1725,  1727,  1729,  1731,  1733,  1735,  1753,  1764,  1774,  1775,
    1791,  1811,  1812,  1813,  1814,  1815,  1816,  1817,  1818,  1822,
    1823,  1828,  1839,  1860,  1867,  1876,  1877,  1878,  1879,  1880,
    1881,  1884,  1885,  1893,  1906
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "INT", "COMPLEX_INT", "FLOAT",
  "COMPLEX_FLOAT", "STRING", "NSSTRING", "SELECTOR", "CHAR", "NAME",
  "UNKNOWN_CPP_NAME", "COMPLETE", "TYPENAME", "CLASSNAME", "OBJC_LBRAC",
  "NAME_OR_INT", "OPERATOR", "STRUCT", "CLASS", "UNION", "ENUM", "SIZEOF",
  "ALIGNOF", "UNSIGNED", "COLONCOLON", "TEMPLATE", "ERROR", "NEW",
  "DELETE", "REINTERPRET_CAST", "DYNAMIC_CAST", "STATIC_CAST",
  "CONST_CAST", "ENTRY", "TYPEOF", "DECLTYPE", "TYPEID", "SIGNED_KEYWORD",
  "LONG", "SHORT", "INT_KEYWORD", "CONST_KEYWORD", "VOLATILE_KEYWORD",
  "DOUBLE_KEYWORD", "RESTRICT", "ATOMIC", "FLOAT_KEYWORD", "COMPLEX",
  "DOLLAR_VARIABLE", "ASSIGN_MODIFY", "TRUEKEYWORD", "FALSEKEYWORD", "','",
  "ABOVE_COMMA", "'='", "'?'", "OROR", "ANDAND", "'|'", "'^'", "'&'",
  "EQUAL", "NOTEQUAL", "'<'", "'>'", "LEQ", "GEQ", "LSH", "RSH", "'@'",
  "'+'", "'-'", "'*'", "'/'", "'%'", "UNARY", "INCREMENT", "DECREMENT",
  "ARROW", "ARROW_STAR", "'.'", "DOT_STAR", "'['", "'('", "BLOCKNAME",
  "FILENAME", "DOTDOTDOT", "')'", "'!'", "'~'", "']'", "':'", "'{'", "'}'",
  "$accept", "start", "type_exp", "exp1", "exp", "$@1", "$@2", "$@3",
  "$@4", "$@5", "msglist", "msgarglist", "msgarg", "$@6", "$@7", "lcurly",
  "arglist", "function_method", "function_method_void",
  "function_method_void_or_typelist", "rcurly", "string_exp", "block",
  "variable", "qualified_name", "const_or_volatile", "single_qualifier",
  "qualifier_seq_noopt", "qualifier_seq", "ptr_operator", "$@8", "$@9",
  "ptr_operator_ts", "abs_decl", "direct_abs_decl", "array_mod",
  "func_mod", "type", "scalar_type", "typebase", "$@10", "$@11", "$@12",
  "$@13", "type_name", "parameter_typelist", "nonempty_typelist", "ptype",
  "conversion_type_id", "conversion_declarator", "const_and_volatile",
  "const_or_volatile_noopt", "oper", "qual_field_name",
  "field_or_destructor", "field_name", "field_name_or_complete",
  "tag_name_or_complete", "name", "name_not_typename", YY_NULLPTRPTR
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
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,    44,   309,    61,    63,   310,   311,
     124,    94,    38,   312,   313,    60,    62,   314,   315,   316,
     317,    64,    43,    45,    42,    47,    37,   318,   319,   320,
     321,   322,    46,   323,    91,    40,   324,   325,   326,    41,
      33,   126,    93,    58,   123,   125
};
# endif

#define YYPACT_NINF -220

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-220)))

#define YYTABLE_NINF -120

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     458,  -220,  -220,  -220,  -220,  -220,  -220,  -220,  -220,  -220,
     -63,    -1,   642,  -220,   905,  -220,  -220,  -220,  -220,   734,
     -46,   176,     1,    19,   -15,   -12,    -4,     7,    -6,    53,
      58,   255,   639,    10,  -220,  -220,  -220,  -220,  -220,  -220,
    -220,   343,  -220,  -220,  -220,   826,   102,   826,   826,   826,
     826,   826,   458,   138,  -220,   826,   826,  -220,   182,  -220,
     119,  1300,   458,   184,  -220,   187,   208,   194,  -220,  -220,
    -220,  1625,  -220,  -220,   113,   312,  -220,   216,  -220,    64,
      -1,  -220,  1300,  -220,   168,    22,    32,  -220,  -220,  -220,
    -220,  -220,  -220,  -220,  -220,  -220,  -220,  -220,  -220,  -220,
    -220,  -220,  -220,  -220,  -220,  -220,  -220,  -220,  -220,  -220,
    -220,  -220,   170,   163,  -220,  -220,   730,  -220,    21,    21,
      21,    21,    -1,   458,   116,  1589,  -220,    89,   221,  -220,
    -220,  -220,  -220,  -220,  -220,  -220,  -220,  -220,  -220,  -220,
     203,  1589,  1589,  1589,  1589,   550,   826,   458,    93,  -220,
    -220,   229,   237,   273,  -220,  -220,   238,   239,  -220,  -220,
     116,  -220,  -220,   116,   116,   116,   116,   116,   193,   -38,
     116,   116,  -220,   826,   826,   826,   826,   826,   826,   826,
     826,   826,   826,   826,   826,   826,   826,   826,   826,   826,
     826,   826,   826,   826,   826,   826,   826,  -220,  -220,  -220,
     826,  -220,   826,   826,   318,   188,  1300,   -40,    19,  -220,
      19,  -220,   113,   113,     8,   140,   225,  -220,    12,   245,
     226,    57,  -220,    69,  -220,  -220,  -220,   826,    19,   275,
       6,     6,     6,  -220,   210,   211,   213,   214,  -220,  -220,
      77,  -220,   295,  -220,  -220,  -220,  -220,  -220,   222,   228,
     278,  -220,  -220,  1625,   256,   257,   258,   259,  1048,   232,
    1084,   242,  1120,   291,  -220,  -220,  -220,   293,   294,  -220,
    -220,  -220,   826,  -220,  1300,   -28,  1300,  1300,   359,  1361,
    1395,  1422,  1456,  1483,  1517,  1517,    96,    96,    96,    96,
     535,   535,   627,    43,    43,   116,   116,   116,    29,   173,
      29,   173,   -26,   118,   826,  -220,   252,   288,  -220,   826,
     826,  -220,  -220,   320,  -220,   262,  -220,   226,   226,   113,
     277,  -220,  -220,   260,   281,  -220,    69,   978,  -220,  -220,
     -27,  -220,    19,   826,   826,   280,     6,  -220,   251,   284,
     285,  -220,  -220,  -220,  -220,  -220,  -220,  -220,  -220,  -220,
    -220,   282,   266,   296,   302,   305,  -220,  -220,  -220,  -220,
    -220,  -220,  -220,  -220,   116,  -220,   826,  -220,  -220,  -220,
    -220,  -220,  -220,  -220,  -220,    19,   321,  -220,   360,  -220,
    -220,  -220,  -220,   334,   349,  -220,  -220,  -220,    -9,   160,
    1014,   116,  1300,  -220,   113,  -220,  -220,  -220,  -220,   113,
    -220,  -220,  1300,  1300,  -220,  -220,   251,   826,  -220,  -220,
    -220,   826,   826,   826,   826,  1334,  -220,    72,  -220,  -220,
    -220,  -220,  -220,  -220,  -220,  -220,  1300,  1156,  1192,  1228,
    1264,    19,  -220,  -220,  -220,  -220,  -220,  -220
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,    87,    88,    91,    92,   101,   104,    95,    89,   281,
     284,   187,     0,    90,     0,   190,   192,   196,   194,     0,
       0,   184,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   186,   155,   156,   154,   120,   121,   180,   123,   122,
     181,     0,    94,   105,   106,     0,     0,     0,     0,     0,
       0,     0,     0,   282,   108,     0,     0,    50,     0,     3,
       2,     8,    51,    56,    58,     0,   103,     0,    93,   115,
     126,     0,     4,   188,   209,   153,   283,   117,    48,     0,
      32,    34,    36,   187,     0,   219,   220,   238,   249,   235,
     246,   245,   232,   230,   231,   241,   242,   236,   237,   243,
     244,   239,   240,   225,   226,   227,   228,   229,   247,   248,
     251,   250,     0,     0,   234,   233,   212,   255,     0,     0,
       0,     0,     0,     0,    22,     0,   201,   203,   204,   202,
     183,   284,   282,   116,   275,   279,   277,   278,   276,   280,
       0,     0,     0,     0,     0,     0,     0,     0,   203,   204,
     185,   163,   159,   164,   157,   182,   178,   176,   174,   189,
      11,   124,   125,    13,    12,    10,    16,    17,     0,     0,
      14,    15,     1,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    18,    19,    24,
       0,    27,     0,     0,    45,     0,    52,     0,     0,   102,
       0,   126,   199,   200,     0,   136,   134,   132,     0,     0,
     138,   140,   210,   141,   144,   146,   110,    51,     0,   112,
       0,     0,     0,   254,     0,     0,     0,     0,   253,   252,
     212,   211,   272,   273,   191,   193,   197,   195,     0,     0,
     170,   161,   177,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   168,   160,   162,   158,   172,   167,   165,
     179,   175,     0,    64,     9,     0,    86,    85,     0,    83,
      82,    81,    80,    79,    73,    74,    77,    78,    75,    76,
      71,    72,    65,    69,    70,    66,    67,    68,     0,    26,
       0,    29,     0,    47,    51,   207,     0,   205,    60,     0,
       0,    61,    59,   111,   127,     0,   148,   137,   135,   129,
       0,   147,   151,     0,     0,   130,   139,     0,   143,   145,
       0,   113,     0,     0,     0,     0,    39,    40,    38,     0,
       0,   223,   221,   224,   222,   130,   213,   274,    96,    23,
     171,     0,     0,     0,     0,     0,     5,     6,     7,    21,
      20,   169,   173,   166,    63,    31,     0,   271,   268,   267,
     265,   266,   264,   262,   263,     0,   258,   261,   269,    25,
     257,    28,    30,   217,   218,    55,   216,   118,     0,   119,
       0,    62,    53,   150,   128,   133,   149,   142,   152,   129,
      49,   114,    44,    43,    33,    41,     0,     0,    35,    37,
     198,     0,     0,     0,     0,    84,   260,     0,   270,   214,
     215,    46,    54,   206,   208,   131,    42,     0,     0,     0,
       0,     0,   256,    97,    99,    98,   100,   259
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -220,  -220,     5,     9,    46,  -220,  -220,  -220,  -220,  -220,
      18,  -220,    62,  -220,  -220,  -220,  -206,  -220,  -220,  -220,
     186,  -220,  -220,  -220,  -220,    11,   -68,   -72,     0,  -106,
    -220,  -220,  -220,   183,   180,  -219,  -215,  -116,   362,    -8,
    -220,  -220,  -220,  -220,   373,   201,  -220,  -220,  -220,   166,
    -220,  -220,   -23,  -220,  -220,  -220,   111,   124,   -22,   391
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    58,   168,   169,    61,   298,   300,   230,   231,   232,
     335,   336,   337,   304,   227,    62,   207,    63,    64,    65,
     309,    66,    67,    68,    69,   385,    70,    71,   395,   220,
     399,   319,   221,   222,   223,   224,   225,    72,    73,    74,
     118,   119,   121,   120,   130,   324,   307,    75,   117,   241,
     386,   387,    76,   376,   377,   378,   379,   244,   338,    77
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     139,   140,   213,   211,   328,    59,   116,   248,   329,    60,
     240,   315,     9,   131,   310,   320,   173,   134,   135,    14,
     136,   330,    78,   137,    14,    79,   173,   310,   173,   259,
     134,   135,   242,   136,   243,   156,   137,    14,   234,   125,
     134,   135,   367,   136,   213,   310,   137,    14,   236,   157,
     141,   273,   158,   142,   368,   308,   139,   229,    82,   174,
     333,   143,   400,   212,   365,   124,   382,   205,   369,   370,
     371,   372,   144,   214,   373,   134,   135,   374,   136,   145,
     421,   137,    14,   134,   135,   214,   136,   132,   305,   137,
      14,   160,   138,   163,   164,   165,   166,   167,   388,   334,
     316,   170,   171,   305,   321,   138,   235,   328,   206,   317,
     318,   329,   174,   161,   162,   138,   237,   194,   195,   196,
     375,   197,   198,   199,   200,   201,   202,   203,   204,   250,
     249,   251,   174,   263,   240,   264,   215,   351,   146,   216,
     213,   218,   219,   147,  -119,   314,   254,   255,   256,   257,
     138,   345,   261,   218,   327,   228,    35,    36,   138,    38,
      39,   383,   384,   431,  -107,   189,   190,   191,   192,   193,
     194,   195,   196,   173,   197,   198,   199,   200,   201,   202,
     203,   204,   172,   275,    46,   139,   312,   139,   313,   174,
     126,   258,   260,   262,   197,   198,   199,   200,   201,   202,
     203,   204,   216,   383,   384,   139,   331,   139,   139,   139,
     -57,   305,   302,   208,   217,   209,   127,   128,   129,   274,
     210,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   245,   246,   247,   299,   394,   301,   339,
     340,   226,   239,   199,   200,   201,   202,   203,   204,    83,
     233,   214,   238,   252,    15,    16,    17,    18,   253,   126,
      21,   265,    23,   206,   424,   139,   380,   139,   380,   266,
     270,   271,   272,   308,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,   148,   149,   129,   267,   217,
     325,   332,   341,   342,   215,   343,   344,   216,   347,   139,
     401,   348,   268,   139,   406,   269,    46,   349,   364,   217,
     350,   357,   352,   353,   354,   355,   314,   394,   214,   218,
     219,   359,    83,   361,   322,   362,   363,    15,    16,    17,
      18,   389,   390,    21,   407,    23,  -109,   417,   410,   397,
     206,   411,   139,   416,   393,   391,   392,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    21,   396,
     398,   215,   404,   418,   216,   174,   408,   409,   419,   402,
     403,   412,    31,    32,    33,    34,   217,   413,    37,    46,
     414,    40,   420,   311,   139,   432,   218,   219,   405,   425,
     422,   326,   323,   159,   150,   306,   346,   303,   139,   437,
     175,   381,   415,   133,     0,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,     0,   197,   198,   199,
     200,   201,   202,   203,   204,     0,     0,     0,     0,     0,
       0,     0,   366,   426,     0,     0,     0,   427,   428,   429,
     430,     1,     2,     3,     4,     5,     6,     7,     8,     9,
      10,     0,    11,     0,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,     0,     0,     0,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,     0,
      43,    44,     0,     0,     0,     0,     0,     0,     0,     0,
      45,     0,     0,     0,     0,     0,     0,     0,     0,    46,
      47,    48,    49,     0,     0,     0,    50,    51,     0,     0,
       0,     0,     0,    52,    53,    54,     0,     0,    55,    56,
       0,   174,    57,     1,     2,     3,     4,     5,     6,     7,
       8,     9,    10,     0,    11,     0,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,     0,     0,
       0,    24,    25,    26,    27,     0,     0,     0,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,     0,    43,    44,     0,     0,   191,   192,   193,   194,
     195,   196,    45,   197,   198,   199,   200,   201,   202,   203,
     204,    46,    47,    48,    49,     0,     0,     0,    50,    51,
       0,     0,     0,     0,     0,    52,    53,    54,     0,     0,
      55,    56,     0,   174,    57,     1,     2,     3,     4,     5,
       6,     7,     8,     9,    10,     0,    80,    81,    12,    13,
      14,     0,     0,     0,   151,    19,    20,     0,    22,     0,
       0,     0,     0,    24,    25,    26,    27,     0,   152,   153,
      30,   154,     0,     0,   155,     0,     0,     0,     0,     0,
       0,     0,    42,     0,    43,    44,     0,     0,     0,   192,
     193,   194,   195,   196,    45,   197,   198,   199,   200,   201,
     202,   203,   204,     0,    47,    48,    49,     0,     0,     0,
      50,    51,     0,     0,     0,     0,     0,    52,    53,    54,
       0,     0,    55,    56,     0,     0,    57,     1,     2,     3,
       4,     5,     6,     7,     8,     9,    10,     0,   122,     0,
      12,    13,    14,     0,     0,     0,     0,    19,    20,     0,
      22,     0,     0,     0,     0,    24,    25,    26,    27,     0,
       0,     0,    30,    35,    36,     0,    38,    39,     0,     0,
       0,     0,     0,     0,    42,     0,    43,    44,     0,   215,
       0,     0,   216,     0,     0,     0,    45,     0,     0,     0,
       0,    46,     0,     0,   217,     0,    47,    48,    49,     0,
       0,     0,    50,    51,     0,     0,     0,     0,     0,   123,
      53,    54,     0,     0,    55,    56,     0,     0,    57,     1,
       2,     3,     4,     5,     6,     7,     8,     9,    10,     0,
     122,     0,    12,    13,    14,     0,     0,     0,     0,    19,
      20,     0,    22,     0,     0,     0,     0,    24,    25,    26,
      27,     0,     0,     0,    30,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    42,     0,    43,    44,
       0,     0,     0,     0,     0,     0,     0,     0,    45,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    47,    48,
      49,     0,     0,     0,    50,    51,     0,     0,     0,     0,
       0,    52,    53,    54,     0,     0,    55,    56,     0,    83,
      57,    84,     0,     0,    15,    16,    17,    18,     0,     0,
      21,     0,    23,     0,    85,    86,     0,     0,     0,     0,
       0,     0,     0,     0,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,     0,    87,     0,     0,    88,
       0,    89,     0,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,    46,   103,   104,   105,
     106,   107,     0,   108,   109,   110,   111,     0,     0,   112,
     113,     0,    83,     0,     0,   114,   115,    15,    16,    17,
      18,     0,     0,    21,     0,    23,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    83,     0,
       0,     0,     0,    15,    16,    17,    18,     0,     0,    21,
       0,    23,     0,     0,     0,     0,     0,     0,     0,    46,
       0,     0,     0,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,   174,     0,     0,   322,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    46,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   175,
     174,     0,   423,     0,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,   200,
     201,   202,   203,   204,     0,   175,   174,   356,     0,     0,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,     0,   197,   198,   199,   200,   201,   202,   203,   204,
       0,   175,   174,   358,     0,     0,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,     0,   197,   198,
     199,   200,   201,   202,   203,   204,     0,   175,   174,   360,
       0,     0,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,     0,   197,   198,   199,   200,   201,   202,
     203,   204,     0,   175,   174,   433,     0,     0,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,     0,
     197,   198,   199,   200,   201,   202,   203,   204,     0,   175,
     174,   434,     0,     0,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,   200,
     201,   202,   203,   204,     0,   175,   174,   435,     0,     0,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,     0,   197,   198,   199,   200,   201,   202,   203,   204,
     174,   175,     0,   436,     0,     0,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   174,   197,   198,
     199,   200,   201,   202,   203,   204,     0,     0,     0,     0,
       0,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   174,   197,   198,   199,   200,   201,   202,   203,   204,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   174,   197,
     198,   199,   200,   201,   202,   203,   204,     0,     0,     0,
       0,     0,     0,     0,     0,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   174,   197,   198,   199,   200,   201,   202,   203,
     204,     0,     0,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   174,
     197,   198,   199,   200,   201,   202,   203,   204,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   174,   197,   198,   199,   200,   201,   202,
     203,   204,     0,     0,     0,     0,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
       0,   197,   198,   199,   200,   201,   202,   203,   204,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,     0,   197,   198,   199,   200,   201,
     202,   203,   204,    83,     0,     0,     0,     0,    15,    16,
      17,    18,     0,     0,    21,     0,    23,     0,     0,     0,
       0,     0,     0,     0,     0,    28,    29,     0,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    83,
       0,     0,     0,     0,    15,    16,    17,    18,     0,     0,
      21,     0,    23,     0,     0,     0,     0,     0,     0,     0,
      46,     0,     0,     0,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    46
};

static const yytype_int16 yycheck[] =
{
      23,    23,    74,    71,   223,     0,    14,   123,   223,     0,
     116,     3,    11,    12,    54,     3,    54,    11,    12,    18,
      14,   227,    85,    17,    18,    26,    54,    54,    54,   145,
      11,    12,    11,    14,    13,    25,    17,    18,    16,    85,
      11,    12,    13,    14,   116,    54,    17,    18,    16,    39,
      65,    89,    42,    65,    25,    95,    79,    79,    12,    16,
      54,    65,    89,    71,    92,    19,    92,    62,    39,    40,
      41,    42,    65,    16,    45,    11,    12,    48,    14,    85,
      89,    17,    18,    11,    12,    16,    14,    86,   204,    17,
      18,    45,    86,    47,    48,    49,    50,    51,   304,    93,
      92,    55,    56,   219,    92,    86,    84,   326,    62,   215,
     216,   326,    16,    11,    12,    86,    84,    74,    75,    76,
      91,    78,    79,    80,    81,    82,    83,    84,    85,    40,
     125,    42,    16,    40,   240,    42,    59,   253,    85,    62,
     212,    84,    85,    85,    26,   213,   141,   142,   143,   144,
      86,    74,   147,    84,    85,    91,    43,    44,    86,    46,
      47,    43,    44,    91,    26,    69,    70,    71,    72,    73,
      74,    75,    76,    54,    78,    79,    80,    81,    82,    83,
      84,    85,     0,   174,    71,   208,   208,   210,   210,    16,
      14,   145,   146,   147,    78,    79,    80,    81,    82,    83,
      84,    85,    62,    43,    44,   228,   228,   230,   231,   232,
      26,   327,   203,    26,    74,     7,    40,    41,    42,   173,
      26,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   119,   120,   121,   200,   319,   202,   231,
     232,    35,    89,    80,    81,    82,    83,    84,    85,    14,
      92,    16,    92,    42,    19,    20,    21,    22,    65,    14,
      25,    42,    27,   227,   390,   298,   298,   300,   300,    42,
      42,    42,    89,    95,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    40,    41,    42,    25,    74,
      74,    26,    92,    92,    59,    92,    92,    62,    13,   332,
     332,    89,    39,   336,   336,    42,    71,    89,   272,    74,
      42,    89,    66,    66,    66,    66,   394,   399,    16,    84,
      85,    89,    14,    42,    89,    42,    42,    19,    20,    21,
      22,    89,    54,    25,    93,    27,    26,    26,    66,    89,
     304,    85,   375,   375,    92,   309,   310,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    25,    92,
      89,    59,    92,    13,    62,    16,    92,    92,    44,   333,
     334,    85,    39,    40,    41,    42,    74,    85,    45,    71,
      85,    48,    43,   207,   417,   417,    84,    85,   336,   399,
     389,   221,   219,    41,    31,   204,   240,    89,   431,   431,
      51,   300,   366,    22,    -1,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    -1,    78,    79,    80,
      81,    82,    83,    84,    85,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    93,   407,    -1,    -1,    -1,   411,   412,   413,
     414,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    -1,    14,    -1,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    -1,    -1,    -1,    31,
      32,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    -1,
      52,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      62,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    -1,    -1,
      -1,    -1,    -1,    85,    86,    87,    -1,    -1,    90,    91,
      -1,    16,    94,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    -1,    14,    -1,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    -1,    -1,
      -1,    31,    32,    33,    34,    -1,    -1,    -1,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    -1,    52,    53,    -1,    -1,    71,    72,    73,    74,
      75,    76,    62,    78,    79,    80,    81,    82,    83,    84,
      85,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      -1,    -1,    -1,    -1,    -1,    85,    86,    87,    -1,    -1,
      90,    91,    -1,    16,    94,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    -1,    14,    15,    16,    17,
      18,    -1,    -1,    -1,    25,    23,    24,    -1,    26,    -1,
      -1,    -1,    -1,    31,    32,    33,    34,    -1,    39,    40,
      38,    42,    -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    -1,    52,    53,    -1,    -1,    -1,    72,
      73,    74,    75,    76,    62,    78,    79,    80,    81,    82,
      83,    84,    85,    -1,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    -1,    -1,    -1,    -1,    -1,    85,    86,    87,
      -1,    -1,    90,    91,    -1,    -1,    94,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    -1,    14,    -1,
      16,    17,    18,    -1,    -1,    -1,    -1,    23,    24,    -1,
      26,    -1,    -1,    -1,    -1,    31,    32,    33,    34,    -1,
      -1,    -1,    38,    43,    44,    -1,    46,    47,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    -1,    52,    53,    -1,    59,
      -1,    -1,    62,    -1,    -1,    -1,    62,    -1,    -1,    -1,
      -1,    71,    -1,    -1,    74,    -1,    72,    73,    74,    -1,
      -1,    -1,    78,    79,    -1,    -1,    -1,    -1,    -1,    85,
      86,    87,    -1,    -1,    90,    91,    -1,    -1,    94,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    -1,
      14,    -1,    16,    17,    18,    -1,    -1,    -1,    -1,    23,
      24,    -1,    26,    -1,    -1,    -1,    -1,    31,    32,    33,
      34,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,    52,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    72,    73,
      74,    -1,    -1,    -1,    78,    79,    -1,    -1,    -1,    -1,
      -1,    85,    86,    87,    -1,    -1,    90,    91,    -1,    14,
      94,    16,    -1,    -1,    19,    20,    21,    22,    -1,    -1,
      25,    -1,    27,    -1,    29,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    -1,    51,    -1,    -1,    54,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    -1,    78,    79,    80,    81,    -1,    -1,    84,
      85,    -1,    14,    -1,    -1,    90,    91,    19,    20,    21,
      22,    -1,    -1,    25,    -1,    27,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    14,    -1,
      -1,    -1,    -1,    19,    20,    21,    22,    -1,    -1,    25,
      -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,
      -1,    -1,    -1,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    16,    -1,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,
      16,    -1,    88,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    -1,    78,    79,    80,    81,
      82,    83,    84,    85,    -1,    51,    16,    89,    -1,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    -1,    78,    79,    80,    81,    82,    83,    84,    85,
      -1,    51,    16,    89,    -1,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    -1,    78,    79,
      80,    81,    82,    83,    84,    85,    -1,    51,    16,    89,
      -1,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    -1,    78,    79,    80,    81,    82,    83,
      84,    85,    -1,    51,    16,    89,    -1,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    -1,
      78,    79,    80,    81,    82,    83,    84,    85,    -1,    51,
      16,    89,    -1,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    -1,    78,    79,    80,    81,
      82,    83,    84,    85,    -1,    51,    16,    89,    -1,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    -1,    78,    79,    80,    81,    82,    83,    84,    85,
      16,    51,    -1,    89,    -1,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    16,    78,    79,
      80,    81,    82,    83,    84,    85,    -1,    -1,    -1,    -1,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    16,    78,    79,    80,    81,    82,    83,    84,    85,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    16,    78,
      79,    80,    81,    82,    83,    84,    85,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    16,    78,    79,    80,    81,    82,    83,    84,
      85,    -1,    -1,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    16,
      78,    79,    80,    81,    82,    83,    84,    85,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    16,    78,    79,    80,    81,    82,    83,
      84,    85,    -1,    -1,    -1,    -1,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      -1,    78,    79,    80,    81,    82,    83,    84,    85,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    -1,    78,    79,    80,    81,    82,
      83,    84,    85,    14,    -1,    -1,    -1,    -1,    19,    20,
      21,    22,    -1,    -1,    25,    -1,    27,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    36,    37,    -1,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    14,
      -1,    -1,    -1,    -1,    19,    20,    21,    22,    -1,    -1,
      25,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    -1,    -1,    -1,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    71
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    14,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    31,    32,    33,    34,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    52,    53,    62,    71,    72,    73,    74,
      78,    79,    85,    86,    87,    90,    91,    94,    97,    98,
      99,   100,   111,   113,   114,   115,   117,   118,   119,   120,
     122,   123,   133,   134,   135,   143,   148,   155,    85,    26,
      14,    15,   100,    14,    16,    29,    30,    51,    54,    56,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    72,    73,    74,    75,    76,    78,    79,
      80,    81,    84,    85,    90,    91,   135,   144,   136,   137,
     139,   138,    14,    85,   100,    85,    14,    40,    41,    42,
     140,    12,    86,   155,    11,    12,    14,    17,    86,   148,
     154,    65,    65,    65,    65,    85,    85,    85,    40,    41,
     140,    25,    39,    40,    42,    45,    25,    39,    42,   134,
     100,    11,    12,   100,   100,   100,   100,   100,    98,    99,
     100,   100,     0,    54,    16,    51,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    78,    79,    80,
      81,    82,    83,    84,    85,    98,   100,   112,    26,     7,
      26,   122,   135,   123,    16,    59,    62,    74,    84,    85,
     125,   128,   129,   130,   131,   132,    35,   110,    91,   154,
     103,   104,   105,    92,    16,    84,    16,    84,    92,    89,
     125,   145,    11,    13,   153,   153,   153,   153,   133,    98,
      40,    42,    42,    65,    98,    98,    98,    98,   100,   133,
     100,    98,   100,    40,    42,    42,    42,    25,    39,    42,
      42,    42,    89,    89,   100,    99,   100,   100,   100,   100,
     100,   100,   100,   100,   100,   100,   100,   100,   100,   100,
     100,   100,   100,   100,   100,   100,   100,   100,   101,   100,
     102,   100,    99,    89,   109,   133,   141,   142,    95,   116,
      54,   116,   154,   154,   122,     3,    92,   125,   125,   127,
       3,    92,    89,   129,   141,    74,   130,    85,   131,   132,
     112,   154,    26,    54,    93,   106,   107,   108,   154,   106,
     106,    92,    92,    92,    92,    74,   145,    13,    89,    89,
      42,   133,    66,    66,    66,    66,    89,    89,    89,    89,
      89,    42,    42,    42,   100,    92,    93,    13,    25,    39,
      40,    41,    42,    45,    48,    91,   149,   150,   151,   152,
     154,   152,    92,    43,    44,   121,   146,   147,   112,    89,
      54,   100,   100,    92,   123,   124,    92,    89,    89,   126,
      89,   154,   100,   100,    92,   108,   154,    93,    92,    92,
      66,    85,    85,    85,    85,   100,   154,    26,    13,    44,
      43,    89,   121,    88,   133,   124,   100,   100,   100,   100,
     100,    91,   154,    89,    89,    89,    89,   154
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    96,    97,    97,    98,    98,    98,    98,    99,    99,
     100,   100,   100,   100,   100,   100,   100,   100,   100,   100,
     100,   100,   100,   100,   101,   100,   100,   102,   100,   100,
     100,   100,   103,   100,   104,   100,   105,   100,   106,   106,
     107,   107,   108,   108,   108,   109,   100,   100,   110,   100,
     111,   112,   112,   112,   113,   114,   100,   115,   115,   100,
     116,   100,   100,   100,   100,   100,   100,   100,   100,   100,
     100,   100,   100,   100,   100,   100,   100,   100,   100,   100,
     100,   100,   100,   100,   100,   100,   100,   100,   100,   100,
     100,   100,   100,   100,   100,   100,   100,   100,   100,   100,
     100,   117,   117,   100,   100,   100,   100,   118,   118,   118,
     119,   119,   120,   120,   120,   119,   119,   119,   121,   121,
     122,   122,   122,   122,   122,   122,   123,   123,   124,   124,
     126,   125,   127,   125,   125,   125,   125,   125,   128,   129,
     129,   129,   130,   130,   130,   130,   130,   131,   131,   131,
     131,   132,   132,   133,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   135,   135,   135,
     136,   135,   137,   135,   138,   135,   139,   135,   135,   135,
     135,   140,   140,   140,   140,   141,   141,   142,   142,   143,
     143,   144,   145,   145,   146,   146,   147,   147,   147,   148,
     148,   148,   148,   148,   148,   148,   148,   148,   148,   148,
     148,   148,   148,   148,   148,   148,   148,   148,   148,   148,
     148,   148,   148,   148,   148,   148,   148,   148,   148,   148,
     148,   148,   148,   148,   148,   148,   149,   149,   150,   150,
     150,   151,   151,   151,   151,   151,   151,   151,   151,   152,
     152,   152,   153,   153,   153,   154,   154,   154,   154,   154,
     154,   155,   155,   155,   155
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     4,     4,     4,     1,     3,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       4,     4,     2,     4,     0,     4,     3,     0,     4,     3,
       4,     4,     0,     5,     0,     5,     0,     5,     1,     1,
       1,     2,     3,     2,     2,     0,     5,     3,     0,     5,
       1,     0,     1,     3,     5,     4,     1,     1,     1,     3,
       1,     3,     4,     4,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     5,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     4,     7,     7,     7,
       7,     1,     2,     1,     1,     1,     1,     1,     1,     3,
       2,     3,     3,     4,     5,     1,     2,     1,     1,     0,
       1,     1,     1,     1,     2,     2,     1,     2,     1,     0,
       0,     4,     0,     3,     1,     2,     1,     2,     1,     2,
       1,     1,     3,     2,     1,     2,     1,     2,     2,     3,
       3,     2,     3,     1,     1,     1,     1,     2,     3,     2,
       3,     3,     3,     2,     2,     3,     4,     3,     3,     4,
       3,     4,     3,     4,     2,     3,     2,     3,     2,     3,
       1,     1,     2,     2,     1,     2,     1,     1,     1,     2,
       0,     3,     0,     3,     0,     3,     0,     3,     5,     2,
       2,     1,     1,     1,     1,     1,     3,     1,     3,     1,
       2,     2,     0,     2,     2,     2,     1,     1,     1,     2,
       2,     4,     4,     4,     4,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     3,     3,     3,     2,     3,     1,     1,     4,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     1,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1
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
yy_symbol_value_print (FILE *yyo, int yytype, c_exp_YYSTYPE const * const yyvaluep)
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
yy_symbol_print (FILE *yyo, int yytype, c_exp_YYSTYPE const * const yyvaluep)
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
yy_reduce_print (yytype_int16 *yyssp, c_exp_YYSTYPE *yyvsp, int yyrule)
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
yydestruct (const char *yymsg, int yytype, c_exp_YYSTYPE *yyvaluep)
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
c_exp_YYSTYPE yylval;
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
    c_exp_YYSTYPE yyvsa[YYINITDEPTH];
    c_exp_YYSTYPE *yyvs;
    c_exp_YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  c_exp_YYSTYPE yyval;

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
        c_exp_YYSTYPE *yyvs1 = yyvs;
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
        union c_exp_yyalloc *yyptr =
          (union c_exp_yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
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
#line 314 "c-exp.y"
    {
			  pstate->push_new<type_operation> ((yyvsp[0].tval));
			}
#line 2103 "c-exp.c.tmp"
    break;

  case 5:
#line 318 "c-exp.y"
    {
			  pstate->wrap<typeof_operation> ();
			}
#line 2111 "c-exp.c.tmp"
    break;

  case 6:
#line 322 "c-exp.y"
    {
			  pstate->push_new<type_operation> ((yyvsp[-1].tval));
			}
#line 2119 "c-exp.c.tmp"
    break;

  case 7:
#line 326 "c-exp.y"
    {
			  pstate->wrap<decltype_operation> ();
			}
#line 2127 "c-exp.c.tmp"
    break;

  case 9:
#line 334 "c-exp.y"
    { pstate->wrap2<comma_operation> (); }
#line 2133 "c-exp.c.tmp"
    break;

  case 10:
#line 339 "c-exp.y"
    { pstate->wrap<unop_ind_operation> (); }
#line 2139 "c-exp.c.tmp"
    break;

  case 11:
#line 343 "c-exp.y"
    { pstate->wrap<unop_addr_operation> (); }
#line 2145 "c-exp.c.tmp"
    break;

  case 12:
#line 347 "c-exp.y"
    { pstate->wrap<unary_neg_operation> (); }
#line 2151 "c-exp.c.tmp"
    break;

  case 13:
#line 351 "c-exp.y"
    { pstate->wrap<unary_plus_operation> (); }
#line 2157 "c-exp.c.tmp"
    break;

  case 14:
#line 355 "c-exp.y"
    {
			  if (pstate->language ()->la_language
			      == language_opencl)
			    pstate->wrap<opencl_not_operation> ();
			  else
			    pstate->wrap<unary_logical_not_operation> ();
			}
#line 2169 "c-exp.c.tmp"
    break;

  case 15:
#line 365 "c-exp.y"
    { pstate->wrap<unary_complement_operation> (); }
#line 2175 "c-exp.c.tmp"
    break;

  case 16:
#line 369 "c-exp.y"
    { pstate->wrap<preinc_operation> (); }
#line 2181 "c-exp.c.tmp"
    break;

  case 17:
#line 373 "c-exp.y"
    { pstate->wrap<predec_operation> (); }
#line 2187 "c-exp.c.tmp"
    break;

  case 18:
#line 377 "c-exp.y"
    { pstate->wrap<postinc_operation> (); }
#line 2193 "c-exp.c.tmp"
    break;

  case 19:
#line 381 "c-exp.y"
    { pstate->wrap<postdec_operation> (); }
#line 2199 "c-exp.c.tmp"
    break;

  case 20:
#line 385 "c-exp.y"
    { pstate->wrap<typeid_operation> (); }
#line 2205 "c-exp.c.tmp"
    break;

  case 21:
#line 389 "c-exp.y"
    { pstate->wrap<typeid_operation> (); }
#line 2211 "c-exp.c.tmp"
    break;

  case 22:
#line 393 "c-exp.y"
    { pstate->wrap<unop_sizeof_operation> (); }
#line 2217 "c-exp.c.tmp"
    break;

  case 23:
#line 397 "c-exp.y"
    { pstate->wrap<unop_alignof_operation> (); }
#line 2223 "c-exp.c.tmp"
    break;

  case 24:
#line 401 "c-exp.y"
    {
			  cpstate->assume_classification = TYPE_CODE_VOID;
			}
#line 2231 "c-exp.c.tmp"
    break;

  case 25:
#line 405 "c-exp.y"
    {
			  cpstate->assume_classification = TYPE_CODE_UNDEF;

			  if ((yyvsp[0].qval).prefix != nullptr)
			    {
			      handle_qualified_field_name ((yyvsp[0].qval));
			      /* exp->type::name becomes exp->*(&type::name) */
			      /* Note: this doesn't work if name is a
				 static member!  FIXME */
			      pstate->wrap<unop_addr_operation> ();
			      pstate->wrap2<structop_mptr_operation> ();
			    }
			  else
			    {
			      structop_base_operation *op
				= new structop_ptr_operation (pstate->pop (),
							      copy_name ((yyvsp[0].qval)));
			      if ((yyvsp[0].qval).complete)
				pstate->mark_struct_expression (op);
			      pstate->push (operation_up (op));
			    }
			}
#line 2258 "c-exp.c.tmp"
    break;

  case 26:
#line 430 "c-exp.y"
    { pstate->wrap2<structop_mptr_operation> (); }
#line 2264 "c-exp.c.tmp"
    break;

  case 27:
#line 434 "c-exp.y"
    {
			  cpstate->assume_classification = TYPE_CODE_VOID;
			}
#line 2272 "c-exp.c.tmp"
    break;

  case 28:
#line 438 "c-exp.y"
    {
			  cpstate->assume_classification = TYPE_CODE_UNDEF;

			  if ((yyvsp[0].qval).prefix != nullptr)
			    {
			      handle_qualified_field_name ((yyvsp[0].qval));
			      /* exp.type::name becomes exp.*(&type::name) */
			      /* Note: this doesn't work if name is a
				 static member!  FIXME */
			      pstate->wrap<unop_addr_operation> ();
			      pstate->wrap2<structop_member_operation> ();
			    }
			  else if (pstate->language ()->la_language
				   == language_opencl
				   && !(yyvsp[0].qval).complete)
			    pstate->push_new<opencl_structop_operation>
			      (pstate->pop (), copy_name ((yyvsp[0].qval)));
			  else
			    {
			      structop_base_operation *op
				= new structop_operation (pstate->pop (),
							  copy_name ((yyvsp[0].qval)));
			      if ((yyvsp[0].qval).complete)
				pstate->mark_struct_expression (op);
			      pstate->push (operation_up (op));
			    }
			}
#line 2304 "c-exp.c.tmp"
    break;

  case 29:
#line 468 "c-exp.y"
    { pstate->wrap2<structop_member_operation> (); }
#line 2310 "c-exp.c.tmp"
    break;

  case 30:
#line 472 "c-exp.y"
    { pstate->wrap2<subscript_operation> (); }
#line 2316 "c-exp.c.tmp"
    break;

  case 31:
#line 476 "c-exp.y"
    { pstate->wrap2<subscript_operation> (); }
#line 2322 "c-exp.c.tmp"
    break;

  case 32:
#line 485 "c-exp.y"
    {
			  CORE_ADDR theclass;

			  std::string copy = copy_name ((yyvsp[0].tsym).stoken);
			  theclass = lookup_objc_class (pstate->gdbarch (),
							copy.c_str ());
			  if (theclass == 0)
			    error (_("%s is not an ObjC Class"),
				   copy.c_str ());
			  pstate->push_new<long_const_operation>
			    (parse_type (pstate)->builtin_int,
			     (LONGEST) theclass);
			  start_msglist();
			}
#line 2341 "c-exp.c.tmp"
    break;

  case 33:
#line 500 "c-exp.y"
    { end_msglist (pstate); }
#line 2347 "c-exp.c.tmp"
    break;

  case 34:
#line 504 "c-exp.y"
    {
			  pstate->push_new<long_const_operation>
			    (parse_type (pstate)->builtin_int,
			     (LONGEST) (yyvsp[0].theclass).theclass);
			  start_msglist();
			}
#line 2358 "c-exp.c.tmp"
    break;

  case 35:
#line 511 "c-exp.y"
    { end_msglist (pstate); }
#line 2364 "c-exp.c.tmp"
    break;

  case 36:
#line 515 "c-exp.y"
    { start_msglist(); }
#line 2370 "c-exp.c.tmp"
    break;

  case 37:
#line 517 "c-exp.y"
    { end_msglist (pstate); }
#line 2376 "c-exp.c.tmp"
    break;

  case 38:
#line 521 "c-exp.y"
    { add_msglist(&(yyvsp[0].sval), 0); }
#line 2382 "c-exp.c.tmp"
    break;

  case 42:
#line 530 "c-exp.y"
    { add_msglist(&(yyvsp[-2].sval), 1); }
#line 2388 "c-exp.c.tmp"
    break;

  case 43:
#line 532 "c-exp.y"
    { add_msglist(0, 1);   }
#line 2394 "c-exp.c.tmp"
    break;

  case 44:
#line 534 "c-exp.y"
    { add_msglist(0, 0);   }
#line 2400 "c-exp.c.tmp"
    break;

  case 45:
#line 540 "c-exp.y"
    { pstate->start_arglist (); }
#line 2406 "c-exp.c.tmp"
    break;

  case 46:
#line 542 "c-exp.y"
    {
			  std::vector<operation_up> args
			    = pstate->pop_vector (pstate->end_arglist ());
			  pstate->push_new<funcall_operation>
			    (pstate->pop (), std::move (args));
			}
#line 2417 "c-exp.c.tmp"
    break;

  case 47:
#line 554 "c-exp.y"
    {
			  pstate->push_new<funcall_operation>
			    (pstate->pop (), std::vector<operation_up> ());
			}
#line 2426 "c-exp.c.tmp"
    break;

  case 48:
#line 562 "c-exp.y"
    {
			  /* This could potentially be a an argument defined
			     lookup function (Koenig).  */
			  /* This is to save the value of arglist_len
			     being accumulated by an outer function call.  */
			  pstate->start_arglist ();
			}
#line 2438 "c-exp.c.tmp"
    break;

  case 49:
#line 570 "c-exp.y"
    {
			  std::vector<operation_up> args
			    = pstate->pop_vector (pstate->end_arglist ());
			  pstate->push_new<adl_func_operation>
			    (copy_name ((yyvsp[-4].ssym).stoken),
			     pstate->expression_context_block,
			     std::move (args));
			}
#line 2451 "c-exp.c.tmp"
    break;

  case 50:
#line 581 "c-exp.y"
    { pstate->start_arglist (); }
#line 2457 "c-exp.c.tmp"
    break;

  case 52:
#line 588 "c-exp.y"
    { pstate->arglist_len = 1; }
#line 2463 "c-exp.c.tmp"
    break;

  case 53:
#line 592 "c-exp.y"
    { pstate->arglist_len++; }
#line 2469 "c-exp.c.tmp"
    break;

  case 54:
#line 596 "c-exp.y"
    {
			  std::vector<struct type *> *type_list = (yyvsp[-2].tvec);
			  /* Save the const/volatile qualifiers as
			     recorded by the const_or_volatile
			     production's actions.  */
			  type_instance_flags flags
			    = (cpstate->type_stack
			       .follow_type_instance_flags ());
			  pstate->push_new<type_instance_operation>
			    (flags, std::move (*type_list),
			     pstate->pop ());
			}
#line 2486 "c-exp.c.tmp"
    break;

  case 55:
#line 611 "c-exp.y"
    {
			  type_instance_flags flags
			    = (cpstate->type_stack
			       .follow_type_instance_flags ());
			  pstate->push_new<type_instance_operation>
			    (flags, std::vector<type *> (), pstate->pop ());
		       }
#line 2498 "c-exp.c.tmp"
    break;

  case 59:
#line 632 "c-exp.y"
    {
			  pstate->push_new<func_static_var_operation>
			    (pstate->pop (), copy_name ((yyvsp[0].sval)));
			}
#line 2507 "c-exp.c.tmp"
    break;

  case 60:
#line 639 "c-exp.y"
    { (yyval.lval) = pstate->end_arglist () - 1; }
#line 2513 "c-exp.c.tmp"
    break;

  case 61:
#line 642 "c-exp.y"
    {
			  std::vector<operation_up> args
			    = pstate->pop_vector ((yyvsp[0].lval) + 1);
			  pstate->push_new<array_operation> (0, (yyvsp[0].lval),
							     std::move (args));
			}
#line 2524 "c-exp.c.tmp"
    break;

  case 62:
#line 651 "c-exp.y"
    { pstate->wrap2<unop_memval_type_operation> (); }
#line 2530 "c-exp.c.tmp"
    break;

  case 63:
#line 655 "c-exp.y"
    {
			  if (pstate->language ()->la_language
			      == language_opencl)
			    pstate->wrap2<opencl_cast_type_operation> ();
			  else
			    pstate->wrap2<unop_cast_type_operation> ();
			}
#line 2542 "c-exp.c.tmp"
    break;

  case 64:
#line 665 "c-exp.y"
    { }
#line 2548 "c-exp.c.tmp"
    break;

  case 65:
#line 671 "c-exp.y"
    { pstate->wrap2<repeat_operation> (); }
#line 2554 "c-exp.c.tmp"
    break;

  case 66:
#line 675 "c-exp.y"
    { pstate->wrap2<mul_operation> (); }
#line 2560 "c-exp.c.tmp"
    break;

  case 67:
#line 679 "c-exp.y"
    { pstate->wrap2<div_operation> (); }
#line 2566 "c-exp.c.tmp"
    break;

  case 68:
#line 683 "c-exp.y"
    { pstate->wrap2<rem_operation> (); }
#line 2572 "c-exp.c.tmp"
    break;

  case 69:
#line 687 "c-exp.y"
    { pstate->wrap2<add_operation> (); }
#line 2578 "c-exp.c.tmp"
    break;

  case 70:
#line 691 "c-exp.y"
    { pstate->wrap2<sub_operation> (); }
#line 2584 "c-exp.c.tmp"
    break;

  case 71:
#line 695 "c-exp.y"
    { pstate->wrap2<lsh_operation> (); }
#line 2590 "c-exp.c.tmp"
    break;

  case 72:
#line 699 "c-exp.y"
    { pstate->wrap2<rsh_operation> (); }
#line 2596 "c-exp.c.tmp"
    break;

  case 73:
#line 703 "c-exp.y"
    {
			  if (pstate->language ()->la_language
			      == language_opencl)
			    pstate->wrap2<opencl_equal_operation> ();
			  else
			    pstate->wrap2<equal_operation> ();
			}
#line 2608 "c-exp.c.tmp"
    break;

  case 74:
#line 713 "c-exp.y"
    {
			  if (pstate->language ()->la_language
			      == language_opencl)
			    pstate->wrap2<opencl_notequal_operation> ();
			  else
			    pstate->wrap2<notequal_operation> ();
			}
#line 2620 "c-exp.c.tmp"
    break;

  case 75:
#line 723 "c-exp.y"
    {
			  if (pstate->language ()->la_language
			      == language_opencl)
			    pstate->wrap2<opencl_leq_operation> ();
			  else
			    pstate->wrap2<leq_operation> ();
			}
#line 2632 "c-exp.c.tmp"
    break;

  case 76:
#line 733 "c-exp.y"
    {
			  if (pstate->language ()->la_language
			      == language_opencl)
			    pstate->wrap2<opencl_geq_operation> ();
			  else
			    pstate->wrap2<geq_operation> ();
			}
#line 2644 "c-exp.c.tmp"
    break;

  case 77:
#line 743 "c-exp.y"
    {
			  if (pstate->language ()->la_language
			      == language_opencl)
			    pstate->wrap2<opencl_less_operation> ();
			  else
			    pstate->wrap2<less_operation> ();
			}
#line 2656 "c-exp.c.tmp"
    break;

  case 78:
#line 753 "c-exp.y"
    {
			  if (pstate->language ()->la_language
			      == language_opencl)
			    pstate->wrap2<opencl_gtr_operation> ();
			  else
			    pstate->wrap2<gtr_operation> ();
			}
#line 2668 "c-exp.c.tmp"
    break;

  case 79:
#line 763 "c-exp.y"
    { pstate->wrap2<bitwise_and_operation> (); }
#line 2674 "c-exp.c.tmp"
    break;

  case 80:
#line 767 "c-exp.y"
    { pstate->wrap2<bitwise_xor_operation> (); }
#line 2680 "c-exp.c.tmp"
    break;

  case 81:
#line 771 "c-exp.y"
    { pstate->wrap2<bitwise_ior_operation> (); }
#line 2686 "c-exp.c.tmp"
    break;

  case 82:
#line 775 "c-exp.y"
    {
			  if (pstate->language ()->la_language
			      == language_opencl)
			    {
			      operation_up rhs = pstate->pop ();
			      operation_up lhs = pstate->pop ();
			      pstate->push_new<opencl_logical_binop_operation>
				(BINOP_LOGICAL_AND, std::move (lhs),
				 std::move (rhs));
			    }
			  else
			    pstate->wrap2<logical_and_operation> ();
			}
#line 2704 "c-exp.c.tmp"
    break;

  case 83:
#line 791 "c-exp.y"
    {
			  if (pstate->language ()->la_language
			      == language_opencl)
			    {
			      operation_up rhs = pstate->pop ();
			      operation_up lhs = pstate->pop ();
			      pstate->push_new<opencl_logical_binop_operation>
				(BINOP_LOGICAL_OR, std::move (lhs),
				 std::move (rhs));
			    }
			  else
			    pstate->wrap2<logical_or_operation> ();
			}
#line 2722 "c-exp.c.tmp"
    break;

  case 84:
#line 807 "c-exp.y"
    {
			  operation_up last = pstate->pop ();
			  operation_up mid = pstate->pop ();
			  operation_up first = pstate->pop ();
			  if (pstate->language ()->la_language
			      == language_opencl)
			    pstate->push_new<opencl_ternop_cond_operation>
			      (std::move (first), std::move (mid),
			       std::move (last));
			  else
			    pstate->push_new<ternop_cond_operation>
			      (std::move (first), std::move (mid),
			       std::move (last));
			}
#line 2741 "c-exp.c.tmp"
    break;

  case 85:
#line 824 "c-exp.y"
    {
			  if (pstate->language ()->la_language
			      == language_opencl)
			    pstate->wrap2<opencl_assign_operation> ();
			  else
			    pstate->wrap2<assign_operation> ();
			}
#line 2753 "c-exp.c.tmp"
    break;

  case 86:
#line 834 "c-exp.y"
    {
			  operation_up rhs = pstate->pop ();
			  operation_up lhs = pstate->pop ();
			  pstate->push_new<assign_modify_operation>
			    ((yyvsp[-1].opcode), std::move (lhs), std::move (rhs));
			}
#line 2764 "c-exp.c.tmp"
    break;

  case 87:
#line 843 "c-exp.y"
    {
			  pstate->push_new<long_const_operation>
			    ((yyvsp[0].typed_val_int).type, (yyvsp[0].typed_val_int).val);
			}
#line 2773 "c-exp.c.tmp"
    break;

  case 88:
#line 850 "c-exp.y"
    {
			  operation_up real
			    = (make_operation<long_const_operation>
			       ((yyvsp[0].typed_val_int).type->target_type (), 0));
			  operation_up imag
			    = (make_operation<long_const_operation>
			       ((yyvsp[0].typed_val_int).type->target_type (), (yyvsp[0].typed_val_int).val));
			  pstate->push_new<complex_operation>
			    (std::move (real), std::move (imag), (yyvsp[0].typed_val_int).type);
			}
#line 2788 "c-exp.c.tmp"
    break;

  case 89:
#line 863 "c-exp.y"
    {
			  struct stoken_vector vec;
			  vec.len = 1;
			  vec.tokens = &(yyvsp[0].tsval);
			  pstate->push_c_string ((yyvsp[0].tsval).type, &vec);
			}
#line 2799 "c-exp.c.tmp"
    break;

  case 90:
#line 872 "c-exp.y"
    { c_exp_YYSTYPE val;
			  parse_number (pstate, (yyvsp[0].ssym).stoken.ptr,
					(yyvsp[0].ssym).stoken.length, 0, &val);
			  pstate->push_new<long_const_operation>
			    (val.typed_val_int.type,
			     val.typed_val_int.val);
			}
#line 2811 "c-exp.c.tmp"
    break;

  case 91:
#line 883 "c-exp.y"
    {
			  float_data data;
			  std::copy (std::begin ((yyvsp[0].typed_val_float).val), std::end ((yyvsp[0].typed_val_float).val),
				     std::begin (data));
			  pstate->push_new<float_const_operation> ((yyvsp[0].typed_val_float).type, data);
			}
#line 2822 "c-exp.c.tmp"
    break;

  case 92:
#line 892 "c-exp.y"
    {
			  struct type *underlying = (yyvsp[0].typed_val_float).type->target_type ();

			  float_data val;
			  target_float_from_host_double (val.data (),
							 underlying, 0);
			  operation_up real
			    = (make_operation<float_const_operation>
			       (underlying, val));

			  std::copy (std::begin ((yyvsp[0].typed_val_float).val), std::end ((yyvsp[0].typed_val_float).val),
				     std::begin (val));
			  operation_up imag
			    = (make_operation<float_const_operation>
			       (underlying, val));

			  pstate->push_new<complex_operation>
			    (std::move (real), std::move (imag),
			     (yyvsp[0].typed_val_float).type);
			}
#line 2847 "c-exp.c.tmp"
    break;

  case 94:
#line 918 "c-exp.y"
    {
			  pstate->push_dollar ((yyvsp[0].sval));
			}
#line 2855 "c-exp.c.tmp"
    break;

  case 95:
#line 924 "c-exp.y"
    {
			  pstate->push_new<objc_selector_operation>
			    (copy_name ((yyvsp[0].sval)));
			}
#line 2864 "c-exp.c.tmp"
    break;

  case 96:
#line 931 "c-exp.y"
    { struct type *type = (yyvsp[-1].tval);
			  struct type *int_type
			    = lookup_signed_typename (pstate->language (),
						      "int");
			  type = check_typedef (type);

			    /* $5.3.3/2 of the C++ Standard (n3290 draft)
			       says of sizeof:  "When applied to a reference
			       or a reference type, the result is the size of
			       the referenced type."  */
			  if (TYPE_IS_REFERENCE (type))
			    type = check_typedef (type->target_type ());

			  pstate->push_new<long_const_operation>
			    (int_type, type->length ());
			}
#line 2885 "c-exp.c.tmp"
    break;

  case 97:
#line 950 "c-exp.y"
    { pstate->wrap2<reinterpret_cast_operation> (); }
#line 2891 "c-exp.c.tmp"
    break;

  case 98:
#line 954 "c-exp.y"
    { pstate->wrap2<unop_cast_type_operation> (); }
#line 2897 "c-exp.c.tmp"
    break;

  case 99:
#line 958 "c-exp.y"
    { pstate->wrap2<dynamic_cast_operation> (); }
#line 2903 "c-exp.c.tmp"
    break;

  case 100:
#line 962 "c-exp.y"
    { /* We could do more error checking here, but
			     it doesn't seem worthwhile.  */
			  pstate->wrap2<unop_cast_type_operation> (); }
#line 2911 "c-exp.c.tmp"
    break;

  case 101:
#line 969 "c-exp.y"
    {
			  /* We copy the string here, and not in the
			     lexer, to guarantee that we do not leak a
			     string.  Note that we follow the
			     NUL-termination convention of the
			     lexer.  */
			  struct typed_stoken *vec = XNEW (struct typed_stoken);
			  (yyval.svec).len = 1;
			  (yyval.svec).tokens = vec;

			  vec->type = (yyvsp[0].tsval).type;
			  vec->length = (yyvsp[0].tsval).length;
			  vec->ptr = (char *) xmalloc ((yyvsp[0].tsval).length + 1);
			  memcpy (vec->ptr, (yyvsp[0].tsval).ptr, (yyvsp[0].tsval).length + 1);
			}
#line 2931 "c-exp.c.tmp"
    break;

  case 102:
#line 986 "c-exp.y"
    {
			  /* Note that we NUL-terminate here, but just
			     for convenience.  */
			  char *p;
			  ++(yyval.svec).len;
			  (yyval.svec).tokens = XRESIZEVEC (struct typed_stoken,
						  (yyval.svec).tokens, (yyval.svec).len);

			  p = (char *) xmalloc ((yyvsp[0].tsval).length + 1);
			  memcpy (p, (yyvsp[0].tsval).ptr, (yyvsp[0].tsval).length + 1);

			  (yyval.svec).tokens[(yyval.svec).len - 1].type = (yyvsp[0].tsval).type;
			  (yyval.svec).tokens[(yyval.svec).len - 1].length = (yyvsp[0].tsval).length;
			  (yyval.svec).tokens[(yyval.svec).len - 1].ptr = p;
			}
#line 2951 "c-exp.c.tmp"
    break;

  case 103:
#line 1004 "c-exp.y"
    {
			  int i;
			  c_string_type type = C_STRING;

			  for (i = 0; i < (yyvsp[0].svec).len; ++i)
			    {
			      switch ((yyvsp[0].svec).tokens[i].type)
				{
				case C_STRING:
				  break;
				case C_WIDE_STRING:
				case C_STRING_16:
				case C_STRING_32:
				  if (type != C_STRING
				      && type != (yyvsp[0].svec).tokens[i].type)
				    error (_("Undefined string concatenation."));
				  type = (enum c_string_type_values) (yyvsp[0].svec).tokens[i].type;
				  break;
				default:
				  /* internal error */
				  internal_error ("unrecognized type in string concatenation");
				}
			    }

			  pstate->push_c_string (type, &(yyvsp[0].svec));
			  for (i = 0; i < (yyvsp[0].svec).len; ++i)
			    xfree ((yyvsp[0].svec).tokens[i].ptr);
			  xfree ((yyvsp[0].svec).tokens);
			}
#line 2985 "c-exp.c.tmp"
    break;

  case 104:
#line 1036 "c-exp.y"
    {
			  /* ObjC NextStep NSString constant of the
			     form '@' '"' string '"'.  */
			  pstate->push_new<objc_nsstring_operation>
			    (std::string ((yyvsp[0].tsval).ptr, (yyvsp[0].tsval).length));
			}
#line 2996 "c-exp.c.tmp"
    break;

  case 105:
#line 1046 "c-exp.y"
    { pstate->push_new<long_const_operation>
			    (parse_type (pstate)->builtin_bool, 1);
			}
#line 3004 "c-exp.c.tmp"
    break;

  case 106:
#line 1052 "c-exp.y"
    { pstate->push_new<long_const_operation>
			    (parse_type (pstate)->builtin_bool, 0);
			}
#line 3012 "c-exp.c.tmp"
    break;

  case 107:
#line 1060 "c-exp.y"
    {
			  if ((yyvsp[0].ssym).sym.symbol)
			    (yyval.bval) = (yyvsp[0].ssym).sym.symbol->value_block ();
			  else
			    error (_("No file or function \"%s\"."),
				   copy_name ((yyvsp[0].ssym).stoken).c_str ());
			}
#line 3024 "c-exp.c.tmp"
    break;

  case 108:
#line 1068 "c-exp.y"
    {
			  (yyval.bval) = (yyvsp[0].bval);
			}
#line 3032 "c-exp.c.tmp"
    break;

  case 109:
#line 1074 "c-exp.y"
    {
			  std::string copy = copy_name ((yyvsp[0].sval));
			  struct symbol *tem
			    = lookup_symbol (copy.c_str (), (yyvsp[-2].bval),
					     SEARCH_FUNCTION_DOMAIN,
					     nullptr).symbol;

			  if (tem == nullptr)
			    error (_("No function \"%ps\" in specified context."),
				   styled_string (function_name_style.style (),
						  copy.c_str ()));
			  (yyval.bval) = tem->value_block (); }
#line 3049 "c-exp.c.tmp"
    break;

  case 110:
#line 1089 "c-exp.y"
    { struct symbol *sym = (yyvsp[-1].ssym).sym.symbol;

			  if (sym == NULL || !sym->is_argument ()
			      || !symbol_read_needs_frame (sym))
			    error (_("@entry can be used only for function "
				     "parameters, not for \"%s\""),
				   copy_name ((yyvsp[-1].ssym).stoken).c_str ());

			  pstate->push_new<var_entry_value_operation> (sym);
			}
#line 3064 "c-exp.c.tmp"
    break;

  case 111:
#line 1102 "c-exp.y"
    {
			  std::string copy = copy_name ((yyvsp[0].sval));
			  struct block_symbol sym
			    = lookup_symbol (copy.c_str (), (yyvsp[-2].bval),
					     SEARCH_VFT, NULL);

			  if (sym.symbol == 0)
			    error (_("No symbol \"%s\" in specified context."),
				   copy.c_str ());
			  if (symbol_read_needs_frame (sym.symbol))
			    pstate->block_tracker->update (sym);

			  pstate->push_new<var_value_operation> (sym);
			}
#line 3083 "c-exp.c.tmp"
    break;

  case 112:
#line 1119 "c-exp.y"
    {
			  struct type *type = (yyvsp[-2].tsym).type;
			  type = check_typedef (type);
			  if (!type_aggregate_p (type))
			    error (_("`%s' is not defined as an aggregate type."),
				   type->safe_name ());

			  pstate->push_new<scope_operation> (type,
							     copy_name ((yyvsp[0].sval)));
			}
#line 3098 "c-exp.c.tmp"
    break;

  case 113:
#line 1130 "c-exp.y"
    {
			  struct type *type = (yyvsp[-3].tsym).type;

			  type = check_typedef (type);
			  if (!type_aggregate_p (type))
			    error (_("`%s' is not defined as an aggregate type."),
				   type->safe_name ());
			  std::string name = "~" + std::string ((yyvsp[0].sval).ptr,
								(yyvsp[0].sval).length);

			  /* Check for valid destructor name.  */
			  destructor_name_p (name.c_str (), (yyvsp[-3].tsym).type);
			  pstate->push_new<scope_operation> (type,
							     std::move (name));
			}
#line 3118 "c-exp.c.tmp"
    break;

  case 114:
#line 1146 "c-exp.y"
    {
			  std::string copy = copy_name ((yyvsp[-2].sval));
			  error (_("No type \"%s\" within class "
				   "or namespace \"%s\"."),
				 copy.c_str (), (yyvsp[-4].tsym).type->safe_name ());
			}
#line 3129 "c-exp.c.tmp"
    break;

  case 116:
#line 1156 "c-exp.y"
    {
			  std::string name = copy_name ((yyvsp[0].ssym).stoken);
			  struct block_symbol sym
			    = lookup_symbol (name.c_str (),
					     (const struct block *) NULL,
					     SEARCH_VFT, NULL);
			  pstate->push_symbol (name.c_str (), sym);
			}
#line 3142 "c-exp.c.tmp"
    break;

  case 117:
#line 1167 "c-exp.y"
    { struct block_symbol sym = (yyvsp[0].ssym).sym;

			  if (sym.symbol)
			    {
			      if (symbol_read_needs_frame (sym.symbol))
				pstate->block_tracker->update (sym);

			      /* If we found a function, see if it's
				 an ifunc resolver that has the same
				 address as the ifunc symbol itself.
				 If so, prefer the ifunc symbol.  */

			      bound_minimal_symbol resolver
				= find_gnu_ifunc (sym.symbol);
			      if (resolver.minsym != NULL)
				pstate->push_new<var_msym_value_operation>
				  (resolver);
			      else
				pstate->push_new<var_value_operation> (sym);
			    }
			  else if ((yyvsp[0].ssym).is_a_field_of_this)
			    {
			      /* C++: it hangs off of `this'.  Must
				 not inadvertently convert from a method call
				 to data ref.  */
			      pstate->block_tracker->update (sym);
			      operation_up thisop
				= make_operation<op_this_operation> ();
			      pstate->push_new<structop_ptr_operation>
				(std::move (thisop), copy_name ((yyvsp[0].ssym).stoken));
			    }
			  else
			    {
			      std::string arg = copy_name ((yyvsp[0].ssym).stoken);

			      bound_minimal_symbol msymbol
				= lookup_minimal_symbol (current_program_space, arg.c_str ());
			      if (msymbol.minsym == NULL)
				{
				  if (!current_program_space->has_full_symbols ()
				      && !current_program_space->has_partial_symbols ())
				    error (_("No symbol table is loaded.  Use the \"%ps\" command."),
					   styled_string (command_style.style (),
							  "file"));
				  else
				    error (_("No symbol \"%s\" in current context."),
					   arg.c_str ());
				}

			      /* This minsym might be an alias for
				 another function.  See if we can find
				 the debug symbol for the target, and
				 if so, use it instead, since it has
				 return type / prototype info.  This
				 is important for example for "p
				 *__errno_location()".  */
			      symbol *alias_target
				= ((msymbol.minsym->type () != mst_text_gnu_ifunc
				    && msymbol.minsym->type () != mst_data_gnu_ifunc)
				   ? find_function_alias_target (msymbol)
				   : NULL);
			      if (alias_target != NULL)
				{
				  block_symbol bsym { alias_target,
				    alias_target->value_block () };
				  pstate->push_new<var_value_operation> (bsym);
				}
			      else
				pstate->push_new<var_msym_value_operation>
				  (msymbol);
			    }
			}
#line 3219 "c-exp.c.tmp"
    break;

  case 120:
#line 1247 "c-exp.y"
    { cpstate->type_stack.insert (tp_const); }
#line 3225 "c-exp.c.tmp"
    break;

  case 121:
#line 1249 "c-exp.y"
    { cpstate->type_stack.insert (tp_volatile); }
#line 3231 "c-exp.c.tmp"
    break;

  case 122:
#line 1251 "c-exp.y"
    { cpstate->type_stack.insert (tp_atomic); }
#line 3237 "c-exp.c.tmp"
    break;

  case 123:
#line 1253 "c-exp.y"
    { cpstate->type_stack.insert (tp_restrict); }
#line 3243 "c-exp.c.tmp"
    break;

  case 124:
#line 1255 "c-exp.y"
    {
		  cpstate->type_stack.insert (pstate->gdbarch (),
					      copy_name ((yyvsp[0].ssym).stoken).c_str ());
		}
#line 3252 "c-exp.c.tmp"
    break;

  case 125:
#line 1260 "c-exp.y"
    {
		  cpstate->type_stack.insert (pstate->gdbarch (),
					      copy_name ((yyvsp[0].ssym).stoken).c_str ());
		}
#line 3261 "c-exp.c.tmp"
    break;

  case 130:
#line 1278 "c-exp.y"
    { cpstate->type_stack.insert (tp_pointer); }
#line 3267 "c-exp.c.tmp"
    break;

  case 132:
#line 1281 "c-exp.y"
    { cpstate->type_stack.insert (tp_pointer); }
#line 3273 "c-exp.c.tmp"
    break;

  case 134:
#line 1284 "c-exp.y"
    { cpstate->type_stack.insert (tp_reference); }
#line 3279 "c-exp.c.tmp"
    break;

  case 135:
#line 1286 "c-exp.y"
    { cpstate->type_stack.insert (tp_reference); }
#line 3285 "c-exp.c.tmp"
    break;

  case 136:
#line 1288 "c-exp.y"
    { cpstate->type_stack.insert (tp_rvalue_reference); }
#line 3291 "c-exp.c.tmp"
    break;

  case 137:
#line 1290 "c-exp.y"
    { cpstate->type_stack.insert (tp_rvalue_reference); }
#line 3297 "c-exp.c.tmp"
    break;

  case 138:
#line 1294 "c-exp.y"
    {
			  (yyval.type_stack) = cpstate->type_stack.create ();
			  cpstate->type_stacks.emplace_back ((yyval.type_stack));
			}
#line 3306 "c-exp.c.tmp"
    break;

  case 139:
#line 1301 "c-exp.y"
    { (yyval.type_stack) = (yyvsp[0].type_stack)->append ((yyvsp[-1].type_stack)); }
#line 3312 "c-exp.c.tmp"
    break;

  case 142:
#line 1307 "c-exp.y"
    { (yyval.type_stack) = (yyvsp[-1].type_stack); }
#line 3318 "c-exp.c.tmp"
    break;

  case 143:
#line 1309 "c-exp.y"
    {
			  cpstate->type_stack.push ((yyvsp[-1].type_stack));
			  cpstate->type_stack.push (tp_array, (yyvsp[0].lval));
			  (yyval.type_stack) = cpstate->type_stack.create ();
			  cpstate->type_stacks.emplace_back ((yyval.type_stack));
			}
#line 3329 "c-exp.c.tmp"
    break;

  case 144:
#line 1316 "c-exp.y"
    {
			  cpstate->type_stack.push (tp_array, (yyvsp[0].lval));
			  (yyval.type_stack) = cpstate->type_stack.create ();
			  cpstate->type_stacks.emplace_back ((yyval.type_stack));
			}
#line 3339 "c-exp.c.tmp"
    break;

  case 145:
#line 1323 "c-exp.y"
    {
			  cpstate->type_stack.push ((yyvsp[-1].type_stack));
			  cpstate->type_stack.push ((yyvsp[0].tvec));
			  (yyval.type_stack) = cpstate->type_stack.create ();
			  cpstate->type_stacks.emplace_back ((yyval.type_stack));
			}
#line 3350 "c-exp.c.tmp"
    break;

  case 146:
#line 1330 "c-exp.y"
    {
			  cpstate->type_stack.push ((yyvsp[0].tvec));
			  (yyval.type_stack) = cpstate->type_stack.create ();
			  cpstate->type_stacks.emplace_back ((yyval.type_stack));
			}
#line 3360 "c-exp.c.tmp"
    break;

  case 147:
#line 1338 "c-exp.y"
    { (yyval.lval) = -1; }
#line 3366 "c-exp.c.tmp"
    break;

  case 148:
#line 1340 "c-exp.y"
    { (yyval.lval) = -1; }
#line 3372 "c-exp.c.tmp"
    break;

  case 149:
#line 1342 "c-exp.y"
    { (yyval.lval) = (yyvsp[-1].typed_val_int).val; }
#line 3378 "c-exp.c.tmp"
    break;

  case 150:
#line 1344 "c-exp.y"
    { (yyval.lval) = (yyvsp[-1].typed_val_int).val; }
#line 3384 "c-exp.c.tmp"
    break;

  case 151:
#line 1348 "c-exp.y"
    {
			  (yyval.tvec) = new std::vector<struct type *>;
			  cpstate->type_lists.emplace_back ((yyval.tvec));
			}
#line 3393 "c-exp.c.tmp"
    break;

  case 152:
#line 1353 "c-exp.y"
    { (yyval.tvec) = (yyvsp[-1].tvec); }
#line 3399 "c-exp.c.tmp"
    break;

  case 154:
#line 1372 "c-exp.y"
    { (yyval.tval) = lookup_signed_typename (pstate->language (),
						       "int"); }
#line 3406 "c-exp.c.tmp"
    break;

  case 155:
#line 1375 "c-exp.y"
    { (yyval.tval) = lookup_signed_typename (pstate->language (),
						       "long"); }
#line 3413 "c-exp.c.tmp"
    break;

  case 156:
#line 1378 "c-exp.y"
    { (yyval.tval) = lookup_signed_typename (pstate->language (),
						       "short"); }
#line 3420 "c-exp.c.tmp"
    break;

  case 157:
#line 1381 "c-exp.y"
    { (yyval.tval) = lookup_signed_typename (pstate->language (),
						       "long"); }
#line 3427 "c-exp.c.tmp"
    break;

  case 158:
#line 1384 "c-exp.y"
    { (yyval.tval) = lookup_signed_typename (pstate->language (),
						       "long"); }
#line 3434 "c-exp.c.tmp"
    break;

  case 159:
#line 1387 "c-exp.y"
    { (yyval.tval) = lookup_signed_typename (pstate->language (),
						       "long"); }
#line 3441 "c-exp.c.tmp"
    break;

  case 160:
#line 1390 "c-exp.y"
    { (yyval.tval) = lookup_signed_typename (pstate->language (),
						       "long"); }
#line 3448 "c-exp.c.tmp"
    break;

  case 161:
#line 1393 "c-exp.y"
    { (yyval.tval) = lookup_unsigned_typename (pstate->language (),
							 "long"); }
#line 3455 "c-exp.c.tmp"
    break;

  case 162:
#line 1396 "c-exp.y"
    { (yyval.tval) = lookup_unsigned_typename (pstate->language (),
							 "long"); }
#line 3462 "c-exp.c.tmp"
    break;

  case 163:
#line 1399 "c-exp.y"
    { (yyval.tval) = lookup_unsigned_typename (pstate->language (),
							 "long"); }
#line 3469 "c-exp.c.tmp"
    break;

  case 164:
#line 1402 "c-exp.y"
    { (yyval.tval) = lookup_signed_typename (pstate->language (),
						       "long long"); }
#line 3476 "c-exp.c.tmp"
    break;

  case 165:
#line 1405 "c-exp.y"
    { (yyval.tval) = lookup_signed_typename (pstate->language (),
						       "long long"); }
#line 3483 "c-exp.c.tmp"
    break;

  case 166:
#line 1408 "c-exp.y"
    { (yyval.tval) = lookup_signed_typename (pstate->language (),
						       "long long"); }
#line 3490 "c-exp.c.tmp"
    break;

  case 167:
#line 1411 "c-exp.y"
    { (yyval.tval) = lookup_signed_typename (pstate->language (),
						       "long long"); }
#line 3497 "c-exp.c.tmp"
    break;

  case 168:
#line 1414 "c-exp.y"
    { (yyval.tval) = lookup_signed_typename (pstate->language (),
						       "long long"); }
#line 3504 "c-exp.c.tmp"
    break;

  case 169:
#line 1417 "c-exp.y"
    { (yyval.tval) = lookup_signed_typename (pstate->language (),
						       "long long"); }
#line 3511 "c-exp.c.tmp"
    break;

  case 170:
#line 1420 "c-exp.y"
    { (yyval.tval) = lookup_unsigned_typename (pstate->language (),
							 "long long"); }
#line 3518 "c-exp.c.tmp"
    break;

  case 171:
#line 1423 "c-exp.y"
    { (yyval.tval) = lookup_unsigned_typename (pstate->language (),
							 "long long"); }
#line 3525 "c-exp.c.tmp"
    break;

  case 172:
#line 1426 "c-exp.y"
    { (yyval.tval) = lookup_unsigned_typename (pstate->language (),
							 "long long"); }
#line 3532 "c-exp.c.tmp"
    break;

  case 173:
#line 1429 "c-exp.y"
    { (yyval.tval) = lookup_unsigned_typename (pstate->language (),
							 "long long"); }
#line 3539 "c-exp.c.tmp"
    break;

  case 174:
#line 1432 "c-exp.y"
    { (yyval.tval) = lookup_signed_typename (pstate->language (),
						       "short"); }
#line 3546 "c-exp.c.tmp"
    break;

  case 175:
#line 1435 "c-exp.y"
    { (yyval.tval) = lookup_signed_typename (pstate->language (),
						       "short"); }
#line 3553 "c-exp.c.tmp"
    break;

  case 176:
#line 1438 "c-exp.y"
    { (yyval.tval) = lookup_signed_typename (pstate->language (),
						       "short"); }
#line 3560 "c-exp.c.tmp"
    break;

  case 177:
#line 1441 "c-exp.y"
    { (yyval.tval) = lookup_unsigned_typename (pstate->language (),
							 "short"); }
#line 3567 "c-exp.c.tmp"
    break;

  case 178:
#line 1444 "c-exp.y"
    { (yyval.tval) = lookup_unsigned_typename (pstate->language (),
							 "short"); }
#line 3574 "c-exp.c.tmp"
    break;

  case 179:
#line 1447 "c-exp.y"
    { (yyval.tval) = lookup_unsigned_typename (pstate->language (),
							 "short"); }
#line 3581 "c-exp.c.tmp"
    break;

  case 180:
#line 1450 "c-exp.y"
    { (yyval.tval) = lookup_typename (pstate->language (),
						"double",
						NULL,
						0); }
#line 3590 "c-exp.c.tmp"
    break;

  case 181:
#line 1455 "c-exp.y"
    { (yyval.tval) = lookup_typename (pstate->language (),
						"float",
						NULL,
						0); }
#line 3599 "c-exp.c.tmp"
    break;

  case 182:
#line 1460 "c-exp.y"
    { (yyval.tval) = lookup_typename (pstate->language (),
						"long double",
						NULL,
						0); }
#line 3608 "c-exp.c.tmp"
    break;

  case 183:
#line 1465 "c-exp.y"
    { (yyval.tval) = lookup_unsigned_typename (pstate->language (),
							 (yyvsp[0].tsym).type->name ()); }
#line 3615 "c-exp.c.tmp"
    break;

  case 184:
#line 1468 "c-exp.y"
    { (yyval.tval) = lookup_unsigned_typename (pstate->language (),
							 "int"); }
#line 3622 "c-exp.c.tmp"
    break;

  case 185:
#line 1471 "c-exp.y"
    { (yyval.tval) = lookup_signed_typename (pstate->language (),
						       (yyvsp[0].tsym).type->name ()); }
#line 3629 "c-exp.c.tmp"
    break;

  case 186:
#line 1474 "c-exp.y"
    { (yyval.tval) = lookup_signed_typename (pstate->language (),
						       "int"); }
#line 3636 "c-exp.c.tmp"
    break;

  case 187:
#line 1490 "c-exp.y"
    { (yyval.tval) = (yyvsp[0].tsym).type; }
#line 3642 "c-exp.c.tmp"
    break;

  case 188:
#line 1492 "c-exp.y"
    { (yyval.tval) = (yyvsp[0].tval); }
#line 3648 "c-exp.c.tmp"
    break;

  case 189:
#line 1494 "c-exp.y"
    {
			  (yyval.tval) = init_complex_type (nullptr, (yyvsp[0].tval));
			}
#line 3656 "c-exp.c.tmp"
    break;

  case 190:
#line 1498 "c-exp.y"
    {
			  cpstate->assume_classification = TYPE_CODE_STRUCT;
			}
#line 3664 "c-exp.c.tmp"
    break;

  case 191:
#line 1502 "c-exp.y"
    {
			  (yyval.tval) = (yyvsp[0].tval);
			}
#line 3672 "c-exp.c.tmp"
    break;

  case 192:
#line 1506 "c-exp.y"
    {
			  cpstate->assume_classification = TYPE_CODE_STRUCT;
			}
#line 3680 "c-exp.c.tmp"
    break;

  case 193:
#line 1510 "c-exp.y"
    {
			  (yyval.tval) = (yyvsp[0].tval);
			}
#line 3688 "c-exp.c.tmp"
    break;

  case 194:
#line 1514 "c-exp.y"
    {
			  cpstate->assume_classification = TYPE_CODE_ENUM;
			}
#line 3696 "c-exp.c.tmp"
    break;

  case 195:
#line 1518 "c-exp.y"
    {
			  (yyval.tval) = (yyvsp[0].tval);
			}
#line 3704 "c-exp.c.tmp"
    break;

  case 196:
#line 1522 "c-exp.y"
    {
			  cpstate->assume_classification = TYPE_CODE_UNION;
			}
#line 3712 "c-exp.c.tmp"
    break;

  case 197:
#line 1526 "c-exp.y"
    {
			  (yyval.tval) = (yyvsp[0].tval);
			}
#line 3720 "c-exp.c.tmp"
    break;

  case 198:
#line 1533 "c-exp.y"
    { (yyval.tval) = lookup_template_type
			    (copy_name((yyvsp[-3].sval)).c_str (), (yyvsp[-1].tval),
			     pstate->expression_context_block);
			}
#line 3729 "c-exp.c.tmp"
    break;

  case 199:
#line 1538 "c-exp.y"
    { (yyval.tval) = cpstate->type_stack.follow_types ((yyvsp[0].tval)); }
#line 3735 "c-exp.c.tmp"
    break;

  case 200:
#line 1540 "c-exp.y"
    { (yyval.tval) = cpstate->type_stack.follow_types ((yyvsp[-1].tval)); }
#line 3741 "c-exp.c.tmp"
    break;

  case 202:
#line 1545 "c-exp.y"
    {
		  (yyval.tsym).stoken.ptr = "int";
		  (yyval.tsym).stoken.length = 3;
		  (yyval.tsym).type = lookup_signed_typename (pstate->language (),
						    "int");
		}
#line 3752 "c-exp.c.tmp"
    break;

  case 203:
#line 1552 "c-exp.y"
    {
		  (yyval.tsym).stoken.ptr = "long";
		  (yyval.tsym).stoken.length = 4;
		  (yyval.tsym).type = lookup_signed_typename (pstate->language (),
						    "long");
		}
#line 3763 "c-exp.c.tmp"
    break;

  case 204:
#line 1559 "c-exp.y"
    {
		  (yyval.tsym).stoken.ptr = "short";
		  (yyval.tsym).stoken.length = 5;
		  (yyval.tsym).type = lookup_signed_typename (pstate->language (),
						    "short");
		}
#line 3774 "c-exp.c.tmp"
    break;

  case 205:
#line 1569 "c-exp.y"
    { check_parameter_typelist ((yyvsp[0].tvec)); }
#line 3780 "c-exp.c.tmp"
    break;

  case 206:
#line 1571 "c-exp.y"
    {
			  (yyvsp[-2].tvec)->push_back (NULL);
			  check_parameter_typelist ((yyvsp[-2].tvec));
			  (yyval.tvec) = (yyvsp[-2].tvec);
			}
#line 3790 "c-exp.c.tmp"
    break;

  case 207:
#line 1580 "c-exp.y"
    {
		  std::vector<struct type *> *typelist
		    = new std::vector<struct type *>;
		  cpstate->type_lists.emplace_back (typelist);

		  typelist->push_back ((yyvsp[0].tval));
		  (yyval.tvec) = typelist;
		}
#line 3803 "c-exp.c.tmp"
    break;

  case 208:
#line 1589 "c-exp.y"
    {
		  (yyvsp[-2].tvec)->push_back ((yyvsp[0].tval));
		  (yyval.tvec) = (yyvsp[-2].tvec);
		}
#line 3812 "c-exp.c.tmp"
    break;

  case 210:
#line 1597 "c-exp.y"
    {
		  cpstate->type_stack.push ((yyvsp[0].type_stack));
		  (yyval.tval) = cpstate->type_stack.follow_types ((yyvsp[-1].tval));
		}
#line 3821 "c-exp.c.tmp"
    break;

  case 211:
#line 1604 "c-exp.y"
    { (yyval.tval) = cpstate->type_stack.follow_types ((yyvsp[-1].tval)); }
#line 3827 "c-exp.c.tmp"
    break;

  case 216:
#line 1616 "c-exp.y"
    { cpstate->type_stack.insert (tp_const);
			  cpstate->type_stack.insert (tp_volatile);
			}
#line 3835 "c-exp.c.tmp"
    break;

  case 217:
#line 1620 "c-exp.y"
    { cpstate->type_stack.insert (tp_const); }
#line 3841 "c-exp.c.tmp"
    break;

  case 218:
#line 1622 "c-exp.y"
    { cpstate->type_stack.insert (tp_volatile); }
#line 3847 "c-exp.c.tmp"
    break;

  case 219:
#line 1626 "c-exp.y"
    { (yyval.sval) = operator_stoken (" new"); }
#line 3853 "c-exp.c.tmp"
    break;

  case 220:
#line 1628 "c-exp.y"
    { (yyval.sval) = operator_stoken (" delete"); }
#line 3859 "c-exp.c.tmp"
    break;

  case 221:
#line 1630 "c-exp.y"
    { (yyval.sval) = operator_stoken (" new[]"); }
#line 3865 "c-exp.c.tmp"
    break;

  case 222:
#line 1632 "c-exp.y"
    { (yyval.sval) = operator_stoken (" delete[]"); }
#line 3871 "c-exp.c.tmp"
    break;

  case 223:
#line 1634 "c-exp.y"
    { (yyval.sval) = operator_stoken (" new[]"); }
#line 3877 "c-exp.c.tmp"
    break;

  case 224:
#line 1636 "c-exp.y"
    { (yyval.sval) = operator_stoken (" delete[]"); }
#line 3883 "c-exp.c.tmp"
    break;

  case 225:
#line 1638 "c-exp.y"
    { (yyval.sval) = operator_stoken ("+"); }
#line 3889 "c-exp.c.tmp"
    break;

  case 226:
#line 1640 "c-exp.y"
    { (yyval.sval) = operator_stoken ("-"); }
#line 3895 "c-exp.c.tmp"
    break;

  case 227:
#line 1642 "c-exp.y"
    { (yyval.sval) = operator_stoken ("*"); }
#line 3901 "c-exp.c.tmp"
    break;

  case 228:
#line 1644 "c-exp.y"
    { (yyval.sval) = operator_stoken ("/"); }
#line 3907 "c-exp.c.tmp"
    break;

  case 229:
#line 1646 "c-exp.y"
    { (yyval.sval) = operator_stoken ("%"); }
#line 3913 "c-exp.c.tmp"
    break;

  case 230:
#line 1648 "c-exp.y"
    { (yyval.sval) = operator_stoken ("^"); }
#line 3919 "c-exp.c.tmp"
    break;

  case 231:
#line 1650 "c-exp.y"
    { (yyval.sval) = operator_stoken ("&"); }
#line 3925 "c-exp.c.tmp"
    break;

  case 232:
#line 1652 "c-exp.y"
    { (yyval.sval) = operator_stoken ("|"); }
#line 3931 "c-exp.c.tmp"
    break;

  case 233:
#line 1654 "c-exp.y"
    { (yyval.sval) = operator_stoken ("~"); }
#line 3937 "c-exp.c.tmp"
    break;

  case 234:
#line 1656 "c-exp.y"
    { (yyval.sval) = operator_stoken ("!"); }
#line 3943 "c-exp.c.tmp"
    break;

  case 235:
#line 1658 "c-exp.y"
    { (yyval.sval) = operator_stoken ("="); }
#line 3949 "c-exp.c.tmp"
    break;

  case 236:
#line 1660 "c-exp.y"
    { (yyval.sval) = operator_stoken ("<"); }
#line 3955 "c-exp.c.tmp"
    break;

  case 237:
#line 1662 "c-exp.y"
    { (yyval.sval) = operator_stoken (">"); }
#line 3961 "c-exp.c.tmp"
    break;

  case 238:
#line 1664 "c-exp.y"
    { const char *op = " unknown";
			  switch ((yyvsp[0].opcode))
			    {
			    case BINOP_RSH:
			      op = ">>=";
			      break;
			    case BINOP_LSH:
			      op = "<<=";
			      break;
			    case BINOP_ADD:
			      op = "+=";
			      break;
			    case BINOP_SUB:
			      op = "-=";
			      break;
			    case BINOP_MUL:
			      op = "*=";
			      break;
			    case BINOP_DIV:
			      op = "/=";
			      break;
			    case BINOP_REM:
			      op = "%=";
			      break;
			    case BINOP_BITWISE_IOR:
			      op = "|=";
			      break;
			    case BINOP_BITWISE_AND:
			      op = "&=";
			      break;
			    case BINOP_BITWISE_XOR:
			      op = "^=";
			      break;
			    default:
			      break;
			    }

			  (yyval.sval) = operator_stoken (op);
			}
#line 4005 "c-exp.c.tmp"
    break;

  case 239:
#line 1704 "c-exp.y"
    { (yyval.sval) = operator_stoken ("<<"); }
#line 4011 "c-exp.c.tmp"
    break;

  case 240:
#line 1706 "c-exp.y"
    { (yyval.sval) = operator_stoken (">>"); }
#line 4017 "c-exp.c.tmp"
    break;

  case 241:
#line 1708 "c-exp.y"
    { (yyval.sval) = operator_stoken ("=="); }
#line 4023 "c-exp.c.tmp"
    break;

  case 242:
#line 1710 "c-exp.y"
    { (yyval.sval) = operator_stoken ("!="); }
#line 4029 "c-exp.c.tmp"
    break;

  case 243:
#line 1712 "c-exp.y"
    { (yyval.sval) = operator_stoken ("<="); }
#line 4035 "c-exp.c.tmp"
    break;

  case 244:
#line 1714 "c-exp.y"
    { (yyval.sval) = operator_stoken (">="); }
#line 4041 "c-exp.c.tmp"
    break;

  case 245:
#line 1716 "c-exp.y"
    { (yyval.sval) = operator_stoken ("&&"); }
#line 4047 "c-exp.c.tmp"
    break;

  case 246:
#line 1718 "c-exp.y"
    { (yyval.sval) = operator_stoken ("||"); }
#line 4053 "c-exp.c.tmp"
    break;

  case 247:
#line 1720 "c-exp.y"
    { (yyval.sval) = operator_stoken ("++"); }
#line 4059 "c-exp.c.tmp"
    break;

  case 248:
#line 1722 "c-exp.y"
    { (yyval.sval) = operator_stoken ("--"); }
#line 4065 "c-exp.c.tmp"
    break;

  case 249:
#line 1724 "c-exp.y"
    { (yyval.sval) = operator_stoken (","); }
#line 4071 "c-exp.c.tmp"
    break;

  case 250:
#line 1726 "c-exp.y"
    { (yyval.sval) = operator_stoken ("->*"); }
#line 4077 "c-exp.c.tmp"
    break;

  case 251:
#line 1728 "c-exp.y"
    { (yyval.sval) = operator_stoken ("->"); }
#line 4083 "c-exp.c.tmp"
    break;

  case 252:
#line 1730 "c-exp.y"
    { (yyval.sval) = operator_stoken ("()"); }
#line 4089 "c-exp.c.tmp"
    break;

  case 253:
#line 1732 "c-exp.y"
    { (yyval.sval) = operator_stoken ("[]"); }
#line 4095 "c-exp.c.tmp"
    break;

  case 254:
#line 1734 "c-exp.y"
    { (yyval.sval) = operator_stoken ("[]"); }
#line 4101 "c-exp.c.tmp"
    break;

  case 255:
#line 1736 "c-exp.y"
    {
			  string_file buf;
			  c_print_type ((yyvsp[0].tval), NULL, &buf, -1, 0,
					pstate->language ()->la_language,
					&type_print_raw_options);
			  std::string name = buf.release ();

			  /* This also needs canonicalization.  */
			  gdb::unique_xmalloc_ptr<char> canon
			    = cp_canonicalize_string (name.c_str ());
			  if (canon != nullptr)
			    name = canon.get ();
			  (yyval.sval) = operator_stoken ((" " + name).c_str ());
			}
#line 4120 "c-exp.c.tmp"
    break;

  case 256:
#line 1754 "c-exp.y"
    {
		  (yyval.qval).complete = false;
		  if ((yyvsp[-2].qval).prefix == nullptr)
		    (yyval.qval).prefix = (yyvsp[-2].qval).name;
		  else
		    (yyval.qval).prefix = obconcat (&cpstate->expansion_obstack,
					  (yyvsp[-2].qval).prefix, "::", (yyvsp[-2].qval).name, nullptr);
		  (yyval.qval).name = obstack_strndup (&cpstate->expansion_obstack,
					     (yyvsp[0].sval).ptr, (yyvsp[0].sval).length);
		}
#line 4135 "c-exp.c.tmp"
    break;

  case 257:
#line 1765 "c-exp.y"
    {
		  (yyval.qval).complete = false;
		  (yyval.qval).prefix = nullptr;
		  (yyval.qval).name = obstack_strndup (&cpstate->expansion_obstack,
					     (yyvsp[0].sval).ptr, (yyvsp[0].sval).length);
		}
#line 4146 "c-exp.c.tmp"
    break;

  case 259:
#line 1776 "c-exp.y"
    {
		  (yyval.qval).complete = false;
		  if ((yyvsp[-3].qval).prefix == nullptr)
		    (yyval.qval).prefix = (yyvsp[-3].qval).name;
		  else
		    (yyval.qval).prefix = obconcat (&cpstate->expansion_obstack,
					  (yyvsp[-3].qval).prefix, "::", (yyvsp[-3].qval).name, nullptr);
		  char *name
		    = (char *) obstack_alloc (&cpstate->expansion_obstack,
					      (yyvsp[0].sval).length + 2);
		  name[0] = '~';
		  memcpy (&name[1], (yyvsp[0].sval).ptr, (yyvsp[0].sval).length);
		  name[(yyvsp[0].sval).length + 1] = '\0';
		  (yyval.qval).name = name;
		}
#line 4166 "c-exp.c.tmp"
    break;

  case 260:
#line 1792 "c-exp.y"
    {
		  (yyval.qval).complete = false;
		  (yyval.qval).prefix = nullptr;
		  char *name
		    = (char *) obstack_alloc (&cpstate->expansion_obstack,
					      (yyvsp[0].sval).length + 2);
		  name[0] = '~';
		  memcpy (&name[1], (yyvsp[0].sval).ptr, (yyvsp[0].sval).length);
		  name[(yyvsp[0].sval).length + 1] = '\0';
		  (yyval.qval).name = name;
		}
#line 4182 "c-exp.c.tmp"
    break;

  case 262:
#line 1812 "c-exp.y"
    { (yyval.qval) = typename_stoken ("double"); }
#line 4188 "c-exp.c.tmp"
    break;

  case 263:
#line 1813 "c-exp.y"
    { (yyval.qval) = typename_stoken ("float"); }
#line 4194 "c-exp.c.tmp"
    break;

  case 264:
#line 1814 "c-exp.y"
    { (yyval.qval) = typename_stoken ("int"); }
#line 4200 "c-exp.c.tmp"
    break;

  case 265:
#line 1815 "c-exp.y"
    { (yyval.qval) = typename_stoken ("long"); }
#line 4206 "c-exp.c.tmp"
    break;

  case 266:
#line 1816 "c-exp.y"
    { (yyval.qval) = typename_stoken ("short"); }
#line 4212 "c-exp.c.tmp"
    break;

  case 267:
#line 1817 "c-exp.y"
    { (yyval.qval) = typename_stoken ("signed"); }
#line 4218 "c-exp.c.tmp"
    break;

  case 268:
#line 1818 "c-exp.y"
    { (yyval.qval) = typename_stoken ("unsigned"); }
#line 4224 "c-exp.c.tmp"
    break;

  case 270:
#line 1824 "c-exp.y"
    {
			  (yyval.qval) = (yyvsp[-1].qval);
			  (yyval.qval).complete = true;
			}
#line 4233 "c-exp.c.tmp"
    break;

  case 271:
#line 1829 "c-exp.y"
    {
			  (yyval.qval) = typename_stoken ("");
			  (yyval.qval).complete = true;
			}
#line 4242 "c-exp.c.tmp"
    break;

  case 272:
#line 1840 "c-exp.y"
    {
		  switch (cpstate->assume_classification)
		    {
		    case TYPE_CODE_STRUCT:
		      (yyval.tval) = lookup_struct (copy_name ((yyvsp[0].ssym).stoken).c_str (),
					  pstate->expression_context_block);
		      break;
		    case TYPE_CODE_ENUM:
		      (yyval.tval) = lookup_enum (copy_name ((yyvsp[0].ssym).stoken).c_str (),
					pstate->expression_context_block);
		      break;
		    case TYPE_CODE_UNION:
		      (yyval.tval) = lookup_union (copy_name ((yyvsp[0].ssym).stoken).c_str (),
					 pstate->expression_context_block);
		      break;
		    default:
		      gdb_assert_not_reached ();
		    }
		  cpstate->assume_classification = TYPE_CODE_UNDEF;
		}
#line 4267 "c-exp.c.tmp"
    break;

  case 273:
#line 1861 "c-exp.y"
    {
		  pstate->mark_completion_tag (cpstate->assume_classification,
					       "", 0);
		  cpstate->assume_classification = TYPE_CODE_UNDEF;
		  (yyval.tval) = nullptr;
		}
#line 4278 "c-exp.c.tmp"
    break;

  case 274:
#line 1868 "c-exp.y"
    {
		  pstate->mark_completion_tag (cpstate->assume_classification,
					       (yyvsp[-1].ssym).stoken.ptr, (yyvsp[-1].ssym).stoken.length);
		  cpstate->assume_classification = TYPE_CODE_UNDEF;
		  (yyval.tval) = nullptr;
		}
#line 4289 "c-exp.c.tmp"
    break;

  case 275:
#line 1876 "c-exp.y"
    { (yyval.sval) = (yyvsp[0].ssym).stoken; }
#line 4295 "c-exp.c.tmp"
    break;

  case 276:
#line 1877 "c-exp.y"
    { (yyval.sval) = (yyvsp[0].ssym).stoken; }
#line 4301 "c-exp.c.tmp"
    break;

  case 277:
#line 1878 "c-exp.y"
    { (yyval.sval) = (yyvsp[0].tsym).stoken; }
#line 4307 "c-exp.c.tmp"
    break;

  case 278:
#line 1879 "c-exp.y"
    { (yyval.sval) = (yyvsp[0].ssym).stoken; }
#line 4313 "c-exp.c.tmp"
    break;

  case 279:
#line 1880 "c-exp.y"
    { (yyval.sval) = (yyvsp[0].ssym).stoken; }
#line 4319 "c-exp.c.tmp"
    break;

  case 280:
#line 1881 "c-exp.y"
    { (yyval.sval) = (yyvsp[0].sval); }
#line 4325 "c-exp.c.tmp"
    break;

  case 283:
#line 1894 "c-exp.y"
    {
			  struct field_of_this_result is_a_field_of_this;

			  (yyval.ssym).stoken = (yyvsp[0].sval);
			  (yyval.ssym).sym
			    = lookup_symbol ((yyvsp[0].sval).ptr,
					     pstate->expression_context_block,
					     SEARCH_VFT,
					     &is_a_field_of_this);
			  (yyval.ssym).is_a_field_of_this
			    = is_a_field_of_this.type != NULL;
			}
#line 4342 "c-exp.c.tmp"
    break;


#line 4346 "c-exp.c.tmp"

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
#line 1909 "c-exp.y"


/* Returns a stoken of the operator name given by OP (which does not
   include the string "operator").  */

static struct stoken
operator_stoken (const char *op)
{
  struct stoken st = { NULL, 0 };
  char *buf;

  st.length = CP_OPERATOR_LEN + strlen (op);
  buf = (char *) xmalloc (st.length + 1);
  strcpy (buf, CP_OPERATOR_STR);
  strcat (buf, op);
  st.ptr = buf;

  /* The toplevel (c_parse) will free the memory allocated here.  */
  cpstate->strings.emplace_back (buf);
  return st;
};

/* Returns a stoken of the type named TYPE.  */

static qualified_name_token
typename_stoken (const char *type)
{
  return qualified_name_token { nullptr, type, false };
};

/* Return true if the type is aggregate-like.  */

static int
type_aggregate_p (struct type *type)
{
  return (type->code () == TYPE_CODE_STRUCT
	  || type->code () == TYPE_CODE_UNION
	  || type->code () == TYPE_CODE_NAMESPACE
	  || (type->code () == TYPE_CODE_ENUM
	      && type->is_declared_class ()));
}

/* Validate a parameter typelist.  */

static void
check_parameter_typelist (std::vector<struct type *> *params)
{
  struct type *type;
  int ix;

  for (ix = 0; ix < params->size (); ++ix)
    {
      type = (*params)[ix];
      if (type != NULL && check_typedef (type)->code () == TYPE_CODE_VOID)
	{
	  if (ix == 0)
	    {
	      if (params->size () == 1)
		{
		  /* Ok.  */
		  break;
		}
	      error (_("parameter types following 'void'"));
	    }
	  else
	    error (_("'void' invalid as parameter type"));
	}
    }
}

/* Take care of parsing a number (anything that starts with a digit).
   Set yylval and return the token type; update lexptr.
   LEN is the number of characters in it.  */

/*** Needs some error checking for the float case ***/

static int
parse_number (struct parser_state *par_state,
	      const char *buf, int len, int parsed_float, c_exp_YYSTYPE *putithere)
{
  ULONGEST n = 0;
  ULONGEST prevn = 0;

  int i = 0;
  int c;
  int base = input_radix;
  int unsigned_p = 0;

  /* Number of "L" suffixes encountered.  */
  int long_p = 0;

  /* Imaginary number.  */
  bool imaginary_p = false;

  /* We have found a "L" or "U" (or "i") suffix.  */
  int found_suffix = 0;

  if (parsed_float)
    {
      if (len >= 1 && buf[len - 1] == 'i')
	{
	  imaginary_p = true;
	  --len;
	}

      /* Handle suffixes for decimal floating-point: "df", "dd" or "dl".  */
      if (len >= 2 && buf[len - 2] == 'd' && buf[len - 1] == 'f')
	{
	  putithere->typed_val_float.type
	    = parse_type (par_state)->builtin_decfloat;
	  len -= 2;
	}
      else if (len >= 2 && buf[len - 2] == 'd' && buf[len - 1] == 'd')
	{
	  putithere->typed_val_float.type
	    = parse_type (par_state)->builtin_decdouble;
	  len -= 2;
	}
      else if (len >= 2 && buf[len - 2] == 'd' && buf[len - 1] == 'l')
	{
	  putithere->typed_val_float.type
	    = parse_type (par_state)->builtin_declong;
	  len -= 2;
	}
      /* Handle suffixes: 'f' for float, 'l' for long double.  */
      else if (len >= 1 && c_tolower (buf[len - 1]) == 'f')
	{
	  putithere->typed_val_float.type
	    = parse_type (par_state)->builtin_float;
	  len -= 1;
	}
      else if (len >= 1 && c_tolower (buf[len - 1]) == 'l')
	{
	  putithere->typed_val_float.type
	    = parse_type (par_state)->builtin_long_double;
	  len -= 1;
	}
      /* Default type for floating-point literals is double.  */
      else
	{
	  putithere->typed_val_float.type
	    = parse_type (par_state)->builtin_double;
	}

      if (!parse_float (buf, len,
			putithere->typed_val_float.type,
			putithere->typed_val_float.val))
	return ERROR;

      if (imaginary_p)
	putithere->typed_val_float.type
	  = init_complex_type (nullptr, putithere->typed_val_float.type);

      return imaginary_p ? COMPLEX_FLOAT : FLOAT;
    }

  /* Handle base-switching prefixes 0x, 0t, 0d, 0 */
  if (buf[0] == '0' && len > 1)
    switch (buf[1])
      {
      case 'x':
      case 'X':
	if (len >= 3)
	  {
	    buf += 2;
	    base = 16;
	    len -= 2;
	  }
	break;

      case 'b':
      case 'B':
	if (len >= 3)
	  {
	    buf += 2;
	    base = 2;
	    len -= 2;
	  }
	break;

      case 't':
      case 'T':
      case 'd':
      case 'D':
	if (len >= 3)
	  {
	    buf += 2;
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
      c = *buf++;
      if (c >= 'A' && c <= 'Z')
	c += 'a' - 'A';
      if (c != 'l' && c != 'u' && c != 'i')
	n *= base;
      if (c >= '0' && c <= '9')
	{
	  if (found_suffix)
	    return ERROR;
	  n += i = c - '0';
	}
      else
	{
	  if (base > 10 && c >= 'a' && c <= 'f')
	    {
	      if (found_suffix)
		return ERROR;
	      n += i = c - 'a' + 10;
	    }
	  else if (c == 'l')
	    {
	      ++long_p;
	      found_suffix = 1;
	    }
	  else if (c == 'u')
	    {
	      unsigned_p = 1;
	      found_suffix = 1;
	    }
	  else if (c == 'i')
	    {
	      imaginary_p = true;
	      found_suffix = 1;
	    }
	  else
	    return ERROR;	/* Char not a digit */
	}
      if (i >= base)
	return ERROR;		/* Invalid digit in this base */

      if (c != 'l' && c != 'u' && c != 'i')
	{
	  /* Test for overflow.  */
	  if (prevn == 0 && n == 0)
	    ;
	  else if (prevn >= n)
	    error (_("Numeric constant too large."));
	}
      prevn = n;
    }

  /* An integer constant is an int, a long, or a long long.  An L
     suffix forces it to be long; an LL suffix forces it to be long
     long.  If not forced to a larger size, it gets the first type of
     the above that it fits in.  To figure out whether it fits, we
     shift it right and see whether anything remains.  Note that we
     can't shift sizeof (LONGEST) * HOST_CHAR_BIT bits or more in one
     operation, because many compilers will warn about such a shift
     (which always produces a zero result).  Sometimes gdbarch_int_bit
     or gdbarch_long_bit will be that big, sometimes not.  To deal with
     the case where it is we just always shift the value more than
     once, with fewer bits each time.  */
  int int_bits = gdbarch_int_bit (par_state->gdbarch ());
  int long_bits = gdbarch_long_bit (par_state->gdbarch ());
  int long_long_bits = gdbarch_long_long_bit (par_state->gdbarch ());
  bool have_signed
    /* No 'u' suffix.  */
    = !unsigned_p;
  bool have_unsigned
    = ((/* 'u' suffix.  */
	unsigned_p)
       || (/* Not a decimal.  */
	   base != 10)
       || (/* Allowed as a convenience, in case decimal doesn't fit in largest
	      signed type.  */
	   !fits_in_type (1, n, long_long_bits, true)));
  bool have_int
    /* No 'l' or 'll' suffix.  */
    = long_p == 0;
  bool have_long
    /* No 'll' suffix.  */
    = long_p <= 1;
  if (have_int && have_signed && fits_in_type (1, n, int_bits, true))
    putithere->typed_val_int.type = parse_type (par_state)->builtin_int;
  else if (have_int && have_unsigned && fits_in_type (1, n, int_bits, false))
    putithere->typed_val_int.type
      = parse_type (par_state)->builtin_unsigned_int;
  else if (have_long && have_signed && fits_in_type (1, n, long_bits, true))
    putithere->typed_val_int.type = parse_type (par_state)->builtin_long;
  else if (have_long && have_unsigned && fits_in_type (1, n, long_bits, false))
    putithere->typed_val_int.type
      = parse_type (par_state)->builtin_unsigned_long;
  else if (have_signed && fits_in_type (1, n, long_long_bits, true))
    putithere->typed_val_int.type
      = parse_type (par_state)->builtin_long_long;
  else if (have_unsigned && fits_in_type (1, n, long_long_bits, false))
    putithere->typed_val_int.type
      = parse_type (par_state)->builtin_unsigned_long_long;
  else
    error (_("Numeric constant too large."));
  putithere->typed_val_int.val = n;

   if (imaginary_p)
     putithere->typed_val_int.type
       = init_complex_type (nullptr, putithere->typed_val_int.type);

   return imaginary_p ? COMPLEX_INT : INT;
}

/* Temporary obstack used for holding strings.  */
static struct obstack tempbuf;
static int tempbuf_init;

/* Parse a C escape sequence.  The initial backslash of the sequence
   is at (*PTR)[-1].  *PTR will be updated to point to just after the
   last character of the sequence.  If OUTPUT is not NULL, the
   translated form of the escape sequence will be written there.  If
   OUTPUT is NULL, no output is written and the call will only affect
   *PTR.  If an escape sequence is expressed in target bytes, then the
   entire sequence will simply be copied to OUTPUT.  Return 1 if any
   character was emitted, 0 otherwise.  */

int
c_parse_escape (const char **ptr, struct obstack *output)
{
  const char *tokptr = *ptr;
  int result = 1;

  /* Some escape sequences undergo character set conversion.  Those we
     translate here.  */
  switch (*tokptr)
    {
      /* Hex escapes do not undergo character set conversion, so keep
	 the escape sequence for later.  */
    case 'x':
      if (output)
	obstack_grow_str (output, "\\x");
      ++tokptr;
      if (!c_isxdigit (*tokptr))
	error (_("\\x escape without a following hex digit"));
      while (c_isxdigit (*tokptr))
	{
	  if (output)
	    obstack_1grow (output, *tokptr);
	  ++tokptr;
	}
      break;

      /* Octal escapes do not undergo character set conversion, so
	 keep the escape sequence for later.  */
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
      {
	int i;
	if (output)
	  obstack_grow_str (output, "\\");
	for (i = 0;
	     i < 3 && c_isdigit (*tokptr) && *tokptr != '8' && *tokptr != '9';
	     ++i)
	  {
	    if (output)
	      obstack_1grow (output, *tokptr);
	    ++tokptr;
	  }
      }
      break;

      /* We handle UCNs later.  We could handle them here, but that
	 would mean a spurious error in the case where the UCN could
	 be converted to the target charset but not the host
	 charset.  */
    case 'u':
    case 'U':
      {
	char c = *tokptr;
	int i, len = c == 'U' ? 8 : 4;
	if (output)
	  {
	    obstack_1grow (output, '\\');
	    obstack_1grow (output, *tokptr);
	  }
	++tokptr;
	if (!c_isxdigit (*tokptr))
	  error (_("\\%c escape without a following hex digit"), c);
	for (i = 0; i < len && c_isxdigit (*tokptr); ++i)
	  {
	    if (output)
	      obstack_1grow (output, *tokptr);
	    ++tokptr;
	  }
      }
      break;

      /* We must pass backslash through so that it does not
	 cause quoting during the second expansion.  */
    case '\\':
      if (output)
	obstack_grow_str (output, "\\\\");
      ++tokptr;
      break;

      /* Escapes which undergo conversion.  */
    case 'a':
      if (output)
	obstack_1grow (output, '\a');
      ++tokptr;
      break;
    case 'b':
      if (output)
	obstack_1grow (output, '\b');
      ++tokptr;
      break;
    case 'f':
      if (output)
	obstack_1grow (output, '\f');
      ++tokptr;
      break;
    case 'n':
      if (output)
	obstack_1grow (output, '\n');
      ++tokptr;
      break;
    case 'r':
      if (output)
	obstack_1grow (output, '\r');
      ++tokptr;
      break;
    case 't':
      if (output)
	obstack_1grow (output, '\t');
      ++tokptr;
      break;
    case 'v':
      if (output)
	obstack_1grow (output, '\v');
      ++tokptr;
      break;

      /* GCC extension.  */
    case 'e':
      if (output)
	obstack_1grow (output, HOST_ESCAPE_CHAR);
      ++tokptr;
      break;

      /* Backslash-newline expands to nothing at all.  */
    case '\n':
      ++tokptr;
      result = 0;
      break;

      /* A few escapes just expand to the character itself.  */
    case '\'':
    case '\"':
    case '?':
      /* GCC extensions.  */
    case '(':
    case '{':
    case '[':
    case '%':
      /* Unrecognized escapes turn into the character itself.  */
    default:
      if (output)
	obstack_1grow (output, *tokptr);
      ++tokptr;
      break;
    }
  *ptr = tokptr;
  return result;
}

/* Parse a string or character literal from TOKPTR.  The string or
   character may be wide or unicode.  *OUTPTR is set to just after the
   end of the literal in the input string.  The resulting token is
   stored in VALUE.  This returns a token value, either STRING or
   CHAR, depending on what was parsed.  *HOST_CHARS is set to the
   number of host characters in the literal.  */

static int
parse_string_or_char (const char *tokptr, const char **outptr,
		      struct typed_stoken *value, int *host_chars)
{
  int quote;
  c_string_type type;
  int is_objc = 0;

  /* Build the gdb internal form of the input string in tempbuf.  Note
     that the buffer is null byte terminated *only* for the
     convenience of debugging gdb itself and printing the buffer
     contents when the buffer contains no embedded nulls.  Gdb does
     not depend upon the buffer being null byte terminated, it uses
     the length string instead.  This allows gdb to handle C strings
     (as well as strings in other languages) with embedded null
     bytes */

  if (!tempbuf_init)
    tempbuf_init = 1;
  else
    obstack_free (&tempbuf, NULL);
  obstack_init (&tempbuf);

  /* Record the string type.  */
  if (*tokptr == 'L')
    {
      type = C_WIDE_STRING;
      ++tokptr;
    }
  else if (*tokptr == 'u')
    {
      type = C_STRING_16;
      ++tokptr;
    }
  else if (*tokptr == 'U')
    {
      type = C_STRING_32;
      ++tokptr;
    }
  else if (*tokptr == '@')
    {
      /* An Objective C string.  */
      is_objc = 1;
      type = C_STRING;
      ++tokptr;
    }
  else
    type = C_STRING;

  /* Skip the quote.  */
  quote = *tokptr;
  if (quote == '\'')
    type |= C_CHAR;
  ++tokptr;

  *host_chars = 0;

  while (*tokptr)
    {
      char c = *tokptr;
      if (c == '\\')
	{
	  ++tokptr;
	  *host_chars += c_parse_escape (&tokptr, &tempbuf);
	}
      else if (c == quote)
	break;
      else
	{
	  obstack_1grow (&tempbuf, c);
	  ++tokptr;
	  /* FIXME: this does the wrong thing with multi-byte host
	     characters.  We could use mbrlen here, but that would
	     make "set host-charset" a bit less useful.  */
	  ++*host_chars;
	}
    }

  if (*tokptr != quote)
    {
      if (quote == '"')
	error (_("Unterminated string in expression."));
      else
	error (_("Unmatched single quote."));
    }
  ++tokptr;

  value->type = type;
  value->ptr = (char *) obstack_base (&tempbuf);
  value->length = obstack_object_size (&tempbuf);

  *outptr = tokptr;

  return quote == '"' ? (is_objc ? NSSTRING : STRING) : CHAR;
}

/* This is used to associate some attributes with a token.  */

enum token_flag
{
  /* If this bit is set, the token is C++-only.  */

  FLAG_CXX = 1,

  /* If this bit is set, the token is C-only.  */

  FLAG_C = 2,

  /* If this bit is set, the token is conditional: if there is a
     symbol of the same name, then the token is a symbol; otherwise,
     the token is a keyword.  */

  FLAG_SHADOW = 4
};
DEF_ENUM_FLAGS_TYPE (enum token_flag, token_flags);

struct c_token
{
  const char *oper;
  int token;
  enum exp_opcode opcode;
  token_flags flags;
};

static const struct c_token tokentab3[] =
  {
    {">>=", ASSIGN_MODIFY, BINOP_RSH, 0},
    {"<<=", ASSIGN_MODIFY, BINOP_LSH, 0},
    {"->*", ARROW_STAR, OP_NULL, FLAG_CXX},
    {"...", DOTDOTDOT, OP_NULL, 0}
  };

static const struct c_token tokentab2[] =
  {
    {"+=", ASSIGN_MODIFY, BINOP_ADD, 0},
    {"-=", ASSIGN_MODIFY, BINOP_SUB, 0},
    {"*=", ASSIGN_MODIFY, BINOP_MUL, 0},
    {"/=", ASSIGN_MODIFY, BINOP_DIV, 0},
    {"%=", ASSIGN_MODIFY, BINOP_REM, 0},
    {"|=", ASSIGN_MODIFY, BINOP_BITWISE_IOR, 0},
    {"&=", ASSIGN_MODIFY, BINOP_BITWISE_AND, 0},
    {"^=", ASSIGN_MODIFY, BINOP_BITWISE_XOR, 0},
    {"++", INCREMENT, OP_NULL, 0},
    {"--", DECREMENT, OP_NULL, 0},
    {"->", ARROW, OP_NULL, 0},
    {"&&", ANDAND, OP_NULL, 0},
    {"||", OROR, OP_NULL, 0},
    /* "::" is *not* only C++: gdb overrides its meaning in several
       different ways, e.g., 'filename'::func, function::variable.  */
    {"::", COLONCOLON, OP_NULL, 0},
    {"<<", LSH, OP_NULL, 0},
    {">>", RSH, OP_NULL, 0},
    {"==", EQUAL, OP_NULL, 0},
    {"!=", NOTEQUAL, OP_NULL, 0},
    {"<=", LEQ, OP_NULL, 0},
    {">=", GEQ, OP_NULL, 0},
    {".*", DOT_STAR, OP_NULL, FLAG_CXX}
  };

/* Identifier-like tokens.  Only type-specifiers than can appear in
   multi-word type names (for example 'double' can appear in 'long
   double') need to be listed here.  type-specifiers that are only ever
   single word (like 'char') are handled by the classify_name function.  */
static const struct c_token ident_tokens[] =
  {
    {"unsigned", UNSIGNED, OP_NULL, 0},
    {"template", TEMPLATE, OP_NULL, FLAG_CXX},
    {"volatile", VOLATILE_KEYWORD, OP_NULL, 0},
    {"struct", STRUCT, OP_NULL, 0},
    {"signed", SIGNED_KEYWORD, OP_NULL, 0},
    {"sizeof", SIZEOF, OP_NULL, 0},
    {"_Alignof", ALIGNOF, OP_NULL, 0},
    {"alignof", ALIGNOF, OP_NULL, FLAG_CXX},
    {"double", DOUBLE_KEYWORD, OP_NULL, 0},
    {"float", FLOAT_KEYWORD, OP_NULL, 0},
    {"false", FALSEKEYWORD, OP_NULL, FLAG_CXX},
    {"class", CLASS, OP_NULL, FLAG_CXX},
    {"union", UNION, OP_NULL, 0},
    {"short", SHORT, OP_NULL, 0},
    {"const", CONST_KEYWORD, OP_NULL, 0},
    {"restrict", RESTRICT, OP_NULL, FLAG_C | FLAG_SHADOW},
    {"__restrict__", RESTRICT, OP_NULL, 0},
    {"__restrict", RESTRICT, OP_NULL, 0},
    {"_Atomic", ATOMIC, OP_NULL, 0},
    {"enum", ENUM, OP_NULL, 0},
    {"long", LONG, OP_NULL, 0},
    {"_Complex", COMPLEX, OP_NULL, 0},
    {"__complex__", COMPLEX, OP_NULL, 0},

    {"true", TRUEKEYWORD, OP_NULL, FLAG_CXX},
    {"int", INT_KEYWORD, OP_NULL, 0},
    {"new", NEW, OP_NULL, FLAG_CXX},
    {"delete", DELETE, OP_NULL, FLAG_CXX},
    {"operator", OPERATOR, OP_NULL, FLAG_CXX},

    {"and", ANDAND, OP_NULL, FLAG_CXX},
    {"and_eq", ASSIGN_MODIFY, BINOP_BITWISE_AND, FLAG_CXX},
    {"bitand", '&', OP_NULL, FLAG_CXX},
    {"bitor", '|', OP_NULL, FLAG_CXX},
    {"compl", '~', OP_NULL, FLAG_CXX},
    {"not", '!', OP_NULL, FLAG_CXX},
    {"not_eq", NOTEQUAL, OP_NULL, FLAG_CXX},
    {"or", OROR, OP_NULL, FLAG_CXX},
    {"or_eq", ASSIGN_MODIFY, BINOP_BITWISE_IOR, FLAG_CXX},
    {"xor", '^', OP_NULL, FLAG_CXX},
    {"xor_eq", ASSIGN_MODIFY, BINOP_BITWISE_XOR, FLAG_CXX},

    {"const_cast", CONST_CAST, OP_NULL, FLAG_CXX },
    {"dynamic_cast", DYNAMIC_CAST, OP_NULL, FLAG_CXX },
    {"static_cast", STATIC_CAST, OP_NULL, FLAG_CXX },
    {"reinterpret_cast", REINTERPRET_CAST, OP_NULL, FLAG_CXX },

    {"__typeof__", TYPEOF, OP_TYPEOF, 0 },
    {"__typeof", TYPEOF, OP_TYPEOF, 0 },
    {"typeof", TYPEOF, OP_TYPEOF, FLAG_SHADOW },
    {"__decltype", DECLTYPE, OP_DECLTYPE, FLAG_CXX },
    {"decltype", DECLTYPE, OP_DECLTYPE, FLAG_CXX | FLAG_SHADOW },

    {"typeid", TYPEID, OP_TYPEID, FLAG_CXX}
  };


static void
scan_macro_expansion (const char *expansion)
{
  /* We'd better not be trying to push the stack twice.  */
  gdb_assert (! cpstate->macro_original_text);

  /* Copy to the obstack.  */
  const char *copy = obstack_strdup (&cpstate->expansion_obstack, expansion);

  /* Save the old lexptr value, so we can return to it when we're done
     parsing the expanded text.  */
  cpstate->macro_original_text = pstate->lexptr;
  pstate->lexptr = copy;
}

static int
scanning_macro_expansion (void)
{
  return cpstate->macro_original_text != 0;
}

static void
finished_macro_expansion (void)
{
  /* There'd better be something to pop back to.  */
  gdb_assert (cpstate->macro_original_text);

  /* Pop back to the original text.  */
  pstate->lexptr = cpstate->macro_original_text;
  cpstate->macro_original_text = 0;
}

/* Return true iff the token represents a C++ cast operator.  */

static int
is_cast_operator (const char *token, int len)
{
  return (! strncmp (token, "dynamic_cast", len)
	  || ! strncmp (token, "static_cast", len)
	  || ! strncmp (token, "reinterpret_cast", len)
	  || ! strncmp (token, "const_cast", len));
}

/* The scope used for macro expansion.  */
static struct macro_scope *expression_macro_scope;

/* This is set if a NAME token appeared at the very end of the input
   string, with no whitespace separating the name from the EOF.  This
   is used only when parsing to do field name completion.  */
static int saw_name_at_eof;

/* This is set if the previously-returned token was a structure
   operator -- either '.' or ARROW.  */
static bool last_was_structop;

/* Depth of parentheses.  */
static int paren_depth;

/* Lex an Objective-C @selector.  Return true if lexed.  In this case,
   sets the resulting token and updates the lex pointer.  Otherwise
   returns false and updates nothing.  */

static bool
lex_selector (const char **lex_ptr, struct stoken *token)
{
  const char *p = *lex_ptr;

  if (!startswith (p, "selector"))
    return false;

  p += strlen ("selector");
  p = skip_spaces (p);
  if (*p != '(')
    return false;
  ++p;

  /* The selector name matches [A-Za-z0-9:_-]+.  We could probably be
     a bit more refined but meh.  */
  const char *start = p;
  while (c_isalnum (*p) || *p == ':' || *p == '_' || *p == '-')
    ++p;
  if (p == start)
    return false;
  const char *end = p;

  p = skip_spaces (p);
  if (*p != ')')
    return false;
  ++p;

  *lex_ptr = p;
  *token = { start, (int) (end - start) };
  return true;
}

/* Read one token, getting characters through lexptr.  */

static int
lex_one_token (struct parser_state *par_state, bool *is_quoted_name)
{
  int c;
  int namelen;
  const char *tokstart;
  bool saw_structop = last_was_structop;

  last_was_structop = false;
  *is_quoted_name = false;

 retry:

  /* Check if this is a macro invocation that we need to expand.  */
  if (! scanning_macro_expansion ())
    {
      gdb::unique_xmalloc_ptr<char> expanded
	= macro_expand_next (&pstate->lexptr, *expression_macro_scope);

      if (expanded != nullptr)
	scan_macro_expansion (expanded.get ());
    }

  pstate->prev_lexptr = pstate->lexptr;

  tokstart = pstate->lexptr;
  /* See if it is a special token of length 3.  */
  for (const auto &token : tokentab3)
    if (strncmp (tokstart, token.oper, 3) == 0)
      {
	if ((token.flags & FLAG_CXX) != 0
	    && par_state->language ()->la_language != language_cplus)
	  break;
	gdb_assert ((token.flags & FLAG_C) == 0);

	pstate->lexptr += 3;
	yylval.opcode = token.opcode;
	return token.token;
      }

  /* See if it is a special token of length 2.  */
  for (const auto &token : tokentab2)
    if (strncmp (tokstart, token.oper, 2) == 0)
      {
	if ((token.flags & FLAG_CXX) != 0
	    && par_state->language ()->la_language != language_cplus)
	  break;
	gdb_assert ((token.flags & FLAG_C) == 0);

	pstate->lexptr += 2;
	yylval.opcode = token.opcode;
	if (token.token == ARROW)
	  last_was_structop = 1;
	return token.token;
      }

  switch (c = *tokstart)
    {
    case 0:
      /* If we were just scanning the result of a macro expansion,
	 then we need to resume scanning the original text.
	 If we're parsing for field name completion, and the previous
	 token allows such completion, return a COMPLETE token.
	 Otherwise, we were already scanning the original text, and
	 we're really done.  */
      if (scanning_macro_expansion ())
	{
	  finished_macro_expansion ();
	  goto retry;
	}
      else if (saw_name_at_eof)
	{
	  saw_name_at_eof = 0;
	  return COMPLETE;
	}
      else if (par_state->parse_completion && saw_structop)
	return COMPLETE;
      else
	return 0;

    case ' ':
    case '\t':
    case '\n':
      pstate->lexptr++;
      goto retry;

    case '[':
    case '(':
      paren_depth++;
      pstate->lexptr++;
      if (par_state->language ()->la_language == language_objc
	  && c == '[')
	return OBJC_LBRAC;
      return c;

    case ']':
    case ')':
      if (paren_depth == 0)
	return 0;
      paren_depth--;
      pstate->lexptr++;
      return c;

    case ',':
      if (pstate->comma_terminates
	  && paren_depth == 0
	  && ! scanning_macro_expansion ())
	return 0;
      pstate->lexptr++;
      return c;

    case '.':
      /* Might be a floating point number.  */
      if (pstate->lexptr[1] < '0' || pstate->lexptr[1] > '9')
	{
	  last_was_structop = true;
	  goto symbol;		/* Nope, must be a symbol. */
	}
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
	int got_dot = 0, got_e = 0, got_p = 0, toktype;
	const char *p = tokstart;
	int hex = input_radix > 10;

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
	    if (!hex && !got_e && !got_p && (*p == 'e' || *p == 'E'))
	      got_dot = got_e = 1;
	    else if (!got_e && !got_p && (*p == 'p' || *p == 'P'))
	      got_dot = got_p = 1;
	    /* This test does not include !hex, because a '.' always indicates
	       a decimal floating point number regardless of the radix.  */
	    else if (!got_dot && *p == '.')
	      got_dot = 1;
	    else if (((got_e && (p[-1] == 'e' || p[-1] == 'E'))
		      || (got_p && (p[-1] == 'p' || p[-1] == 'P')))
		     && (*p == '-' || *p == '+'))
	      {
		/* This is the sign of the exponent, not the end of
		   the number.  */
	      }
	    else if (*p == '\'')
	      {
		if (!no_tick.has_value ())
		  no_tick.emplace (tokstart, p);
		continue;
	      }
	    /* We will take any letters or digits.  parse_number will
	       complain if past the radix, or if L or U are not final.  */
	    else if ((*p < '0' || *p > '9')
		     && ((*p < 'a' || *p > 'z')
				  && (*p < 'A' || *p > 'Z')))
	      break;
	    if (no_tick.has_value ())
	      no_tick->push_back (*p);
	  }
	if (no_tick.has_value ())
	  toktype = parse_number (par_state, no_tick->c_str (),
				  no_tick->length (),
				  got_dot | got_e | got_p, &yylval);
	else
	  toktype = parse_number (par_state, tokstart, p - tokstart,
				  got_dot | got_e | got_p, &yylval);
	if (toktype == ERROR)
	  error (_("Invalid number \"%.*s\"."), (int) (p - tokstart),
		 tokstart);
	pstate->lexptr = p;
	return toktype;
      }

    case '@':
      {
	const char *p = &tokstart[1];

	if (par_state->language ()->la_language == language_objc)
	  {
	    struct stoken sel_token;
	    if (lex_selector (&p, &sel_token))
	      {
		pstate->lexptr = p;
		yylval.sval = sel_token;
		return SELECTOR;
	      }
	    else if (*p == '"')
	      goto parse_string;
	  }

	while (c_isspace (*p))
	  p++;
	size_t len = strlen ("entry");
	if (strncmp (p, "entry", len) == 0 && !c_ident_is_alnum (p[len])
	    && p[len] != '_')
	  {
	    pstate->lexptr = &p[len];
	    return ENTRY;
	  }
      }
      [[fallthrough]];
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '|':
    case '&':
    case '^':
    case '~':
    case '!':
    case '<':
    case '>':
    case '?':
    case ':':
    case '=':
    case '{':
    case '}':
    symbol:
      pstate->lexptr++;
      return c;

    case 'L':
    case 'u':
    case 'U':
      if (tokstart[1] != '"' && tokstart[1] != '\'')
	break;
      [[fallthrough]];
    case '\'':
    case '"':

    parse_string:
      {
	int host_len;
	int result = parse_string_or_char (tokstart, &pstate->lexptr,
					   &yylval.tsval, &host_len);
	if (result == CHAR)
	  {
	    if (host_len == 0)
	      error (_("Empty character constant."));
	    else if (host_len > 2 && c == '\'')
	      {
		++tokstart;
		namelen = pstate->lexptr - tokstart - 1;
		*is_quoted_name = true;

		goto tryname;
	      }
	    else if (host_len > 1)
	      error (_("Invalid character constant."));
	  }
	return result;
      }
    }

  if (!(c == '_' || c == '$' || c_ident_is_alpha (c)))
    /* We must have come across a bad character (e.g. ';').  */
    error (_("Invalid character '%c' in expression."), c);

  /* It's a name.  See how long it is.  */
  namelen = 0;
  for (c = tokstart[namelen];
       (c == '_' || c == '$' || c_ident_is_alnum (c) || c == '<');)
    {
      /* Template parameter lists are part of the name.
	 FIXME: This mishandles `print $a<4&&$a>3'.  */

      if (c == '<')
	{
	  if (! is_cast_operator (tokstart, namelen))
	    {
	      /* Scan ahead to get rest of the template specification.  Note
		 that we look ahead only when the '<' adjoins non-whitespace
		 characters; for comparison expressions, e.g. "a < b > c",
		 there must be spaces before the '<', etc. */
	      const char *p = find_template_name_end (tokstart + namelen);

	      if (p)
		namelen = p - tokstart;
	    }
	  break;
	}
      c = tokstart[++namelen];
    }

  /* The token "if" terminates the expression and is NOT removed from
     the input stream.  It doesn't count if it appears in the
     expansion of a macro.  */
  if (namelen == 2
      && tokstart[0] == 'i'
      && tokstart[1] == 'f'
      && ! scanning_macro_expansion ())
    {
      return 0;
    }

  /* For the same reason (breakpoint conditions), "thread N"
     terminates the expression.  "thread" could be an identifier, but
     an identifier is never followed by a number without intervening
     punctuation.  "task" is similar.  Handle abbreviations of these,
     similarly to breakpoint.c:find_condition_and_thread.  */
  if (namelen >= 1
      && (strncmp (tokstart, "thread", namelen) == 0
	  || strncmp (tokstart, "task", namelen) == 0)
      && (tokstart[namelen] == ' ' || tokstart[namelen] == '\t')
      && ! scanning_macro_expansion ())
    {
      const char *p = skip_spaces (tokstart + namelen + 1);
      if (*p >= '0' && *p <= '9')
	return 0;
    }

  pstate->lexptr += namelen;

  tryname:

  yylval.sval.ptr = tokstart;
  yylval.sval.length = namelen;

  /* Catch specific keywords.  */
  std::string copy = copy_name (yylval.sval);
  for (const auto &token : ident_tokens)
    if (copy == token.oper)
      {
	if ((token.flags & FLAG_CXX) != 0
	    && par_state->language ()->la_language != language_cplus)
	  break;
	if ((token.flags & FLAG_C) != 0
	    && par_state->language ()->la_language != language_c
	    && par_state->language ()->la_language != language_objc)
	  break;

	if ((token.flags & FLAG_SHADOW) != 0)
	  {
	    struct field_of_this_result is_a_field_of_this;

	    if (lookup_symbol (copy.c_str (),
			       pstate->expression_context_block,
			       SEARCH_VFT, &is_a_field_of_this).symbol
		!= NULL)
	      {
		/* The keyword is shadowed.  */
		break;
	      }
	  }

	/* It is ok to always set this, even though we don't always
	   strictly need to.  */
	yylval.opcode = token.opcode;
	return token.token;
      }

  if (*tokstart == '$')
    return DOLLAR_VARIABLE;

  if (pstate->parse_completion && *pstate->lexptr == '\0')
    saw_name_at_eof = 1;

  yylval.ssym.stoken = yylval.sval;
  yylval.ssym.sym.symbol = NULL;
  yylval.ssym.sym.block = NULL;
  yylval.ssym.is_a_field_of_this = 0;
  return NAME;
}

/* An object of this type is pushed on a FIFO by the "outer" lexer.  */
struct c_token_and_value
{
  int token;
  c_exp_YYSTYPE value;
};

/* A FIFO of tokens that have been read but not yet returned to the
   parser.  */
static std::vector<c_token_and_value> token_fifo;

/* Non-zero if the lexer should return tokens from the FIFO.  */
static int popping;

/* Temporary storage for c_lex; this holds symbol names as they are
   built up.  */
static auto_obstack name_obstack;

/* Classify a NAME token.  The contents of the token are in `yylval'.
   Updates yylval and returns the new token type.  BLOCK is the block
   in which lookups start; this can be NULL to mean the global scope.
   IS_QUOTED_NAME is non-zero if the name token was originally quoted
   in single quotes.  IS_AFTER_STRUCTOP is true if this name follows
   a structure operator -- either '.' or ARROW  */

static int
classify_name (struct parser_state *par_state, const struct block *block,
	       bool is_quoted_name, bool is_after_structop)
{
  struct block_symbol bsym;
  struct field_of_this_result is_a_field_of_this;

  std::string copy = copy_name (yylval.sval);

  bsym = lookup_symbol (copy.c_str (), block, SEARCH_VFT,
			&is_a_field_of_this);

  if (bsym.symbol && bsym.symbol->loc_class () == LOC_BLOCK)
    {
      yylval.ssym.sym = bsym;
      yylval.ssym.is_a_field_of_this = is_a_field_of_this.type != NULL;
      return BLOCKNAME;
    }
  else if (!bsym.symbol)
    {
      /* If we found a field of 'this', we might have erroneously
	 found a constructor where we wanted a type name.  Handle this
	 case by noticing that we found a constructor and then look up
	 the type tag instead.  */
      if (is_a_field_of_this.type != NULL
	  && is_a_field_of_this.fn_field != NULL
	  && TYPE_FN_FIELD_CONSTRUCTOR (is_a_field_of_this.fn_field->fn_fields,
					0))
	{
	  struct field_of_this_result inner_is_a_field_of_this;

	  bsym = lookup_symbol (copy.c_str (), block, SEARCH_STRUCT_DOMAIN,
				&inner_is_a_field_of_this);
	  if (bsym.symbol != NULL)
	    {
	      yylval.tsym.type = bsym.symbol->type ();
	      return TYPENAME;
	    }
	}

      /* If we found a field on the "this" object, or we are looking
	 up a field on a struct, then we want to prefer it over a
	 filename.  However, if the name was quoted, then it is better
	 to check for a filename or a block, since this is the only
	 way the user has of requiring the extension to be used.  */
      if ((is_a_field_of_this.type == NULL && !is_after_structop)
	  || is_quoted_name)
	{
	  /* See if it's a file name. */
	  if (auto symtab = lookup_symtab (current_program_space, copy.c_str ());
	      symtab != nullptr)
	    {
	      yylval.bval
		= symtab->compunit ().blockvector ()->static_block ();

	      return FILENAME;
	    }
	}
    }

  if (bsym.symbol && bsym.symbol->loc_class () == LOC_TYPEDEF)
    {
      yylval.tsym.type = bsym.symbol->type ();
      return TYPENAME;
    }

  /* See if it's an ObjC classname.  */
  if (par_state->language ()->la_language == language_objc && !bsym.symbol)
    {
      CORE_ADDR Class = lookup_objc_class (par_state->gdbarch (),
					   copy.c_str ());
      if (Class)
	{
	  struct symbol *sym;

	  yylval.theclass.theclass = Class;
	  sym = lookup_struct_noerr (copy.c_str (),
				     par_state->expression_context_block);
	  if (sym)
	    yylval.theclass.type = sym->type ();
	  return CLASSNAME;
	}
    }

  /* Input names that aren't symbols but ARE valid hex numbers, when
     the input radix permits them, can be names or numbers depending
     on the parse.  Note we support radixes > 16 here.  */
  if (!bsym.symbol
      && ((copy[0] >= 'a' && copy[0] < 'a' + input_radix - 10)
	  || (copy[0] >= 'A' && copy[0] < 'A' + input_radix - 10)))
    {
      c_exp_YYSTYPE newlval;	/* Its value is ignored.  */
      int hextype = parse_number (par_state, copy.c_str (), yylval.sval.length,
				  0, &newlval);

      if (hextype == INT)
	{
	  yylval.ssym.sym = bsym;
	  yylval.ssym.is_a_field_of_this = is_a_field_of_this.type != NULL;
	  return NAME_OR_INT;
	}
    }

  /* Any other kind of symbol */
  yylval.ssym.sym = bsym;
  yylval.ssym.is_a_field_of_this = is_a_field_of_this.type != NULL;

  if (bsym.symbol == NULL
      && par_state->language ()->la_language == language_cplus
      && is_a_field_of_this.type == NULL
      && lookup_minimal_symbol (current_program_space, copy.c_str ()).minsym == nullptr)
    return UNKNOWN_CPP_NAME;

  return NAME;
}

/* Like classify_name, but used by the inner loop of the lexer, when a
   name might have already been seen.  CONTEXT is the context type, or
   NULL if this is the first component of a name.  */

static int
classify_inner_name (struct parser_state *par_state,
		     const struct block *block, struct type *context)
{
  struct type *type;

  if (context == NULL)
    return classify_name (par_state, block, false, false);

  type = check_typedef (context);
  if (!type_aggregate_p (type))
    return ERROR;

  std::string copy = copy_name (yylval.ssym.stoken);
  /* N.B. We assume the symbol can only be in VAR_DOMAIN.  */
  yylval.ssym.sym = cp_lookup_nested_symbol (type, copy.c_str (), block,
					     SEARCH_VFT);

  /* If no symbol was found, search for a matching base class named
     COPY.  This will allow users to enter qualified names of class members
     relative to the `this' pointer.  */
  if (yylval.ssym.sym.symbol == NULL)
    {
      struct type *base_type = cp_find_type_baseclass_by_name (type,
							       copy.c_str ());

      if (base_type != NULL)
	{
	  yylval.tsym.type = base_type;
	  return TYPENAME;
	}

      return ERROR;
    }

  switch (yylval.ssym.sym.symbol->loc_class ())
    {
    case LOC_BLOCK:
    case LOC_LABEL:
      /* cp_lookup_nested_symbol might have accidentally found a constructor
	 named COPY when we really wanted a base class of the same name.
	 Double-check this case by looking for a base class.  */
      {
	struct type *base_type
	  = cp_find_type_baseclass_by_name (type, copy.c_str ());

	if (base_type != NULL)
	  {
	    yylval.tsym.type = base_type;
	    return TYPENAME;
	  }
      }
      return ERROR;

    case LOC_TYPEDEF:
      yylval.tsym.type = yylval.ssym.sym.symbol->type ();
      return TYPENAME;

    default:
      return NAME;
    }
  internal_error (_("not reached"));
}

/* A helper function for the specific case of a qualified field name,
   like "obj->type1::type2::field".  This takes the type prefix
   ("type1::type2" in the example) and finds the corresponding type.
   It will either throw an exception, or push a scope_operation on the
   operation stack.  */
static void
handle_qualified_field_name (qualified_name_token token)
{
  struct type *type = nullptr;
  std::string accum;
  for (const auto name : split_name (token.prefix, split_style::CXX))
    {
      std::string current (name);

      if (accum.empty ())
	accum = name;
      else
	accum = accum + "::" + current;

      yylval.ssym.stoken.ptr = current.c_str ();
      yylval.ssym.stoken.length = current.size ();
      yylval.ssym.sym = {};
      yylval.ssym.is_a_field_of_this = 0;

      int kind = classify_inner_name (pstate,
				      pstate->expression_context_block,
				      type);
      if (kind != TYPENAME)
	error (_("could not find type '%s'"), accum.c_str ());

      type = yylval.tsym.type;
    }

  type = check_typedef (type);
  if (!type_aggregate_p (type))
    error (_("`%s' is not defined as an aggregate type."),
	   type->safe_name ());
  if (token.name[0] == '~')
    destructor_name_p (token.name, type);
  pstate->push_new<scope_operation> (type, token.name);
}

/* The outer level of a two-level lexer.  This calls the inner lexer
   to return tokens.  It then either returns these tokens, or
   aggregates them into a larger token.  This lets us work around a
   problem in our parsing approach, where the parser could not
   distinguish between qualified names and qualified types at the
   right point.

   This approach is still not ideal, because it mishandles template
   types.  See the comment in lex_one_token for an example.  However,
   this is still an improvement over the earlier approach, and will
   suffice until we move to better parsing technology.  */

static int
yylex (void)
{
  c_token_and_value current;
  int first_was_coloncolon, last_was_coloncolon;
  struct type *context_type = NULL;
  int last_to_examine, next_to_examine, checkpoint;
  const struct block *search_block;
  bool is_quoted_name, last_lex_was_structop;

  if (popping && !token_fifo.empty ())
    goto do_pop;
  popping = 0;

  last_lex_was_structop = last_was_structop;

  /* Read the first token and decide what to do.  Most of the
     subsequent code is C++-only; but also depends on seeing a "::" or
     name-like token.  */
  current.token = lex_one_token (pstate, &is_quoted_name);
  if (cpstate->assume_classification == TYPE_CODE_UNDEF
      && current.token == NAME)
    current.token = classify_name (pstate, pstate->expression_context_block,
				   is_quoted_name, last_lex_was_structop);
  if (pstate->language ()->la_language != language_cplus
      || (current.token != TYPENAME && current.token != COLONCOLON
	  && current.token != FILENAME
	  && (cpstate->assume_classification == TYPE_CODE_UNDEF
	      || current.token != NAME))
      || cpstate->assume_classification == TYPE_CODE_VOID)
    return current.token;

  /* Read any sequence of alternating "::" and name-like tokens into
     the token FIFO.  */
  current.value = yylval;
  token_fifo.push_back (current);
  last_was_coloncolon = current.token == COLONCOLON;
  while (1)
    {
      bool ignore;

      /* We ignore quoted names other than the very first one.
	 Subsequent ones do not have any special meaning.  */
      current.token = lex_one_token (pstate, &ignore);
      current.value = yylval;
      token_fifo.push_back (current);

      if ((last_was_coloncolon && current.token != NAME)
	  || (!last_was_coloncolon && current.token != COLONCOLON))
	break;
      last_was_coloncolon = !last_was_coloncolon;
    }
  popping = 1;

  /* We always read one extra token, so compute the number of tokens
     to examine accordingly.  */
  last_to_examine = token_fifo.size () - 2;
  next_to_examine = 0;

  current = token_fifo[next_to_examine];
  ++next_to_examine;

  name_obstack.clear ();
  checkpoint = 0;
  if (current.token == FILENAME)
    search_block = current.value.bval;
  else if (current.token == COLONCOLON)
    search_block = NULL;
  else
    {
      gdb_assert (current.token == TYPENAME
		  || cpstate->assume_classification != TYPE_CODE_UNDEF);
      search_block = pstate->expression_context_block;
      obstack_grow (&name_obstack, current.value.sval.ptr,
		    current.value.sval.length);
      context_type = current.value.tsym.type;
      checkpoint = 1;
    }

  first_was_coloncolon = current.token == COLONCOLON;
  last_was_coloncolon = first_was_coloncolon;

  while (next_to_examine <= last_to_examine)
    {
      c_token_and_value next;

      next = token_fifo[next_to_examine];
      ++next_to_examine;

      if (next.token == NAME && last_was_coloncolon)
	{
	  int classification;

	  yylval = next.value;
	  if (cpstate->assume_classification != TYPE_CODE_UNDEF)
	    classification = NAME;
	  else
	    classification = classify_inner_name (pstate, search_block,
						  context_type);
	  /* We keep going until we either run out of names, or until
	     we have a qualified name which is not a type.  */
	  if (classification != TYPENAME && classification != NAME)
	    break;

	  /* Accept up to this token.  */
	  checkpoint = next_to_examine;

	  /* Update the partial name we are constructing.  */
	  if (next_to_examine > 1)
	    {
	      /* We don't want to put a leading "::" into the name.  */
	      obstack_grow_str (&name_obstack, "::");
	    }
	  obstack_grow (&name_obstack, next.value.sval.ptr,
			next.value.sval.length);

	  yylval.sval.ptr = (const char *) obstack_base (&name_obstack);
	  yylval.sval.length = obstack_object_size (&name_obstack);
	  current.value = yylval;
	  current.token = classification;

	  last_was_coloncolon = 0;

	  if (cpstate->assume_classification == TYPE_CODE_UNDEF
	      && classification == NAME)
	    break;

	  context_type = yylval.tsym.type;
	}
      else if (next.token == COLONCOLON && !last_was_coloncolon)
	last_was_coloncolon = 1;
      else
	{
	  /* We've reached the end of the name.  */
	  break;
	}
    }

  /* If we have a replacement token, install it as the first token in
     the FIFO, and delete the other constituent tokens.  */
  if (checkpoint > 0)
    {
      current.value.sval.ptr
	= obstack_strndup (&cpstate->expansion_obstack,
			   current.value.sval.ptr,
			   current.value.sval.length);

      token_fifo[0] = current;
      if (checkpoint > 1)
	token_fifo.erase (token_fifo.begin () + 1,
			  token_fifo.begin () + checkpoint);
    }

 do_pop:
  current = token_fifo[0];
  token_fifo.erase (token_fifo.begin ());
  yylval = current.value;
  return current.token;
}

int
c_parse (struct parser_state *par_state)
{
  /* Setting up the parser state.  */
  scoped_restore pstate_restore = make_scoped_restore (&pstate);
  gdb_assert (par_state != NULL);
  pstate = par_state;

  c_parse_state cstate;
  scoped_restore cstate_restore = make_scoped_restore (&cpstate, &cstate);

  macro_scope macro_scope;

  if (par_state->expression_context_block)
    macro_scope
      = sal_macro_scope (find_sal_for_pc (par_state->expression_context_pc, 0));
  else
    macro_scope = default_macro_scope ();
  if (!macro_scope.is_valid ())
    macro_scope = user_macro_scope ();

  scoped_restore restore_macro_scope
    = make_scoped_restore (&expression_macro_scope, &macro_scope);

  scoped_restore restore_yydebug = make_scoped_restore (&yydebug,
							par_state->debug);

  /* Initialize some state used by the lexer.  */
  last_was_structop = false;
  saw_name_at_eof = 0;
  paren_depth = 0;

  token_fifo.clear ();
  popping = 0;
  name_obstack.clear ();

  int result = yyparse ();
  if (!result)
    pstate->set_operation (pstate->pop ());
  return result;
}

#if defined(YYBISON) && YYBISON < 30800


/* This is called via the YYPRINT macro when parser debugging is
   enabled.  It prints a token's value.  */

static void
c_print_token (FILE *file, int type, c_exp_YYSTYPE value)
{
  switch (type)
    {
    case INT:
      parser_fprintf (file, "typed_val_int<%s, %s>",
		      value.typed_val_int.type->safe_name (),
		      pulongest (value.typed_val_int.val));
      break;

    case CHAR:
    case STRING:
      parser_fprintf (file, "tsval<type=%d, %.*s>", value.tsval.type,
		      value.tsval.length, value.tsval.ptr);
      break;

    case NSSTRING:
    case DOLLAR_VARIABLE:
    case SELECTOR:
      parser_fprintf (file, "sval<%s>", copy_name (value.sval).c_str ());
      break;

    case TYPENAME:
      parser_fprintf (file, "tsym<type=%s, name=%s>",
		      value.tsym.type->safe_name (),
		      copy_name (value.tsym.stoken).c_str ());
      break;

    case NAME:
    case UNKNOWN_CPP_NAME:
    case NAME_OR_INT:
    case BLOCKNAME:
      parser_fprintf (file, "ssym<name=%s, sym=%s, field_of_this=%d>",
		       copy_name (value.ssym.stoken).c_str (),
		       (value.ssym.sym.symbol == NULL
			? "(null)" : value.ssym.sym.symbol->print_name ()),
		       value.ssym.is_a_field_of_this);
      break;

    case FILENAME:
      parser_fprintf (file, "bval<%s>", host_address_to_string (value.bval));
      break;
    }
}

#endif

static void
yyerror (const char *msg)
{
  pstate->parse_error (msg);
}
