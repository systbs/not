#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "types.h"
#include "utils.h"
#include "token.h"
#include "lexer.h"

#define max(a,b) a > b ? a : b

char *
lexer_load_file(char *path){
	int64_t i;
    FILE *fd;

	char destination [ MAX_PATH ];

	if(*path != '/'){
		char cwd[MAX_PATH];
		if (getcwd(cwd, sizeof(cwd)) == NULL) {
			perror("getcwd() error");
			return NULL;
		}
		utils_combine_path ( destination, cwd, path );
	} else {
		strcpy(destination, path);
	}

    if (!(fd = fopen(destination, "rb"))) {
        fprintf(stderr, "could not open(%s)\n", destination);
        return NULL;
    }
    
    // Current position
    int64_t pos = ftell(fd);
    // Go to end
    fseek(fd, 0, SEEK_END);
    // read the position which is the size
    int64_t chunk = ftell(fd);
    // restore original position
    fseek(fd, pos, SEEK_SET);

    char *buf;

    if (!(buf = malloc(chunk + 1))) {
    	fprintf(stderr, "unable to allocted a block of %zu bytes", chunk);
        return NULL;
    }

    // read the source file
    if ((i = fread(buf, 1, chunk, fd)) < chunk) {
        fprintf(stderr, "read returned %ld\n", i);
        return NULL;
    }

    buf[i] = '\0';

    fclose(fd);
    
    return buf;
}

token_t *
lexer_get_token(lexer_t *lexer){
	return &lexer->token;
}

char *
lexer_get_path(lexer_t *lexer){
	return lexer->path;
}

char *
lexer_get_source(lexer_t *lexer){
	return lexer->source;
}

uint64_t
lexer_get_position(lexer_t *lexer){
	return lexer->position;
}

uint64_t
lexer_get_line(lexer_t *lexer){
	return lexer->line;
}

uint64_t
lexer_get_column(lexer_t *lexer){
	return lexer->column;
}

void 
lexer_set_token(lexer_t *lexer, token_t token){
	lexer->token = (token_t){ 
		.type = token.type,
		.value = token.value,
		.position = token.position,
		.line = token.line,
		.column = token.column
	};
}

void
lexer_set_path(lexer_t *lexer, char *path){
	lexer->path = path;
}

void
lexer_set_source(lexer_t *lexer, char *source){
	lexer->source = source;
}

void
lexer_set_position(lexer_t *lexer, uint64_t position){
	lexer->position = position;
}

void
lexer_set_line(lexer_t *lexer, uint64_t line){
	lexer->line = line;
}

void
lexer_set_column(lexer_t *lexer, uint64_t column){
	lexer->column = column;
}

static void 
lexer_set_token_unpack(lexer_t *lexer, int32_t type, char *value, 
	uint64_t position, uint64_t line, uint64_t column){
	lexer->token = (token_t){ 
		.type = type,
		.value = value,
		.position = position,
		.line = line,
		.column = column
	};
}

lexer_t *
lexer_create(char *path, char *source){
	lexer_t *lexer;
	
	if(!(lexer = (lexer_t *)malloc(sizeof(lexer_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(lexer_t));
		return NULL;
	}
	memset(lexer, 0, sizeof(lexer_t));
	
	lexer->path = path;
	lexer->source = source;
	lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    
    lexer_set_token_unpack(lexer, TOKEN_EOF, NULL, 0, 0, 0);
    
	return lexer;
}

void
lexer_destroy(lexer_t *lexer){
	free(lexer);
}

int32_t
lexer_get_next_token(lexer_t *lexer){
	
	int32_t c, a;
    char *source = lexer->source;

    while ((c = source[lexer->position])) {
        if(c == '\n' || c == '\v'){
            lexer->line++;
            lexer->position++;
            lexer->column = 1;
            continue;
        }

        if(utils_white_space(c)){
            lexer->position++;
            lexer->column++;
            continue;
        }

        if(!utils_isalpha(c) && !utils_isdigit(c) && c != '_'){
            if(c == '"' || c == '\''){
                // parse string literal, currently, the only supported escape
                // character is '\n', store the string literal into data.
                uint64_t start_position = lexer->position + 1;
                lexer->position++;
                lexer->column++;

                while ((a = source[lexer->position]) && a != c) {
                    lexer->position++;
                    lexer->column++;
                }

                char *data;
                if(!(data = malloc(sizeof(char) * (lexer->position - start_position)))){
                	fprintf(stderr, "unable to allocted a block of %zu bytes", 
                		sizeof(char) * (lexer->position - start_position));
                	return 0;
                }
                strncpy(data, source + start_position, lexer->position - start_position );
                data[lexer->position - start_position] = '\0';
                
                lexer_set_token(lexer, (token_t){ 
					.type = TOKEN_LETTERS,
					.value = data,
					.position = start_position,
					.line = lexer->line,
					.column = lexer->column-(lexer->position -start_position)
				});       

                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == '('){
            	lexer_set_token(lexer, (token_t){
            		.type = TOKEN_LPAREN, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
            	
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == ')'){
                lexer_set_token(lexer, (token_t){
            		.type = TOKEN_RPAREN, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});;
            		
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == '['){
            	lexer_set_token(lexer, (token_t){
            		.type = TOKEN_LBRACKET, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
            		
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == ']'){
            	lexer_set_token(lexer, (token_t){
            		.type = TOKEN_RBRACKET, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
            		
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == '{'){
            	lexer_set_token(lexer, (token_t){
            		.type = TOKEN_LBRACE, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
            		
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == '}'){
            	lexer_set_token(lexer, (token_t){
            		.type = TOKEN_RBRACE, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
            	
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == '='){
                lexer_set_token(lexer, (token_t){
            		.type = TOKEN_EQ, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
		        	
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == '?'){
            	lexer_set_token(lexer, (token_t){
            		.type = TOKEN_QUESTION, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
		        
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == '|'){
                lexer_set_token(lexer, (token_t){
            		.type = TOKEN_OR, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
				    
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == '&'){
                lexer_set_token(lexer, (token_t){
            		.type = TOKEN_AND, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
				
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == '^'){
            	lexer_set_token(lexer, (token_t){
            		.type = TOKEN_CARET, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
				
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == '!'){
                lexer_set_token(lexer, (token_t){
            		.type = TOKEN_NOT, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
				
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == '<'){
                lexer_set_token(lexer, (token_t){
            		.type = TOKEN_LT, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
					
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == '>'){
                lexer_set_token(lexer, (token_t){
            		.type = TOKEN_GT, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
					
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == '+'){
            	lexer_set_token(lexer, (token_t){
            		.type = TOKEN_PLUS, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
					
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == '-'){
            	lexer_set_token(lexer, (token_t){
            		.type = TOKEN_MINUS, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
					
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == '*'){
            	lexer_set_token(lexer, (token_t){
            		.type = TOKEN_STAR, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
					
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == '/'){
                if ((a = source[lexer->position + 1]) && a == '/') {
                    while ((a = source[lexer->position]) && a != '\n') {
                        lexer->position++;
                        lexer->column++;
                    }
                    lexer->line++;
                    return 1;
                }
                else if ((a = source[lexer->position + 1]) && a == '*') {
                    lexer->position += 2;
                    lexer->column += 2;
                    int64_t i = 0, p = 0;
                    while ((a = source[lexer->position])) {
                        if(a == '*'){
                            if ((a = source[lexer->position + 1]) && a == '/' && i < 1) {
                                break;
                            }
                            i--;
                        }
                        if(a == '\n'){
                            lexer->line++;
                        }
                        if(a == '/'){
                            if ((p = source[lexer->position + 1]) && p == '*') {
                                i++;
                            }
                        }
                        lexer->position++;
                        lexer->column++;
                    }
                    return 1;
                } else {
                	lexer_set_token(lexer, (token_t){
		        		.type = TOKEN_SLASH, 
		        		.value = NULL, 
		        		.position = lexer->position,
		        		.line = lexer->line,
		        		.column = lexer->column
		        	});
						
		            lexer->position++;
		            lexer->column++;
		            return 1;
                }
            }
            else if(c == '%'){
				lexer_set_token(lexer, (token_t){
            		.type = TOKEN_PERCENT, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
            	
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == '.'){
            	lexer_set_token(lexer, (token_t){
            		.type = TOKEN_DOT, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
					
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == ','){
            	lexer_set_token(lexer, (token_t){
            		.type = TOKEN_COMMA, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
					
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == ':'){
            	lexer_set_token(lexer, (token_t){
            		.type = TOKEN_COLON, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
					
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == ';'){
            	lexer_set_token(lexer, (token_t){
            		.type = TOKEN_SEMICOLON, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
				
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == '~'){
            	lexer_set_token(lexer, (token_t){
            		.type = TOKEN_TILDE, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
					
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == '#'){
            	lexer_set_token(lexer, (token_t){
            		.type = TOKEN_HASH, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
					
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == '_'){
            	lexer_set_token(lexer, (token_t){
            		.type = TOKEN_UNDERLINE, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
				
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == '@'){
            	lexer_set_token(lexer, (token_t){
            		.type = TOKEN_AT, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
					
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else if(c == '$'){
            	lexer_set_token(lexer, (token_t){
            		.type = TOKEN_DOLLER, 
            		.value = NULL, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column
            	});
					
                lexer->position++;
                lexer->column++;
                return 1;
            }
            else {
            	fprintf(stderr, "%s (%ld:%ld): error: unknown token", lexer->path, lexer->line, lexer->column);
                return 0;
            }
        }
        else if(utils_isdigit(c)) {
            // parse number, three kinds: dec(123) hex(0x123) oct(017)
            int64_t token_val = c - '0';
            uint64_t start_position = lexer->position;

            if (token_val > 0) {
                lexer->position++;
                lexer->column++;

                // dec, starts with [1-9]
                while ((a = source[lexer->position]) && (utils_isdigit(a) || a == '.') && !utils_white_space(a)) {
                    lexer->position++;
                    lexer->column++;
                }

                char *data;
                if(!(data = malloc(sizeof(char) * (lexer->position - start_position)))){
                	fprintf(stderr, "unable to allocted a block of %zu bytes", 
                		sizeof(char) * (lexer->position - start_position));
                	return 0;
                }
                strncpy(data, source + start_position, lexer->position - start_position );
                data[lexer->position - start_position] = '\0';
					
				lexer_set_token(lexer, (token_t){
            		.type = TOKEN_NUMBER, 
            		.value = data, 
            		.position = lexer->position,
            		.line = lexer->line,
            		.column = lexer->column-(lexer->position-start_position)
            	});
				
                return 1;
            } else {
                // starts with number 0
                a = source[lexer->position + 1];
                if (a == 'x' || a == 'X') {
                    lexer->position+=2;
                    lexer->column+=2;
                    //hex
                    while ((a = source[lexer->position]) && utils_ishex(a)) {
                        token_val = token_val * 16 + (a & 15) + (a >= 'A' ? 9 : 0);
                        lexer->position++;
                        lexer->column++;
                    }

                    char *data;
                    if(!(data = malloc(sizeof(char) * (lexer->position - start_position)))){
                    	fprintf(stderr, "unable to allocted a block of %zu bytes", 
                			sizeof(char) * (lexer->position - start_position));
                    	return 0;
                    }
                    strncpy(data, source + start_position, lexer->position - start_position );
                    data[lexer->position - start_position] = '\0';
                    
                    lexer_set_token(lexer, (token_t){
		        		.type = TOKEN_NUMBER, 
		        		.value = data, 
		        		.position = lexer->position,
		        		.line = lexer->line,
		        		.column = lexer->column-(lexer->position-start_position)
		        	});
					
                    return 1;
                } else if(utils_isdigit(a)){
                    // oct
                    while ((a = source[lexer->position]) && utils_isoctal(a)) {
                        token_val = token_val * 8 + a - '0';
                        lexer->position++;
                        lexer->column++;
                    }

                    char *data;
                    if(!(data = malloc(sizeof(char) * (lexer->position - start_position)))){
                    	fprintf(stderr, "unable to allocted a block of %zu bytes", 
                			sizeof(char) * (lexer->position - start_position));
                    	return 0;
                    }
                    strncpy(data, source + start_position, lexer->position - start_position );
                    data[lexer->position - start_position] = '\0';
                    
                    lexer_set_token(lexer, (token_t){
		        		.type = TOKEN_NUMBER, 
		        		.value = data, 
		        		.position = lexer->position,
		        		.line = lexer->line,
		        		.column = lexer->column-(lexer->position-start_position)
		        	});
					
                } else {
                    lexer->position++;
                    lexer->column++;
                }
            }

            char *data;
            if(!(data = malloc(sizeof(char) * (lexer->position - start_position)))){
            	fprintf(stderr, "unable to allocted a block of %zu bytes", 
                	sizeof(char) * (lexer->position - start_position));
            	return 0;
            }
            strncpy(data, source + start_position, lexer->position - start_position );
            data[lexer->position - start_position] = '\0';
            
            lexer_set_token(lexer, (token_t){
            	.type = TOKEN_NUMBER, 
            	.value = data, 
            	.position = lexer->position,
            	.line = lexer->line,
            	.column = lexer->column-(lexer->position-start_position)
            });
				
            return 1;
        }
        else {
            // parse identifier
            uint64_t start_position = lexer->position;
            while ((a = source[lexer->position]) && ( utils_isalpha(a) || utils_isdigit(a) || (a == '_'))) {
                lexer->position++;
                lexer->column++;
            }
            uint32_t length = lexer->position - start_position;

            if(strncmp(source + start_position, "while", max(length, 5)) == 0){
				lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_WHILE, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
				
                return 1;
            } else if(strncmp(source + start_position, "class", max(length, 5)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_CLASS, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            } else if(strncmp(source + start_position, "if", max(length, 2)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_IF, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            } else if(strncmp(source + start_position, "else", max(length, 4)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_ELSE, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            } else if(strncmp(source + start_position, "break", max(length, 5)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_BREAK, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            } else if(strncmp(source + start_position, "continue", max(length, 8)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_CONTINUE, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            } else if(strncmp(source + start_position, "this", max(length, 4)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_THIS, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            } else if(strncmp(source + start_position, "super", max(length, 5)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_SUPER, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            } else if(strncmp(source + start_position, "return", max(length, 6)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_RETURN, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            } else if(strncmp(source + start_position, "import", max(length, 6)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_IMPORT, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            } else if(strncmp(source + start_position, "from", max(length, 4)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_FROM, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            } else if(strncmp(source + start_position, "as", max(length, 2)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_AS, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            } else if(strncmp(source + start_position, "sizeof", max(length, 6)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_SIZEOF, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            } else if(strncmp(source + start_position, "typeof", max(length, 6)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_TYPEOF, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            } else if(strncmp(source + start_position, "null", max(length, 4)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_NULL, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            } else if(strncmp(source + start_position, "fn", max(length, 2)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_FN, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            } else if(strncmp(source + start_position, "var", max(length, 3)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_VAR, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            } else if(strncmp(source + start_position, "enum", max(length, 4)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_ENUM, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            } else if(strncmp(source + start_position, "extern", max(length, 6)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_EXTERN, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            } else if(strncmp(source + start_position, "char", max(length, 4)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_CHAR, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            } else if(strncmp(source + start_position, "string", max(length, 6)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_STRING, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            } else if(strncmp(source + start_position, "int", max(length, 3)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_INT, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            } else if(strncmp(source + start_position, "long", max(length, 4)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_LONG, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            } else if(strncmp(source + start_position, "float", max(length, 5)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_FLOAT, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            } else if(strncmp(source + start_position, "double", max(length, 6)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_DOUBLE, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            } else if(strncmp(source + start_position, "any", max(length, 3)) == 0){
            	lexer_set_token(lexer, (token_t){
		        	.type = TOKEN_ANY, 
		        	.value = NULL, 
		        	.position = lexer->position,
		        	.line = lexer->line,
		        	.column = lexer->column-length
		        });
					
                return 1;
            }

            char *data;
            if(!(data = malloc(sizeof(char) * (length)))){
            	fprintf(stderr, "unable to allocted a block of %zu bytes", 
                	sizeof(char) * (length));
            	return 0;
            }
            strncpy(data, source + start_position, length);
            data[length] = '\0';
            
			lexer_set_token(lexer, (token_t){
		        .type = TOKEN_ID, 
		        .value = data, 
		        .position = lexer->position,
		        .line = lexer->line,
		        .column = lexer->column-length
		    });
					
            return 1;
        }

        lexer->position++;
        lexer->column++;
    }
    
    lexer_set_token(lexer, (token_t){
		.type = TOKEN_EOF, 
		.value = NULL, 
		.position = lexer->position,
		.line = lexer->line,
		.column = lexer->column
	});
    
    return 1;
}
