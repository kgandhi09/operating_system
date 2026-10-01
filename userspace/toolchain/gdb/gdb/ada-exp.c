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
#line 36 "ada-exp.y"


#include "gdbsupport/unordered_map.h"
#include "expression.h"
#include "value.h"
#include "parser-defs.h"
#include "language.h"
#include "ada-lang.h"
#include "frame.h"
#include "block.h"
#include "ada-exp.h"
#include "cli/cli-style.h"

#define parse_type(ps) builtin_type (ps->gdbarch ())

/* Remap normal yacc parser interface names (yyparse, yylex, yyerror,
   etc).  */
#define GDB_YY_REMAP_PREFIX ada_
#include "yy-remap.h"

struct name_info {
  struct symbol *sym;
  struct minimal_symbol *msym;
  const struct block *block;
  struct stoken stoken;
};

/* The state of the parser, used internally when we are parsing the
   expression.  */

static struct parser_state *pstate = NULL;

using namespace expr;

/* A convenience typedef.  */
typedef std::unique_ptr<ada_assign_operation> ada_assign_up;

/* Data that must be held for the duration of a parse.  */

struct ada_parse_state
{
  explicit ada_parse_state (const char *expr)
    : m_original_expr (expr)
  {
  }

  std::string find_completion_bounds ();

  const gdb_mpz *push_integer (gdb_mpz &&val)
  {
    auto &result = m_int_storage.emplace_back (new gdb_mpz (std::move (val)));
    return result.get ();
  }

  /* The components being constructed during this parse.  */
  std::vector<ada_component_up> components;

  /* The associations being constructed during this parse.  */
  std::vector<ada_association_up> associations;

  /* The stack of currently active assignment expressions.  This is used
     to implement '@', the target name symbol.  */
  std::vector<ada_assign_up> assignments;

  /* Track currently active iterated assignment names.  */
  gdb::unordered_string_map<std::vector<ada_index_var_operation *>>
       iterated_associations;

  auto_obstack temp_space;

  /* Depth of parentheses, used by the lexer.  */
  int paren_depth = 0;

  /* When completing, we'll return a special character at the end of the
     input, to signal the completion position to the lexer.  This is
     done because flex does not have a generally useful way to detect
     EOF in a pattern.  This variable records whether the special
     character has been emitted.  */
  bool returned_complete = false;

private:

  /* We don't have a good way to manage non-POD data in Yacc, so store
     values here.  The storage here is only valid for the duration of
     the parse.  */
  std::vector<std::unique_ptr<gdb_mpz>> m_int_storage;

  /* The original expression string.  */
  const char *m_original_expr;
};

/* The current Ada parser object.  */

static ada_parse_state *ada_parser;

int yyparse (void);

static int yylex (void);

static void yyerror (const char *);

static void write_int (struct parser_state *, LONGEST, struct type *);

static void write_object_renaming (struct parser_state *,
				   const struct block *, const char *, int,
				   const char *, int);

static struct type* write_var_or_type (struct parser_state *,
				       const struct block *, struct stoken);
static struct type *write_var_or_type_completion (struct parser_state *,
						  const struct block *,
						  struct stoken);

static void write_name_assoc (struct parser_state *, struct stoken);

static const struct block *block_lookup (const struct block *, const char *);

static void write_ambiguous_var (struct parser_state *,
				 const struct block *, const char *, int);

static struct type *type_for_char (struct parser_state *, ULONGEST);

static struct type *type_system_address (struct parser_state *);

/* Handle Ada type resolution for OP.  DEPROCEDURE_P and CONTEXT_TYPE
   are passed to the resolve method, if called.  */
static operation_up
resolve (operation_up &&op, bool deprocedure_p, struct type *context_type)
{
  operation_up result = std::move (op);
  ada_resolvable *res = dynamic_cast<ada_resolvable *> (result.get ());
  if (res != nullptr)
    return res->replace (std::move (result),
			 pstate->expout.get (),
			 deprocedure_p,
			 pstate->parse_completion,
			 pstate->block_tracker,
			 context_type);
  return result;
}

/* Like parser_state::pop, but handles Ada type resolution.
   DEPROCEDURE_P and CONTEXT_TYPE are passed to the resolve method, if
   called.  */
static operation_up
ada_pop (bool deprocedure_p = true, struct type *context_type = nullptr)
{
  /* Of course it's ok to call parser_state::pop here... */
  return resolve (pstate->pop (), deprocedure_p, context_type);
}

/* Like parser_state::wrap, but use ada_pop to pop the value.  */
template<typename T, typename... Args>
void
ada_wrap (Args... args)
{
  operation_up arg = ada_pop ();
  pstate->push_new<T> (std::move (arg), std::forward<Args> (args)...);
}

/* Create and push an address-of operation, as appropriate for Ada.
   If TYPE is not NULL, the resulting operation will be wrapped in a
   cast to TYPE.  */
static void
ada_addrof (struct type *type = nullptr)
{
  operation_up arg = ada_pop (false);
  operation_up addr = make_operation<unop_addr_operation> (std::move (arg));
  operation_up wrapped
    = make_operation<ada_wrapped_operation> (std::move (addr));
  if (type != nullptr)
    wrapped = make_operation<unop_cast_operation> (std::move (wrapped), type);
  pstate->push (std::move (wrapped));
}

/* Handle operator overloading.  Either returns a function all
   operation wrapping the arguments, or it returns null, leaving the
   caller to construct the appropriate operation.  If RHS is null, a
   unary operator is assumed.  */
static operation_up
maybe_overload (enum exp_opcode op, operation_up &lhs, operation_up &rhs)
{
  struct value *args[2];

  int nargs = 1;
  args[0] = lhs->evaluate (nullptr, pstate->expout.get (),
			   EVAL_AVOID_SIDE_EFFECTS);
  if (rhs == nullptr)
    args[1] = nullptr;
  else
    {
      args[1] = rhs->evaluate (nullptr, pstate->expout.get (),
			       EVAL_AVOID_SIDE_EFFECTS);
      ++nargs;
    }

  block_symbol fn = ada_find_operator_symbol (op, pstate->parse_completion,
					      nargs, args);
  if (fn.symbol == nullptr)
    return {};

  if (symbol_read_needs_frame (fn.symbol))
    pstate->block_tracker->update (fn.block, INNERMOST_BLOCK_FOR_SYMBOLS);
  operation_up callee = make_operation<ada_var_value_operation> (fn);

  std::vector<operation_up> argvec;
  argvec.push_back (std::move (lhs));
  if (rhs != nullptr)
    argvec.push_back (std::move (rhs));
  return make_operation<ada_funcall_operation> (std::move (callee),
						std::move (argvec));
}

/* Like parser_state::wrap, but use ada_pop to pop the value, and
   handle unary overloading.  */
template<typename T>
void
ada_wrap_overload (enum exp_opcode op)
{
  operation_up arg = ada_pop ();
  operation_up empty;

  operation_up call = maybe_overload (op, arg, empty);
  if (call == nullptr)
    call = make_operation<T> (std::move (arg));
  pstate->push (std::move (call));
}

/* A variant of parser_state::wrap2 that uses ada_pop to pop both
   operands, and then pushes a new Ada-wrapped operation of the
   template type T.  */
template<typename T>
void
ada_un_wrap2 (enum exp_opcode op)
{
  operation_up rhs = ada_pop ();
  operation_up lhs = ada_pop ();

  operation_up wrapped = maybe_overload (op, lhs, rhs);
  if (wrapped == nullptr)
    {
      wrapped = make_operation<T> (std::move (lhs), std::move (rhs));
      wrapped = make_operation<ada_wrapped_operation> (std::move (wrapped));
    }
  pstate->push (std::move (wrapped));
}

/* A variant of parser_state::wrap2 that uses ada_pop to pop both
   operands.  Unlike ada_un_wrap2, ada_wrapped_operation is not
   used.  */
template<typename T>
void
ada_wrap2 (enum exp_opcode op)
{
  operation_up rhs = ada_pop ();
  operation_up lhs = ada_pop ();
  operation_up call = maybe_overload (op, lhs, rhs);
  if (call == nullptr)
    call = make_operation<T> (std::move (lhs), std::move (rhs));
  pstate->push (std::move (call));
}

/* A variant of parser_state::wrap2 that uses ada_pop to pop both
   operands.  OP is also passed to the constructor of the new binary
   operation.  */
template<typename T>
void
ada_wrap_op (enum exp_opcode op)
{
  operation_up rhs = ada_pop ();
  operation_up lhs = ada_pop ();
  operation_up call = maybe_overload (op, lhs, rhs);
  if (call == nullptr)
    call = make_operation<T> (op, std::move (lhs), std::move (rhs));
  pstate->push (std::move (call));
}

/* Pop three operands using ada_pop, then construct a new ternary
   operation of type T and push it.  */
template<typename T>
void
ada_wrap3 ()
{
  operation_up rhs = ada_pop ();
  operation_up mid = ada_pop ();
  operation_up lhs = ada_pop ();
  pstate->push_new<T> (std::move (lhs), std::move (mid), std::move (rhs));
}

/* Pop NARGS operands, then a callee operand, and use these to
   construct and push a new Ada function call operation.  */
static void
ada_funcall (int nargs)
{
  /* We use the ordinary pop here, because we're going to do
     resolution in a separate step, in order to handle array
     indices.  */
  std::vector<operation_up> args = pstate->pop_vector (nargs);
  /* Call parser_state::pop here, because we don't want to
     function-convert the callee slot of a call we're already
     constructing.  */
  operation_up callee = pstate->pop ();

  ada_var_value_operation *vvo
    = dynamic_cast<ada_var_value_operation *> (callee.get ());
  int array_arity = 0;
  struct type *callee_t = nullptr;
  if (vvo == nullptr
      || vvo->get_symbol ()->domain () != UNDEF_DOMAIN)
    {
      struct value *callee_v = callee->evaluate (nullptr,
						 pstate->expout.get (),
						 EVAL_AVOID_SIDE_EFFECTS);
      callee_t = ada_check_typedef (callee_v->type ());
      array_arity = ada_array_arity (callee_t);
    }

  for (int i = 0; i < nargs; ++i)
    {
      struct type *subtype = nullptr;
      if (i < array_arity)
	subtype = ada_index_type (callee_t, i + 1, "array type");
      args[i] = resolve (std::move (args[i]), true, subtype);
    }

  std::unique_ptr<ada_funcall_operation> funcall
    (new ada_funcall_operation (std::move (callee), std::move (args)));
  funcall->resolve (pstate->expout.get (), true, pstate->parse_completion,
		    pstate->block_tracker, nullptr);
  pstate->push (std::move (funcall));
}

/* Create a new ada_component_up of the indicated type and arguments,
   and push it on the global 'components' vector.  */
template<typename T, typename... Arg>
void
push_component (Arg... args)
{
  ada_parser->components.emplace_back (new T (std::forward<Arg> (args)...));
}

/* Examine the final element of the 'components' vector, and return it
   as a pointer to an ada_choices_component.  The caller is
   responsible for ensuring that the final element is in fact an
   ada_choices_component.  */
static ada_choices_component *
choice_component ()
{
  ada_component *last = ada_parser->components.back ().get ();
  return gdb::checked_static_cast<ada_choices_component *> (last);
}

/* Pop the most recent component from the global stack, and return
   it.  */
static ada_component_up
pop_component ()
{
  ada_component_up result = std::move (ada_parser->components.back ());
  ada_parser->components.pop_back ();
  return result;
}

/* Pop the N most recent components from the global stack, and return
   them in a vector.  */
static std::vector<ada_component_up>
pop_components (int n)
{
  std::vector<ada_component_up> result (n);
  for (int i = 1; i <= n; ++i)
    result[n - i] = pop_component ();
  return result;
}

/* Create a new ada_association_up of the indicated type and
   arguments, and push it on the global 'associations' vector.  */
template<typename T, typename... Arg>
void
push_association (Arg... args)
{
  ada_parser->associations.emplace_back (new T (std::forward<Arg> (args)...));
}

/* Pop the most recent association from the global stack, and return
   it.  */
static ada_association_up
pop_association ()
{
  ada_association_up result = std::move (ada_parser->associations.back ());
  ada_parser->associations.pop_back ();
  return result;
}

/* Pop the N most recent associations from the global stack, and
   return them in a vector.  */
static std::vector<ada_association_up>
pop_associations (int n)
{
  std::vector<ada_association_up> result (n);
  for (int i = 1; i <= n; ++i)
    result[n - i] = pop_association ();
  return result;
}

/* Expression completer for attributes.  */
struct ada_tick_completer : public expr_completion_base
{
  explicit ada_tick_completer (std::string &&name)
    : m_name (std::move (name))
  {
  }

  bool complete (struct expression *exp,
		 completion_tracker &tracker) override;

private:

  std::string m_name;
};

/* Make a new ada_tick_completer and wrap it in a unique pointer.  */
static std::unique_ptr<expr_completion_base>
make_tick_completer (struct stoken tok)
{
  return (std::unique_ptr<expr_completion_base>
	  (new ada_tick_completer (std::string (tok.ptr, tok.length))));
}


#line 499 "ada-exp.c.tmp"

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
    NULL_PTR = 259,
    CHARLIT = 260,
    FLOAT = 261,
    TRUEKEYWORD = 262,
    FALSEKEYWORD = 263,
    WITH = 264,
    DELTA = 265,
    COLONCOLON = 266,
    STRING = 267,
    NAME = 268,
    DOT_ID = 269,
    TICK_COMPLETE = 270,
    DOT_COMPLETE = 271,
    NAME_COMPLETE = 272,
    DOLLAR_VARIABLE = 273,
    ASSIGN = 274,
    _AND_ = 275,
    OR = 276,
    XOR = 277,
    THEN = 278,
    ELSE = 279,
    NOTEQUAL = 280,
    LEQ = 281,
    GEQ = 282,
    IN = 283,
    DOTDOT = 284,
    UNARY = 285,
    MOD = 286,
    REM = 287,
    STARSTAR = 288,
    ABS = 289,
    NOT = 290,
    VAR = 291,
    ARROW = 292,
    TICK_ACCESS = 293,
    TICK_ADDRESS = 294,
    TICK_FIRST = 295,
    TICK_LAST = 296,
    TICK_LENGTH = 297,
    TICK_MAX = 298,
    TICK_MIN = 299,
    TICK_MODULUS = 300,
    TICK_POS = 301,
    TICK_RANGE = 302,
    TICK_SIZE = 303,
    TICK_OBJECT_SIZE = 304,
    TICK_TAG = 305,
    TICK_VAL = 306,
    TICK_ENUM_REP = 307,
    TICK_ENUM_VAL = 308,
    NEW = 309,
    OTHERS = 310,
    FOR = 311
  };
#endif
/* Tokens.  */
#define INT 258
#define NULL_PTR 259
#define CHARLIT 260
#define FLOAT 261
#define TRUEKEYWORD 262
#define FALSEKEYWORD 263
#define WITH 264
#define DELTA 265
#define COLONCOLON 266
#define STRING 267
#define NAME 268
#define DOT_ID 269
#define TICK_COMPLETE 270
#define DOT_COMPLETE 271
#define NAME_COMPLETE 272
#define DOLLAR_VARIABLE 273
#define ASSIGN 274
#define _AND_ 275
#define OR 276
#define XOR 277
#define THEN 278
#define ELSE 279
#define NOTEQUAL 280
#define LEQ 281
#define GEQ 282
#define IN 283
#define DOTDOT 284
#define UNARY 285
#define MOD 286
#define REM 287
#define STARSTAR 288
#define ABS 289
#define NOT 290
#define VAR 291
#define ARROW 292
#define TICK_ACCESS 293
#define TICK_ADDRESS 294
#define TICK_FIRST 295
#define TICK_LAST 296
#define TICK_LENGTH 297
#define TICK_MAX 298
#define TICK_MIN 299
#define TICK_MODULUS 300
#define TICK_POS 301
#define TICK_RANGE 302
#define TICK_SIZE 303
#define TICK_OBJECT_SIZE 304
#define TICK_TAG 305
#define TICK_VAL 306
#define TICK_ENUM_REP 307
#define TICK_ENUM_VAL 308
#define NEW 309
#define OTHERS 310
#define FOR 311

/* Value type.  */
#if ! defined ada_exp_YYSTYPE && ! defined ada_exp_YYSTYPE_IS_DECLARED
union ada_exp_YYSTYPE
{
#line 466 "ada-exp.y"

    LONGEST lval;
    struct {
      const gdb_mpz *val;
      struct type *type;
    } typed_val;
    struct {
      LONGEST val;
      struct type *type;
    } typed_char;
    struct {
      gdb_byte val[16];
      struct type *type;
    } typed_val_float;
    struct type *tval;
    struct stoken sval;
    const struct block *bval;
    struct internalvar *ivar;
  

#line 672 "ada-exp.c.tmp"

};
typedef union ada_exp_YYSTYPE ada_exp_YYSTYPE;
# define ada_exp_YYSTYPE_IS_TRIVIAL 1
# define ada_exp_YYSTYPE_IS_DECLARED 1
#endif


extern ada_exp_YYSTYPE yylval;

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
         || (defined ada_exp_YYSTYPE_IS_TRIVIAL && ada_exp_YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union ada_exp_yyalloc
{
  yytype_int16 yyss_alloc;
  ada_exp_YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union ada_exp_yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (ada_exp_YYSTYPE)) \
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
#define YYFINAL  60
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   852

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  77
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  27
/* YYNRULES -- Number of rules.  */
#define YYNRULES  127
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  251

#define YYUNDEFTOK  2
#define YYMAXUTOK   311

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
       2,     2,     2,     2,     2,     2,     2,     2,    36,    72,
      65,    71,    38,    34,    73,    35,    64,    39,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    70,
      27,    25,    28,     2,    33,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    66,     2,    76,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    74,    47,    75,     2,     2,     2,     2,
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
      26,    29,    30,    31,    32,    37,    40,    41,    42,    43,
      44,    45,    46,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    67,
      68,    69
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   534,   534,   538,   539,   542,   541,   565,   578,   592,
     594,   609,   621,   623,   631,   642,   648,   652,   659,   670,
     673,   677,   693,   700,   704,   707,   709,   711,   713,   717,
     730,   734,   738,   742,   746,   750,   754,   758,   762,   766,
     769,   773,   777,   781,   783,   790,   798,   801,   809,   820,
     824,   828,   832,   833,   834,   835,   836,   837,   841,   844,
     850,   853,   859,   862,   868,   870,   874,   877,   890,   892,
     894,   898,   904,   910,   916,   918,   920,   922,   924,   926,
     928,   934,   940,   946,   968,   970,   975,   984,   987,   991,
     998,  1004,  1015,  1023,  1030,  1035,  1042,  1046,  1048,  1054,
    1056,  1062,  1070,  1081,  1083,  1088,  1097,  1108,  1109,  1115,
    1120,  1126,  1135,  1136,  1137,  1141,  1148,  1154,  1153,  1188,
    1194,  1200,  1209,  1214,  1219,  1233,  1235,  1237
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "INT", "NULL_PTR", "CHARLIT", "FLOAT",
  "TRUEKEYWORD", "FALSEKEYWORD", "WITH", "DELTA", "COLONCOLON", "STRING",
  "NAME", "DOT_ID", "TICK_COMPLETE", "DOT_COMPLETE", "NAME_COMPLETE",
  "DOLLAR_VARIABLE", "ASSIGN", "_AND_", "OR", "XOR", "THEN", "ELSE", "'='",
  "NOTEQUAL", "'<'", "'>'", "LEQ", "GEQ", "IN", "DOTDOT", "'@'", "'+'",
  "'-'", "'&'", "UNARY", "'*'", "'/'", "MOD", "REM", "STARSTAR", "ABS",
  "NOT", "VAR", "ARROW", "'|'", "TICK_ACCESS", "TICK_ADDRESS",
  "TICK_FIRST", "TICK_LAST", "TICK_LENGTH", "TICK_MAX", "TICK_MIN",
  "TICK_MODULUS", "TICK_POS", "TICK_RANGE", "TICK_SIZE",
  "TICK_OBJECT_SIZE", "TICK_TAG", "TICK_VAL", "TICK_ENUM_REP",
  "TICK_ENUM_VAL", "'.'", "'('", "'['", "NEW", "OTHERS", "FOR", "';'",
  "')'", "'\\''", "','", "'{'", "'}'", "']'", "$accept", "start", "exp1",
  "$@1", "primary", "simple_exp", "arglist", "relation", "exp", "and_exp",
  "and_then_exp", "or_exp", "or_else_exp", "xor_exp", "tick_arglist",
  "type_prefix", "opt_type_prefix", "var_or_type", "block", "aggregate",
  "aggregate_component_list", "positional_list", "component_groups",
  "others", "component_group", "$@2", "component_associations", YY_NULLPTRPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,    61,   280,    60,    62,   281,
     282,   283,   284,    64,    43,    45,    38,   285,    42,    47,
     286,   287,   288,   289,   290,   291,   292,   124,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,    46,    40,    91,   309,   310,   311,
      59,    41,    39,    44,   123,   125,    93
};
# endif

#define YYPACT_NINF -110

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-110)))

#define YYTABLE_NINF -87

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     460,  -110,  -110,  -110,  -110,  -110,  -110,  -110,    11,  -110,
    -110,  -110,   460,   460,   586,   586,   460,   460,   291,    20,
      13,    16,   -18,    86,   745,   101,  -110,    35,    42,    46,
      47,    51,   -15,    57,   258,    37,  -110,  -110,  -110,   640,
     115,   115,    -2,    -2,   115,   115,    10,    34,    90,   -43,
     682,     6,    33,   291,  -110,  -110,    43,  -110,  -110,    50,
    -110,   460,  -110,  -110,  -110,  -110,  -110,  -110,    62,    62,
      62,  -110,  -110,  -110,   333,   460,   460,   460,   460,   460,
     460,   460,   460,   460,   460,   460,   460,   460,   460,   460,
     460,   460,    99,   376,   418,   460,   460,   108,   460,   109,
     460,  -110,    75,    76,    77,    82,    83,    84,   333,    85,
      18,  -110,   460,   502,   460,   127,  -110,   460,   460,   502,
     150,  -110,  -110,    88,  -110,   291,   586,  -110,   460,   159,
    -110,  -110,  -110,    23,   705,   -22,  -110,    89,   810,   810,
     810,   810,   810,   810,   581,   789,   190,   623,   115,   115,
     115,   122,   122,   122,   122,   122,   460,   460,  -110,   460,
    -110,  -110,  -110,   460,  -110,   460,  -110,   460,   460,   460,
     460,   460,   460,   725,    36,   460,  -110,  -110,  -110,   758,
    -110,  -110,  -110,   773,  -110,  -110,   291,  -110,  -110,    -2,
    -110,    95,   460,   460,  -110,   544,  -110,    62,   460,   620,
     800,   217,  -110,  -110,  -110,  -110,   103,   116,   117,   107,
     118,   123,   460,  -110,   125,   502,   460,   502,   124,   130,
    -110,  -110,     3,    71,  -110,  -110,   810,    62,   460,  -110,
    -110,  -110,   460,   460,  -110,   143,  -110,  -110,  -110,  -110,
    -110,  -110,   460,  -110,   810,   131,   132,  -110,  -110,  -110,
    -110
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      88,    89,    92,    90,    91,    94,    95,    93,    97,    98,
      16,    18,    88,    88,    88,    88,    88,    88,    88,     0,
       0,     0,     2,    19,    39,    52,     3,    53,    54,    55,
      56,    57,    87,     0,    15,     0,    17,   103,   101,    19,
      21,    20,   126,   125,    23,    22,    97,     0,     0,     0,
      39,     3,     0,    88,   107,   112,   113,   116,    96,     0,
       1,    88,     7,    70,     8,     5,    68,    69,    84,    84,
      84,    74,    75,    76,    88,    88,    88,    88,    88,    88,
      88,    88,    88,    88,    88,    88,    88,    88,    88,    88,
      88,    88,     0,    88,    88,    88,    88,     0,    88,     0,
      88,    83,     0,     0,     0,     0,     0,     0,    88,     0,
      99,   100,    88,    88,    88,     0,    14,    88,    88,    88,
       0,   110,   106,   108,   109,    88,    88,     4,    88,     0,
      71,    72,    73,    97,    39,     0,    25,     0,    40,    41,
      50,    51,    42,    49,    19,     0,    15,    35,    36,    38,
      37,    31,    32,    34,    33,    30,    88,    88,    58,    88,
      62,    66,    59,    88,    63,    88,    67,    88,    88,    88,
      88,    88,    88,    39,     0,    88,   104,   102,   119,     0,
     122,   115,   117,     0,   120,   123,    88,   111,   114,    29,
       6,     0,    88,    88,     9,    88,   127,    84,    88,    19,
       0,    15,    60,    64,    61,    65,     0,     0,     0,     0,
       0,     0,    88,    10,     0,    88,    88,    88,     0,     0,
      85,    26,     0,    97,    27,    44,    43,    84,    88,    80,
      81,    82,    88,    88,    79,     0,    11,   118,   121,   124,
     105,    12,    88,    47,    46,     0,     0,    13,    28,    78,
      77
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -110,  -110,   171,  -110,    17,     7,    96,   -87,     0,  -110,
    -110,  -110,  -110,  -110,   -68,  -110,  -110,   -17,  -110,  -110,
      12,  -110,   -48,  -110,  -110,  -110,  -109
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    21,    22,   128,    39,    24,   135,    25,   136,    27,
      28,    29,    30,    31,   130,    32,    33,    34,    35,    36,
      52,    53,    54,    55,    56,   215,    57
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      26,   131,   132,    59,   180,   124,   158,   160,   161,   162,
     185,   164,    62,   166,    64,   120,    60,    23,    51,    40,
      41,    37,    37,    44,    45,    50,     8,    61,   116,   176,
       9,    42,    43,    58,    37,    23,    83,    84,    85,    86,
     101,    87,    88,    89,    90,    91,   102,   103,   104,   194,
     110,   195,    61,   123,   111,    96,   112,   113,    38,    38,
      50,   127,    97,    74,    75,   146,   177,    98,    99,   192,
     202,    38,   203,   100,   241,   137,   204,   188,   205,   121,
     114,   134,    37,   138,   139,   140,   141,   142,   143,   145,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   144,
      62,    63,    64,   115,   122,    65,   237,   213,   239,   195,
     105,   106,   178,   107,   181,   173,   125,   242,   184,    38,
     179,    93,    94,    95,   183,   126,   179,   129,   190,   225,
     156,   163,   179,   165,    66,    67,    68,    69,    70,   201,
     167,   168,   169,   189,    71,    72,    73,   170,   171,   172,
     175,    74,    75,    87,    88,    89,    90,    91,   182,   243,
     186,   187,   191,   200,    91,   196,   220,   206,   207,   208,
     209,   210,   211,   199,   229,   214,    83,    84,    85,    86,
     232,    87,    88,    89,    90,    91,   218,   230,   231,    49,
     -45,   233,   221,    50,   234,   224,   236,   121,   219,   -45,
     222,   240,   249,   250,   174,   226,     0,     0,     0,     0,
     -45,   -45,   -45,     0,   247,     0,   238,   -48,     0,   235,
       0,     0,   179,     0,   179,     0,   -48,     0,     0,     0,
       0,     0,   245,   246,     0,   244,     0,   -48,   -48,   -48,
       0,     0,   248,   -86,   -86,   -86,   -86,     0,     0,     0,
       0,   -86,   -86,   -86,     0,   108,     0,     0,     0,     0,
     -45,   -45,   109,   -45,     0,     0,   -45,     0,     0,     0,
     -86,   -86,   -86,   -86,     0,     0,     0,     0,   -86,   -86,
     -86,     0,   108,     0,     0,     0,     0,   -48,   -48,   109,
     -48,     0,     0,   -48,     1,     2,     3,     4,     5,     6,
       0,     0,     0,     7,    46,     0,     0,     0,     9,    10,
       0,   -86,   -86,   -86,   -86,     0,     0,     0,     0,   -86,
     -86,   -86,     0,   108,    11,    12,    13,    14,     0,    15,
     109,     0,     0,     0,    16,    17,     1,     2,     3,     4,
       5,     6,     0,     0,     0,     7,   133,     0,     0,     0,
       9,    10,     0,     0,     0,     0,    18,     0,    19,    47,
      48,     0,     0,     0,     0,    20,    11,    12,    13,    14,
       0,    15,     0,     0,     0,     0,    16,    17,     0,     1,
       2,     3,     4,     5,     6,     0,     0,     0,     7,     8,
       0,     0,     0,     9,    10,     0,     0,     0,    18,   157,
      19,     0,     0,     0,   -24,     0,   -24,    20,     0,    11,
      12,    13,    14,     0,    15,     0,     0,     0,     0,    16,
      17,     1,     2,     3,     4,     5,     6,     0,     0,     0,
       7,     8,     0,     0,     0,     9,    10,     0,     0,     0,
       0,    18,   159,    19,     0,     0,     0,     0,     0,     0,
      20,    11,    12,    13,    14,     0,    15,     0,     0,     0,
       0,    16,    17,     1,     2,     3,     4,     5,     6,     0,
       0,     0,     7,     8,     0,     0,     0,     9,    10,     0,
       0,     0,     0,    18,     0,    19,     0,     0,     0,     0,
       0,     0,    20,    11,    12,    13,    14,     0,    15,     0,
       0,     0,     0,    16,    17,     1,     2,     3,     4,     5,
       6,     0,     0,     0,     7,    46,     0,     0,     0,     9,
      10,     0,     0,     0,     0,    18,     0,    19,     0,     0,
       0,     0,     0,     0,    20,    11,    12,    13,    14,     0,
      15,     0,     0,     0,     0,    16,    17,     1,     2,     3,
       4,     5,     6,     0,     0,     0,     7,   223,     0,     0,
       0,     9,    10,     0,     0,     0,     0,    18,     0,    19,
       0,     0,     0,     0,     0,     0,    20,    11,    12,    13,
      14,     0,    15,     0,     0,     0,     0,    16,    17,     1,
       2,     3,     4,     5,     6,    62,    63,    64,     7,     8,
       0,     0,     0,     9,    10,     0,     0,     0,     0,    18,
       0,    19,     0,     0,     0,     0,     0,     0,    20,    11,
       0,     0,    14,     0,    15,     0,     0,     0,     0,    66,
      67,    68,    69,    70,    62,    63,    64,     0,   197,    71,
      72,    73,     0,     0,     0,     0,    74,    75,     0,     0,
       0,    18,     0,    19,    62,    63,    64,    84,    85,    86,
      20,    87,    88,    89,    90,    91,     0,     0,    66,    67,
      68,    69,    70,     0,     0,     0,     0,   227,    71,    72,
      73,     0,     0,     0,     0,    74,    75,     0,    66,    67,
      68,    69,    70,     0,     0,     0,     0,     0,    71,    72,
      73,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,    81,    82,   117,    83,    84,    85,    86,     0,
      87,    88,    89,    90,    91,     0,    92,     0,   118,   119,
      76,    77,    78,    79,    80,    81,    82,   193,    83,    84,
      85,    86,     0,    87,    88,    89,    90,    91,     0,    92,
      76,    77,    78,    79,    80,    81,    82,   212,    83,    84,
      85,    86,     0,    87,    88,    89,    90,    91,     0,    92,
      76,    77,    78,    79,    80,    81,    82,     0,    83,    84,
      85,    86,     0,    87,    88,    89,    90,    91,     0,    92,
     117,    83,    84,    85,    86,     0,    87,    88,    89,    90,
      91,     0,     0,     0,   118,   119,    83,    84,    85,    86,
       0,    87,    88,    89,    90,    91,     0,     0,     0,   216,
     217,   198,    83,    84,    85,    86,     0,    87,    88,    89,
      90,    91,   228,    83,    84,    85,    86,     0,    87,    88,
      89,    90,    91,    83,    84,    85,    86,     0,    87,    88,
      89,    90,    91
};

static const yytype_int16 yycheck[] =
{
       0,    69,    70,    20,   113,    53,    93,    94,    95,    96,
     119,    98,    14,   100,    16,     9,     0,     0,    18,    12,
      13,    11,    11,    16,    17,    18,    13,    70,    71,    11,
      17,    14,    15,    13,    11,    18,    33,    34,    35,    36,
      55,    38,    39,    40,    41,    42,    61,    62,    63,    71,
      13,    73,    70,    53,    17,    20,    46,    47,    48,    48,
      53,    61,    20,    65,    66,    82,    48,    21,    21,    46,
     157,    48,   159,    22,    71,    75,   163,   125,   165,    73,
      46,    74,    11,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    82,
      14,    15,    16,    13,    71,    19,   215,    71,   217,    73,
      53,    54,   112,    56,   114,   108,    73,    46,   118,    48,
     113,    20,    21,    22,   117,    75,   119,    65,   128,   197,
      31,    23,   125,    24,    48,    49,    50,    51,    52,   156,
      65,    65,    65,   126,    58,    59,    60,    65,    65,    65,
      65,    65,    66,    38,    39,    40,    41,    42,    31,   227,
      10,    73,     3,   156,    42,    76,    71,   167,   168,   169,
     170,   171,   172,   156,    71,   175,    33,    34,    35,    36,
      73,    38,    39,    40,    41,    42,   186,    71,    71,    18,
       0,    73,   192,   186,    71,   195,    71,    73,   186,     9,
     193,    71,    71,    71,   108,   198,    -1,    -1,    -1,    -1,
      20,    21,    22,    -1,    71,    -1,   216,     0,    -1,   212,
      -1,    -1,   215,    -1,   217,    -1,     9,    -1,    -1,    -1,
      -1,    -1,   232,   233,    -1,   228,    -1,    20,    21,    22,
      -1,    -1,   242,    53,    54,    55,    56,    -1,    -1,    -1,
      -1,    61,    62,    63,    -1,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    -1,    -1,    76,    -1,    -1,    -1,
      53,    54,    55,    56,    -1,    -1,    -1,    -1,    61,    62,
      63,    -1,    65,    -1,    -1,    -1,    -1,    70,    71,    72,
      73,    -1,    -1,    76,     3,     4,     5,     6,     7,     8,
      -1,    -1,    -1,    12,    13,    -1,    -1,    -1,    17,    18,
      -1,    53,    54,    55,    56,    -1,    -1,    -1,    -1,    61,
      62,    63,    -1,    65,    33,    34,    35,    36,    -1,    38,
      72,    -1,    -1,    -1,    43,    44,     3,     4,     5,     6,
       7,     8,    -1,    -1,    -1,    12,    13,    -1,    -1,    -1,
      17,    18,    -1,    -1,    -1,    -1,    65,    -1,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    33,    34,    35,    36,
      -1,    38,    -1,    -1,    -1,    -1,    43,    44,    -1,     3,
       4,     5,     6,     7,     8,    -1,    -1,    -1,    12,    13,
      -1,    -1,    -1,    17,    18,    -1,    -1,    -1,    65,    23,
      67,    -1,    -1,    -1,    71,    -1,    73,    74,    -1,    33,
      34,    35,    36,    -1,    38,    -1,    -1,    -1,    -1,    43,
      44,     3,     4,     5,     6,     7,     8,    -1,    -1,    -1,
      12,    13,    -1,    -1,    -1,    17,    18,    -1,    -1,    -1,
      -1,    65,    24,    67,    -1,    -1,    -1,    -1,    -1,    -1,
      74,    33,    34,    35,    36,    -1,    38,    -1,    -1,    -1,
      -1,    43,    44,     3,     4,     5,     6,     7,     8,    -1,
      -1,    -1,    12,    13,    -1,    -1,    -1,    17,    18,    -1,
      -1,    -1,    -1,    65,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    -1,    74,    33,    34,    35,    36,    -1,    38,    -1,
      -1,    -1,    -1,    43,    44,     3,     4,     5,     6,     7,
       8,    -1,    -1,    -1,    12,    13,    -1,    -1,    -1,    17,
      18,    -1,    -1,    -1,    -1,    65,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    -1,    74,    33,    34,    35,    36,    -1,
      38,    -1,    -1,    -1,    -1,    43,    44,     3,     4,     5,
       6,     7,     8,    -1,    -1,    -1,    12,    13,    -1,    -1,
      -1,    17,    18,    -1,    -1,    -1,    -1,    65,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    74,    33,    34,    35,
      36,    -1,    38,    -1,    -1,    -1,    -1,    43,    44,     3,
       4,     5,     6,     7,     8,    14,    15,    16,    12,    13,
      -1,    -1,    -1,    17,    18,    -1,    -1,    -1,    -1,    65,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    74,    33,
      -1,    -1,    36,    -1,    38,    -1,    -1,    -1,    -1,    48,
      49,    50,    51,    52,    14,    15,    16,    -1,    57,    58,
      59,    60,    -1,    -1,    -1,    -1,    65,    66,    -1,    -1,
      -1,    65,    -1,    67,    14,    15,    16,    34,    35,    36,
      74,    38,    39,    40,    41,    42,    -1,    -1,    48,    49,
      50,    51,    52,    -1,    -1,    -1,    -1,    57,    58,    59,
      60,    -1,    -1,    -1,    -1,    65,    66,    -1,    48,    49,
      50,    51,    52,    -1,    -1,    -1,    -1,    -1,    58,    59,
      60,    -1,    -1,    -1,    -1,    65,    66,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    -1,
      38,    39,    40,    41,    42,    -1,    44,    -1,    46,    47,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    -1,    38,    39,    40,    41,    42,    -1,    44,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    -1,    38,    39,    40,    41,    42,    -1,    44,
      25,    26,    27,    28,    29,    30,    31,    -1,    33,    34,
      35,    36,    -1,    38,    39,    40,    41,    42,    -1,    44,
      32,    33,    34,    35,    36,    -1,    38,    39,    40,    41,
      42,    -1,    -1,    -1,    46,    47,    33,    34,    35,    36,
      -1,    38,    39,    40,    41,    42,    -1,    -1,    -1,    46,
      47,    32,    33,    34,    35,    36,    -1,    38,    39,    40,
      41,    42,    32,    33,    34,    35,    36,    -1,    38,    39,
      40,    41,    42,    33,    34,    35,    36,    -1,    38,    39,
      40,    41,    42
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,    12,    13,    17,
      18,    33,    34,    35,    36,    38,    43,    44,    65,    67,
      74,    78,    79,    81,    82,    84,    85,    86,    87,    88,
      89,    90,    92,    93,    94,    95,    96,    11,    48,    81,
      82,    82,    81,    81,    82,    82,    13,    68,    69,    79,
      82,    85,    97,    98,    99,   100,   101,   103,    13,    94,
       0,    70,    14,    15,    16,    19,    48,    49,    50,    51,
      52,    58,    59,    60,    65,    66,    25,    26,    27,    28,
      29,    30,    31,    33,    34,    35,    36,    38,    39,    40,
      41,    42,    44,    20,    21,    22,    20,    20,    21,    21,
      22,    55,    61,    62,    63,    53,    54,    56,    65,    72,
      13,    17,    46,    47,    46,    13,    71,    32,    46,    47,
       9,    73,    71,    85,    99,    73,    75,    85,    80,    65,
      91,    91,    91,    13,    82,    83,    85,    85,    82,    82,
      82,    82,    82,    82,    81,    82,    94,    82,    82,    82,
      82,    82,    82,    82,    82,    82,    31,    23,    84,    24,
      84,    84,    84,    23,    84,    24,    84,    65,    65,    65,
      65,    65,    65,    82,    83,    65,    11,    48,    85,    82,
     103,    85,    31,    82,    85,   103,    10,    73,    99,    81,
      85,     3,    46,    32,    71,    73,    76,    57,    32,    81,
      82,    94,    84,    84,    84,    84,    85,    85,    85,    85,
      85,    85,    32,    71,    85,   102,    46,    47,    85,    97,
      71,    85,    82,    13,    85,    91,    82,    57,    32,    71,
      71,    71,    73,    73,    71,    82,    71,   103,    85,   103,
      71,    71,    46,    91,    82,    85,    85,    71,    85,    71,
      71
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    77,    78,    79,    79,    80,    79,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    82,
      82,    82,    82,    82,    83,    83,    83,    83,    83,    81,
      82,    82,    82,    82,    82,    82,    82,    82,    82,    84,
      84,    84,    84,    84,    84,    84,    84,    84,    84,    84,
      84,    84,    85,    85,    85,    85,    85,    85,    86,    86,
      87,    87,    88,    88,    89,    89,    90,    90,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    91,    91,    92,    93,    93,    81,
      81,    81,    81,    81,    81,    81,    81,    94,    94,    94,
      94,    94,    94,    95,    95,    96,    96,    97,    97,    97,
      98,    98,    99,    99,    99,   100,   101,   102,   101,   103,
     103,   103,   103,   103,   103,    81,    81,    81
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     3,     0,     4,     2,     2,     4,
       4,     5,     6,     6,     3,     1,     1,     1,     1,     1,
       2,     2,     2,     2,     0,     1,     3,     3,     5,     4,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     1,
       3,     3,     3,     5,     5,     3,     6,     6,     4,     3,
       3,     3,     1,     1,     1,     1,     1,     1,     3,     3,
       4,     4,     3,     3,     4,     4,     3,     3,     2,     2,
       2,     3,     3,     3,     2,     2,     2,     7,     7,     5,
       5,     5,     5,     2,     0,     3,     1,     1,     0,     1,
       1,     1,     1,     1,     1,     1,     2,     1,     1,     2,
       2,     2,     3,     2,     3,     6,     3,     1,     2,     2,
       2,     3,     1,     1,     3,     3,     1,     0,     5,     3,
       3,     5,     3,     3,     5,     2,     2,     4
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
yy_symbol_value_print (FILE *yyo, int yytype, ada_exp_YYSTYPE const * const yyvaluep)
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
yy_symbol_print (FILE *yyo, int yytype, ada_exp_YYSTYPE const * const yyvaluep)
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
yy_reduce_print (yytype_int16 *yyssp, ada_exp_YYSTYPE *yyvsp, int yyrule)
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
yydestruct (const char *yymsg, int yytype, ada_exp_YYSTYPE *yyvaluep)
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
ada_exp_YYSTYPE yylval;
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
    ada_exp_YYSTYPE yyvsa[YYINITDEPTH];
    ada_exp_YYSTYPE *yyvs;
    ada_exp_YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  ada_exp_YYSTYPE yyval;

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
        ada_exp_YYSTYPE *yyvs1 = yyvs;
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
        union ada_exp_yyalloc *yyptr =
          (union ada_exp_yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
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
#line 540 "ada-exp.y"
    { ada_wrap2<comma_operation> (BINOP_COMMA); }
#line 2066 "ada-exp.c.tmp"
    break;

  case 5:
#line 542 "ada-exp.y"
    {
			  ada_parser->assignments.emplace_back
			    (new ada_assign_operation (ada_pop (), nullptr));
			}
#line 2075 "ada-exp.c.tmp"
    break;

  case 6:
#line 547 "ada-exp.y"
    {
			  ada_assign_up assign
			    = std::move (ada_parser->assignments.back ());
			  ada_parser->assignments.pop_back ();
			  value *lhs_val = (assign->eval_for_resolution
					    (pstate->expout.get ()));

			  operation_up rhs = pstate->pop ();
			  rhs = resolve (std::move (rhs), true,
					 lhs_val->type ());

			  assign->set_rhs (std::move (rhs));
			  pstate->push (std::move (assign));
			}
#line 2094 "ada-exp.c.tmp"
    break;

  case 7:
#line 566 "ada-exp.y"
    {
			  if (streq ((yyvsp[0].sval).ptr, "all"))
			    ada_wrap<ada_unop_ind_operation> ();
			  else
			    {
			      operation_up arg = ada_pop ();
			      pstate->push_new<ada_structop_operation>
				(std::move (arg), copy_name ((yyvsp[0].sval)));
			    }
			}
#line 2109 "ada-exp.c.tmp"
    break;

  case 8:
#line 579 "ada-exp.y"
    {
			  /* This is done even for ".all", because
			     that might be a prefix.  */
			  operation_up arg = ada_pop ();
			  ada_structop_operation *str_op
			    = (new ada_structop_operation
			       (std::move (arg), copy_name ((yyvsp[0].sval))));
			  str_op->set_prefix (ada_parser->find_completion_bounds ());
			  pstate->push (operation_up (str_op));
			  pstate->mark_struct_expression (str_op);
			}
#line 2125 "ada-exp.c.tmp"
    break;

  case 9:
#line 593 "ada-exp.y"
    { ada_funcall ((yyvsp[-1].lval)); }
#line 2131 "ada-exp.c.tmp"
    break;

  case 10:
#line 595 "ada-exp.y"
    {
			  if ((yyvsp[-3].tval) != NULL)
			    {
			      if ((yyvsp[-1].lval) != 1)
				error (_("Invalid conversion"));
			      operation_up arg = ada_pop ();
			      pstate->push_new<unop_cast_operation>
				(std::move (arg), (yyvsp[-3].tval));
			    }
			  else
			    ada_funcall ((yyvsp[-1].lval));
			}
#line 2148 "ada-exp.c.tmp"
    break;

  case 11:
#line 610 "ada-exp.y"
    {
			  if ((yyvsp[-4].tval) == NULL)
			    error (_("Type required for qualification"));
			  operation_up arg = ada_pop (true,
						      check_typedef ((yyvsp[-4].tval)));
			  pstate->push_new<ada_qual_operation>
			    (std::move (arg), (yyvsp[-4].tval));
			}
#line 2161 "ada-exp.c.tmp"
    break;

  case 12:
#line 622 "ada-exp.y"
    { ada_wrap3<ada_ternop_slice_operation> (); }
#line 2167 "ada-exp.c.tmp"
    break;

  case 13:
#line 624 "ada-exp.y"
    { if ((yyvsp[-5].tval) == NULL)
			    ada_wrap3<ada_ternop_slice_operation> ();
			  else
			    error (_("Cannot slice a type"));
			}
#line 2177 "ada-exp.c.tmp"
    break;

  case 14:
#line 631 "ada-exp.y"
    { }
#line 2183 "ada-exp.c.tmp"
    break;

  case 15:
#line 643 "ada-exp.y"
    { if ((yyvsp[0].tval) != NULL)
			    pstate->push_new<type_operation> ((yyvsp[0].tval));
			}
#line 2191 "ada-exp.c.tmp"
    break;

  case 16:
#line 649 "ada-exp.y"
    { pstate->push_dollar ((yyvsp[0].sval)); }
#line 2197 "ada-exp.c.tmp"
    break;

  case 17:
#line 653 "ada-exp.y"
    {
			  pstate->push_new<ada_aggregate_operation>
			    (pop_component ());
			}
#line 2206 "ada-exp.c.tmp"
    break;

  case 18:
#line 660 "ada-exp.y"
    {
			  if (ada_parser->assignments.empty ())
			    error (_("the target name symbol ('@') may only "
				     "appear in an assignment context"));
			  ada_assign_operation *current
			    = ada_parser->assignments.back ().get ();
			  pstate->push_new<ada_target_operation> (current);
			}
#line 2219 "ada-exp.c.tmp"
    break;

  case 20:
#line 674 "ada-exp.y"
    { ada_wrap_overload<ada_neg_operation> (UNOP_NEG); }
#line 2225 "ada-exp.c.tmp"
    break;

  case 21:
#line 678 "ada-exp.y"
    {
			  operation_up arg = ada_pop ();
			  operation_up empty;

			  /* If an overloaded operator was found, use
			     it.  Otherwise, unary + has no effect and
			     the argument can be pushed instead.  */
			  operation_up call = maybe_overload (UNOP_PLUS, arg,
							      empty);
			  if (call != nullptr)
			    arg = std::move (call);
			  pstate->push (std::move (arg));
			}
#line 2243 "ada-exp.c.tmp"
    break;

  case 22:
#line 694 "ada-exp.y"
    {
			  ada_wrap_overload<unary_logical_not_operation>
			    (UNOP_LOGICAL_NOT);
			}
#line 2252 "ada-exp.c.tmp"
    break;

  case 23:
#line 701 "ada-exp.y"
    { ada_wrap_overload<ada_abs_operation> (UNOP_ABS); }
#line 2258 "ada-exp.c.tmp"
    break;

  case 24:
#line 704 "ada-exp.y"
    { (yyval.lval) = 0; }
#line 2264 "ada-exp.c.tmp"
    break;

  case 25:
#line 708 "ada-exp.y"
    { (yyval.lval) = 1; }
#line 2270 "ada-exp.c.tmp"
    break;

  case 26:
#line 710 "ada-exp.y"
    { (yyval.lval) = 1; }
#line 2276 "ada-exp.c.tmp"
    break;

  case 27:
#line 712 "ada-exp.y"
    { (yyval.lval) = (yyvsp[-2].lval) + 1; }
#line 2282 "ada-exp.c.tmp"
    break;

  case 28:
#line 714 "ada-exp.y"
    { (yyval.lval) = (yyvsp[-4].lval) + 1; }
#line 2288 "ada-exp.c.tmp"
    break;

  case 29:
#line 719 "ada-exp.y"
    {
			  if ((yyvsp[-2].tval) == NULL)
			    error (_("Type required within braces in coercion"));
			  operation_up arg = ada_pop ();
			  pstate->push_new<unop_memval_operation>
			    (std::move (arg), (yyvsp[-2].tval));
			}
#line 2300 "ada-exp.c.tmp"
    break;

  case 30:
#line 731 "ada-exp.y"
    { ada_wrap2<ada_binop_exp_operation> (BINOP_EXP); }
#line 2306 "ada-exp.c.tmp"
    break;

  case 31:
#line 735 "ada-exp.y"
    { ada_wrap2<ada_binop_mul_operation> (BINOP_MUL); }
#line 2312 "ada-exp.c.tmp"
    break;

  case 32:
#line 739 "ada-exp.y"
    { ada_wrap2<ada_binop_div_operation> (BINOP_DIV); }
#line 2318 "ada-exp.c.tmp"
    break;

  case 33:
#line 743 "ada-exp.y"
    { ada_wrap2<ada_binop_rem_operation> (BINOP_REM); }
#line 2324 "ada-exp.c.tmp"
    break;

  case 34:
#line 747 "ada-exp.y"
    { ada_wrap2<ada_binop_mod_operation> (BINOP_MOD); }
#line 2330 "ada-exp.c.tmp"
    break;

  case 35:
#line 751 "ada-exp.y"
    { ada_wrap2<repeat_operation> (BINOP_REPEAT); }
#line 2336 "ada-exp.c.tmp"
    break;

  case 36:
#line 755 "ada-exp.y"
    { ada_wrap_op<ada_binop_addsub_operation> (BINOP_ADD); }
#line 2342 "ada-exp.c.tmp"
    break;

  case 37:
#line 759 "ada-exp.y"
    { ada_wrap2<ada_concat_operation> (BINOP_CONCAT); }
#line 2348 "ada-exp.c.tmp"
    break;

  case 38:
#line 763 "ada-exp.y"
    { ada_wrap_op<ada_binop_addsub_operation> (BINOP_SUB); }
#line 2354 "ada-exp.c.tmp"
    break;

  case 40:
#line 770 "ada-exp.y"
    { ada_wrap_op<ada_binop_equal_operation> (BINOP_EQUAL); }
#line 2360 "ada-exp.c.tmp"
    break;

  case 41:
#line 774 "ada-exp.y"
    { ada_wrap_op<ada_binop_equal_operation> (BINOP_NOTEQUAL); }
#line 2366 "ada-exp.c.tmp"
    break;

  case 42:
#line 778 "ada-exp.y"
    { ada_un_wrap2<leq_operation> (BINOP_LEQ); }
#line 2372 "ada-exp.c.tmp"
    break;

  case 43:
#line 782 "ada-exp.y"
    { ada_wrap3<ada_ternop_range_operation> (); }
#line 2378 "ada-exp.c.tmp"
    break;

  case 44:
#line 784 "ada-exp.y"
    {
			  operation_up rhs = ada_pop ();
			  operation_up lhs = ada_pop ();
			  pstate->push_new<ada_binop_in_bounds_operation>
			    (std::move (lhs), std::move (rhs), (yyvsp[0].lval));
			}
#line 2389 "ada-exp.c.tmp"
    break;

  case 45:
#line 791 "ada-exp.y"
    {
			  if ((yyvsp[0].tval) == NULL)
			    error (_("Right operand of 'in' must be type"));
			  operation_up arg = ada_pop ();
			  pstate->push_new<ada_unop_range_operation>
			    (std::move (arg), (yyvsp[0].tval));
			}
#line 2401 "ada-exp.c.tmp"
    break;

  case 46:
#line 799 "ada-exp.y"
    { ada_wrap3<ada_ternop_range_operation> ();
			  ada_wrap<unary_logical_not_operation> (); }
#line 2408 "ada-exp.c.tmp"
    break;

  case 47:
#line 802 "ada-exp.y"
    {
			  operation_up rhs = ada_pop ();
			  operation_up lhs = ada_pop ();
			  pstate->push_new<ada_binop_in_bounds_operation>
			    (std::move (lhs), std::move (rhs), (yyvsp[0].lval));
			  ada_wrap<unary_logical_not_operation> ();
			}
#line 2420 "ada-exp.c.tmp"
    break;

  case 48:
#line 810 "ada-exp.y"
    {
			  if ((yyvsp[0].tval) == NULL)
			    error (_("Right operand of 'in' must be type"));
			  operation_up arg = ada_pop ();
			  pstate->push_new<ada_unop_range_operation>
			    (std::move (arg), (yyvsp[0].tval));
			  ada_wrap<unary_logical_not_operation> ();
			}
#line 2433 "ada-exp.c.tmp"
    break;

  case 49:
#line 821 "ada-exp.y"
    { ada_un_wrap2<geq_operation> (BINOP_GEQ); }
#line 2439 "ada-exp.c.tmp"
    break;

  case 50:
#line 825 "ada-exp.y"
    { ada_un_wrap2<less_operation> (BINOP_LESS); }
#line 2445 "ada-exp.c.tmp"
    break;

  case 51:
#line 829 "ada-exp.y"
    { ada_un_wrap2<gtr_operation> (BINOP_GTR); }
#line 2451 "ada-exp.c.tmp"
    break;

  case 58:
#line 842 "ada-exp.y"
    { ada_wrap2<bitwise_and_operation>
			    (BINOP_BITWISE_AND); }
#line 2458 "ada-exp.c.tmp"
    break;

  case 59:
#line 845 "ada-exp.y"
    { ada_wrap2<bitwise_and_operation>
			    (BINOP_BITWISE_AND); }
#line 2465 "ada-exp.c.tmp"
    break;

  case 60:
#line 851 "ada-exp.y"
    { ada_wrap2<logical_and_operation>
			    (BINOP_LOGICAL_AND); }
#line 2472 "ada-exp.c.tmp"
    break;

  case 61:
#line 854 "ada-exp.y"
    { ada_wrap2<logical_and_operation>
			    (BINOP_LOGICAL_AND); }
#line 2479 "ada-exp.c.tmp"
    break;

  case 62:
#line 860 "ada-exp.y"
    { ada_wrap2<bitwise_ior_operation>
			    (BINOP_BITWISE_IOR); }
#line 2486 "ada-exp.c.tmp"
    break;

  case 63:
#line 863 "ada-exp.y"
    { ada_wrap2<bitwise_ior_operation>
			    (BINOP_BITWISE_IOR); }
#line 2493 "ada-exp.c.tmp"
    break;

  case 64:
#line 869 "ada-exp.y"
    { ada_wrap2<logical_or_operation> (BINOP_LOGICAL_OR); }
#line 2499 "ada-exp.c.tmp"
    break;

  case 65:
#line 871 "ada-exp.y"
    { ada_wrap2<logical_or_operation> (BINOP_LOGICAL_OR); }
#line 2505 "ada-exp.c.tmp"
    break;

  case 66:
#line 875 "ada-exp.y"
    { ada_wrap2<bitwise_xor_operation>
			    (BINOP_BITWISE_XOR); }
#line 2512 "ada-exp.c.tmp"
    break;

  case 67:
#line 878 "ada-exp.y"
    { ada_wrap2<bitwise_xor_operation>
			    (BINOP_BITWISE_XOR); }
#line 2519 "ada-exp.c.tmp"
    break;

  case 68:
#line 891 "ada-exp.y"
    { ada_addrof (); }
#line 2525 "ada-exp.c.tmp"
    break;

  case 69:
#line 893 "ada-exp.y"
    { ada_addrof (type_system_address (pstate)); }
#line 2531 "ada-exp.c.tmp"
    break;

  case 70:
#line 895 "ada-exp.y"
    {
			  pstate->mark_completion (make_tick_completer ((yyvsp[0].sval)));
			}
#line 2539 "ada-exp.c.tmp"
    break;

  case 71:
#line 899 "ada-exp.y"
    {
			  operation_up arg = ada_pop ();
			  pstate->push_new<ada_unop_atr_operation>
			    (std::move (arg), OP_ATR_FIRST, (yyvsp[0].lval));
			}
#line 2549 "ada-exp.c.tmp"
    break;

  case 72:
#line 905 "ada-exp.y"
    {
			  operation_up arg = ada_pop ();
			  pstate->push_new<ada_unop_atr_operation>
			    (std::move (arg), OP_ATR_LAST, (yyvsp[0].lval));
			}
#line 2559 "ada-exp.c.tmp"
    break;

  case 73:
#line 911 "ada-exp.y"
    {
			  operation_up arg = ada_pop ();
			  pstate->push_new<ada_unop_atr_operation>
			    (std::move (arg), OP_ATR_LENGTH, (yyvsp[0].lval));
			}
#line 2569 "ada-exp.c.tmp"
    break;

  case 74:
#line 917 "ada-exp.y"
    { ada_wrap<ada_atr_size_operation> (true); }
#line 2575 "ada-exp.c.tmp"
    break;

  case 75:
#line 919 "ada-exp.y"
    { ada_wrap<ada_atr_size_operation> (false); }
#line 2581 "ada-exp.c.tmp"
    break;

  case 76:
#line 921 "ada-exp.y"
    { ada_wrap<ada_atr_tag_operation> (); }
#line 2587 "ada-exp.c.tmp"
    break;

  case 77:
#line 923 "ada-exp.y"
    { ada_wrap2<ada_binop_min_operation> (BINOP_MIN); }
#line 2593 "ada-exp.c.tmp"
    break;

  case 78:
#line 925 "ada-exp.y"
    { ada_wrap2<ada_binop_max_operation> (BINOP_MAX); }
#line 2599 "ada-exp.c.tmp"
    break;

  case 79:
#line 927 "ada-exp.y"
    { ada_wrap<ada_pos_operation> (); }
#line 2605 "ada-exp.c.tmp"
    break;

  case 80:
#line 929 "ada-exp.y"
    {
			  operation_up arg = ada_pop ();
			  pstate->push_new<ada_atr_val_operation>
			    ((yyvsp[-4].tval), std::move (arg));
			}
#line 2615 "ada-exp.c.tmp"
    break;

  case 81:
#line 935 "ada-exp.y"
    {
			  operation_up arg = ada_pop (true, (yyvsp[-4].tval));
			  pstate->push_new<ada_atr_enum_rep_operation>
			    ((yyvsp[-4].tval), std::move (arg));
			}
#line 2625 "ada-exp.c.tmp"
    break;

  case 82:
#line 941 "ada-exp.y"
    {
			  operation_up arg = ada_pop (true, (yyvsp[-4].tval));
			  pstate->push_new<ada_atr_enum_val_operation>
			    ((yyvsp[-4].tval), std::move (arg));
			}
#line 2635 "ada-exp.c.tmp"
    break;

  case 83:
#line 947 "ada-exp.y"
    {
			  struct type *type_arg = check_typedef ((yyvsp[-1].tval));
			  if (!ada_is_modular_type (type_arg))
			    error (_("'modulus must be applied to modular type"));
			  std::optional<ULONGEST> bound
			    = ada_modular_bound (type_arg);
			  if (!bound.has_value ())
			    error (_("'modulus applied to type with non-constant bound"));
			  gdb_mpz modulus = gdb_mpz (*bound) + 1;
			  /* Pick something that's almost certainly
			     large enough.  */
			  struct type *r_type
			    = language_lookup_primitive_type (pstate->language (),
							      pstate->gdbarch (),
							      "unsigned_long_long_long_integer");
			  pstate->push_new<long_const_operation> (r_type,
								  modulus);
			  ada_wrap<ada_wrapped_operation> ();
			}
#line 2659 "ada-exp.c.tmp"
    break;

  case 84:
#line 969 "ada-exp.y"
    { (yyval.lval) = 1; }
#line 2665 "ada-exp.c.tmp"
    break;

  case 85:
#line 971 "ada-exp.y"
    { (yyval.lval) = (yyvsp[-1].typed_val).val->as_integer<LONGEST> (); }
#line 2671 "ada-exp.c.tmp"
    break;

  case 86:
#line 976 "ada-exp.y"
    {
			  if ((yyvsp[0].tval) == NULL)
			    error (_("Prefix must be type"));
			  (yyval.tval) = (yyvsp[0].tval);
			}
#line 2681 "ada-exp.c.tmp"
    break;

  case 87:
#line 985 "ada-exp.y"
    { (yyval.tval) = (yyvsp[0].tval); }
#line 2687 "ada-exp.c.tmp"
    break;

  case 88:
#line 987 "ada-exp.y"
    { (yyval.tval) = parse_type (pstate)->builtin_void; }
#line 2693 "ada-exp.c.tmp"
    break;

  case 89:
#line 992 "ada-exp.y"
    {
			  pstate->push_new<long_const_operation> ((yyvsp[0].typed_val).type, *(yyvsp[0].typed_val).val);
			  ada_wrap<ada_wrapped_operation> ();
			}
#line 2702 "ada-exp.c.tmp"
    break;

  case 90:
#line 999 "ada-exp.y"
    {
			  pstate->push_new<ada_char_operation> ((yyvsp[0].typed_char).type, (yyvsp[0].typed_char).val);
			}
#line 2710 "ada-exp.c.tmp"
    break;

  case 91:
#line 1005 "ada-exp.y"
    {
			  float_data data;
			  std::copy (std::begin ((yyvsp[0].typed_val_float).val), std::end ((yyvsp[0].typed_val_float).val),
				     std::begin (data));
			  pstate->push_new<float_const_operation>
			    ((yyvsp[0].typed_val_float).type, data);
			  ada_wrap<ada_wrapped_operation> ();
			}
#line 2723 "ada-exp.c.tmp"
    break;

  case 92:
#line 1016 "ada-exp.y"
    {
			  struct type *null_ptr_type
			    = lookup_pointer_type (parse_type (pstate)->builtin_int0);
			  write_int (pstate, 0, null_ptr_type);
			}
#line 2733 "ada-exp.c.tmp"
    break;

  case 93:
#line 1024 "ada-exp.y"
    {
			  pstate->push_new<ada_string_operation>
			    (copy_name ((yyvsp[0].sval)));
			}
#line 2742 "ada-exp.c.tmp"
    break;

  case 94:
#line 1031 "ada-exp.y"
    {
			  write_int (pstate, 1,
				     parse_type (pstate)->builtin_bool);
			}
#line 2751 "ada-exp.c.tmp"
    break;

  case 95:
#line 1036 "ada-exp.y"
    {
			  write_int (pstate, 0,
				     parse_type (pstate)->builtin_bool);
			}
#line 2760 "ada-exp.c.tmp"
    break;

  case 96:
#line 1043 "ada-exp.y"
    { error (_("NEW not implemented.")); }
#line 2766 "ada-exp.c.tmp"
    break;

  case 97:
#line 1047 "ada-exp.y"
    { (yyval.tval) = write_var_or_type (pstate, NULL, (yyvsp[0].sval)); }
#line 2772 "ada-exp.c.tmp"
    break;

  case 98:
#line 1049 "ada-exp.y"
    {
				  (yyval.tval) = write_var_or_type_completion (pstate,
								     NULL,
								     (yyvsp[0].sval));
				}
#line 2782 "ada-exp.c.tmp"
    break;

  case 99:
#line 1055 "ada-exp.y"
    { (yyval.tval) = write_var_or_type (pstate, (yyvsp[-1].bval), (yyvsp[0].sval)); }
#line 2788 "ada-exp.c.tmp"
    break;

  case 100:
#line 1057 "ada-exp.y"
    {
				  (yyval.tval) = write_var_or_type_completion (pstate,
								     (yyvsp[-1].bval),
								     (yyvsp[0].sval));
				}
#line 2798 "ada-exp.c.tmp"
    break;

  case 101:
#line 1063 "ada-exp.y"
    {
			  (yyval.tval) = write_var_or_type (pstate, NULL, (yyvsp[-1].sval));
			  if ((yyval.tval) == NULL)
			    ada_addrof ();
			  else
			    (yyval.tval) = lookup_pointer_type ((yyval.tval));
			}
#line 2810 "ada-exp.c.tmp"
    break;

  case 102:
#line 1071 "ada-exp.y"
    {
			  (yyval.tval) = write_var_or_type (pstate, (yyvsp[-2].bval), (yyvsp[-1].sval));
			  if ((yyval.tval) == NULL)
			    ada_addrof ();
			  else
			    (yyval.tval) = lookup_pointer_type ((yyval.tval));
			}
#line 2822 "ada-exp.c.tmp"
    break;

  case 103:
#line 1082 "ada-exp.y"
    { (yyval.bval) = block_lookup (NULL, (yyvsp[-1].sval).ptr); }
#line 2828 "ada-exp.c.tmp"
    break;

  case 104:
#line 1084 "ada-exp.y"
    { (yyval.bval) = block_lookup ((yyvsp[-2].bval), (yyvsp[-1].sval).ptr); }
#line 2834 "ada-exp.c.tmp"
    break;

  case 105:
#line 1089 "ada-exp.y"
    {
			  std::vector<ada_component_up> components
			    = pop_components ((yyvsp[-1].lval));
			  operation_up base = ada_pop ();

			  push_component<ada_aggregate_component>
			    (std::move (base), std::move (components));
			}
#line 2847 "ada-exp.c.tmp"
    break;

  case 106:
#line 1098 "ada-exp.y"
    {
			  std::vector<ada_component_up> components
			    = pop_components ((yyvsp[-1].lval));

			  push_component<ada_aggregate_component>
			    (std::move (components));
			}
#line 2859 "ada-exp.c.tmp"
    break;

  case 107:
#line 1108 "ada-exp.y"
    { (yyval.lval) = (yyvsp[0].lval); }
#line 2865 "ada-exp.c.tmp"
    break;

  case 108:
#line 1110 "ada-exp.y"
    {
			  push_component<ada_positional_component>
			    ((yyvsp[-1].lval), ada_pop ());
			  (yyval.lval) = (yyvsp[-1].lval) + 1;
			}
#line 2875 "ada-exp.c.tmp"
    break;

  case 109:
#line 1116 "ada-exp.y"
    { (yyval.lval) = (yyvsp[-1].lval) + (yyvsp[0].lval); }
#line 2881 "ada-exp.c.tmp"
    break;

  case 110:
#line 1121 "ada-exp.y"
    {
			  push_component<ada_positional_component>
			    (0, ada_pop ());
			  (yyval.lval) = 1;
			}
#line 2891 "ada-exp.c.tmp"
    break;

  case 111:
#line 1127 "ada-exp.y"
    {
			  push_component<ada_positional_component>
			    ((yyvsp[-2].lval), ada_pop ());
			  (yyval.lval) = (yyvsp[-2].lval) + 1;
			}
#line 2901 "ada-exp.c.tmp"
    break;

  case 112:
#line 1135 "ada-exp.y"
    { (yyval.lval) = 1; }
#line 2907 "ada-exp.c.tmp"
    break;

  case 113:
#line 1136 "ada-exp.y"
    { (yyval.lval) = 1; }
#line 2913 "ada-exp.c.tmp"
    break;

  case 114:
#line 1138 "ada-exp.y"
    { (yyval.lval) = (yyvsp[0].lval) + 1; }
#line 2919 "ada-exp.c.tmp"
    break;

  case 115:
#line 1142 "ada-exp.y"
    {
			  push_component<ada_others_component> (ada_pop ());
			}
#line 2927 "ada-exp.c.tmp"
    break;

  case 116:
#line 1149 "ada-exp.y"
    {
			  ada_choices_component *choices = choice_component ();
			  choices->set_associations (pop_associations ((yyvsp[0].lval)));
			}
#line 2936 "ada-exp.c.tmp"
    break;

  case 117:
#line 1154 "ada-exp.y"
    {
			  std::string name = copy_name ((yyvsp[-1].sval));

			  auto iter = ada_parser->iterated_associations.find (name);
			  if (iter != ada_parser->iterated_associations.end ())
			    error (_("Nested use of index parameter '%s'"),
				   name.c_str ());

			  ada_parser->iterated_associations[name] = {};
			}
#line 2951 "ada-exp.c.tmp"
    break;

  case 118:
#line 1165 "ada-exp.y"
    {
			  std::string name = copy_name ((yyvsp[-3].sval));

			  ada_choices_component *choices = choice_component ();
			  choices->set_associations (pop_associations ((yyvsp[0].lval)));

			  auto iter = ada_parser->iterated_associations.find (name);
			  gdb_assert (iter != ada_parser->iterated_associations.end ());
			  for (ada_index_var_operation *var : iter->second)
			    var->set_choices (choices);

			  ada_parser->iterated_associations.erase (name);

			  choices->set_name (std::move (name));
			}
#line 2971 "ada-exp.c.tmp"
    break;

  case 119:
#line 1189 "ada-exp.y"
    {
			  push_component<ada_choices_component> (ada_pop ());
			  write_name_assoc (pstate, (yyvsp[-2].sval));
			  (yyval.lval) = 1;
			}
#line 2981 "ada-exp.c.tmp"
    break;

  case 120:
#line 1195 "ada-exp.y"
    {
			  push_component<ada_choices_component> (ada_pop ());
			  push_association<ada_name_association> (ada_pop ());
			  (yyval.lval) = 1;
			}
#line 2991 "ada-exp.c.tmp"
    break;

  case 121:
#line 1201 "ada-exp.y"
    {
			  push_component<ada_choices_component> (ada_pop ());
			  operation_up rhs = ada_pop ();
			  operation_up lhs = ada_pop ();
			  push_association<ada_discrete_range_association>
			    (std::move (lhs), std::move (rhs));
			  (yyval.lval) = 1;
			}
#line 3004 "ada-exp.c.tmp"
    break;

  case 122:
#line 1210 "ada-exp.y"
    {
			  write_name_assoc (pstate, (yyvsp[-2].sval));
			  (yyval.lval) = (yyvsp[0].lval) + 1;
			}
#line 3013 "ada-exp.c.tmp"
    break;

  case 123:
#line 1215 "ada-exp.y"
    {
			  push_association<ada_name_association> (ada_pop ());
			  (yyval.lval) = (yyvsp[0].lval) + 1;
			}
#line 3022 "ada-exp.c.tmp"
    break;

  case 124:
#line 1221 "ada-exp.y"
    {
			  operation_up rhs = ada_pop ();
			  operation_up lhs = ada_pop ();
			  push_association<ada_discrete_range_association>
			    (std::move (lhs), std::move (rhs));
			  (yyval.lval) = (yyvsp[0].lval) + 1;
			}
#line 3034 "ada-exp.c.tmp"
    break;

  case 125:
#line 1234 "ada-exp.y"
    { ada_wrap<ada_unop_ind_operation> (); }
#line 3040 "ada-exp.c.tmp"
    break;

  case 126:
#line 1236 "ada-exp.y"
    { ada_addrof (); }
#line 3046 "ada-exp.c.tmp"
    break;

  case 127:
#line 1238 "ada-exp.y"
    {
			  ada_wrap2<subscript_operation> (BINOP_SUBSCRIPT);
			  ada_wrap<ada_wrapped_operation> ();
			}
#line 3055 "ada-exp.c.tmp"
    break;


#line 3059 "ada-exp.c.tmp"

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
#line 1244 "ada-exp.y"


/* yylex defined in ada-lex.c: Reads one token, getting characters */
/* through lexptr.  */

/* Remap normal flex interface names (yylex) as well as gratuitously */
/* global symbol names, so we can have multiple flex-generated parsers */
/* in gdb.  */

/* (See note above on previous definitions for YACC.) */

#define yy_create_buffer ada_yy_create_buffer
#define yy_delete_buffer ada_yy_delete_buffer
#define yy_init_buffer ada_yy_init_buffer
#define yy_load_buffer_state ada_yy_load_buffer_state
#define yy_switch_to_buffer ada_yy_switch_to_buffer
#define yyrestart ada_yyrestart
#define yytext ada_yytext

/* The following kludge was found necessary to prevent conflicts between */
/* defs.h and non-standard stdlib.h files.  */
#define qsort __qsort__dummy
#include "ada-lex.c"

int
ada_parse (struct parser_state *par_state)
{
  /* Setting up the parser state.  */
  scoped_restore pstate_restore = make_scoped_restore (&pstate, par_state);
  gdb_assert (par_state != NULL);

  ada_parse_state parser (par_state->lexptr);
  scoped_restore parser_restore = make_scoped_restore (&ada_parser, &parser);

  scoped_restore restore_yydebug = make_scoped_restore (&yydebug,
							par_state->debug);

  lexer_init (yyin);		/* (Re-)initialize lexer.  */

  int result = yyparse ();
  if (!result)
    {
      struct type *context_type = nullptr;
      if (par_state->void_context_p)
	context_type = parse_type (par_state)->builtin_void;
      pstate->set_operation (ada_pop (true, context_type));
    }
  return result;
}

static void
yyerror (const char *msg)
{
  pstate->parse_error (msg);
}

/* Emit expression to access an instance of SYM, in block BLOCK (if
   non-NULL).  */

static void
write_var_from_sym (struct parser_state *par_state, block_symbol sym)
{
  if (symbol_read_needs_frame (sym.symbol))
    par_state->block_tracker->update (sym.block, INNERMOST_BLOCK_FOR_SYMBOLS);

  par_state->push_new<ada_var_value_operation> (sym);
}

/* Write integer or boolean constant ARG of type TYPE.  */

static void
write_int (struct parser_state *par_state, LONGEST arg, struct type *type)
{
  pstate->push_new<long_const_operation> (type, arg);
  ada_wrap<ada_wrapped_operation> ();
}

/* Emit expression corresponding to the renamed object named
   designated by RENAMED_ENTITY[0 .. RENAMED_ENTITY_LEN-1] in the
   context of ORIG_LEFT_CONTEXT, to which is applied the operations
   encoded by RENAMING_EXPR.  MAX_DEPTH is the maximum number of
   cascaded renamings to allow.  If ORIG_LEFT_CONTEXT is null, it
   defaults to the currently selected block. ORIG_SYMBOL is the
   symbol that originally encoded the renaming.  It is needed only
   because its prefix also qualifies any index variables used to index
   or slice an array.  It should not be necessary once we go to the
   new encoding entirely (FIXME pnh 7/20/2007).  */

static void
write_object_renaming (struct parser_state *par_state,
		       const struct block *orig_left_context,
		       const char *renamed_entity, int renamed_entity_len,
		       const char *renaming_expr, int max_depth)
{
  char *name;
  enum { SIMPLE_INDEX, LOWER_BOUND, UPPER_BOUND } slice_state;

  if (max_depth <= 0)
    error (_("Could not find renamed symbol"));

  if (orig_left_context == NULL)
    orig_left_context = get_selected_block ();

  name = obstack_strndup (&ada_parser->temp_space, renamed_entity,
			  renamed_entity_len);
  block_symbol sym_info = ada_lookup_encoded_symbol (name, orig_left_context,
						     SEARCH_VFT);
  if (sym_info.symbol == NULL)
    error (_("Could not find renamed variable: %ps"),
	   styled_string (variable_name_style.style (),
			  ada_decode (name).c_str ()));
  else if (sym_info.symbol->loc_class () == LOC_TYPEDEF)
    /* We have a renaming of an old-style renaming symbol.  Don't
       trust the block information.  */
    sym_info.block = orig_left_context;

  {
    const char *inner_renamed_entity;
    int inner_renamed_entity_len;
    const char *inner_renaming_expr;

    switch (ada_parse_renaming (sym_info.symbol, &inner_renamed_entity,
				&inner_renamed_entity_len,
				&inner_renaming_expr))
      {
      case ADA_NOT_RENAMING:
	write_var_from_sym (par_state, sym_info);
	break;
      case ADA_OBJECT_RENAMING:
	write_object_renaming (par_state, sym_info.block,
			       inner_renamed_entity, inner_renamed_entity_len,
			       inner_renaming_expr, max_depth - 1);
	break;
      default:
	goto BadEncoding;
      }
  }

  slice_state = SIMPLE_INDEX;
  while (*renaming_expr == 'X')
    {
      renaming_expr += 1;

      switch (*renaming_expr) {
      case 'A':
	renaming_expr += 1;
	ada_wrap<ada_unop_ind_operation> ();
	break;
      case 'L':
	slice_state = LOWER_BOUND;
	[[fallthrough]];
      case 'S':
	renaming_expr += 1;
	if (c_isdigit (*renaming_expr))
	  {
	    char *next;
	    long val = strtol (renaming_expr, &next, 10);
	    if (next == renaming_expr)
	      goto BadEncoding;
	    renaming_expr = next;
	    write_int (par_state, val, parse_type (par_state)->builtin_int);
	  }
	else
	  {
	    const char *end;
	    char *index_name;

	    end = strchr (renaming_expr, 'X');
	    if (end == NULL)
	      end = renaming_expr + strlen (renaming_expr);

	    index_name = obstack_strndup (&ada_parser->temp_space,
					  renaming_expr,
					  end - renaming_expr);
	    renaming_expr = end;

	    block_symbol index_sym_info
	      = ada_lookup_encoded_symbol (index_name, orig_left_context,
					   SEARCH_VFT);
	    if (index_sym_info.symbol == NULL)
	      error (_("Could not find %s"), index_name);
	    else if (index_sym_info.symbol->loc_class () == LOC_TYPEDEF)
	      /* Index is an old-style renaming symbol.  */
	      index_sym_info.block = orig_left_context;
	    write_var_from_sym (par_state, index_sym_info);
	  }
	if (slice_state == SIMPLE_INDEX)
	  ada_funcall (1);
	else if (slice_state == LOWER_BOUND)
	  slice_state = UPPER_BOUND;
	else if (slice_state == UPPER_BOUND)
	  {
	    ada_wrap3<ada_ternop_slice_operation> ();
	    slice_state = SIMPLE_INDEX;
	  }
	break;

      case 'R':
	{
	  const char *end;

	  renaming_expr += 1;

	  if (slice_state != SIMPLE_INDEX)
	    goto BadEncoding;
	  end = strchr (renaming_expr, 'X');
	  if (end == NULL)
	    end = renaming_expr + strlen (renaming_expr);

	  operation_up arg = ada_pop ();
	  pstate->push_new<ada_structop_operation>
	    (std::move (arg), std::string (renaming_expr,
					   end - renaming_expr));
	  renaming_expr = end;
	  break;
	}

      default:
	goto BadEncoding;
      }
    }
  if (slice_state == SIMPLE_INDEX)
    return;

 BadEncoding:
  error (_("Internal error in encoding of renaming declaration"));
}

static const struct block*
block_lookup (const struct block *context, const char *raw_name)
{
  const char *name;
  struct symtab *symtab;
  const struct block *result = NULL;

  std::string name_storage;
  if (raw_name[0] == '\'')
    {
      raw_name += 1;
      name = raw_name;
    }
  else
    {
      name_storage = ada_encode (raw_name);
      name = name_storage.c_str ();
    }

  std::vector<struct block_symbol> syms
    = ada_lookup_symbol_list (name, context, SEARCH_FUNCTION_DOMAIN);

  if (context == NULL
      && (syms.empty () || syms[0].symbol->loc_class () != LOC_BLOCK))
    symtab = lookup_symtab (current_program_space, name);
  else
    symtab = NULL;

  if (symtab != NULL)
    result = symtab->compunit ().blockvector ()->static_block ();
  else if (syms.empty () || syms[0].symbol->loc_class () != LOC_BLOCK)
    {
      if (context == NULL)
	error (_("No file or function \"%s\"."), raw_name);
      else
	error (_("No function \"%s\" in specified context."), raw_name);
    }
  else
    {
      if (syms.size () > 1)
	warning (_("Function name \"%s\" ambiguous here"), raw_name);
      result = syms[0].symbol->value_block ();
    }

  return result;
}

static struct symbol*
select_possible_type_sym (const std::vector<struct block_symbol> &syms)
{
  int i;
  int preferred_index;
  struct type *preferred_type;

  preferred_index = -1; preferred_type = NULL;
  for (i = 0; i < syms.size (); i += 1)
    switch (syms[i].symbol->loc_class ())
      {
      case LOC_TYPEDEF:
	if (ada_prefer_type (syms[i].symbol->type (), preferred_type))
	  {
	    preferred_index = i;
	    preferred_type = syms[i].symbol->type ();
	  }
	break;
      case LOC_REGISTER:
      case LOC_ARG:
      case LOC_REF_ARG:
      case LOC_REGPARM_ADDR:
      case LOC_LOCAL:
      case LOC_COMPUTED:
	return NULL;
      default:
	break;
      }
  if (preferred_type == NULL)
    return NULL;
  return syms[preferred_index].symbol;
}

static struct type*
find_primitive_type (struct parser_state *par_state, const char *name)
{
  struct type *type;
  type = language_lookup_primitive_type (par_state->language (),
					 par_state->gdbarch (),
					 name);
  if (type == NULL && streq ("system__address", name))
    type = type_system_address (par_state);

  if (type != NULL)
    {
      /* Check to see if we have a regular definition of this
	 type that just didn't happen to have been read yet.  */
      struct symbol *sym;
      char *expanded_name =
	(char *) alloca (strlen (name) + sizeof ("standard__"));
      strcpy (expanded_name, "standard__");
      strcat (expanded_name, name);
      sym = ada_lookup_symbol (expanded_name, NULL, SEARCH_TYPE_DOMAIN).symbol;
      if (sym != NULL && sym->loc_class () == LOC_TYPEDEF)
	type = sym->type ();
    }

  return type;
}

static int
chop_selector (const char *name, int end)
{
  int i;
  for (i = end - 1; i > 0; i -= 1)
    if (name[i] == '.' || (name[i] == '_' && name[i+1] == '_'))
      return i;
  return -1;
}

/* If NAME is a string beginning with a separator (either '__', or
   '.'), chop this separator and return the result; else, return
   NAME.  */

static const char *
chop_separator (const char *name)
{
  if (*name == '.')
   return name + 1;

  if (name[0] == '_' && name[1] == '_')
    return name + 2;

  return name;
}

/* Given that SELS is a string of the form (<sep><identifier>)*, where
   <sep> is '__' or '.', write the indicated sequence of
   STRUCTOP_STRUCT expression operators.  Returns a pointer to the
   last operation that was pushed.  */
static ada_structop_operation *
write_selectors (struct parser_state *par_state, const char *sels)
{
  ada_structop_operation *result = nullptr;
  while (*sels != '\0')
    {
      const char *p = chop_separator (sels);
      sels = p;
      while (*sels != '\0' && *sels != '.'
	     && (sels[0] != '_' || sels[1] != '_'))
	sels += 1;
      operation_up arg = ada_pop ();
      result = new ada_structop_operation (std::move (arg),
					   std::string (p, sels - p));
      pstate->push (operation_up (result));
    }
  return result;
}

/* Write a variable access (OP_VAR_VALUE) to ambiguous encoded name
   NAME[0..LEN-1], in block context BLOCK, to be resolved later.  Writes
   a temporary symbol that is valid until the next call to ada_parse.
   */
static void
write_ambiguous_var (struct parser_state *par_state,
		     const struct block *block, const char *name, int len)
{
  struct symbol *sym = new (&ada_parser->temp_space) symbol ();

  sym->set_domain (UNDEF_DOMAIN);
  sym->set_linkage_name (obstack_strndup (&ada_parser->temp_space, name, len));
  sym->set_language (language_ada, nullptr);

  block_symbol bsym { sym, block };
  par_state->push_new<ada_var_value_operation> (bsym);
}

/* A convenient wrapper around ada_get_field_index that takes
   a non NUL-terminated FIELD_NAME0 and a FIELD_NAME_LEN instead
   of a NUL-terminated field name.  */

static int
ada_nget_field_index (const struct type *type, const char *field_name0,
		      int field_name_len, int maybe_missing)
{
  char *field_name = (char *) alloca ((field_name_len + 1) * sizeof (char));

  strncpy (field_name, field_name0, field_name_len);
  field_name[field_name_len] = '\0';
  return ada_get_field_index (type, field_name, maybe_missing);
}

/* If encoded_field_name is the name of a field inside symbol SYM,
   then return the type of that field.  Otherwise, return NULL.

   This function is actually recursive, so if ENCODED_FIELD_NAME
   doesn't match one of the fields of our symbol, then try to see
   if ENCODED_FIELD_NAME could not be a succession of field names
   (in other words, the user entered an expression of the form
   TYPE_NAME.FIELD1.FIELD2.FIELD3), in which case we evaluate
   each field name sequentially to obtain the desired field type.
   In case of failure, we return NULL.  */

static struct type *
get_symbol_field_type (struct symbol *sym, const char *encoded_field_name)
{
  const char *field_name = encoded_field_name;
  const char *subfield_name;
  struct type *type = sym->type ();
  int fieldno;

  if (type == NULL || field_name == NULL)
    return NULL;
  type = check_typedef (type);

  while (field_name[0] != '\0')
    {
      field_name = chop_separator (field_name);

      fieldno = ada_get_field_index (type, field_name, 1);
      if (fieldno >= 0)
	return type->field (fieldno).type ();

      subfield_name = field_name;
      while (*subfield_name != '\0' && *subfield_name != '.'
	     && (subfield_name[0] != '_' || subfield_name[1] != '_'))
	subfield_name += 1;

      if (subfield_name[0] == '\0')
	return NULL;

      fieldno = ada_nget_field_index (type, field_name,
				      subfield_name - field_name, 1);
      if (fieldno < 0)
	return NULL;

      type = type->field (fieldno).type ();
      field_name = subfield_name;
    }

  return NULL;
}

/* Look up NAME0 (an unencoded identifier or dotted name) in BLOCK (or
   expression_block_context if NULL).  If it denotes a type, return
   that type.  Otherwise, write expression code to evaluate it as an
   object and return NULL. In this second case, NAME0 will, in general,
   have the form <name>(.<selector_name>)*, where <name> is an object
   or renaming encoded in the debugging data.  Calls error if no
   prefix <name> matches a name in the debugging data (i.e., matches
   either a complete name or, as a wild-card match, the final
   identifier).  */

static struct type*
write_var_or_type (struct parser_state *par_state,
		   const struct block *block, struct stoken name0)
{
  int depth;
  char *encoded_name;
  int name_len;

  std::string name_storage = ada_encode (name0.ptr);

  if (block == nullptr)
    {
      auto iter = ada_parser->iterated_associations.find (name_storage);
      if (iter != ada_parser->iterated_associations.end ())
	{
	  auto op = std::make_unique<ada_index_var_operation> ();
	  iter->second.push_back (op.get ());
	  par_state->push (std::move (op));
	  return nullptr;
	}

      block = par_state->expression_context_block;
    }

  name_len = name_storage.size ();
  encoded_name = obstack_strndup (&ada_parser->temp_space,
				  name_storage.c_str (),
				  name_len);
  for (depth = 0; depth < MAX_RENAMING_CHAIN_LENGTH; depth += 1)
    {
      int tail_index;

      tail_index = name_len;
      while (tail_index > 0)
	{
	  struct symbol *type_sym;
	  struct symbol *renaming_sym;
	  const char* renaming;
	  int renaming_len;
	  const char* renaming_expr;
	  int terminator = encoded_name[tail_index];

	  encoded_name[tail_index] = '\0';
	  /* In order to avoid double-encoding, we want to only pass
	     the decoded form to lookup functions.  */
	  std::string decoded_name = ada_decode (encoded_name);
	  encoded_name[tail_index] = terminator;

	  std::vector<struct block_symbol> syms
	    = ada_lookup_symbol_list (decoded_name.c_str (), block,
				      SEARCH_VFT);

	  type_sym = select_possible_type_sym (syms);

	  if (type_sym != NULL)
	    renaming_sym = type_sym;
	  else if (syms.size () == 1)
	    renaming_sym = syms[0].symbol;
	  else
	    renaming_sym = NULL;

	  switch (ada_parse_renaming (renaming_sym, &renaming,
				      &renaming_len, &renaming_expr))
	    {
	    case ADA_NOT_RENAMING:
	      break;
	    case ADA_PACKAGE_RENAMING:
	    case ADA_EXCEPTION_RENAMING:
	    case ADA_SUBPROGRAM_RENAMING:
	      {
		int alloc_len = renaming_len + name_len - tail_index + 1;
		char *new_name
		  = (char *) obstack_alloc (&ada_parser->temp_space,
					    alloc_len);
		strncpy (new_name, renaming, renaming_len);
		strcpy (new_name + renaming_len, encoded_name + tail_index);
		encoded_name = new_name;
		name_len = renaming_len + name_len - tail_index;
		goto TryAfterRenaming;
	      }
	    case ADA_OBJECT_RENAMING:
	      write_object_renaming (par_state, block, renaming, renaming_len,
				     renaming_expr, MAX_RENAMING_CHAIN_LENGTH);
	      write_selectors (par_state, encoded_name + tail_index);
	      return NULL;
	    default:
	      internal_error (_("impossible value from ada_parse_renaming"));
	    }

	  if (type_sym != NULL)
	    {
	      struct type *field_type;

	      if (tail_index == name_len)
		return type_sym->type ();

	      /* We have some extraneous characters after the type name.
		 If this is an expression "TYPE_NAME.FIELD0.[...].FIELDN",
		 then try to get the type of FIELDN.  */
	      field_type
		= get_symbol_field_type (type_sym, encoded_name + tail_index);
	      if (field_type != NULL)
		return field_type;
	      else
		error (_("Invalid attempt to select from type: \"%s\"."),
		       name0.ptr);
	    }
	  else if (tail_index == name_len && syms.empty ())
	    {
	      struct type *type = find_primitive_type (par_state,
						       encoded_name);

	      if (type != NULL)
		return type;
	    }

	  if (syms.size () == 1)
	    {
	      write_var_from_sym (par_state, syms[0]);
	      write_selectors (par_state, encoded_name + tail_index);
	      return NULL;
	    }
	  else if (syms.empty ())
	    {
	      struct objfile *objfile = nullptr;
	      if (block != nullptr)
		objfile = block->objfile ();

	      bound_minimal_symbol msym
		= ada_lookup_simple_minsym (decoded_name.c_str (), objfile);
	      if (msym.minsym != NULL)
		{
		  par_state->push_new<ada_var_msym_value_operation> (msym);
		  /* Maybe cause error here rather than later? FIXME? */
		  write_selectors (par_state, encoded_name + tail_index);
		  return NULL;
		}

	      if (tail_index == name_len
		  && strncmp (encoded_name, "standard__",
			      sizeof ("standard__") - 1) == 0)
		error (_("No definition of \"%s\" found."), name0.ptr);

	      tail_index = chop_selector (encoded_name, tail_index);
	    }
	  else
	    {
	      write_ambiguous_var (par_state, block, encoded_name,
				   tail_index);
	      write_selectors (par_state, encoded_name + tail_index);
	      return NULL;
	    }
	}

      if (!current_program_space->has_full_symbols ()
	  && !current_program_space->has_partial_symbols ()
	  && block == NULL)
	error (_("No symbol table is loaded.  Use the \"%ps\" command."),
	       styled_string (command_style.style (), "file"));
      if (block == par_state->expression_context_block)
	error (_("No definition of \"%s\" in current context."), name0.ptr);
      else
	error (_("No definition of \"%s\" in specified context."), name0.ptr);

    TryAfterRenaming: ;
    }

  error (_("Could not find renamed symbol \"%s\""), name0.ptr);

}

/* Because ada_completer_word_break_characters does not contain '.' --
   and it cannot easily be added, this breaks other completions -- we
   have to recreate the completion word-splitting here, so that we can
   provide a prefix that is then used when completing field names.
   Without this, an attempt like "complete print abc.d" will give a
   result like "print def" rather than "print abc.def".  */

std::string
ada_parse_state::find_completion_bounds ()
{
  const char *end = pstate->lexptr;
  /* First the end of the prefix.  Here we stop at the token start or
     at '.' or space.  */
  for (; end > m_original_expr && end[-1] != '.' && !c_isspace (end[-1]); --end)
    {
      /* Nothing.  */
    }
  /* Now find the start of the prefix.  */
  const char *ptr = end;
  /* Here we allow '.'.  */
  for (;
       ptr > m_original_expr && (ptr[-1] == '.'
				 || ptr[-1] == '_'
				 || (ptr[-1] >= 'a' && ptr[-1] <= 'z')
				 || (ptr[-1] >= 'A' && ptr[-1] <= 'Z')
				 || (ptr[-1] & 0xff) >= 0x80);
       --ptr)
    {
      /* Nothing.  */
    }
  /* ... except, skip leading spaces.  */
  ptr = skip_spaces (ptr);

  return std::string (ptr, end);
}

/* A wrapper for write_var_or_type that is used specifically when
   completion is requested for the last of a sequence of
   identifiers.  */

static struct type *
write_var_or_type_completion (struct parser_state *par_state,
			      const struct block *block, struct stoken name0)
{
  int tail_index = chop_selector (name0.ptr, name0.length);
  /* If there's no separator, just defer to ordinary symbol
     completion.  */
  if (tail_index == -1)
    return write_var_or_type (par_state, block, name0);

  std::string copy (name0.ptr, tail_index);
  struct type *type = write_var_or_type (par_state, block,
					 { copy.c_str (),
					   (int) copy.length () });
  /* For completion purposes, it's enough that we return a type
     here.  */
  if (type != nullptr)
    return type;

  ada_structop_operation *op = write_selectors (par_state,
						name0.ptr + tail_index);
  op->set_prefix (ada_parser->find_completion_bounds ());
  par_state->mark_struct_expression (op);
  return nullptr;
}

/* Write a left side of a component association (e.g., NAME in NAME =>
   exp).  If NAME has the form of a selected component, write it as an
   ordinary expression.  If it is a simple variable that unambiguously
   corresponds to exactly one symbol that does not denote a type or an
   object renaming, also write it normally as an OP_VAR_VALUE.
   Otherwise, write it as an OP_NAME.

   Unfortunately, we don't know at this point whether NAME is supposed
   to denote a record component name or the value of an array index.
   Therefore, it is not appropriate to disambiguate an ambiguous name
   as we normally would, nor to replace a renaming with its referent.
   As a result, in the (one hopes) rare case that one writes an
   aggregate such as (R => 42) where R renames an object or is an
   ambiguous name, one must write instead ((R) => 42). */

static void
write_name_assoc (struct parser_state *par_state, struct stoken name)
{
  if (strchr (name.ptr, '.') == NULL)
    {
      std::vector<struct block_symbol> syms
	= ada_lookup_symbol_list (name.ptr,
				  par_state->expression_context_block,
				  SEARCH_VFT);

      if (syms.size () != 1 || syms[0].symbol->loc_class () == LOC_TYPEDEF)
	pstate->push_new<ada_string_operation> (copy_name (name));
      else
	write_var_from_sym (par_state, syms[0]);
    }
  else
    if (write_var_or_type (par_state, NULL, name) != NULL)
      error (_("Invalid use of type."));

  push_association<ada_name_association> (ada_pop ());
}

static struct type *
type_for_char (struct parser_state *par_state, ULONGEST value)
{
  if (value <= 0xff)
    return language_string_char_type (par_state->language (),
				      par_state->gdbarch ());
  else if (value <= 0xffff)
    return language_lookup_primitive_type (par_state->language (),
					   par_state->gdbarch (),
					   "wide_character");
  return language_lookup_primitive_type (par_state->language (),
					 par_state->gdbarch (),
					 "wide_wide_character");
}

static struct type *
type_system_address (struct parser_state *par_state)
{
  struct type *type
    = language_lookup_primitive_type (par_state->language (),
				      par_state->gdbarch (),
				      "system__address");
  return  type != NULL ? type : parse_type (par_state)->builtin_data_ptr;
}
