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
#include "list.h"
#include "lexer.h"
#include "memory.h"
#include "object.h"
#include "data.h"

static char *STR_NULL = "NULL";
//static char *STR_CHAR = "CHAR";
//static char *STR_NUMBER = "NUMBER";
//static char *STR_DATA = "DATA";

long_t
data_sizeof(list_t *lst)
{
    long_t res = 0;
    ilist_t *t;
    for(t = lst->begin; t != lst->end; t = t->next){
        res += object_sizeof((object_t *) t->value);
    }
    return res;
}

list_t *
data_clone(list_t *lst)
{
    list_t *res = list_create();
    ilist_t *t;
    for(t = lst->begin; t != lst->end; t = t->next){
        object_t *obj;
        assert_format(!!(obj = object_clone((object_t *)t->value)),
			"data clone, object not clone");
        list_rpush(res, (list_value_t)obj);
    }
    return res;
}

list_t *
data_from(char *str)
{
    list_t *lst = list_create();
    long_t i;
    for(i = 0; i < strlen(str); i++){
        object_t *obj;
        assert_format(!!(obj = object_define(OTP_CHAR, sizeof(char_t))),
			"data from string, bad object malloc");
        *(char_t *)obj->ptr = (char_t)str[i];
        list_rpush(lst, (list_value_t)obj);
    }
    return lst;
}

list_t *
data_merge(list_t *target, list_t *source){
    ilist_t *t;
    for(t = source->begin; t != source->end; t = t->next){
        list_rpush(target, t->value);
    }
    return target;
}

list_t *
data_format(list_t *lst, list_t *format)
{
    list_t *deformed = list_create();
	ilist_t *t = lst->begin, *f;
	object_t *esp, *obj;

    while( t != lst->end ){
        obj = (object_t *)t->value;

        if(obj->type == OTP_NULL){
			long_t i;
			for(i = 0; i < strlen(STR_NULL); i++){
				assert_format(!!(esp = object_define(OTP_CHAR, sizeof(char_t))),
					"data from string, bad object malloc");
                *(char_t *)esp->ptr = (char_t)STR_NULL[i];
				list_rpush(deformed, (list_value_t)esp);
			}
			assert_format(!!(esp = object_define(OTP_CHAR, sizeof(char_t))),
				"data from string, bad object malloc");
			*(char_t *)esp->ptr = ' ';
			list_rpush(deformed, (list_value_t)esp);
            t = t->next;
            continue;
        }
		else if(obj->type == OTP_CHAR){
			long_t num = 0;
			if(*(char_t *)obj->ptr == '%'){
				t = t->next;
				obj = (object_t *)t->value;

				if(*(char_t *)obj->ptr == 's'){
					assert_format(!!(f = list_lpop(format)), 
						"format, %%s require a string data!");
					
					obj = (object_t *)f->value;

					assert_format((obj->type == OTP_ARRAY),
						"%%s must be input string data!");

                    data_merge(deformed, data_format((list_t *)obj->ptr, format));
					t = t->next;
					continue;
				}
				else if(*(char_t *)obj->ptr == '.'){
					t = t->next;
					obj = (object_t *)t->value;

					assert_format((obj->type == OTP_CHAR),
						"object not a char type!");

					while(valid_digit(*(char_t *)obj->ptr)){
						assert_format(obj->type == OTP_CHAR, 
							"object not a char type");
						num = num * 10 + *(char_t *)obj->ptr - '0';
						t = t->next;
						obj = (object_t *)t->value;
					}
				}

				if(*(char_t *)obj->ptr == 'n'){

					assert_format(!!(f = list_lpop(format)),
						"format, %%n require a number!");

					obj = (object_t *)f->value;

					char_t *fmt = qalam_malloc(sizeof(char_t) * 255);
					sprintf(fmt, "%%.%ldf", num);

					char_t *str_num = qalam_malloc(sizeof(char_t) * 255);
					if(((*(double_t *)obj->ptr - *(long_t *)obj->ptr) != 0) || (num > 0)){
						sprintf(str_num, fmt, oget(obj));
					}
					else {
						sprintf(str_num, "%f", oget(obj));
					}

					// qalam_free(fmt);

					long_t i;
					for(i = 0; i < strlen(str_num); i++){
						assert_format(!!(esp = object_define(OTP_CHAR, sizeof(char_t))),
							"data from string, bad object malloc");
						*(char_t *)esp->ptr = (char_t)str_num[i];
						list_rpush(deformed, (list_value_t)esp);
					}

					// qalam_free(str_num);

					t = t->next;
					continue;
				}
				else
				if(*(char_t *)obj->ptr == 'h'){
					assert_format(!!(f = list_lpop(format)),
						"format, %%n require a number!");

					obj = (object_t *)f->value;

					char_t *fmt = qalam_malloc(sizeof(char_t) * 255);
					sprintf(fmt, "%%0%ldllx", num);

					char_t *str_num = qalam_malloc(sizeof(char_t) * 255);
					if(((*(double_t *)obj->ptr - *(long_t *)obj->ptr) != 0) || (num > 0)){
						sprintf(str_num, fmt, *(double_t *)obj->ptr);
					}
					else {
						sprintf(str_num, "%lx", *(long_t *)obj->ptr);
					}

					// qalam_free(fmt);

					long_t i;
					for(i = 0; i < strlen(str_num); i++){
						assert_format(!!(esp = object_define(OTP_CHAR, sizeof(char_t))),
							"data from string, bad object malloc");
						*(char_t *)esp->ptr = (char_t)str_num[i];
						list_rpush(deformed, (list_value_t)esp);
					}

					// qalam_free(str_num);

					t = t->next;
					continue;
				}
				else
				if(*(char_t *)obj->ptr == 'H'){
					if(!(f = list_lpop(format))){
						printf("format, %%n require a number!\n");
						exit(-1);
					}

					obj = (object_t *)f->value;

					char_t *fmt = qalam_malloc(sizeof(char_t) * 255);
					sprintf(fmt, "%%0%ldllX", num);

					char_t *str_num = qalam_malloc(sizeof(char_t) * 255);
					if(((*(double_t *)obj->ptr - *(long_t *)obj->ptr) != 0) || (num > 0)){
						sprintf(str_num, fmt, *(double_t *)obj->ptr);
					}
					else {
						sprintf(str_num, "%lx", *(long_t *)obj->ptr);
					}

					// qalam_free(fmt);

					long_t i;
					for(i = 0; i < strlen(str_num); i++){
						assert_format(!!(esp = object_define(OTP_CHAR, sizeof(char_t))),
							"data from string, bad object malloc");
						*(char_t *)esp->ptr = (char)str_num[i];
						list_rpush(deformed, (list_value_t)esp);
					}

					// qalam_free(str_num);

					t = t->next;
					continue;
				}

				assert_format(!!(esp = object_define(OTP_CHAR, sizeof(char_t))),
					"data from string, bad object malloc");

				*(char_t *)esp->ptr = *(char_t *)obj->ptr;

				list_rpush(deformed, (list_value_t)esp);

				t = t->next;
				continue;
			}

			assert_format(!!(esp = object_define(OTP_CHAR, sizeof(char_t))),
				"data from string, bad object malloc");

			*(char_t *)esp->ptr = *(char_t *)obj->ptr;

			list_rpush(deformed, (list_value_t)esp);
			t = t->next;
			continue;
		}
		else if(obj->type == OTP_LONG){
			char *str_num = qalam_malloc(sizeof(char_t) * 255);
			sprintf(str_num, "%ld", *(long_t *)obj->ptr);
			long_t i;
			for(i = 0; i < strlen(str_num); i++){
				assert_format(!!(esp = object_define(OTP_CHAR, sizeof(char_t))),
					"data from string, bad object malloc");
				*(char_t *)esp->ptr = (char_t)str_num[i];
				list_rpush(deformed, (list_value_t)esp);
			}

			// qalam_free(str_num);

			assert_format(!!(esp = object_define(OTP_CHAR, sizeof(char_t))),
				"data from string, bad object malloc");

			*(char_t *)esp->ptr = ' ';
			list_rpush(deformed, (list_value_t)esp);
			t = t->next;
			continue;
		}
		else if(obj->type == OTP_DOUBLE){
			char *str_num = qalam_malloc(sizeof(char_t) * 255);
			sprintf(str_num, "%.16f", *(double_t *)obj->ptr);
			long_t i;
			for(i = 0; i < strlen(str_num); i++){
				assert_format(!!(esp = object_define(OTP_CHAR, sizeof(char_t))),
					"data from string, bad object malloc");
				*(char_t *)esp->ptr = (char_t)str_num[i];
				list_rpush(deformed, (list_value_t)esp);
			}

			// qalam_free(str_num);

			assert_format(!!(esp = object_define(OTP_CHAR, sizeof(char_t))),
				"data from string, bad object malloc");

			*(char_t *)esp->ptr = ' ';
			list_rpush(deformed, (list_value_t)esp);
			t = t->next;
			continue;
		}
		else if(obj->type == OTP_ARRAY){
            data_merge(deformed, data_format((list_t *)obj->ptr, format));
			t = t->next;
			continue;
		}

		printf("this type is not acceptable!\n");
		exit(-1);
	}

	return deformed;
}

long_t
data_compare(list_t *lst1, list_t *lst2)
{
	if(list_count(lst1) != list_count(lst2)){
		return 0;
	}

	ilist_t *t1 = lst1->begin, *t2 = lst2->begin;
	object_t *obj1, *obj2;

	while( t1 != lst1->end && t2 != lst2->end){
		obj1 = (object_t *)t1->value;
		obj2 = (object_t *)t2->value;
		if(obj1->type != obj2->type){
			return 0;
		}
		if(obj1->type == OTP_NULL){
			if(obj1->ptr != obj2->ptr){
				return 0;
			}
		}
		else if(obj1->type == OTP_LONG){
			if(*(long_t *)obj1->ptr != *(long_t *)obj2->ptr){
				return 0;
			}
		}
		else if(obj1->type == OTP_DOUBLE){
			if(*(double_t *)obj1->ptr != *(double_t *)obj2->ptr){
				return 0;
			}
		}
		else if(obj1->type == OTP_CHAR){
			if(*(char_t *)obj1->ptr != *(char_t *)obj2->ptr){
				return 0;
			}
		}

		t1 = t1->next;
		t2 = t2->next;
	}

	return 1;
}

char *
data_to(list_t *lst)
{
    char *str = qalam_malloc(list_count(lst) * sizeof(char));
    long_t i = 0;
    ilist_t *b;
    for(b = lst->begin; b && (b != lst->end); b = b->next){
        object_t *obj = (object_t *)b->value;
        if(obj->type == OTP_CHAR){
            str[i++] = *(char_t *)obj->ptr;
            continue;
        }
    }
    return str;
}

void
data_delete(list_t *lst)
{
    ilist_t *b, *n;
    for( b = lst->begin; b != lst->end; b = n ){
        n = b->next;
        object_delete((object_t *)b->value);
        qalam_free(b);
    }
    qalam_free(lst);
}


