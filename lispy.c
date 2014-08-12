// vim: noai:ts=2:sw=2
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <editline/readline.h>
#include "mpc.h"

typedef struct {
  int type;
  long num;
  double dub;
  int err;
} lval;

/* Create Enumeration of Possible lval Types */
enum { LVAL_NUM, LVAL_DUB, LVAL_ERR };

/* Create Enumeration of Possible Error Types */
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

/* Create a new number type lval */
lval lval_num(long x) {
  lval v;
  v.type = LVAL_NUM;
  v.num = x;
  return v;
}

/* Create a new double type lval */
lval lval_dub(double x) {
  lval v;
  v.type = LVAL_DUB;
  v.dub = x;
  return v;
}

/* Create a new error type lval */
lval lval_err(int x) {
  lval v;
  v.type = LVAL_ERR;
  v.err = x;
  return v;
}

/* Print an "lval" */
void lval_print(lval v) {
  switch (v.type) {

    case LVAL_DUB: printf("%g", v.dub); break;
    /* In the case the type is a number print it, then 'break' out of the switch. */
    case LVAL_NUM: printf("%li", v.num); break;

    /* In the case the type is an error */
    case LVAL_ERR:
      /* Check What exact type of error it is and print it */
      if (v.err == LERR_DIV_ZERO) { printf("Error: Division By Zero!"); }
      if (v.err == LERR_BAD_OP)   { printf("Error: Invalid Operator!"); }
      if (v.err == LERR_BAD_NUM)  { printf("Error: Invalid Number!"); }
    break;
  }
}

/* Print an "lval" followed by a newline */
void lval_println(lval v) { lval_print(v); putchar('\n'); }

lval eval_op(lval x, char* op, lval y) {

  /* If either value is an error return it */
  if (x.type == LVAL_ERR) { return x; }
  if (y.type == LVAL_ERR) { return y; }

  /* Otherwise do maths on the number values */
  if (x.type == LVAL_NUM && y.type == LVAL_NUM) {
    if (strcmp(op, "+") == 0) { return lval_num(x.num + y.num); }
    if (strcmp(op, "-") == 0) { return lval_num(x.num - y.num); }
    if (strcmp(op, "*") == 0) { return lval_num(x.num * y.num); }
    if (strcmp(op, "/") == 0) {
      /* If second operand is zero return error instead of result */
      return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num);
    }
    if (strcmp(op, "%") == 0) {
      return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num % y.num);
    }
    if (strcmp(op, "^") == 0) { return lval_num(pow(x.num,y.num)); }
    if (strcmp(op, "min") == 0) { return lval_num(fmin(x.num,y.num)); }
    if (strcmp(op, "max") == 0) { return lval_num(fmax(x.num,y.num)); }
  } else if (x.type == LVAL_DUB || y.type == LVAL_DUB) {
    double a = (x.type == LVAL_NUM) ? x.num : x.dub;
    double b = (y.type == LVAL_NUM) ? y.num : y.dub;
    if (strcmp(op, "+") == 0) { return lval_dub(a + b); }
    if (strcmp(op, "-") == 0) { return lval_dub(a - b); }
    if (strcmp(op, "*") == 0) { return lval_dub(a * b); }
    if (strcmp(op, "/") == 0) {
      /* If second operand is zero return error instead of result */
      return b == 0 ? lval_err(LERR_DIV_ZERO) : lval_dub(a / b);
    }
    if (strcmp(op, "%") == 0) {
      return b == 0 ? lval_err(LERR_DIV_ZERO) : lval_dub(fmod(a, b));
    }
    if (strcmp(op, "^") == 0) { return lval_dub(pow(a,b)); }
    if (strcmp(op, "min") == 0) { return lval_dub(fmin(a,b)); }
    if (strcmp(op, "max") == 0) { return lval_dub(fmax(a,b)); }
  }

  return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t* t) {
  if (strstr(t->tag, "number")) {
    /* Check if there is some error in conversion */
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
  }

  if (strstr(t->tag, "double")) {
    /* Check if there is some error in conversion */
    errno = 0;
    double x = strtod(t->contents, NULL);
    return errno != ERANGE ? lval_dub(x) : lval_err(LERR_BAD_NUM);
  }

  char* op = t->children[1]->contents;
  lval x = eval(t->children[2]);

  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }

  return x;
}

int main(int argc, char** argv) {

  /* Create Some Parsers */
  mpc_parser_t* Number   = mpc_new("number");
  mpc_parser_t* Double   = mpc_new("double");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr     = mpc_new("expr");
  mpc_parser_t* Lispy    = mpc_new("lispy");

  /* Define them with the following Language */
  mpca_lang(MPCA_LANG_DEFAULT,
      "                                                            \
      number   : /-?[0-9]+/ ;                          \
      double   : /-?[0-9]+\\.[0-9]+/ ;                          \
      operator : '+' | '-' | '*' | '/' | '%' | '^' | \"min\" | \"max\"; \
      expr     : <double> | <number> | '(' <operator> <expr>+ ')' ;           \
      lispy    : /^/ <operator> <expr>+ /$/ ;                      \
      ",
      Number, Double, Operator, Expr, Lispy);

  /* Print Version and Exit Information */
  puts("Lispy Version 0.0.0.0.1");
  puts("Press Ctrl+c to Exit\n");

  /* In a never ending loop */
  while (1) {

    /* Output our prompt and get input */
    char* input = readline("lispy> ");

    /* Add input to history */
    add_history(input);

    /* Attempt to Parse the user Input */
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      /* eval */
      lval result = eval(r.output);
      lval_println(result);
      mpc_ast_delete(r.output);
    } else {
      /* Otherwise Print the Error */
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    /* Free retrived input */
    free(input);

  }

  /* Undefine and Delete our Parsers */
  mpc_cleanup(5, Number, Double, Operator, Expr, Lispy);

  return 0;
}
