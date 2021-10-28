#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include <dirent.h>


#include "types.h"
#include "utils.h"
#include "array.h"
#include "table.h"
#include "lexer.h"
#include "memory.h"
#include "object.h"
#include "data.h"

static char *STR_NULL = "NULL";
//static char *STR_CHAR = "CHAR";
//static char *STR_NUMBER = "NUMBER";
//static char *STR_DATA = "DATA";

value_t
data_sizeof(table_t *tbl)
{
    value_t res = 0;
    itable_t *t;
    for(t = tbl->begin; t != tbl->end; t = t->next){
        res += object_sizeof((object_t *) t->value);
    }
    return res;
}

table_t *
data_clone(table_t *tbl)
{
    table_t *res = table_create();
    itable_t *t;
    for(t = tbl->begin; t != tbl->end; t = t->next){
        object_t *obj;
        if(!(obj = object_clone((object_t *)t->value))){
            printf("data clone, object not clone!\n");
            exit(-1);
        }
        table_rpush(res, (value_p)obj);
    }
    return res;
}

table_t *
data_from(char *str)
{
    table_t *tbl = table_create();
    value_t i;
    for(i = 0; i < strlen(str); i++){
        object_t *obj;
        if(!(obj = qalam_object_malloc(sizeof(char)))){
            printf("string to data convert, bad memory!\n");
            exit(-1);
        }
        obj->type = TP_CHAR;
        obj->num = str[i];
        table_rpush(tbl, (value_p)obj);
    }
    return tbl;
}

table_t *
data_merge(table_t *target, table_t *source){
    itable_t *t;
    for(t = source->begin; t != source->end; t = t->next){
        table_rpush(target, t->value);
    }
    return target;
}

table_t *
data_format(table_t *tbl, table_t *format)
{
    table_t *deformed = table_create();
	itable_t *t = tbl->begin, *f;
	object_t *esp, *obj;

    while( t != tbl->end ){
        obj = (object_t *)t->value;

        if(obj->type == TP_NULL){
			value_t i;
			for(i = 0; i < strlen(STR_NULL); i++){
				if(!(esp = qalam_object_malloc(sizeof(char)))){
                    printf("unable to alloc memory!\n");
                    exit(-1);
                }
                esp->type = TP_CHAR;
                esp->num = (char)STR_NULL[i];
				table_rpush(deformed, (value_p)esp);
			}
			if(!(esp = qalam_object_malloc(sizeof(char)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}
			esp->type = TP_CHAR;
			esp->num = ' ';
			table_rpush(deformed, (value_p)esp);
            t = t->next;
            continue;
        }
		else
		if(obj->type == TP_CHAR){
			value_t num = 0;
			if(obj->num == '%'){
				t = t->next;
				obj = (object_t *)t->value;

				if(obj->num == 's'){
					if(!(f = table_rpop(format))){
						printf("format, %%s require a string data!\n");
						exit(-1);
					}
					obj = (object_t *)f->value;
					if(obj->type != TP_ARRAY){
						printf("%%s must be input string data!\n");
						exit(-1);
					}
                    data_merge(deformed, data_format((table_t *)obj->ptr, format));
					t = t->next;
					continue;
				}
				else
				if(obj->num == '.'){
					t = t->next;
					obj = (object_t *)t->value;
					if(obj->type != TP_CHAR){
						printf("object not a char type!\n");
						exit(-1);
					}
					while(valid_digit(obj->num)){
						if(obj->type != TP_CHAR){
							printf("object not a char type!\n");
							exit(-1);
						}
						num = num * 10 + (char)obj->num - '0';
						t = t->next;
						obj = (object_t *)t->value;
					}
				}

				if(obj->num == 'n'){
					if(!(f = table_rpop(format))){
						printf("format, %%n require a number!\n");
						exit(-1);
					}

					obj = (object_t *)f->value;

					char *fmt = qalam_malloc(sizeof(char) * 255);
					sprintf(fmt, "%%.%lldf", num);

					char *str_num = qalam_malloc(sizeof(char) * 255);
					if(((obj->num - (value_t)obj->num) != 0) || (num > 0)){
						sprintf(str_num, fmt, obj->num);
					}
					else {
						sprintf(str_num, "%lld", (value_t)obj->num);
					}

					qalam_free(fmt);

					value_t i;
					for(i = 0; i < strlen(str_num); i++){
						if(!(esp = qalam_object_malloc(sizeof(char)))){
							printf("unable to alloc memory!\n");
							exit(-1);
						}
						esp->type = TP_CHAR;
						esp->num = (char)str_num[i];
						table_rpush(deformed, (value_p)esp);
					}

					qalam_free(str_num);

					t = t->next;
					continue;
				}
				else
				if(obj->num == 'h'){
					if(!(f = table_rpop(format))){
						printf("format, %%n require a number!\n");
						exit(-1);
					}

					obj = (object_t *)f->value;

					char *fmt = qalam_malloc(sizeof(char) * 255);
					sprintf(fmt, "%%0%lldllx", num);

					char *str_num = qalam_malloc(sizeof(char) * 255);
					if(((obj->num - (value_t)obj->num) != 0) || (num > 0)){
						sprintf(str_num, fmt, obj->num);
					}
					else {
						sprintf(str_num, "%llx", (value_t)obj->num);
					}

					qalam_free(fmt);

					value_t i;
					for(i = 0; i < strlen(str_num); i++){
						if(!(esp = qalam_object_malloc(sizeof(char)))){
							printf("unable to alloc memory!\n");
							exit(-1);
						}
						esp->type = TP_CHAR;
						esp->num = (char)str_num[i];
						table_rpush(deformed, (value_p)esp);
					}

					qalam_free(str_num);

					t = t->next;
					continue;
				}
				else
				if(obj->num == 'H'){
					if(!(f = table_rpop(format))){
						printf("format, %%n require a number!\n");
						exit(-1);
					}

					obj = (object_t *)f->value;

					char *fmt = qalam_malloc(sizeof(char) * 255);
					sprintf(fmt, "%%0%lldllX", num);

					char *str_num = qalam_malloc(sizeof(char) * 255);
					if(((obj->num - (value_t)obj->num) != 0) || (num > 0)){
						sprintf(str_num, fmt, obj->num);
					}
					else {
						sprintf(str_num, "%llX", (value_t)obj->num);
					}

					qalam_free(fmt);

					value_t i;
					for(i = 0; i < strlen(str_num); i++){
						if(!(esp = qalam_object_malloc(sizeof(char)))){
							printf("unable to alloc memory!\n");
							exit(-1);
						}
						esp->type = TP_CHAR;
						esp->num = (char)str_num[i];
						table_rpush(deformed, (value_p)esp);
					}

					qalam_free(str_num);

					t = t->next;
					continue;
				}

				if(!(esp = qalam_object_malloc(sizeof(char)))){
					printf("unable to alloc memory!\n");
					exit(-1);
				}
				esp->type = TP_CHAR;
				esp->num = obj->num;
				table_rpush(deformed, (value_p)esp);

				t = t->next;
				continue;
			}

			if(!(esp = qalam_object_malloc(sizeof(char)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}
			esp->type = TP_CHAR;
			esp->num = obj->num;
			table_rpush(deformed, (value_p)esp);
			t = t->next;
			continue;
		}
		else
		if(obj->type == TP_NUMBER){
			char *str_num = qalam_malloc(sizeof(char) * 255);
			if((obj->num - (value_t)obj->num) != 0){
				sprintf(str_num, "%.16f", obj->num);
			}
			else {
				sprintf(str_num, "%lld", (value_t)obj->num);
			}
			value_t i;
			for(i = 0; i < strlen(str_num); i++){
				if(!(esp = qalam_object_malloc(sizeof(char)))){
					printf("unable to alloc memory!\n");
					exit(-1);
				}
				esp->type = TP_CHAR;
				esp->num = (char)str_num[i];
				table_rpush(deformed, (value_p)esp);
			}

			qalam_free(str_num);

			if(!(esp = qalam_object_malloc(sizeof(char)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}
			esp->type = TP_CHAR;
			esp->num = ' ';
			table_rpush(deformed, (value_p)esp);
			t = t->next;
			continue;
		}
		else
		if(obj->type == TP_ARRAY){
            data_merge(deformed, data_format((table_t *)obj->ptr, format));
			t = t->next;
			continue;
		}

		printf("this type is not acceptable!\n");
		exit(-1);
	}

	return deformed;
}

value_t
data_compare(table_t *tbl1, table_t *tbl2)
{
	if(table_count(tbl1) != table_count(tbl2)){
		return 0;
	}

	itable_t *t1 = tbl1->begin, *t2 = tbl2->begin;
	object_t *obj1, *obj2;

	while( t1 != tbl1->end && t2 != tbl2->end){
		obj1 = (object_t *)t1->value;
		obj2 = (object_t *)t2->value;
		if(obj1->type != obj2->type){
			return 0;
		}
		if(obj1->type == TP_NULL){
			if(obj1->ptr != obj2->ptr){
				return 0;
			}
		}
		else
		if(obj1->type == TP_NUMBER){
			if(obj1->num != obj2->num){
				return 0;
			}
		}
		else
		if(obj1->type == TP_CHAR){
			if(obj1->num != obj2->num){
				return 0;
			}
		}

		t1 = t1->next;
		t2 = t2->next;
	}

	return 1;
}

char *
data_to(table_t *tbl)
{
    char *str = qalam_malloc(table_count(tbl) * sizeof(char));
    value_t i = 0;
    itable_t *b;
    for(b = tbl->begin; b && (b != tbl->end); b = b->next){
        object_t *obj = (object_t *)b->value;
        if(obj->type == TP_CHAR){
            str[i++] = (char)obj->num;
            continue;
        }
    }
    return str;
}

void
data_delete(table_t *tbl)
{
    itable_t *b, *n;
    for( b = tbl->begin; b != tbl->end; b = n ){
        n = b->next;
        object_delete((object_t *)b->value);
        qalam_free(b);
    }
    qalam_free(tbl);
}


