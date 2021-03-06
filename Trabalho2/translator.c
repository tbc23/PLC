#include "translator.h"

void ifInstr (char **r, char *expr, char *instr, int *count) {
    asprintf (r, "%sjz cond%d\n%scond%d:\n", expr, *count, instr, *count);
    *count = *count + 1;
}

void ifElse (char **r, char *expr, char *instr1, char *instr2, int *count) {
    asprintf (r, "%sjz cond%d\n%sjump cond%d\ncond%d:\n%scond%d:\n", expr, *count, instr1, *count + 1, *count, instr2, *count + 1);
    *count = *count + 2;
}

void ifElseif (char **r, char *expr, char *instr, char *cond, int *count) {
    asprintf (r, "%sjz cond%d\n%sjump cond%d\ncond%d:\n%s", expr, *count, instr, *count - 1, *count, cond);
    *count = *count + 1;
}

void exprAtr (char **r, char *id, char *expr, AVLTree *vars) {
    int sp, size;
    char *class, *type;
    if (!searchAVL (*vars, id, &class, &type, &size, &sp)) notDeclared (r, id, "variable");
    else if (strcmp (class, "var")==0) asprintf (r, "%sstoreg %d\n", expr, sp);
    else if (strcmp (class, "array")==0) assignIntArray (r, id);
    else if (strcmp (class, "func")==0) assignFunc (r, id);
}

void arrayAtr (char **r, char *id, char *instr, char *expr, AVLTree *vars, int *count, int line) {
    int sp, size;
    char *class, *type;
    if (!searchAVL (*vars, id, &class, &type, &size, &sp)) notDeclared (r, id, "array");
    else if (strcmp (class, "array")==0) {
        char *error_str = outOfRange (id, instr, size, count, line);
        asprintf (r, "%spushgp\npushi %d\npadd\n%s%sstoren\n", 
                  error_str, sp, instr, expr);
    }
    else if (strcmp (class, "var")==0) intIndex (r, id);
    else if (strcmp (class, "func")==0) assignFunc (r, id);
}

void readAtr (char **r, char *id, char *instr, AVLTree *vars) {
    int sp, size;
    char *class, *type;
    if (!searchAVL (*vars, id, &class, &type, &size, &sp)) notDeclared (r, id, "variable");
    else if (strcmp (class, "var")==0) asprintf (r, "%sstoreg %d\n", instr, sp);
    else if (strcmp (class, "array")==0) assignIntArray (r, id);
    else if (strcmp (class, "func")==0) assignFunc (r, id);
}

void readArrayAtr (char **r, char *id, char *instr1, char *instr2, AVLTree *vars, int *count, int line) {
    int sp, size;
    char *class, *type;
    if (!searchAVL (*vars, id, &class, &type, &size, &sp)) notDeclared (r, id, "array");
    else if (strcmp (class, "array")==0) {
        char *error_str = outOfRange (id, instr1, size, count, line);
        asprintf (r, "%spushgp\npushi %d\npadd\n%s%s\nstoren\n", error_str, sp, instr1, instr2);
    }
    else if (strcmp (class, "var")==0) intIndex (r, id);
    else if (strcmp (class, "func")==0) assignFunc (r, id);
}

void declaration (char **r, char *id, int *count, AVLTree *vars) {
    char *class, *type; int size, sp;
    if (searchAVL (*vars, id, &class, &type, &size, &sp)) reDeclaration (r, id);
    else {
        insertAVL (vars, id, "var", "int", 1, *count);
        asprintf (r, "pushn 1\n");
        *count = *count + 1;
    }
}

void declrArray (char **r, char *id, char *index, char *count, AVLTree *vars) {
    char *class, *type; int size, sp;
    if (searchAVL (*vars, id, &class, &type, &size, &sp)) reDeclaration (r, id);
    else if (index <= 0) myyyerror (r, "Array size must be a natural number.");
    else {
        insertAVL (vars, id, "array", "int", index, *count);
        asprintf (r, "pushn %d\n", index);
        *count = *count + index;
    }
}

void declrExpr (char **r, char *id, char *expr, AVLTree *vars, int *count) {
    char *class, *type; int size, sp;
    if (searchAVL (*vars, id, &class, &type, &size, &sp)) reDeclaration (r, id);
    else {
        insertAVL (vars, id, "var", "int", 1, *count);
        asprintf (r, "pushn 1\n%sstoreg %d\n", expr, *count);
        *count = *count + 1;
    }
}

void declrRead (char **r, char *id, char *instr, AVLTree *vars, int *count) {
    char *class, *type; int size, sp;
    if (searchAVL (*vars, id, &class, &type, &size, &sp)) reDeclaration (r, id);
    else {
        insertAVL (vars, id, "var", "int", 1, *count);
        asprintf (r, "pushn 1\n%sstoreg %d\n", instr, *count);
        *count = *count + 1;
    }
}

void decList (char **r, char *id, int index, char *instr, AVLTree *vars, int *count, int *size) {
    if (*size == index) {
        char *class, *type; int size, sp;
        if (searchAVL (*vars, id, &class, &type, &size, &sp)) reDeclaration (r, id);
        else {
            insertAVL (vars, id, "array", "int", index, *count);
            asprintf (r, "%s", instr);
            *count = *count + index;
        }
    }
    else indexSizeDontMatch (r, id, index, *size);
    *size = 0;
}

void declrFunc (char **r, char *id, char *instrs1, char *instrs2, AVLTree *vars, char *ftype) {
    char *class, *type; int size, sp;
    if (searchAVL (*vars, id, &class, &type, &size, &sp)) reDeclaration (r, id);
    else {
        insertAVL (vars, id, "func", ftype, 1, -1);
        if (strcmp (ftype, "void")==0) asprintf (r, "\n%s:\nnop\n%sreturn\n%s", id, instrs1, instrs2);
        else if (strcmp (ftype, "int")==0) asprintf (r, "\n%s:\nnop\n%sstorel -1\nreturn\n%s", id, instrs1, instrs2);
    }
}

void factorId (char **r, char *id, AVLTree *vars) {
    int sp, size;
    char *class, *type;
    if (!searchAVL (*vars, id, &class, &type, &size, &sp)) notDeclared (r, id, "variable");
    else if (strcmp (class, "var")==0) asprintf (r, "pushg %d\n", sp);
    else if (strcmp (class, "array")==0) assignIntArray (r, id);
    else if (strcmp (class, "func")==0) assignFunc (r, id);
}

void factorArray (char **r, char *id, char *instr, AVLTree *vars, int *count, int line) {
    int sp, size;
    char *class, *type;
    if (!searchAVL (*vars, id, &class, &type, &size, &sp)) notDeclared (r, id, "array");
    else if (strcmp (class, "array")==0) {
        char *error_str = outOfRange (id, instr, size, count, line);
        asprintf (r, "%spushgp\npushi %d\npadd\n%sloadn\n", error_str, sp, instr);
    }
    else if (strcmp (class, "var")==0) intIndex (r, id);
    else if (strcmp (class, "func")==0) assignFunc (r, id);
}

void negfactorId (char **r, char *id, AVLTree *vars) {
    int sp, size;
    char *class, *type;
    if (!searchAVL (*vars, id, &class, &type, &size, &sp)) notDeclared (r, id, "variable");
    else if (strcmp (class, "var")==0) asprintf (r, "pushg %d\npushi -1\nmul\n", sp);
    else if (strcmp (class, "array")==0) assignIntArray (r, id);
    else if (strcmp (class, "func")==0) assignFunc (r, id);
}

void negfactorArray (char **r, char *id, char *instr, AVLTree *vars, int *count, int line) {
    int sp, size;
    char *class, *type;
    if (!searchAVL (*vars, id, &class, &type, &size, &sp)) notDeclared (r, id, "array");
    else if (strcmp (class, "array")==0) {
        char *error_str = outOfRange (id, instr, size, count, line);
        asprintf (r, "%spushgp\npushi %d\npadd\n%sloadn\npushi -1\nmul\n", error_str, sp, instr);
    }
    else if (strcmp (class, "var")==0) intIndex (r, id);
    else if (strcmp (class, "func")==0) assignFunc (r, id);
}

void forStartEnd (char **r, char *id, char *expr1, char *expr2, char *instr, AVLTree *vars, int *count) {
    int sp, size;
    char *class, *type;
    if (!searchAVL (*vars, id, &class, &type, &size, &sp)) notDeclared (r, id, "variable");
    else if (strcmp (class, "var")==0) {
        asprintf (r, "%s%sinf\njz cycle%d\n%sstoreg %d\ncycle%d:\n%spushg %d\npushi 1\nadd\nstoreg %d\npushg %d\n%ssupeq\njz cycle%d\ncycle%d:\n", 
                  expr1, expr2, *count + 1, expr1, sp, *count, instr, sp, sp, sp, expr2, *count, *count + 1);
        *count = *count + 2;
    }
    else if (strcmp (class, "array")==0) myyyerror (r, "Can't iterate variable of array, use integer instead.");
    else if (strcmp (class, "func")==0) myyyerror (r, "Can't iterate function, use integer instead.");
}

void forStep (char **r, char *id, char *expr1, char *expr2, char *expr3, char *instr, AVLTree *vars, int *count) {
    int sp, size;
    char *class, *type;
    if (!searchAVL (*vars, id, &class, &type, &size, &sp)) notDeclared (r, id, "variable");
    else if (strcmp (class, "var")==0) {
        char *aux, *aux1;
        asprintf (&aux, "%s%s%spushi 0\ninf\njz cycle%d\ncycle%d:\ninfeq\njz cycle%d\njump cycle%d\ncycle%d:\nsupeq\njz cycle%d\njump cycle%d\n", 
                  expr1, expr2, expr3, *count + 1, *count, *count + 2, *count + 4, *count + 1, *count + 2, *count + 4);
        asprintf (&aux1, "%spushi 0\ninf\njz cycle%d\ncycle%d:\ninfeq\njz cycle%d\njump cycle%d\ncycle%d:\nsupeq\njz cycle%d\njump cycle%d\n", 
                  expr3, *count + 6, *count + 5, *count + 3, *count + 7, *count + 6, *count + 3, *count + 7);
        asprintf (r, "%scycle%d:\n%sstoreg %d\ncycle%d:\n%spushg %d\n%sadd\nstoreg %d\npushg %d\n%s%scycle%d:\n", 
                  aux, *count + 2, expr1, sp, *count + 3, instr, sp, expr3, sp, sp, expr2, aux1, *count + 7);
        *count = *count + 8;
    }
    else if (strcmp (class, "array")==0) myyyerror (r, "Can't iterate variable of array, use integer instead.");
    else if (strcmp (class, "func")==0) myyyerror (r, "Can't iterate function, use integer instead.");
}

void forArrayIV (char **r, char *index, char *v, char *id, char *instr, AVLTree *vars, int *count) {
    int spi, spv, spa, size;
    char *class, *type, *ind_id;
    if (!searchAVL (*vars, index, &class, &type, &size, &spi)) notDeclared (r, index, "variable");
    else if (strcmp (class, "var")!=0) myyyerror (r, "Must use integer to hold array indices.");
    else if (!searchAVL (*vars, v, &class, &type, &size, &spv)) notDeclared (r, v, "variable");
    else if (strcmp (class, "var")!=0) myyyerror (r, "Must use integer to hold array values.");
    else if (!searchAVL (*vars, id, &class, &type, &size, &spa)) notDeclared (r, id, "array");
    else if (strcmp (class, "array")==0) {
        char *init, *update, *jump;
        asprintf (&init, "pushi 0\nstoreg %d\npushgp\npushi %d\npadd\npushi 0\nloadn\nstoreg %d\n", 
                  spi, spa, spv);
        asprintf (&update, "pushi %d\npushg %d\npushi 1\nadd\nstoreg %d\npushgp\npushi %d\npadd\npushg %d\nloadn\nstoreg %d\n", 
                  size, spi, spi, spa, spi, spv);
        asprintf (&jump, "pushg %d\ninfeq\njz cycle%d\n", spi, *count);
        asprintf (r, "%scycle%d:\n%s%s%s", init, *count, instr, update, jump);
        *count = *count + 1;
    }
    else if (strcmp (class, "var")==0) myyyerror (r, "Can't iterate integer, use array instead.");
    else if (strcmp (class, "func")==0) myyyerror (r, "Can't iterate function, use integer instead.");
}

void forCond (char **r, char *id1, char *id2, char *expr1, char *expr2, char *expr3, char *instrs, AVLTree *vars, int *count) {
    int sp, size;
    char *class, *type;
    if (!searchAVL (*vars, id1, &class, &type, &size, &sp)) notDeclared (r, id1, "variable");
    else if (strcmp (class, "var")!=0) myyyerror (r, "Must use integer to hold array indices.");
    else if (strcmp (id1, id2) != 0) myyyerror (r, "Update rule and declaration rule use different variables.");
    else {
        asprintf (r, "%sstoreg %d\n%sjz cycle%d\ncycle%d:\n%s%sstoreg %d\n%snot\njz cycle%d\ncycle%d:\n", 
                  expr1, sp, expr2, *count + 1, *count, instrs, expr3, sp, expr2, *count, *count + 1);
        *count = *count + 2;
    }
}

void funcCall (char **r, char *id, AVLTree *vars) {
    int sp, size;
    char *class, *type;
    if (!searchAVL (*vars, id, &class, &type, &size, &sp)) notDeclared (r, id, "function");
    else if (strcmp (class, "func")!=0) myyyerror (r, "Variable not callable.");
    else if (strcmp (type, "void")!=0) myyyerror (r, "Non-void function not assigning its return.");
    else asprintf (r, "pusha %s\ncall\nnop\n", id);
}

void funcAtr (char **r, char *id1, char *id2, AVLTree *vars) {
    int sp, size;
    char *class, *type1, *type2;
    if (!searchAVL (*vars, id2, &class, &type1, &size, &sp)) notDeclared (r, id2, "function");
    else if (strcmp (class, "func")!=0) myyyerror (r, "Variable not callable.");
    else if (strcmp (type1, "void")==0) myyyerror (r, "Can't atribute value of void function");
    else if (!searchAVL (*vars, id1, &class, &type2, &size, &sp)) notDeclared (r, id1, "variable");
    else if (strcmp (type1, type2)!=0) myyyerror (r, "Function and variable types don't match.");
    else asprintf (r, "pushi 0\npusha %s\ncall\nnop\nstoreg %d\n", id2, sp);
}