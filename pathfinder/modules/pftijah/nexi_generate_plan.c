/*

     NEXItoAlgebra.c
     =========================
     Author: Vojkan Mihajlovic
     University of Twente

     Module that generates logical query plans and MIL query plans from NEXI queries

*/



#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include "nexi.h"
#include "nexi_generate_plan.h"

/***
    Function for parsing NEXI CAS Queries and generating logical query plan and MIL code that simulates physical query plan.
        query_num is a number of the starting query
***/

command_tree **CAS_plan_gen(
	int query_num,
	int topic_type,
	struct_RMT *txt_retr_model,
	struct_RF *rel_feedback,
	bool alg_type,
	char *mil_fname,
	char *log_fname,
	char *result_name,
	bool scale_on)
{

  /* command count */
  int com_cnt;
  /* step inside predicate */
  bool in_step;
  /* variable for reading parser command output */
  command parse_com;
  /* variable for reading parser token output */
  char parse_nam[20];
  /* probabilistic propagation */
  bool follow_step;
  /* counters for brackets inside about predicate */
  int num_bracket;
  int br_count;
  /* indicator for reinitialization of variables in mil (:= nil) */
  bool first_step;

  /* number of MIL variables */
  int var_num;

  /* indicator for file end */
  bool end_process;

  /* topic number (based on a query num imput parameter) */
  int topic_num;

  /* temporarary store for commands */
  command parse_com_tmp;
 

  /* number of terms */
  unsigned int num_term = 0;
  /* number of phrases */
  int num_phrase = 0;
  /* number of terms with plus */
  unsigned int plus_term = 0;
  /* number of terms with minus */
  unsigned int minus_term = 0;
  /* type of term prefix */
  command prefix;

  /* temorary store for comparison operator type */
  unsigned int operator_tmp;

  /* temporary storage for node name */
  bool ctx_in_node;
  char node_tmp[TERM_LENGTH];

  /* stack pointers for stack structures */
  unsigned int term_sp, phrase_sp, op_sp, res_sp, step_sp;
  /* term reader */
  char t[TERM_LENGTH];
  /* phrase reader */
  char p[TERM_LENGTH];
  /* command readers */
  command s;
  command op_cod;
  command step_ty;
  /* command pointer readers */
  command_tree *p_leftop;
  command_tree *p_result;
  command_tree *p_step;
  /* term stacks */
  char term_lifo[STACK_MAX][TERM_LENGTH];
  int ts_lifo[STACK_MAX];
  /* phrase stacks */
  char phrase_lifo[STACK_MAX][TERM_LENGTH];
  int ps_lifo[STACK_MAX];
  /* command stacks */
  command op_lifo[STACK_MAX];
  command stt_lifo[STACK_MAX];
  command_tree *opp_lifo[STACK_MAX];
  command_tree *res_lifo[STACK_MAX];
  command_tree *step_lifo[STACK_MAX];

  /* pointers for command tree structure */
  command_tree *p_command, *p1_command=NULL, *p2_command=NULL, *p3_command, *p_adjcommand;
  command_tree *p_ctx=NULL, *p_ctx_in;
  command_tree **p_command_array,**p_command_start;

  /* files obtained after NEXI parsing and for storing logical and phisical query plans */
  FILE *command_file;
  FILE *token_file;
  FILE *mil_file = NULL;
  FILE *log_file = NULL;

  end_process = FALSE;

  if ( mil_fname && !(mil_file = fopen(myfileName(WORKDIR,mil_fname),"w")) ) {
    printf("Error: cannot find file for writing mil code.\n");
    return NULL;
  }

  if ( log_fname && !(log_file = fopen(myfileName(WORKDIR,log_fname),"w")) ) {
    printf("Error: cannot find file for writing logical plan.\n");
    return NULL;
  }

  command_file = fopen(myfileName(WORKDIR,"file_command.nxi"),"r");
  token_file   = fopen(myfileName(WORKDIR,"file_token.nxi"),"r");

  if (command_file == NULL) {
    printf("Error: cannot find command file for reading.\n");
    return NULL;
  }


  if (token_file == NULL) {
    printf("Error: cannot find token file for reading.\n");
    return NULL;
  }

  /* formating the mil header */

  if ( mil_file ) fprintf(mil_file, "\tVAR ");

  for (var_num = 0; var_num < MAX_VARS-1; var_num++) {
    if ( mil_file ) fprintf(mil_file, "R%d, ", var_num);
  }

  if ( mil_file ) fprintf(mil_file, "R%d;\n", MAX_VARS-1);

  if ( mil_file ) fprintf(mil_file, "\tVAR ");

  for (var_num = query_num; var_num < query_num + MAX_QUERIES-1; var_num++) {
    if ( mil_file ) fprintf(mil_file, "topic_%d, ", var_num);
  }

  if ( mil_file ) fprintf(mil_file, "topic%d;\n\n", query_num + MAX_QUERIES-1);

  if ( mil_file ) fprintf(mil_file, "VAR topics := new(int,str,50);\n\n");


  /* initialization for query plan trees */
  p_command_array = calloc(MAX_QUERIES, sizeof(command_tree*));
  p_command_start = p_command_array;
  p_command = calloc(MAX_QUERIES*OPERAND_MAX, sizeof(command_tree));

  /* printf("%d\n",p_command_array); */
  /* printf("%d\n",p_command); */

  /* for number initialization */
  topic_num = query_num - 1;

  /* iterates untill the end of the input query file */

  while(!feof(command_file) && !end_process) {

    com_cnt = 0;
    in_step = FALSE;
    parse_com = EMPTY;
    follow_step = FALSE;
    num_bracket = 0;
    br_count = 0;
    first_step = FALSE;
    ctx_in_node = FALSE;

    topic_num++;

    /* INEX specific !!!!!!!!!!!!!!!!!! SHOULD BE REMOVED */
    if (topic_num == 148 || (topic_type == CAS_TOPIC && (topic_num == 206 || topic_num == 209 || topic_num == 221 || topic_num == 227 || topic_num == 235 || topic_num == 237)))
      topic_num++;
    if (topic_type == CAS_TOPIC && (topic_num == 217))
      topic_num+=2;
    if (topic_type == CAS_TOPIC && (topic_num == 213))
      topic_num+=3;

    if ( mil_file ) fprintf(mil_file, "printf(\"Executing topic number %d...\\t\");\n\n", topic_num);

    if ( mil_file ) fprintf(mil_file, "topics.insert(%d,\"%s%d_probab\");\n", topic_num, result_name, topic_num);
    if ( mil_file ) fprintf(mil_file, "restore := false;\n");
    if ( mil_file ) fprintf(mil_file, "link := false;\n\n");


    if ( mil_file ) fprintf(mil_file, "var lambda := %f;\n", txt_retr_model->param1);
    if ( mil_file ) fprintf(mil_file, "var eta := %f;\n\n", txt_retr_model->param2);


    if ( log_file ) fprintf(log_file, "# Query plan for topic number %d.\n\n", topic_num);

    parse_com_tmp = EMPTY;


    term_sp = 0;
    phrase_sp = 0;
    op_sp = 0;
    res_sp = 0;
    step_sp = 0;
    s = EMPTY;
    op_cod = EMPTY;
    p_leftop = NULL;
    p_result = NULL;
    p_step = NULL;


    /* initialization: selecting the root node */
    
    fscanf(command_file, "%d", &parse_com);

    if (parse_com == DSC) {

	p_command->number = com_cnt;
    	p_command->operator = SELECT_NODE;
    	p_command->left = NULL;
    	p_command->right = NULL;
    	strcpy(p_command->argument,"\"Root\"");
  
    	if ( log_file ) fprintf(log_file, "%s\n", "R0 := SELECT_NODE(\"Root\");");
    	if ( mil_file ) fprintf(mil_file, "%s\n", "R0 := ctx_pre;");
    	p1_command = p_command;
	
    }

    else if (parse_com == JOURNAL_ROOT) {

	p_command->number = com_cnt;
    	p_command->operator = SELECT_NODE;
    	p_command->left = NULL;
    	p_command->right = NULL;
    	strcpy(p_command->argument,rel_feedback->journal_name);
  
    	if ( log_file ) fprintf(log_file, "R0 := SELECT_NODE(\"%s\");\n",rel_feedback->journal_name);
    	if ( mil_file ) fprintf(mil_file, "R0 := select_journal(\"%s\");\n",rel_feedback->journal_name);
    	p1_command = p_command;

    }

    /* loop for reading the command file */

    while (!feof(command_file) && parse_com != QUERY_END) {


      /*loop for the execution descendants + name tests including UNION of name tests, e.g., //(a|b) */

      while (parse_com != OPEN && !feof(command_file)) {

	/* normal step before, inside or after the predicate */

	fscanf(command_file, "%d", &parse_com);
	/*printf("x:%d\n", parse_com); */

	if (parse_com == STAR) {

	  if (follow_step == FALSE) {

	    com_cnt++;
	    p_command++;

	    p_command->number = com_cnt;
	    p_command->operator = CONTAINED_BY;
	    p_command->left = NULL;
	    p_command->right = p1_command;
	    strcpy(p_command->argument, "C");

	    if ( log_file ) fprintf(log_file, "R%d := C CONTAINED_BY R%d;\n",  com_cnt, p1_command->number);

	    if (topic_type == CAS_TOPIC) {
	      if ( mil_file ) fprintf(mil_file, "R%d := R%d.descendant();\n",  com_cnt, p1_command->number);
	    else if (topic_type == CO_TOPIC)
	      if ( mil_file ) fprintf(mil_file, "R%d := R%d.descendant_or_self();\n",  com_cnt, p1_command->number);

	    }
	    if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p1_command->number);

	    /* EXCLUDED IN THE NEW VERSION BECAUSE OF MEMORY PROBLEMS
	    p1_command = p_command;

	    com_cnt++;
	    p_command++;

	    p_command->number = com_cnt;
	    p_command->operator = SELECT_NODE;
	    p_command->left = p1_command;
	    p_command->right = NULL;
	    strcpy(p_command->argument, "");

	    if ( log_file ) fprintf(log_file, "R%d := SELECT_NODE(R%d);\n",  com_cnt, p1_command->number);
	    */

	    if ( mil_file ) fprintf(mil_file, "R%d := R%d.node();\n",  com_cnt, p1_command->number);

	  }

	  else {

	    com_cnt++;
	    p_command++;

	    p_command->number = com_cnt;
	    p_command->operator = P_CONTAINED_BY;
	    p_command->left = NULL;
	    p_command->right = p1_command;
	    strcpy(p_command->argument, "C");

	    if ( log_file ) fprintf(log_file, "R%d := C P_CONTAINED_BY R%d;\n",  com_cnt, p1_command->number);

	    if (first_step == FALSE) {

	      if (topic_type == CAS_TOPIC) {
		if ( mil_file ) fprintf(mil_file, "R%d := R%d.p_descendant();\n",  com_cnt, p1_command->number);
	      else if (topic_type == CO_TOPIC)
		if ( mil_file ) fprintf(mil_file, "R%d := R%d.p_descendant_or_self();\n",  com_cnt, p1_command->number);
	      }
	      if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p1_command->number);

	    }

	    else {

	      if (topic_type == CAS_TOPIC) {
		if ( mil_file ) fprintf(mil_file, "R%d := R%d.p_descendant();\n",  com_cnt, p_ctx->number);
	      else if (topic_type == CO_TOPIC)
		if ( mil_file ) fprintf(mil_file, "R%d := R%d.p_descendant_or_self();\n",  com_cnt, p_ctx->number);
		if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p_ctx->number);
	      }
	      first_step = FALSE;

	    }

	    /* EXCLUDED IN THE NEW VERSION BECAUSE OF MEMORY PROBLEMS
	    p1_command = p_command;

	    com_cnt++;
	    p_command++;

	    p_command->number = com_cnt;
	    p_command->operator = SELECT_NODE;
	    p_command->left = p1_command;
	    p_command->right = NULL;
	    strcpy(p_command->argument, "");

	    if ( log_file ) fprintf(log_file, "R%d := SELECT_NODE(R%d);\n",  com_cnt, p1_command->number);
	    */

	    if ( mil_file ) fprintf(mil_file, "R%d := R%d.node();\n",  com_cnt, p_command->number);

	  }

	}

	else if (parse_com == OB) {

	  p3_command = p1_command;

	  fscanf(command_file, "%d", &parse_com);
	  /*printf("STRUCT_OR:%d\n", parse_com); */
	  fscanf(token_file, "%s", &parse_nam[0]);

	  com_cnt++;
	  p_command = p_command + 1;

	  p_command->number = com_cnt;
	  p_command->operator = SELECT_NODE;
	  p_command->left = NULL;
	  p_command->right = NULL;
	  strcpy(p_command->argument, parse_nam);

	  if ( log_file ) fprintf(log_file, "R%d := SELECT_NODE(%s);\n", com_cnt, parse_nam);

	  if (follow_step == FALSE) {

	    if ( mil_file ) fprintf(mil_file, "R%d := qname(%s,ENTITY_PN_TYPE);\n",  com_cnt, parse_nam);

	  }

	  else {

	    if ( mil_file ) fprintf(mil_file, "R%d := p_qname(%s,ENTITY_PN_TYPE);\n",  com_cnt, parse_nam);

	  }

	  p1_command = p_command;

	  fscanf(command_file, "%d", &parse_com);
	  /*printf("STRUCT_OR:%d\n", parse_com); */

	  while (parse_com != CB) {

	    fscanf(command_file, "%d", &parse_com);
	    /*printf("STRUCT_OR:%d\n", parse_com); */
	    fscanf(token_file, "%s", &parse_nam[0]);

	    com_cnt++;
	    p_command = p_command + 1;

	    p_command->number = com_cnt;
	    p_command->operator = SELECT_NODE;
	    p_command->left = NULL;
	    p_command->right = NULL;
	    strcpy(p_command->argument, parse_nam);

	    if ( log_file ) fprintf(log_file, "R%d := SELECT_NODE(%s);\n", com_cnt, parse_nam);

	    p2_command = p_command;

	    if (follow_step == FALSE) {

	      if ( mil_file ) fprintf(mil_file, "R%d := qname(%s,ENTITY_PN_TYPE);\n",  com_cnt, parse_nam);

	      com_cnt++;
	      p_command++;

	      p_command->number = com_cnt;
	      p_command->operator = UNION;
	      p_command->left = p1_command;
	      p_command->right = p2_command;
	      strcpy(p_command->argument, "");

	      if ( log_file ) fprintf(log_file, "R%d := R%d UNION R%d;\n",  com_cnt, p1_command->number, p2_command->number);
	      if ( mil_file ) fprintf(mil_file, "R%d := R%d.union(R%d);\n",  com_cnt, p1_command->number, p2_command->number);

	    }

	    else {

	      if ( mil_file ) fprintf(mil_file, "R%d := p_qname(%s,ENTITY_PN_TYPE);\n",  com_cnt, parse_nam);

	      com_cnt++;
	      p_command++;

	      p_command->number = com_cnt;
	      p_command->operator = P_UNION;
	      p_command->left = p1_command;
	      p_command->right = p2_command;
	      strcpy(p_command->argument, "");

	      if ( log_file ) fprintf(log_file, "R%d := R%d P_UNION R%d;\n",  com_cnt, p1_command->number, p2_command->number);
	      if ( mil_file ) fprintf(mil_file, "R%d := R%d.p_union(R%d);\n",  com_cnt, p1_command->number, p2_command->number);

	    }

	    if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p1_command->number);
	    if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p2_command->number);

	    fscanf(command_file, "%d", &parse_com);
	    /*printf("x:%d\n", parse_com); */

	  }

	  p1_command = p_command;

	  if (follow_step == FALSE) {

	    com_cnt++;
	    p_command++;

	    p_command->number = com_cnt;
	    p_command->operator = CONTAINED_BY;
	    p_command->left = p1_command;
	    p_command->right = p3_command;
	    strcpy(p_command->argument, "");

	    if ( log_file ) fprintf(log_file, "R%d := R%d CONTAINED_BY R%d;\n",  com_cnt, p1_command->number, p3_command->number);
	    if ( mil_file ) fprintf(mil_file, "R%d := R%d.descendant(R%d);\n",  com_cnt, p3_command->number, p1_command->number);

	    if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p1_command->number);
	    if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p3_command->number);

	  }

	  else {

	    com_cnt++;
	    p_command++;

	    p_command->number = com_cnt;
	    p_command->operator = P_CONTAINED_BY;
	    p_command->left = p2_command;
	    p_command->right = p1_command;
	    strcpy(p_command->argument, "");

	    if ( log_file ) fprintf(log_file, "R%d := R%d P_CONTAINED_BY R%d;\n",  com_cnt, p3_command->number, p1_command->number);

	    if (first_step == FALSE) {
	      if ( mil_file ) fprintf(mil_file, "R%d := R%d.p_descendant(R%d);\n",  com_cnt, p1_command->number, p3_command->number);
	      if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p1_command->number);
	      if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p3_command->number);
	    }
	    else {
	      if ( mil_file ) fprintf(mil_file, "R%d := R%d.p_descendant(R%d);\n",  com_cnt, p_ctx->number, p3_command->number);
	      if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p_ctx->number);
	      if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p3_command->number);
	      first_step = FALSE;
	    }

	  }

	}

	else if (parse_com == VAGUE) {

	  fscanf(command_file, "%d", &parse_com);

 	  fscanf(token_file, "%s", &parse_nam[0]);
	  com_cnt++;
	  p_command = p_command + 1;

	  p_command->number = com_cnt;
	  p_command->operator = SELECT_NODE_VAGUE;
	  p_command->left = NULL;
	  p_command->right = NULL;
	  strcpy(p_command->argument, parse_nam);

	  if ( log_file ) fprintf(log_file, "R%d := SELECT_NODE_VAGUE(%s);\n", com_cnt, parse_nam);
	  p2_command = p_command;

	  if (follow_step == FALSE) {

	    if ( mil_file ) fprintf(mil_file, "R%d := qname(%s,ENTITY_PN_TYPE);\n",  com_cnt, parse_nam);

	    com_cnt++;
	    p_command++;

	    p_command->number = com_cnt;
	    p_command->operator = CONTAINED_BY;
	    p_command->left = p2_command;
	    p_command->right = p1_command;
	    strcpy(p_command->argument, "");

	    if ( log_file ) fprintf(log_file, "R%d := R%d CONTAINED_BY R%d;\n",  com_cnt, p2_command->number, p1_command->number);
	    if ( mil_file ) fprintf(mil_file, "R%d := R%d.descendant(R%d);\n",  com_cnt, p1_command->number, p2_command->number);

	    if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p1_command->number);
	    if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p2_command->number);

	  }

	  else {

	    if ( mil_file ) fprintf(mil_file, "R%d := p_qname(%s,ENTITY_PN_TYPE);\n",  com_cnt, parse_nam);

	    com_cnt++;
	    p_command++;

	    p_command->number = com_cnt;
	    p_command->operator = P_CONTAINED_BY;
	    p_command->left = p2_command;
	    p_command->right = p1_command;
	    strcpy(p_command->argument, "");

	    if ( log_file ) fprintf(log_file, "R%d := R%d P_CONTAINED_BY R%d;\n",  com_cnt, p2_command->number, p1_command->number);

	    if (first_step == FALSE) {
	      if ( mil_file ) fprintf(mil_file, "R%d := R%d.p_descendant(R%d);\n",  com_cnt, p1_command->number, p2_command->number);
	      if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p1_command->number);
	      if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p2_command->number);
	    }
	    else {
	      if ( mil_file ) fprintf(mil_file, "R%d := R%d.p_descendant(R%d);\n",  com_cnt, p_ctx->number, p2_command->number);
	      if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p_ctx->number);
	      if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p2_command->number);
	      first_step = FALSE;
	    }

	  }

	}

	else if (parse_com == SELECT_NODE) {

 	  fscanf(token_file, "%s", &parse_nam[0]);
	  com_cnt++;
	  p_command = p_command + 1;

	  p_command->number = com_cnt;
	  p_command->operator = SELECT_NODE;
	  p_command->left = NULL;
	  p_command->right = NULL;
	  strcpy(p_command->argument, parse_nam);

	  if ( log_file ) fprintf(log_file, "R%d := SELECT_NODE(%s);\n", com_cnt, parse_nam);
	  p2_command = p_command;

	  if (follow_step == FALSE) {

	    if ( mil_file ) fprintf(mil_file, "R%d := qname(%s,ENTITY_PN_TYPE);\n",  com_cnt, parse_nam);

	    com_cnt++;
	    p_command++;

	    p_command->number = com_cnt;
	    p_command->operator = CONTAINED_BY;
	    p_command->left = p2_command;
	    p_command->right = p1_command;
	    strcpy(p_command->argument, "");

	    if ( log_file ) fprintf(log_file, "R%d := R%d CONTAINED_BY R%d;\n",  com_cnt, p2_command->number, p1_command->number);
	    if ( mil_file ) fprintf(mil_file, "R%d := R%d.descendant(R%d);\n",  com_cnt, p1_command->number, p2_command->number);

	    if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p1_command->number);
	    if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p2_command->number);

	  }

	  else {

	    if ( mil_file ) fprintf(mil_file, "R%d := p_qname(%s,ENTITY_PN_TYPE);\n",  com_cnt, parse_nam);

	    com_cnt++;
	    p_command++;

	    p_command->number = com_cnt;
	    p_command->operator = P_CONTAINED_BY;
	    p_command->left = p2_command;
	    p_command->right = p1_command;
	    strcpy(p_command->argument, "");

	    if ( log_file ) fprintf(log_file, "R%d := R%d P_CONTAINED_BY R%d;\n",  com_cnt, p2_command->number, p1_command->number);

	    if (first_step == FALSE) {
	      if ( mil_file ) fprintf(mil_file, "R%d := R%d.p_descendant(R%d);\n",  com_cnt, p1_command->number, p2_command->number);
	      if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p1_command->number);
	      if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p2_command->number);
	    }
	    else {
	      if ( mil_file ) fprintf(mil_file, "R%d := R%d.p_descendant(R%d);\n",  com_cnt, p_ctx->number, p2_command->number);
	      if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p_ctx->number);
	      if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p2_command->number);
	      first_step = FALSE;
	    }

	  }

	}

	p1_command = p_command;

	fscanf(command_file, "%d", &parse_com);
	/*printf("x:%d\n", parse_com); */

      }



      /* entering the predicate */

      if (parse_com == OPEN) {

	op_sp = 0;
	op_cod = EMPTY;
	p_leftop = NULL;
	ctx_in_node = FALSE;

	p_ctx = p_command;
	if ( mil_file ) fprintf(mil_file, "\nR%d := init_IR(R%d);\n",  p_ctx->number, p_ctx->number);
	fscanf (command_file, "%d", &parse_com);
	/*printf("%d\n",parse_com); */
	/* loop for the predicate */

	while (parse_com != CLOSE) {

	  first_step = TRUE;

	  while (parse_com == OB) {

	    op_sp++;
	    PUSH_OP(OB,p_command);

	    fscanf (command_file, "%d", &parse_com);
	    /*printf("%d %d\n",parse_com,op_sp); */
	  }

	  /* near value search */

	  if (parse_com == CURRENT) {

	    fscanf (command_file, "%d", &parse_com);
	    /*printf("%d\n",parse_com); */
	    p1_command = p_ctx;

	    if ( mil_file ) fprintf(mil_file, "\n\tR%d := init_XE(R%d);\n",  p_ctx->number, p_ctx->number);

	    if (parse_com != GR && parse_com != LS && parse_com != EQ) {

	      in_step = TRUE;

	      fscanf (command_file, "%d", &parse_com);
	      /*printf("%d\n", parse_com); */

	      while (parse_com != GR && parse_com != LS && parse_com != EQ) {

		if (parse_com == STAR) {

		  com_cnt++;
		  p_command++;

		  p_command->number = com_cnt;
		  p_command->operator = CONTAINED_BY;
		  p_command->left = NULL;
		  p_command->right = p1_command;
		  strcpy(p_command->argument, "C");

		  if ( log_file ) fprintf(log_file, "R%d := C CONTAINED_BY R%d;\n", com_cnt, p1_command->number);

		  step_sp++;
		  PUSH_STEP(STAR,p_command);

		  if ( mil_file ) fprintf(mil_file, "\tR%d := R%d.p_descendant();\n",  com_cnt, p1_command->number);

		  if (first_step == FALSE) {
		    if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p1_command->number);
		  } else
		    first_step = FALSE;

		}

		else if (parse_com == OB) {

		  p3_command = p_command;

		  fscanf(command_file, "%d", &parse_com);
		  /*printf("STRUCT_OR:%d\n", parse_com); */
		  fscanf(token_file, "%s", &parse_nam[0]);
		  com_cnt++;
		  p_command++;

		  p_command->number = com_cnt;
		  p_command->operator = SELECT_NODE;
		  p_command->left = NULL;
		  p_command->right = NULL;
		  strcpy(p_command->argument, parse_nam);

		  if ( log_file ) fprintf(log_file, "R%d := SELECT_NODE(%s);\n", com_cnt, parse_nam);
		  if ( mil_file ) fprintf(mil_file, "\tR%d := p_qname(%s,ENTITY_PN_TYPE);\n",  com_cnt, parse_nam);

		  p1_command = p_command;

		  step_sp++;
		  PUSH_STEP(DSC,p_command);

		  fscanf(command_file, "%d", &parse_com);
		  /*printf("%d\n", parse_com); */

		  while (parse_com != CB) {

		    fscanf(command_file, "%d", &parse_com);
		    /*printf("STRUCT_OR:%d\n", parse_com); */
		    fscanf(token_file, "%s", &parse_nam[0]);
		    com_cnt++;
		    p_command = p_command + 1;

		    p_command->number = com_cnt;
		    p_command->operator = SELECT_NODE;
		    p_command->left = NULL;
		    p_command->right = NULL;
		    strcpy(p_command->argument, parse_nam);

		    if ( log_file ) fprintf(log_file, "R%d := SELECT_NODE(%s);\n", com_cnt, parse_nam);
		    if ( mil_file ) fprintf(mil_file, "\tR%d := p_qname(%s,ENTITY_PN_TYPE);\n",  com_cnt, parse_nam);

		    p2_command = p_command;

		    step_sp++;
		    PUSH_STEP(STRUCT_OR, p_command);

		    if ( mil_file ) fprintf(mil_file, "\tR%d := R%d.p_union(R%d);\n",  com_cnt, p1_command->number, p2_command->number);

		    if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p1_command->number);

		    p1_command = p_command;
		    
		    fscanf(command_file, "%d", &parse_com);
		    /*printf("x:%d\n", parse_com); */
		    
		  }
		  
		  if ( mil_file ) fprintf(mil_file, "\tR%d := R%d.p_descendant(R%d);\n",  com_cnt, p3_command->number, p1_command->number);
		  
		  if (first_step == FALSE) { 
		    if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p3_command->number);
		  } else 
		    first_step = FALSE;
		
		}

		else if (parse_com == SELECT_NODE) {
		  
		  fscanf(token_file, "%s", &parse_nam[0]);
		  com_cnt++;
		  p_command++;

		  p_command->number = com_cnt;
		  p_command->operator = SELECT_NODE;
		  p_command->left = NULL;
		  p_command->right = NULL;
		  strcpy(p_command->argument, parse_nam);

		  if ( log_file ) fprintf(log_file, "R%d := SELECT_NODE(%s);\n", com_cnt, parse_nam);
		  if ( mil_file ) fprintf(mil_file, "\tR%d := p_qname(%s,ENTITY_PN_TYPE);\n",  com_cnt, parse_nam);

		  step_sp++;
		  PUSH_STEP(DSC,p_command);

		  if ( mil_file ) fprintf(mil_file, "\tR%d := R%d.p_descendant(R%d);\n",  com_cnt, p1_command->number, p_command->number);

		  if (first_step == FALSE) {
		    if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p1_command->number);
		  } else
		    first_step = FALSE;

		}

		p1_command = p_command;

		fscanf(command_file, "%d", &parse_com);
		/*printf("x:%d\n", parse_com); */

	      }

	      fscanf (command_file, "%d", &parse_com_tmp);
	      /*printf("%d\n",parse_com_tmp); */

	      /* comparing the content of the element (<,>,=) with the specified  value */
	      /* finding the right operator (<=,>=) */
	      if (parse_com_tmp == SELECT_NODE) {

		if (parse_com == GR) {
		  parse_com = GREATER;
		  operator_tmp = P_SELECT_GR;
		}
		else if (parse_com == LS) {
		  parse_com = LESS;
		  operator_tmp = P_SELECT_LS;
		}
		else {
		  parse_com = EQUAL;
		  operator_tmp = P_SELECT_EQ;
		}

	      }

	      else {

		if (parse_com == GR) {
		  parse_com = GEQ;
		  operator_tmp = P_SELECT_GEQ;
		}
		else {
		  parse_com = LEQ;
		  operator_tmp = P_SELECT_LEQ;
		}

		fscanf (command_file, "%d", &parse_com_tmp);
		/*printf("%d\n",parse_com_tmp); */

	      }

	      fscanf(token_file, "%s", &parse_nam[0]);
	      /* seting the right logical plan */

	      if (step_sp > 0) {

		p3_command = p1_command;

		POP_STEP();
		step_sp--;
		p1_command = p_step;

		while (step_sp > 0 && step_ty == STRUCT_OR) {

		  POP_STEP();
		  step_sp--;

		  com_cnt++;
		  p_command++;

		  p_command->number = com_cnt;
		  p_command->operator = P_UNION;
		  p_command->left = p_step;
		  p_command->right = p1_command;
		  strcpy(p_command->argument, "");

		  if ( log_file ) fprintf(log_file, "R%d := R%d P_UNION R%d;\n",  com_cnt, p_step->number, p1_command->number);

		  p1_command = p_command;

		}

	      }

	      p2_command = p_command;

	      com_cnt++;
	      p_command++;

	      p_command->number = com_cnt;
	      p_command->operator = operator_tmp;
	      p_command->left = p1_command;
	      p_command->right = NULL;
	      strcpy(p_command->argument, parse_nam);

	      switch(parse_com) {

	      case GREATER:

		if ( log_file ) fprintf(log_file, "R%d := P_SELECT_GT(R%d,%s);\n",  com_cnt, p1_command->number, p_command->argument);
		break;

	      case LESS:

		if ( log_file ) fprintf(log_file, "R%d := P_SELECT_LT(R%d,%s);\n",  com_cnt, p1_command->number, p_command->argument);
		break;

	      case EQUAL:

		if ( log_file ) fprintf(log_file, "R%d := P_SELECT_EQ(R%d,%s);\n",  com_cnt, p1_command->number, p_command->argument);
		break;


	      case GEQ:

		if ( log_file ) fprintf(log_file, "R%d := P_SELECT_GEQ(R%d,%s);\n",  com_cnt, p1_command->number, p_command->argument);
		break;

	      case LEQ:

		if ( log_file ) fprintf(log_file, "R%d := P_SELECT_LEQ(R%d,%s);\n",  com_cnt, p1_command->number, p_command->argument);
		break;

	      }

	      if ( mil_file ) fprintf(mil_file, "\tR%d := R%d.near_val(%d,%s);\n",  com_cnt, p2_command->number, parse_com, p_command->argument);

	      if (first_step == FALSE) {
		if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p2_command->number);
	      } else
		first_step = FALSE;


	      p1_command = p_command;
	      p2_command = p_command;

	      while (step_sp > 0) {

		POP_STEP();
		step_sp--;

		if (step_ty == STAR && step_ty == DSC) {

		  com_cnt++;
		  p_command++;

		  p_command->number = com_cnt;
		  p_command->operator = P_CONTAINING;
		  p_command->left = p_step;
		  p_command->right = p1_command;
		  strcpy(p_command->argument, "");

		  if ( log_file ) fprintf(log_file, "R%d := R%d P_CONTAINING R%d;\n",  com_cnt, p_step->number, p1_command->number);

		}

		else {

		  p3_command = p_step;

		  while (step_sp > 0 && step_ty == STRUCT_OR) {

		    POP_STEP();
		    step_sp--;

		    com_cnt++;
		    p_command++;

		    p_command->number = com_cnt;
		    p_command->operator = P_UNION;
		    p_command->left = p_step;
		    p_command->right = p3_command;
		    strcpy(p_command->argument, "");

		    if ( log_file ) fprintf(log_file, "R%d := R%d P_UNION R%d;\n",  com_cnt, p_step->number, p3_command->number);

		    p3_command = p_command;

		  }

		  com_cnt++;
		  p_command++;

		  p_command->number = com_cnt;
		  p_command->operator = P_CONTAINING;
		  p_command->left = p3_command;
		  p_command->right = p1_command;
		  strcpy(p_command->argument, "");

		  if ( log_file ) fprintf(log_file, "R%d := R%d P_CONTAINING R%d;\n",  com_cnt, p3_command->number, p1_command->number);

		}

		p1_command = p_command;

	      }

	    }

	    if (in_step == TRUE) {

	      com_cnt++;
	      p_command++;

	      p_command->number = com_cnt;
	      p_command->operator = P_CONTAINING;
	      p_command->left = p_ctx;
	      p_command->right = p1_command;
	      strcpy(p_command->argument, "");

	      if ( log_file ) fprintf(log_file, "R%d := R%d P_CONTAINING R%d;\n",  com_cnt, p_ctx->number, p1_command->number);

	      in_step = FALSE;
	      if ( mil_file ) fprintf(mil_file, "\tR%d := exit_XE(R%d,R%d);\n",  com_cnt, p_ctx->number, p2_command->number);

	      if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p2_command->number);

	      p1_command = p_command;

	    }

	    fscanf (command_file, "%d", &parse_com);
	    /*printf("%d\n",parse_com); */

	  }



	  else if (parse_com == ABOUT) {

	    /* NEXI about predicate search */
 
	    char node_about[100];
	    char tpi_about[200];

	    fscanf(token_file, "%s", &parse_nam[0]);

	    step_sp = 0;
	    p_step = NULL;
	    in_step = FALSE;

	    /* OB */
	    fscanf (command_file, "%d", &parse_com);
	    /*printf("%d\n",parse_com); */
	    /* CURRENT */
	    fscanf (command_file, "%d", &parse_com);
	    /*printf("%d\n",parse_com); */
	    /* DSC or COMMA */
	    fscanf (command_file, "%d", &parse_com);
	    /*printf("%d\n",parse_com); */

	    
	    /* resolving steps in the about predicate */
	    if (parse_com == DSC) {

	      in_step = TRUE;

	      if ( mil_file ) fprintf(mil_file, "\n\tR%d := init_XE(R%d);\n",  p_ctx->number, p_ctx->number);

	      fscanf (command_file, "%d", &parse_com);
	      /*printf("%d\n", parse_com); */

	      /* resets the default value */
	      p1_command = p_ctx;

	      while (parse_com != COMMA) {

		if (parse_com == STAR) {

		  com_cnt++;
		  p_command++;

		  p_command->number = com_cnt;
		  p_command->operator = CONTAINED_BY;
		  p_command->left = NULL;
		  p_command->right = p1_command;
		  strcpy(p_command->argument, "C");

		  if ( log_file ) fprintf(log_file, "R%d := C CONTAINED_BY R%d;\n", com_cnt, p1_command->number);

		  step_sp++;
		  PUSH_STEP(STAR,p_command);

		  if ( mil_file ) fprintf(mil_file, "\tR%d := R%d.p_descendant();\n",  com_cnt, p1_command->number);

		  if (first_step == FALSE) {
		    if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p1_command->number);
		  } else
		    first_step = FALSE;

		}

		else if (parse_com == OB) {

		  p3_command = p_command;

		  fscanf(command_file, "%d", &parse_com);
		  /*printf("STRUCT_OR:%d\n", parse_com); */
		  fscanf(token_file, "%s", &parse_nam[0]);
		  com_cnt++;
		  p_command++;

		  p_command->number = com_cnt;
		  p_command->operator = SELECT_NODE;
		  p_command->left = NULL;
		  p_command->right = NULL;
		  strcpy(p_command->argument, parse_nam);

		  if ( log_file ) fprintf(log_file, "R%d := SELECT_NODE(%s);\n", com_cnt, parse_nam);
		  if ( mil_file ) fprintf(mil_file, "\tR%d := p_qname(%s,ENTITY_PN_TYPE);\n",  com_cnt, parse_nam);

		  p1_command = p_command;

		  step_sp++;
		  PUSH_STEP(DSC,p_command);

		  fscanf(command_file, "%d", &parse_com);
		  /*printf("%d\n", parse_com); */

		  while (parse_com != CB) {

		    fscanf(command_file, "%d", &parse_com);
		    /*printf("STRUCT_OR:%d\n", parse_com); */
		    fscanf(token_file, "%s", &parse_nam[0]);
		    com_cnt++;
		    p_command = p_command + 1;

		    p_command->number = com_cnt;
		    p_command->operator = SELECT_NODE;
		    p_command->left = NULL;
		    p_command->right = NULL;
		    strcpy(p_command->argument, parse_nam);

		    if ( log_file ) fprintf(log_file, "R%d := SELECT_NODE(%s);\n", com_cnt, parse_nam);
		    if ( mil_file ) fprintf(mil_file, "\tR%d := p_qname(%s,ENTITY_PN_TYPE);\n",  com_cnt, parse_nam);

		    p2_command = p_command;

		    step_sp++;
		    PUSH_STEP(STRUCT_OR, p_command);

		    if ( mil_file ) fprintf(mil_file, "\tR%d := R%d.p_union(R%d);\n",  com_cnt, p1_command->number, p2_command->number);

		    if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p1_command->number);

		    p1_command = p_command;

		    fscanf(command_file, "%d", &parse_com);
		    /*printf("x:%d\n", parse_com); */

		  }

		  if ( mil_file ) fprintf(mil_file, "\tR%d := R%d.p_descendant(R%d);\n",  com_cnt, p3_command->number, p1_command->number);

		  if (first_step == FALSE) {
		    if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p3_command->number);
		  } else
		    first_step = FALSE;

		}

		else if (parse_com == VAGUE) {

		  fscanf(command_file, "%d", &parse_com);
		  fscanf(token_file, "%s", &parse_nam[0]);
		  com_cnt++;
		  p_command++;

		  p_command->number = com_cnt;
		  p_command->operator = SELECT_NODE_VAGUE;
		  p_command->left = NULL;
		  p_command->right = NULL;
		  strcpy(p_command->argument, parse_nam);

		  if ( log_file ) fprintf(log_file, "R%d := SELECT_NODE_VAGUE(%s);\n", com_cnt, parse_nam);
		  fprintf(mil_file, "\tR%d := p_qname(%s,ENTITY_PN_TYPE);\n",  com_cnt, parse_nam);

		  step_sp++;
		  PUSH_STEP(DSC,p_command);

		  if ( mil_file ) fprintf(mil_file, "\tR%d := R%d.p_descendant(R%d);\n",  com_cnt, p1_command->number, p_command->number);

		  if (first_step == FALSE) {
		    if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p1_command->number);
		  } else
		    first_step = FALSE;

		}

		else if (parse_com == SELECT_NODE) {

		  fscanf(token_file, "%s", &parse_nam[0]);
		  com_cnt++;
		  p_command++;

		  p_command->number = com_cnt;
		  p_command->operator = SELECT_NODE;
		  p_command->left = NULL;
		  p_command->right = NULL;
		  strcpy(p_command->argument, parse_nam);

		  if ( log_file ) fprintf(log_file, "R%d := SELECT_NODE(%s);\n", com_cnt, parse_nam);
		  if ( mil_file ) fprintf(mil_file, "\tR%d := p_qname(%s,ENTITY_PN_TYPE);\n",  com_cnt, parse_nam);

		  step_sp++;
		  PUSH_STEP(DSC,p_command);

		  if ( mil_file ) fprintf(mil_file, "\tR%d := R%d.p_descendant(R%d);\n",  com_cnt, p1_command->number, p_command->number);

		  if (first_step == FALSE) {
		    if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p1_command->number);
		  } else
		    first_step = FALSE;

		}

		p1_command = p_command;

		fscanf(command_file, "%d", &parse_com);
		/*printf("x:%d\n", parse_com); */

	      }

	      p_ctx_in = p1_command;

	    }

	    else {


	      p1_command = p_command;
	      p_ctx_in = p_ctx;

	    }

	    fscanf (command_file, "%d", &parse_com);
	    /*printf("%d\n",parse_com); */


	    /* storing terms and phrases */

	    if (alg_type == ASPECT) {
	      
	      num_term = 0;
	      num_phrase = 0;
	      plus_term =0;
	      minus_term =0;
	      prefix = NORMAL;
	      
	      term_sp =0;
	      phrase_sp =0;
	    
	      while (parse_com != CB) {

		if (parse_com == PLUS) {
		  plus_term++;
		  prefix = PLUS;
		  fscanf (command_file, "%d", &parse_com);
		  /*printf("%d\n",parse_com); */
		}
		else if (parse_com == MINUS) {
		  prefix = MINUS;
		  minus_term++;
		  fscanf (command_file, "%d", &parse_com);
		  /*printf("%d\n",parse_com); */
		}
		else if (parse_com == MUST) {
		  prefix = MUST;
		  fscanf (command_file, "%d", &parse_com);
		  /*printf("%d\n",parse_com); */
		}
		else if (parse_com == MUST_NOT) {
		  prefix = MUST_NOT;
		  fscanf (command_file, "%d", &parse_com);
		  /*printf("%d\n",parse_com); */
		}
		else {
		  prefix = NORMAL;
		}
		
		if (parse_com != QUOTE && parse_com != IMAGE_ABOUT) {
		  
		  num_term++;
		  fscanf(token_file, "%s", &parse_nam[0]);

		  term_sp++;
		  PUSH_TERM(parse_nam, prefix);

		  fscanf (command_file, "%d", &parse_com);
		  /*printf("%d\n",parse_com); */
		  
		}
		
		else if (parse_com == QUOTE) {
		  
		
		  num_phrase++;
		  
		  fscanf (command_file, "%d", &parse_com);
		  /*printf("%d\n",parse_com); */
		  
		  while (parse_com != QUOTE) {
		    		    
		    if (prefix == PLUS)
		      plus_term++;
		    else if (prefix == MINUS)
		      minus_term++;
		    
		    num_term++;
		    fscanf(token_file, "%s", &parse_nam[0]);
		    
		    term_sp++;
		    phrase_sp++;
		    
		    PUSH_TERM(parse_nam, prefix);
		    PUSH_PHRASE(parse_nam, prefix);
		    
		    fscanf (command_file, "%d", &parse_com);
		    /*printf("%d\n",parse_com); */

		  }

		  phrase_sp++;
		  PUSH_PHRASE("", END_PHRASE);
		  
		  fscanf (command_file, "%d", &parse_com);
		  /*printf("%d\n",parse_com); */
		  
		  
		}
		
		else if (parse_com == IMAGE_ABOUT) {
		  
		  num_term++;
		  fscanf(token_file, "%s", &parse_nam[0]);
		  
		  term_sp++;
		  PUSH_TERM(parse_nam, IMAGE_ABOUT+prefix);
		  
		  fscanf (command_file, "%d", &parse_com);
		  /*printf("%d\n",parse_com); */
		  
		}
		
	      }

	    }

  	    else if (alg_type == COARSE) {

	      strcpy(tpi_about,"|");

	      while (parse_com != CB) {

		if (parse_com == PLUS || parse_com == MUST) {
		  strcat(tpi_about,"+");
		  fscanf (command_file, "%d", &parse_com);
		  /*printf("%d\n",parse_com); */
		}
		else if (parse_com == MINUS || parse_com == MUST_NOT) {
		  strcat(tpi_about,"-");
		  fscanf (command_file, "%d", &parse_com);
		  /*printf("%d\n",parse_com); */
		}

		
		if (parse_com != QUOTE && parse_com != IMAGE_ABOUT) {

		  fscanf(token_file, "%s", &parse_nam[0]);
		  strcat(tpi_about,parse_nam);
		  strcat(tpi_about,"|");
		  fscanf (command_file, "%d", &parse_com);
		  /*printf("%d\n",parse_com); */
		  
		}
		
		else if (parse_com == QUOTE) {
		  
		  strcat(tpi_about,"\"");
		  fscanf (command_file, "%d", &parse_com);
		  /*printf("%d\n",parse_com); */
		  
		  while (parse_com != QUOTE) {

		    fscanf(token_file, "%s", &parse_nam[0]);
		    strcat(tpi_about,parse_nam);
		    strcat(tpi_about," ");
		    fscanf (command_file, "%d", &parse_com);
		    /*printf("%d\n",parse_com); */

		  }
		  
		  strcat(tpi_about,"\"|");
		  fscanf (command_file, "%d", &parse_com);
		  /*printf("%d\n",parse_com); */
		  
		}
		
		else if (parse_com == IMAGE_ABOUT) {

		  fscanf(token_file, "%s", &parse_nam[0]);
		  strcat(tpi_about,"IMG:");
		  strcat(tpi_about,parse_nam);
		  strcat(tpi_about,"|");
		  fscanf (command_file, "%d", &parse_com);
		  /*printf("%d\n",parse_com); */
		  
		}
		
	      }

	      /*printf("%s\n",tpi_about); */

	    }



	    /* for prob_LM */
	    if (in_step == TRUE)
	      p2_command = p_command;
	    else
	      p2_command = p_ctx;


	    if (step_sp > 0) {

	      p3_command = p1_command;

	      POP_STEP();
	      step_sp--;
	      p1_command = p_step;

	      if (alg_type == ASPECT) {

		while (step_sp > 0 && step_ty == STRUCT_OR) {

		  POP_STEP();
		  step_sp--;
		  
		  com_cnt++;
		  p_command++;
		  
		  p_command->number = com_cnt;
		  p_command->operator = P_UNION;
		  p_command->left = p_step;
		  p_command->right = p1_command;
		  strcpy(p_command->argument, "");
		  
		  if ( log_file ) fprintf(log_file, "R%d := R%d P_UNION R%d;\n",  com_cnt, p_step->number, p1_command->number);

		  p1_command = p_command;

		}

		p_ctx_in = p1_command;

	      }

	      else if (alg_type == COARSE) {

		strcpy(node_about,"|");

		if (strcmp("C",p_step->argument)) {
		  if (p_step->operator == SELECT_NODE_VAGUE)
		    strcat(node_about,"~");
		  strcat(node_about,p_step->argument);
		}
		else
		  strcat(node_about,"\"*\"");

		while (step_sp > 0 && step_ty == STRUCT_OR) {

		  POP_STEP();
		  step_sp--;
		  com_cnt--;
		  
		  strcat(node_about,"|");
		  strcat(node_about,p_step->argument);
		  

		}

		strcat(node_about,"|");

		com_cnt++;
		p_command = p_step;
		
		p_command->number = com_cnt;
		p_command->operator = P_SELECT_NODE_T;
		p_command->left = NULL;
		p_command->right = NULL;
		strcpy(p_command->argument, node_about);
		strcat(p_command->argument, ",");
		strcat(p_command->argument, tpi_about);
		  
		if ( log_file ) fprintf(log_file, "R%d := P_SELECT_NODE_T(%s);\n",  com_cnt, p_command->argument);
		
		p1_command = p_command;

	      }

	    }



	    /* Resolving terms and phrases */

	    /* forming term expression */


	    if (alg_type == ASPECT) {

	      /* normal term treatment */
	      
	      command_tree *p_term;

	      float normal_term, extend_term;
	      char normal_term_s[10], extend_term_s[10];
	      p_term = NULL;
	      
	      if (scale_on == TRUE) {
		normal_term = 1.0/((float)(num_term + plus_term - minus_term));
		/*printf("nt:%f\n",normal_term); */
		extend_term = 2.0 * normal_term;
		/*printf("nt:%f\n",extend_term); */
	      }
	      else {
		normal_term = 1.0;
		/*printf("nt:%f\n",normal_term); */
		extend_term = 1.0;
		/*printf("nt:%f\n",extend_term); */
	      }
	      
	      sprintf(normal_term_s, "%f", normal_term);
	      sprintf(extend_term_s, "%f", extend_term);
	      
	      term_sp = 0;
	      
	      while (term_sp < num_term) {
	      
		term_sp++;
		/*	    printf("%d\n",term_sp); */
		POP_TERM();
		
		/*printf("%s\n",t); */
		
		if (s == NORMAL || s == PLUS || s == MINUS || s == MUST || s == MUST_NOT) {
		  
		  com_cnt++;
		  p_command++;
		  
		  p_command->number = com_cnt;
		  p_command->operator = SELECT_TERM;
		  p_command->left = NULL;
		  p_command->right = NULL;
		  strcpy(p_command->argument, t);
		  
		  if ( log_file ) fprintf(log_file, "R%d := SELECT_TERM(%s);\n",  com_cnt, t);
		  
		  
		  if (s == NORMAL || s == PLUS) {
		    
		    p1_command = p_command;
		    
		    com_cnt++;
		    p_command++;
		    
		    p_command->number = com_cnt;
		    p_command->operator = P_CONTAINING_T;
		    p_command->left = p_ctx_in;
		    p_command->right = p1_command;
		    strcpy(p_command->argument, "");
		    
		    if ( log_file ) fprintf(log_file, "R%d := R%d P_CONTAINING_T R%d;\n",  com_cnt, p_ctx_in->number, p1_command->number);
		    if ( mil_file ) fprintf(mil_file, "\tR%d := prob_LM(R%d,%s,lambda);\n",  com_cnt, p2_command->number, t);
		    
		    if (s == PLUS) {

		      p1_command = p_command;

		      com_cnt++;
		      p_command++;

		      p_command->number = com_cnt;
		      p_command->operator = SCALE;
		      p_command->left = p1_command;
		      p_command->right = NULL;
		      strcpy(p_command->argument, extend_term_s);

		      if ( log_file ) fprintf(log_file, "R%d := R%d SCALE %s;\n",  com_cnt, p1_command->number, p_command->argument);
		      if ( mil_file ) fprintf(mil_file, "\tR%d := R%d.scale(%f);\n",  com_cnt, p1_command->number, extend_term);

		      if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p1_command->number);

		    }

		    else {

		      p1_command = p_command;

		      com_cnt++;
		      p_command++;
		    
		      p_command->number = com_cnt;
		      p_command->operator = SCALE;
		      p_command->left = p1_command;
		      p_command->right = NULL;
		      strcpy(p_command->argument, normal_term_s);
		      
		      if ( log_file ) fprintf(log_file, "R%d := R%d SCALE %s;\n",  com_cnt, p1_command->number, p_command->argument);
		      if ( mil_file ) fprintf(mil_file, "\tR%d := R%d.scale(%f);\n",  com_cnt, p1_command->number, normal_term);
		      
		      if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p1_command->number);
		      
		    }
		    
		  }
		  
		  else if (s == MINUS) {
		    
		    p1_command = p_command;
		    
		    com_cnt++;
		    p_command++;
		    
		    p_command->number = com_cnt;
		    p_command->operator = P_NOT_CONTAINING_T;
		    p_command->left = p_ctx_in;
		    p_command->right = p1_command;
		    strcpy(p_command->argument, "");
		    
		    if ( log_file ) fprintf(log_file, "R%d := R%d P_NOT_CONTAINING_T R%d;\n",  com_cnt, p_ctx_in->number, p1_command->number);
		    if ( mil_file ) fprintf(mil_file, "\tR%d := soft_not(R%d,%s,lambda);\n",  com_cnt, p_ctx_in->number,t);
		    
		  }
		  
		  else if (s == MUST || s == MUST_NOT) {
		    
		    if (s == MUST) {

		      p1_command = p_command;
		      
		      com_cnt++;
		      p_command++;
		      
		      p_command->number = com_cnt;
		      p_command->operator = MUST_CONTAIN_T;
		      p_command->left = p_ctx_in;
		      p_command->right = p1_command;
		      strcpy(p_command->argument, "");
		      
		      if ( log_file ) fprintf(log_file, "R%d := R%d MUST_CONTAIN_T R%d;\n",  com_cnt, p_ctx_in->number, p1_command->number);
		      if ( mil_file ) fprintf(mil_file, "\tR%d := must(R%d,%s);\n",  com_cnt, p2_command->number, t);
		    
		    }
		    
		    else {
		      
		      p1_command = p_command;
		      
		      com_cnt++;
		      p_command++;
		      
		      p_command->number = com_cnt;
		      p_command->operator = MUST_NOT_CONTAIN_T;
		      p_command->left = p_ctx_in;
		      p_command->right = p1_command;
		      strcpy(p_command->argument, "");
		      
		      if ( log_file ) fprintf(log_file, "R%d := R%d MUST_NOT_CONTAIN_T R%d;\n",  com_cnt, p_ctx_in->number, p1_command->number);
		      if ( mil_file ) fprintf(mil_file, "\tR%d := must_not(R%d,%s);\n",  com_cnt, p2_command->number, t);
		      
		    }
		  
		    p1_command = p_command;
		    
		    com_cnt++;
		    p_command++;
		    
		    p_command->number = com_cnt;
		    p_command->operator = SCALE;
		    p_command->left = p1_command;
		    p_command->right = NULL;
		    strcpy(p_command->argument, normal_term_s);
		    
		    if ( log_file ) fprintf(log_file, "R%d := R%d SCALE %s;\n",  com_cnt, p1_command->number, p_command->argument);
		    if ( mil_file ) fprintf(mil_file, "\tR%d := R%d.scale(%f);\n",  com_cnt, p1_command->number, normal_term);
		  
		    if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p1_command->number);
		  
		  }
		  
		}
	      
		else if (s == IMAGE_ABOUT+NORMAL || s == IMAGE_ABOUT+PLUS || s == IMAGE_ABOUT+MINUS || s == IMAGE_ABOUT+MUST || s == IMAGE_ABOUT+MUST_NOT) {

		  com_cnt++;
		  p_command++;
		  
		  p_command->number = com_cnt;
		  p_command->operator = P_SELECT_IMAGE;
		  p_command->left = NULL;
		  p_command->right = NULL;
		  strcpy(p_command->argument, parse_nam);
		  
		  if ( log_file ) fprintf(log_file, "R%d := P_SELECT_IMAGE(%s);\n",  com_cnt, t);
		  
		  if (s == IMAGE_ABOUT+NORMAL || s == IMAGE_ABOUT+PLUS || s == IMAGE_ABOUT+MUST) {
		    
		    p1_command = p_command;
		    
		    com_cnt++;
		    p_command++;
		    
		    p_command->number = com_cnt;
		    p_command->operator = P_CONTAINING_I;
		    p_command->left = p_ctx_in;
		    p_command->right = p1_command;
		    strcpy(p_command->argument, "");
		    
		    if ( log_file ) fprintf(log_file, "R%d := R%d P_CONTAINING_I R%d;\n",  com_cnt, p_ctx_in->number, p1_command->number);
		    
		    if (s == IMAGE_ABOUT+PLUS) {
		      
		      p1_command = p_command;
		      
		      com_cnt++;
		      p_command++;
		      
		      p_command->number = com_cnt;
		      p_command->operator = SCALE;
		      p_command->left = p1_command;
		      p_command->right = NULL;
		      strcpy(p_command->argument, extend_term_s);
		      
		      if ( log_file ) fprintf(log_file, "R%d := R%d SCALE %s;\n",  com_cnt, p1_command->number, p_command->argument);
		      if ( mil_file ) fprintf(mil_file, "\tR%d := R%d.scale(%f);\n",  com_cnt, p1_command->number, extend_term);
		      
		      if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p1_command->number);
		      
		    }
		    
		    else {
		      
		      p1_command = p_command;
		      
		      com_cnt++;
		      p_command++;
		      
		      p_command->number = com_cnt;
		      p_command->operator = SCALE;
		      p_command->left = p1_command;
		      p_command->right = NULL;
		      strcpy(p_command->argument, normal_term_s);
		      
		      if ( log_file ) fprintf(log_file, "R%d := R%d SCALE %s;\n",  com_cnt, p1_command->number, p_command->argument);
		      if ( mil_file ) fprintf(mil_file, "\tR%d := R%d.scale(%f);\n",  com_cnt, p1_command->number, normal_term);
		      
		      if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p1_command->number);
		      
		    }

		  }

		  else if (s == IMAGE_ABOUT+MINUS || s == IMAGE_ABOUT+MUST_NOT) {

		    p1_command = p_command;

		    com_cnt++;
		    p_command++;
		    
		    p_command->number = com_cnt;
		    p_command->operator = P_NOT_CONTAINING_I;
		    p_command->left = p_ctx_in;
		    p_command->right = p1_command;
		    strcpy(p_command->argument, "");
		    
		    if ( log_file ) fprintf(log_file, "R%d := R%d P_NOT_CONTAINING_I R%d;\n",  com_cnt, p_ctx_in->number, p1_command->number);
		    
		  }
		  
		}
		
		if (term_sp > 1 && p_term != NULL) {
		  
		  p1_command = p_command;
		  
		  com_cnt++;
		  p_command++;
		  
		  p_command->number = com_cnt;
		  p_command->operator = P_AND;
		  p_command->left = p_term;
		  p_command->right = p1_command;
		  strcpy(p_command->argument, "");
		  
		  if ( log_file ) fprintf(log_file, "R%d := R%d P_AND R%d;\n",  com_cnt, p_term->number, p1_command->number);
		  if ( mil_file ) fprintf(mil_file, "\tR%d := R%d.and(R%d);\n",  com_cnt, p_term->number, p1_command->number);
		  
		  if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p_term->number);
		  if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p1_command->number);
		  
		}

		p_term = p_command;

	      }

	      /* forming phrase expressions */

	      /*printf("%d\n",num_phrase); */

	      if (num_phrase >0) {

		int ph_count = 0;
		command ph_sign = EMPTY;

		char phrase[100];

		phrase_sp = 0;

		while (ph_count < num_phrase) {

		  int t_count = 0;

		  if ( mil_file ) fprintf(mil_file, "\tphrase := new(int,str);\n");
		  strcpy(phrase,"|");

		  phrase_sp++;
		  ph_count++;

		  POP_PHRASE();
		  ph_sign = s;
		
		  while (s != END_PHRASE) {

		    t_count++;

		    strcat(phrase,p);
		    strcat(phrase,"|");

		    if ( mil_file ) fprintf(mil_file, "\tphrase.insert(%d, %s);\n", t_count, p);

		    phrase_sp++;
		    POP_PHRASE();

		  }

		  p1_command = p_command;
		  p_adjcommand = p_command;

		  com_cnt++;
		  p_command++;

		  p_command->number = com_cnt;
		  p_command->operator = SELECT_ADJ;
		  p_command->left = NULL;
		  p_command->right = NULL;
		  strcpy(p_command->argument, phrase);
		  
		  if ( log_file ) fprintf(log_file, "R%d := SELECT_ADJ(%s);\n",  com_cnt, phrase);
		  p1_command = p_command;
		  
		  com_cnt++;
		  p_command++;
		  
		  if (ph_sign != MINUS) {
		    
		    p_command->number = com_cnt;
		    p_command->operator = P_ADJ;
		    p_command->left = p_ctx_in;
		    p_command->right = p1_command;
		    strcpy(p_command->argument, "");
		    
		    if ( log_file ) fprintf(log_file, "R%d := R%d P_ADJ R%d;\n",  com_cnt, p_ctx_in->number, p1_command->number);
		    if ( mil_file ) fprintf(mil_file, "\tR%d := R%d.adj_term(phrase,ord,eta);\n",  com_cnt, p2_command->number);
		    
		  }
		  
		  else {
		    
		    p_command->number = com_cnt;
		    p_command->operator = P_ADJ_NOT;
		    p_command->left = p_ctx_in;
		    p_command->right = p1_command;
		    strcpy(p_command->argument, "");
		    
		    if ( log_file ) fprintf(log_file, "R%d := R%d P_ADJ_NOT R%d;\n",  com_cnt, p_ctx_in->number, p1_command->number);
		    if ( mil_file ) fprintf(mil_file, "\tR%d := R%d.adj_term_not(phrase,ord,eta);\n",  com_cnt, p2_command->number);
		    
		  }
		  
		  p3_command = p_command;
		  
		  com_cnt++;
		  p_command++;
		  
		  p_command->number = com_cnt;
		  p_command->operator = P_AND;
		  p_command->left = p_adjcommand;
		  p_command->right = p3_command;
		  strcpy(p_command->argument, "");

		  if ( log_file ) fprintf(log_file, "R%d := R%d P_AND R%d;\n",  com_cnt, p_adjcommand->number, p3_command->number);
		  if ( mil_file ) fprintf(mil_file, "\tR%d := R%d.and(R%d);\n",  com_cnt, p_adjcommand->number, p3_command->number);
		  
		  if ( mil_file ) fprintf(mil_file, "\tphrase := nil;\n");
		  if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p_adjcommand->number);
		  if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p3_command->number);
		  
		}
		
		p1_command = p_command;

	      }

	    }

	    else if (alg_type == COARSE) {

	      strcpy(node_about,"|");
	      
	      /*printf("%d\n",p_ctx_in->operator); */
	      /*printf("%s\n",node_about); */
	      /*printf("%s\n",tpi_about); */
	      /*printf("XXX:%d\n",p_ctx_in->operator); */

	      if ((p_ctx_in->operator == CONTAINED_BY || p_ctx_in->operator == P_CONTAINED_BY) && (p_ctx_in->left == NULL || (ctx_in_node == TRUE && !strcmp(node_tmp,"*")))) {
		
		if (ctx_in_node == FALSE) {
		  ctx_in_node = TRUE;
		  strcpy(node_tmp,"*");
		
		  p1_command = p_command;
		  
		  com_cnt++;
		  p_command++;
		  
		  strcat(node_about,"\"*\"");
		  strcat(node_about,"|");
		  
		  p_command->number = com_cnt;
		  p_command->operator = P_SELECT_NODE_T;
		  p_command->left = NULL;
		  p_command->right = NULL;
		  strcpy(p_command->argument, node_about);
		  strcat(p_command->argument, ",");
		  strcat(p_command->argument, tpi_about);
		  
		  if ( log_file ) fprintf(log_file, "R%d := P_SELECT_NODE_T(%s);\n",  p_command->number, p_command->argument);


		  p1_command = p_command;

		  com_cnt++;
		  p_command++;

		  p_command->number = com_cnt;
		  p_command->operator = p_ctx_in->operator;
		  p_command->left = p1_command;
		  p_command->right = p_ctx_in->right;
		  strcpy(p_command->argument, "");

		  if (p_ctx_in->operator == CONTAINED_BY) {
		    if ( log_file ) fprintf(log_file, "R%d := R%d CONTAINED_BY R%d;\n",  p_command->number, p_command->left->number, p_ctx_in->right->number);
		  }
		  else {
		    if ( log_file ) fprintf(log_file, "R%d := R%d P_CONTAINED_BY R%d;\n",  p_command->number, p_command->left->number, p_ctx_in->right->number);
		  }

		
		}

		else {

		  p1_command = p_command;
		  
		  com_cnt++;
		  p_command++;
		  
		  strcat(node_about,"\"*\"");
		  strcat(node_about,"|");
		  
		  p_command->number = com_cnt;
		  p_command->operator = P_SELECT_NODE_T;
		  p_command->left = NULL;
		  p_command->right = NULL;
		  strcpy(p_command->argument, node_about);
		  strcat(p_command->argument, ",");
		  strcat(p_command->argument, tpi_about);
		  
		  if ( log_file ) fprintf(log_file, "R%d := P_SELECT_NODE_T(%s);\n",  p_command->number, p_command->argument);

		  p1_command = p_command;

		  com_cnt++;
		  p_command++;

		  p_command->number = com_cnt;
		  p_command->operator = p_ctx_in->operator;
		  p_command->left = p1_command;
		  p_command->right = p_ctx_in->right;
		  strcpy(p_command->argument, "");

		  if (p_ctx_in->operator == CONTAINED_BY) {
		    if ( log_file ) fprintf(log_file, "R%d := R%d CONTAINED_BY R%d;\n",  p_command->number, p_command->left->number, p_ctx_in->right->number);
		  }
		  else {
		    if ( log_file ) fprintf(log_file, "R%d := R%d P_CONTAINED_BY R%d;\n",  p_command->number, p_command->left->number, p_ctx_in->right->number);
		  }

		}
	  
	      }

	      else if ((p_ctx_in->operator == CONTAINED_BY || p_ctx_in->operator == P_CONTAINED_BY) && p_ctx_in->left->operator != P_OR && p_ctx_in->left->operator != UNION) {

		if (ctx_in_node == FALSE) {

		  ctx_in_node = TRUE;

		  if (p_ctx_in->left->operator == SELECT_NODE_VAGUE) {
		    strcpy(node_tmp,"~");
		    strcat(node_tmp,p_ctx_in->left->argument);
		  }
		  else {
		    strcpy(node_tmp,p_ctx_in->left->argument);
		  }
		
		  strcat(node_about,node_tmp);
		  strcat(node_about,"|");

		  p1_command = p_command;

		  com_cnt++;
		  p_command++;

		  p_command->number = com_cnt;
		  p_command->operator = P_SELECT_NODE_T;
		  p_command->left = NULL;
		  p_command->right = NULL;
		  strcpy(p_command->argument, node_about);
		  strcat(p_command->argument, ",");
		  strcat(p_command->argument, tpi_about);

		  if ( log_file ) fprintf(log_file, "R%d := P_SELECT_NODE_T(%s);\n",  p_command->number, p_command->argument);


		  p1_command = p_command;

		  com_cnt++;
		  p_command++;

		  p_command->number = com_cnt;
		  p_command->operator = p_ctx_in->operator;
		  p_command->left = p1_command;
		  p_command->right = p_ctx_in->right;
		  strcpy(p_command->argument, "");

		  if (p_ctx_in->operator == CONTAINED_BY) {
		    if ( log_file ) fprintf(log_file, "R%d := R%d CONTAINED_BY R%d;\n",  p_command->number, p1_command->number, p_ctx_in->right->number);
		  }
		  else {
		    if ( log_file ) fprintf(log_file, "R%d := R%d P_CONTAINED_BY R%d;\n",  p_command->number, p1_command->number, p_ctx_in->right->number);
		  }


		}

		else {

		  p1_command = p_command;

		  strcat(node_about,node_tmp);
		  strcat(node_about,"|");

		  com_cnt++;
		  p_command++;

		  p_command->number = com_cnt;
		  p_command->operator = P_SELECT_NODE_T;
		  p_command->left = NULL;
		  p_command->right = NULL;
		  strcpy(p_command->argument, node_about);
		  strcat(p_command->argument, ",");
		  strcat(p_command->argument, tpi_about);

		  if ( log_file ) fprintf(log_file, "R%d := P_SELECT_NODE_T(%s);\n",  p_command->number, p_command->argument);


		  p1_command = p_command;

		  com_cnt++;
		  p_command++;

		  p_command->number = com_cnt;
		  p_command->operator = p_ctx_in->operator;
		  p_command->left = p1_command;
		  p_command->right = p_ctx_in->right;
		  strcpy(p_command->argument, "");
		  

		  if (p_ctx_in->operator == CONTAINED_BY) {
		    if ( log_file ) fprintf(log_file, "R%d := R%d CONTAINED_BY R%d;\n",  p_command->number, p1_command->number, p_ctx_in->right->number);
		  }
		  else {
		    if ( log_file ) fprintf(log_file, "R%d := R%d P_CONTAINED_BY R%d;\n",  p_command->number, p1_command->number, p_ctx_in->right->number);
		  }


		}

	      }

	      else if ((p_ctx_in->operator == CONTAINED_BY || p_ctx_in->operator == P_CONTAINED_BY) && (p_ctx_in->left->operator == P_OR || p_ctx_in->left->operator == UNION)) {


		if (ctx_in_node == FALSE) {

		  ctx_in_node = TRUE;

		  p1_command = p_command;
		  
		  p_command = p_ctx_in->left;

		  strcpy(node_tmp,"|");
		  strcat(node_tmp,p_command->right->argument);
		  strcat(node_tmp,"|");

		  while (p_command->left->operator == P_OR || p_command->left->operator == UNION) {

		    p_command = p_command->left;
		    strcat(node_tmp,p_command->right->argument);
		    strcat(node_tmp,"|");

		  }

		  strcat(node_tmp,p_command->left->argument);
		  strcat(node_tmp,"|");

		  p_command = p1_command;

		  com_cnt++;
		  p_command++;

		  p_command->number = com_cnt;
		  p_command->operator = P_SELECT_NODE_T;
		  p_command->left = NULL;
		  p_command->right = NULL;
		  strcpy(p_command->argument, node_tmp);
		  strcat(p_command->argument, ",");
		  strcat(p_command->argument, tpi_about);

		  if ( log_file ) fprintf(log_file, "R%d := P_SELECT_NODE_T(%s);\n",  p_command->number, p_command->argument);


		  p1_command = p_command;

		  com_cnt++;
		  p_command++;

		  p_command->number = com_cnt;
		  p_command->operator = p_ctx_in->operator;
		  p_command->left = p1_command;
		  p_command->right = p_ctx_in->right;
		  strcpy(p_command->argument, "");
		  
		  if (p_ctx_in->operator == CONTAINED_BY) {
		    if ( log_file ) fprintf(log_file, "R%d := R%d CONTAINED_BY R%d;\n",  p_command->number, p1_command->number, p_ctx_in->right->number);
		  }
		  else {
		    if ( log_file ) fprintf(log_file, "R%d := R%d P_CONTAINED_BY R%d;\n",  p_command->number, p1_command->number, p_ctx_in->right->number);
		  }


		}

		else {

		  p1_command = p_command;

		  com_cnt++;
		  p_command++;

		  p_command->number = com_cnt;
		  p_command->operator = P_SELECT_NODE_T;
		  p_command->left = NULL;
		  p_command->right = NULL;
		  strcpy(p_command->argument, node_tmp);
		  strcat(p_command->argument, ",");
		  strcat(p_command->argument, tpi_about);

		  if ( log_file ) fprintf(log_file, "R%d := P_SELECT_NODE_T(%s);\n",  p_command->number, p_command->argument);


		  p1_command = p_command;

		  com_cnt++;
		  p_command++;

		  p_command->number = com_cnt;
		  p_command->operator = p_ctx_in->operator;
		  p_command->left = p1_command;
		  p_command->right = p_ctx_in->right;
		  strcpy(p_command->argument, "");
		  

		  if (p_ctx_in->operator == CONTAINED_BY) {
		    if ( log_file ) fprintf(log_file, "R%d := R%d CONTAINED_BY R%d;\n",  p_command->number, p1_command->number, p_ctx_in->right->number);
		  }
		  else {
		    if ( log_file ) fprintf(log_file, "R%d := R%d P_CONTAINED_BY R%d;\n",  p_command->number, p1_command->number, p_ctx_in->right->number);
		  }

		}

		/*p_command = p_ctx_in; */

	      }

	    }

	    strcpy(tpi_about,"");
	    strcpy(node_about,"");

	    if (p2_command->number != p_ctx->number)
	      if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p2_command->number);
	    
	    p1_command = p_command;

	    /* for exit_XE */
	    p2_command = p_command;

	    while (step_sp > 0) {

	      POP_STEP();
	      step_sp--;

	      if (step_ty == STAR && step_ty == DSC) {

		com_cnt++;
		p_command++;

		p_command->number = com_cnt;
		p_command->operator = P_CONTAINING;
		p_command->left = p_step;
		p_command->right = p1_command;
		strcpy(p_command->argument, "");

		if ( log_file ) fprintf(log_file, "R%d := R%d P_CONTAINING R%d;\n",  com_cnt, p_step->number, p1_command->number);

	      }

	      else {

		p3_command = p_step;

		while (step_sp > 0 && step_ty == STRUCT_OR) {

		  POP_STEP();
		  step_sp--;

		  com_cnt++;
		  p_command++;

		  p_command->number = com_cnt;
		  p_command->operator = P_UNION;
		  p_command->left = p_step;
		  p_command->right = p3_command;
		  strcpy(p_command->argument, "");

		  if ( log_file ) fprintf(log_file, "R%d := R%d P_UNION R%d;\n",  com_cnt, p_step->number, p3_command->number);

		  p3_command = p_command;

		}

		com_cnt++;
		p_command++;

		p_command->number = com_cnt;
		p_command->operator = P_CONTAINING;
		p_command->left = p3_command;
		p_command->right = p1_command;
		strcpy(p_command->argument, "");

		if ( log_file ) fprintf(log_file, "R%d := R%d P_CONTAINING R%d;\n",  com_cnt, p3_command->number, p1_command->number);

	      }

	      p1_command = p_command;

	    }


	    if (in_step == TRUE) {

	      com_cnt++;
	      p_command++;

	      p_command->number = com_cnt;
	      p_command->operator = P_CONTAINING;
	      p_command->left = p_ctx;
	      p_command->right = p1_command;
	      strcpy(p_command->argument, "");

	      if ( log_file ) fprintf(log_file, "R%d := R%d P_CONTAINING R%d;\n",  com_cnt, p_ctx->number, p1_command->number);

	      in_step = FALSE;
	      if ( mil_file ) fprintf(mil_file, "\tR%d := exit_XE(R%d,R%d);\n",  com_cnt, p_ctx->number, p2_command->number);

	      if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p2_command->number);

	    }

	    p1_command = p_command;

	    fscanf (command_file, "%d", &parse_com);
	    /*printf("%d\n",parse_com); */

	  }

	  else if (parse_com == UNION || parse_com == INTERSECT) {


	    /* and and or expressions in NEXI */
	    fscanf(token_file, "%s", &parse_nam[0]);

	    /* and and or expressions inside predicate */
	    num_bracket =0;
	    br_count = 0;

	    p1_command = p_command;

	    /*printf("PC%d\n",parse_com); */

	    if (op_sp > 0) {

	      POP_OP();

	      if (op_cod != OB) {

		op_sp--;

		while (op_cod == CB) {

		  POP_OP();
		  op_sp--;
		  num_bracket++;
		  /*printf("%d\n",op_cod); */

		}


		if (op_cod != OB) {

		  com_cnt++;
		  p_command++;

		  if (op_cod == INTERSECT) {

		    p_command->number = com_cnt;
		    p_command->operator = P_AND;
		    p_command->left = p_leftop;
		    p_command->right = p1_command;
		    strcpy(p_command->argument, "");

		    if ( log_file ) fprintf(log_file, "R%d := R%d P_AND R%d;\n",  com_cnt, p_leftop->number, p1_command->number);
		    if ( mil_file ) fprintf(mil_file, "\n\tR%d := R%d.and(R%d);\n",  com_cnt, p_leftop->number, p1_command->number);

		  }

		  else {

		    p_command->number = com_cnt;
		    p_command->operator = P_OR;
		    p_command->left = p_leftop;
		    p_command->right = p1_command;
		    strcpy(p_command->argument, "");

		    if ( log_file ) fprintf(log_file, "R%d := R%d P_OR R%d;\n",  com_cnt, p_leftop->number, p1_command->number);
		    if ( mil_file ) fprintf(mil_file, "\n\tR%d := R%d.or(R%d);\n",  com_cnt, p_leftop->number, p1_command->number);

		  }

		  if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p_leftop->number);
		  if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n\n", p1_command->number);

		  for (br_count = 0; br_count < num_bracket; br_count++) {

		    POP_OP();
		    op_sp--;
		    /*printf("%d\n",op_sp); */

		  }

		}

		else {

		  for (br_count = 0; br_count < num_bracket-1; br_count++) {

		    POP_OP();
		    op_sp--;
		    /*printf("%d\n",op_sp); */

		  }

		}

		p1_command = p_command;

		if (op_sp >0) {

		  POP_OP();

		  if ( op_cod != OB) {

		    op_sp--;

		    com_cnt++;
		    p_command++;

		    if (op_cod == INTERSECT) {

		      p_command->number = com_cnt;
		      p_command->operator = P_AND;
		      p_command->left = p_leftop;
		      p_command->right = p1_command;
		      strcpy(p_command->argument, "");

		      if ( log_file ) fprintf(log_file, "R%d := R%d P_AND R%d;\n",  com_cnt, p_leftop->number, p1_command->number);
		      if ( mil_file ) fprintf(mil_file, "\n\tR%d := R%d.and(R%d);\n",  com_cnt, p_leftop->number, p1_command->number);

		    }

		    else {

		      p_command->number = com_cnt;
		      p_command->operator = P_OR;
		      p_command->left = p_leftop;
		      p_command->right = p1_command;
		      strcpy(p_command->argument, "");

		      if ( log_file ) fprintf(log_file, "R%d := R%d P_OR R%d;\n",  com_cnt, p_leftop->number, p1_command->number);
		      if ( mil_file ) fprintf(mil_file, "\n\tR%d := R%d.or(R%d);\n",  com_cnt, p_leftop->number, p1_command->number);

		    }

		    if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p_leftop->number);
		    if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n\n", p1_command->number);

		  }

		  p1_command = p_command;

		}

		op_sp++;
		PUSH_OP(parse_com,p_command);

	      }

	      else {

		op_sp++;
		PUSH_OP(parse_com,p_command);

	      }

	    }

	    else {

	      op_sp++;
	      PUSH_OP(parse_com,p_command);

	    }

	    fscanf (command_file, "%d", &parse_com);
	    /*printf("%d\n",parse_com); */

	  }

	  while (parse_com == CB) {

	    op_sp++;
	    PUSH_OP(CB,p_command);

	    fscanf (command_file, "%d", &parse_com);
	    /*printf("%d\n",parse_com); */
	  }

	}

	/* finalizing the and and or expressions if any */

	if (op_sp > 0) {

	  num_bracket =0;
	  br_count =0;

	  POP_OP();
	  op_sp--;

	  p1_command = p_command;

	  while (op_cod == CB) {

	    POP_OP();
	    op_sp--;
	    num_bracket++;
	    /*printf("num: %d\n",num_bracket); */

	  }

	  if (op_cod != OB) {

	    com_cnt++;
	    p_command++;

	    if (op_cod == INTERSECT) {

	      p_command->number = com_cnt;
	      p_command->operator = P_AND;
	      p_command->left = p_leftop;
	      p_command->right = p1_command;
	      strcpy(p_command->argument, "");

	      if ( log_file ) fprintf(log_file, "R%d := R%d P_AND R%d;\n",  com_cnt, p_leftop->number, p1_command->number);
	      if ( mil_file ) fprintf(mil_file, "\n\tR%d := R%d.and(R%d);\n",  com_cnt, p_leftop->number, p1_command->number);

	    }

	    else if (op_cod == UNION) {

	      p_command->number = com_cnt;
	      p_command->operator = P_OR;
	      p_command->left = p_leftop;
	      p_command->right = p1_command;
	      strcpy(p_command->argument, "");

	      if ( log_file ) fprintf(log_file, "R%d := R%d P_OR R%d;\n",  com_cnt, p_leftop->number, p1_command->number);
	      if ( mil_file ) fprintf(mil_file, "\n\tR%d := R%d.or(R%d);\n",  com_cnt, p_leftop->number, p1_command->number);

	    }


	    if (op_cod == UNION || op_cod == INTERSECT) {
	      if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p_leftop->number);
	      if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n\n", p1_command->number);
	    }

	  }

	  else
	    num_bracket--;

	  for (br_count = 0; br_count < num_bracket; br_count++) {

	    POP_OP();
	    op_sp--;

	  }

	  p1_command = p_command;

	  if (op_sp >0) {

	    POP_OP();

	    if ( op_cod != OB) {

	      op_sp--;

	      com_cnt++;
	      p_command++;

	      if (op_cod == INTERSECT) {

	        p_command->number = com_cnt;
	        p_command->operator = P_AND;
	        p_command->left = p_leftop;
	        p_command->right = p1_command;
	        strcpy(p_command->argument, "");

		if ( log_file ) fprintf(log_file, "R%d := R%d P_AND R%d;\n",  com_cnt, p_leftop->number, p1_command->number);
		if ( mil_file ) fprintf(mil_file, "\n\tR%d := R%d.and(R%d);\n",  com_cnt, p_leftop->number, p1_command->number);

	      }

	      else {

	        p_command->number = com_cnt;
	        p_command->operator = P_OR;
	        p_command->left = p_leftop;
	        p_command->right = p1_command;
	        strcpy(p_command->argument, "");

		if ( log_file ) fprintf(log_file, "R%d := R%d P_OR R%d;\n",  com_cnt, p_leftop->number, p1_command->number);
		if ( mil_file ) fprintf(mil_file, "\n\tR%d := R%d.or(R%d);\n",  com_cnt, p_leftop->number, p1_command->number);

	      }

	      if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n", p_leftop->number);
	      if ( mil_file ) fprintf(mil_file, "\tR%d := nil;\n\n", p1_command->number);

	    }

	    p1_command = p_command;

	  }

	}

	if ( mil_file ) fprintf(mil_file, "\nR%d := exit_IR(R%d,R%d);\n",  com_cnt, p_ctx->number, p1_command->number);

	fscanf (command_file, "%d", &parse_com);
	/*printf("e1:%d\n",parse_com); */

	/* checking if the end of the query is reached */

	if (!feof(command_file) && parse_com != QUERY_END && parse_com != P_PRIOR) {

	  follow_step = TRUE;
	  first_step = TRUE;
	  res_sp++;
	  PUSH_RES(p_command);
	  p1_command = p_command;

	}

	else {

	  if (res_sp >0) {

	    POP_RES();
	    res_sp--;

	    if ( mil_file ) fprintf(mil_file, "\nR%d := cross_contained(R%d,R%d,R%d);\n",  com_cnt, p_ctx->number, p_command->number, p_result->number);

	    if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p_ctx->number);
	    if ( mil_file ) fprintf(mil_file, "R%d := nil;\n\n", p_result->number);

	  }

	  /* incorporating the prior into the model */
	  if (!feof(command_file) && parse_com == P_PRIOR) {

	    if (rel_feedback != NULL) {

	      if (rel_feedback->rf_type != RF_SIZE || rel_feedback->rf_type != RF_JOURNAL_SIZE || rel_feedback->rf_type != RF_ELEMENT_SIZE || rel_feedback->rf_type != RF_ALL ) {

	      	p1_command = p_command;

	      	com_cnt++;
		p_command++;

		p_command->number = com_cnt;
		p_command->operator = P_PRIOR;
		p_command->left = p1_command;
	      	p_command->right = NULL;
		strcpy(p_command->argument, "");

		if ( log_file ) fprintf(log_file, "R%d := P_PRIOR(R%d);\n",  com_cnt, p1_command->number);

		if ( mil_file ) fprintf(mil_file, "\nR%d := R%d.log_normal_prior(%d);\n",  com_cnt, p1_command->number, rel_feedback->prior_size);

	      }

	      rel_feedback = rel_feedback->next;

	    }
	    else {

		if (txt_retr_model->prior_type == LENGTH_PRIOR || txt_retr_model->prior_type == LOG_NORMAL_PRIOR) {

		  p1_command = p_command;

		  com_cnt++;
		  p_command++;

		  p_command->number = com_cnt;
		  p_command->operator = P_PRIOR;
		  p_command->left = p1_command;
		  p_command->right = NULL;
		  strcpy(p_command->argument, "");

		  if ( log_file ) fprintf(log_file, "R%d := P_PRIOR(R%d);\n",  com_cnt, p1_command->number);

		  if (txt_retr_model->prior_type == LENGTH_PRIOR) {
		    if ( mil_file ) fprintf(mil_file, "\nR%d := R%d.length_prior();\n",  com_cnt, p1_command->number);
		  } else if (txt_retr_model->prior_type == LOG_NORMAL_PRIOR)
		    if ( mil_file ) fprintf(mil_file, "\nR%d := R%d.log_normal_prior(%d);\n",  com_cnt, p1_command->number, txt_retr_model->prior_size);

		}

		txt_retr_model = txt_retr_model->next;

	    }

	    fscanf (command_file, "%d", &parse_com);

	  }


	  /* writing final topic in MIL */

	  if ( mil_file ) fprintf(mil_file, "%s_%d := R%d;\n",  result_name, topic_num, p_command->number);
	  if ( mil_file ) fprintf(mil_file, "R%d := nil;\n", p_command->number);

	  if ( mil_file ) fprintf(mil_file, "%s_%d.persists(true).rename(\"%s%d_probab\");\n", result_name, topic_num, result_name, topic_num);
	  if ( mil_file ) fprintf(mil_file, "unload(\"%s%d_probab\");\n", result_name, topic_num);

	  if ( mil_file ) fprintf(mil_file, "printf(\"Topic %d finished!\\n\");\n", topic_num);

	}

      }

    }

    if ( log_file ) fprintf(log_file, "\n\n");
    if ( mil_file ) fprintf(mil_file, "\n\n\n");


    /* checking for carriage return, line feed, tabs, etc. and for the end of input file */
    fscanf (command_file, "%d", &parse_com_tmp);

    if (feof(command_file))
      end_process = TRUE;
    else {

      if (parse_com_tmp == QUERY_END) {

	while (!feof(command_file) && parse_com_tmp == QUERY_END)
	  fscanf (command_file, "%d", &parse_com_tmp);

      }

      if (!feof(command_file))
        fseek(command_file, -2, SEEK_CUR );

    }

    *p_command_array = p_command;
    p_command_array++;

    /* printf("%d\n",p_command_array); */
    /* printf("%d\n",p_command); */

    p_command++;

  }

  if ( mil_file ) fprintf(mil_file, "topics.persists(true).rename(\"topics\");\n");
  if ( mil_file ) fprintf(mil_file, "unload(\"topics\");\n");

  fclose(command_file);
  fclose(token_file);
  if ( mil_file ) fclose(mil_file);
  if ( log_file ) fclose(log_file);

  /*printf("\tQuery plans generated.\n"); */

  *p_command_array = NULL;

  return p_command_start;

}

